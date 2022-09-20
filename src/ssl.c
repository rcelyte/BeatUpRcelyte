#include "ssl.h"
#include <mbedtls/ssl.h>
#include <errno.h>
#ifdef WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

int ssl_send(void *fd, const unsigned char *buf, size_t len) {
	ssize_t ret = send((intptr_t)fd, (char*)buf, len, 0);
	if(ret >= 0)
		return ret;
	#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && !defined(EFI32)
	if(WSAGetLastError() == WSAECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	#else
	if(errno == EPIPE || errno == ECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	if(errno == EINTR)
		return MBEDTLS_ERR_SSL_WANT_WRITE;
	#endif
	return -1;
}

int ssl_recv(void *fd, unsigned char *buf, size_t len) {
	ssize_t ret = recv((intptr_t)fd, (char*)buf, len, 0);
	if(ret >= 0)
		return ret;
	#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && !defined(EFI32)
	if(WSAGetLastError() == WSAECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	#else
	if(errno == EPIPE || errno == ECONNRESET)
		return MBEDTLS_ERR_NET_CONN_RESET;
	if(errno == EINTR)
		return MBEDTLS_ERR_SSL_WANT_READ;
	#endif
	return -1;
}
