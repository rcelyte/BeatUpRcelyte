#pragma once
#include <stddef.h>

#ifndef MBEDTLS_ERR_NET_CONN_RESET
#define MBEDTLS_ERR_NET_CONN_RESET -0x0050
#endif

int ssl_send(void *fd, const unsigned char *buf, size_t len);
int ssl_recv(void *fd, unsigned char *buf, size_t len);
