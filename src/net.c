#include "enum_reflection.h"
#include "net.h"
#include "serial.h"

#ifdef WINDOWS
#define SHUT_RD SD_RECEIVE
#else
#include <netdb.h>
#include <fcntl.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

const char *net_tostr(struct SS *a) {
	static char out[INET6_ADDRSTRLEN + 8];
	char ipStr[INET6_ADDRSTRLEN];
	switch(a->ss.ss_family) {
		case AF_UNSPEC:
			sprintf(out, "AF_UNSPEC");
		case AF_INET:
			inet_ntop(AF_INET, &a->in.sin_addr, ipStr, INET_ADDRSTRLEN);
			sprintf(out, "%s:%u", ipStr, htons(a->in.sin_port));
			break;
		case AF_INET6:
			inet_ntop(AF_INET6, &a->in6.sin6_addr, ipStr, INET6_ADDRSTRLEN);
			sprintf(out, "[%s]:%u", ipStr, htons(a->in6.sin6_port));
			break;
		default:
			sprintf(out, "???");
	}
	return out;
}

int findandconn(struct addrinfo *res, int family) {
	int sockfd = -1;
	for(struct addrinfo *rp = res; rp; rp = rp->ai_next) {
		if(rp->ai_family == family) {
			sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if(sockfd >= 0) {
				if(bind(sockfd, rp->ai_addr, rp->ai_addrlen) < 0) {
					close(sockfd);
					sockfd = -1;
					fprintf(stderr, "Error while binding socket\n");
				} else {
					/*printf("Bound %s\n", net_tostr((struct SS[]){{
						.len = rp->ai_addrlen,
						.sa = *rp->ai_addr,
					}}));*/
					break;
				}
			} else
				fprintf(stderr, "%s", strerror(errno));
		}
	}
	return sockfd;
}

uint8_t addrs_are_equal(struct SS *a0, struct SS *a1) {
	if(a0->ss.ss_family == AF_INET && a1->ss.ss_family == AF_INET) {
		return a0->in.sin_addr.s_addr == a1->in.sin_addr.s_addr && a0->in.sin_port == a1->in.sin_port;
	} else if(a0->ss.ss_family == AF_INET6 && a1->ss.ss_family == AF_INET6) {
		for(uint_fast8_t i = 0; i < sizeof(struct in6_addr); ++i)
			if(a0->in6.sin6_addr.s6_addr[i] != a1->in6.sin6_addr.s6_addr[i])
				return 0;
		return a0->in6.sin6_port == a1->in6.sin6_port;
	}
	return 0;
}

int32_t net_init() {
	struct addrinfo hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = 0,
		.ai_addrlen = 0,
		.ai_addr = NULL,
		.ai_canonname = NULL,
		.ai_next = NULL,
	}, *res;
	int32_t gAddRes = getaddrinfo(NULL, "2328", &hints, &res);
	if(gAddRes != 0) {
		fprintf(stderr, "%s\n", gai_strerror(gAddRes));
		return 1;
	}
	if(!res) {
		fprintf(stderr, "Found no host address to use\n");
		return 1;
	}
	int32_t sockfd = findandconn(res, AF_INET6);
	// sockfd = findandconn(res, AF_INET);
	freeaddrinfo(res);
	/*int32_t prop = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&prop, sizeof(int32_t))) {fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n"); close(sockfd); return -1;}*/
	return sockfd;
}

void net_cleanup(int32_t sockfd) {
	if(sockfd != -1) {
		shutdown(sockfd, SHUT_RD);
		close(sockfd);
	}
}

static void net_cookie(mbedtls_ctr_drbg_context *ctr_drbg, uint8_t *out) {
	mbedtls_ctr_drbg_random(ctr_drbg, out, 32);
}

struct SessionList {
	struct SessionList *next;
	struct MasterServerSession data;
} static *sessionList = NULL;
static uint8_t pkt[8192];
uint32_t net_recv(int32_t sockfd, mbedtls_ctr_drbg_context *ctr_drbg, struct MasterServerSession **session, PacketProperty *property, uint8_t **buf) {
	struct SS addr = {sizeof(struct sockaddr_storage)};
	#ifdef WINSOCK_VERSION
	ssize_t size = recvfrom(sockfd, (char*)pkt, sizeof(pkt), 0, &addr.sa, &addr.len);
	#else
	ssize_t size = recvfrom(sockfd, pkt, sizeof(pkt), 0, &addr.sa, &addr.len);
	#endif
	if(size <= 0)
		return 0;
	if(addr.sa.sa_family == AF_UNSPEC) {
		fprintf(stderr, "UNSPEC\n");
		return net_recv(sockfd, ctr_drbg, session, property, buf);
	}
	if(pkt[0] > 1) {
		fprintf(stderr, "testval: %hhu\n", pkt[0]);
		return net_recv(sockfd, ctr_drbg, session, property, buf);
	}
	struct SessionList *it = sessionList;
	for(; it; it = it->next) {
		if(addrs_are_equal(&addr, &it->data.addr)) {
			*session = &it->data;
			break;
		}
	}
	if(!it) {
		struct SessionList *sptr = malloc(sizeof(struct SessionList));
		sptr->next = sessionList;
		sessionList = sptr;
		memset(&sptr->data, 0, sizeof(sptr->data));
		sptr->data.addr = addr;
		sptr->data.lastSentRequestId = 0;
		sptr->data.state = MasterServerSessionState_None;
		sptr->data.lastKeepAlive = time(NULL);
		net_cookie(ctr_drbg, sptr->data.cookie);
		net_cookie(ctr_drbg, sptr->data.serverRandom);

		mbedtls_ecp_keypair_init(&sptr->data.key);
		if(mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP384R1, &sptr->data.key, mbedtls_ctr_drbg_random, ctr_drbg) != 0) {
			fprintf(stderr, "mbedtls_ecp_gen_key() failed\n");
			return 0;
		}

		*session = &sptr->data;
		fprintf(stderr, "NEW SESSION: %s\n", net_tostr(&addr));
	}
	fprintf(stderr, "recvfrom(%s): %zi\n", net_tostr(&addr), size);
	*buf = pkt;
	struct PacketEncryptionLayer layer = pkt_readPacketEncryptionLayer(buf);
	// fprintf(stderr, "\t[to %zu]layer.encrypted=%hhu\n", *buf - pkt, layer.encrypted);
	if(layer.encrypted) {
		fprintf(stderr, "ENCRYPTED PACKET\n");
		exit(-1);
	}
	struct NetPacketHeader packet = pkt_readNetPacketHeader(buf);
	*property = packet.property;
	// fprintf(stderr, "\t[to %zu]packet.property=%s\n", *buf - pkt, reflect(PacketProperty, packet.property));
	// fprintf(stderr, "\t[to %zu]packet.connectionNumber=%u\n", *buf - pkt, packet.connectionNumber);
	// fprintf(stderr, "\t[to %zu]packet.isFragmented=%u\n", *buf - pkt, packet.isFragmented);
	/*if(NetPacketHeaderSize[packet.Property] >= 3) fprintf(stderr, "\tSequence=%u\n", packet.Sequence);
	if(NetPacketHeaderSize[packet.Property] >= 4) fprintf(stderr, "\tChannelId=%u\n", packet.ChannelId);
	if(NetPacketHeaderSize[packet.Property] >= 6) fprintf(stderr, "\tFragmentId=%u\n", packet.FragmentId);
	if(NetPacketHeaderSize[packet.Property] >= 8) fprintf(stderr, "\tFragmentPart=%u\n", packet.FragmentPart);
	if(NetPacketHeaderSize[packet.Property] >= 10) fprintf(stderr, "\tFragmentsTotal=%u\n", packet.FragmentsTotal);*/
	return &pkt[size] - *buf;
}
#if 1
static void _send(int32_t sockfd, struct MasterServerSession *session, PacketProperty property, void *buf, uint32_t len) {
	uint8_t *data = pkt;
	pkt_writePacketEncryptionLayer(&data, (struct PacketEncryptionLayer){
		.encrypted = 0,
	});
	pkt_writeNetPacketHeader(&data, (struct NetPacketHeader){
		.property = property,
		.connectionNumber = 0,
		.isFragmented = 0,
		.sequence = 0,
		.channelId = 0,
		.fragmentId = 0,
		.fragmentPart = 0,
		.fragmentsTotal = 0,
	});
	pkt_writeBytes(&data, buf, len);
	#ifdef WINSOCK_VERSION
	sendto(sockfd, (char*)pkt, data - pkt, 0, &session->addr.sa, session->addr.len);
	#else
	sendto(sockfd, pkt, data - pkt, 0, &session->addr.sa, session->addr.len);
	#endif
	fprintf(stderr, "sendto(%s): %zu\n", net_tostr(&session->addr), data - pkt);
}
void net_send(int32_t sockfd, struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len) {
	if(len <= 384)
		return _send(sockfd, session, property, buf, len);
	uint8_t *data = buf;
	struct MessageHeader message = pkt_readMessageHeader(&data);
	struct SerializeHeader serial = pkt_readSerializeHeader(&data);
	if(message.type == MessageType_UserMessage) {
		fprintf(stderr, "serialize UserMessageType_MultipartMessage (%s)\n", reflect(UserMessageType, serial.type));
		serial.type = UserMessageType_MultipartMessage;
	} else if(message.type == MessageType_DedicatedServerMessage) {
		fprintf(stderr, "serialize DedicatedServerMessageType_MultipartMessage (%s)\n", reflect(DedicatedServerMessageType, serial.type));
		serial.type = DedicatedServerMessageType_MultipartMessage;
	} else if(message.type == MessageType_HandshakeMessage) {
		fprintf(stderr, "serialize HandshakeMessageType_MultipartMessage (%s)\n", reflect(HandshakeMessageType, serial.type));
		serial.type = HandshakeMessageType_MultipartMessage;
	} else {
		return;
	}
	struct BaseMasterServerMultipartMessage mp;
	mp.base.requestId = net_getNextRequestId(session);
	mp.multipartMessageId = pkt_readBaseMasterServerReliableRequest(&data).requestId;
	mp.offset = 0;
	mp.length = 384;
	mp.totalLength = len;
	do {
		if(len - mp.offset < mp.length)
			mp.length = len - mp.offset;
		uint8_t mpbuf[512];
		uint8_t *resp = mpbuf;
		memcpy(mp.data, &buf[mp.offset], mp.length);
		pkt_writeMessageHeader(&resp, message);
		uint8_t *end = resp;
		pkt_writeBaseMasterServerMultipartMessage(&end, mp);
		serial.length = end + 1 - resp;
		pkt_writeSerializeHeader(&resp, serial);
		pkt_writeBaseMasterServerMultipartMessage(&resp, mp);
		_send(sockfd, session, property, mpbuf, resp - mpbuf);
		mp.offset += 384;
	} while(mp.offset < len);
}
#else
void (*OnPeerConnected)(NetPeer peer);
void (*OnPeerDisconnected)(NetPeer peer, DisconnectInfo disconnectInfo);
void (*OnNetworkError)(IPEndPoint endPoint, SocketError socketError);
void (*OnNetworkReceive)(NetPeer peer, NetPacketReader reader, byte channel, DeliveryMethod deliveryMethod);
void (*OnNetworkReceiveUnconnected)(IPEndPoint remoteEndPoint, NetPacketReader reader, UnconnectedMessageType messageType);
void (*OnNetworkLatencyUpdate)(NetPeer peer, int latency);
void (*OnConnectionRequest)(ConnectionRequest request);
void (*OnDeliveryEvent)(NetPeer peer, object userData);
void (*OnNtpResponseEvent)(NtpPacket packet);
void net_send(struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len) {
	// something something LiteNetLib
}
#endif
uint32_t net_getNextRequestId(struct MasterServerSession *session) {
	++session->lastSentRequestId;
	return (session->lastSentRequestId & 63) | session->epoch;
}
