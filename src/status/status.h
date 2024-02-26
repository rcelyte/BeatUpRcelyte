#include <stdint.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include "../packets.h"

struct WireContext;
bool status_ssl_init(const char *path, uint16_t port, mbedtls_x509_crt certs[2], mbedtls_pk_context keys[2], const char *domain, const char *remoteMaster, struct WireContext *localMaster, bool quiet);
void status_ssl_cleanup(void);

void status_internal_init(bool quiet);
void status_internal_cleanup(void);
