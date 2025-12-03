#include "internal.h"
#include "status.h"
#include "json.h"
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdatomic.h>

#define READ_SYM(dest, sym) _read_sym(dest, sym, (uint32_t)(sym##_end - sym))
static inline uint32_t _read_sym(char *restrict dest, const uint8_t *restrict sym, uint32_t length) {
	memcpy(dest, sym, length);
	return length;
}

static const uint64_t TEST_maintenanceStartTime = 0;
static const uint64_t TEST_maintenanceEndTime = 0;
static const char TEST_maintenanceMessage[] = "";

__asm__(
		".global head_html;"
	"head_html:"
		".incbin \"src/status/head.html\";"
		".global head_html_end;"
	"head_html_end:");
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

static _Atomic(bool) quietMode = false;
void status_internal_init(const bool quiet) {
	atomic_store(&quietMode, quiet);
	/*uint8_t data[0x200];
	status_update_index(0, data, pkt_write_c((uint8_t*[]){data}, endof(data), PV_WIRE, WireStatusEntry, {
		.code = StringToServerCode("TEST", 4),
		.protocolVersion = 8,
		.public = true,
		.perPlayerDifficulty = true,
		.levelName = String_from("very very very very very very long level name"),
	}));*/
}

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
	[[gnu::nonstring]] static const char table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const div_t length = div((int32_t)buffer->length, 3);
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

static void status_web_index(struct HttpContext *const http) {
	char page[65536];
	uint32_t page_len = READ_SYM(page, head_html);
	page_len += (uint32_t)sprintf(&page[page_len],
		"<style>span#back {display:none}</style>"
		"<table id=index style=width:100%%;table-layout:fixed>"
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
		if(!entry.public) // TODO: make private rooms visible to matching IPs
			continue;
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
			[6] = "1.19.0",
			[7] = "1.19.1",
			[8] = "1.20.0 ⬌ 1.31.1",
			[9] = "1.32.0 ⬌ 1.41.1", // TODO: protocol ABI ranges
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
			"<th><a href=\"", scode, "\">", (entry.protocolVersion < lengthof(protocolNames) && protocolNames[entry.protocolVersion] != NULL) ? protocolNames[entry.protocolVersion] : "???",
			"<th><a href=\"", scode, "\">", noteRate,
			"<th", cover, "><a href=\"", scode, "\"><div>&nbsp;");
	}
	HttpContext_respond(http, 200, "text/html; charset=utf-8", page, page_len);
}

static void status_web_room(struct HttpContext *const http, const ServerCode /*code*/) {
	char page[65536];
	uint32_t page_len = READ_SYM(page, head_html);
	page_len += (uint32_t)sprintf(&page[page_len], "<div id=main>This page is still under construction</div>");
	uprintf("TODO: room status page\n");
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

static UserAgent ProbeHeaders(const char *buf, const char *const end) {
	buf = nextLine(buf, end);
	for(const char *line = buf; line != NULL; line = nextLine(line, end))
		if(startsWith(line, end, "User-Agent: "))
			return startsWith(&line[12], end, "BeatSaberServerBrowser API") ? UserAgent_BSSB : UserAgent_Web;
	for(const char *line = buf; line != NULL; line = nextLine(line, end))
		if(startsWith(line, end, "Accept: "))
			return UserAgent_Web;
	return UserAgent_Game;
}

#ifndef STATUS_APPVER_POSTFIX
#define STATUS_APPVER_POSTFIX ""
#endif

#define PUT(...) (msg_end += (uint32_t)snprintf(msg_end, (msg_end >= endof(msg)) ? 0 : (uint32_t)(endof(msg) - msg_end), __VA_ARGS__))
static void status_status(struct HttpContext *http, bool isGame) {
	char msg[65536], *msg_end = msg;
	PUT("%s%s%s%u%c", "{"
		"\"minimum_app_version\":\"1.19.0", isGame ? "b2147483647" : STATUS_APPVER_POSTFIX, "\","
		"\"maximumAppVersion\":\"1.41.1\","
		"\"status\":", TEST_maintenanceStartTime != 0, ',');
	if(TEST_maintenanceStartTime) {
		PUT("%s%"PRIu64"%s%"PRIu64"%s%"PRIu64"%s%s%s",
			"\"maintenance_start_time\":", TEST_maintenanceStartTime, ","
			"\"maintenance_end_time\":", TEST_maintenanceEndTime, ","
			"\"maintenanceEndTime\":", TEST_maintenanceEndTime, "," // legacy
			"\"user_message\":{\"localizations\":[{\"language\":0,\"message\":\"", TEST_maintenanceMessage, "\"}]},");
	}
	PUT("%s",
		"\"use_ssl\": true,"
		"\"max_players\": 126,"
		"\"supports_pp_modifiers\": true,"
		"\"supports_pp_difficulties\": true,"
		"\"requiredMods\": ["
			"{\"id\":\"BeatUpClient\",\"version\":\"0.4.6\"},"
			"{\"id\":\"MultiplayerCore\",\"version\":\"1.1.1\"},"
			"{\"id\":\"BeatTogether\",\"version\":\"2.0.1\"},"
			"{\"id\":\"BeatSaberPlus_SongOverlay\",\"version\":\"4.6.1\"},"
			"{\"id\":\"_Heck\",\"version\":\"1.4.1\"},"
			"{\"id\":\"NoodleExtensions\",\"version\":\"1.5.1\"},"
			"{\"id\":\"Chroma\",\"version\":\"2.6.1\"},"
			"{\"id\":\"EditorEX\",\"version\":\"1.2.0\"},"
			"{\"id\":\"LeaderboardCore\",\"version\":\"1.2.2\"}"
		"]"
	"}");
	if(msg_end >= endof(msg))
		HttpContext_respond(http, 500, "text/plain; charset=utf-8", NULL, 0);
	else
		HttpContext_respond(http, 200, "application/json; charset=utf-8", msg, (size_t)(msg_end - msg));
}

static float ClampFloat(const float value, const float min, const float max) {
	return (value <= min) ? min : (value >= max) ? max : value;
}

static void status_graph(struct HttpContext *http, struct HttpRequest req, struct WireLink *master) {
	struct GraphConnectCookie state = {
		.cookieType = StatusCookieType_GraphConnect,
		.http = http,
	};
	struct WireGraphConnect connectInfo = {
		.configuration = {
			.shortCountdownMs = 5000,
			.longCountdownMs = 15000,
		},
	};
	struct JsonIterator iter = {(const char*)req.body, (const char*)&req.body[req.body_len], false, {0}};
	JSON_ITER_OBJECT(&iter, key0) {
		if(String_is(key0, "version")) {
			const struct String version = json_read_string(&iter);
			if(version.length < 2 || version.data[0] != '1' || version.data[1] != '.')
				continue;
			const char *end = memchr(version.data, '_', version.length);
			struct String semver = {.length = (end != NULL) ? end - version.data : version.length};
			for(unsigned i = 0; i < semver.length; ++i)
				semver.data[i] = (version.data[i] != '.') ? version.data[i] : '_';
			for(connectInfo.gameVersion = GameVersion_COUNT - 1; strncmp(semver.data, _reflect_GameVersion(connectInfo.gameVersion), semver.length) != 0;) {
				static_assert(GameVersion_Unknown == 0);
				if(--connectInfo.gameVersion == GameVersion_Unknown) {
					uprintf("Unexpected game version: %.*s\n", version.length, version.data);
					break;
				}
			}
			connectInfo.protocolVersion =
				(connectInfo.gameVersion < GameVersion_1_19_1) ? 6 :
				(connectInfo.gameVersion < GameVersion_1_20_0) ? 7 :
				(connectInfo.gameVersion < GameVersion_1_32_0) ? 8 :
				9;
			state.shortMask = (connectInfo.gameVersion != GameVersion_Unknown && connectInfo.gameVersion < GameVersion_1_34_0);
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
					connectInfo.configuration.base.maxPlayerCount = (int32_t)json_read_uint64(&iter);
				else if(String_is(key1, "discovery_policy"))
					connectInfo.configuration.base.discoveryPolicy = (DiscoveryPolicy)json_read_uint64(&iter);
				else if(String_is(key1, "invite_policy"))
					connectInfo.configuration.base.invitePolicy = (InvitePolicy)json_read_uint64(&iter);
				else if(String_is(key1, "gameplay_server_mode"))
					connectInfo.configuration.base.gameplayServerMode = (GameplayServerMode)json_read_uint64(&iter);
				else if(String_is(key1, "song_selection_mode"))
					connectInfo.configuration.base.songSelectionMode = (SongSelectionMode)json_read_uint64(&iter);
				else if(String_is(key1, "gameplay_server_control_settings"))
					connectInfo.configuration.base.gameplayServerControlSettings = (GameplayServerControlSettings)json_read_uint64(&iter);
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
		} else if(String_is(key0, "extra_server_configuration")) {
			uint32_t totalCountdownMs = 0;
			JSON_ITER_OBJECT(&iter, key1) {
				// bool permenant_manager - IGNORED
				// float instance_destroy_timeout - IGNORED
				// string server_name - IGNORED
				if(String_is(key1, "lock_in_beatmap_time"))
					connectInfo.configuration.shortCountdownMs = (uint32_t)(ClampFloat(json_read_float(&iter), 0, 60 * 60 * 24) * 1000);
				else if(String_is(key1, "countdown_time"))
					totalCountdownMs = (uint32_t)(ClampFloat(json_read_float(&iter), 0, 60 * 60 * 24) * 1000);
				else if(String_is(key1, "per_player_modifiers"))
					connectInfo.configuration.perPlayerModifiers = json_read_bool(&iter);
				else if(String_is(key1, "per_player_difficulties"))
					connectInfo.configuration.perPlayerDifficulty = json_read_bool(&iter);
				// bool enable_chroma - IGNORED
				// bool enable_mapping_extensions - IGNORED
				// bool enable_noodle_extensions - IGNORED
				else
					json_skip_any(&iter);
			}
			if(totalCountdownMs == 0)
				totalCountdownMs = 15000 + 5000;
			if(connectInfo.configuration.shortCountdownMs > totalCountdownMs)
				connectInfo.configuration.shortCountdownMs = totalCountdownMs;
			connectInfo.configuration.longCountdownMs = totalCountdownMs - connectInfo.configuration.shortCountdownMs;
		} else {
			json_skip_any(&iter);
		}
	}
	if(iter.fault || connectInfo.gameVersion == GameVersion_Unknown || connectInfo.userId.length == 0 || connectInfo.configuration.base.maxPlayerCount == 0) {
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
		PUT("%s%hhu%s%s%s", "{\"error_code\":", result, ",\"player_session_info\":{\"game_session_id\":\"\",\"port\":-1,\"dns_name\":\"\",\"player_session_id\":\"\","
		    "\"private_game_code\":\"\",\"gameplay_server_configuration\":{\"max_player_count\":5,\"discovery_policy\":1,\"invite_policy\":0,"
		    "\"gameplay_server_mode\":1,\"song_selection_mode\":2,\"gameplay_server_control_settings\":3},\"beatmap_level_selection_mask\":{"
		    "\"difficulties\":31,\"modifiers\":65535,\"song_packs\":\"", packMask, "\"},\"private_game_secret\":\"\"},\"poll_interval_ms\":-1}");
	} else {
		uprintf("TODO: encode songPackMask\n");
		PUT("%s%08x%s%u%s%.*s%s%u%c%03u%s%s%s%.*s%s%hhu%s%u%s%s%s%d%s%d%s%d%s%d%s%d%s%d%s", "{"
			"\"error_code\":0,"
			"\"player_session_info\":{"
				"\"game_session_id\":\"beatupserver:", resp->hostId, "\","
				"\"port\":", resp->endPoint.port, ","
				"\"dns_name\":\"", resp->endPoint.address.length, resp->endPoint.address.data, "\","
				"\"player_session_id\":\"pslot$", resp->roomSlot, ',', resp->playerSlot, "\","
				"\"private_game_code\":\"", ServerCodeToString((char[8]){0}, resp->code), "\","
				"\"private_game_secret\":\"", state->secret.length, state->secret.data, "\","
				"\"beatmap_level_selection_mask\":{"
					"\"difficulties\":", state->selectionMask.difficulties, ","
					"\"modifiers\":", state->selectionMask.modifiers, ","
					"\"song_packs\":\"", packMask, "\""
				"},"
				"\"gameplay_server_configuration\":{"
					"\"max_player_count\":", resp->configuration.maxPlayerCount, ","
					"\"discovery_policy\":", resp->configuration.discoveryPolicy, ","
					"\"invite_policy\":", resp->configuration.invitePolicy, ","
					"\"gameplay_server_mode\":", resp->configuration.gameplayServerMode, ","
					"\"song_selection_mode\":", resp->configuration.songSelectionMode, ","
					"\"gameplay_server_control_settings\":", resp->configuration.gameplayServerControlSettings,
				"}"
			"},"
			"\"poll_interval_ms\":-1"
		"}");
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
	const UserAgent userAgent = ProbeHeaders(req, req_end);
	const char *reqPath_end = (char*)memchr(req, ' ', (uint32_t)(req_end - req));
	if(!atomic_load(&quietMode) || userAgent != UserAgent_BSSB || reqPath_end != req || post)
		uprintf("(%s,%s): %.*s\n", http->encrypt ? "HTTPS" : "HTTP", UserAgent_ToString[userAgent],
			(reqPath_end != NULL) ? (int)(reqPath_end - httpRequest.header) : (int)(httpRequest.header_len), httpRequest.header);
	if(!post && startsWith(req, req_end, "robots.txt")) {
		static const char robots_txt[] = "User-agent: *\nDisallow: /\n";
		HttpContext_respond(http, 200, "text/plain", robots_txt, sizeof(robots_txt) - sizeof(""));
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
	if(req_end - req < 6) {
		HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
		return;
	}
	const char *code_end = memchr(req, '?', 6);
	if(code_end == NULL)
		code_end = memchr(req, ' ', 6);
	if(code_end == req) {
		status_web_index(http);
		return;
	}
	if(code_end == NULL) {
		HttpContext_respond(http, 404, "text/plain; charset=utf-8", NULL, 0);
		return;
	}
	status_web_room(http, StringToServerCode(req, (uint32_t)(code_end - req)));
}
