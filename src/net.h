#pragma once
#include "wire.h"
#include "encryption.h"
#include "perf.h"
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windef.h>
#define SHUT_RD SD_RECEIVE
#define SHUT_RDWR SD_BOTH
typedef int socklen_t;
#else
#include <arpa/inet.h>
#endif

#ifndef NET_H_PRIVATE
#define CONCAT2(a,b) a##b
#define CONCAT(a,b) CONCAT2(a,b)
#define NET_H_PRIVATE(x) CONCAT(_p_,__COUNTER__)
#endif

#define NET_MAX_PKT_SIZE 1432
#define NET_MAX_SEQUENCE 32768
#define NET_MAX_WINDOW_SIZE 64
#define NET_RESEND_DELAY 27

#define NET_THREAD_INVALID 0 // TODO: this macro marks all non-portable uses of the pthreads API

char* net_get_error_str();
char* net_get_error_str_from(int);

bool sockets_init();

struct SS {
	socklen_t len;
	union {
		struct sockaddr_storage ss;
		struct sockaddr sa;
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
	};
};

bool SS_equal(const struct SS *a0, const struct SS *a1);
void net_tostr(const struct SS *a, char out[static INET6_ADDRSTRLEN + 8]);

struct NetKeypair {
	uint8_t NET_H_PRIVATE(random)[32];
	mbedtls_mpi NET_H_PRIVATE(secret);
	mbedtls_ecp_point NET_H_PRIVATE(public);
};

struct NetSession {
	struct NetKeypair keys;
	struct PacketContext version;
	uint8_t clientRandom[32];
	uint8_t NET_H_PRIVATE(cookie)[32];
	struct EncryptionState NET_H_PRIVATE(encryptionState);
	struct SS NET_H_PRIVATE(addr);
	uint32_t lastKeepAlive;
	uint16_t NET_H_PRIVATE(mtu);
	uint8_t NET_H_PRIVATE(mtuIdx);
	bool alive;
	uint16_t maxChanneledSize, maxFragmentSize, fragmentId;
	uint8_t *NET_H_PRIVATE(mergeData_end);
	uint8_t NET_H_PRIVATE(mergeData)[NET_MAX_PKT_SIZE];
};

struct NetContext {
	WireLinkType _typeid; // used to distinguish between local (struct NetContext) and remote (mbedtls_ssl_context) connections
	int32_t NET_H_PRIVATE(sockfd), NET_H_PRIVATE(listenfd);
	atomic_bool NET_H_PRIVATE(run);
	bool NET_H_PRIVATE(filterUnencrypted);
	pthread_mutex_t NET_H_PRIVATE(mutex);
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context NET_H_PRIVATE(entropy);
	mbedtls_ecp_group NET_H_PRIVATE(grp);
	uint32_t NET_H_PRIVATE(remoteLinks_len);
	uint32_t NET_H_PRIVATE(cookies_len);
	union {
		mbedtls_ssl_context *single;
		mbedtls_ssl_context **list;
	} NET_H_PRIVATE(remoteLinks);
	struct WireCookie *NET_H_PRIVATE(cookies);
	void *userptr;
	struct NetSession *(*onResolve)(void *userptr, struct SS addr, void **userdata_out);
	void (*onResend)(void *userptr, uint32_t currentTime, uint32_t *nextTick);
	void (*onWireLink)(void *userptr, union WireLink *link);
	void (*onWireMessage)(void *userptr, union WireLink *link, const struct WireMessage *message);
	struct Performance NET_H_PRIVATE(perf);
};

// Initialize memory such that calling `net_cleanup()` without a prior `net_init()` is well defined
#define CLEAR_NETCONTEXT {._typeid = WireLinkType_INVALID}

void net_keypair_init(struct NetKeypair *keys);
void net_keypair_gen(struct NetContext *ctx, struct NetKeypair *keys);
void net_keypair_free(struct NetKeypair *keys);
const uint8_t *NetKeypair_get_random(const struct NetKeypair *keys);
bool NetKeypair_write_key(const struct NetKeypair *keys, struct NetContext *ctx, uint8_t *out, uint32_t *out_len);

const uint8_t *NetSession_get_cookie(const struct NetSession *session);
bool NetSession_signature(const struct NetSession *session, struct NetContext *ctx, const mbedtls_pk_context *key, const uint8_t *in, uint32_t in_len, struct ByteArrayNetSerializable *out);
bool NetSession_set_clientPublicKey(struct NetSession *session, struct NetContext *ctx, const struct ByteArrayNetSerializable *in);
uint32_t NetSession_get_lastKeepAlive(struct NetSession *session);
const struct SS *NetSession_get_addr(struct NetSession *session);

bool net_init(struct NetContext *ctx, uint16_t port, bool filterUnencrypted);
void net_stop(struct NetContext *ctx);
void net_cleanup(struct NetContext *ctx);
void net_lock(struct NetContext *ctx);
void net_unlock(struct NetContext *ctx);
void net_session_init(struct NetContext *ctx, struct NetSession *session, struct SS addr);
void net_session_reset(struct NetContext *ctx, struct NetSession *session);
void net_session_free(struct NetSession *session);
bool net_add_remote(struct NetContext *ctx, mbedtls_ssl_context *link);
bool net_remove_remote(struct NetContext *ctx, mbedtls_ssl_context *link);
uint32_t net_recv(struct NetContext *ctx, uint8_t out[static 1536], struct NetSession **session, void **userdata_out);
void net_flush_merged(struct NetContext *ctx, struct NetSession *session);
void net_queue_merged(struct NetContext *ctx, struct NetSession *session, const uint8_t *buf, uint16_t len);
void net_send_internal(struct NetContext *ctx, struct NetSession *session, const uint8_t *buf, uint32_t len, bool encrypt);
int32_t net_get_sockfd(struct NetContext *ctx);
mbedtls_ctr_drbg_context *net_get_ctr_drbg(struct NetContext *ctx);

uint32_t net_time();

static inline int32_t RelativeSequenceNumber(int32_t to, int32_t from) {
	return (to - from + NET_MAX_SEQUENCE + NET_MAX_SEQUENCE / 2) % NET_MAX_SEQUENCE - NET_MAX_SEQUENCE / 2;
}

extern bool net_useIPv4;
