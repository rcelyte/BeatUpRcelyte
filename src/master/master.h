#pragma once
#include "../net.h"

struct NetContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port);
void master_cleanup();
