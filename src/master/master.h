#pragma once
#include "../net.h"
#include <mbedtls/x509_crt.h>

struct WireContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port);
void master_cleanup(void);
