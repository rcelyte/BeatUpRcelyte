#include "../net.h"
#include "internal.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#define SHUT_RD SD_RECEIVE
#else
#include <pthread.h>
#endif
#include <mbedtls/ssl.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct Context {
	int32_t listenfd;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	const char *path;
};

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

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
status_ssl_handler(struct Context *ctx) {
	fprintf(stderr, "HTTPS status started\n");
	struct SS addr = {sizeof(struct sockaddr_storage)};
	int32_t clientfd;
	while((clientfd = accept(ctx->listenfd, &addr.sa, &addr.len)) != -1) {
		mbedtls_ssl_set_bio(&ctx->ssl, &clientfd, status_send, status_recv, NULL);
		int ret;
		while((ret = mbedtls_ssl_handshake(&ctx->ssl)) != 0)
			if(ret != STATUS_ERR_INTR)
				goto reset;
		uint8_t buf[65536];
		do {
			memset(buf, 0, sizeof(buf));
			ret = mbedtls_ssl_read(&ctx->ssl, buf, sizeof(buf) - 1);
		} while(ret == STATUS_ERR_INTR);
		if(ret < 0) {
			fprintf(stderr, "mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(ret));
		} else {
			size_t len = status_resp("HTTPS", ctx->path, (char*)buf, ret);
			if(len) {
				while((ret = mbedtls_ssl_write(&ctx->ssl, buf, len)) <= 0) {
					if(ret == STATUS_ERR_RESET)
						goto reset;
					if(ret != STATUS_ERR_INTR) {
						fprintf(stderr, "mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(ret));
						close(clientfd);
						return 0;
					}
				}
			}
		}
		while((ret = mbedtls_ssl_close_notify(&ctx->ssl)) < 0)
			if(ret != STATUS_ERR_INTR)
				break;
		reset:;
		close(clientfd);
		mbedtls_ssl_session_reset(&ctx->ssl);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE status_thread = NULL;
#else
static pthread_t status_thread = 0;
#endif
static struct Context ctx = {-1};
_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port) {
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
		if(bind(ctx.listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			fprintf(stderr, "Cannot bind socket to port %hu: %s\n", port, strerror(errno));
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
	if(ctx.listenfd != -1) {
		fprintf(stderr, "Stopping HTTPS status\n");
		mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
		mbedtls_entropy_free(&ctx.entropy);
		mbedtls_ssl_config_free(&ctx.conf);
		mbedtls_ssl_free(&ctx.ssl);
		shutdown(ctx.listenfd, SHUT_RD);
		close(ctx.listenfd);
		ctx.listenfd = -1;
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
