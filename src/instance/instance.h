#include <stdint.h>
#include <mbedtls/ctr_drbg.h>
#include "../net.h"

_Bool instance_init(const char *domain, const char *domainIPv4);
void instance_cleanup();

_Bool instance_request_block(uint16_t thread, uint16_t *group_out, uint16_t notify);
_Bool instance_room_open(uint16_t thread, uint16_t group, uint8_t sub, struct String managerId, struct GameplayServerConfiguration configuration);
void instance_room_close(uint16_t thread, uint16_t group, uint8_t sub);
struct String instance_room_get_managerId(uint16_t thread, uint16_t group, uint8_t sub);
struct GameplayServerConfiguration instance_room_get_configuration(uint16_t thread, uint16_t group, uint8_t sub);
uint32_t instance_room_get_protocolVersion(uint16_t thread, uint16_t group, uint8_t sub);
struct IPEndPoint instance_get_endpoint(_Bool ipv4);

struct NetSession *instance_room_resolve_session(uint16_t thread, uint16_t group, uint8_t sub, struct SS addr, struct String secret, struct String userId, struct String userName, uint32_t protocolVersion);
struct NetContext *instance_get_net(uint16_t thread);
