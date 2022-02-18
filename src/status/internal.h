#include "../log.h"
LOG_CTX("STATUS")

#include <stdint.h>

uint32_t status_resp(const char *source, const char *path, char *buf, uint32_t buf_len);

/*static tempList[2] = {{
	.playerNPms = 0,
	.levelNPms = 0,
	.playerCount = 0,
	.playerCap = 0xff,
	.levelName = "In-Game Room Listing",
}, {
	.code = 385792,
	.playerCount = 0,
	.playerCap = 10,
	.playerNPms = 14520,
	.levelNPms = 16330,
	.levelName = "rcelyte ''Serializers'' Summary & VIPs 02 [RCCD-0017] (C94) - 11. rcelyte — Exit This Thread's Stack Space (rcelyte's ''OVERFLOWING--200MB'' Generator)",
}};*/
