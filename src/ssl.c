#include "ssl.h"
#include <mbedtls/ssl.h>
#ifdef WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <errno.h>
#endif

static inline int ssl_error(int intr) {
	#ifdef WINDOWS
	(void)intr;
	if(WSAGetLastError() == WSAECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	#else
	if(errno == EPIPE || errno == ECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	if(errno == EINTR)
		return intr;
	#endif
	return -1;
}

int ssl_send(void *fd, const unsigned char *buf, size_t len) {
	ssize_t ret = send((intptr_t)fd, (char*)buf, len, 0);
	if(ret >= 0)
		return ret;
	return ssl_error(MBEDTLS_ERR_SSL_WANT_WRITE);
}

int ssl_recv(void *fd, unsigned char *buf, size_t len) {
	ssize_t ret = recv((intptr_t)fd, (char*)buf, len, 0);
	if(ret >= 0)
		return ret;
	return ssl_error(MBEDTLS_ERR_SSL_WANT_READ);
}
