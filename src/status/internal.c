#include "internal.h"
#include "status.h"
#include "json.h"
#include <inttypes.h>

#define READ_SYM(dest, sym) _read_sym(dest, sym, (uint32_t)(sym##_end - sym))
static inline uint32_t _read_sym(char *restrict dest, const uint8_t *restrict sym, uint32_t length) {
	memcpy(dest, sym, length);
	return length;
}

static const uint64_t TEST_maintenanceStartTime = 0;
static const uint64_t TEST_maintenanceEndTime = 0;
static const char TEST_maintenanceMessage[] = "";

extern const uint8_t head_html[], head_html_end[];

struct PacketBuffer {
	uint32_t data_len;
	uint8_t data[];
};
static struct PacketBuffer *roomIndex[0x200] = {0}; // TODO: dynamic resizing
static uint32_t roomIndex_tail = 0;
static inline struct PacketBuffer **roomIndex_get(const uint32_t sequence) {
	return &roomIndex[sequence & (lengthof(roomIndex) - 1)];
}

void status_internal_init() {}

void status_internal_cleanup() {
	for(struct PacketBuffer **buffer = roomIndex; buffer < endof(roomIndex); ++buffer) {
		free(*buffer);
		*buffer = NULL;
	}
}

void status_update_index(const uint32_t sequence, const uint8_t entry[], const uint32_t entry_len) {
	while(sequence - roomIndex_tail > lengthof(roomIndex)) {
		free(*roomIndex_get(roomIndex_tail));
		*roomIndex_get(roomIndex_tail++) = NULL;
	}
	free(*roomIndex_get(sequence));
	struct PacketBuffer *const buffer = *roomIndex_get(sequence) = malloc(sizeof(*buffer) + entry_len);
	if(buffer == NULL) {
		uprintf("alloc error\n");
		return;
	}
	buffer->data_len = entry_len;
	memcpy(buffer->data, entry, entry_len);
}

static uint32_t escape(uint8_t *out, size_t limit, const uint8_t *in, size_t in_len) {
	uint8_t *start = out;
	for(size_t i = 0; i < in_len; ++i) {
		if(i + 5 + 3 > limit) {
			if(out > start && (in[i] & 192) == 128) { // remove incomplete UTF-8 character
				--out;
				while(out > start && (*out & 192) == 128)
					--out;
			}
			*out++ = 0xE2; *out++ = 0x80; *out++ = 0xA6;
			break;
		}
		switch(in[i]) {
			case '>': *out++ = '&'; *out++ = 'g'; *out++ = 't'; *out++ = ';'; break;
			case '<': *out++ = '&'; *out++ = 'l'; *out++ = 't'; *out++ = ';'; break;
			case '&': *out++ = '&'; *out++ = 'a'; *out++ = 'm'; *out++ = 'p'; *out++ = ';'; break;
			default: *out++ = in[i];
		}
	}
	return (uint32_t)(out - start);
}

static char *base64_encode(char *out, const struct ByteArrayNetSerializable *const buffer) {
	static const char table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const div_t length = div(buffer->length, 3);
	const uint8_t *bin = buffer->data;
	for(uint32_t i = 0; i < (uint32_t)length.quot; ++i, bin += 3) {
		*out++ = table[bin[0] >> 2];
		*out++ = table[(bin[0] << 4 | bin[1] >> 4) & 63];
		*out++ = table[(bin[1] << 2 | bin[2] >> 6) & 63];
		*out++ = table[bin[2] & 63];
	}
	if(length.rem != 0) {
		*out++ = table[bin[0] >> 2];
		if(length.rem == 2) {
			*out++ = table[(bin[0] << 4 | bin[1] >> 4) & 63];
			*out++ = table[(bin[1] << 2) & 63];
		} else {
			*out++ = table[(bin[0] << 4) & 63];
			*out++ = '=';
		}
		*out++ = '=';
	}
	return out;
}

static void status_web(struct HttpContext *const http, const ServerCode code) {
	bool index = (code == ServerCode_NONE);
	char page[65536];
	uint32_t page_len = READ_SYM(page, head_html);
	if(index) {
		page_len += (uint32_t)sprintf(&page[page_len],
			"<style>#head>a>span{display:none}</style>"
			"<table id=main style=width:100%%;table-layout:fixed>"
				"<thead><tr>"
					"<th style=width:6ch>Code"
					"<th>Current Level"
					"<th id=ph>Players"
					"<th style=width:13ch>Version"
					"<th style=width:15ch>Player/Level NPS"
					"<th style=width:13.5pt>"
				"<tbody>");
		for(uint32_t i = roomIndex_tail, end = roomIndex_tail + lengthof(roomIndex); i != end; ++i) {
			const struct PacketBuffer *const packet = *roomIndex_get(i);
			struct WireStatusEntry entry;
			if(packet == NULL || packet->data_len == 0 || pkt_read(&entry, (const uint8_t*[]){packet->data}, &packet->data[packet->data_len], PV_WIRE) != packet->data_len)
				continue;
			/*if(!entry.public) // TODO: make private rooms visible to matching IPs
				continue;*/
			char scode[8], playerCapacity[16] = "∞", noteRate[24] = "", *noteRate_end = noteRate;
			if(entry.playerCapacity < 0xff)
				sprintf(playerCapacity, "%u", entry.playerCapacity);
			if(entry.playerNPS != UINT16_MAX)
				noteRate_end += sprintf(noteRate_end, "%.2f / ", entry.playerNPS / 256.);
			if(entry.levelNPS != UINT16_MAX)
				sprintf(noteRate_end, "%.2f", entry.levelNPS / 256.);
			else
				sprintf(noteRate_end, "too much");
			ServerCodeToString(scode, entry.code);
			if(entry.levelName.isNull)
				entry.levelName = entry.levelID;
			struct LongString levelName;
			levelName.length = escape((uint8_t*)levelName.data, sizeof(levelName.data) - 10, (const uint8_t*)entry.levelName.data,
				entry.levelName.length - (entry.levelName.length == lengthof(entry.levelName.data)));
			if(entry.levelName.length == lengthof(entry.levelName.data)) {
				memcpy(&levelName.data[levelName.length], "<i>...</i>", 10);
				levelName.length += 10;
			}
			static const char *const protocolNames[] = {
				"???", "???", "???", "???", "???", "???",
				[6] = "1.19.0",
				[7] = "1.19.1",
				[8] = "1.20.0 ⬌ 1.31.1",
				[9] = "1.32.0 ⬌ 1.34.4",
			};
			char cover[(sizeof(entry.levelCover.data) * 4 + 3) / 3 + 53] = "\0style=background-image:url(data:image/jpeg;base64,";
			if(entry.levelCover.length > 4 && memcmp(entry.levelCover.data, (const uint8_t[4]){0xff,0xd8,0xff,0xe0}, 4) == 0) {
				cover[0] = ' ';
				*base64_encode(&cover[51], &entry.levelCover) = ')';
			}
			page_len += (uint32_t)sprintf(&page[page_len], "%s%s%s%s%s%s%s%.*s%s%.*s%s%.*s%s%s%s%u%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", "<tr>"
				"<th><a href=\"", scode, "\"><code>", scode,
				"<td><a href=\"", scode, "\"><div class=\"ln\"><span>", (int)levelName.length, levelName.data, "</span><br>"
					"<div>▏", (int)levelName.length, levelName.data, "▕▏", (int)levelName.length, levelName.data, "▕" // TODO: resolve level name for ID
				"<th><a href=\"", scode, "\">", entry.playerCount, " / ", playerCapacity,
				"<th><a href=\"", scode, "\">", protocolNames[(entry.protocolVersion < lengthof(protocolNames)) ? entry.protocolVersion : 0],
				"<th><a href=\"", scode, "\">", noteRate,
				"<th", cover, "><a href=\"", scode, "\"><div>&nbsp;");
		}
	} else {
		page_len += (uint32_t)sprintf(&page[page_len], "<div id=main>This page is still under construction</div>");
		uprintf("TODO: room status page\n");
	}
	HttpContext_respond(http, 200, "text/html; charset=utf-8", page, page_len);
}

#define startsWithBytes(start, end, str, len) ((uintptr_t)((end) - (start)) >= (len) && memcmp((start), (str), (len)) == 0)
#define startsWith(start, end, str) startsWithBytes(start, end, str, sizeof(str) - sizeof(""))

static const char *nextLine(const char *start, const char *end) {
	for(end -= 5 /* "\n\r\n\r\n" */; (start = memchr(start, '\r', (size_t)(end - start))) != NULL; ++start)
		if(start[1] == '\n')
			return &start[2];
	return NULL;
}

typedef uint_least8_t UserAgent;
enum {
	UserAgent_Web,
	UserAgent_Game,
	UserAgent_BSSB,
};

static const char *UserAgent_ToString[] = {
	[UserAgent_Web] = "web",
	[UserAgent_Game] = "game",
	[UserAgent_BSSB] = "bssb",
};

static UserAgent ProbeHeaders(const char *buf, const char *end, size_t *contentLength_out) {
	uint32_t lineCount = 0;
	for(; (buf = nextLine(buf, end)); ++lineCount) {
		if(startsWith(buf, end, "Host: ") ||
		   startsWith(buf, end, "Connection: ") ||
		   startsWith(buf, end, "Content-Type: application/json") ||
		   startsWith(buf, end, "Authorization: "))
			continue;
		if(startsWith(buf, end, "Content-Length: ")) {
			size_t length = 0;
			for(const char *it = &buf[16]; it < end && *it >= '0' && *it <= '9'; ++it)
				length = length * 10 + ((size_t)*it - '0');
			*contentLength_out = length;
			continue;
		}
		return startsWith(buf, end, "User-Agent: BeatSaberServerBrowser") ? UserAgent_BSSB : UserAgent_Web;
	}
	return lineCount ? UserAgent_Game : UserAgent_Web;
}

#ifndef STATUS_APPVER_POSTFIX
#define STATUS_APPVER_POSTFIX ""
#endif

#define PUT(...) (msg_end += (uint32_t)snprintf(msg_end, (msg_end >= endof(msg)) ? 0 : (uint32_t)(endof(msg) - msg_end), __VA_ARGS__))
static void status_status(struct HttpContext *http, bool isGame) {
	char msg[65536], *msg_end = msg;
	PUT("{\"minimum_app_version\":\"1.19.0%s\""
	    ",\"maximumAppVersion\":\"1.34.4_🅱️\""
	    ",\"status\":%u", isGame ? "b2147483647" : STATUS_APPVER_POSTFIX, TEST_maintenanceStartTime != 0);
	if(TEST_maintenanceStartTime) {
		PUT(",\"maintenance_start_time\":%" PRIu64, TEST_maintenanceStartTime);
		PUT(",\"maintenance_end_time\":%" PRIu64, TEST_maintenanceEndTime);
		PUT(",\"maintenanceEndTime\":%" PRIu64, TEST_maintenanceEndTime); // legacy
		PUT(",\"user_message\":{\"localizations\":[{\"language\":0,\"message\":\"%s\"}]}", TEST_maintenanceMessage);
	}
	PUT(",\"requiredMods\": ["
	    "{\"id\":\"BeatUpClient\",\"version\":\"0.4.6\"},"
	    "{\"id\":\"MultiplayerCore\",\"version\":\"1.1.1\"},"
	    "{\"id\":\"BeatTogether\",\"version\":\"2.0.1\"},"
	    "{\"id\":\"BeatSaberPlus_SongOverlay\",\"version\":\"4.6.1\"},"
	    "{\"id\":\"_Heck\",\"version\":\"1.4.1\"},"
	    "{\"id\":\"NoodleExtensions\",\"version\":\"1.5.1\"},"
	    "{\"id\":\"Chroma\",\"version\":\"2.6.1\"},"
	    "{\"id\":\"EditorEX\",\"version\":\"1.2.0\"},"
	    "{\"id\":\"LeaderboardCore\",\"version\":\"1.2.2\"}"
	"]}");
	if(msg_end >= endof(msg))
		HttpContext_respond(http, 500, "text/plain; charset=utf-8", NULL, 0);
	else
		HttpContext_respond(http, 200, "application/json; charset=utf-8", msg, (size_t)(msg_end - msg));
}

static void status_graph(struct HttpContext *http, struct HttpRequest req, struct WireLink *master) {
	struct GraphConnectCookie state = {
		.cookieType = StatusCookieType_GraphConnect,
		.http = http,
	};
	struct WireGraphConnect connectInfo = {0};
	struct JsonIterator iter = {(const char*)req.body, (const char*)&req.body[req.body_len], false, {0}};
	JSON_ITER_OBJECT(&iter, key0) {
		if(String_is(key0, "version")) {
			const struct String version = json_read_string(&iter);
			if(version.length < 2 || version.data[0] != '1' || version.data[1] != '.')
				continue;
			const char *sep = memchr(version.data, '_', version.length);
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wmultichar"
			_Static_assert('abc' == ('a' << 16 | 'b' << 8 | 'c'));
			uint64_t versionBE = 0;
			for(const char *it = &version.data[2], *const it_end = sep ? sep : &version.data[version.length]; it < it_end; ++it)
				versionBE = versionBE << 8 | *(const uint8_t*)it;
			switch(versionBE) {
				case '19.1': connectInfo.gameVersion = GameVersion_1_19_1; break;
				case '20.0': connectInfo.gameVersion = GameVersion_1_20_0; break;
				case '21.0': connectInfo.gameVersion = GameVersion_1_21_0; break;
				case '22.0': connectInfo.gameVersion = GameVersion_1_22_0; break;
				case '22.1': connectInfo.gameVersion = GameVersion_1_22_1; break;
				case '23.0': connectInfo.gameVersion = GameVersion_1_23_0; break;
				case '24.0': connectInfo.gameVersion = GameVersion_1_24_0; break;
				case '24.1': connectInfo.gameVersion = GameVersion_1_24_1; break;
				case '25.0': connectInfo.gameVersion = GameVersion_1_25_0; break;
				case '25.1': connectInfo.gameVersion = GameVersion_1_25_1; break;
				case '26.0': connectInfo.gameVersion = GameVersion_1_26_0; break;
				case '27.0': connectInfo.gameVersion = GameVersion_1_27_0; break;
				case '28.0': connectInfo.gameVersion = GameVersion_1_28_0; break;
				case '29.0': connectInfo.gameVersion = GameVersion_1_29_0; break;
				case '29.1': connectInfo.gameVersion = GameVersion_1_29_1; break;
				case '29.4': connectInfo.gameVersion = GameVersion_1_29_4; break;
				case '30.0': connectInfo.gameVersion = GameVersion_1_30_0; break;
				case '30.2': connectInfo.gameVersion = GameVersion_1_30_2; break;
				case '31.0': connectInfo.gameVersion = GameVersion_1_31_0; break;
				case '31.1': connectInfo.gameVersion = GameVersion_1_31_1; break;
				case '32.0': connectInfo.gameVersion = GameVersion_1_32_0; break;
				case '33.0': connectInfo.gameVersion = GameVersion_1_33_0; break;
				case '34.0': connectInfo.gameVersion = GameVersion_1_34_0; break;
				case '34.2': connectInfo.gameVersion = GameVersion_1_34_2; break;
				case '34.4': connectInfo.gameVersion = GameVersion_1_34_4; break;
				default: {
					connectInfo.gameVersion = GameVersion_Unknown;
					uprintf("Unexpected game version: %.*s\n", version.length, version.data);
				}
			}
			connectInfo.protocolVersion =
				(connectInfo.gameVersion < GameVersion_1_19_1) ? 6 :
				(connectInfo.gameVersion < GameVersion_1_20_0) ? 7 :
				(connectInfo.gameVersion < GameVersion_1_32_0) ? 8 :
				9;
			state.shortMask = (connectInfo.gameVersion != GameVersion_Unknown && connectInfo.gameVersion < GameVersion_1_34_0);
			#pragma GCC diagnostic pop
		} else if(String_is(key0, "beatmap_level_selection_mask")) {
			JSON_ITER_OBJECT(&iter, key1) {
				if(String_is(key1, "difficulties")) {
					state.selectionMask.difficulties = (BeatmapDifficultyMask)json_read_uint64(&iter);
				} else if(String_is(key1, "modifiers")) {
					state.selectionMask.modifiers = (GameplayModifierMask)json_read_uint64(&iter);
				} else if(String_is(key1, "song_packs")) {
					const struct String songPacks = json_read_string(&iter);
					// state.selectionMask.songPacks = ...
					uprintf("TODO: parse songPackMask\n");
					(void)songPacks;
				} else {
					json_skip_any(&iter);
				}
			}
		} else if(String_is(key0, "gameplay_server_configuration")) {
			JSON_ITER_OBJECT(&iter, key1) {
				if(String_is(key1, "max_player_count"))
					connectInfo.configuration.maxPlayerCount = (int32_t)json_read_uint64(&iter);
				else if(String_is(key1, "discovery_policy"))
					connectInfo.configuration.discoveryPolicy = (DiscoveryPolicy)json_read_uint64(&iter);
				else if(String_is(key1, "invite_policy"))
					connectInfo.configuration.invitePolicy = (InvitePolicy)json_read_uint64(&iter);
				else if(String_is(key1, "gameplay_server_mode"))
					connectInfo.configuration.gameplayServerMode = (GameplayServerMode)json_read_uint64(&iter);
				else if(String_is(key1, "song_selection_mode"))
					connectInfo.configuration.songSelectionMode = (SongSelectionMode)json_read_uint64(&iter);
				else if(String_is(key1, "gameplay_server_control_settings"))
					connectInfo.configuration.gameplayServerControlSettings = (GameplayServerControlSettings)json_read_uint64(&iter);
				else
					json_skip_any(&iter);
			}
		} else if(String_is(key0, "user_id")) {
			connectInfo.userId = json_read_string(&iter);
		} else if(String_is(key0, "private_game_secret")) {
			connectInfo.secret = state.secret = json_read_string(&iter);
		} else if(String_is(key0, "private_game_code")) {
			const struct String code = json_read_string(&iter);
			connectInfo.code = StringToServerCode(code.data, code.length);
		} else {
			json_skip_any(&iter);
		}
	}
	if(iter.fault || connectInfo.gameVersion == GameVersion_Unknown || connectInfo.userId.length == 0 || connectInfo.configuration.maxPlayerCount == 0) {
		status_graph_resp((struct DataView){&state, sizeof(state)}, &(struct WireGraphConnectResp){
			.result = MultiplayerPlacementErrorCode_Unknown,
		});
		return;
	}
	const WireCookie cookie = WireLink_makeCookie(master, &state, sizeof(state));
	const bool failed = WireLink_send(master, &(struct WireMessage){
		.type = WireMessageType_WireGraphConnect,
		.cookie = cookie,
		.graphConnect = connectInfo,
	});
	if(failed) {
		WireLink_freeCookie(master, cookie);
		status_graph_resp((struct DataView){&state, sizeof(state)}, NULL);
	}
}

void status_graph_resp(struct DataView cookieView, const struct WireGraphConnectResp *resp) {
	const struct GraphConnectCookie *state = (struct GraphConnectCookie*)cookieView.data;
	if(cookieView.length != sizeof(*state) || state->cookieType != StatusCookieType_GraphConnect) {
		uprintf("Graph Connect Error: Malformed wire cookie\n");
		return;
	}
	char msg[65536], *msg_end = msg;
	const MultiplayerPlacementErrorCode result = (resp != NULL) ? resp->result : MultiplayerPlacementErrorCode_MatchmakingTimeout;
	const char *const packMask = state->shortMask ? "/////////////////////w" : "//////////////////////////////////////////8";
	if(result != MultiplayerPlacementErrorCode_Success) {
		PUT("{\"error_code\":%hhu,\"player_session_info\":{\"game_session_id\":\"\",\"port\":-1,\"dns_name\":\"\",\"player_session_id\":\"\","
		    "\"private_game_code\":\"\",\"gameplay_server_configuration\":{\"max_player_count\":5,\"discovery_policy\":1,\"invite_policy\":0,"
		    "\"gameplay_server_mode\":1,\"song_selection_mode\":2,\"gameplay_server_control_settings\":3},\"beatmap_level_selection_mask\":{"
		    "\"difficulties\":31,\"modifiers\":65535,\"song_packs\":\"%s\"},\"private_game_secret\":\"\"},\"poll_interval_ms\":-1}",
		    result, packMask);
	} else {
		uprintf("TODO: encode songPackMask\n");
		PUT("{"
				"\"error_code\":0," // resp->result
				"\"player_session_info\":{"
					"\"game_session_id\":\"beatupserver:%08x\"," // resp->hostId
					"\"port\":%u," // resp->endPoint.port
					"\"dns_name\":\"%.*s\"," // resp->endPoint.address.length, resp->endPoint.address.data
					"\"player_session_id\":\"pslot$%u,%03u\"," // resp->roomSlot, resp->playerSlot
					"\"private_game_code\":\"%s\"," // ServerCodeToString((char[8]){0}, resp->code)
					"\"private_game_secret\":\"%.*s\"," // state->secret.length, state->secret.data
					"\"beatmap_level_selection_mask\":{"
						"\"difficulties\":%hhu," // state->selectionMask.difficulties
						"\"modifiers\":%u," // state->selectionMask.modifiers
						"\"song_packs\":\"%s\"" // packMask
					"},"
					"\"gameplay_server_configuration\":{"
						"\"max_player_count\":%d," // resp->configuration.maxPlayerCount
						"\"discovery_policy\":%d," // resp->configuration.discoveryPolicy
						"\"invite_policy\":%d," // resp->configuration.invitePolicy
						"\"gameplay_server_mode\":%d," // resp->configuration.gameplayServerMode
						"\"song_selection_mode\":%d," // resp->configuration.songSelectionMode
						"\"gameplay_server_control_settings\":%d" // resp->configuration.gameplayServerControlSettings
					"}"
				"},"
				"\"poll_interval_ms\":-1"
			"}",
			resp->hostId,
			resp->endPoint.port,
			resp->endPoint.address.length, resp->endPoint.address.data,
			resp->roomSlot, resp->playerSlot,
			ServerCodeToString((char[8]){0}, resp->code),
			state->secret.length, state->secret.data,
			state->selectionMask.difficulties,
			state->selectionMask.modifiers,
			packMask,
			resp->configuration.maxPlayerCount,
			resp->configuration.discoveryPolicy,
			resp->configuration.invitePolicy,
			resp->configuration.gameplayServerMode,
			resp->configuration.songSelectionMode,
			resp->configuration.gameplayServerControlSettings);
	}
	if(msg_end >= endof(msg))
		HttpContext_respond(state->http, 500, "text/plain; charset=utf-8", NULL, 0);
	else
		HttpContext_respond(state->http, 200, "application/json; charset=utf-8", msg, (size_t)(msg_end - msg));
}

void status_resp(struct HttpContext *http, const char path[], struct HttpRequest httpRequest, struct WireLink *master) {
	const char *req = httpRequest.header, *const req_end = &httpRequest.header[httpRequest.header_len];
	bool post = false;
	if(startsWith(req, req_end, "GET /")) {
		req += 5;
	} else if(startsWith(req, req_end, "POST /")) {
		req += 6;
		post = true;
	} else {
		HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
		return;
	}
	size_t contentLength = 0;
	UserAgent userAgent = ProbeHeaders(req, req_end, &contentLength);
	const char *reqPath_end = (char*)memchr(req, ' ', (uint32_t)(req_end - req));
	uprintf("(%s,%s): %.*s\n", http->encrypt ? "HTTPS" : "HTTP", UserAgent_ToString[userAgent],
		(reqPath_end != NULL) ? (int)(reqPath_end - httpRequest.header) : (int)(httpRequest.header_len), httpRequest.header);
	if(!post && startsWith(req, req_end, "robots.txt")) {
		static const char robots_txt[] = "User-agent: *\nDisallow: /\n";
		HttpContext_respond(http, 200, "text/plain", robots_txt, sizeof(robots_txt));
		return;
	}
	if(userAgent != UserAgent_Web) {
		size_t path_len = strlen(path);
		if(!startsWithBytes(req, req_end, path, path_len * sizeof(*path))) {
			HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
			return;
		}
		req += path_len;
		req += (req < req_end && *req == '/'); // some clients appear to be sending "GET //mp_override.json" requests
		if(post) {
			if(userAgent == UserAgent_Game && startsWith(req, req_end, "beat_saber_get_multiplayer_instance"))
				status_graph(http, httpRequest, master);
			else
				HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
			return;
		}
		static const char mp_override_json[] = "{\"quickPlayAvailablePacksOverride\":{\"predefinedPackIds\":[{\"order\":0,\"packId\":\"ALL_LEVEL_PACKS\"},"
			"{\"order\":1,\"packId\":\"BUILT_IN_LEVEL_PACKS\"}],\"localizedCustomPacks\":[{\"serializedName\":\"customlevels\",\"order\":2,"
			"\"localizedNames\":[{\"language\":0,\"packName\":\"Custom\"}],\"packIds\":[\"custom_levelpack_CustomLevels\"]}]}}";
		if(startsWith(req, req_end, "mp_override.json"))
			HttpContext_respond(http, 200, "application/json; charset=utf-8", mp_override_json, sizeof(mp_override_json));
		else
			status_status(http, userAgent == UserAgent_Game);
		return;
	}
	if(post) {
		/*if(startsWith(req, req_end, "create"))
			status_graph(http, httpRequest, master);
		else*/
			HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
		return;
	}
	if(startsWith(req, req_end, "favicon.ico")) {
		static const uint8_t favicon[] = {0,0,1,0,2,0,32,32,0,0,1,0,24,0,168,12,0,0,38,0,0,0,32,32,2,0,1,0,1,0,48,1,0,0,206,12,0,0,40,0,0,0,32,0,0,0,64,0,0,0,1,0,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,252,31,255,255,240,3,255,255,192,0,255,254,3,240,31,252,15,252,7,240,63,255,131,224,255,255,227,193,255,255,241,199,255,255,241,207,255,255,249,143,255,255,249,143,255,255,249,143,255,255,249,143,255,255,249,142,7,255,249,136,199,255,241,137,143,255,241,143,24,63,241,140,96,7,241,145,131,128,241,158,31,240,49,152,127,254,17,144,255,255,145,139,255,255,227,143,255,255,227,199,255,255,135,193,255,254,15,224,63,248,31,248,7,224,127,255,0,1,255,255,224,7,255,255,252,31,255,40,0,0,0,32,0,0,0,64,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
		HttpContext_respond(http, 200, "image/x-icon", favicon, sizeof(favicon));
		return;
	}
	if(startsWith(req, req_end, "apple-touch-icon")) {
		HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
		return;
	}
	if(httpRequest.header_len > 10) {
		const char *const code_end = memchr(&httpRequest.header[5], ' ', 5);
		status_web(http, StringToServerCode(&httpRequest.header[5], (code_end != NULL) ? (uint32_t)(code_end - &httpRequest.header[5]) : 5));
	}
}
