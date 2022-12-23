#pragma once
#include <mbedtls/ssl.h>
#include <stdbool.h>

typedef uintptr_t WireLinkType;
enum { // Take advantage of the first entry in `struct mbedtls_ssl_context` being an aligned pointer
	WireLinkType_INVALID,
	WireLinkType_LOCAL,
	WireLinkType_REMOTE_START,
};

struct WireCookie {
	void *data;
	size_t length;
};

union WireLink;
struct NetContext *WireLink_cast_local(union WireLink *link);
mbedtls_ssl_context *WireLink_cast_remote(union WireLink *link);

struct NetContext;
struct WireMessage;
void wire_set_key(uint8_t key[static 32], uint8_t key_len);
union WireLink *wire_connect_local(struct NetContext *self, struct NetContext *link);
union WireLink *wire_connect_remote(struct NetContext *self, const char *address);
void wire_disconnect(struct NetContext *self, union WireLink *link);
void wire_accept(struct NetContext *self, int32_t listenfd);
bool wire_send(struct NetContext *self, union WireLink *link, const struct WireMessage *message);
void wire_recv(struct NetContext *self, mbedtls_ssl_context *link);
uint32_t wire_reserveCookie(struct NetContext *self, void *data, size_t length);
void *wire_getCookie(struct NetContext *self, uint32_t cookie);
uint32_t wire_nextCookie(struct NetContext *self, uint32_t start);
void wire_releaseCookie(struct NetContext *self, uint32_t cookie);
