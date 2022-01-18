#include <stdint.h>
#include <mbedtls/ctr_drbg.h>
#include "../net.h"

_Bool instance_get_isopen(ServerCode code);
_Bool instance_open(ServerCode *out);
struct NetContext *instance_get_net(ServerCode code);
struct NetSession *instance_resolve_session(ServerCode code, struct SS addr);
struct IPEndPoint instance_get_address(ServerCode code);
