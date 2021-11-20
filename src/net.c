#include "enum_reflection.h"
#include "net.h"
#include "status.h"
#include "status_ssl.h"
#include "serial.h"
#include <netdb.h>
#include <fcntl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

static const char *ipStr(struct SS *a) {
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
					printf("Bound %s\n", ipStr((struct SS[]){{
						.len = rp->ai_addrlen,
						.sa = *rp->ai_addr,
					}}));
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

static int32_t sockfd;
static int32_t rfd;
_Bool net_init(mbedtls_x509_crt srvcert, mbedtls_pk_context pkey) {
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
	sockfd = findandconn(res, AF_INET6);
	freeaddrinfo(res);
	/*int32_t prop = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&prop, sizeof(int32_t))) {fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n"); close(sockfd); return -1;}*/
	rfd = open("/dev/urandom", O_RDONLY);
	if(rfd == -1) {
		printf("Failed to open /dev/urandom\n");
		return 1;
	}
	return status_init() /*|| sslstatus_init(srvcert, pkey)*/;
}
void net_cleanup() {
	// sslstatus_cleanup();
	status_cleanup();
	if(rfd >= 0)
		close(rfd);
	if(sockfd >= 0)
		close(sockfd);
}
struct SessionList {
	struct SessionList *next;
	struct MasterServerSession data;
} static *sessionList = NULL;
static uint8_t pkt[8192];
uint32_t net_recv(struct MasterServerSession **session, PacketProperty *property, uint8_t **buf) {
	struct SS addr;
	ssize_t size = recvfrom(sockfd, pkt, sizeof(pkt), 0, &addr.sa, &addr.len);
	if(addr.sa.sa_family == AF_UNSPEC) {
		fprintf(stderr, "UNSPEC\n");
		return net_recv(session, property, buf);
	}
	if(size == 0)
		return 0;
	if(size < 0)
		return net_recv(session, property, buf);
	if(pkt[0] > 1) {
		fprintf(stderr, "testval: %hhu\n", pkt[0]);
		return net_recv(session, property, buf);
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
		net_cookie(sptr->data.cookie);
		net_cookie(sptr->data.serverRandom);

		mbedtls_ctr_drbg_context ctr_drbg;
		mbedtls_ctr_drbg_init(&ctr_drbg);
		mbedtls_entropy_context entropy;
		mbedtls_entropy_init(&entropy);
		if(mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14) != 0) {
			fprintf(stderr, "mbedtls_ctr_drbg_seed() failed\n");
			return 0;
		}
		mbedtls_entropy_free(&entropy);

		mbedtls_ecp_keypair_init(&sptr->data.key);
		if(mbedtls_ecp_group_load(&sptr->data.key.grp, MBEDTLS_ECP_DP_SECP384R1) != 0) {
			fprintf(stderr, "mbedtls_ecp_group_load() failed\n");
			return 0;
		}
		if(mbedtls_ecp_gen_keypair(&sptr->data.key.grp, &sptr->data.key.d, &sptr->data.key.Q, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
			fprintf(stderr, "mbedtls_ecp_gen_keypair() failed\n");
			return 0;
		}
		mbedtls_ctr_drbg_free(&ctr_drbg);

		*session = &sptr->data;
		fprintf(stderr, "NEW SESSION: %s\n", ipStr(&addr));
	}
	fprintf(stderr, "recv: %zi\n", size);
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
static void _send(struct MasterServerSession *session, PacketProperty property, void *buf, uint32_t len) {
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
	sendto(sockfd, pkt, data - pkt, 0, &session->addr.sa, session->addr.len);
	fprintf(stderr, "send: %zu\n", data - pkt);
}
void net_send(struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len) {
	if(len <= 384)
		return _send(session, property, buf, len);
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
		_send(session, property, mpbuf, resp - mpbuf);
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
void net_cookie(uint8_t *out) {
	read(rfd, out, 32);
}
uint32_t net_getNextRequestId(struct MasterServerSession *session) {
	++session->lastSentRequestId;
	return (session->lastSentRequestId & 63) | session->epoch;
}
