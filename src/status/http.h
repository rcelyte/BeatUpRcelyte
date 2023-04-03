#pragma once
#include <mbedtls/ssl.h>
#include <stdbool.h>

struct HttpContext {
	bool encrypt;
	union {
		void *fd;
		mbedtls_ssl_context ssl;
	};
	uint16_t overflow_len;
	uint8_t overflow[8192];
};

struct HttpRequest {
	size_t header_len;
	char *header;
	size_t body_len;
	uint8_t *body;
};

bool HttpContext_init(struct HttpContext *self, int fd, mbedtls_ssl_config *sslConfig);
void HttpContext_cleanup(struct HttpContext *self);
struct HttpRequest HttpContext_recieve(struct HttpContext *self, uint8_t buffer[], size_t buffer_len);
void HttpContext_respond(struct HttpContext *self, uint16_t code, const char *mime, const void *data, size_t length);
