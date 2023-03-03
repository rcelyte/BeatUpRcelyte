#include "common.h"
#include <stdlib.h>
#include <time.h>

#define NET_MAX_SEQUENCE 0x8000
static inline int16_t RelativeSequenceNumber(uint16_t to, uint16_t from) {
	return (int16_t)((to - from + 0x4000u) & 0x7fffu) - 0x4000;
}

static inline bool GetBit(const uint8_t *data, uint32_t bit) {
	return (data[bit / 8] >> (bit % 8)) & 1u;
}

static inline bool SetBit(uint8_t *data, uint32_t bit) {
	bool prev = GetBit(data, bit);
	data[bit / 8] |= 1u << (bit % 8);
	return prev;
}

static inline bool ClearBit(uint8_t *data, uint32_t bit) {
	bool prev = GetBit(data, bit);
	data[bit / 8] &= ~(1u << (bit % 8));
	return prev;
}

void instance_channels_init(struct Channels *channels) {
	*channels = (struct Channels){
		.ru.base = {
			.ack.channelId = DeliveryMethod_ReliableUnordered,
			.backlogEnd = &channels->ru.base.backlog,
		},
		.ro.base = {
			.ack.channelId = DeliveryMethod_ReliableOrdered,
			.backlogEnd = &channels->ro.base.backlog,
			.outboundSequence = 1, // ID 0 is skipped due to window size probing
		},
		.rs.ack.channelId = DeliveryMethod_ReliableSequenced,
	};
}

void instance_channels_reset(struct Channels *channels) {
	while(channels->ru.base.backlog) {
		struct InstancePacketList *e = channels->ru.base.backlog;
		channels->ru.base.backlog = e->next;
		free(e);
	}
	while(channels->ro.base.backlog) {
		struct InstancePacketList *e = channels->ro.base.backlog;
		channels->ro.base.backlog = e->next;
		free(e);
	}
	while(channels->incomingFragmentsList) {
		struct IncomingFragments *e = channels->incomingFragmentsList;
		channels->incomingFragmentsList = e->next;
		free(e);
	}
	instance_channels_init(channels);
}

static struct InstancePacket *resend_add(struct PacketContext version, struct ReliableChannel *channel, DeliveryMethod method, bool isFragmented) {
	struct InstanceResendPacket *resend = &channel->resend[channel->outboundSequence % version.windowSize];
	resend->timeStamp = net_time() - NET_RESEND_DELAY;
	resend->pkt.len = (uint16_t)pkt_write_c((uint8_t*[]){resend->pkt.data}, endof(resend->pkt.data), version, NetPacketHeader, {
		.property = PacketProperty_Channeled,
		.isFragmented = isFragmented,
		.channeled = {
			.sequence = channel->outboundSequence,
			.channelId = method,
		},
	});
	channel->outboundSequence = (channel->outboundSequence + 1) % NET_MAX_SEQUENCE;
	return &resend->pkt;
}

static void instance_send_backlog(struct PacketContext version, struct Channels *channels, const uint8_t *buf, uint16_t len, DeliveryMethod channelId, struct FragmentedHeader fragmentHeader) {
	if(channelId != DeliveryMethod_ReliableOrdered) {
		uprintf("instance_send_channeled(DeliveryMethod_%s) not implemented\n", reflect(DeliveryMethod, channelId));
		abort();
	}
	struct ReliableChannel *channel = &channels->ro.base;
	bool isFragmented = (fragmentHeader.fragmentsTotal != 0);
	struct InstancePacket *packet = NULL;
	if(RelativeSequenceNumber(channel->outboundSequence, channel->outboundWindowStart) < version.windowSize) {
		packet = resend_add(version, channel, channelId, isFragmented);
	} else {
		*channels->ro.base.backlogEnd = malloc(sizeof(struct InstancePacketList));
		if(!*channels->ro.base.backlogEnd) {
			uprintf("alloc error\n");
			abort();
		}
		(*channels->ro.base.backlogEnd)->next = NULL;
		(*channels->ro.base.backlogEnd)->isFragmented = isFragmented;
		packet = &(*channels->ro.base.backlogEnd)->pkt;
		packet->len = 0;
		channels->ro.base.backlogEnd = &(*channels->ro.base.backlogEnd)->next;
	}
	if(isFragmented)
		packet->len += pkt_write(&fragmentHeader, (uint8_t*[]){&packet->data[packet->len]}, endof(packet->data), version);
	packet->len += pkt_write_bytes(buf, (uint8_t*[]){&packet->data[packet->len]}, endof(packet->data), version, len);
}

void instance_send_channeled(struct NetSession *session, struct Channels *channels, const uint8_t *buf, uint32_t len, DeliveryMethod channelId) {
	if(len <= session->maxChanneledSize) {
		instance_send_backlog(session->version, channels, buf, (uint16_t)len, channelId, (struct FragmentedHeader){0});
		return;
	}
	if(channelId != DeliveryMethod_ReliableUnordered && channelId != DeliveryMethod_ReliableOrdered) {
		uprintf("Packet too large (%u > %u)\n", len, session->maxChanneledSize);
		return;
	}
	if(session->maxFragmentSize == 0 || session->maxFragmentSize > NET_MAX_PKT_SIZE) {
		uprintf("CORRUPTION DETECTED: `session->maxFragmentSize` out of range\n");
		return; // Guard against SIGFPE
	}
	uint32_t fragmentCount = (len + session->maxFragmentSize - 1) / session->maxFragmentSize;
	if(fragmentCount >= UINT16_MAX) {
		uprintf("Reliable packet too large (%u >= %u)\n", len, session->maxFragmentSize * (UINT16_MAX - 1));
		return;
	}
	struct FragmentedHeader fragmentHeader = {
		.fragmentId = ++session->fragmentId,
		.fragmentsTotal = (uint16_t)fragmentCount,
	};
	for(fragmentHeader.fragmentPart = 0; fragmentHeader.fragmentPart < fragmentHeader.fragmentsTotal - 1; ++fragmentHeader.fragmentPart, buf += session->maxFragmentSize, len -= session->maxFragmentSize)
		instance_send_backlog(session->version, channels, buf, session->maxFragmentSize, channelId, fragmentHeader);
	instance_send_backlog(session->version, channels, buf, (uint16_t)len, channelId, fragmentHeader);
}

static void ReliableChannel_popBacklog(struct ReliableChannel *channel, struct PacketContext version, DeliveryMethod channelId) {
	struct InstancePacket *resend = resend_add(version, channel, channelId, channel->backlog->isFragmented);
	resend->len += pkt_write_bytes(channel->backlog->pkt.data, (uint8_t*[]){&resend->data[resend->len]}, endof(resend->data), version, channel->backlog->pkt.len);

	struct InstancePacketList *e = channel->backlog;
	channel->backlog = e->next;
	free(e);
	if(!channel->backlog)
		channel->backlogEnd = &channel->backlog;
}

static void ReliableChannel_flushBacklog(struct ReliableChannel *channel, struct PacketContext version, DeliveryMethod channelId) {
	while(RelativeSequenceNumber(channel->outboundSequence, channel->outboundWindowStart) < version.windowSize && channel->backlog != NULL)
		ReliableChannel_popBacklog(channel, version, channelId);
}

void instance_channels_flushBacklog(struct Channels *channels, struct NetSession *session) {
	ReliableChannel_flushBacklog(&channels->ru.base, session->version, DeliveryMethod_ReliableUnordered);
	ReliableChannel_flushBacklog(&channels->ro.base, session->version, DeliveryMethod_ReliableOrdered);
}

void handle_Ack(struct NetSession *session, struct Channels *channels, const struct Ack *ack) {
	if(ack->channelId == DeliveryMethod_ReliableSequenced) {
		if(ack->sequence == channels->rs.outboundSequence)
			channels->rs.resend.pkt.len = 0;
		return;
	}
	struct ReliableChannel *channel = (ack->channelId == DeliveryMethod_ReliableUnordered) ? &channels->ru.base : &channels->ro.base;
	if(ack->sequence >= NET_MAX_SEQUENCE || RelativeSequenceNumber(channel->outboundWindowStart, ack->sequence) < 0) {
		uprintf("BAD ACK WINDOW\n");
		return;
	}
	for(uint16_t sequence = channel->outboundWindowStart, end = channel->outboundSequence; sequence != end; sequence = (sequence + 1) % NET_MAX_SEQUENCE) {
		if(RelativeSequenceNumber(sequence, ack->sequence) >= session->version.windowSize)
			break;
		uint16_t pendingIdx = sequence % session->version.windowSize;
		if(GetBit(ack->data, pendingIdx))
			channel->resend[pendingIdx].pkt.len = 0;
		if(channel->resend[pendingIdx].pkt.len || sequence != channel->outboundWindowStart)
			continue;
		channel->outboundWindowStart = (channel->outboundWindowStart + 1) % NET_MAX_SEQUENCE;
		if(channel->backlog != NULL)
			ReliableChannel_popBacklog(channel, session->version, ack->channelId);
	}
}

static void process_Reliable(ChanneledHandler handler, struct PacketContext version, struct Channels *channels, void *userptr, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId, bool isFragmented) {
	if(!isFragmented) {
		handler(userptr, data, end, channelId);
		return;
	}
	struct FragmentedHeader header;
	if(!pkt_read(&header, data, end, version))
		return;
	struct IncomingFragments **incoming = &channels->incomingFragmentsList;
	while(*incoming != NULL && (*incoming)->fragmentId != header.fragmentId)
		incoming = &(*incoming)->next;
	if(*incoming == NULL) {
		*incoming = malloc(sizeof(**incoming) + header.fragmentsTotal * sizeof(*(*incoming)->fragments));
		if(*incoming == NULL) {
			uprintf("alloc error\n");
			abort();
		}
		(*incoming)->next = NULL;
		(*incoming)->fragmentId = header.fragmentId;
		(*incoming)->channelId = channelId;
		(*incoming)->count = 0;
		(*incoming)->total = header.fragmentsTotal;
		(*incoming)->size = 0;
		for(uint32_t i = 0; i < header.fragmentsTotal; ++i)
			(*incoming)->fragments[i].len = 0;
	}
	if(header.fragmentPart >= (*incoming)->total || (*incoming)->fragments[header.fragmentPart].len || channelId != (*incoming)->channelId)
		return;
	(*incoming)->size += end - *data;
	if(++(*incoming)->count < (*incoming)->total) {
		(*incoming)->fragments[header.fragmentPart].len = (uint16_t)(end - *data);
		memcpy((*incoming)->fragments[header.fragmentPart].data, *data, (size_t)(end - *data));
		*data = end;
		return;
	}
	uint8_t pkt[(*incoming)->size], *pkt_end = pkt;
	for(uint32_t i = 0; i < header.fragmentPart; ++i)
		pkt_write_bytes((*incoming)->fragments[i].data, &pkt_end, endof(pkt), version, (*incoming)->fragments[i].len);
	pkt_write_bytes(*data, &pkt_end, endof(pkt), version, (size_t)(end - *data));
	*data = end;
	for(uint32_t i = header.fragmentPart + 1; i < (*incoming)->total; ++i)
		pkt_write_bytes((*incoming)->fragments[i].data, &pkt_end, endof(pkt), version, (*incoming)->fragments[i].len);
	const uint8_t *pkt_it = pkt;
	handler(userptr, &pkt_it, pkt_end, channelId);
	pkt_debug("BAD FRAGMENTED PACKET LENGTH", pkt_it, pkt_end, (size_t)(pkt_end - pkt), version);

	struct IncomingFragments *e = *incoming;
	*incoming = e->next;
	free(e);
}

void handle_Channeled(ChanneledHandler handler, void *userptr, struct NetContext *net, struct NetSession *session, struct Channels *channels, const struct NetPacketHeader *header, const uint8_t **data, const uint8_t *end) {
	struct Channeled channeled = header->channeled;
	if(channeled.sequence >= NET_MAX_SEQUENCE)
		return;
	struct ReliableChannel *channel = &channels->ro.base;
	switch(channeled.channelId) {
		case DeliveryMethod_ReliableUnordered: channel = &channels->ru.base; [[fallthrough]];
		case DeliveryMethod_ReliableOrdered: {
			if(RelativeSequenceNumber(channeled.sequence, channel->inboundSequence) > session->version.windowSize)
				break;
			int16_t delta = RelativeSequenceNumber(channeled.sequence, channel->ack.sequence) - (int16_t)session->version.windowSize;
			if(delta < -session->version.windowSize || delta >= session->version.windowSize)
				break;
			while(delta >= 0) {
				uint16_t ackIdx = channel->ack.sequence % session->version.windowSize;
				ClearBit(channel->ack.data, ackIdx);
				channel->ack.sequence = (channel->ack.sequence + 1) % NET_MAX_SEQUENCE;
				--delta;
			}
			channel->sendAck = true;
			uint16_t ackIdx = channeled.sequence % session->version.windowSize;
			if(SetBit(channel->ack.data, ackIdx))
				break;
			if(channeled.sequence == channel->inboundSequence) {
				process_Reliable(handler, session->version, channels, userptr, data, end, channeled.channelId, header->isFragmented);
				channel->inboundSequence = (channel->inboundSequence + 1) % NET_MAX_SEQUENCE;
				if(channeled.channelId == DeliveryMethod_ReliableUnordered) {
					while(ClearBit(channels->ru.earlyReceived, channel->inboundSequence % session->version.windowSize))
						channel->inboundSequence = (channel->inboundSequence + 1) % NET_MAX_SEQUENCE;
					return;
				}
				while(channels->ro.receivedPackets[channel->inboundSequence % session->version.windowSize].len) {
					struct EarlyReceivedPacket *ipkt = &channels->ro.receivedPackets[channel->inboundSequence % session->version.windowSize];
					const uint8_t *const pkt = ipkt->data, *pkt_it = ipkt->data;
					process_Reliable(handler, session->version, channels, userptr, &pkt_it, &pkt[ipkt->len], DeliveryMethod_ReliableOrdered, ipkt->isFragmented);
					if(pkt_it != &pkt[ipkt->len])
						uprintf("BAD RELIABLE PACKET LENGTH (expected %zu, read %zu)\n", ipkt->len, pkt_it - pkt);
					ipkt->len = 0;
					channel->inboundSequence = (channel->inboundSequence + 1) % NET_MAX_SEQUENCE;
				}
				return;
			}
			if(channeled.channelId != DeliveryMethod_ReliableOrdered) {
				SetBit(channels->ru.earlyReceived, ackIdx);
				process_Reliable(handler, session->version, channels, userptr, data, end, DeliveryMethod_ReliableUnordered, header->isFragmented);
				return;
			}
			channels->ro.receivedPackets[ackIdx].len = (uint16_t)(end - *data);
			channels->ro.receivedPackets[ackIdx].isFragmented = header->isFragmented;
			memcpy(channels->ro.receivedPackets[ackIdx].data, *data, (uint16_t)(end - *data));
			break;
		}
		case DeliveryMethod_Sequenced: break;
		case DeliveryMethod_ReliableSequenced: {
			if(header->isFragmented) {
				uprintf("MALFORMED PACKET\n");
				break;
			}
			if(channeled.sequence < NET_MAX_SEQUENCE && RelativeSequenceNumber(channeled.sequence, channels->rs.ack.sequence) > 0) {
				channels->rs.ack.sequence = channeled.sequence;
				handler(userptr, data, end, DeliveryMethod_ReliableSequenced);
			}
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->version, NetPacketHeader, {
				.property = PacketProperty_Ack,
				.ack = channels->rs.ack,
			});
			net_queue_merged(net, session, resp, (uint16_t)(resp_end - resp));
			return;
		}
		default:;
	}
	*data = end;
	return;
}

static uint64_t get_time() {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return (uint64_t)now.tv_sec * 10000000LLU + (uint64_t)now.tv_nsec / 100LLU;
}

void handle_Ping(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, struct Ping ping) {
	uint64_t time = get_time();
	if(RelativeSequenceNumber(ping.sequence, pingpong->pong.sequence) > 0) {
		pingpong->pong.sequence = ping.sequence;
		pingpong->pong.time = time;
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), session->version, NetPacketHeader, {
			.property = PacketProperty_Pong,
			.pong = pingpong->pong,
		});
		net_send_internal(net, session, resp, (uint32_t)(resp_end - resp), true);
	}
	if((time - pingpong->lastPing > 5000000LLU && !pingpong->waiting) || time - pingpong->lastPing > 30000000LLU) {
		pingpong->lastPing = time;
		pingpong->waiting = true;
		++pingpong->ping.sequence;
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), session->version, NetPacketHeader, {
			.property = PacketProperty_Ping,
			.ping = pingpong->ping,
		});
		net_send_internal(net, session, resp, (uint32_t)(resp_end - resp), true);
	}
}

float handle_Pong(struct NetContext*, struct NetSession*, struct PingPong *pingpong, struct Pong pong) {
	if(pong.sequence != pingpong->ping.sequence)
		return -1;
	pingpong->waiting = false;
	return (float)((double)(get_time() - pingpong->lastPing) / 10000000.);
}

void handle_MtuCheck(struct NetContext *net, struct NetSession *session, const struct MtuCheck *req) {
	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->version, NetPacketHeader, {
		.property = PacketProperty_MtuOk,
		.mtuOk.base = req->base,
	});
	net_send_internal(net, session, resp, (uint32_t)(resp_end - resp), true);
}

static void InstanceResendPacket_trySend(struct InstanceResendPacket *packet, struct NetContext *net, struct NetSession *session, uint32_t currentTime) {
	if(packet->pkt.len == 0 || currentTime - packet->timeStamp < NET_RESEND_DELAY)
		return;
	net_queue_merged(net, session, packet->pkt.data, packet->pkt.len);
	packet->timeStamp = currentTime - (currentTime - packet->timeStamp) % NET_RESEND_DELAY;
}

static void Ack_flush(struct Ack *ack, struct NetContext *net, struct NetSession *session) {
	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->version, NetPacketHeader, {
		.property = PacketProperty_Ack,
		.ack = *ack,
	});
	net_queue_merged(net, session, resp, (uint16_t)(resp_end - resp));
}

uint32_t instance_channels_tick(struct Channels *channels, struct NetContext *net, struct NetSession *session, uint32_t currentTime) {
	if(!session->version.windowSize) {
		uint8_t resp[65536];
		uint16_t length = (uint16_t)pkt_write_c((uint8_t*[]){resp}, endof(resp), session->version, NetPacketHeader, {
			.property = PacketProperty_Channeled,
			.channeled.channelId = DeliveryMethod_ReliableOrdered,
		});
		net_queue_merged(net, session, resp, length);
		return 15;
	}
	for(; channels->ru.base.sendAck; channels->ru.base.sendAck = false)
		Ack_flush(&channels->ru.base.ack, net, session);
	for(; channels->ro.base.sendAck; channels->ro.base.sendAck = false)
		Ack_flush(&channels->ro.base.ack, net, session);
	for(uint32_t i = 0; i < lengthof(channels->ru.base.resend); ++i)
		InstanceResendPacket_trySend(&channels->ru.base.resend[i], net, session, currentTime);
	for(uint32_t i = 0; i < lengthof(channels->ro.base.resend); ++i)
		InstanceResendPacket_trySend(&channels->ro.base.resend[i], net, session, currentTime);
	InstanceResendPacket_trySend(&channels->rs.resend, net, session, currentTime);
	return 15; // TODO: proper resend timing
}
