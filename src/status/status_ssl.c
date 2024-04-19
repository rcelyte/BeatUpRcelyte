#include "status.h"
#include "../net.h"
#include "internal.h"
#include <mbedtls/ssl.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <unistd.h>
#include <string.h>

struct StatusContext {
	NetSocket listenfd;
	void *(*handleClient)(void*);
	const char *path;
	mbedtls_ssl_config conf;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_x509_crt *certs;
	mbedtls_pk_context *keys;
	const char *domain;
	pthread_mutex_t mutex;
	struct WireContext wire;
	struct WireLink *master;
	bool quiet;
};
static struct StatusContext ctx = {
	.listenfd = NetSocket_Invalid,
};

static void status_onWireMessage(struct WireContext *wire, struct WireLink *link, const struct WireMessage *message) {
	struct StatusContext *const ctx = (struct StatusContext*)wire->userptr;
	pthread_mutex_lock(&ctx->mutex);
	if(link != ctx->master)
		goto unlock;
	if(message == NULL) {
		for(WireCookie cookie = 1; cookie <= WireLink_lastCookieIndex(link); ++cookie) {
			const struct DataView cookieView = WireLink_getCookie(link, cookie);
			if(cookieView.length == sizeof(struct GraphConnectCookie) && ((struct GraphConnectCookie*)cookieView.data)->cookieType == StatusCookieType_GraphConnect)
				status_graph_resp(cookieView, NULL);
			WireLink_freeCookie(link, cookie);
		}
		ctx->master = NULL;
		goto unlock;
	}
	const uint8_t *entry = NULL;
	uint32_t entry_len = 0;
	switch(message->type) {
		case WireMessageType_WireGraphConnectResp: {
			status_graph_resp(WireLink_getCookie(link, message->cookie), &message->graphConnectResp);
			WireLink_freeCookie(link, message->cookie);
		} break;
		case WireMessageType_WireRoomStatusNotify: entry = message->roomStatusNotify.entry; entry_len = message->roomStatusNotify.entry_len; [[fallthrough]];
		case WireMessageType_WireRoomCloseNotify: status_update_index(message->cookie, entry, entry_len); break;
		default: uprintf("Unhandled wire message [%s]\n", reflect(WireMessageType, message->type));
	}
	unlock: pthread_mutex_unlock(&ctx->mutex);
}

static void handle_client(const NetSocket fd, mbedtls_ssl_config *const config) {
	struct HttpContext http;
	if(HttpContext_init(&http, fd, config, ctx.quiet))
		goto fail0;
	const struct HttpRequest req = HttpContext_recieve(&http, (uint8_t[65536]){0}, 65536);
	if(req.header_len)
		status_resp(&http, ctx.path, req, ctx.master);
	// TODO: wait for `status_graph_resp()` to finish asynchronously
	HttpContext_cleanup(&http);
	fail0:
	#ifdef WINDOWS
	closesocket(fd);
	#else
	close(fd);
	#endif
}

static void *handle_client_http(void *fd) {
	handle_client((NetSocket)(uintptr_t)fd, NULL);
	return NULL;
}

static void *handle_client_https(void *fd) {
	handle_client((NetSocket)(uintptr_t)fd, &ctx.conf);
	return NULL;
}

static NetSocket status_accept(struct StatusContext *ctx, struct SS *addr_out) {
	const NetSocket listenfd = ctx->listenfd;
	pthread_mutex_unlock(&ctx->mutex);
	addr_out->len = sizeof(struct sockaddr_storage);
	const NetSocket clientfd = accept(listenfd, &addr_out->sa, &addr_out->len);
	pthread_mutex_lock(&ctx->mutex);
	return clientfd;
}

static struct {
	bool isRemote;
	union {
		struct WireContext *local;
		const char *remote;
	};
} status_master = {0};
static void *status_handler(struct StatusContext *ctx) {
	pthread_attr_t attr;
	if(pthread_attr_init(&attr)) {
		uprintf("pthread_attr_init() failed\n");
		return NULL;
	}
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_mutex_lock(&ctx->mutex);
	{
		const struct WireMessage connectMessage = {
			.type = WireMessageType_WireStatusAttach,
		};
		if(status_master.isRemote) {
			uprintf("TODO: async wire support\n");
			// ctx->master = WireContext_connect(&ctx->wire, status_master.remote, &connectMessage);
		} else if(status_master.local != NULL) {
			ctx->master = WireContext_attach(&ctx->wire, status_master.local, &connectMessage);
		}
	}
	if(ctx->master == NULL) {
		uprintf("WireContext_connect() failed\n");
		goto unlock;
	}
	uprintf("Started HTTP%s\n", (ctx->handleClient == handle_client_http) ? "" : "S");
	struct SS addr;
	for(NetSocket clientfd; (clientfd = status_accept(ctx, &addr)) != NetSocket_Invalid;) {
		// TODO: shared poll thread (current system is NOT threadsafe); non-blocking I/O
		if(pthread_create((pthread_t[]){NET_THREAD_INVALID}, &attr, ctx->handleClient, (void*)(uintptr_t)clientfd))
			uprintf("pthread_create() failed\n");
	}
	WireLink_free(ctx->master);
	ctx->master = NULL;
	unlock: pthread_mutex_unlock(&ctx->mutex);
	if(pthread_attr_destroy(&attr))
		uprintf("pthread_attr_destroy() failed\n");
	return NULL;
}

static int status_ssl_sni(struct StatusContext *ctx, mbedtls_ssl_context *ssl, const unsigned char *name, size_t name_len) {
	uint8_t i = (name_len == strlen(ctx->domain) && memcmp(name, ctx->domain, name_len) == 0);
	mbedtls_ssl_set_hs_ca_chain(ssl, ctx->certs[i].next, NULL);
	return mbedtls_ssl_set_hs_own_cert(ssl, &ctx->certs[i], &ctx->keys[i]);
}

static pthread_t status_thread = NET_THREAD_INVALID;
bool status_ssl_init(const char *path, uint16_t port, mbedtls_x509_crt certs[2], mbedtls_pk_context keys[2], const char *domain, const char *remoteMaster, struct WireContext *const localMaster, const bool quiet) {
	status_master.isRemote = (*remoteMaster != 0);
	if(status_master.isRemote)
		status_master.remote = remoteMaster;
	else
		status_master.local = localMaster;
	ctx = (struct StatusContext){
		.listenfd = status_bind_tcp(port, 128),
		.handleClient = handle_client_http,
		.path = path,
		.quiet = quiet,
	};
	if(ctx.listenfd == NetSocket_Invalid)
		return true;
	if(mbedtls_pk_get_type(&keys[1]) != MBEDTLS_PK_NONE) {
		mbedtls_ssl_config_init(&ctx.conf);
		mbedtls_entropy_init(&ctx.entropy);
		mbedtls_ctr_drbg_init(&ctx.ctr_drbg);
		int res = mbedtls_ctr_drbg_seed(&ctx.ctr_drbg, mbedtls_entropy_func, &ctx.entropy, (const unsigned char*)"ssl_server", 10);
		if(res) {
			uprintf("mbedtls_ctr_drbg_seed() failed: %s\n", mbedtls_high_level_strerr(res));
			goto fail0;
		}
		res = mbedtls_ssl_config_defaults(&ctx.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
		if(res) {
			uprintf("mbedtls_ssl_config_defaults() failed: %s\n", mbedtls_high_level_strerr(res));
			goto fail0;
		}
		mbedtls_ssl_conf_rng(&ctx.conf, mbedtls_ctr_drbg_random, &ctx.ctr_drbg);
		mbedtls_ssl_conf_sni(&ctx.conf, (int (*)(void*, mbedtls_ssl_context*, const unsigned char*, size_t))status_ssl_sni, &ctx);
		/*mbedtls_ssl_conf_ca_chain(&ctx.conf, certs[1].MBEDTLS_PRIVATE(next), NULL);
		if((res = mbedtls_ssl_conf_own_cert(&ctx.conf, &certs[1], &keys[1])) != 0) {
			uprintf("mbedtls_ssl_conf_own_cert() failed: %s\n", mbedtls_high_level_strerr(res));
			goto fail0;
		}*/
		ctx.certs = certs;
		ctx.keys = keys;
		ctx.domain = domain;
		ctx.handleClient = handle_client_https;
	}
	pthread_mutexattr_t mutexAttribs;
	if(pthread_mutexattr_init(&mutexAttribs) ||
	   pthread_mutexattr_settype(&mutexAttribs, PTHREAD_MUTEX_RECURSIVE) ||
	   pthread_mutex_init(&ctx.mutex, &mutexAttribs)) {
		uprintf("pthread_mutex_init() failed\n");
		goto fail0;
	}
	if(WireContext_init(&ctx.wire, &ctx, 0)) {
		uprintf("WireContext_init() failed\n");
		goto fail1;
	}
	ctx.wire.onMessage = status_onWireMessage;
	if(pthread_create(&status_thread, NULL, (void *(*)(void*))status_handler, &ctx)) {
		status_thread = NET_THREAD_INVALID;
		goto fail2;
	}
	return false;
	fail2: WireContext_cleanup(&ctx.wire);
	fail1: pthread_mutex_destroy(&ctx.mutex);
	fail0:
	net_close(ctx.listenfd);
	ctx.listenfd = NetSocket_Invalid;
	return true;
}

void status_ssl_cleanup() {
	if(ctx.listenfd == NetSocket_Invalid)
		return;
	uprintf("Stopping HTTPS\n");
	shutdown(ctx.listenfd, SHUT_RD);
	if(status_thread != NET_THREAD_INVALID) {
		pthread_join(status_thread, NULL); // TODO: ensure all active connections are closed as well
		status_thread = NET_THREAD_INVALID;
	}
	net_close(ctx.listenfd);
	ctx.listenfd = NetSocket_Invalid;
	WireContext_cleanup(&ctx.wire);
	pthread_mutex_destroy(&ctx.mutex);
	if(ctx.handleClient == handle_client_https) {
		mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
		mbedtls_entropy_free(&ctx.entropy);
		mbedtls_ssl_config_free(&ctx.conf);
	}
}
