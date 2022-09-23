#include "../net.h"
#include "../ssl.h"
#include "internal.h"
#include <mbedtls/ssl.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

struct Context {
	int32_t listenfd;
	mbedtls_ssl_config conf;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_x509_crt *certs;
	mbedtls_pk_context *keys;
	const char *domain;
	const char *path;
};
static struct Context ctx = {
	.listenfd = -1
};

static void *handle_client(void *fd) {
	mbedtls_ssl_context ssl;
	mbedtls_ssl_init(&ssl);
	int res = mbedtls_ssl_setup(&ssl, &ctx.conf);
	if(res) {
		uprintf("mbedtls_ssl_setup() failed: %s\n", mbedtls_high_level_strerr(res));
		goto reset;
	}
	mbedtls_ssl_set_bio(&ssl, fd, ssl_send, ssl_recv, NULL);
	while((res = mbedtls_ssl_handshake(&ssl)) != 0) {
		if(res == MBEDTLS_ERR_SSL_WANT_READ || res == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;
		uprintf("mbedtls_ssl_handshake() failed: %s\n", mbedtls_high_level_strerr(res));
		goto reset;
	}
	uint8_t buf[65536];
	do {
		memset(buf, 0, sizeof(buf)); // TODO: this doesn't look right
		res = mbedtls_ssl_read(&ssl, buf, sizeof(buf) - 1);
	} while(res == MBEDTLS_ERR_SSL_WANT_READ);
	if(res < 0) {
		uprintf("mbedtls_ssl_read() failed: %s\n", mbedtls_high_level_strerr(res));
	} else {
		size_t len = status_resp("HTTPS", ctx.path, (char*)buf, res);
		if(len) {
			while((res = mbedtls_ssl_write(&ssl, buf, len)) <= 0) { // TODO: len > 16384 support
				if(res == MBEDTLS_ERR_SSL_WANT_WRITE)
					continue;
				if(res != MBEDTLS_ERR_NET_CONN_RESET)
					uprintf("mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(res));
				goto reset;
			}
		}
	}
	do {
		res = mbedtls_ssl_close_notify(&ssl);
	} while(res == MBEDTLS_ERR_SSL_WANT_READ || res == MBEDTLS_ERR_SSL_WANT_WRITE);
	reset:;
	mbedtls_ssl_free(&ssl);
	close((intptr_t)fd);
	return 0;
}

static void *status_ssl_handler(void*) {
	uprintf("Started HTTPS\n");
	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	for(intptr_t clientfd; (clientfd = accept(ctx.listenfd, &addr.sa, &addr.len)) != -1;)
		pthread_create((pthread_t[]){NET_THREAD_INVALID}, NULL, (void *(*)(void*))handle_client, (void*)clientfd);
	return 0;
}

static int status_ssl_sni(struct Context *ctx, mbedtls_ssl_context *ssl, const unsigned char *name, size_t name_len) {
	uint8_t i = (name_len == strlen(ctx->domain) && memcmp(name, ctx->domain, name_len) == 0);
	mbedtls_ssl_set_hs_ca_chain(ssl, ctx->certs[i].next, NULL);
	return mbedtls_ssl_set_hs_own_cert(ssl, &ctx->certs[i], &ctx->keys[i]);
}

static pthread_t status_thread = NET_THREAD_INVALID;
bool status_ssl_init(mbedtls_x509_crt certs[2], mbedtls_pk_context keys[2], const char *domain, const char *path, uint16_t port) {
	ctx.listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	{
		int32_t iSetOption = 1;
		setsockopt(ctx.listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
		struct sockaddr_in6 serv_addr = {
			.sin6_family = AF_INET6,
			.sin6_port = htons(port),
			.sin6_flowinfo = 0,
			.sin6_addr = IN6ADDR_ANY_INIT,
			.sin6_scope_id = 0,
		};
		#ifndef WINDOWS
		signal(SIGPIPE, SIG_IGN);
		#endif
		if(bind(ctx.listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			uprintf("Cannot bind socket to port %hu: %s\n", port, strerror(errno));
			close(ctx.listenfd);
			ctx.listenfd = -1;
			return true;
		}
	}
	if(listen(ctx.listenfd, 128) < 0) {
		close(ctx.listenfd);
		ctx.listenfd = -1;
		return true;
	}
	mbedtls_ssl_config_init(&ctx.conf);
	mbedtls_entropy_init(&ctx.entropy);
	mbedtls_ctr_drbg_init(&ctx.ctr_drbg);
	int res = mbedtls_ctr_drbg_seed(&ctx.ctr_drbg, mbedtls_entropy_func, &ctx.entropy, (const unsigned char*)"ssl_server", 10);
	if(res) {
		uprintf("mbedtls_ctr_drbg_seed() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	char service[8];
	sprintf(service, "%hu", port);
	res = mbedtls_ssl_config_defaults(&ctx.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if(res) {
		uprintf("mbedtls_ssl_config_defaults() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	mbedtls_ssl_conf_rng(&ctx.conf, mbedtls_ctr_drbg_random, &ctx.ctr_drbg);
	mbedtls_ssl_conf_sni(&ctx.conf, (int (*)(void*, mbedtls_ssl_context*, const unsigned char*, size_t))status_ssl_sni, &ctx);
	/*mbedtls_ssl_conf_ca_chain(&ctx.conf, certs[1].MBEDTLS_PRIVATE(next), NULL);
	if((res = mbedtls_ssl_conf_own_cert(&ctx.conf, &certs[1], &keys[1])) != 0) {
		uprintf("mbedtls_ssl_conf_own_cert() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}*/
	ctx.certs = certs;
	ctx.keys = keys;
	ctx.domain = domain;
	ctx.path = path;
	if(pthread_create(&status_thread, NULL, (void *(*)(void*))status_ssl_handler, NULL)) {
		status_thread = NET_THREAD_INVALID;
		return true;
	}
	return false;
}
void status_ssl_cleanup() {
	if(ctx.listenfd == -1)
		return;
	uprintf("Stopping HTTPS\n");
	mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
	mbedtls_entropy_free(&ctx.entropy);
	mbedtls_ssl_config_free(&ctx.conf);
	shutdown(ctx.listenfd, SHUT_RD);
	close(ctx.listenfd);
	ctx.listenfd = -1;
	if(status_thread != NET_THREAD_INVALID) {
		pthread_join(status_thread, NULL);
		status_thread = NET_THREAD_INVALID;
	}
}
