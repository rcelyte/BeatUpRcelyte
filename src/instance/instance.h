#include <stdint.h>
#include <mbedtls/ctr_drbg.h>
#include "../net.h"

_Bool instance_init(const char *domain);
void instance_cleanup();

_Bool instance_get_isopen(ServerCode code, struct String *managerId_out, int32_t *playerLimit_out);
_Bool instance_open(ServerCode *out, struct String managerId, uint8_t playerLimit);
struct NetContext *instance_get_net(ServerCode code);
struct NetSession *instance_resolve_session(ServerCode code, struct SS addr, struct String userId);
struct IPEndPoint instance_get_address(ServerCode code);
