#include "../enum_reflection.h"
#include "../packets.h"
#include "../debug.h"
#include "common.h"
#include <stdlib.h>
#include <time.h>

void instance_pingpong_init(struct PingPong *pingpong) {
	pingpong->lastPing = 0;
	pingpong->waiting = 0;
	pingpong->ping.sequence = 0;
	pingpong->pong.sequence = 0;
}

void instance_channels_init(struct Channels *channels) {
	memset(&channels->ru, 0, sizeof(channels->ru));
	memset(&channels->ro, 0, sizeof(channels->ro));
	memset(&channels->rs, 0, sizeof(channels->rs));
	channels->ru.base.ack.channelId = DeliveryMethod_ReliableUnordered;
	channels->ro.base.ack.channelId = DeliveryMethod_ReliableOrdered;
	channels->rs.ack.channelId = DeliveryMethod_ReliableSequenced;
	channels->ru.base.backlog = NULL;
	channels->ru.base.backlogEnd = &channels->ru.base.backlog;
	channels->ro.base.backlog = NULL;
	channels->ro.base.backlogEnd = &channels->ro.base.backlog;
}

void instance_channels_free(struct Channels *channels) {
	while(channels->ru.base.backlog) {
		struct InstancePacketList *e = channels->ru.base.backlog;
		channels->ru.base.backlog = channels->ru.base.backlog->next;
		free(e);
	}
	while(channels->ro.base.backlog) {
		struct InstancePacketList *e = channels->ro.base.backlog;
		channels->ro.base.backlog = channels->ro.base.backlog->next;
		free(e);
	}
}

static void resend_add(struct ReliableChannel *channel, const uint8_t *buf, uint32_t len, DeliveryMethod method) {
	struct InstanceResendPacket *resend = &channel->resend[channel->localSeqence % NET_WINDOW_SIZE];
	resend->timeStamp = net_time() - NET_RESEND_DELAY;
	uint8_t *data_end = resend->data;
	pkt_writeNetPacketHeader(&data_end, (struct NetPacketHeader){PacketProperty_Channeled, 0, 0});
	pkt_writeChanneled(&data_end, (struct Channeled){
		.sequence = channel->localSeqence,
		.channelId = method,
	});
	if(&data_end[len] > &resend->data[lengthof(channel->resend->data)]) {
		fprintf(stderr, "Fragmenting not implemented\n");
		abort();
	}
	pkt_writeUint8Array(&data_end, buf, len);
	resend->len = data_end - resend->data;
	channel->localSeqence = (channel->localSeqence + 1) % NET_MAX_SEQUENCE;
}

void instance_send_channeled(struct Channels *channels, const uint8_t *buf, uint32_t len, DeliveryMethod method) {
	if(method != DeliveryMethod_ReliableOrdered) {
		fprintf(stderr, "instance_send_channeled(DeliveryMethod_%s) not implemented\n", reflect(DeliveryMethod, method));
		abort();
	}
	// fprintf(stderr, "[%p] instance_send_channeled\n", (void*)channels);
	struct ReliableChannel *channel = &channels->ro.base;
	if(RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart) >= NET_WINDOW_SIZE) {
		*channels->ro.base.backlogEnd = malloc(sizeof(struct InstancePacketList));
		if(!*channels->ro.base.backlogEnd) {
			fprintf(stderr, "alloc error\n");
			abort();
		}
		(*channels->ro.base.backlogEnd)->next = NULL;
		memcpy((*channels->ro.base.backlogEnd)->pkt.data, buf, len);
		(*channels->ro.base.backlogEnd)->pkt.len = len;
		channels->ro.base.backlogEnd = &(*channels->ro.base.backlogEnd)->next;
	} else {
		resend_add(channel, buf, len, method);
		// fprintf(stderr, " send[%i]: %hu\n", RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart), channel->localSeqence);
	}
	/*{
		const uint8_t *read = buf;
		pkt_readRoutingHeader(&read);
		struct SerializeHeader serial = pkt_readSerializeHeader(&read);
		fprintf(stderr, "\tserial.type=%s\n", reflect(InternalMessageType, serial.type));
		if(serial.type == InternalMessageType_MultiplayerSession) {
			struct MultiplayerSessionMessageHeader smsg = pkt_readMultiplayerSessionMessageHeader(&read);
			fprintf(stderr, "\tsmsg.type=%s\n", reflect(MultiplayerSessionMessageType, smsg.type));
			if(smsg.type == MultiplayerSessionMessageType_MenuRpc) {
				struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(&read);
				fprintf(stderr, "\tmenurpc.type=%s\n", reflect(MenuRpcType, rpc.type));
			} else if(smsg.type == MultiplayerSessionMessageType_GameplayRpc) {
				struct GameplayRpcHeader rpc = pkt_readGameplayRpcHeader(&read);
				fprintf(stderr, "\tgameplayrpc.type=%s\n", reflect(GameplayRpcType, rpc.type));
			}
		}
	}*/
}

#define bitsize(e) (sizeof(e) * 8)

void handle_Ack(struct Channels *channels, const uint8_t **data) {
	struct Ack ack = pkt_readAck(data);
	/*if(rand() > RAND_MAX / 500) {
		// fprintf(stderr, "nope.\n");
		return;
	}*/
	/*if(packet.Size != _outgoingAcks.Size) // [PA]Invalid acks packet size
		return;*/
	if(ack.channelId == DeliveryMethod_ReliableSequenced) {
		if(ack.sequence == channels->rs.localSeqence)
			channels->rs.resend.len = 0;
		return;
	}
	struct ReliableChannel *channel = (ack.channelId == DeliveryMethod_ReliableUnordered) ? &channels->ru.base : &channels->ro.base;
	int32_t windowRel = RelativeSequenceNumber(channel->localWindowStart, ack.sequence);
	if(ack.sequence >= NET_MAX_SEQUENCE || windowRel < 0 || windowRel >= NET_WINDOW_SIZE) // [PA]Bad window start
		return;
	for(uint16_t pendingSeq = channel->localWindowStart; pendingSeq != channel->localSeqence; pendingSeq = (pendingSeq + 1) % NET_MAX_SEQUENCE) {
		if(RelativeSequenceNumber(pendingSeq, ack.sequence) >= NET_WINDOW_SIZE)
			break;
		uint16_t pendingIdx = pendingSeq % NET_WINDOW_SIZE;
		if((ack.data[pendingIdx / bitsize(*ack.data)] >> (pendingIdx % bitsize(*ack.data))) & 1) {
			// fprintf(stderr, "  ack[%i]: %hu\n", RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart), pendingSeq);
			if(pendingSeq == channel->localWindowStart) { //Move window
				channel->localWindowStart = (channel->localWindowStart + 1) % NET_MAX_SEQUENCE;
				// fprintf(stderr, "clear[%i]: %hu\n", RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart), pendingSeq);
			}
			channel->resend[pendingIdx].len = 0;
		}
	}
	while(channel->backlog) {
		if(RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart) >= NET_WINDOW_SIZE)
			return;
		// fprintf(stderr, " late[%i]: %hu\n", RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart), channel->localSeqence);
		resend_add(channel, channel->backlog->pkt.data, channel->backlog->pkt.len, ack.channelId);
		struct InstancePacketList *e = channel->backlog;
		channel->backlog = channel->backlog->next;
		free(e);
	}
	channel->backlogEnd = &channel->backlog;
}

static void process_Reliable(ChanneledHandler handler, struct NetSession *session, struct Channels *channels, void *p_ctx, void *p_room, void *p_session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId, _Bool isFragmented) {
	if(!isFragmented) {
		handler(p_ctx, p_room, p_session, data, end, channelId);
		return;
	}
	struct FragmentedHeader header = pkt_readFragmentedHeader(data);
	struct IncomingFragments **incoming = &channels->incomingFragmentsList;
	do {
		if(!*incoming) {
			*incoming = malloc(sizeof(**incoming) + header.fragmentsTotal * sizeof(*(*incoming)->fragments));
			if(!*incoming) {
				fprintf(stderr, "[INSTANCE] alloc error\n");
				abort();
			}
			(*incoming)->fragmentId = header.fragmentId;
			(*incoming)->channelId = channelId;
			(*incoming)->count = 0;
			(*incoming)->total = header.fragmentsTotal;
			(*incoming)->size = 0;
			for(uint32_t i = 0; i < header.fragmentsTotal; ++i)
				(*incoming)->fragments[i].isFragmented = 0;
		} else if((*incoming)->fragmentId != header.fragmentId) {
			incoming = &(*incoming)->next;
			continue;
		}
		if(header.fragmentPart >= (*incoming)->total || (*incoming)->fragments[header.fragmentPart].isFragmented || channelId != (*incoming)->channelId)
			return;
		(*incoming)->size += end - *data;
		if(++(*incoming)->count < (*incoming)->total) {
			(*incoming)->fragments[header.fragmentPart].len = end - *data;
			(*incoming)->fragments[header.fragmentPart].isFragmented = 1;
			pkt_readUint8Array(data, (*incoming)->fragments[header.fragmentPart].data, end - *data);
			return;
		} else {
			uint8_t pkt[(*incoming)->size], *pkt_end = pkt;
			const uint8_t *pkt_it = pkt;
			for(uint32_t i = 0; i < (*incoming)->total; ++i) {
				if(i == header.fragmentPart)
					pkt_writeUint8Array(&pkt_end, *data, end - *data), *data = end;
				else
					pkt_writeUint8Array(&pkt_end, (*incoming)->fragments[i].data, (*incoming)->fragments[i].len);
			}
			#ifdef PACKET_LOGGING_FUNCS
			{
				char buf[1024*16];
				fprintf(stderr, "fragmented\n");
				debug_logRouting(pkt, pkt, pkt_end, buf, session->protocolVersion);
			}
			#endif
			handler(p_ctx, p_room, p_session, &pkt_it, pkt_end, channelId);
			if(pkt_it != pkt_end)
				fprintf(stderr, "[INSTANCE] BAD FRAGMENTED PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
			struct IncomingFragments *e = *incoming;
			*incoming = (*incoming)->next;
			free(e);
		}
	} while(0);
}

void handle_Channeled(ChanneledHandler handler, struct NetContext *net, struct NetSession *session, struct Channels *channels, void *p_ctx, void *p_room, void *p_session, const uint8_t **data, const uint8_t *end, _Bool isFragmented) {
	struct Channeled channeled = pkt_readChanneled(data);
	if(channeled.sequence >= NET_MAX_SEQUENCE)
		return;
	struct ReliableChannel *channel = &channels->ro.base;
	switch(channeled.channelId) {
		case DeliveryMethod_ReliableUnordered: channel = &channels->ru.base;
		case DeliveryMethod_ReliableOrdered: {
			int32_t relate = RelativeSequenceNumber(channeled.sequence, channel->remoteWindowStart);
			if(RelativeSequenceNumber(channeled.sequence, channel->remoteSequence) > NET_WINDOW_SIZE)
				break;
			if(relate < 0 || relate >= NET_WINDOW_SIZE * 2)
				break;
			if(relate >= NET_WINDOW_SIZE) {
				uint16_t newWindowStart = (channel->remoteWindowStart + relate - NET_WINDOW_SIZE + 1) % NET_MAX_SEQUENCE;
				channel->ack.sequence = newWindowStart;
				while(channel->remoteWindowStart != newWindowStart) {
					uint16_t ackIdx = channel->remoteWindowStart % NET_WINDOW_SIZE;
					channel->ack.data[ackIdx / bitsize(*channel->ack.data)] &= ~(1 << (ackIdx % bitsize(*channel->ack.data)));
					channel->remoteWindowStart = (channel->remoteWindowStart + 1) % NET_MAX_SEQUENCE;
				}
			}
			uint16_t ackIdx = channeled.sequence % NET_WINDOW_SIZE;
			if(channel->ack.data[ackIdx / bitsize(*channel->ack.data)] & (1 << (ackIdx % bitsize(*channel->ack.data))))
				break;
			channel->ack.data[ackIdx / bitsize(*channel->ack.data)] |= 1 << (ackIdx % bitsize(*channel->ack.data));
			if(channeled.sequence == channel->remoteSequence) {
				process_Reliable(handler, session, channels, p_ctx, p_room, p_session, data, end, channeled.channelId, isFragmented);
				channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
				if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
					while(channels->ro.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len) {
						struct InstancePacket *ipkt = &channels->ro.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE];
						const uint8_t *const pkt = ipkt->data, *pkt_it = ipkt->data;
						const uint8_t *const pkt_end = &pkt[ipkt->len];
						ipkt->len = 0;
						process_Reliable(handler, session, channels, p_ctx, p_room, p_session, &pkt_it, pkt_end, DeliveryMethod_ReliableOrdered, ipkt->isFragmented);
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
						if(pkt_it != pkt_end)
							fprintf(stderr, "[INSTANCE] BAD RELIABLE PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
					}
				} else {
					while(channels->ru.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE]) {
						channels->ru.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE] = 0;
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
					}
				}
				return;
			} else if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
				channels->ro.receivedPackets[ackIdx].len = end - *data;
				channels->ro.receivedPackets[ackIdx].isFragmented = isFragmented;
				memcpy(channels->ro.receivedPackets[ackIdx].data, *data, end - *data);
				*data = end;
			} else {
				channels->ru.earlyReceived[ackIdx] = 1;
				process_Reliable(handler, session, channels, p_ctx, p_room, p_session, data, end, DeliveryMethod_ReliableUnordered, isFragmented);
				return;
			}
		}
		case DeliveryMethod_Sequenced: break;
		case DeliveryMethod_ReliableSequenced: {
			if(isFragmented) {
				fprintf(stderr, "MALFORMED PACKET\n");
				break;
			}
			int32_t relative = RelativeSequenceNumber(channeled.sequence, channels->rs.ack.sequence);
			if(channeled.sequence < NET_MAX_SEQUENCE && relative > 0) {
				channels->rs.ack.sequence = channeled.sequence;
				handler(p_ctx, p_room, p_session, data, end, DeliveryMethod_ReliableSequenced);
			}
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ack, 0, 0});
			pkt_writeAck(&resp_end, channels->rs.ack);
			net_queue_merged(net, session, resp, resp_end - resp);
			return;
		}
		default:;
	}
	*data = end;
	return;
}

uint64_t get_time() {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)now.tv_sec * 10000000LLU + (uint64_t)now.tv_nsec / 100LLU;
}

void handle_Ping(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, const uint8_t **data) {
	struct Ping ping = pkt_readPing(data);
	uint64_t time = get_time();
	if(RelativeSequenceNumber(ping.sequence, pingpong->pong.sequence) > 0) {
		pingpong->pong.sequence = ping.sequence;
		pingpong->pong.time = time;
		uint8_t resp[65536], *resp_end = resp;
		pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Pong, 0, 0});
		pkt_writePong(&resp_end, pingpong->pong);
		net_send_internal(net, session, resp, resp_end - resp, 1);
	}
	if((time - pingpong->lastPing > 5000000LLU && !pingpong->waiting) || time - pingpong->lastPing > 30000000LLU) {
		pingpong->lastPing = time;
		pingpong->waiting = 1;
		++pingpong->ping.sequence;
		uint8_t resp[65536], *resp_end = resp;
		pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ping, 0, 0});
		pkt_writePing(&resp_end, pingpong->ping);
		net_send_internal(net, session, resp, resp_end - resp, 1);
	}
}

float handle_Pong(struct NetContext *net, struct NetSession *session, struct PingPong *pingpong, const uint8_t **data) {
	struct Pong pong = pkt_readPong(data);
	if(pong.sequence != pingpong->ping.sequence)
		return 0;
	pingpong->waiting = 0;
	return (get_time() - pingpong->lastPing) / 10000000.f;
}

void handle_MtuCheck(struct NetContext *net, struct NetSession *session, const uint8_t **data) {
	struct MtuCheck req = pkt_readMtuCheck(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_MtuOk, 0, 0});
	pkt_writeMtuOk(&resp_end, (struct MtuOk){
		.newMtu0 = req.newMtu0,
		.newMtu1 = req.newMtu1,
	});
	net_send_internal(net, session, resp, resp_end - resp, 1);
}

void try_resend(struct NetContext *net, struct NetSession *session, struct InstanceResendPacket *p, uint32_t currentTime) {
	if(p->len == 0 || currentTime - p->timeStamp < NET_RESEND_DELAY)
		return;
	net_queue_merged(net, session, p->data, p->len);
	while(currentTime - p->timeStamp >= NET_RESEND_DELAY)
		p->timeStamp += NET_RESEND_DELAY;
}

void flush_ack(struct NetContext *net, struct NetSession *session, struct Ack *ack) {
	for(uint_fast8_t i = 0; i < lengthof(ack->data); ++i) {
		if(ack->data[i]) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ack, 0, 0});
			pkt_writeAck(&resp_end, *ack);
			net_queue_merged(net, session, resp, resp_end - resp);
			// memset(ack->data, 0, sizeof(ack->data));
			return;
		}
	}
}
