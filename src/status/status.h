#include <stdint.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include "../packets.h"

typedef uint16_t StatusHandle;

_Bool status_init(const char *path, uint16_t port);
void status_cleanup();

_Bool status_ssl_init(mbedtls_x509_crt *cert, mbedtls_pk_context *key, const char *path, uint16_t port);
void status_ssl_cleanup();

void status_internal_init();
StatusHandle status_entry_new(ServerCode code, uint8_t playerCap);
void status_entry_set_playerCount(StatusHandle index, uint8_t count);
void status_entry_set_level(StatusHandle index, const char *name, float nps);
void status_entry_free(StatusHandle index);
