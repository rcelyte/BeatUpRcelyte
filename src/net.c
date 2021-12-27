#define NET_H_PRIVATE(x) x
#include "enum_reflection.h"
#include "net.h"
#include "encryption.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/error.h>

#define RESEND_BUFFER 48

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
	uint32_t lastSend;
	uint32_t requestId;
	uint32_t data;
};
struct MasterServerSession {
	struct MasterServerSession *next; // Temporary; to be replaced with sparse array once instance servers are implemented
	uint8_t clientRandom[32];
	uint8_t serverRandom[32];
	uint8_t cookie[32];
	mbedtls_mpi serverSecret;
	mbedtls_ecp_point serverPublic;
	struct EncryptionState encryptionState;
	uint32_t epoch;
	HandshakeMessageType state;
	uint32_t ClientHelloWithCookieRequest_requestId;
	struct SS addr;
	uint32_t lastSentRequestId;
	char gameId[22];
	uint32_t lastKeepAlive;
	uint32_t resend_count;
	struct ResendSparsePtr resend[RESEND_BUFFER];
	struct ResendPacket resend_data[RESEND_BUFFER];
};

uint8_t *MasterServerSession_get_clientRandom(struct MasterServerSession *session) {
	return session->clientRandom;
}
const uint8_t *MasterServerSession_get_serverRandom(const struct MasterServerSession *session) {
	return session->serverRandom;
}
const uint8_t *MasterServerSession_get_cookie(const struct MasterServerSession *session) {
	return session->cookie;
}
_Bool MasterServerSession_write_key(const struct MasterServerSession *session, struct NetContext *ctx, uint8_t *out, uint32_t *out_len) {
	size_t keylen = 0;
	int32_t err = mbedtls_ecp_tls_write_point(&ctx->grp, &session->serverPublic, MBEDTLS_ECP_PF_UNCOMPRESSED, &keylen, out, *out_len);
	if(err) {
		fprintf(stderr, "mbedtls_ecp_tls_write_point() failed: %s\n", mbedtls_high_level_strerr(err));
		*out_len = 0;
		return 1;
	}
	*out_len = keylen;
	return 0;
}
_Bool MasterServerSession_signature(const struct MasterServerSession *session, struct NetContext *ctx, const mbedtls_pk_context *key, const uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out) {
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
_Bool MasterServerSession_set_clientPublicKey(struct MasterServerSession *session, struct NetContext *ctx, const struct ByteArrayNetSerializable *in) {
	const uint8_t *buf = in->data;
	mbedtls_ecp_point clientPublicKey;
	mbedtls_ecp_point_init(&clientPublicKey);
	int32_t err = mbedtls_ecp_tls_read_point(&ctx->grp, &clientPublicKey, &buf, in->length);
	if(err != 0) {
		fprintf(stderr, "mbedtls_ecp_tls_read_point() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	mbedtls_mpi preMasterSecret;
	mbedtls_mpi_init(&preMasterSecret);
	err = mbedtls_ecdh_compute_shared(&ctx->grp, &preMasterSecret, &clientPublicKey, &session->serverSecret, mbedtls_ctr_drbg_random, &ctx->ctr_drbg);
	if(err != 0) {
		fprintf(stderr, "mbedtls_ecdh_compute_shared() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	EncryptionState_init(&session->encryptionState, &preMasterSecret, session->serverRandom, session->clientRandom, 0);
	return 0;
}
void MasterServerSession_set_epoch(struct MasterServerSession *session, uint32_t epoch) {
	session->epoch = epoch;
}
_Bool MasterServerSession_change_state(struct MasterServerSession *session, HandshakeMessageType old, HandshakeMessageType new) {
	if(session->state != old)
		return 1;
	session->state = new;
	return 0;
}
void MasterServerSession_set_state(struct MasterServerSession *session, HandshakeMessageType state) {
	session->state = state;
}
char *MasterServerSession_get_gameId(struct MasterServerSession *session) {
	return session->gameId;
}
uint32_t MasterServerSession_get_lastKeepAlive(struct MasterServerSession *session) {
	return session->lastKeepAlive;
}
uint32_t *MasterServerSession_ClientHelloWithCookieRequest_requestId(struct MasterServerSession *session) {
	return &session->ClientHelloWithCookieRequest_requestId;
}
struct SS MasterServerSession_get_addr(struct MasterServerSession *session) {
	return session->addr;
}

int32_t net_get_sockfd(struct NetContext *ctx) {
	return ctx->sockfd;
}

mbedtls_ctr_drbg_context *net_get_ctr_drbg(struct NetContext *ctx) {
	return &ctx->ctr_drbg;
}

static void net_tostr(const struct SS *a, char *out) {
	char ipStr[INET6_ADDRSTRLEN];
	switch(a->ss.ss_family) {
		case AF_UNSPEC: {
			sprintf(out, "AF_UNSPEC");
			break;
		}
		case AF_INET: {
			inet_ntop(AF_INET, &a->in.sin_addr, ipStr, INET_ADDRSTRLEN);
			sprintf(out, "%s:%u", ipStr, htons(a->in.sin_port));
			break;
		}
		case AF_INET6: {
			inet_ntop(AF_INET6, &a->in6.sin6_addr, ipStr, INET6_ADDRSTRLEN);
			sprintf(out, "[%s]:%u", ipStr, htons(a->in6.sin6_port));
			break;
		}
		default:
		sprintf(out, "???");
	}
}

static uint8_t addrs_are_equal(const struct SS *a0, const struct SS *a1) {
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

static void net_cookie(mbedtls_ctr_drbg_context *ctr_drbg, uint8_t *out) {
	mbedtls_ctr_drbg_random(ctr_drbg, out, 32);
}

uint32_t net_time() {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static struct ResendPacket *add_resend(struct MasterServerSession *session, PacketProperty property, uint32_t requestId, _Bool shouldResend) {
	if(session->resend_count < RESEND_BUFFER) {
		struct ResendSparsePtr *p = &session->resend[session->resend_count++];
		p->shouldResend = shouldResend;
		p->lastSend = net_time();
		p->requestId = requestId;
		session->resend_data[p->data].property = property;
		return &session->resend_data[p->data];
	}
	fprintf(stderr, "RESEND BUFFER FULL\n");
	return NULL;
}

static void _send(struct NetContext *ctx, struct MasterServerSession *session, PacketProperty property, const uint8_t *buf, uint32_t len, _Bool reliable) {
	const uint8_t *buf_read = buf;
	struct MessageHeader message = pkt_readMessageHeader(&buf_read);
	if(reliable) {
		pkt_readSerializeHeader(&buf_read);
		struct ResendPacket *p = add_resend(session, property, pkt_readBaseMasterServerReliableRequest(&buf_read).requestId, 1);
		if(p) {
			p->len = len;
			memcpy(p->data, buf, len);
		}
	}
	struct PacketEncryptionLayer layer;
	struct NetPacketHeader packet;
	layer.encrypted = 0;
	packet.property = property;
	packet.connectionNumber = 0;
	packet.isFragmented = 0;
	packet.sequence = 0;
	packet.channelId = 0;
	packet.fragmentId = 0;
	packet.fragmentPart = 0;
	packet.fragmentsTotal = 0;
	
	uint8_t head[512], body[512];
	uint8_t *head_end = head;
	if(session->encryptionState.initialized && message.type != MessageType_HandshakeMessage) {
		uint8_t *pkt_end = head;
		pkt_writeNetPacketHeader(&pkt_end, packet);
		EncryptionState_encrypt(&session->encryptionState, &layer, &ctx->ctr_drbg, (const uint8_t*[]){head, buf, NULL}, (const uint32_t[]){pkt_end - head, len}, body, &len);
		pkt_writePacketEncryptionLayer(&head_end, layer);
	} else {
		pkt_writePacketEncryptionLayer(&head_end, layer);
		pkt_writeNetPacketHeader(&head_end, packet);
		memcpy(body, buf, len); // const correctness ._.
	}
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
	// fprintf(stderr, "[NET] sendto[%lu]\n", size);
	#else
	struct iovec iov[] = {
		{.iov_base = head, .iov_len = head_end - head},
		{.iov_base = body, .iov_len = len},
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
	/*ssize_t size =*/ sendmsg(ctx->sockfd, &msg, 0);
	// fprintf(stderr, "[NET] sendto[%zd]\n", size);
	#endif
}

static struct MasterServerSession *onConnect_default(struct NetContext *ctx, struct SS addr) {return NULL;}

_Bool net_init(struct NetContext *ctx, uint16_t port, struct MasterServerSession *(*onConnect)(struct NetContext *ctx, struct SS addr)) {
	ctx->sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	mbedtls_ctr_drbg_init(&ctx->ctr_drbg);
	mbedtls_entropy_init(&ctx->entropy);
	mbedtls_ecp_group_init(&ctx->grp);
	if(ctx->sockfd == -1) {
		fprintf(stderr, "Socket creation failed\n");
		return -1;
	}
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
		fprintf(stderr, "Socket binding failed\n");
		return 1;
	}
	/*struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;
	if(setsockopt(ctx->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		close(ctx->sockfd);
		ctx->sockfd = -1;
		fprintf(stderr, "setsockopt(SO_RCVTIMEO) failed\n");
		return 1;
	}*/
	/*int32_t prop = 1;
	if(setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&prop, sizeof(int32_t))) {fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n"); close(sockfd); return -1;}*/
	struct SS realAddr = {sizeof(struct sockaddr_storage)};
	getsockname(ctx->sockfd, &realAddr.sa, &realAddr.len);
	char namestr[INET6_ADDRSTRLEN + 8];
	net_tostr(&realAddr, namestr);
	fprintf(stderr, "[NET] Bound %s\n", namestr);
	if(mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14) != 0) {
		fprintf(stderr, "mbedtls_ctr_drbg_seed() failed\n");
		return 1;
	}
	if(mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_SECP384R1)) {
		fprintf(stderr, "mbedtls_ecp_group_load() failed\n");
		return 1;
	}
	ctx->sessionList = NULL;
	ctx->onConnect = onConnect ? onConnect : onConnect_default;
	ctx->dirt = NULL;
	return 0;
}

static void net_free_session(struct MasterServerSession *session) {
	EncryptionState_free(&session->encryptionState);
	mbedtls_mpi_free(&session->serverSecret);
	mbedtls_ecp_point_free(&session->serverPublic);
}

static struct MasterServerSession *net_delete_session(struct MasterServerSession *session) {
	struct MasterServerSession *next = session->next;
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&session->addr, addrstr);
	fprintf(stderr, "[NET] disconnect %s\n", addrstr);
	net_free_session(session);
	free(session);
	return next;
}

struct MasterServerSession *net_resolve_session(struct NetContext *ctx, struct SS addr) {
	struct MasterServerSession *session = ctx->sessionList;
	for(; session; session = session->next)
		if(addrs_are_equal(&addr, &session->addr))
			return session;
	return NULL;
}

struct MasterServerSession *net_create_session(struct NetContext *ctx, struct SS addr) {
	struct MasterServerSession *session = malloc(sizeof(struct MasterServerSession));
	if(!session) {
		fprintf(stderr, "alloc error\n");
		return NULL;
	}
	session->next = ctx->sessionList;
	session->addr = addr;
	session->encryptionState.initialized = 0;
	mbedtls_mpi_init(&session->serverSecret);
	mbedtls_ecp_point_init(&session->serverPublic);
	if(net_reset_session(ctx, session)) {
		free(session);
		return NULL;
	}
	ctx->sessionList = session;

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	fprintf(stderr, "[NET] connect %s\n", addrstr);
	return session;
}

_Bool net_reset_session(struct NetContext *ctx, struct MasterServerSession *session) {
	struct MasterServerSession *next = session->next;
	struct SS addr = session->addr;
	net_free_session(session);
	memset(session, 0, sizeof(*session));
	session->next = next;
	net_cookie(&ctx->ctr_drbg, session->serverRandom);
	net_cookie(&ctx->ctr_drbg, session->cookie);
	session->state = 255;
	session->addr = addr;
	session->lastKeepAlive = net_time();
	for(uint32_t i = 0; i < RESEND_BUFFER; ++i)
		session->resend[i].data = i;

	if(mbedtls_ecp_gen_keypair(&ctx->grp, &session->serverSecret, &session->serverPublic, mbedtls_ctr_drbg_random, &ctx->ctr_drbg)) {
		fprintf(stderr, "mbedtls_ecp_gen_keypair() failed\n");
		return 1;
	}
	return 0;
}

void net_stop(struct NetContext *ctx) {
	if(ctx->sockfd == -1)
		return;
	shutdown(ctx->sockfd, SHUT_RD);
	close(ctx->sockfd);
}

void net_cleanup(struct NetContext *ctx) {
	while(ctx->sessionList) {
		ctx->sessionList = net_delete_session(ctx->sessionList);
	}
	mbedtls_entropy_free(&ctx->entropy);
	mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
	ctx->sockfd = -1;
}

uint32_t net_recv(struct NetContext *ctx, uint8_t *buf, uint32_t buf_len, struct MasterServerSession **session, PacketProperty *property, uint8_t **pkt) {
	retry:; // tail calls are theoretical but stack overflows are real
	uint32_t currentTime = net_time(), longestIdle = 0, hasResends = 0;
	for(struct MasterServerSession **sp = &ctx->sessionList; *sp;) {
		uint32_t idleTime = currentTime - (*sp)->lastKeepAlive;
		if(idleTime > 180000) { // this filters the RFC-1149 user
			*sp = net_delete_session(*sp);
		} else {
			if(idleTime > longestIdle)
				longestIdle = idleTime;
			hasResends |= (*sp)->resend_count;
			for(struct ResendSparsePtr *p = (*sp)->resend; p < &(*sp)->resend[(*sp)->resend_count]; ++p) {
				if(currentTime - p->lastSend >= 100) {
					_send(ctx, *sp, (*sp)->resend_data[p->data].property, (*sp)->resend_data[p->data].data, (*sp)->resend_data[p->data].len, 0);
					p->lastSend += 100;
				}
			}
			sp = &(*sp)->next;
		}
	}
	struct timeval timeout;
	timeout.tv_sec = hasResends ? 0 : (180000 - longestIdle) / 1000; // Don't loop if there's nothing to do
	timeout.tv_usec = 100000;
	if(setsockopt(ctx->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		fprintf(stderr, "setsockopt(SO_RCVTIMEO) failed\n");
		return 0;
	}
	struct SS addr = {sizeof(struct sockaddr_storage)};
	#ifdef WINSOCK_VERSION
	ssize_t size = recvfrom(ctx->sockfd, (char*)buf, buf_len, 0, &addr.sa, &addr.len);
	#else
	ssize_t size = recvfrom(ctx->sockfd, buf, buf_len, 0, &addr.sa, &addr.len);
	#endif
	if(size <= 0) {
		if(errno == EAGAIN || errno == EWOULDBLOCK)
			goto retry;
		return 0;
	}
	if(&buf[buf_len] < ctx->dirt) {
		fprintf(stderr, "BAD BUFFER CONTENTS\n");
		abort();
	}
	if(&buf[size] < ctx->dirt) // Since deserialization doesn't have range checks, we need to clean up data from previous messages
		memset(&buf[size], 0, ctx->dirt - &buf[size]);
	ctx->dirt = &buf[size];
	if(addr.sa.sa_family == AF_UNSPEC) {
		fprintf(stderr, "UNSPEC\n");
		goto retry;
	}
	if(buf[0] > 1) {
		fprintf(stderr, "testval: %hhu\n", buf[0]);
		goto retry;
	}
	*session = net_resolve_session(ctx, addr);
	if(!*session) {
		*session = ctx->onConnect(ctx, addr);
		if(!*session)
			goto retry;
	}
	// fprintf(stderr, "[NET] recvfrom[%zi]\n", size);
	*pkt = buf;
	struct PacketEncryptionLayer layer = pkt_readPacketEncryptionLayer((const uint8_t**)pkt);
	if(layer.encrypted == 1) { // TODO: filter unencrypted?
		uint32_t length = &buf[size] - *pkt;
		if(EncryptionState_decrypt(&(*session)->encryptionState, layer, *pkt, &length)) {
			fprintf(stderr, "Packet decryption failed\n");
			goto retry;
		}
		size = length + (*pkt - buf);
		(*session)->lastKeepAlive = net_time();
	} else if(layer.encrypted) {
		fprintf(stderr, "Invalid packet\n");
		goto retry;
	}
	struct NetPacketHeader packet = pkt_readNetPacketHeader((const uint8_t**)pkt);
	*property = packet.property;
	return &buf[size] - *pkt;
}
#if 1
_Bool net_handle_ack(struct MasterServerSession *session, struct MessageHeader *message_out, struct SerializeHeader *serial_out, uint32_t requestId) {
	for(uint32_t i = 0; i < session->resend_count; ++i) {
		if(requestId == session->resend[i].requestId) {
			--session->resend_count;
			uint32_t data = session->resend[i].data;
			session->resend[i] = session->resend[session->resend_count];
			session->resend[session->resend_count].data = data;
			const uint8_t *msg = session->resend_data[data].data;
			*message_out = pkt_readMessageHeader(&msg);
			*serial_out = pkt_readSerializeHeader(&msg);
			return 1;
		}
	}
	return 0;
}
void net_send(struct NetContext *ctx, struct MasterServerSession *session, PacketProperty property, const uint8_t *buf, uint32_t len, _Bool reliable) {
	if(&buf[len] > ctx->dirt)
		ctx->dirt = &buf[len];
	if(len <= 414)
		return _send(ctx, session, property, buf, len, reliable);
	const uint8_t *data = buf;
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
	const uint8_t *data2 = data;
	mp.multipartMessageId = pkt_readBaseMasterServerReliableRequest(&data2).requestId;
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
	struct ResendPacket *p = add_resend(session, property, mp.multipartMessageId, 0);
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
