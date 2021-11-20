#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

static _Bool sslstatus_init(mbedtls_x509_crt srvcert, mbedtls_pk_context pkey) {
	mbedtls_net_context listen_fd, client_fd;
	mbedtls_net_init(&listen_fd);
	mbedtls_net_init(&client_fd);
	mbedtls_ssl_context ssl;
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config conf;
	mbedtls_ssl_config_init(&conf);
	mbedtls_entropy_context entropy;
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)"ssl_server", 10);
	if(ret != 0) {
		fprintf(stderr, "mbedtls_ctr_drbg_seed() returned %d\n", ret);
		return 1;
	}
	if((ret = mbedtls_net_bind(&listen_fd, NULL, "443", MBEDTLS_NET_PROTO_TCP)) != 0) {
		fprintf(stderr, "mbedtls_net_bind() returned %d\n", ret);
		return 1;
	}
	if((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		fprintf(stderr, "mbedtls_ssl_config_defaults() returned %d\n", ret);
		return 1;
	}
	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
	if((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0) {
		fprintf(stderr, "mbedtls_ssl_conf_own_cert() returned %d\n", ret);
		return 1;
	}
	if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
		fprintf(stderr, "mbedtls_ssl_setup() returned %d\n", ret);
		return 1;
	}

	while(mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL) == 0) {
		mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
		while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				goto reset;
		uint8_t buf[1024];
		do {
			memset(buf, 0, sizeof(buf));
			ret = mbedtls_ssl_read(&ssl, buf, sizeof(buf) - 1);
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
			while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
				if(ret == MBEDTLS_ERR_NET_CONN_RESET)
					goto reset;
				if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
					fprintf(stderr, "mbedtls_ssl_write() returned %d\n", ret);
					return 1;
				}
			}
		}
		while((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				break;

		reset:;
		mbedtls_net_free(&client_fd);
		mbedtls_ssl_session_reset(&ssl);
	}
	return 0;
}
static void sslstatus_cleanup() {}