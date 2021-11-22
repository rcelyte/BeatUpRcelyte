#include "net.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <string.h>

struct Data {
	mbedtls_net_context listenfd;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
};

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
status_ssl_handler(struct Data *data) {
	mbedtls_net_context clientfd;
	mbedtls_net_init(&clientfd);
	mbedtls_net_free(&clientfd);
	while(mbedtls_net_accept(&data->listenfd, &clientfd, NULL, 0, NULL) == 0) {
		mbedtls_ssl_set_bio(&data->ssl, &clientfd, mbedtls_net_send, mbedtls_net_recv, NULL);
		int ret;
		while((ret = mbedtls_ssl_handshake(&data->ssl)) != 0)
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				goto reset;
		uint8_t buf[1024];
		do {
			memset(buf, 0, sizeof(buf));
			ret = mbedtls_ssl_read(&data->ssl, buf, sizeof(buf) - 1);
		} while(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
		if(ret >= 6 && strncmp((char*)buf, "GET /", 5) == 0) {
			fprintf(stderr, "[HTTPS] GET /\n");
			size_t len;
			if(buf[5] == ' ') {
				len = sprintf((char*)buf,
					"HTTP/1.1 200 OK\r\n"
					"Connection: close\r\n"
					"Content-Length: 182\r\n"
					"X-Frame-Options: DENY\r\n"
					"X-Content-Type-Options: nosniff\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"X-DNS-Prefetch-Control: off\r\n"
					"\r\n"
					"{\"minimumAppVersion\":\"1.16.4\",\"status\":0,\"maintenanceStartTime\":0,\"maintenanceEndTime\":0,\"userMessage\":{\"localizedMessages\":[{\"language\":\"en\",\"message\":\"Test message from server\"}]}}");
			} else {
				len = sprintf((char*)buf,
					"HTTP/1.1 404 Not Found\r\n"
					"Connection: close\r\n"
					"Content-Length: 39\r\n"
					"X-Frame-Options: DENY\r\n"
					"X-Content-Type-Options: nosniff\r\n"
					"Content-Type: text/html; charset=utf-8\r\n"
					"X-DNS-Prefetch-Control: off\r\n"
					"\r\n"
					"<html><body>404 not found</body></html>");
			}
			while((ret = mbedtls_ssl_write(&data->ssl, buf, len)) <= 0) {
				if(ret == MBEDTLS_ERR_NET_CONN_RESET)
					goto reset;
				if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
					fprintf(stderr, "mbedtls_ssl_write() returned %d\n", ret);
					mbedtls_net_free(&clientfd);
					return 0;
				}
			}
		}
		while((ret = mbedtls_ssl_close_notify(&data->ssl)) < 0)
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				break;
		reset:;
		mbedtls_net_free(&clientfd);
		mbedtls_ssl_session_reset(&data->ssl);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE status_thread;
#else
static pthread_t status_thread;
#endif
static struct Data data;
_Bool status_ssl_init(mbedtls_x509_crt srvcert, mbedtls_pk_context pkey) {
	mbedtls_net_init(&data.listenfd);
	mbedtls_ssl_init(&data.ssl);
	mbedtls_ssl_config_init(&data.conf);
	mbedtls_entropy_init(&data.entropy);
	mbedtls_ctr_drbg_init(&data.ctr_drbg);
	int ret = mbedtls_ctr_drbg_seed(&data.ctr_drbg, mbedtls_entropy_func, &data.entropy, (const unsigned char*)"ssl_server", 10);
	if(ret != 0) {
		fprintf(stderr, "mbedtls_ctr_drbg_seed() returned %d\n", ret);
		return 1;
	}
	if((ret = mbedtls_net_bind(&data.listenfd, NULL, "443", MBEDTLS_NET_PROTO_TCP)) != 0) {
		fprintf(stderr, "mbedtls_net_bind() returned %d\n", ret);
		return 1;
	}
	if((ret = mbedtls_ssl_config_defaults(&data.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		fprintf(stderr, "mbedtls_ssl_config_defaults() returned %d\n", ret);
		return 1;
	}
	mbedtls_ssl_conf_rng(&data.conf, mbedtls_ctr_drbg_random, &data.ctr_drbg);
	mbedtls_ssl_conf_ca_chain(&data.conf, srvcert.MBEDTLS_PRIVATE(next), NULL);
	if((ret = mbedtls_ssl_conf_own_cert(&data.conf, &srvcert, &pkey)) != 0) {
		fprintf(stderr, "mbedtls_ssl_conf_own_cert() returned %d\n", ret);
		return 1;
	}
	if((ret = mbedtls_ssl_setup(&data.ssl, &data.conf)) != 0) {
		fprintf(stderr, "mbedtls_ssl_setup() returned %d\n", ret);
		return 1;
	}
	#ifdef WINDOWS
	status_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)status_ssl_handler, &data, 0, NULL);
	return !status_thread;
	#else
	return pthread_create(&status_thread, NULL, (void*)&status_ssl_handler, &data) != 0;
	#endif
}
void status_ssl_cleanup() {
	if(data.listenfd.MBEDTLS_PRIVATE(fd) != -1) {
		mbedtls_ctr_drbg_free(&data.ctr_drbg);
		mbedtls_entropy_free(&data.entropy);
		mbedtls_ssl_config_free(&data.conf);
		mbedtls_ssl_free(&data.ssl);
		mbedtls_net_free(&data.listenfd);
		#ifdef WINDOWS
		WaitForSingleObject(status_thread, INFINITE);
		#else
		pthread_join(status_thread, NULL);
		#endif
	}
}