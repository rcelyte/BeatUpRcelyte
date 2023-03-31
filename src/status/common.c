#include "internal.h"
#include "../net.h"
#ifdef WINDOWS
#define net_error() WSAGetLastError()
#else
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#define net_error() (errno)
#endif

// TODO: cut dependency on `net`
int32_t status_bind_tcp(uint16_t port, uint32_t backlog) {
	#ifdef WINDOWS
	int err = WSAStartup(MAKEWORD(2,0), &(WSADATA){0});
	if(err) {
		uprintf("WSAStartup failed: %s\n", port, net_strerror(err));
		return -1;
	}
	#else
	signal(SIGPIPE, SIG_IGN);
	#endif
	int32_t listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(listenfd == -1) {
		uprintf("Failed to open TCP socket: %s\n", net_strerror(net_error()));
		#ifdef WINDOWS
		WSACleanup();
		#endif
		return -1;
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)(int32_t[]){1}, sizeof(int32_t));
	struct sockaddr_in6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(port),
		.sin6_flowinfo = 0,
		.sin6_addr = IN6ADDR_ANY_INIT,
		.sin6_scope_id = 0,
	};
	if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		uprintf("Cannot bind socket to port %hu: %s\n", port, net_strerror(net_error()));
		goto fail;
	}
	if(listen(listenfd, (int)backlog) < 0) {
		uprintf("listen() failed: %s\n", net_strerror(net_error()));
		goto fail;
	}
	return listenfd;
	fail:
	net_close(listenfd);
	return -1;
}
