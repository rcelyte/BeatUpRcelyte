#include "../net.h"
#include "internal.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#define SHUT_RD SD_RECEIVE
#else
#include <pthread.h>
#endif
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct Context {
	int32_t listenfd;
	const char *path;
};

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
status_handler(struct Context *ctx) {
	uprintf("Started HTTP\n");
	struct SS addr = {sizeof(struct sockaddr_storage)};
	int32_t clientfd;
	while((clientfd = accept(ctx->listenfd, &addr.sa, &addr.len)) != -1) {
		char buf[65536];
		ssize_t size = recv(clientfd, buf, sizeof(buf), 0);
		if(size < 0) {
			close(clientfd);
			continue;
		}
		size = status_resp("HTTP", ctx->path, buf, size);
		if(size)
			send(clientfd, buf, size, 0);
		close(clientfd);
	}
	return 0;
}
#ifdef WINDOWS
static HANDLE status_thread = NULL;
#else
static pthread_t status_thread = 0;
#endif
static struct Context ctx;
_Bool status_init(const char *path, uint16_t port) {
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
	#ifdef WINDOWS
	status_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)status_handler, &ctx, 0, NULL);
	return !status_thread;
	#else
	return pthread_create(&status_thread, NULL, (void*(*)(void*))&status_handler, &ctx) != 0;
	#endif
}
void status_cleanup() {
	if(ctx.listenfd != -1) {
		uprintf("Stopping HTTP\n");
		shutdown(ctx.listenfd, SHUT_RD);
		close(ctx.listenfd);
		ctx.listenfd = -1;
		if(status_thread) {
			#ifdef WINDOWS
			WaitForSingleObject(status_thread, INFINITE);
			#else
			pthread_join(status_thread, NULL);
			#endif
			status_thread = 0;
		}
	}
}
