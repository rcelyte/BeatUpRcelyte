#include "wire.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

struct WireLink {
	void *userptr;
	uint32_t cookies_len;
	struct DataView *cookies;
	union {
		struct {
			struct WireContext *ctx;
			struct WireLink *link;
		} local;
	};
};

void wire_init(uint8_t remoteKey[32], uint8_t remoteKey_len) {
	(void)remoteKey; (void)remoteKey_len;
}

void wire_cleanup() {}

static void onMessage_stub(struct WireContext*, struct WireLink*, const struct WireMessage*) {}
bool WireContext_init(struct WireContext *self, void *userptr, uint32_t tcpBacklog) {
	*self = (struct WireContext){
		.userptr = userptr,
		.onMessage = onMessage_stub,
	};
	if(tcpBacklog)
		uprintf("TODO: remote wire\n");
	return false;
}

void WireContext_cleanup(struct WireContext*) {}

struct WireLink *WireContext_attach(struct WireContext *self, struct WireContext *peer, const struct WireMessage *message) {
	struct WireLink *const link = malloc(sizeof(struct WireLink));
	struct WireLink *const peerLink = malloc(sizeof(struct WireLink));
	if(link == NULL || peerLink == NULL) {
		free(link);
		free(peerLink);
		return NULL;
	}
	*link = (struct WireLink){
		.local = {peer, peerLink},
	};
	*peerLink = (struct WireLink){
		.local = {self, link},
	};
	peer->onMessage(peer, peerLink, message);
	return link;
}

struct WireLink *WireContext_connect(struct WireContext*, const char[], const struct WireMessage*) {
	uprintf("TODO: remote wire\n");
	return NULL;
}

void WireLink_free(struct WireLink *self) {
	struct WireLink *const peerLink = self->local.link;
	if(peerLink == NULL) // deletion in progress
		return;
	struct WireContext *const ctx = peerLink->local.ctx;
	self->local.link = NULL;
	peerLink->local.link = NULL;
	ctx->onMessage(ctx, self, NULL);
	self->local.ctx->onMessage(self->local.ctx, peerLink, NULL);
	for(WireCookie cookie = 1; cookie <= WireLink_lastCookieIndex(peerLink); ++cookie)
		WireLink_freeCookie(peerLink, cookie);
	free(peerLink->cookies);
	free(peerLink);
	for(WireCookie cookie = 1; cookie <= WireLink_lastCookieIndex(self); ++cookie)
		WireLink_freeCookie(self, cookie);
	free(self->cookies);
	free(self);
}

void **WireLink_userptr(struct WireLink *self) {
	return &self->userptr;
}

bool WireLink_send(struct WireLink *self, const struct WireMessage *message) {
	self->local.ctx->onMessage(self->local.ctx, self->local.link, message);
	return false;
}

WireCookie WireLink_makeCookie(struct WireLink *self, const void *data, size_t length) {
	if(!length)
		return 0;
	const struct DataView buffer = {
		.data = malloc(length),
		.length = length,
	};
	if(buffer.data == NULL)
		return 0;
	memcpy(buffer.data, data, length);
	for(WireCookie cookie = 1; cookie <= self->cookies_len; ++cookie) {
		if(self->cookies[cookie - 1].data != NULL)
			continue;
		self->cookies[cookie - 1] = buffer;
		return cookie;
	}
	struct DataView *newCookies = realloc(self->cookies, (self->cookies_len + 1) * sizeof(struct DataView));
	if(newCookies == NULL) {
		free(buffer.data);
		return 0;
	}
	self->cookies = newCookies;
	self->cookies[self->cookies_len++] = buffer;
	return self->cookies_len;
}

struct DataView WireLink_getCookie(struct WireLink *self, WireCookie cookie) {
	if(!cookie || cookie > WireLink_lastCookieIndex(self))
		return (struct DataView){0};
	return self->cookies[cookie - 1];

}

void WireLink_freeCookie(struct WireLink *self, WireCookie cookie) {
	if(!cookie || cookie > WireLink_lastCookieIndex(self))
		return;
	free(self->cookies[cookie - 1].data);
	self->cookies[cookie - 1] = (struct DataView){0};
}

WireCookie WireLink_lastCookieIndex(struct WireLink *self) {
	return self->cookies_len;
}
