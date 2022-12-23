#define NET_H_PRIVATE(x) x
#include "net.h"
#include "packets.h"
#include "ssl.h"
#include <mbedtls/error.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef WINDOWS
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif
// TODO: heartbeats+timeout to ensure the other side hasn't stalled

struct RemoteLink {
	mbedtls_ssl_context ctx;
	mbedtls_ssl_config conf;
};

union WireLink {
	WireLinkType type;
	struct NetContext local;
	struct RemoteLink remote;
};

struct NetContext *WireLink_cast_local(union WireLink *link) {
	return (link->type == WireLinkType_LOCAL) ? &link->local : NULL;
}

mbedtls_ssl_context *WireLink_cast_remote(union WireLink *link) {
	return (link->type >= WireLinkType_REMOTE_START) ? &link->remote.ctx : NULL;
}

static struct RemoteLink *RemoteLink_new() {
	struct RemoteLink *out = malloc(sizeof(struct RemoteLink));
	if(out) {
		mbedtls_ssl_config_init(&out->conf);
		mbedtls_ssl_init(&out->ctx);
	} else {
		uprintf("alloc error\n");
	}
	return out;
}

static intptr_t wire_connect_tcp(const char *address) {
	const char *address_end = &address[strlen(address)];
	const char *host_end = address_end, *port = address_end;
	char host[65536];
	if((uintptr_t)(address_end - address) >= lengthof(host)) {
		uprintf("address too long\n");
		return -1;
	}
	bool ipv6 = (*address == '[');
	if(ipv6) {
		++address;
		host_end = memchr(address, ']', address_end - address);
		if(!host_end) {
			uprintf("missing closing `]`\n");
			return -1;
		}
		for(const char *it = host_end; (it = memchr(it, ':', address_end - it));)
			port = &it[1];
	} else {
		for(const char *it = address; (it = memchr(it, ':', address_end - it));)
			host_end = it, port = &it[1];
	}
	memcpy(host, address, (const uint8_t*)host_end - (const uint8_t*)address);
	host[host_end - address] = 0;

	#ifdef WINDOWS // `getaddrinfo()` requires dynamic linking in glibc
	struct addrinfo *result = NULL;
	int res = getaddrinfo(host, *port ? port : "2328", &(struct addrinfo){ // TODO: default to "Port" config value
		.ai_family = AF_INET6,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = 0,
		.ai_protocol = 0,
	}, &result);
	if(res) {
		uprintf("getaddrinfo: %s\n", gai_strerror(res));
		return -1;
	}

	intptr_t sockfd;
	for(struct addrinfo *entry = result; entry != NULL; entry = entry->ai_next) {
		sockfd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
		if(sockfd == -1)
			continue;
		if(connect(sockfd, entry->ai_addr, entry->ai_addrlen) != -1)
			goto end;
		close(sockfd);
	}
	sockfd = -1;
	fprintf(stderr, "No connection found\n");
	end:
	freeaddrinfo(result);
	return sockfd;
	#else
	uint16_t portNum = *port ? htons(atoi(port)) : htons(2328); // TODO: default to "Port" config value
	struct SS addr = ipv6 ? (struct SS){
		.len = sizeof(struct sockaddr_in6),
		.in6 = {
			.sin6_family = AF_INET6,
			.sin6_port = portNum,
			.sin6_flowinfo = 0,
			.sin6_addr = IN6ADDR_ANY_INIT,
			.sin6_scope_id = 0,
		},
	} : (struct SS){
		.len = sizeof(struct sockaddr_in),
		.in = {
			.sin_family = AF_INET,
			.sin_port = portNum,
			.sin_addr.s_addr = htonl(INADDR_ANY),
		},
	};
	inet_pton(addr.sa.sa_family, host, ipv6 ? (void*)&addr.in6.sin6_addr : (void*)&addr.in.sin_addr);

	intptr_t sockfd = socket(addr.sa.sa_family, SOCK_STREAM, 0);
	if(sockfd == -1) {
		uprintf("Failed to create TCP socket\n");
		return -1;
	}
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	if(connect(sockfd, &addr.sa, addr.len) != -1) {
		uprintf("Connected to %s\n", addrstr);
		return sockfd;
	}
	uprintf("Failed to connect to %s\n", addrstr);
	close(sockfd);
	return -1;
	#endif
}

static uint8_t remoteKey_len = 0;
static uint8_t remoteKey[32];
void wire_set_key(uint8_t key[static 32], uint8_t key_len) {
	memcpy(remoteKey, key, key_len);
	remoteKey_len = key_len;
}

static bool wire_remote_handshake(struct NetContext *self, struct RemoteLink *link, intptr_t sockfd, bool server) {
	int res = mbedtls_ssl_config_defaults(&link->conf, server ? MBEDTLS_SSL_IS_SERVER : MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if(res != 0) {
		uprintf("mbedtls_ssl_config_defaults() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	mbedtls_ssl_conf_rng(&link->conf, mbedtls_ctr_drbg_random, &self->ctr_drbg);
	mbedtls_ssl_conf_psk(&link->conf, remoteKey, remoteKey_len, (const uint8_t*)"placeholder", 11); // TODO: use `mbedtls_ssl_conf_psk_cb()` on master side
	res = mbedtls_ssl_setup(&link->ctx, &link->conf);
	if(res != 0) {
		uprintf("mbedtls_ssl_setup() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	mbedtls_ssl_set_bio(&link->ctx, (void*)sockfd, ssl_send, ssl_recv, NULL);
	if(server)
		return false;
	while((res = mbedtls_ssl_handshake(&link->ctx))) {
		if(res == MBEDTLS_ERR_SSL_WANT_READ || res == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;
		uprintf("mbedtls_ssl_handshake() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	return false;
}

union WireLink *wire_connect_local(struct NetContext *self, struct NetContext *link) {
	if(!link->onWireLink)
		return NULL;
	net_lock(link);
	link->onWireLink(link->userptr, (union WireLink*)self);
	net_unlock(link);
	return (union WireLink*)link;
}

union WireLink *wire_connect_remote(struct NetContext *self, const char *address) {
	if(remoteKey_len == 0)
		return NULL;
	struct RemoteLink *link = RemoteLink_new();
	if(!link)
		return NULL;

	intptr_t sockfd = wire_connect_tcp(address);
	if(sockfd != -1 && !wire_remote_handshake(self, link, sockfd, false)) {
		// TODO: TLS ALPN protcol negotiation thing
		net_add_remote(self, &link->ctx);
		return (union WireLink*)link;
	}
	mbedtls_ssl_free(&link->ctx);
	mbedtls_ssl_config_free(&link->conf);
	close(sockfd);
	free(link);
	return NULL;
}

void wire_accept(struct NetContext *self, int32_t listenfd) {
	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	intptr_t sockfd = accept(listenfd, &addr.sa, &addr.len);
	if(sockfd == -1)
		return;
	if(remoteKey_len == 0 || !self->onWireLink)
		goto fail;
	// TODO: don't block during the handshake (plus websocket handshake once that's a thing) since this is in the middle of `net_recv()`

	struct RemoteLink *link = RemoteLink_new();
	if(!link)
		goto fail;

	if(!wire_remote_handshake(self, link, sockfd, true)) {
		uprintf("wire_accept(%d)\n", sockfd);
		// TODO: TLS ALPN protcol negotiation thing
		net_add_remote(self, &link->ctx);
		self->onWireLink(self->userptr, (union WireLink*)link);
		return;
	}
	mbedtls_ssl_free(&link->ctx);
	mbedtls_ssl_config_free(&link->conf);
	free(link);
	fail:
	close(sockfd);
	uprintf("wire_accept(%d) failed\n", sockfd);
}

void wire_disconnect(struct NetContext *self, union WireLink *link) {
	if(link->type == WireLinkType_INVALID)
		return;
	self->onWireMessage(self->userptr, link, NULL);
	if(link->type == WireLinkType_LOCAL) {
		link->local.onWireMessage(link->local.userptr, (union WireLink*)self, NULL);
		return;
	}
	net_remove_remote(self, &link->remote.ctx);
	int res = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
	do {
		res = mbedtls_ssl_close_notify(&link->remote.ctx);
	} while(res == MBEDTLS_ERR_SSL_WANT_READ || res == MBEDTLS_ERR_SSL_WANT_WRITE);
	int32_t sockfd = (intptr_t)link->remote.ctx.MBEDTLS_PRIVATE(p_bio);
	mbedtls_ssl_free(&link->remote.ctx);
	mbedtls_ssl_config_free(&link->remote.conf);
	free(link);
	close(sockfd);
}

bool wire_send(struct NetContext *self, union WireLink *link, const struct WireMessage *message) {
	if(link->type == WireLinkType_INVALID)
		return true;
	if(link->type == WireLinkType_LOCAL) {
		uprintf("wire_send_local(%s)\n", reflect(WireMessageType, message->type));
		net_lock(&link->local);
		link->local.onWireMessage(link->local.userptr, (union WireLink*)self, message);
		net_unlock(&link->local);
		return false;
	}
	uprintf("wire_send(%s)\n", reflect(WireMessageType, message->type));
	uint32_t cookie = message->cookie - 1;
	if(cookie < self->cookies_len && self->cookies[cookie].length) {
		void *buffer = malloc(self->cookies[cookie].length);
		if(!buffer) {
			uprintf("alloc error\n");
			return true;
		}
		memcpy(buffer, self->cookies[cookie].data, self->cookies[cookie].length);
		self->cookies[cookie] = (struct WireCookie){buffer, 0};
	}
	uint8_t pkt[16384], *pkt_end = pkt;
	pkt_write(message, &pkt_end, endof(pkt), PV_LEGACY_DEFAULT);
	for(int res = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED; (res = mbedtls_ssl_write(&link->remote.ctx, pkt, pkt_end - pkt)) <= 0;) {
		if(res == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;
		if(res != MBEDTLS_ERR_NET_CONN_RESET)
			uprintf("mbedtls_ssl_write() failed: %s\n", mbedtls_high_level_strerr(res));
		wire_disconnect(self, link);
		return true;
	}
	return false;
}

void wire_recv(struct NetContext *self, mbedtls_ssl_context *link) {
	uint8_t pkt[16384];
	int res;
	do {
		memset(pkt, 0, sizeof(pkt)); // TODO: this doesn't look right
		res = mbedtls_ssl_read(link, pkt, sizeof(pkt));
	} while(res == MBEDTLS_ERR_SSL_WANT_READ);
	struct WireMessage message;
	if(res < 0) {
		if(res != MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
			uprintf("mbedtls_ssl_read() failed: %s\n", mbedtls_high_level_strerr(res));
		goto fail;
	}
	const uint8_t *pkt_end = pkt;
	if(!pkt_read(&message, &pkt_end, &pkt[res], PV_LEGACY_DEFAULT)) {
		uprintf("pkt_read() failed\n");
		goto fail;
	}
	if(pkt_end != &pkt[res]) {
		uprintf("bad packet length\n");
		goto fail;
	}
	uprintf("wire_recv(%s)\n", reflect(WireMessageType, message.type));
	self->onWireMessage(self->userptr, (union WireLink*)link, &message);
	return;
	fail:
	wire_disconnect(self, (union WireLink*)link);
}

uint32_t wire_reserveCookie(struct NetContext *self, void *data, size_t length) {
	for(uint32_t i = 0; i < self->cookies_len; ++i) {
		if(self->cookies[i].data)
			continue;
		self->cookies[i] = (struct WireCookie){data, length};
		return i + 1;
	}
	struct WireCookie *list = realloc(self->cookies, (self->cookies_len + 1) * sizeof(*list));
	if(!list) {
		uprintf("alloc error");
		return 0;
	}
	self->cookies = list;
	list[self->cookies_len] = (struct WireCookie){data, length};
	return ++self->cookies_len;
}

void *wire_getCookie(struct NetContext *self, uint32_t cookie) {
	--cookie;
	return (cookie < self->cookies_len) ? self->cookies[cookie].data : NULL;
}

uint32_t wire_nextCookie(struct NetContext *self, uint32_t start) {
	for(uint32_t i = start; i < self->cookies_len; ++i)
		if(self->cookies[i].data)
			return i + 1;
	return 0;
}

void wire_releaseCookie(struct NetContext *self, uint32_t cookie) {
	--cookie;
	if(cookie >= self->cookies_len)
		return;
	if(!self->cookies[cookie].length)
		free(self->cookies[cookie].data);
	self->cookies[cookie] = (struct WireCookie){NULL, 0};
}
