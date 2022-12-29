#define __STDC_WANT_LIB_EXT1__ 1
#include "global.h"
#define NET_H_PRIVATE(x) x
#include "net.h"
#include "packets.h"
#include <mbedtls/error.h>
#include <mbedtls/sha256.h>

#ifdef WINDOWS
#define net_error() WSAGetLastError()
#else
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#define net_error() (errno)
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENCRYPTION_LAYER_SIZE 63

static const uint16_t PossibleMtu[] = {
	576 - ENCRYPTION_LAYER_SIZE - 68,
	1024 - ENCRYPTION_LAYER_SIZE,
	1232 - ENCRYPTION_LAYER_SIZE - 68,
	1460 - ENCRYPTION_LAYER_SIZE - 68,
	1472 - ENCRYPTION_LAYER_SIZE - 68,
	1492 - ENCRYPTION_LAYER_SIZE - 68,
	1500 - ENCRYPTION_LAYER_SIZE - 68,
};

const struct Cookie32 *NetKeypair_get_random(const struct NetKeypair *keys) {
	return &keys->random;
}
static uint32_t NetKeypair_write_key_internal(const struct NetKeypair *keys, struct NetContext *ctx, uint8_t *out, size_t out_len) {
	size_t keylen = 0;
	int32_t err = mbedtls_ecp_tls_write_point(&ctx->grp, &keys->public, MBEDTLS_ECP_PF_UNCOMPRESSED, &keylen, out, out_len);
	if(err) {
		uprintf("mbedtls_ecp_tls_write_point() failed: %s\n", mbedtls_high_level_strerr(err));
		keylen = 0;
	}
	return (uint32_t)keylen;
}
bool NetKeypair_write_key(const struct NetKeypair *keys, struct NetContext *ctx, struct ByteArrayNetSerializable *out) {
	out->length = NetKeypair_write_key_internal(keys, ctx, out->data, sizeof(out->data));
	return out->length == 0;
}
const struct Cookie32 *NetSession_get_cookie(const struct NetSession *session) {
	return &session->cookie;
}
bool NetSession_signature(struct NetSession *session, struct NetContext *ctx, const mbedtls_pk_context *key, struct ByteArrayNetSerializable *out) {
	out->length = 0;
	if(mbedtls_pk_get_type(key) != MBEDTLS_PK_RSA) {
		uprintf("Key should be RSA\n");
		return true;
	}
	struct {
		struct Cookie32 clientRandom;
		struct Cookie32 serverRandom;
		uint8_t publicKey[4096];
	} input = {
		.clientRandom = session->clientRandom,
		.serverRandom = session->keys.random,
	};
	size_t publicKey_len = NetKeypair_write_key_internal(&session->keys, ctx, input.publicKey, sizeof(input.publicKey));
	if(!publicKey_len)
		return true;
	uint8_t hash[32];
	int32_t err = mbedtls_sha256((const uint8_t*)&input, publicKey_len + sizeof(struct Cookie32[2]), hash, 0);
	if(err != 0) {
		uprintf("mbedtls_sha256() failed: %s\n", mbedtls_high_level_strerr(err));
		return true;
	}
	mbedtls_rsa_context *rsa = mbedtls_pk_rsa(*key);
	err = mbedtls_rsa_pkcs1_sign(rsa, mbedtls_ctr_drbg_random, &ctx->ctr_drbg, MBEDTLS_MD_SHA256, sizeof(hash), hash, out->data);
	if(err != 0) {
		uprintf("mbedtls_rsa_pkcs1_sign() failed: %s\n", mbedtls_high_level_strerr(err));
		return true;
	}
	out->length = (uint32_t)mbedtls_rsa_get_len(rsa);
	return false;
}
bool NetSession_set_remotePublicKey(struct NetSession *session, struct NetContext *ctx, const struct ByteArrayNetSerializable *in, bool client) {
	bool res = true;
	mbedtls_ecp_point clientPublicKey;
	mbedtls_mpi sharedSecret;
	mbedtls_ecp_point_init(&clientPublicKey);
	mbedtls_mpi_init(&sharedSecret);
	int32_t err = mbedtls_ecp_tls_read_point(&ctx->grp, &clientPublicKey, (const uint8_t*[]){in->data}, in->length);
	if(err != 0) {
		uprintf("mbedtls_ecp_tls_read_point() failed: %s\n", mbedtls_high_level_strerr(err));
		goto fail;
	}
	err = mbedtls_ecdh_compute_shared(&ctx->grp, &sharedSecret, &clientPublicKey, &session->keys.secret, mbedtls_ctr_drbg_random, &ctx->ctr_drbg);
	if(err != 0) {
		uprintf("mbedtls_ecdh_compute_shared() failed: %s\n", mbedtls_high_level_strerr(err));
		goto fail;
	}
	res = EncryptionState_init(&session->encryptionState, &sharedSecret, (struct Cookie32[]){session->keys.random, session->clientRandom}, client);
	fail:
	mbedtls_mpi_free(&sharedSecret);
	mbedtls_ecp_point_free(&clientPublicKey);
	return res;
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

void net_tostr(const struct SS *address, char out[static INET6_ADDRSTRLEN + 8]) {
	char ipStr[INET6_ADDRSTRLEN];
	switch(address->ss.ss_family) {
		case AF_UNSPEC: {
			sprintf(out, "AF_UNSPEC");
			break;
		}
		case AF_INET: {
			inet_ntop(AF_INET, &address->in.sin_addr, ipStr, INET_ADDRSTRLEN);
			snprintf(out, INET6_ADDRSTRLEN + 8, "%s:%u", ipStr, htons(address->in.sin_port));
			break;
		}
		case AF_INET6: {
			inet_ntop(AF_INET6, &address->in6.sin6_addr, ipStr, INET6_ADDRSTRLEN);
			snprintf(out, INET6_ADDRSTRLEN + 8, "[%s]:%u", ipStr, htons(address->in6.sin6_port));
			break;
		}
		default: sprintf(out, "???");
	}
}

static struct sockaddr_in6 SS_to6(const struct SS *addr) {
	if(addr->in6.sin6_family != AF_INET)
		return addr->in6;
	const uint8_t *bytes = (const uint8_t*)&addr->in.sin_addr.s_addr;
	return (struct sockaddr_in6){
		.sin6_family = AF_INET6,
		.sin6_port = addr->in.sin_port,
		.sin6_flowinfo = 0,
		.sin6_addr.s6_addr = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, bytes[0], bytes[1], bytes[2], bytes[3]},
		.sin6_scope_id = 0,
	};
}

bool SS_equal(const struct SS *addr0, const struct SS *addr1) {
	struct sockaddr_in6 norm[2] = {SS_to6(addr0), SS_to6(addr1)};
	if(norm[0].sin6_family != AF_INET6 || norm[1].sin6_family != AF_INET6)
		return false;
	return memcmp(&norm[0].sin6_addr, &norm[1].sin6_addr, sizeof(struct in6_addr)) == 0 && norm[0].sin6_port == norm[1].sin6_port;
}

static struct Cookie32 net_cookie(mbedtls_ctr_drbg_context *ctr_drbg) {
	struct Cookie32 out;
	mbedtls_ctr_drbg_random(ctr_drbg, out.raw, sizeof(out.raw));
	return out;
}

static struct timespec GetTime() {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return (struct timespec){0, 0};
	return now;
}

uint32_t net_time() {
	struct timespec now = GetTime();
	return (uint32_t)((uint64_t)now.tv_sec * UINT64_C(1000) + (uint64_t)now.tv_nsec / UINT64_C(1000000));
}

void net_send_internal(struct NetContext *ctx, struct NetSession *session, const uint8_t *buf, uint32_t len, bool encrypt) {
	uint8_t body[1536];
	uint32_t body_len = EncryptionState_encrypt(encrypt ? &session->encryptionState : NULL, &ctx->ctr_drbg, buf, len, body);
	sendto(ctx->sockfd, (char*)body, body_len, 0, &session->addr.sa, session->addr.len);
}

static struct NetSession *onResolve_stub(void*, struct SS, void**) {return NULL;}
static uint32_t onResend_stub(void*, uint32_t) {return 180000;}
static void onWireMessage_stub(void*, union WireLink*, const struct WireMessage*) {}

static const char *net_strerror(int32_t err) {
	static _Thread_local char message[1024];
	*message = 0;
	#if defined(WINDOWS)
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, message, sizeof(message), NULL);
	#elif defined(__STDC_LIB_EXT1__)
	strerror_s(message, lengthof(message), err);
	#elif (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !defined(_GNU_SOURCE)
	strerror_r(err, message, lengthof(message));
	#else
	#error No strerror_s implementation available
	#endif
	if(!*message)
		snprintf(message, lengthof(message), "%d", err);
	return message;
}

bool net_useIPv4 = 0;
static int32_t net_bind_udp(uint16_t port) {
	#ifdef WINDOWS
	int err = WSAStartup(MAKEWORD(2,0), &(WSADATA){0});
	if(err) {
		uprintf("WSAStartup failed: %s\n", port, net_strerror(err));
		return -1;
	}
	#endif
	int32_t sockfd = socket(net_useIPv4 ? AF_INET : AF_INET6, SOCK_DGRAM, 0);
	if(sockfd == -1) {
		uprintf("Failed to open UDP socket: %s\n", net_strerror(net_error()));
		#ifdef WINDOWS
		WSACleanup();
		#endif
		return -1;
	}
	struct SS addr;
	if(net_useIPv4) {
		addr.len = sizeof(struct sockaddr_in);
		addr.in = (struct sockaddr_in){
			.sin_family = AF_INET,
			.sin_port = htons(port),
			.sin_addr.s_addr = htonl(INADDR_ANY),
		};
	} else {
		addr.len = sizeof(struct sockaddr_in6);
		addr.in6 = (struct sockaddr_in6){
			.sin6_family = AF_INET6,
			.sin6_port = htons(port),
			.sin6_flowinfo = 0,
			.sin6_addr = IN6ADDR_ANY_INIT,
			.sin6_scope_id = 0,
		};
	}
	if(bind(sockfd, &addr.sa, addr.len) < 0) {
		uprintf("Cannot bind socket to port %hu: %s\n", port, net_strerror(net_error()));
		net_close(sockfd);
		return -1;
	}
	return sockfd;
}

int32_t net_bind_tcp(uint16_t port, uint32_t backlog) {
	#ifdef WINDOWS
	int err = WSAStartup(MAKEWORD(2,0), &(WSADATA){0});
	if(err) {
		uprintf("WSAStartup failed: %s\n", port, net_strerror(err));
		return -1;
	}
	#else
	signal(SIGPIPE, SIG_IGN);
	#endif
	int32_t listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(listenfd == -1) {
		uprintf("Failed to open TCP socket: %s\n", net_strerror(net_error()));
		#ifdef WINDOWS
		WSACleanup();
		#endif
		return -1;
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)(int32_t[]){1}, sizeof(int32_t));
	struct sockaddr_in6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(port),
		.sin6_flowinfo = 0,
		.sin6_addr = IN6ADDR_ANY_INIT,
		.sin6_scope_id = 0,
	};
	if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		uprintf("Cannot bind socket to port %hu: %s\n", port, net_strerror(net_error()));
		goto fail;
	}
	if(listen(listenfd, (int)backlog) < 0) {
		uprintf("listen() failed: %s\n", net_strerror(net_error()));
		goto fail;
	}
	return listenfd;
	fail:
	net_close(listenfd);
	return -1;
}

void net_close(int32_t sockfd) {
	if(sockfd == -1)
		return;
	close(sockfd);
	#ifdef WINDOWS
	WSACleanup();
	#endif
}

bool net_init(struct NetContext *ctx, uint16_t port, bool filterUnencrypted, uint32_t tcpBacklog) {
	*ctx = (struct NetContext){
		._typeid = WireLinkType_LOCAL,
		.sockfd = net_bind_udp(port),
		.listenfd = tcpBacklog ? net_bind_tcp(port, tcpBacklog) : -1,
		.run = false,
		.filterUnencrypted = filterUnencrypted,
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		// .ctr_drbg = {},
		// .entropy = {},
		// .grp = {},
		.remoteLinks_len = 0,
		.cookies_len = 0,
		.remoteLinks = {NULL},
		.cookies = NULL,
		.userptr = NULL,
		.onResolve = onResolve_stub,
		.onResend = onResend_stub,
		.onWireLink = NULL,
		.onWireMessage = onWireMessage_stub,
		.perf = perf_init(),
	};
	mbedtls_ctr_drbg_init(&ctx->ctr_drbg);
	mbedtls_entropy_init(&ctx->entropy);
	mbedtls_ecp_group_init(&ctx->grp);
	pthread_mutexattr_t mutexAttribs;
	if(pthread_mutexattr_init(&mutexAttribs) ||
	   pthread_mutexattr_settype(&mutexAttribs, PTHREAD_MUTEX_RECURSIVE) ||
	   pthread_mutex_init(&ctx->mutex, &mutexAttribs)) {
		uprintf("pthread_mutex_init() failed\n");
		goto fail;
	}
	if(ctx->sockfd == -1 || (tcpBacklog && ctx->listenfd == -1)) {
		uprintf("Socket creation failed\n");
		goto fail;
	}
	struct SS realAddr = {.len = sizeof(struct sockaddr_storage)};
	getsockname(ctx->sockfd, &realAddr.sa, &realAddr.len);
	char namestr[INET6_ADDRSTRLEN + 8];
	net_tostr(&realAddr, namestr);
	uprintf("Bound %s\n", namestr);

	if(mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14) != 0) {
		uprintf("mbedtls_ctr_drbg_seed() failed\n");
		goto fail;
	}
	if(mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_SECP384R1)) {
		uprintf("mbedtls_ecp_group_load() failed\n");
		goto fail;
	}
	atomic_store(&ctx->run, true);
	return false;
	fail:
	net_cleanup(ctx);
	return true;
}

static void net_set_mtu(struct NetSession *session, uint8_t idx) {
	uint32_t oldMtu = session->mtu;
	session->mtu = PossibleMtu[idx];
	session->mtuIdx = idx;
	uprintf("MTU %u -> %u\n", oldMtu, session->mtu);
	uint8_t buf[NET_MAX_PKT_SIZE], *buf_end = buf;
	session->maxChanneledSize = session->mtu - (uint16_t)pkt_write_c(&buf_end, endof(buf), session->version, NetPacketHeader, {
		.property = PacketProperty_Channeled,
	});
	session->maxFragmentSize = session->maxChanneledSize - (uint16_t)pkt_write_c(&buf_end, endof(buf), session->version, FragmentedHeader, {0});
}

void net_session_init(struct NetContext *ctx, struct NetSession *session, struct SS addr) {
	*session = (struct NetSession){
		.version = PV_LEGACY_DEFAULT,
		.cookie = net_cookie(&ctx->ctr_drbg),
		.addr = addr,
		.lastKeepAlive = net_time(),
		.mtu = 0,
		.alive = true,
		.fragmentId = 0,
		.mergeData_end = session->mergeData,
	};
	net_keypair_init(ctx, &session->keys);
	net_set_mtu(session, 0);
	net_flush_merged(ctx, session);
}

void net_session_free(struct NetSession *session) {
	EncryptionState_free(&session->encryptionState);
	net_keypair_free(&session->keys);
}

void net_keypair_init(struct NetContext *ctx, struct NetKeypair *keys) {
	keys->random = net_cookie(&ctx->ctr_drbg);
	mbedtls_mpi_init(&keys->secret);
	mbedtls_ecp_point_init(&keys->public);
	if(mbedtls_ecp_gen_keypair(&ctx->grp, &keys->secret, &keys->public, mbedtls_ctr_drbg_random, &ctx->ctr_drbg)) {
		uprintf("mbedtls_ecp_gen_keypair() failed\n");
		abort();
	}
}

void net_keypair_free(struct NetKeypair *keys) {
	mbedtls_mpi_free(&keys->secret);
	mbedtls_ecp_point_free(&keys->public);
}

void net_stop(struct NetContext *ctx) {
	if(ctx->sockfd == -1)
		return;
	atomic_store(&ctx->run, false);
	shutdown(ctx->sockfd, SHUT_RDWR);
}

static mbedtls_ssl_context **NetContext_remoteLinks(struct NetContext *ctx) {
	return (ctx->remoteLinks_len == 1) ? &ctx->remoteLinks.single : ctx->remoteLinks.list;
}

void net_cleanup(struct NetContext *ctx) {
	if(ctx->_typeid != WireLinkType_LOCAL)
		return;
	while(ctx->remoteLinks_len) {
		union WireLink *link = (union WireLink*)*NetContext_remoteLinks(ctx);
		if(WireLink_cast_remote(link)) {
			wire_disconnect(ctx, link);
		} else {
			uprintf("WireLink_cast_remote() failed\n");
			net_remove_remote(ctx, (mbedtls_ssl_context*)link);
		}
	}
	if(pthread_mutex_destroy(&ctx->mutex)) // TODO: ensure unlock
		uprintf("pthread_mutex_destroy() failed\n");
	free(ctx->cookies);
	mbedtls_entropy_free(&ctx->entropy);
	mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
	net_close(ctx->listenfd);
	net_close(ctx->sockfd);
	ctx->_typeid = WireLinkType_INVALID;
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

bool net_add_remote(struct NetContext *ctx, mbedtls_ssl_context *link) {
	uprintf("net_add_remote(%p) %u -> %u\n", link, ctx->remoteLinks_len, ctx->remoteLinks_len + 1);
	mbedtls_ssl_context **list;
	switch(ctx->remoteLinks_len) {
		case 0: {
			ctx->remoteLinks.single = link;
			++ctx->remoteLinks_len;
			return false;
		}
		case 1: {
			list = malloc((ctx->remoteLinks_len + 1) * sizeof(*list));
			if(list)
				list[0] = ctx->remoteLinks.single;
			break;
		}
		default: list = realloc(ctx->remoteLinks.list, (ctx->remoteLinks_len + 1) * sizeof(*list));
	}
	if(!list) {
		uprintf("alloc error\n");
		return true;
	}
	list[ctx->remoteLinks_len++] = link;
	ctx->remoteLinks.list = list;
	return false;
}

// TODO: Invalidating the `remoteLinks` causes pain elsewhere (see hacks at `wire_recv()` callsite)
// Entries should be set to NULL here, and the array should be left sparse until it is safe to resize
bool net_remove_remote(struct NetContext *ctx, mbedtls_ssl_context *link) {
	mbedtls_ssl_context **list = NetContext_remoteLinks(ctx), **it = list;
	for(mbedtls_ssl_context *const *end = &list[ctx->remoteLinks_len]; it < end; ++it)
		if(*it == link)
			goto found;
	uprintf("net_remove_remote(): no match found\n");
	return true;
	found:
	uprintf("net_remove_remote(%p) %u -> %u\n", link, ctx->remoteLinks_len, ctx->remoteLinks_len - 1);
	*it = list[--ctx->remoteLinks_len];
	switch(ctx->remoteLinks_len) {
		case 0: ctx->remoteLinks.single = NULL; break;
		case 1: {
			ctx->remoteLinks.single = list[0];
			free(list);
			break;
		}
		default: {
			list = realloc(ctx->remoteLinks.list, ctx->remoteLinks_len * sizeof(*list));
			if(list)
				ctx->remoteLinks.list = list;
		}
	}
	return false;
}

static inline int32_t max32(int32_t a, int32_t b) {
	return a > b ? a : b;
}

static bool net_poll(struct NetContext *ctx, fd_set *fdSet, uint32_t timeout) {
	FD_ZERO(fdSet);
	FD_SET(ctx->sockfd, fdSet);
	if(ctx->listenfd != -1)
		FD_SET(ctx->listenfd, fdSet);
	int32_t fdMax = max32(ctx->sockfd, ctx->listenfd);
	for(mbedtls_ssl_context **link = NetContext_remoteLinks(ctx), **end = &link[ctx->remoteLinks_len]; link < end; ++link) {
		int32_t remotefd = (int32_t)(intptr_t)(*link)->MBEDTLS_PRIVATE(p_bio);
		FD_SET(remotefd, fdSet);
		fdMax = max32(fdMax, remotefd);
	}
	net_unlock(ctx);
	[[maybe_unused]] struct timespec sleepStart = GetTime();
	int nfd = select(fdMax + 1, fdSet, NULL, NULL, &(struct timeval){
		.tv_sec = timeout / 1000,
		.tv_usec = (timeout % 1000) * 1000,
	});
	[[maybe_unused]] struct timespec sleepEnd = GetTime();
	net_lock(ctx);
	#ifdef PERFTEST
	perf_tick(&ctx->perf, sleepStart, sleepEnd);
	#endif
	if(nfd == -1) {
		uprintf("select() failed: %s\n", net_strerror(net_error()));
		FD_ZERO(fdSet);
		FD_SET(ctx->sockfd, fdSet);
		atomic_store(&ctx->run, false);
	}
	return nfd == 0;
}

uint32_t net_recv(struct NetContext *ctx, uint8_t out[static 1536], struct NetSession **session, void **userdata_out) {
	retry:; // __attribute__((musttail)) not available in all compilers
	fd_set fdSet;
	for(uint32_t nextTick = 0; net_poll(ctx, &fdSet, nextTick); nextTick = (nextTick >= 2) ? nextTick : 2)
		nextTick = ctx->onResend(ctx->userptr, net_time());
	for(uint32_t i = 0, len = ctx->remoteLinks_len; i < len; ++i) {
		mbedtls_ssl_context *link = NetContext_remoteLinks(ctx)[i];
		if(!FD_ISSET((intptr_t)link->MBEDTLS_PRIVATE(p_bio), &fdSet))
			continue;
		wire_recv(ctx, link); // May invalidate `link` AND `ctx->remoteLinks`

		// TODO: This is a really bad hack to check for disconnects, expoliting the fact that the current link is the only one disconnected in any wire handler
		if(ctx->remoteLinks_len < len) {
			len = ctx->remoteLinks_len;
			--i;
		}
	}
	if(ctx->listenfd != -1 && FD_ISSET(ctx->listenfd, &fdSet))
		wire_accept(ctx, ctx->listenfd);
	if(!FD_ISSET(ctx->sockfd, &fdSet))
		goto retry;
	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	uint8_t raw[1536];
	#ifdef WINSOCK_VERSION
	ssize_t raw_len = recvfrom(ctx->sockfd, (char*)raw, sizeof(raw), 0, &addr.sa, &addr.len);
	#else
	ssize_t raw_len = recvfrom(ctx->sockfd, raw, sizeof(raw), 0, &addr.sa, &addr.len);
	#endif
	if(raw_len <= 0) {
		if(atomic_load(&ctx->run))
			goto retry;
		if(raw_len == -1)
			uprintf("recvfrom() failed: %s\n", net_strerror(net_error()));
		return 0;
	}
	if(addr.sa.sa_family == AF_UNSPEC) {
		uprintf("UNSPEC\n");
		goto retry;
	}
	if(raw[0] > 1) { // protocol extension for pinging the server
		sendto(ctx->sockfd, (char*)raw, 1, 0, &addr.sa, addr.len);
		char namestr[INET6_ADDRSTRLEN + 8];
		net_tostr(&addr, namestr);
		// uprintf("ping[%s]: %hhu\n", namestr, raw[0]);
		goto retry;
	}
	*session = ctx->onResolve(ctx->userptr, addr, userdata_out);
	if(!*session)
		goto retry;
	uint32_t length = EncryptionState_decrypt(&(*session)->encryptionState, raw, &raw[raw_len], out);
	if(!length) {
		uprintf("Packet decryption failed\n");
		goto retry;
	}
	if(*raw == 1) { // TODO: expose encryption state from `EncryptionState_decrypt`
		if((*session)->alive)
			(*session)->lastKeepAlive = net_time();
		while((*session)->mtu < length && (*session)->mtuIdx < lengthof(PossibleMtu) - 1)
			net_set_mtu(*session, (*session)->mtuIdx + 1);
	} else if(ctx->filterUnencrypted) {
		goto retry;
	}
	return length;
}
void net_flush_merged(struct NetContext *ctx, struct NetSession *session) {
	if(session->mergeData_end - session->mergeData > 3)
		net_send_internal(ctx, session, session->mergeData, (uint32_t)(session->mergeData_end - session->mergeData), 1);
	session->mergeData_end = session->mergeData;
	pkt_write_c(&session->mergeData_end, endof(session->mergeData), session->version, NetPacketHeader, {
		.property = PacketProperty_Merged,
		.connectionNumber = 0,
		.isFragmented = 0,
	});
}
void net_queue_merged(struct NetContext *ctx, struct NetSession *session, const uint8_t *buf, uint16_t len) {
	if((session->mergeData_end - session->mergeData) + len + 2 > session->mtu)
		net_flush_merged(ctx, session);
	pkt_write_c(&session->mergeData_end, endof(session->mergeData), session->version, MergedHeader, {
		.length = len,
	});
	pkt_write_bytes(buf, &session->mergeData_end, endof(session->mergeData), session->version, len);
}
