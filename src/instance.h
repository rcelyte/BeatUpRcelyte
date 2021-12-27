#include <stdint.h>
#include <mbedtls/ctr_drbg.h>
#include "packets.h"

_Bool instance_open(ServerCode *out, mbedtls_ctr_drbg_context *ctr_drbg);
void instance_close(ServerCode code);
_Bool instance_get_isopen(ServerCode code);
struct NetContext *instance_get_net(ServerCode code);
struct IPEndPoint instance_get_address(ServerCode code);
