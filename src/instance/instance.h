#pragma once
#include "../net.h"
#include <mbedtls/x509_crt.h>

struct WireContext;
bool instance_init(const char *domainIPv4, const char *domain, const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, const char *remoteMaster, struct WireContext *localMaster, const char *mapPoolFile, uint32_t count);
void instance_cleanup(void);
