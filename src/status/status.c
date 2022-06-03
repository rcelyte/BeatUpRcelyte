#include "../thread.h"
#include "../net.h"
#include "internal.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct Context {
	int32_t listenfd;
	const char *path;
} static ctx;

static thread_return_t handle_client(intptr_t fd) {
	char buf[65536];
	ssize_t size = recv(fd, buf, sizeof(buf), 0);
	if(size >= 0) {
		size = status_resp("HTTP", ctx.path, buf, size);
		if(size)
			send(fd, buf, size, 0);
	}
	close(fd);
	return 0;
}

static thread_return_t status_handler(void *userptr) {
	uprintf("Started HTTP\n");
	struct SS addr = {sizeof(struct sockaddr_storage)};
	for(intptr_t clientfd; (clientfd = accept(ctx.listenfd, &addr.sa, &addr.len)) != -1;)
		thread_create((thread_t[]){0}, handle_client, (void*)clientfd);
	return 0;
}

static thread_t status_thread = 0;
bool status_init(const char *path, uint16_t port) {
	ctx.listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	{
		int32_t iSetOption = 1;
		setsockopt(ctx.listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
		struct sockaddr_in6 serv_addr = {
			.sin6_family = AF_INET6,
			.sin6_port = htons(port),
			.sin6_flowinfo = 0,
			.sin6_addr = IN6ADDR_ANY_INIT,
			.sin6_scope_id = 0,
		};
		if(bind(ctx.listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			uprintf("Cannot bind socket to port %hu: %s\n", port, strerror(errno));
			close(ctx.listenfd);
			ctx.listenfd = -1;
			return 1;
		}
	}
	if(listen(ctx.listenfd, 128) < 0) {
		close(ctx.listenfd);
		ctx.listenfd = -1;
		return 1;
	}
	ctx.path = path;
	return thread_create(&status_thread, status_handler, NULL);
}
void status_cleanup() {
	if(ctx.listenfd != -1) {
		uprintf("Stopping HTTP\n");
		shutdown(ctx.listenfd, SHUT_RD);
		close(ctx.listenfd);
		ctx.listenfd = -1;
		if(status_thread) {
			thread_join(status_thread);
			status_thread = 0;
		}
	}
}
