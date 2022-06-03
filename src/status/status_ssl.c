#include "../net.h"
#include "../thread.h"
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
} static ctx = {-1};

#define STATUS_ERR_RESET -2
#define STATUS_ERR_INTR -3

static int status_send(void *fd, const unsigned char *buf, size_t len) {
	ssize_t ret = send(*(int32_t*)fd, (char*)buf, len, 0);
	if(ret >= 0)
		return ret;
	#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && !defined(EFI32)
	if(WSAGetLastError() == WSAECONNRESET)
		return STATUS_ERR_RESET;
	#else
	if(errno == EPIPE || errno == ECONNRESET)
		return STATUS_ERR_RESET;
	if(errno == EINTR)
		return STATUS_ERR_INTR;
	#endif
	return -1;
}

static int status_recv(void *fd, unsigned char *buf, size_t len) {
	ssize_t ret = recv(*(int32_t*)fd, (char*)buf, len, 0);
	if(ret >= 0)
		return ret;
	#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && !defined(EFI32)
	if(WSAGetLastError() == WSAECONNRESET)
		return STATUS_ERR_RESET;
	#else
	if(errno == EPIPE || errno == ECONNRESET)
		return STATUS_ERR_RESET;
	if(errno == EINTR)
		return STATUS_ERR_INTR;
	#endif
	return -1;
}

static thread_return_t handle_client(intptr_t fd) {
	mbedtls_ssl_context ssl;
	mbedtls_ssl_init(&ssl);
	int ret = mbedtls_ssl_setup(&ssl, &ctx.conf);
	if(ret != 0) {
		uprintf("mbedtls_ssl_setup() failed: %s\n", mbedtls_high_level_strerr(ret));
		goto reset;
	}
	mbedtls_ssl_set_bio(&ssl, &fd, status_send, status_recv, NULL);
	while((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
		if(ret == STATUS_ERR_INTR)
			continue;
		uprintf("mbedtls_ssl_handshake() failed: %s\n", mbedtls_high_level_strerr(ret));
		goto reset;
	}
	uint8_t buf[65536];
	do {
		memset(buf, 0, sizeof(buf));
		ret = mbedtls_ssl_read(&ssl, buf, sizeof(buf) - 1);
	} while(ret == STATUS_ERR_INTR);
	if(ret < 0) {
		uprintf("mbedtls_ssl_read() failed: %s\n", mbedtls_high_level_strerr(ret));
	} else {
		size_t len = status_resp("HTTPS", ctx.path, (char*)buf, ret);
		if(len) {
			while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
				if(ret == STATUS_ERR_INTR)
					continue;
				if(ret != STATUS_ERR_RESET)
					uprintf("mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(ret));
				goto reset;
			}
		}
	}
	while((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
		if(ret != STATUS_ERR_INTR)
			break;
	reset:;
	mbedtls_ssl_free(&ssl);
	close(fd);
	return 0;
}

static thread_return_t status_ssl_handler(void *userptr) {
	uprintf("Started HTTPS\n");
	struct SS addr = {sizeof(struct sockaddr_storage)};
	for(intptr_t clientfd; (clientfd = accept(ctx.listenfd, &addr.sa, &addr.len)) != -1;)
		thread_create((thread_t[]){0}, handle_client, (void*)clientfd);
	return 0;
}

static int sni_cb(struct Context *ctx, mbedtls_ssl_context *ssl, const unsigned char *name, size_t name_len) {
	uint8_t i = (name_len == strlen(ctx->domain) && memcmp(name, ctx->domain, name_len) == 0);
	mbedtls_ssl_set_hs_ca_chain(ssl, ctx->certs[i].MBEDTLS_PRIVATE(next), NULL);
	return mbedtls_ssl_set_hs_own_cert(ssl, &ctx->certs[i], &ctx->keys[i]);
}

static thread_t status_thread = 0;
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
			return 1;
		}
	}
	if(listen(ctx.listenfd, 128) < 0) {
		close(ctx.listenfd);
		ctx.listenfd = -1;
		return 1;
	}
	mbedtls_ssl_config_init(&ctx.conf);
	mbedtls_entropy_init(&ctx.entropy);
	mbedtls_ctr_drbg_init(&ctx.ctr_drbg);
	int ret = mbedtls_ctr_drbg_seed(&ctx.ctr_drbg, mbedtls_entropy_func, &ctx.entropy, (const unsigned char*)"ssl_server", 10);
	if(ret != 0) {
		uprintf("mbedtls_ctr_drbg_seed() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	char service[8];
	sprintf(service, "%hu", port);
	if((ret = mbedtls_ssl_config_defaults(&ctx.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		uprintf("mbedtls_ssl_config_defaults() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	mbedtls_ssl_conf_rng(&ctx.conf, mbedtls_ctr_drbg_random, &ctx.ctr_drbg);
	mbedtls_ssl_conf_sni(&ctx.conf, (int (*)(void*, mbedtls_ssl_context*, const unsigned char*, size_t))sni_cb, &ctx);
	/*mbedtls_ssl_conf_ca_chain(&ctx.conf, certs[1].MBEDTLS_PRIVATE(next), NULL);
	if((ret = mbedtls_ssl_conf_own_cert(&ctx.conf, &certs[1], &keys[1])) != 0) {
		uprintf("mbedtls_ssl_conf_own_cert() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}*/
	ctx.certs = certs;
	ctx.keys = keys;
	ctx.domain = domain;
	ctx.path = path;
	return thread_create(&status_thread, status_ssl_handler, NULL);
}
void status_ssl_cleanup() {
	if(ctx.listenfd != -1) {
		uprintf("Stopping HTTPS\n");
		mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
		mbedtls_entropy_free(&ctx.entropy);
		mbedtls_ssl_config_free(&ctx.conf);
		shutdown(ctx.listenfd, SHUT_RD);
		close(ctx.listenfd);
		ctx.listenfd = -1;
		if(status_thread) {
			thread_join(status_thread);
			status_thread = 0;
		}
	}
}
