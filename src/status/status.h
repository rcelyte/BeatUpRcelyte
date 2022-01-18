#include <stdint.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>

_Bool status_init(const char *path, uint16_t port);
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port);
void status_ssl_cleanup();
