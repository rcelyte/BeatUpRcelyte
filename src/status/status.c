#include "status.h"
#include "../net.h"
#include "internal.h"
#include <unistd.h>
#include <string.h>

struct Context {
	int32_t listenfd;
	const char *path;
};
static struct Context ctx = {-1, NULL};

static void *handle_client(void *fd) {
	char buf[65536];
	ssize_t size = recv((int)(intptr_t)fd, buf, sizeof(buf), 0);
	if(size >= 0) {
		size = status_resp("HTTP", ctx.path, buf, (uint32_t)size);
		if(size)
			send((int)(intptr_t)fd, buf, (uint32_t)size, 0);
	}
	close((int)(intptr_t)fd);
	return 0;
}

static void *status_handler(void*) {
	uprintf("Started HTTP\n");
	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	for(intptr_t clientfd; (clientfd = accept(ctx.listenfd, &addr.sa, &addr.len)) != -1;)
		pthread_create((pthread_t[]){NET_THREAD_INVALID}, NULL, (void *(*)(void*))handle_client, (void*)clientfd);
	return 0;
}

static pthread_t status_thread = NET_THREAD_INVALID;
bool status_init(const char *path, uint16_t port) {
	ctx.listenfd = net_bind_tcp(port, 128);
	if(ctx.listenfd == -1)
		return true;
	ctx.path = path;
	if(pthread_create(&status_thread, NULL, (void *(*)(void*))status_handler, NULL)) {
		status_thread = NET_THREAD_INVALID;
		return true;
	}
	return false;
}
void status_cleanup() {
	if(ctx.listenfd == -1)
		return;
	uprintf("Stopping HTTP\n");
	shutdown(ctx.listenfd, SHUT_RD);
	net_close(ctx.listenfd);
	ctx.listenfd = -1;
	if(status_thread != NET_THREAD_INVALID) {
		pthread_join(status_thread, NULL);
		status_thread = NET_THREAD_INVALID;
	}
}
