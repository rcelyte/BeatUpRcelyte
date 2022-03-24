#include "log.h"
#define NET_H_PRIVATE(x) x
#include "enum_reflection.h"
#include "net.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/error.h>

#define lengthof(x) (sizeof(x)/sizeof(*(x)))

#ifdef WINDOWS
#define SHUT_RDWR SD_BOTH
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

#define ENCRYPTION_LAYER_SIZE 63

static const uint32_t PossibleMtu[] = {
	576 - ENCRYPTION_LAYER_SIZE - 68,
	1024 - ENCRYPTION_LAYER_SIZE,
	1232 - ENCRYPTION_LAYER_SIZE - 68,
	1460 - ENCRYPTION_LAYER_SIZE - 68,
	1472 - ENCRYPTION_LAYER_SIZE - 68,
	1492 - ENCRYPTION_LAYER_SIZE - 68,
	1500 - ENCRYPTION_LAYER_SIZE - 68,
};

const uint8_t *NetKeypair_get_random(const struct NetKeypair *keys) {
	return keys->random;
}
_Bool NetKeypair_write_key(const struct NetKeypair *keys, struct NetContext *ctx, uint8_t *out, uint32_t *out_len) {
	size_t keylen = 0;
	int32_t err = mbedtls_ecp_tls_write_point(&ctx->grp, &keys->public, MBEDTLS_ECP_PF_UNCOMPRESSED, &keylen, out, *out_len);
	if(err) {
		uprintf("mbedtls_ecp_tls_write_point() failed: %s\n", mbedtls_high_level_strerr(err));
		*out_len = 0;
		return 1;
	}
	*out_len = keylen;
	return 0;
}
const uint8_t *NetSession_get_cookie(const struct NetSession *session) {
	return session->cookie;
}
_Bool NetSession_signature(const struct NetSession *session, struct NetContext *ctx, const mbedtls_pk_context *key, const uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out) {
	out->length = 0;
	if(mbedtls_pk_get_type(key) != MBEDTLS_PK_RSA) {
		uprintf("Key should be RSA\n");
		return 1;
	}
	mbedtls_rsa_context *rsa = key->MBEDTLS_PRIVATE(pk_ctx);
	uint8_t hash[32];
	int32_t err = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), in, in_len, hash);
	if(err != 0) {
		uprintf("mbedtls_md() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	err = mbedtls_rsa_pkcs1_sign(rsa, mbedtls_ctr_drbg_random, &ctx->ctr_drbg, MBEDTLS_MD_SHA256, 32, hash, out->data);
	if(err != 0) {
		uprintf("mbedtls_rsa_pkcs1_sign() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	out->length = rsa->MBEDTLS_PRIVATE(len);
	return 0;
}
_Bool NetSession_set_clientPublicKey(struct NetSession *session, struct NetContext *ctx, const struct ByteArrayNetSerializable *in) {
	const uint8_t *buf = in->data;
	mbedtls_ecp_point clientPublicKey;
	mbedtls_ecp_point_init(&clientPublicKey);
	int32_t err = mbedtls_ecp_tls_read_point(&ctx->grp, &clientPublicKey, &buf, in->length);
	if(err != 0) {
		uprintf("mbedtls_ecp_tls_read_point() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	mbedtls_mpi preMasterSecret;
	mbedtls_mpi_init(&preMasterSecret);
	err = mbedtls_ecdh_compute_shared(&ctx->grp, &preMasterSecret, &clientPublicKey, &session->keys.secret, mbedtls_ctr_drbg_random, &ctx->ctr_drbg);
	if(err != 0) {
		uprintf("mbedtls_ecdh_compute_shared() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	return EncryptionState_init(&session->encryptionState, &preMasterSecret, session->keys.random, session->clientRandom, 0);
}
uint32_t NetSession_get_lastKeepAlive(struct NetSession *session) {
	return session->lastKeepAlive;
}
const struct SS *NetSession_get_addr(struct NetSession *session) {
	return &session->addr;
}

int32_t net_get_sockfd(struct NetContext *ctx) {
	return ctx->sockfd;
}

mbedtls_ctr_drbg_context *net_get_ctr_drbg(struct NetContext *ctx) {
	return &ctx->ctr_drbg;
}

void net_tostr(const struct SS *a, char *out) {
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

uint8_t addrs_are_equal(const struct SS *a0, const struct SS *a1) {
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

void net_send_internal(struct NetContext *ctx, struct NetSession *session, const uint8_t *buf, uint32_t len, _Bool encrypt) {
	struct PacketEncryptionLayer layer;
	layer.encrypted = 0;
	
	uint8_t head[512], body[1536];
	uint8_t *head_end = head;

	if(session->encryptionState.initialized && encrypt)
		EncryptionState_encrypt(&session->encryptionState, &layer, &ctx->ctr_drbg, buf, len, body, &len);
	else
		memcpy(body, buf, len); // const correctness ._.
	pkt_writePacketEncryptionLayer(session->version, &head_end, layer);
	#ifdef WINSOCK_VERSION
	WSABUF iov[] = {
		{.len = head_end - head, .buf = (char*)head},
		{.len = len, .buf = (char*)body},
	};
	WSAMSG msg = {
		.name = &session->addr.sa,
		.namelen = session->addr.len,
		.lpBuffers = iov,
		.dwBufferCount = lengthof(iov),
		.Control = {
			.len = 0,
			.buf = NULL,
		},
		.dwFlags = 0,
	};
	DWORD size = 0;
	WSASendMsg(ctx->sockfd, &msg, 0, &size, NULL, NULL);
	// uprintf("sendto[%lu]\n", size);
	#else
	struct iovec iov[] = {
		{.iov_base = head, .iov_len = head_end - head},
		{.iov_base = body, .iov_len = len},
	};
	struct msghdr msg = {
		.msg_name = &session->addr.sa,
		.msg_namelen = session->addr.len,
		.msg_iov = iov,
		.msg_iovlen = lengthof(iov),
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};
	/*ssize_t size =*/ sendmsg(ctx->sockfd, &msg, 0);
	// uprintf("sendto[%zd]\n", size);
	#endif
}

_Bool net_useIPv4 = 0;
_Bool net_init(struct NetContext *ctx, uint16_t port) {
	ctx->sockfd = socket(net_useIPv4 ? AF_INET : AF_INET6, SOCK_DGRAM, 0);
	mbedtls_ctr_drbg_init(&ctx->ctr_drbg);
	mbedtls_entropy_init(&ctx->entropy);
	mbedtls_ecp_group_init(&ctx->grp);
	if(ctx->sockfd == -1) {
		uprintf("Socket creation failed\n");
		return -1;
	}
	int res;
	if(net_useIPv4) {
		const struct sockaddr_in addr = {
			.sin_family = AF_INET,
			.sin_port = htons(port),
			.sin_addr = {
				.s_addr = htonl(INADDR_ANY),
			},
		};
		res = bind(ctx->sockfd, (const struct sockaddr*)&addr, sizeof(addr));
	} else {
		const struct sockaddr_in6 addr = {
			.sin6_family = AF_INET6,
			.sin6_port = htons(port),
			.sin6_flowinfo = 0,
			.sin6_addr = IN6ADDR_ANY_INIT,
			.sin6_scope_id = 0,
		};
		res = bind(ctx->sockfd, (const struct sockaddr*)&addr, sizeof(addr));
	}
	if(res < 0) {
		close(ctx->sockfd);
		ctx->sockfd = -1;
		uprintf("Socket binding failed\n");
		return 1;
	}
	struct SS realAddr = {sizeof(struct sockaddr_storage)};
	getsockname(ctx->sockfd, &realAddr.sa, &realAddr.len);
	char namestr[INET6_ADDRSTRLEN + 8];
	net_tostr(&realAddr, namestr);
	uprintf("Bound %s\n", namestr);
	if(mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14) != 0) {
		uprintf("mbedtls_ctr_drbg_seed() failed\n");
		return 1;
	}
	if(mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_SECP384R1)) {
		uprintf("mbedtls_ecp_group_load() failed\n");
		return 1;
	}
	if(pthread_mutex_init(&ctx->mutex, NULL)) {
		uprintf("pthread_mutex_init() failed\n");
		return 1;
	}
	ctx->run = 1;
	ctx->sessionList = NULL;
	ctx->dirt = NULL;
	return 0;
}

void net_session_init(struct NetContext *ctx, struct NetSession *session, struct SS addr) {
	session->addr = addr;
	session->encryptionState.initialized = 0;
	net_keypair_init(&session->keys);
	net_session_reset(ctx, session);
}

void net_session_free(struct NetSession *session) {
	EncryptionState_free(&session->encryptionState);
	net_keypair_free(&session->keys);
}

void net_keypair_init(struct NetKeypair *keys) {
	mbedtls_mpi_init(&keys->secret);
	mbedtls_ecp_point_init(&keys->public);
}

void net_keypair_gen(struct NetContext *ctx, struct NetKeypair *keys) {
	net_cookie(&ctx->ctr_drbg, keys->random);
	if(mbedtls_ecp_gen_keypair(&ctx->grp, &keys->secret, &keys->public, mbedtls_ctr_drbg_random, &ctx->ctr_drbg)) {
		uprintf("mbedtls_ecp_gen_keypair() failed\n");
		abort();
	}
}

void net_keypair_free(struct NetKeypair *keys) {
	mbedtls_mpi_free(&keys->secret);
	mbedtls_ecp_point_free(&keys->public);
}

void net_session_reset(struct NetContext *ctx, struct NetSession *session) {
	struct SS addr = session->addr;
	net_session_free(session);
	memset(session, 0, sizeof(*session));
	session->version = PV_LEGACY_DEFAULT;
	net_cookie(&ctx->ctr_drbg, session->cookie);
	session->addr = addr;
	session->lastKeepAlive = net_time();
	session->mtu = PossibleMtu[0];
	session->mtuIdx = 0;
	session->mergeData_end = session->mergeData;
	net_flush_merged(ctx, session);
	net_keypair_gen(ctx, &session->keys);
}

void net_stop(struct NetContext *ctx) {
	if(ctx->sockfd == -1)
		return;
	ctx->run = 0;
	shutdown(ctx->sockfd, SHUT_RDWR);
}

void net_cleanup(struct NetContext *ctx) {
	if(pthread_mutex_destroy(&ctx->mutex))
		uprintf("pthread_mutex_destroy() failed\n");
	mbedtls_entropy_free(&ctx->entropy);
	mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
	close(ctx->sockfd);
	ctx->sockfd = -1;
}

void net_lock(struct NetContext *ctx) {
	#if 1
	if(pthread_mutex_trylock(&ctx->mutex) == 0)
		return;
	uprintf("block\n");
	#endif
	pthread_mutex_lock(&ctx->mutex);
}
void net_unlock(struct NetContext *ctx) {
	pthread_mutex_unlock(&ctx->mutex);
}

uint32_t net_recv(struct NetContext *ctx, uint8_t *buf, uint32_t buf_len, struct NetSession **session, const uint8_t **pkt, void **userdata_out) {
	retry:; // __attribute__((musttail)) not available in all compilers
	uint32_t currentTime = net_time(), nextTick = currentTime + 180000;
	ctx->onResend(ctx->user, currentTime, &nextTick);
	nextTick -= currentTime;
	if(nextTick < 2)
		nextTick = 2;
	struct timeval timeout;
	timeout.tv_sec = nextTick / 1000; // Don't loop if there's nothing to do
	timeout.tv_usec = (nextTick % 1000) * 1000;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(ctx->sockfd, &fds);
	net_unlock(ctx);
	if(select(ctx->sockfd+1, &fds, NULL, NULL, &timeout) == 0) {
		net_lock(ctx);
		goto retry;
	}
	struct SS addr = {sizeof(struct sockaddr_storage)};
	#ifdef WINSOCK_VERSION
	ssize_t size = recvfrom(ctx->sockfd, (char*)buf, buf_len, 0, &addr.sa, &addr.len);
	#else
	ssize_t size = recvfrom(ctx->sockfd, buf, buf_len, 0, &addr.sa, &addr.len);
	#endif
	net_lock(ctx);
	if(size <= 0) {
		if(ctx->run)
			goto retry;
		if(size == -1)
			uprintf("recvfrom() failed: %s\n", strerror(errno));
		return 0;
	}
	if(&buf[buf_len] < ctx->dirt) {
		uprintf("BAD BUFFER CONTENTS\n");
		abort();
	}
	if(&buf[size] < ctx->dirt) // Since deserialization doesn't have range checks, we need to clean up data from previous messages
		memset(&buf[size], 0, ctx->dirt - &buf[size]);
	ctx->dirt = &buf[size];
	if(addr.sa.sa_family == AF_UNSPEC) {
		uprintf("UNSPEC\n");
		goto retry;
	}
	if(buf[0] > 1) {
		sendto(ctx->sockfd, (char*)buf, 1, 0, &addr.sa, addr.len);
		char namestr[INET6_ADDRSTRLEN + 8];
		net_tostr(&addr, namestr);
		uprintf("ping[%s]: %hhu\n", namestr, buf[0]);
		goto retry;
	}
	*session = ctx->onResolve(ctx->user, addr, userdata_out);
	if(!*session)
		goto retry;
	// uprintf("recvfrom[%zi]\n", size);
	uint8_t *head = buf;
	struct PacketEncryptionLayer layer = pkt_readPacketEncryptionLayer((*session)->version, (const uint8_t**)&head);
	if(layer.encrypted == 1) { // TODO: filter unencrypted?
		uint32_t length = &buf[size] - head;
		if(EncryptionState_decrypt(&(*session)->encryptionState, layer, head, &length)) {
			uprintf("Packet decryption failed\n");
			goto retry;
		}
		size = length + (head - buf);
		(*session)->lastKeepAlive = net_time();
		while((*session)->mtu < size && (*session)->mtuIdx < lengthof(PossibleMtu) - 1) {
			uint32_t oldMtu = (*session)->mtu;
			(*session)->mtu = PossibleMtu[++(*session)->mtuIdx];
			uprintf("MTU %u -> %u\n", oldMtu, (*session)->mtu);
		}
	} else if(layer.encrypted) {
		uprintf("Invalid packet\n");
		goto retry;
	}
	*pkt = head;
	return &buf[size] - head;
}
void net_flush_merged(struct NetContext *ctx, struct NetSession *session) {
	if(session->mergeData_end - session->mergeData > 3)
		net_send_internal(ctx, session, session->mergeData, session->mergeData_end - session->mergeData, 1);
	session->mergeData_end = session->mergeData;
	pkt_writeNetPacketHeader(session->version, &session->mergeData_end, (struct NetPacketHeader){
		.property = PacketProperty_Merged,
		.connectionNumber = 0,
		.isFragmented = 0,
	});
}
void net_queue_merged(struct NetContext *ctx, struct NetSession *session, const uint8_t *buf, uint16_t len) {
	if((session->mergeData_end - session->mergeData) + len + 2 > session->mtu)
		net_flush_merged(ctx, session);
	pkt_writeUint16(session->version, &session->mergeData_end, len);
	pkt_writeUint8Array(session->version, &session->mergeData_end, buf, len);
}
