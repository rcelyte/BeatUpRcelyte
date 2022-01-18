#include "../net.h"
#include "status_internal.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include <mbedtls/net_sockets.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <string.h>

struct Context {
	mbedtls_net_context listenfd;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	const char *path;
};

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
status_ssl_handler(struct Context *ctx) {
	fprintf(stderr, "HTTPS status started\n");
	mbedtls_net_context clientfd;
	while(mbedtls_net_accept(&ctx->listenfd, &clientfd, NULL, 0, NULL) == 0) {
		mbedtls_ssl_set_bio(&ctx->ssl, &clientfd, mbedtls_net_send, mbedtls_net_recv, NULL);
		int ret;
		while((ret = mbedtls_ssl_handshake(&ctx->ssl)) != 0)
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				goto reset;
		uint8_t buf[65536];
		do {
			memset(buf, 0, sizeof(buf));
			ret = mbedtls_ssl_read(&ctx->ssl, buf, sizeof(buf) - 1);
		} while(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
		if(ret < 0) {
			fprintf(stderr, "mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(ret));
		} else {
			size_t len = status_resp("HTTPS", ctx->path, (char*)buf, ret);
			if(len) {
				while((ret = mbedtls_ssl_write(&ctx->ssl, buf, len)) <= 0) {
					if(ret == MBEDTLS_ERR_NET_CONN_RESET)
						goto reset;
					if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
						fprintf(stderr, "mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(ret));
						mbedtls_net_free(&clientfd);
						return 0;
					}
				}
			}
		}
		while((ret = mbedtls_ssl_close_notify(&ctx->ssl)) < 0)
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				break;
		reset:;
		mbedtls_net_free(&clientfd);
		mbedtls_ssl_session_reset(&ctx->ssl);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE status_thread = NULL;
#else
static pthread_t status_thread = 0;
#endif
static struct Context ctx;
_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port) {
	mbedtls_net_init(&ctx.listenfd);
	mbedtls_ssl_init(&ctx.ssl);
	mbedtls_ssl_config_init(&ctx.conf);
	mbedtls_entropy_init(&ctx.entropy);
	mbedtls_ctr_drbg_init(&ctx.ctr_drbg);
	int ret = mbedtls_ctr_drbg_seed(&ctx.ctr_drbg, mbedtls_entropy_func, &ctx.entropy, (const unsigned char*)"ssl_server", 10);
	if(ret != 0) {
		fprintf(stderr, "mbedtls_ctr_drbg_seed() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	char service[8];
	sprintf(service, "%hu", port);
	if((ret = mbedtls_net_bind(&ctx.listenfd, NULL, service, MBEDTLS_NET_PROTO_TCP)) != 0) {
		fprintf(stderr, "mbedtls_net_bind() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	if((ret = mbedtls_ssl_config_defaults(&ctx.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		fprintf(stderr, "mbedtls_ssl_config_defaults() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	mbedtls_ssl_conf_rng(&ctx.conf, mbedtls_ctr_drbg_random, &ctx.ctr_drbg);
	mbedtls_ssl_conf_ca_chain(&ctx.conf, cert->MBEDTLS_PRIVATE(next), NULL);
	if((ret = mbedtls_ssl_conf_own_cert(&ctx.conf, cert, key)) != 0) {
		fprintf(stderr, "mbedtls_ssl_conf_own_cert() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	if((ret = mbedtls_ssl_setup(&ctx.ssl, &ctx.conf)) != 0) {
		fprintf(stderr, "mbedtls_ssl_setup() failed: %s\n", mbedtls_high_level_strerr(ret));
		return 1;
	}
	ctx.path = path;
	#ifdef WINDOWS
	status_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)status_ssl_handler, &ctx, 0, NULL);
	return !status_thread;
	#else
	return pthread_create(&status_thread, NULL, (void*(*)(void*))status_ssl_handler, &ctx) != 0;
	#endif
}
void status_ssl_cleanup() {
	if(ctx.listenfd.MBEDTLS_PRIVATE(fd) != -1) {
		fprintf(stderr, "Stopping HTTPS status\n");
		mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
		mbedtls_entropy_free(&ctx.entropy);
		mbedtls_ssl_config_free(&ctx.conf);
		mbedtls_ssl_free(&ctx.ssl);
		mbedtls_net_free(&ctx.listenfd);
		if(status_thread) {
			#ifdef WINDOWS
			WaitForSingleObject(status_thread, INFINITE);
			#else
			pthread_join(status_thread, NULL);
			#endif
			status_thread = 0;
		}
	}
}