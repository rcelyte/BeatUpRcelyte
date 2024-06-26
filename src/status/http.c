#include "http.h"
#include "../global.h"
#include <mbedtls/error.h>
#include <stdlib.h>
#include <string.h>
#ifdef WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <errno.h>
#endif

#ifndef MBEDTLS_ERR_NET_CONN_RESET
#define MBEDTLS_ERR_NET_CONN_RESET -0x0050
#endif

static inline int ssl_error(int intr) {
	#ifdef WINDOWS
	(void)intr;
	if(WSAGetLastError() == WSAECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	#else
	if(errno == EPIPE || errno == ECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	if(errno == EINTR)
		return intr;
	#endif
	return -1;
}

static int ssl_send_internal(void *fd, const uint8_t *data, const size_t data_len) {
	#ifdef WINDOWS
	const ssize_t ret = send((NetSocket)(uintptr_t)fd, (const char*)data, (int)(unsigned)data_len, 0);
	#else
	const ssize_t ret = send((NetSocket)(uintptr_t)fd, (const char*)data, data_len, 0);
	#endif
	if(ret >= 0)
		return (int)ret;
	return ssl_error(MBEDTLS_ERR_SSL_WANT_WRITE);
}

static int ssl_recv_internal(void *fd, uint8_t *data, const size_t data_len) {
	#ifdef WINDOWS
	const ssize_t ret = recv((NetSocket)(uintptr_t)fd, (char*)data, (int)(unsigned)data_len, 0);
	#else
	const ssize_t ret = recv((NetSocket)(uintptr_t)fd, (char*)data, data_len, 0);
	#endif
	if(ret >= 0)
		return (int)ret;
	return ssl_error(MBEDTLS_ERR_SSL_WANT_READ);
}

bool HttpContext_init(struct HttpContext *const self, const NetSocket fd, mbedtls_ssl_config *const sslConfig, const bool quiet) {
	*self = (struct HttpContext){
		.encrypt = (sslConfig != NULL),
		.quiet = quiet,
	};
	if(!self->encrypt) {
		self->fd = (void*)(uintptr_t)fd;
		return false;
	}
	mbedtls_ssl_init(&self->ssl);
	mbedtls_ssl_conf_max_tls_version(sslConfig, MBEDTLS_SSL_VERSION_TLS1_2); // https://github.com/Mbed-TLS/mbedtls/issues/9223
	int res = mbedtls_ssl_setup(&self->ssl, sslConfig);
	if(res) {
		uprintf("mbedtls_ssl_setup() failed: %s\n", mbedtls_high_level_strerr(res));
		goto fail0;
	}
	mbedtls_ssl_set_bio(&self->ssl, (void*)(uintptr_t)fd, ssl_send_internal, ssl_recv_internal, NULL);
	do {
		res = mbedtls_ssl_handshake(&self->ssl);
		if(res == 0)
			return false;
	} while(res == MBEDTLS_ERR_SSL_WANT_READ || res == MBEDTLS_ERR_SSL_WANT_WRITE);
	if(!self->quiet || res != MBEDTLS_ERR_SSL_CONN_EOF)
		uprintf("mbedtls_ssl_handshake() failed: %s\n", mbedtls_high_level_strerr(res));
	fail0: mbedtls_ssl_free(&self->ssl);
	return true;
}

void HttpContext_cleanup(struct HttpContext *self) {
	if(!self->encrypt)
		return;
	int res;
	do {
		res = mbedtls_ssl_close_notify(&self->ssl);
	} while(res == MBEDTLS_ERR_SSL_WANT_READ || res == MBEDTLS_ERR_SSL_WANT_WRITE);
	mbedtls_ssl_free(&self->ssl);
	*self = (struct HttpContext){0};
}

static bool ReadUntil(struct HttpContext *const self, uint8_t **head, const uint8_t *const end, const uint8_t *const target) {
	if(target > end)
		return true;
	while(*head < target) {
		size_t limit = (size_t)(end - *head);
		if(limit > sizeof(self->overflow))
			limit = sizeof(self->overflow);
		const int res = self->encrypt ? mbedtls_ssl_read(&self->ssl, *head, limit) : ssl_recv_internal(self->fd, *head, limit);
		if(res > 0)
			*head += res;
		else if(res != MBEDTLS_ERR_SSL_WANT_READ)
			return true;
	}
	return false;
}

struct HttpRequest HttpContext_recieve(struct HttpContext *self, uint8_t buffer[], size_t buffer_len) {
	if(buffer_len <= self->overflow_len)
		return (struct HttpRequest){0};
	uint8_t *head = buffer, *data_end = buffer, *const end = &buffer[buffer_len - 1]; // leave byte for null terminator
	memcpy(head, self->overflow, self->overflow_len);
	head += self->overflow_len;
	while(true) {
		if(ReadUntil(self, &head, end, &data_end[4]))
			return (struct HttpRequest){0};
		data_end = memchr(data_end, '\r', (size_t)(head - 3 - data_end));
		if(data_end == NULL) {
			data_end = head - 3;
		} else if(memcmp(++data_end, "\n\r\n", 3) == 0) {
			data_end += 3;
			break;
		}
	}
	struct HttpRequest request = {
		.header_len = (size_t)(data_end - buffer),
		.header = (char*)buffer,
	};
	*head = 0;
	const uint8_t *contentLength = (uint8_t*)strstr(request.header, "\r\nContent-Length: ");
	if(contentLength != NULL)
		for(const uint8_t *it = &contentLength[18]; it < end && *it >= '0' && *it <= '9'; ++it)
			request.body_len = request.body_len * 10 + (*it - '0');
	if(request.body_len) {
		request.body = data_end;
		data_end += request.body_len;
		if(ReadUntil(self, &head, end, data_end))
			return (struct HttpRequest){0};
	}
	self->overflow_len = (uint16_t)(head - data_end);
	memcpy(self->overflow, data_end, self->overflow_len);
	return request;
}

static bool SendData(struct HttpContext *self, const uint8_t *data, size_t data_len) {
	for(const uint8_t *const end = &data[data_len]; data < end;) {
		const int res = self->encrypt ? mbedtls_ssl_write(&self->ssl, data, (size_t)(end - data)) : ssl_send_internal(self->fd, data, (size_t)(end - data));
		if(res > 0)
			data += res;
		else if(res != MBEDTLS_ERR_SSL_WANT_WRITE)
			return true;
	}
	return false;
}

void HttpContext_respond(struct HttpContext *const self, const uint16_t code, const char mime[], const void *const data, const size_t data_len) {
	const char *codeText;
	switch(code) {
		case 200: codeText = "200 OK"; break;
		case 404: codeText = "404 Not Found"; break;
		case 500: codeText = "500 Internal Server Error"; break;
		default: uprintf("unexpected HTTP response code: %hu\n", code); abort();
	}
	char header[0x400] = {0};
	const uint32_t header_len = (uint32_t)snprintf(header, lengthof(header), "%s%s%s%zu%s%s%s",
		"HTTP/1.1 ", codeText, "\r\n"
		"Connection: close\r\n"
		"Content-Length: ", data_len, "\r\n"
		"Content-Type: ", mime, "\r\n"
		"Cross-Origin-Embedder-Policy: require-corp\r\n"
		"Cross-Origin-Opener-Policy: same-origin\r\n"
		"Server: beatupproto\r\n"
		"X-Frame-Options: DENY\r\n"
		"X-Content-Type-Options: nosniff\r\n"
		"X-DNS-Prefetch-Control: off\r\n"
		"X-Robots-Tag: noindex\r\n"
		"\r\n");
	if(!SendData(self, (const uint8_t*)header, header_len))
		SendData(self, data, data_len);
}
