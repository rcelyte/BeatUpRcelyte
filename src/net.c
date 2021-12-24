#include "enum_reflection.h"
#include "net.h"
#include "encryption.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/error.h>

#define RESEND_BUFFER 32

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

struct ResendPacket {
	PacketProperty property;
	uint32_t len;
	uint8_t data[512];
};
struct ResendSparsePtr {
	_Bool shouldResend;
	clock_t lastSend;
	uint32_t requestId;
	uint32_t data;
};
struct MasterServerSession {
	uint8_t clientRandom[32];
	uint8_t serverRandom[32];
	uint8_t cookie[32];
	mbedtls_ecp_keypair serverKey;
	mbedtls_ecp_point clientPublicKey;
	mbedtls_mpi preMasterSecret;
	struct EncryptionState encryptionState;
	uint32_t epoch;
	HandshakeMessageType state;
	uint32_t ClientHelloWithCookieRequest_requestId;
	struct SS addr;
	uint32_t lastSentRequestId;
	// time_t lastKeepAlive;
	uint32_t resend_count;
	struct ResendSparsePtr resend[RESEND_BUFFER];
	struct ResendPacket resend_data[RESEND_BUFFER];
};

uint8_t *MasterServerSession_get_clientRandom(struct MasterServerSession *session) {
	return session->clientRandom;
}
uint8_t *MasterServerSession_get_serverRandom(struct MasterServerSession *session) {
	return session->serverRandom;
}
uint8_t *MasterServerSession_get_cookie(struct MasterServerSession *session) {
	return session->cookie;
}
_Bool MasterServerSession_write_key(struct MasterServerSession *session, uint8_t *out, uint32_t *out_len) {
	size_t keylen = 0;
	int32_t err = mbedtls_ecp_tls_write_point(&session->serverKey.MBEDTLS_PRIVATE(grp), &session->serverKey.MBEDTLS_PRIVATE(Q), MBEDTLS_ECP_PF_UNCOMPRESSED, &keylen, out, *out_len);
	if(err) {
		fprintf(stderr, "mbedtls_ecp_tls_write_point() failed: %s\n", mbedtls_high_level_strerr(err));
		*out_len = 0;
		return 1;
	}
	*out_len = keylen;
	return 0;
}
_Bool MasterServerSession_signature(struct MasterServerSession *session, struct NetContext *ctx, mbedtls_pk_context *key, uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out) {
	out->length = 0;
	if(mbedtls_pk_get_type(key) != MBEDTLS_PK_RSA) {
		fprintf(stderr, "Key should be RSA\n");
		return 1;
	}
	mbedtls_rsa_context *rsa = key->MBEDTLS_PRIVATE(pk_ctx);
	uint8_t hash[32];
	int32_t err = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), in, in_len, hash);
	if(err != 0) {
		fprintf(stderr, "mbedtls_md() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	err = mbedtls_rsa_pkcs1_sign(rsa, mbedtls_ctr_drbg_random, &ctx->ctr_drbg, MBEDTLS_MD_SHA256, 32, hash, out->data);
	if(err != 0) {
		fprintf(stderr, "mbedtls_rsa_pkcs1_sign() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	out->length = rsa->MBEDTLS_PRIVATE(len);
	return 0;
}
_Bool MasterServerSession_set_clientPublicKey(struct MasterServerSession *session, struct NetContext *ctx, struct ByteArrayNetSerializable *in) {
	#if 1
	const uint8_t *buf = in->data;
	int32_t err = mbedtls_ecp_tls_read_point(&session->serverKey.MBEDTLS_PRIVATE(grp), &session->clientPublicKey, &buf, in->length);
	if(err != 0) {
		fprintf(stderr, "mbedtls_ecp_tls_read_point() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	err = mbedtls_ecdh_compute_shared(&session->serverKey.MBEDTLS_PRIVATE(grp), &session->preMasterSecret, &session->clientPublicKey, &session->serverKey.MBEDTLS_PRIVATE(d), mbedtls_ctr_drbg_random, &ctx->ctr_drbg);
	if(err != 0) {
		fprintf(stderr, "mbedtls_ecdh_compute_shared() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	EncryptionState_init(&session->encryptionState, &session->preMasterSecret, session->serverRandom, session->clientRandom, 0);
	return 0;
	#else
	int32_t err = mbedtls_ecdh_read_public(ctx, in->data, in->length);
	if(err != 0) {
		fprintf(stderr, "mbedtls_ecdh_read_public() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	return 0;
	#endif
}
void MasterServerSession_set_epoch(struct MasterServerSession *session, uint32_t epoch) {
	session->epoch = epoch;
}
_Bool MasterServerSession_set_state(struct MasterServerSession *session, HandshakeMessageType state) {
	if(session->state != state) {
		session->state = state;
		return 0;
	}
	return 1;
}
uint32_t *MasterServerSession_ClientHelloWithCookieRequest_requestId(struct MasterServerSession *session) {
	return &session->ClientHelloWithCookieRequest_requestId;
}

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

static uint8_t addrs_are_equal(struct SS *a0, struct SS *a1) {
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

// Temporary; to be replaced with sparse array once instance servers are implemented
struct SessionList {
	struct SessionList *next;
	struct MasterServerSession data;
};

static void net_cookie(mbedtls_ctr_drbg_context *ctr_drbg, uint8_t *out) {
	mbedtls_ctr_drbg_random(ctr_drbg, out, 32);
}

static uint32_t get_requestId(uint8_t *msg) {
	pkt_readMessageHeader(&msg);
	pkt_readSerializeHeader(&msg);
	return pkt_readBaseMasterServerReliableRequest(&msg).requestId;
}

static struct ResendPacket *add_resend(struct MasterServerSession *session, PacketProperty property, uint32_t requestId, _Bool shouldResend) {
	if(session->resend_count < RESEND_BUFFER) {
		struct ResendSparsePtr *p = &session->resend[session->resend_count++];
		p->shouldResend = shouldResend;
		p->lastSend = clock();
		p->requestId = requestId;
		session->resend_data[p->data].property = property;
		return &session->resend_data[p->data];
	}
	fprintf(stderr, "RESEND BUFFER FULL\n");
	return NULL;
}

static void _send(struct NetContext *ctx, struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len, _Bool reliable) {
	if(reliable) {
		struct ResendPacket *p = add_resend(session, property, get_requestId(buf), 1);
		if(p) {
			p->len = len;
			memcpy(p->data, buf, len);
		}
	}
	uint8_t head[512];
	uint8_t *head_end = head;
	pkt_writePacketEncryptionLayer(&head_end, (struct PacketEncryptionLayer){
		.encrypted = 0,
	});
	pkt_writeNetPacketHeader(&head_end, (struct NetPacketHeader){
		.property = property,
		.connectionNumber = 0,
		.isFragmented = 0,
		.sequence = 0,
		.channelId = 0,
		.fragmentId = 0,
		.fragmentPart = 0,
		.fragmentsTotal = 0,
	});
	#ifdef WINSOCK_VERSION
	WSABUF iov[] = {
		{.len = head_end - head, .buf = (char*)head},
		{.len = len, .buf = (char*)buf},
	};
	WSAMSG msg = {
		.name = &session->addr.sa,
		.namelen = session->addr.len,
		.lpBuffers = iov,
		.dwBufferCount = sizeof(iov) / sizeof(*iov),
		.Control = {
			.len = 0,
			.buf = NULL,
		},
		.dwFlags = 0,
	};
	DWORD size = 0;
	WSASendMsg(ctx->sockfd, &msg, 0, &size, NULL, NULL);
	fprintf(stderr, "[NET] sendto[%lu]\n", size);
	#else
	struct iovec iov[] = {
		{.iov_base = head, .iov_len = head_end - head},
		{.iov_base = buf, .iov_len = len},
	};
	struct msghdr msg = {
		.msg_name = &session->addr.sa,
		.msg_namelen = session->addr.len,
		.msg_iov = iov,
		.msg_iovlen = sizeof(iov) / sizeof(*iov),
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};
	ssize_t size = sendmsg(ctx->sockfd, &msg, 0);
	fprintf(stderr, "[NET] sendto[%zd]\n", size);
	#endif
}

_Bool net_init(struct NetContext *ctx, uint16_t port) {
	ctx->sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if(ctx->sockfd >= 0) {
		const struct sockaddr_in6 addr = {
			.sin6_family = AF_INET6,
			.sin6_port = htons(port),
			.sin6_flowinfo = 0,
			.sin6_addr = IN6ADDR_ANY_INIT,
			.sin6_scope_id = 0,
		};
		if(bind(ctx->sockfd, (const struct sockaddr*)&addr, sizeof(addr)) < 0) {
			close(ctx->sockfd);
			ctx->sockfd = -1;
			fprintf(stderr, "Error while binding socket\n");
		}
	}
	/*int32_t prop = 1;
	if(setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&prop, sizeof(int32_t))) {fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n"); close(sockfd); return -1;}*/
	mbedtls_ctr_drbg_init(&ctx->ctr_drbg);
	mbedtls_entropy_init(&ctx->entropy);
	if(mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14) != 0) {
		fprintf(stderr, "mbedtls_ctr_drbg_seed() failed\n");
		return 1;
	}
	ctx->sessionList = NULL;
	ctx->prev_size = 0;
	memset(ctx->buf, 0, sizeof(ctx->buf));
	return ctx->sockfd == -1;
}

void net_stop(struct NetContext *ctx) {
	if(ctx->sockfd == -1)
		return;
	shutdown(ctx->sockfd, SHUT_RD);
	close(ctx->sockfd);
}

void net_cleanup(struct NetContext *ctx) {
	while(ctx->sessionList) {
		struct SessionList *e = ctx->sessionList;
		ctx->sessionList = ctx->sessionList->next;
		free(e);
	}
	mbedtls_entropy_free(&ctx->entropy);
	mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
	ctx->sockfd = -1;
}

uint32_t net_recv(struct NetContext *ctx, struct MasterServerSession **session, PacketProperty *property, uint8_t **buf) {
	retry:; // tail calls are theoretical but stack overflows are real
	int32_t res;
	do {
		clock_t time = clock();
		for(struct SessionList *s = ctx->sessionList; s; s = s->next) {
			for(struct ResendSparsePtr *p = s->data.resend; p < &s->data.resend[s->data.resend_count]; ++p) {
				if(time - p->lastSend >= CLOCKS_PER_SEC / 10) {
					_send(ctx, &s->data, s->data.resend_data[p->data].property, s->data.resend_data[p->data].data, s->data.resend_data[p->data].len, 0);
					p->lastSend += CLOCKS_PER_SEC / 10;
				}
			}
		}
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(ctx->sockfd, &fds);
		struct timeval timeout = {
			.tv_sec = 0,
			.tv_usec = 100000,
		};
		res = select(ctx->sockfd+1, &fds, NULL, NULL, &timeout);
	} while(res == 0);
	if(res == -1)
		return 0;

	struct SS addr = {sizeof(struct sockaddr_storage)};
	#ifdef WINSOCK_VERSION
	ssize_t size = recvfrom(ctx->sockfd, (char*)ctx->buf, sizeof(ctx->buf), 0, &addr.sa, &addr.len);
	#else
	ssize_t size = recvfrom(ctx->sockfd, ctx->buf, sizeof(ctx->buf), 0, &addr.sa, &addr.len);
	#endif
	if(size <= 0)
		return 0;
	if(size < ctx->prev_size) // Since deserialization doesn't have range checks, we need to clean up data from previous messages
		memset(&ctx->buf[size], 0, ctx->prev_size - size);
	ctx->prev_size = size;
	if(addr.sa.sa_family == AF_UNSPEC) {
		fprintf(stderr, "UNSPEC\n");
		goto retry;
	}
	if(ctx->buf[0] > 1) {
		fprintf(stderr, "testval: %hhu\n", ctx->buf[0]);
		goto retry;
	}
	struct SessionList *it = ctx->sessionList;
	for(; it; it = it->next) {
		if(addrs_are_equal(&addr, &it->data.addr)) {
			*session = &it->data;
			break;
		}
	}
	if(!it) {
		struct SessionList *sptr = malloc(sizeof(struct SessionList));
		memset(&sptr->data, 0, sizeof(sptr->data));
		net_cookie(&ctx->ctr_drbg, sptr->data.serverRandom);
		net_cookie(&ctx->ctr_drbg, sptr->data.cookie);
		sptr->data.state = 255;
		sptr->data.ClientHelloWithCookieRequest_requestId = 0;
		sptr->data.addr = addr;
		sptr->data.lastSentRequestId = 0;
		// sptr->data.lastKeepAlive = time(NULL);
		sptr->data.resend_count = 0;
		for(uint32_t i = 0; i < RESEND_BUFFER; ++i)
			sptr->data.resend[i].data = i;

		mbedtls_ecp_keypair_init(&sptr->data.serverKey);
		if(mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP384R1, &sptr->data.serverKey, mbedtls_ctr_drbg_random, &ctx->ctr_drbg) != 0) { // valgrind warning: Source and destination overlap in memcpy()
			fprintf(stderr, "mbedtls_ecp_gen_key() failed\n");
			free(sptr);
			return 0;
		}

		sptr->next = ctx->sessionList;
		ctx->sessionList = sptr;
		*session = &sptr->data;
		fprintf(stderr, "NEW SESSION: %s\n", net_tostr(&addr));
	}
	fprintf(stderr, "[NET] recvfrom[%zi]\n", size);
	*buf = ctx->buf;
	struct PacketEncryptionLayer layer = pkt_readPacketEncryptionLayer(buf);
	// fprintf(stderr, "\t[to %zu]layer.encrypted=%hhu\n", *buf - ctx->buf, layer.encrypted);
	if(layer.encrypted == 1) {
		uint32_t length = &ctx->buf[size] - *buf;
		if(EncryptionState_decrypt(&(*session)->encryptionState, layer, *buf, &length)) {
			fprintf(stderr, "Packet decryption failed\n");
			goto retry;
		}
		size = length + (*buf - ctx->buf);
	} else if(layer.encrypted) {
		fprintf(stderr, "Invalid packet\n");
		goto retry;
	}
	struct NetPacketHeader packet = pkt_readNetPacketHeader(buf);
	*property = packet.property;
	// fprintf(stderr, "\t[to %zu]packet.property=%s\n", *buf - ctx->buf, reflect(PacketProperty, packet.property));
	// fprintf(stderr, "\t[to %zu]packet.connectionNumber=%u\n", *buf - ctx->buf, packet.connectionNumber);
	// fprintf(stderr, "\t[to %zu]packet.isFragmented=%u\n", *buf - ctx->buf, packet.isFragmented);
	/*if(NetPacketHeaderSize[packet.Property] >= 3) fprintf(stderr, "\tSequence=%u\n", packet.Sequence);
	if(NetPacketHeaderSize[packet.Property] >= 4) fprintf(stderr, "\tChannelId=%u\n", packet.ChannelId);
	if(NetPacketHeaderSize[packet.Property] >= 6) fprintf(stderr, "\tFragmentId=%u\n", packet.FragmentId);
	if(NetPacketHeaderSize[packet.Property] >= 8) fprintf(stderr, "\tFragmentPart=%u\n", packet.FragmentPart);
	if(NetPacketHeaderSize[packet.Property] >= 10) fprintf(stderr, "\tFragmentsTotal=%u\n", packet.FragmentsTotal);*/
	return &ctx->buf[size] - *buf;
}
#if 1
_Bool net_handle_ack(struct MasterServerSession *session, struct MessageHeader *message_out, struct SerializeHeader *serial_out, uint32_t requestId) {
	for(uint32_t i = 0; i < session->resend_count; ++i) {
		if(requestId == session->resend[i].requestId) {
			--session->resend_count;
			uint32_t data = session->resend[i].data;
			session->resend[i] = session->resend[session->resend_count];
			session->resend[session->resend_count].data = data;
			uint8_t *msg = session->resend_data[data].data;
			*message_out = pkt_readMessageHeader(&msg);
			*serial_out = pkt_readSerializeHeader(&msg);
			return 1;
		}
	}
	return 0;
}
void net_send(struct NetContext *ctx, struct MasterServerSession *session, PacketProperty property, uint8_t *buf, uint32_t len, _Bool reliable) {
	if(buf >= ctx->buf && buf < &ctx->buf[sizeof(ctx->buf)])
		if(&buf[len] - ctx->buf > ctx->prev_size)
			ctx->prev_size = &buf[len] - ctx->buf;
	if(len <= 414)
		return _send(ctx, session, property, buf, len, reliable);
	uint8_t *data = buf;
	struct MessageHeader message = pkt_readMessageHeader(&data);
	struct SerializeHeader serial = pkt_readSerializeHeader(&data);
	if(message.type == MessageType_UserMessage) {
		fprintf(stderr, "serialize UserMessageType_UserMultipartMessage (%s)\n", reflect(UserMessageType, serial.type));
		serial.type = UserMessageType_UserMultipartMessage;
	} else if(message.type == MessageType_DedicatedServerMessage) {
		fprintf(stderr, "serialize DedicatedServerMessageType_DedicatedServerMultipartMessage (%s)\n", reflect(DedicatedServerMessageType, serial.type));
		serial.type = DedicatedServerMessageType_DedicatedServerMultipartMessage;
	} else if(message.type == MessageType_HandshakeMessage) {
		fprintf(stderr, "serialize HandshakeMessageType_HandshakeMultipartMessage (%s)\n", reflect(HandshakeMessageType, serial.type));
		serial.type = HandshakeMessageType_HandshakeMultipartMessage;
	} else {
		return;
	}
	struct BaseMasterServerMultipartMessage mp;
	mp.multipartMessageId = get_requestId(buf);
	mp.offset = 0;
	mp.length = 384;
	mp.totalLength = len;
	do {
		mp.base.requestId = net_getNextRequestId(session);
		if(len - mp.offset < mp.length)
			mp.length = len - mp.offset;
		uint8_t mpbuf[512];
		uint8_t *mpbuf_end = mpbuf;
		memcpy(mp.data, &buf[mp.offset], mp.length);
		pkt_writeMessageHeader(&mpbuf_end, message);
		uint8_t *msg_end = mpbuf_end;
		pkt_writeBaseMasterServerMultipartMessage(&msg_end, mp);
		serial.length = msg_end + 1 - mpbuf_end;
		pkt_writeSerializeHeader(&mpbuf_end, serial);
		pkt_writeBaseMasterServerMultipartMessage(&mpbuf_end, mp);
		_send(ctx, session, property, mpbuf, mpbuf_end - mpbuf, 1);
		mp.offset += 384;
	} while(mp.offset < len);
	struct ResendPacket *p = add_resend(session, property, get_requestId(buf), 0);
	if(p) {
		p->len = data - buf;
		memcpy(p->data, buf, data - buf);
	}
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
