#include "../enum_reflection.h"
#include "../packets.h"
#include "../debug.h"
#include "common.h"
#include <stdlib.h>
#include <time.h>

void instance_channels_init(struct Channels *channels) {
	memset(&channels->ru, 0, sizeof(channels->ru));
	memset(&channels->ro, 0, sizeof(channels->ro));
	memset(&channels->rs, 0, sizeof(channels->rs));
	channels->ru.base.ack.channelId = DeliveryMethod_ReliableUnordered;
	channels->ro.base.ack.channelId = DeliveryMethod_ReliableOrdered;
	channels->rs.ack.channelId = DeliveryMethod_ReliableSequenced;
}

void instance_send_channeled(struct Channels *channels, uint8_t *buf, uint32_t len, DeliveryMethod method) {
	if(method != DeliveryMethod_ReliableOrdered) {
		fprintf(stderr, "instance_send_channeled(DeliveryMethod_%s) not implemented\n", reflect(DeliveryMethod, method));
		abort();
	}
	struct ReliableChannel *channel = &channels->ro.base;
	if(RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart) >= NET_WINDOW_SIZE) {
		fprintf(stderr, "Resend overflow buffer not implemented\n");
		abort();
	}
	channel->resend[channel->localSeqence % NET_WINDOW_SIZE].timeStamp = net_time() - NET_RESEND_DELAY;
	uint8_t *pkt = channel->resend[channel->localSeqence % NET_WINDOW_SIZE].data, *pkt_end = pkt;
	pkt_writeNetPacketHeader(&pkt_end, (struct NetPacketHeader){PacketProperty_Channeled, 0, 0});
	pkt_writeChanneled(&pkt_end, (struct Channeled){
		.sequence = channel->localSeqence,
		.channelId = method,
	});
	if(&pkt_end[len] > &pkt[lengthof(channel->resend->data)]) {
		fprintf(stderr, "Fragmenting not implemented\n");
		abort();
	}
	pkt_writeUint8Array(&pkt_end, buf, len);
	channel->resend[channel->localSeqence % NET_WINDOW_SIZE].len = pkt_end - pkt;
	channel->localSeqence = (channel->localSeqence + 1) % NET_MAX_SEQUENCE;
}

void handle_Ack(struct Channels *channels, const uint8_t **data) {
	struct Ack ack = pkt_readAck(data);
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
		if((ack.data[pendingIdx / 8] & (1 << (pendingIdx % 8))) == 0) //Skip false ack
			continue;
		if(pendingSeq == channel->localWindowStart) //Move window
			channel->localWindowStart = (channel->localWindowStart + 1) % NET_MAX_SEQUENCE;
		channel->resend[pendingIdx].len = 0;
	}
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
				fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
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
			if(channeled.sequence >= NET_MAX_SEQUENCE)
				return;
			int32_t relate = RelativeSequenceNumber(channeled.sequence, channel->remoteWindowStart);
			if(RelativeSequenceNumber(channeled.sequence, channel->remoteSequence) > NET_WINDOW_SIZE)
				return;
			if(relate < 0 || relate >= NET_WINDOW_SIZE * 2)
				return;
			if(relate >= NET_WINDOW_SIZE) {
				uint16_t newWindowStart = (channel->remoteWindowStart + relate - NET_WINDOW_SIZE + 1) % NET_MAX_SEQUENCE;
				channel->ack.sequence = newWindowStart;
				while(channel->remoteWindowStart != newWindowStart) {
					uint16_t ackIdx = channel->remoteWindowStart % NET_WINDOW_SIZE;
					channel->ack.data[ackIdx / 8] &= ~(1 << (ackIdx % 8));
					channel->remoteWindowStart = (channel->remoteWindowStart + 1) % NET_MAX_SEQUENCE;
				}
			}
			uint16_t ackIdx = channeled.sequence % NET_WINDOW_SIZE;
			if(channel->ack.data[ackIdx / 8] & (1 << (ackIdx % 8)))
				return;
			channel->ack.data[ackIdx / 8] |= 1 << (ackIdx % 8);
			if(channeled.sequence == channel->remoteSequence) {
				process_Reliable(handler, session, channels, p_ctx, p_room, p_session, data, end, channeled.channelId, isFragmented);
				channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
				if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
					while(channels->ro.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len) {
						struct InstancePacket *ipkt = &channels->ro.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE];
						const uint8_t *pkt = ipkt->data, *pkt_it = pkt;
						const uint8_t *pkt_end = &pkt[ipkt->len];
						ipkt->len = 0;
						process_Reliable(handler, session, channels, p_ctx, p_room, p_session, &pkt, pkt_end, DeliveryMethod_ReliableOrdered, ipkt->isFragmented);
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
						if(pkt_it != pkt_end)
							fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
					}
				} else {
					while(channels->ru.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE]) {
						channels->ru.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE] = 0;
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
					}
				}
			} else if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
				channels->ro.receivedPackets[ackIdx].len = end - *data;
				channels->ro.receivedPackets[ackIdx].isFragmented = isFragmented;
				memcpy(channels->ro.receivedPackets[ackIdx].data, *data, end - *data);
				*data = end;
			} else {
				channels->ru.earlyReceived[ackIdx] = 1;
				process_Reliable(handler, session, channels, p_ctx, p_room, p_session, data, end, DeliveryMethod_ReliableUnordered, isFragmented);
			}
		}
		case DeliveryMethod_Sequenced: return;
		case DeliveryMethod_ReliableSequenced: {
			if(isFragmented) {
				fprintf(stderr, "MALFORMED PACKET\n");
				return;
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
		}
		default:;
	}
	return;
}

void handle_Ping(struct NetContext *net, struct NetSession *session, struct Pong *pong, const uint8_t **data) {
	struct Ping ping = pkt_readPing(data);
	if(RelativeSequenceNumber(ping.sequence, pong->sequence) > 0) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		pong->sequence = ping.sequence;
		pong->time = (uint64_t)now.tv_sec * 10000000LLU + (uint64_t)now.tv_nsec / 100LLU;
		uint8_t resp[65536], *resp_end = resp;
		pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Pong, 0, 0});
		pkt_writePong(&resp_end, *pong);
		net_send_internal(net, session, resp, resp_end - resp, 1);
	}
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
			memset(ack->data, 0, sizeof(ack->data));
			return;
		}
	}
}
