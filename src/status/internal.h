#include "../global.h"
#include "../wire.h"
#include "../packets.h"
#include "http.h"
#include <stdint.h>

typedef uint8_t StatusCookieType;
enum {
	StatusCookieType_INVALID,
	StatusCookieType_GraphConnect,
};

struct GraphConnectCookie {
	StatusCookieType cookieType;
	struct HttpContext *http;
	struct String secret;
	struct BeatmapLevelSelectionMask selectionMask;
};

int32_t status_bind_tcp(uint16_t port, uint32_t backlog);
void status_resp(struct HttpContext *http, const char path[], struct HttpRequest httpRequest, struct WireLink *master);
void status_graph_resp(struct DataView cookieView, const struct WireGraphConnectResp *resp);

/*static tempList[2] = {{
	.playerNPms = 0,
	.levelNPms = 0,
	.playerCount = 0,
	.playerCap = 0xff,
	.levelName = "In-Game Room Listing",
}};*/
