#include "../global.h"
#include <stdint.h>

void *status_handler(void *(*handleClient)(void*));
uint32_t status_resp(const char *source, const char *path, char *buf, uint32_t buf_len);

/*static tempList[2] = {{
	.playerNPms = 0,
	.levelNPms = 0,
	.playerCount = 0,
	.playerCap = 0xff,
	.levelName = "In-Game Room Listing",
}};*/
