#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint32_t WireCookie;

struct WireLink;
struct WireMessage;
struct WireContext {
	void *userptr;
	void (*onMessage)(struct WireContext *ctx, struct WireLink *link, const struct WireMessage *message);
};

struct DataView {
	void *data;
	size_t length;
};

void wire_init(uint8_t remoteKey[32], uint8_t remoteKey_len);
void wire_cleanup();

bool WireContext_init(struct WireContext *self, void *userptr, uint32_t tcpBacklog);
void WireContext_cleanup(struct WireContext *self);
struct WireLink *WireContext_attach(struct WireContext *self, struct WireContext *peer, const struct WireMessage *message);
struct WireLink *WireContext_connect(struct WireContext *self, const char address[], const struct WireMessage *message);

void WireLink_free(struct WireLink *self);
void **WireLink_userptr(struct WireLink *self);
bool WireLink_send(struct WireLink *self, const struct WireMessage *message);
WireCookie WireLink_makeCookie(struct WireLink *self, const void *data, size_t length);
struct DataView WireLink_getCookie(struct WireLink *self, WireCookie cookie);
void WireLink_freeCookie(struct WireLink *self, WireCookie cookie);
WireCookie WireLink_lastCookieIndex(struct WireLink *self);
