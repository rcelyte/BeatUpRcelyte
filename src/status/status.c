#include "status.h"
#include "../net.h"
#include "internal.h"
#include <unistd.h>
#include <string.h>

struct Context {
	struct ContextBase base;
	const char *path;
};
static struct Context ctx = {{-1, NULL}, NULL};

static void *handle_client_http(void *fd) {
	char buf[65536];
	ssize_t size = recv((int)(uintptr_t)fd, buf, sizeof(buf), 0);
	if(size >= 0) {
		size = status_resp("HTTP", ctx.path, buf, (uint32_t)size);
		if(size)
			send((int)(uintptr_t)fd, buf, (uint32_t)size, 0);
	}
	close((int)(uintptr_t)fd);
	return 0;
}

void *status_handler(struct ContextBase *ctx) {
	pthread_attr_t attr;
	if(pthread_attr_init(&attr)) {
		uprintf("pthread_attr_init() failed\n");
		return 0;
	}
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	uprintf("Started HTTP%s\n", (ctx->handleClient == handle_client_http) ? "" : "S");
	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	for(int clientfd; (clientfd = accept(ctx->listenfd, &addr.sa, &addr.len)) != -1;) {
		if(pthread_create((pthread_t[]){NET_THREAD_INVALID}, &attr, ctx->handleClient, (void*)(uintptr_t)clientfd))
			uprintf("pthread_create() failed\n");
	}
	if(pthread_attr_destroy(&attr))
		uprintf("pthread_attr_destroy() failed\n");
	return 0;
}

static pthread_t status_thread = NET_THREAD_INVALID;
bool status_init(const char *path, uint16_t port) {
	ctx.base = (struct ContextBase){
		.listenfd = net_bind_tcp(port, 128),
		.handleClient = handle_client_http,
	};
	if(ctx.base.listenfd == -1)
		return true;
	ctx.path = path;
	if(pthread_create(&status_thread, NULL, (void *(*)(void*))status_handler, &ctx.base)) {
		status_thread = NET_THREAD_INVALID;
		return true;
	}
	return false;
}
void status_cleanup() {
	if(ctx.base.listenfd == -1)
		return;
	uprintf("Stopping HTTP\n");
	shutdown(ctx.base.listenfd, SHUT_RD);
	net_close(ctx.base.listenfd);
	ctx.base.listenfd = -1;
	if(status_thread != NET_THREAD_INVALID) {
		pthread_join(status_thread, NULL);
		status_thread = NET_THREAD_INVALID;
	}
}
