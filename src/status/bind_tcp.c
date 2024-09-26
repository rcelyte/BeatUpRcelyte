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
NetSocket status_bind_tcp(uint16_t port, uint32_t backlog) {
	#ifdef WINDOWS
	int err = WSAStartup(MAKEWORD(2,0), &(WSADATA){0});
	if(err) {
		uprintf("WSAStartup failed: %s\n", net_strerror(err));
		return NetSocket_Invalid;
	}
	#else
	signal(SIGPIPE, SIG_IGN);
	#endif
	const NetSocket listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(listenfd == NetSocket_Invalid) {
		uprintf("Failed to open TCP socket: %s\n", net_strerror(net_error()));
		#ifdef WINDOWS
		WSACleanup();
		#endif
		return NetSocket_Invalid;
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)(const int[]){1}, sizeof(int));
	setsockopt(listenfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)(const int[]){0}, sizeof(int));
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
	return NetSocket_Invalid;
}
