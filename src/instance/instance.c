#include "instance.h"
#include "common.h"
#include "../wire.h"
#include "../master/master.h"
#include "../counter.h"
#include <mbedtls/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>

#define ENABLE_PASSTHROUGH_ENCRYPTION // the client throws exceptions without this
static const bool AnnotateIDs = false;

#ifdef DEBUG
#define NOT_IMPLEMENTED(type) case type: uprintf(#type " not implemented\n"); abort();
#else
#define NOT_IMPLEMENTED(type) case type: uprintf(#type " not implemented\n"); break;
#endif

typedef uint32_t playerid_t;

#define COUNTER_VAR CONCAT(_i_,__LINE__)

#define FOR_SOME_PLAYERS(id, counter, ...) \
	struct CounterP COUNTER_VAR = (counter); \
	__VA_ARGS__; \
	for(playerid_t id = 0; CounterP_clear_next(&COUNTER_VAR, &(id)); ++(id))

#define FOR_EXCLUDING_PLAYER(id, counter, exc) \
	FOR_SOME_PLAYERS(id, counter, CounterP_clear(&COUNTER_VAR, exc))

#define FOR_ALL_ROOMS(ctx, room) \
	struct Counter64 COUNTER_VAR = (ctx)->roomMask; \
	for(uint32_t group; (group = Counter64_clear_next(&COUNTER_VAR)) != COUNTER64_INVALID;) \
		for(struct Room **(room) = (ctx)->rooms[group]; (room) < endof((ctx)->rooms[group]); ++(room)) \
			if(*room)

struct InstanceSession {
	struct NetSession net;
	struct String secret;
	struct String userName, userId;
	struct PingPong tableTennis;
	struct Channels channels;
	ENetHost *enet;
	struct PlayerStateHash stateHash;
	struct MultiplayerAvatarsData avatars;
	#ifdef ENABLE_PASSTHROUGH_ENCRYPTION
	struct {
		struct Cookie32 random;
		struct ByteArrayNetSerializable publicEncryptionKey;
	} identity;
	#endif
	bool sentIdentity;
	uint32_t chatProtocol;
	uint32_t joinOrder;

	ServerState state;
	time_t recommendTime;
	struct BeatmapIdentifierNetSerializable recommendedBeatmap;
	struct GameplayModifiers recommendedModifiers;
	struct PlayerSpecificSettings settings;
};
struct Room {
	struct NetKeypair keys;
	playerid_t serverOwner;
	struct GameplayServerConfiguration configuration;
	struct timespec syncBase;
	time_t shortCountdown, longCountdown;
	bool skipResults, perPlayerDifficulty, perPlayerModifiers;
	uint32_t joinCount;

	ServerState state;
	struct {
		uint64_t sessionId[2];
		struct CounterP inLobby;
		struct CounterP isSpectating;
		struct BeatmapIdentifierNetSerializable selectedBeatmap;
		struct GameplayModifiers selectedModifiers;
		uint32_t roundRobin;
		time_t timeout;
	} global;

	union {
		struct {
			struct CounterP isEntitled;
			struct CounterP isDownloaded;
			struct CounterP isReady;
			CannotStartGameReason reason;
			playerid_t requester;
			struct {
				struct CounterP missing;
			} entitlement;
		} lobby;
		struct {
			struct CounterP activePlayers;
			time_t startTime;
			bool showResults;
			union {
				struct {
					struct CounterP isLoaded;
				} loadingScene;
				struct {
					struct CounterP isLoaded;
				} loadingSong;
			};
		} game;
	};

	struct CounterP connected;
	struct CounterP playerSort;
	struct InstanceSession players[];
};

struct InstanceContext {
	struct MasterContext base;
	struct NetContext net;
	struct WireContext wire;
	struct WireLink *master;
	struct Counter64 roomMask;
	struct Room *rooms[64][4];
};
static struct InstanceContext *contexts = NULL;

static time_t room_get_syncTime(struct Room *room) {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	
	return (now.tv_sec - room->syncBase.tv_sec) * 1000 + (now.tv_nsec - room->syncBase.tv_nsec) / 1000000;
}

static bool PlayerStateHash_contains(struct PlayerStateHash state, const char *key) {
	uint32_t len = (uint32_t)strlen(key);
	uint32_t hash = 0x21 ^ len;
	int32_t num3 = 0;
	while(len >= 4) {
		uint32_t num4 = ((uint32_t)key[num3 + 3] << 24) | ((uint32_t)key[num3 + 2] << 16) | ((uint32_t)key[num3 + 1] << 8) | (uint32_t)key[num3];
		num4 *= 1540483477;
		num4 ^= num4 >> 24;
		num4 *= 1540483477;
		hash *= 1540483477;
		hash ^= num4;
		num3 += 4;
		len -= 4;
	}
	switch(len) {
		case 3: hash ^= (uint32_t)key[num3 + 2] << 16; [[fallthrough]];
		case 2: hash ^= (uint32_t)key[num3 + 1] << 8; [[fallthrough]];
		case 1: hash ^= (uint32_t)key[num3];
		hash *= 1540483477; [[fallthrough]];
		case 0: break;
	}
	hash ^= hash >> 13;
	hash *= 1540483477;
	hash ^= (hash >> 15);
	for(uint_fast8_t i = 0; i < 3; ++i) {
		uint_fast8_t ind = (hash % 128);
		if(!(((ind >= 64) ? state.bloomFilter.d0 >> (ind - 64) : state.bloomFilter.d1 >> ind) & 1))
			return false;
		hash >>= 8;
	}
	return true;
}

static uint8_t InstanceSession_connectionId(const struct InstanceSession *list, const struct InstanceSession *session) {
	uint8_t index = (uint8_t)indexof(list, session);
	return index + 1 + (index >= 126);
}

static uint8_t ConnectionId_index(uint8_t connectionId) {
	return connectionId - 1 - (connectionId >= 128);
}

static bool BeatmapIdentifier_eq(const struct BeatmapIdentifierNetSerializable *a, const struct BeatmapIdentifierNetSerializable *b, bool ignoreDifficulty) {
	if(!LongString_eq(a->levelID, b->levelID))
		return false;
	return ignoreDifficulty || (String_eq(a->characteristic, b->characteristic) && a->difficulty == b->difficulty);
}

static bool GameplayModifiers_eq(const struct GameplayModifiers *a, const struct GameplayModifiers *b, bool optional) {
	struct GameplayModifiers delta = {a->raw ^ b->raw};
	struct GameplayModifiers mask = {REQUIRED_MODIFIER_MASK};
	mask.raw |= optional * UINT32_C(0xffffffff);
	return (delta.raw & mask.raw) == 0;
}

static struct String OwnUserId(struct String userId) {
	const char *userId_end = memchr(userId.data, '$', userId.length);
	if(userId_end != NULL)
		userId.length = (uint32_t)(userId_end - userId.data);
	return userId;
}

static inline struct PlayerLobbyPermissionConfiguration session_get_permissions(const struct Room *room, const struct InstanceSession *session, bool self) {
	bool serverOwner = (indexof(room->players, session) == room->serverOwner);
	bool canSuggest = (room->configuration.songSelectionMode != SongSelectionMode_Random);
	return (struct PlayerLobbyPermissionConfiguration){
		.userId = self ? OwnUserId(session->userId) : session->userId,
		.serverOwner = serverOwner,
		.recommendBeatmaps = canSuggest,
		.recommendModifiers = canSuggest && (room->configuration.gameplayServerControlSettings & GameplayServerControlSettings_AllowModifierSelection),
		.kickVote = serverOwner,
		.invite = (bool[]){true, serverOwner, false}[room->configuration.invitePolicy],
	};
}

static struct BeatmapIdentifierNetSerializable session_get_beatmap(const struct Room *room, const struct InstanceSession *session) {
	if(room->perPlayerDifficulty && BeatmapIdentifier_eq(&session->recommendedBeatmap, &room->global.selectedBeatmap, 1))
		return session->recommendedBeatmap;
	return room->global.selectedBeatmap;
}

static struct GameplayModifiers session_get_modifiers(const struct Room *room, const struct InstanceSession *session) {
	if(room->perPlayerModifiers && GameplayModifiers_eq(&session->recommendedModifiers, &room->global.selectedModifiers, false))
		return session->recommendedModifiers;
	return room->global.selectedModifiers;
}

static uint32_t instance_mapPool_len = 0;
static struct MpBeatmapPacket *instance_mapPool = NULL;
static void mapPool_init(const char *filename) {
	FILE *file = fopen(filename, "rb");
	if(file == NULL) {
		uprintf("Failed to open %s: %s\n", filename, strerror(errno));
		return;
	}
	fseek(file, 0, SEEK_END);
	size_t raw_len = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	uint8_t raw[3145728], *raw_end = &raw[raw_len];
	if(raw_len > sizeof(raw)) {
		uprintf("Failed to read %s: File too large\n", filename);
		goto fail;
	}
	if(fread(raw, 1, raw_len, file) != raw_len) {
		uprintf("Failed to read %s\n", filename);
		goto fail;
	}
	uint16_t formatVersion = raw[4] | (uint16_t)(raw[5] << 8);
	if(raw_len < 7 || memcmp(raw, "BSmp", 4) || formatVersion == 0) {
		uprintf("Failed to read %s: not a BeatUpServer map pool file\n", filename);
		goto fail;
	}
	if(formatVersion != 1) {
		uprintf("Failed to read %s: unsupported format\n", filename);
		goto fail;
	}
	const uint8_t *raw_it = &raw[6];
	for(instance_mapPool_len = 0; raw_it < raw_end; ++instance_mapPool_len)
		if(!pkt_read(&(struct MpBeatmapPacket){.difficulty=0}, &raw_it, raw_end, (struct PacketContext){.netVersion = 12}))
			break;
	if(raw_it != raw_end) {
		uprintf("Failed to read %s: corrupt data\n", filename);
		goto fail;
	}
	instance_mapPool = calloc(instance_mapPool_len, sizeof(*instance_mapPool));
	if(!instance_mapPool) {
		uprintf("alloc error\n");
		goto fail;
	}
	raw_it = &raw[6];
	for(uint32_t i = 0; i < instance_mapPool_len; ++i)
		pkt_read(&instance_mapPool[i], &raw_it, raw_end, (struct PacketContext){.netVersion = 12});
	return;
	fail:
	fclose(file);
}

static void mapPool_update(struct Room *room) {
	if(!instance_mapPool) {
		uprintf("No map pool to select from!\n");
		return;
	}
	room->lobby.requester = (playerid_t)room->configuration.maxPlayerCount;
	room->global.selectedBeatmap = (struct BeatmapIdentifierNetSerializable){
		.levelID = LongString_from("custom_level_"),
		.characteristic = instance_mapPool[room->global.roundRobin].characteristic,
		.difficulty = instance_mapPool[room->global.roundRobin].difficulty,
	};
	room->global.selectedModifiers.raw = GameplayModifierFlags_NoFailOn0Energy;
	const struct String *levelHash = &instance_mapPool[room->global.roundRobin].levelHash;
	memcpy(&room->global.selectedBeatmap.levelID.data[13], levelHash->data, levelHash->length);
	room->global.selectedBeatmap.levelID.length += levelHash->length;

	room->players[room->lobby.requester].recommendedBeatmap = room->global.selectedBeatmap;
	room->players[room->lobby.requester].recommendedModifiers = room->global.selectedModifiers;
}

static uint32_t roundRobin_next(uint32_t prev, struct CounterP players) {
	if(CounterP_isEmpty(players))
		return (prev + 1) % (instance_mapPool_len ? instance_mapPool_len : 1);
	FOR_SOME_PLAYERS(id, players,)
		if(id > prev)
			return id;
	playerid_t id = 0;
	if(!CounterP_get(players, 0))
		CounterP_clear_next(&players, &id);
	return id;
}

static struct UTimestamp UTimestamp_FromTime(time_t time) {
	return (struct UTimestamp){
		.legacy = (float)((double)time / 1000.),
		.value = (uint64_t)((time < 0) ? 0 : time),
	};
}

static struct STimestamp STimestamp_FromTime(time_t time) {
	return (struct STimestamp){
		.legacy = (float)((double)time / 1000.),
		.value = (int64_t)time,
	};
}

static struct STimestamp room_get_countdownEnd(const struct Room *room, struct UTimestamp defaultTime) {
	switch(room->state) {
		case ServerState_Lobby_LongCountdown: return STimestamp_FromTime(room->global.timeout + room->shortCountdown);
		case ServerState_Lobby_ShortCountdown: return STimestamp_FromTime(room->global.timeout);
		default: return (struct STimestamp){defaultTime.legacy, (int64_t)defaultTime.value};
	}
}

static void instance_send(struct InstanceContext *ctx, struct InstanceSession *session, const uint8_t *resp, uint32_t resp_len, bool reliable) {
	if(session->enet != NULL)
		eenet_send(session->enet, resp, resp_len, reliable);
	else if(reliable)
		instance_send_channeled(&session->net, &session->channels, resp, resp_len, DeliveryMethod_ReliableOrdered);
	else
		net_queue_merged(&ctx->net, &session->net, resp, (uint16_t)resp_len, &(const struct NetPacketHeader){PacketProperty_Unreliable, 0, 0, {{0}}});
}

#define STATE_EDGE(from, to, mask) ((to & (mask)) && !(from & (mask)))
static bool room_try_finish(struct InstanceContext *ctx, struct Room *room);
static void session_set_state(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, ServerState state) {
	struct RemoteProcedureCall base = {
		.syncTime = UTimestamp_FromTime(room_get_syncTime(room)),
	};
	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
	uint8_t *start = resp_end;
	if(STATE_EDGE(session->state, state, ServerState_Connected)) {
		CounterP_set(&room->connected, (uint32_t)indexof(room->players, session));
	} else if(STATE_EDGE(state, session->state, ServerState_Connected)) {
		CounterP_clear(&room->connected, (uint32_t)indexof(room->players, session));
		if(room->configuration.songSelectionMode != SongSelectionMode_Random && room->global.roundRobin == indexof(room->players, session))
			room->global.roundRobin = roundRobin_next(room->global.roundRobin, room->connected);
		struct InternalMessage r_disconnect = {
			.type = InternalMessageType_PlayerDisconnected,
			.playerDisconnected.disconnectedReason = DisconnectedReason_ClientConnectionClosed,
		};
		FOR_SOME_PLAYERS(id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {InstanceSession_connectionId(room->players, session), 0, false, 0});
			if(pkt_serialize(&r_disconnect, &resp_end, endof(resp), room->players[id].net.version))
				instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
		}
		if(room->state & ServerState_Game) {
			CounterP_clear(&room->game.activePlayers, (uint32_t)indexof(room->players, session));
			room_try_finish(ctx, room);
		}
	}
	if(state & ServerState_Lobby) {
		bool needSetSelectedBeatmap = (state & ServerState_Lobby_Entitlement) != 0;
		if(!(session->state & ServerState_Lobby)) {
			needSetSelectedBeatmap = true;
			session->recommendedBeatmap = CLEAR_BEATMAP;
			SERIALIZE_GAMEPLAYRPC(&resp_end, endof(resp), session->net.version, {
				.type = GameplayRpcType_ReturnToMenu,
				.returnToMenu.base = base,
			});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetMultiplayerGameState,
				.setMultiplayerGameState = {
					.base = base,
					.flags = {true, false, false, false},
					.lobbyState = MultiplayerGameState_Lobby,
				},
			});
		} else if(STATE_EDGE(state, session->state, ServerState_Countdown | ServerState_Lobby_Downloading)) {
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_CancelCountdown,
				.cancelCountdown.base = base,
			});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_CancelLevelStart,
				.cancelLevelStart.base = base,
			});
		}
		if(needSetSelectedBeatmap) {
			if(room->configuration.songSelectionMode == SongSelectionMode_Random && instance_mapPool) {
				SERIALIZE_MPCORE(&resp_end, endof(resp), session->net.version, {
					.type = String_from("MpBeatmapPacket"),
					.mpBeatmap = instance_mapPool[room->global.roundRobin],
				});
			}
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = room->global.selectedBeatmap.levelID.length ? MenuRpcType_SetSelectedBeatmap : MenuRpcType_ClearSelectedBeatmap,
				.setSelectedBeatmap = {
					.base = base,
					.flags = {true, false, false, false},
					.identifier = session_get_beatmap(room, session),
				},
			});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetSelectedGameplayModifiers,
				.setSelectedGameplayModifiers = {
					.base = base,
					.flags = {true, false, false, false},
					.gameplayModifiers = session_get_modifiers(room, session),
				},
			});
		}
		if(STATE_EDGE(session->state, state, ServerState_Countdown | ServerState_Lobby_Downloading)) {
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_StartLevel,
				.startLevel = {
					.base = base,
					.flags = {true, true, true, false},
					.beatmapId = session_get_beatmap(room, session),
					.gameplayModifiers = session_get_modifiers(room, session),
					.startTime = STimestamp_FromTime(room->global.timeout + 1048576000),
				},
			});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, { // TODO: this does nothing if `newTime` is within 1.5 seconds of the client's `predictedCountdownEndTime`
				.type = MenuRpcType_SetCountdownEndTime,
				.setCountdownEndTime = {
					.base = base,
					.flags = {true, false, false, false},
					.newTime = room_get_countdownEnd(room, base.syncTime),
				},
			});
		} else if(state & ServerState_Countdown) { // TODO: less copy+paste
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_CancelCountdown,
				.cancelCountdown.base = base,
			});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetCountdownEndTime,
				.setCountdownEndTime = {
					.base = base,
					.flags = {true, false, false, false},
					.newTime = room_get_countdownEnd(room, base.syncTime),
				},
			});
		}
	} else if(STATE_EDGE(session->state, state, ServerState_Game)) {
		SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
			.type = MenuRpcType_SetMultiplayerGameState,
			.setMultiplayerGameState = {
				.base = base,
				.flags = {true, false, false, false},
				.lobbyState = MultiplayerGameState_Game,
			},
		});
	}
	switch(state) {
		case ServerState_Lobby_Entitlement: {
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_GetIsEntitledToLevel,
				.getIsEntitledToLevel = {
					.base = base,
					.flags = {true, false, false, false},
					.levelId = room->global.selectedBeatmap.levelID,
				},
			});
			if(session->net.version.beatUpVersion) {
				SERIALIZE_BEATUP(&resp_end, endof(resp), session->net.version, {
					.type = BeatUpMessageType_ShareInfo,
					.shareInfo = {
						.meta.byteLength = 0,
						.id = {
							.usage = ShareableType_BeatmapSet,
							.name = room->global.selectedBeatmap.levelID,
						},
					},
				});
			}
		} [[fallthrough]];
		case ServerState_Lobby_Idle:
		case ServerState_Lobby_Ready: {
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetIsStartButtonEnabled,
				.setIsStartButtonEnabled = {
					.base = base,
					.flags = {true, false, false, false},
					.reason = room->lobby.reason,
				},
			});
			break;
		}
		case ServerState_Lobby_LongCountdown: break;
		case ServerState_Lobby_ShortCountdown: break;
		case ServerState_Lobby_Downloading: break;
		case ServerState_Game_LoadingScene: {
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetStartGameTime,
				.setStartGameTime = {
					.base = base,
					.flags = {true, false, false, false},
					.newTime = (struct STimestamp){base.syncTime.legacy, (int64_t)base.syncTime.value},
				},
			});
			SERIALIZE_GAMEPLAYRPC(&resp_end, endof(resp), session->net.version, {
				.type = GameplayRpcType_GetGameplaySceneReady,
				.getGameplaySceneReady.base = base,
			});
			break;
		}
		case ServerState_Game_LoadingSong: {
			struct GameplayRpc r_sync;
			struct PlayerSpecificSettingsAtStart *settings;
			struct String *id;
			bool active = CounterP_get(room->game.activePlayers, (uint32_t)indexof(room->players, session));
			if(active) {
				r_sync = (struct GameplayRpc){
					.type = GameplayRpcType_SetGameplaySceneSyncFinish,
					.setGameplaySceneSyncFinish = {
						.base = base,
						.flags = {true, true, false, false},
						.settings.count = 0,
					},
				};
				settings = &r_sync.setGameplaySceneSyncFinish.settings;
				id = &r_sync.setGameplaySceneSyncFinish.sessionGameId;
			} else {
				r_sync = (struct GameplayRpc){
					.type = GameplayRpcType_SetActivePlayerFailedToConnect,
					.setActivePlayerFailedToConnect = {
						.base = base,
						.flags = {true, true, true, false},
						.failedUserId = OwnUserId(session->userId),
						.settings.count = 0,
					},
				};
				settings = &r_sync.setActivePlayerFailedToConnect.settings;
				id = &r_sync.setActivePlayerFailedToConnect.sessionGameId;
			}
			*id = String_fmt("%08"PRIx64"-%04"PRIx64"-%04"PRIx64"-%04"PRIx64"-%012"PRIx64,
				room->global.sessionId[0] >> 32 & 0xffffffff,
				room->global.sessionId[0] >> 16 & 0xffff,
				room->global.sessionId[0]       & 0xffff,
				room->global.sessionId[1] >> 48 & 0xffff,
				room->global.sessionId[1]       & 0xffffffffffff);
			FOR_SOME_PLAYERS(id, room->game.activePlayers,) {
				settings->players[settings->count] = room->players[id].settings;
				settings->players[settings->count++].userId = (&room->players[id] == session) ? OwnUserId(session->userId) : room->players[id].userId;
			}
			if(active) {
				SERIALIZE_GAMEPLAYRPC(&resp_end, endof(resp), session->net.version, {
					.type = GameplayRpcType_GetGameplaySongReady,
					.getGameplaySongReady.base = base,
				});
			}
			SERIALIZE_GAMEPLAYRPC(&resp_end, endof(resp), session->net.version, r_sync);
			if(active)
				break;
			state = ServerState_Game_Gameplay;
		} [[fallthrough]];
		case ServerState_Game_Gameplay: {
			SERIALIZE_GAMEPLAYRPC(&resp_end, endof(resp), session->net.version, {
				.type = GameplayRpcType_SetSongStartTime,
				.setSongStartTime = {
					.base = base,
					.flags = {true, false, false, false},
					.startTime = STimestamp_FromTime(room->game.startTime),
				},
			});
			break;
		}
		case ServerState_Game_Results: break;
	}
	if(resp_end != start)
		instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
	session->state = state;
}

static const char *ServerState_toString(ServerState state) {
	switch(state) {
		case 0: return "(none)";
		case ServerState_Lobby_Idle: return "Lobby.Idle";
		case ServerState_Lobby_Entitlement: return "Lobby.Entitlement";
		case ServerState_Lobby_Ready: return "Lobby.Ready";
		case ServerState_Lobby_LongCountdown: return "Lobby.LongCountdown";
		case ServerState_Lobby_ShortCountdown: return "Lobby.ShortCountdown";
		case ServerState_Lobby_Downloading: return "Lobby.Downloading";
		case ServerState_Game_LoadingScene: return "Game.LoadingScene";
		case ServerState_Game_LoadingSong: return "Game.LoadingSong";
		case ServerState_Game_Gameplay: return "Game.Gameplay";
		case ServerState_Game_Results: return "Game.Results";
		default:;
	}
	return "???";
}

[[gnu::format(printf, 4, 0)]] static void chat(struct InstanceContext *const ctx, struct Room *const room, const struct InstanceSession *const session, const char *const format, ...);
static void room_set_state(struct InstanceContext *ctx, struct Room *room, ServerState state) {
	uprintf("state %s -> %s\n", ServerState_toString(room->state), ServerState_toString(state));
	if(STATE_EDGE(room->state, state, ServerState_Lobby)) {
		room->global.selectedBeatmap = CLEAR_BEATMAP;
		room->global.selectedModifiers = CLEAR_MODIFIERS;
		room->lobby.isEntitled = COUNTERP_CLEAR;
		room->lobby.isDownloaded = COUNTERP_CLEAR;
		room->lobby.isReady = COUNTERP_CLEAR;
		room->lobby.reason = CannotStartGameReason_NoSongSelected;
		room->lobby.requester = (playerid_t)~0u;
		if(room->configuration.songSelectionMode == SongSelectionMode_Random) {
			mapPool_update(room);
			state = ServerState_Lobby_Entitlement;
			uprintf("state %s -> %s\n", ServerState_toString(room->state), ServerState_toString(state));
		}
	} else if(STATE_EDGE(room->state, state, ServerState_Game)) {
		room->game.activePlayers = room->connected;
		room->game.showResults = false;
	}
	switch(state) {
		case ServerState_Lobby_Entitlement: {
			playerid_t select = ~UINT32_C(0);
			switch(room->configuration.songSelectionMode) {
				case SongSelectionMode_Vote: {
					uint32_t max = 0;
					FOR_SOME_PLAYERS(id, room->connected,) {
						if(!room->players[id].recommendedBeatmap.characteristic.length)
							continue;
						uint32_t biasedVotes = (id >= room->global.roundRobin) + 1;
						time_t requestTime = room->players[id].recommendTime;
						playerid_t firstRequest = id;
						FOR_EXCLUDING_PLAYER(cmp, room->connected, id) { // TODO: this scales horribly
							if(!BeatmapIdentifier_eq(&room->players[id].recommendedBeatmap, &room->players[cmp].recommendedBeatmap, room->perPlayerDifficulty))
								continue;
							++biasedVotes;
							if(room->players[cmp].recommendTime >= requestTime)
								continue;
							requestTime = room->players[cmp].recommendTime;
							firstRequest = cmp;
						}
						if(biasedVotes <= max)
							continue;
						max = biasedVotes;
						select = firstRequest;
					}
					break;
				}
				case SongSelectionMode_Random: select = (playerid_t)room->configuration.maxPlayerCount; break;
				case SongSelectionMode_OwnerPicks: select = room->serverOwner; break;
				case SongSelectionMode_RandomPlayerPicks: select = room->global.roundRobin; break;
			}
			room->lobby.requester = select;
			if(select > (playerid_t)room->configuration.maxPlayerCount || room->players[select].recommendedBeatmap.characteristic.length == 0) {
				uprintf("    no selection from user \"%.*s\"\n", room->players[select].userName.length, room->players[select].userName.data);
				room->lobby.isEntitled = room->connected;
				room->global.selectedBeatmap = CLEAR_BEATMAP;
				room->global.selectedModifiers = CLEAR_MODIFIERS;
				room_set_state(ctx, room, ServerState_Lobby_Idle);
				return;
			}
			const bool cached = (room->state & ServerState_Selected) && CounterP_contains(room->lobby.isEntitled, room->connected) &&
				BeatmapIdentifier_eq(&room->players[select].recommendedBeatmap, &room->global.selectedBeatmap, 0);
			uprintf("    selecting [levelID=\"%.*s\" characteristic=\"%.*s\" difficulty=%s] from user \"%.*s\": %s\n",
				room->players[select].recommendedBeatmap.levelID.length, room->players[select].recommendedBeatmap.levelID.data,
				room->players[select].recommendedBeatmap.characteristic.length, room->players[select].recommendedBeatmap.characteristic.data,
				reflect(BeatmapDifficulty, room->players[select].recommendedBeatmap.difficulty),
				room->players[select].userName.length, room->players[select].userName.data,
				cached ? "cached" : "new");
			if(cached)
				return;
			room->lobby.reason = 0;
			room->lobby.isEntitled = COUNTERP_CLEAR;
			room->lobby.isDownloaded = COUNTERP_CLEAR;
			room->global.selectedBeatmap = room->players[select].recommendedBeatmap;
			room->global.selectedModifiers = room->players[select].recommendedModifiers;
			room->lobby.entitlement.missing = COUNTERP_CLEAR;
			break;
		}
		case ServerState_Lobby_Idle: {
			room->lobby.reason = room->global.selectedBeatmap.characteristic.length ? CannotStartGameReason_DoNotOwnSong : CannotStartGameReason_NoSongSelected;
			break;
		}
		case ServerState_Lobby_Ready: {
			room->lobby.reason = CannotStartGameReason_None;
			if(CounterP_containsNone(room->global.inLobby, room->connected)) {
				room->lobby.reason = CannotStartGameReason_AllPlayersNotInLobby;
				break;
			}
			if(CounterP_contains(room->global.isSpectating, room->connected)) {
				room->lobby.reason = CannotStartGameReason_AllPlayersSpectating;
				break;
			}
			bool shouldCountdown = CounterP_contains(CounterP_or(room->lobby.isReady, room->global.isSpectating), room->connected);
			shouldCountdown |= CounterP_get(room->lobby.isReady, room->serverOwner);
			if(!shouldCountdown)
				break;
			room_set_state(ctx, room, ServerState_Lobby_LongCountdown);
			return;
		}
		case ServerState_Lobby_LongCountdown: {
			if(CounterP_contains(CounterP_or(room->lobby.isReady, room->global.isSpectating), room->connected)) {
				room_set_state(ctx, room, ServerState_Lobby_ShortCountdown);
				return;
			} else if((room->state & ServerState_Selected) >= ServerState_Lobby_LongCountdown) {
				return;
			}
			room->global.timeout = room_get_syncTime(room) + room->longCountdown;
			break;
		}
		case ServerState_Lobby_ShortCountdown: {
			if((room->state & ServerState_Selected) >= ServerState_Lobby_ShortCountdown)
				return;
			room->global.timeout = room_get_syncTime(room) + room->shortCountdown;
			break;
		}
		case ServerState_Lobby_Downloading: {
			if(CounterP_contains(room->lobby.isDownloaded, room->connected)) {
				room_set_state(ctx, room, ServerState_Game_LoadingScene);
				return;
			}
			break;
		}
		case ServerState_Game_LoadingScene: {
			room->game.loadingScene.isLoaded = COUNTERP_CLEAR;
			room->global.timeout = room_get_syncTime(room) + LOAD_TIMEOUT_MS;
			break;
		}
		case ServerState_Game_LoadingSong: {
			if(room->state & ServerState_Game_LoadingScene) {
				room->game.activePlayers = CounterP_and(room->game.activePlayers, room->game.loadingScene.isLoaded);
				if(room_try_finish(ctx, room)) {
					chat(ctx, room, NULL, "All players failed to load gameplay scene");
					return;
				}
			}
			mbedtls_ctr_drbg_random(&ctx->net.ctr_drbg, (uint8_t*)room->global.sessionId, sizeof(room->global.sessionId));
			room->game.loadingSong.isLoaded = COUNTERP_CLEAR;
			room->global.timeout = room_get_syncTime(room) + LOAD_TIMEOUT_MS;
			break;
		}
		case ServerState_Game_Gameplay: {
			if(room->state & ServerState_Game_LoadingSong) {
				room->game.activePlayers = CounterP_and(room->game.activePlayers, room->game.loadingSong.isLoaded);
				if(room_try_finish(ctx, room)) {
					chat(ctx, room, NULL, "All players failed to load song");
					return;
				}
			}
			room->game.startTime = room_get_syncTime(room) + 250;
			break;
		}
		case ServerState_Game_Results: room->global.timeout = room_get_syncTime(room) + (room->game.showResults ? 20000 : 1000); break;
	}
	room->state = state;
	FOR_SOME_PLAYERS(id, room->connected,)
		session_set_state(ctx, room, &room->players[id], state);
}

static struct PlayersLobbyPermissionConfiguration room_get_permissions(const struct Room *room, struct CounterP set, const struct InstanceSession *self) {
	struct PlayersLobbyPermissionConfiguration out = {0};
	FOR_SOME_PLAYERS(id, set,)
		out.playersPermission[out.count++] = session_get_permissions(room, &room->players[id], &room->players[id] == self);
	return out;
}

static void handle_MenuRpc(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct MenuRpc *rpc) {
	switch(rpc->type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: uprintf("BAD TYPE: MenuRpcType_SetPlayersMissingEntitlementsToLevel\n"); break;
		case MenuRpcType_GetIsEntitledToLevel: uprintf("BAD TYPE: MenuRpcType_GetIsEntitledToLevel\n"); break;
		case MenuRpcType_SetIsEntitledToLevel: {
			struct SetIsEntitledToLevel entitlement = rpc->setIsEntitledToLevel;
			if(!((room->state & ServerState_Lobby) && entitlement.flags.hasValue0 && LongString_eq(entitlement.levelId, room->global.selectedBeatmap.levelID)))
				break;
			if(!entitlement.flags.hasValue1 || entitlement.entitlementStatus == EntitlementsStatus_Unknown) {
				entitlement.entitlementStatus = EntitlementsStatus_NotOwned;
			} else if(entitlement.entitlementStatus == EntitlementsStatus_Ok) {
				if(!PlayerStateHash_contains(session->stateHash, "modded") && entitlement.levelId.length >= 13 && memcmp(entitlement.levelId.data, "custom_level_", 13) == 0)
					entitlement.entitlementStatus = EntitlementsStatus_NotOwned; // Vanilla clients will misreport all custom IDs as owned
				else if(CounterP_set(&room->lobby.isDownloaded, (uint32_t)indexof(room->players, session)) == 0)
					if((room->state & ServerState_Lobby_Downloading) && CounterP_contains(room->lobby.isDownloaded, room->connected))
						room_set_state(ctx, room, ServerState_Game_LoadingScene);
			}
			if(!(room->state & ServerState_Lobby_Entitlement))
				break;
			if(CounterP_set(&room->lobby.isEntitled, (uint32_t)indexof(room->players, session)))
				break;
			if(entitlement.entitlementStatus != EntitlementsStatus_Ok && entitlement.entitlementStatus != EntitlementsStatus_NotDownloaded)
				CounterP_set(&room->lobby.entitlement.missing, (uint32_t)indexof(room->players, session));
			uprintf("entitlement[%.*s (%.*s)]: %s\n", session->userName.length, session->userName.data, session->userId.length, session->userId.data, reflect(EntitlementsStatus, entitlement.entitlementStatus));
			if(!CounterP_contains(room->lobby.isEntitled, room->connected))
				break;
			struct MenuRpc r_missing = {
				.type = MenuRpcType_SetPlayersMissingEntitlementsToLevel,
				.setPlayersMissingEntitlementsToLevel = {
					.base.syncTime = UTimestamp_FromTime(room_get_syncTime(room)),
					.flags = {true, false, false, false},
					.count = 0,
				},
			};
			FOR_SOME_PLAYERS(id, room->lobby.entitlement.missing,)
				r_missing.setPlayersMissingEntitlementsToLevel.players[r_missing.setPlayersMissingEntitlementsToLevel.count++] = room->players[id].userId;
			FOR_SOME_PLAYERS(id, room->connected,) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 127, false, 0});
				SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, r_missing);
				instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
			}
			if(r_missing.setPlayersMissingEntitlementsToLevel.count == 0) {
				room_set_state(ctx, room, ServerState_Lobby_Ready);
			} else if(room->configuration.songSelectionMode == SongSelectionMode_Random) {
				room->global.roundRobin = roundRobin_next(room->global.roundRobin, COUNTERP_CLEAR);
				mapPool_update(room);
				room_set_state(ctx, room, ServerState_Lobby_Entitlement);
			} else {
				room_set_state(ctx, room, ServerState_Lobby_Idle);
			}
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_InvalidateLevelEntitlementStatuses);
		NOT_IMPLEMENTED(MenuRpcType_SelectLevelPack);
		case MenuRpcType_SetSelectedBeatmap: uprintf("BAD TYPE: MenuRpcType_SetSelectedBeatmap\n"); break;
		case MenuRpcType_GetSelectedBeatmap: {
			struct RemoteProcedureCall base = {UTimestamp_FromTime(room_get_syncTime(room))};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = room->global.selectedBeatmap.levelID.length ? MenuRpcType_SetSelectedBeatmap : MenuRpcType_ClearSelectedBeatmap,
				.setSelectedBeatmap = {
					.base = base,
					.flags = {true, false, false, false},
					.identifier = session_get_beatmap(room, session),
				},
			});
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			break;
		}
		case MenuRpcType_RecommendBeatmap: {
			struct RecommendBeatmap beatmap = rpc->recommendBeatmap;
			if(!beatmap.flags.hasValue0) {
				[[fallthrough]]; case MenuRpcType_ClearRecommendedBeatmap:
				beatmap.identifier = CLEAR_BEATMAP;
			}
			if(!((room->state & ServerState_Lobby) && session_get_permissions(room, session, false).recommendBeatmaps))
				break;
			if(!BeatmapIdentifier_eq(&session->recommendedBeatmap, &beatmap.identifier, room->perPlayerDifficulty))
				session->recommendTime = room_get_syncTime(room);
			session->recommendedBeatmap = beatmap.identifier;
			room_set_state(ctx, room, ServerState_Lobby_Entitlement);
			break;
		}
		case MenuRpcType_GetRecommendedBeatmap: break;
		case MenuRpcType_SetSelectedGameplayModifiers: uprintf("BAD TYPE: MenuRpcType_SetSelectedGameplayModifiers\n"); break;
		case MenuRpcType_GetSelectedGameplayModifiers: {
			struct RemoteProcedureCall base = {UTimestamp_FromTime(room_get_syncTime(room))};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetSelectedGameplayModifiers,
				.setSelectedGameplayModifiers = {
					.base = base,
					.flags = {true, false, false, false},
					.gameplayModifiers = session_get_modifiers(room, session),
				},
			});
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			break;
		}
		case MenuRpcType_RecommendGameplayModifiers: {
			struct RecommendGameplayModifiers modifiers = rpc->recommendGameplayModifiers;
			if(!modifiers.flags.hasValue0) {
				[[fallthrough]]; case MenuRpcType_ClearRecommendedGameplayModifiers:
				modifiers.gameplayModifiers = CLEAR_MODIFIERS;
			}
			if(!((room->state & ServerState_Lobby) && session_get_permissions(room, session, false).recommendModifiers))
				break;
			session->recommendedModifiers = modifiers.gameplayModifiers;
			if(indexof(room->players, session) != room->lobby.requester)
				break;
			room->global.selectedModifiers = modifiers.gameplayModifiers;
			struct RemoteProcedureCall base = {UTimestamp_FromTime(room_get_syncTime(room))};
			FOR_SOME_PLAYERS(id, room->connected,) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 127, false, 0});
				SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
					.type = MenuRpcType_SetSelectedGameplayModifiers,
					.setSelectedGameplayModifiers = {
						.base = base,
						.flags = {true, false, false, false},
						.gameplayModifiers = session_get_modifiers(room, session),
					},
				});
				instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
			}
			break;
		}
		case MenuRpcType_GetRecommendedGameplayModifiers: break;
		NOT_IMPLEMENTED(MenuRpcType_LevelLoadError);
		NOT_IMPLEMENTED(MenuRpcType_LevelLoadSuccess);
		case MenuRpcType_StartLevel: uprintf("BAD TYPE: MenuRpcType_StartLevel\n"); break;
		case MenuRpcType_GetStartedLevel: {
			if(!(session->state & ServerState_Synchronizing)) {
				break;
			} else if(!(room->state & ServerState_Game)) {
				session_set_state(ctx, room, session, room->state);
				if(room->configuration.songSelectionMode == SongSelectionMode_Random && (room->state & ServerState_Lobby))
					room_set_state(ctx, room, ServerState_Lobby_Entitlement);
				break;
			}
			struct RemoteProcedureCall base = {UTimestamp_FromTime(room_get_syncTime(room))};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_StartLevel,
				.startLevel = {
					.base = base,
					.flags = {true, true, true, false},
					.beatmapId = session_get_beatmap(room, session),
					.gameplayModifiers = session_get_modifiers(room, session),
					.startTime = (struct STimestamp){base.syncTime.legacy, (int64_t)base.syncTime.value},
				},
			});
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			session_set_state(ctx, room, session, ServerState_Game_LoadingScene);
			break;
		}
		case MenuRpcType_CancelLevelStart: uprintf("BAD TYPE: MenuRpcType_CancelLevelStart\n"); break;
		case MenuRpcType_GetMultiplayerGameState: {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetMultiplayerGameState,
				.setMultiplayerGameState = {
					.base.syncTime = UTimestamp_FromTime(room_get_syncTime(room)),
					.flags = {true, false, false, false},
					.lobbyState = (room->state & ServerState_Lobby) ? MultiplayerGameState_Lobby : MultiplayerGameState_Game,
				},
			});
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			break;
		}
		case MenuRpcType_SetMultiplayerGameState: uprintf("BAD TYPE: MenuRpcType_SetMultiplayerGameState\n"); break;
		case MenuRpcType_GetIsReady: break;
		case MenuRpcType_SetIsReady: {
			bool isReady = rpc->setIsReady.flags.hasValue0 && rpc->setIsReady.isReady;
			if(room->state & ServerState_Lobby)
				if(CounterP_overwrite(&room->lobby.isReady, (uint32_t)indexof(room->players, session), isReady) != isReady && (room->state & ServerState_Selected))
					room_set_state(ctx, room, ServerState_Lobby_Ready);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_SetStartGameTime);
		NOT_IMPLEMENTED(MenuRpcType_CancelStartGameTime);
		case MenuRpcType_GetIsInLobby: break;
		case MenuRpcType_SetIsInLobby: {
			bool inLobby = rpc->setIsInLobby.flags.hasValue0 && rpc->setIsInLobby.isBack;
			if(CounterP_overwrite(&room->global.inLobby, (uint32_t)indexof(room->players, session), inLobby) != inLobby && (room->state & ServerState_Selected))
				room_set_state(ctx, room, ServerState_Lobby_Ready);
			break;
		}
		case MenuRpcType_GetCountdownEndTime: {
			if(!(room->state & ServerState_Lobby))
				break;
			struct RemoteProcedureCall base = {UTimestamp_FromTime(room_get_syncTime(room))};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetIsStartButtonEnabled,
				.setIsStartButtonEnabled = {
					.base = base,
					.flags = {true, false, false, false},
					.reason = room->lobby.reason,
				},
			});
			if(room->state & (ServerState_Countdown | ServerState_Lobby_Downloading)) {
				SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
					.type = MenuRpcType_SetCountdownEndTime,
					.setCountdownEndTime = {
						.base = base,
						.flags = {true, false, false, false},
						.newTime = room_get_countdownEnd(room, base.syncTime),
					},
				});
			}
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			break;
		}
		case MenuRpcType_SetCountdownEndTime: uprintf("BAD TYPE: MenuRpcType_SetCountdownEndTime\n"); break;
		case MenuRpcType_CancelCountdown: uprintf("BAD TYPE: MenuRpcType_CancelCountdown\n"); break;
		NOT_IMPLEMENTED(MenuRpcType_GetOwnedSongPacks);
		case MenuRpcType_SetOwnedSongPacks: break;
		case MenuRpcType_RequestKickPlayer: {
			if(!(session_get_permissions(room, session, false).kickVote && rpc->requestKickPlayer.flags.hasValue0))
				break;
			struct CounterP players = room->playerSort;
			CounterP_clear(&players, (uint32_t)indexof(room->players, session));
			FOR_SOME_PLAYERS(id, players,) {
				if(!String_eq(room->players[id].userId, rpc->requestKickPlayer.kickedPlayerId))
					continue;
				uint8_t resp[65536], *resp_end = resp;
				struct InternalMessage r_kick = {
					.type = InternalMessageType_KickPlayer,
					.kickPlayer.disconnectedReason = DisconnectedReason_Kicked,
				};
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 0, false, 0});
				if(pkt_serialize(&r_kick, &resp_end, endof(resp), room->players[id].net.version))
					instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
				room->players[id].net.alive = false; // timeout if client refuses to leave
				uint32_t time = net_time();
				if(time - room->players[id].net.lastKeepAlive < IDLE_TIMEOUT_MS - KICK_TIMEOUT_MS)
					room->players[id].net.lastKeepAlive = time + KICK_TIMEOUT_MS - IDLE_TIMEOUT_MS;
			}
			break;
		}
		case MenuRpcType_GetPermissionConfiguration:  {
			uint8_t resp[65536], *resp_end = resp;
			struct MenuRpc r_permission = {
				.type = MenuRpcType_SetPermissionConfiguration,
				.setPermissionConfiguration = {
					.base.syncTime = UTimestamp_FromTime(room_get_syncTime(room)),
					.flags = {true, false, false, false},
					.playersPermissionConfiguration = room_get_permissions(room, room->connected, session),
				},
			};
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, r_permission);
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_SetPermissionConfiguration);
		case MenuRpcType_GetIsStartButtonEnabled: {
			uprintf("GET BUTTON GET BUTTON GET BUTTON GET BUTTON GET BUTTON\n");
			if(!(room->state & ServerState_Lobby))
				break;
			struct RemoteProcedureCall base = {UTimestamp_FromTime(room_get_syncTime(room))};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetIsStartButtonEnabled,
				.setIsStartButtonEnabled = {
					.base = base,
					.flags = {true, false, false, false},
					.reason = room->lobby.reason,
				},
			});
			instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
			break;
		}
		case MenuRpcType_SetIsStartButtonEnabled: uprintf("BAD TYPE: MenuRpcType_SetIsStartButtonEnabled\n"); break;
		default: uprintf("BAD MENU RPC TYPE\n");
	}
}

static bool room_try_finish(struct InstanceContext *ctx, struct Room *room) {
	if(!CounterP_isEmpty(room->game.activePlayers))
		return false;
	room->global.roundRobin = roundRobin_next(room->global.roundRobin, (room->configuration.songSelectionMode == SongSelectionMode_Random) ? COUNTERP_CLEAR : room->connected);
	room_set_state(ctx, room, ServerState_Game_Results);
	return true;
}

static void handle_GameplayRpc(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct GameplayRpc *rpc) {
	switch(rpc->type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: uprintf("BAD TYPE: GameplayRpcType_SetGameplaySceneSyncFinish\n"); break;
		case GameplayRpcType_SetGameplaySceneReady: {
			if((room->state & ServerState_Game) == 0)
				break;
			if(!(room->state & ServerState_Game_LoadingScene)) {
				if((room->state & ServerState_Game) && room->state > ServerState_Game_LoadingScene)
					session_set_state(ctx, room, session, ServerState_Game_LoadingSong);
				break;
			}
			session->settings = rpc->setGameplaySceneReady.flags.hasValue0 ? rpc->setGameplaySceneReady.playerSpecificSettings : CLEAR_SETTINGS;
			if(CounterP_set(&room->game.loadingScene.isLoaded, (uint32_t)indexof(room->players, session)) == 0)
				if(CounterP_contains(room->game.loadingScene.isLoaded, room->game.activePlayers))
					room_set_state(ctx, room, ServerState_Game_LoadingSong);
			break;
		}
		case GameplayRpcType_GetGameplaySceneReady: uprintf("BAD TYPE: GameplayRpcType_GetGameplaySceneReady\n"); break;
		NOT_IMPLEMENTED(GameplayRpcType_SetActivePlayerFailedToConnect);
		case GameplayRpcType_SetGameplaySongReady: {
			if(!(room->state & ServerState_Game_LoadingSong)) {
				if((room->state & ServerState_Game) && room->state > ServerState_Game_LoadingSong)
					session_set_state(ctx, room, session, ServerState_Game_Gameplay);
				break;
			}
			if(CounterP_set(&room->game.loadingSong.isLoaded, (uint32_t)indexof(room->players, session)) == 0)
				if(CounterP_contains(room->game.loadingSong.isLoaded, room->game.activePlayers))
					room_set_state(ctx, room, ServerState_Game_Gameplay);
			break;
		}
		case GameplayRpcType_GetGameplaySongReady: uprintf("BAD TYPE: GameplayRpcType_GetGameplaySongReady\n"); break;
		NOT_IMPLEMENTED(GameplayRpcType_SetSongStartTime);
		case GameplayRpcType_NoteCut: break;
		case GameplayRpcType_NoteMissed: break;
		case GameplayRpcType_LevelFinished: {
			if(!(room->state & ServerState_Game))
				break;
			if(!CounterP_clear(&room->game.activePlayers, (uint32_t)indexof(room->players, session)))
				break;
			if(!room->skipResults) {
				if(session->net.version.protocolVersion < 7)
					room->game.showResults |= (rpc->levelFinished.results.levelEndState == MultiplayerLevelEndState_Cleared);
				else
					room->game.showResults |= (rpc->levelFinished.results.playerLevelEndReason == MultiplayerPlayerLevelEndReason_Cleared);
			}
			room_try_finish(ctx, room);
			break;
		}
		NOT_IMPLEMENTED(GameplayRpcType_ReturnToMenu);
		case GameplayRpcType_RequestReturnToMenu: {
			if((room->state & ServerState_Game) && indexof(room->players, session) == room->serverOwner)
				room_set_state(ctx, room, ServerState_Lobby_Idle);
			break;
		}
		case GameplayRpcType_NoteSpawned: break;
		case GameplayRpcType_ObstacleSpawned: break;
		case GameplayRpcType_SliderSpawned: break;
		default: uprintf("BAD GAMEPLAY RPC TYPE\n");
	}
}

enum ChatCommand {
	ChatCommand_Help_Short = 'h',
	ChatCommand_Help_Long = 'h' | 'e' << 8 | 'l' << 16 | 'p' << 24,
	ChatCommand_Countdown = 'c',
	ChatCommand_PerPlayerDifficulty = 'p' | 'p' << 8 | 'd' << 16,
	ChatCommand_PerPlayerModifiers = 'p' | 'p' << 8 | 'm' << 16,
	ChatCommand_Host = 'h' | 'o' << 8 | 's' << 16 | 't' << 24,
	ChatCommand_Mode = 'm',
	ChatCommand_Skip = 's',
	ChatCommand_Pool = 'p',
};

static enum ChatCommand ChatCommand_readVerb(const char **cmd, const char *const cmd_end) {
	if(*cmd == cmd_end)
		return ChatCommand_Help_Long;

	const char *verb_end = memchr(*cmd, ' ', (size_t)(cmd_end - *cmd));
	if(verb_end == NULL)
		verb_end = cmd_end;
	struct String verb = {0};
	if((size_t)(verb_end - *cmd) < lengthof(verb.data)) {
		verb.length = (uint32_t)(verb_end - *cmd);
		memcpy(verb.data, *cmd, verb.length);
	}
	*cmd = verb_end;

	if(String_is(verb, "mode")) return ChatCommand_Mode;
	if(String_is(verb, "skip")) return ChatCommand_Skip;
	if(String_is(verb, "pool")) return ChatCommand_Pool;
	if(verb.length <= 4) return (uint32_t)verb.data[0] | (uint32_t)verb.data[1] << 8 | (uint32_t)verb.data[2] << 16 | (uint32_t)verb.data[3] << 24;
	if(String_is(verb, "countdown")) return ChatCommand_Countdown;
	if(String_is(verb, "perplayerdifficulty")) return ChatCommand_PerPlayerDifficulty;
	if(String_is(verb, "perplayerdifficulties")) return ChatCommand_PerPlayerDifficulty;
	if(String_is(verb, "perplayermodifiers")) return ChatCommand_PerPlayerModifiers;
	return (enum ChatCommand)0;
}

static bool ChatCommand_readBool(const char **cmd, const char *const cmd_end, bool *value_out) {
	struct String lower = {0};
	if((size_t)(cmd_end - *cmd) >= lengthof(lower.data)) {
		*cmd = cmd_end;
		return false;
	}
	for(; *cmd < cmd_end; ++*cmd)
		lower.data[lower.length++] = **cmd | (**cmd >> 1 & 0x20);
	if(String_is(lower, "1") || String_is(lower, "t") || String_is(lower, "y") || String_is(lower, "on") || String_is(lower, "yes") ||
	   String_is(lower, "true") || String_is(lower, "enable") || String_is(lower, "enabled"))
		*value_out = true;
	else if(String_is(lower, "0") || String_is(lower, "f") || String_is(lower, "n") || String_is(lower, "no") || String_is(lower, "off") ||
	        String_is(lower, "false") || String_is(lower, "disable") || String_is(lower, "disabled"))
		*value_out = false;
	else
		return false;
	return true;
}

static void chat(struct InstanceContext *const ctx, struct Room *const room, const struct InstanceSession *const session, const char *const format, ...) {
	va_list args;
	va_start(args, format);
	struct LongString message = {.isNull = false};
	message.length = (uint32_t)vsnprintf(message.data, sizeof(message.data) / sizeof(*message.data), format, args);
	va_end(args);

	struct InternalMessage r_message = {
		.type = InternalMessageType_MultiplayerSession,
		.multiplayerSession = {
			.type = MultiplayerSessionMessageType_MpCore,
			.mpCore = {
				.type = String_from("MpcTextChatPacket"),
				.mpcTextChat.text = message,
			},
		},
	};
	struct CounterP mask = COUNTERP_CLEAR;
	if(session != NULL)
		CounterP_set(&mask, (uint32_t)indexof(room->players, session));
	else
		mask = room->connected;
	FOR_SOME_PLAYERS(id, mask,) {
		if(!room->players[id].chatProtocol)
			continue;
		r_message.multiplayerSession.mpCore.mpcTextChat.protocolVersion = room->players[id].chatProtocol;
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, (session != NULL) ? 0 : 127, false, 0});
		pkt_serialize(&r_message, &resp_end, endof(resp), room->players[id].net.version);
		instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
	}
}

static void handle_ChatCommand(struct InstanceContext *const ctx, struct Room *const room, const struct InstanceSession *session, const char *cmd, const char *const cmd_end) {
	/* TODO: help + command details
	 * /countdown, /c [wait time] [start time] - Get or set countdown duration
	 * /perplayerdifficulties, /perplayerdifficulty, /ppd [true/false] - Toggle per-player difficulty
	 *     Allows players to choose other difficulties of the selected beatmap
	 *     BeatUpClient users may additionally switch difficulties mid-level
	 * /perplayermodifiers, /ppm [true/false] - Toggle per-player modifiers
	 *     Allows players to select different modifier combinations (not including speed modifiers)
	 * /host [player] - Pass host to a different player
	 * /mode, /m [vote/host/cycle/pool] - Get or set song selection mode
	 *     Vote: Selects the beatmap suggested by the most players
	 *     Host: Selects the host's suggestion
	 *     Cycle: Cycles through each player
	 *     Pool: Selects from a server-side beatmap pool
	 * /skip, /s [player, beatmap, or index] - Skip to the specified player ('cycle' mode) or beatmap ('pool' mode)
	 * /pool, /p [pool name] [beatmap or index] - Get or set the active map pool (setting this will automatically enable the 'pool' song selection mode)
	 */

	/*
		break;*/
	const enum ChatCommand verb = ChatCommand_readVerb(&cmd, cmd_end);
	switch((uint32_t)verb) {
		case ChatCommand_Help_Short: chat(ctx, room, session, "Command not yet implemented"); break;
		case ChatCommand_Help_Long: chat(ctx, room, session, "Command not yet implemented"); break;
		case ChatCommand_Countdown: {
			if(cmd++ < cmd_end) { // skip ' '
				if(indexof(room->players, session) != room->serverOwner) {
					chat(ctx, room, session, "Error: unexpected argument");
					break;
				}
				chat(ctx, room, session, "Command not yet implemented");
				break;
			}
			chat(ctx, room, session, "Wait time is %lldms; countdown is %lldms", room->longCountdown, room->shortCountdown);
			break;
		}
		case ChatCommand_PerPlayerDifficulty: [[fallthrough]];
		case ChatCommand_PerPlayerModifiers: {
			bool *const option = (verb == ChatCommand_PerPlayerDifficulty) ? &room->perPlayerDifficulty : &room->perPlayerModifiers, value = *option;
			if(cmd++ < cmd_end) { // skip ' '
				if(indexof(room->players, session) != room->serverOwner) {
					chat(ctx, room, session, "Error: unexpected argument");
					break;
				}
				if(!ChatCommand_readBool(&cmd, cmd_end, &value)) {
					chat(ctx, room, session, "Error: expected 'true' or 'false'");
					break;
				}
				if(value != *option) {
					if(!(room->state & ServerState_Lobby)) {
						chat(ctx, room, session, "Error: room must be in lobby");
						break;
					}
					if(room->state & (ServerState_Countdown | ServerState_Lobby_Downloading)) {
						chat(ctx, room, session, "Error: cannot configure during countdown");
						break;
					}
					*option = value;
					room_set_state(ctx, room, ServerState_Lobby_Entitlement);
				}
				session = NULL; // broadcast message
			}
			chat(ctx, room, session, "Per-player %s are %s", (verb == ChatCommand_PerPlayerDifficulty) ? "difficulty" : "modifiers", value ? "enabled" : "disabled");
			break;
		}
		case ChatCommand_Host: {
			if(cmd++ < cmd_end) { // skip ' '
				if(indexof(room->players, session) != room->serverOwner) {
					chat(ctx, room, session, "Error: unexpected argument");
					break;
				}
			}
			// Waiting for MultiplayerChat to implement a convention for referencing users
			// chat(ctx, room, session, "The room host is: %.*s", /* ping... */);
			chat(ctx, room, session, "Command not yet implemented");
			break;
		}
		case ChatCommand_Mode: { // This can't update variables client-side, but the game doesn't seem to actually use the selection mode for anything so it's fine
			if(cmd++ < cmd_end) { // skip ' '
				if(indexof(room->players, session) != room->serverOwner) {
					chat(ctx, room, session, "Error: unexpected argument");
					break;
				}
				chat(ctx, room, session, "Command not yet implemented"); // TODO: Switching selection modes will break many assumptions in the code
				break;
			}
			const char *mode = NULL;
			switch(room->configuration.songSelectionMode) {
				case SongSelectionMode_Vote: mode = "vote"; break;
				case SongSelectionMode_Random: mode = "pool"; break;
				case SongSelectionMode_OwnerPicks: mode = "host"; break;
				case SongSelectionMode_RandomPlayerPicks: mode = "cycle"; break;
				default: mode = reflect(SongSelectionMode, room->configuration.songSelectionMode);
			}
			chat(ctx, room, session, "Song selection mode is '%s'", mode);
			break;
		}
		case ChatCommand_Skip: chat(ctx, room, session, "Command not yet implemented"); break;
		case ChatCommand_Pool: chat(ctx, room, session, "Command not yet implemented"); break;
		default: chat(ctx, room, session, "Unrecognized command"); break;
	}
}

static void handle_MpCore(struct InstanceContext *const ctx, struct Room *const room, struct InstanceSession *const session, const struct MpCore *const mpCore) {
	switch(MpCoreType_From(&mpCore->type)) {
		case MpCoreType_MpcCapabilitiesPacket: {
			if(session->chatProtocol != 0 || !mpCore->mpcCapabilities.canText)
				break;
			session->chatProtocol = mpCore->mpcCapabilities.protocolVersion;
			chat(ctx, room, session, "Welcome to BeatUpServer | BETA!\n* Per-player difficulty is %s\n* Per-player modifiers are %s",
				room->perPlayerDifficulty ? "enabled" : "disabled", room->perPlayerModifiers ? "enabled" : "disabled");
			break;
		}
		case MpCoreType_MpcTextChatPacket: {
			if(session->chatProtocol && mpCore->mpcTextChat.text.length && mpCore->mpcTextChat.text.data[0] == '/')
				handle_ChatCommand(ctx, room, session, &mpCore->mpcTextChat.text.data[1], &mpCore->mpcTextChat.text.data[mpCore->mpcTextChat.text.length]);
			break;
		}
		default:;
	}
}

static bool handle_BeatUpMessage(const struct BeatUpMessage *message) {
	switch(message->type) {
		case BeatUpMessageType_ConnectInfo: break;
		case BeatUpMessageType_RecommendPreview: break;
		case BeatUpMessageType_ShareInfo: break;
		case BeatUpMessageType_DataFragmentRequest: break;
		case BeatUpMessageType_DataFragment: uprintf("BAD TYPE: BeatUpMessageType_LevelFragment\n"); return false;
		case BeatUpMessageType_LoadProgress: break;
		default: uprintf("BAD BEAT UP MESSAGE TYPE\n");
	}
	return true;
}

static bool handle_MultiplayerSession(struct InstanceContext *ctx, struct Room *const room, struct InstanceSession *session, const struct MultiplayerSession *message) {
	switch(message->type) {
		case MultiplayerSessionMessageType_MenuRpc: handle_MenuRpc(ctx, room, session, &message->menuRpc); break;
		case MultiplayerSessionMessageType_GameplayRpc: handle_GameplayRpc(ctx, room, session, &message->gameplayRpc); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: break;
		case MultiplayerSessionMessageType_ScoreSyncState: break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: uprintf("BAD TYPE: MultiplayerSessionMessageType_NodePoseSyncStateDelta\n"); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: uprintf("BAD TYPE: MultiplayerSessionMessageType_ScoreSyncStateDelta\n"); break;
		case MultiplayerSessionMessageType_MpCore: handle_MpCore(ctx, room, session, &message->mpCore); return false;
		case MultiplayerSessionMessageType_BeatUpMessage: return handle_BeatUpMessage(&message->beatUpMessage);
		default: uprintf("BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
	}
	return true;
}

static void session_refresh_stateHash(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session) {
	bool isSpectating = !PlayerStateHash_contains(session->stateHash, "wants_to_play_next_level");
	if(CounterP_overwrite(&room->global.isSpectating, (uint32_t)indexof(room->players, session), isSpectating) != isSpectating && (room->state & ServerState_Selected))
		room_set_state(ctx, room, ServerState_Lobby_Ready);
}

static struct String GameVersion_toString(const GameVersion version) {
	struct String out = {0};
	strncpy(out.data, reflect(GameVersion, version), lengthof(out.data));
	for(; out.length < lengthof(out.data) && out.data[out.length] != 0; ++out.length)
		if(out.data[out.length] == '_')
			out.data[out.length] = '.';
	return out;
}

#include "avatar_compat.h"
static void handle_PlayerIdentity(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct PlayerIdentity *identity) {
	session->stateHash = identity->playerState;
	session_refresh_stateHash(ctx, room, session);
	session->avatars = (session->net.version.protocolVersion >= 9) ? identity->playerAvatars : (struct MultiplayerAvatarsData){
		.legacy = identity->playerAvatars.legacy,
		.count = 1,
		.avatars = {OpaqueAvatarData_FromLegacy(&identity->playerAvatars.legacy, session->net.version)},
	};
	for(const struct OpaqueAvatarData *avatar = session->avatars.avatars; avatar < &session->avatars.avatars[session->avatars.count]; ++avatar)
		if(avatar->typeHash == BeatAvatarMagic)
			session->avatars.legacy = LegacyAvatarData_FromOpaque(avatar, session->net.version);
	#ifdef ENABLE_PASSTHROUGH_ENCRYPTION
	if(identity->random.length == sizeof(session->identity.random.raw))
		memcpy(session->identity.random.raw, identity->random.data, sizeof(session->identity.random.raw));
	else
		session->identity.random = (struct Cookie32){0};
	session->identity.publicEncryptionKey = identity->publicEncryptionKey;
	#endif
	if(session->sentIdentity)
		return;
	session->sentIdentity = true;

	{
		struct InternalMessage r_connected = {
			.type = InternalMessageType_PlayerConnected,
			.playerConnected = {
				.remoteConnectionId = InstanceSession_connectionId(room->players, session),
				.userId = session->userId,
				.userName = session->userName,
			},
		};
		struct InternalMessage r_sort = {
			.type = InternalMessageType_PlayerSortOrderUpdate,
			.playerSortOrderUpdate = {
				.userId = session->userId,
				.sortIndex = (int32_t)indexof(room->players, session),
			},
		};
		struct InternalMessage r_identity = {
			.type = InternalMessageType_PlayerIdentity,
			.playerIdentity = *identity,
		};
		const struct MpPlayerData r_playerData = {
			.platform = MpPlatform_Unknown, // TODO: platformId, platform
			.gameVersion = GameVersion_toString(session->net.version.gameVersion),
		};
		#ifndef ENABLE_PASSTHROUGH_ENCRYPTION
		memset(r_identity.playerIdentity.random.data, 0, r_identity.playerIdentity.random.length);
		r_identity.playerIdentity.publicEncryptionKey.length = 0;
		#endif
		FOR_EXCLUDING_PLAYER(id, room->connected, (uint32_t)indexof(room->players, session)) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 0, false, 0});
			pkt_serialize(&r_connected, &resp_end, endof(resp), room->players[id].net.version);
			pkt_serialize(&r_sort, &resp_end, endof(resp), room->players[id].net.version);
			instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
		}
		r_sort.playerSortOrderUpdate.userId = OwnUserId(r_sort.playerSortOrderUpdate.userId);
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
		pkt_serialize(&r_sort, &resp_end, endof(resp), session->net.version);
		instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
		FOR_SOME_PLAYERS(id, room->connected,) {
			resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {InstanceSession_connectionId(room->players, session), 0, false, 0});
			pkt_serialize(&r_identity, &resp_end, endof(resp), room->players[id].net.version);
			if(&room->players[id] != session && session->net.version.gameVersion >= GameVersion_1_29_4) {
				SERIALIZE_MPCORE(&resp_end, endof(resp), room->players[id].net.version, {
					.type = String_from("MpPlayerData"),
					.mpPlayerData = r_playerData,
				});
			}
			instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
		}
	}

	struct RemoteProcedureCall base = {
		.syncTime = UTimestamp_FromTime(room_get_syncTime(room)),
	};
	FOR_SOME_PLAYERS(id, room->connected,) {
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 127, false, 0});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetRecommendedBeatmap,
			.getRecommendedBeatmap.base = base,
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetRecommendedGameplayModifiers,
			.getRecommendedGameplayModifiers.base = base,
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetOwnedSongPacks,
			.getOwnedSongPacks.base = base,
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetIsReady,
			.getIsReady.base = base,
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetIsInLobby,
			.getIsInLobby.base = base,
		});
		instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
	}
}

static bool handle_RoutingHeader(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, bool reliable, DeliveryMethod channelId) {
	struct RoutingHeader routing;
	if(room->configuration.maxPlayerCount >= 127) {
		struct BTRoutingHeader extendedRouting;
		if(!pkt_read(&extendedRouting, data, end, session->net.version))
			return true;
		routing = (struct RoutingHeader){extendedRouting.remoteConnectionId, extendedRouting.connectionId, false, 0};
	} else if(!pkt_read(&routing, data, end, session->net.version)) {
		return true;
	}
	if(routing.connectionId == 0)
		return false;
	struct CounterP mask = COUNTERP_CLEAR;
	if(routing.connectionId == 127) {
		mask = room->connected;
	} else {
		uint32_t index = ConnectionId_index(routing.connectionId);
		if(index < (uint32_t)room->configuration.maxPlayerCount && CounterP_get(room->connected, index)) {
			CounterP_set(&mask, index);
		} else {
			uprintf("connectionId %hhu points to nonexistent player!\n", routing.connectionId);
			return true;
		}
		routing.connectionId = 0;
	}
	routing.remoteConnectionId = InstanceSession_connectionId(room->players, session);
	if(reliable) {
		if(channelId != DeliveryMethod_ReliableOrdered)
			return true;
		FOR_EXCLUDING_PLAYER(id, mask, (uint32_t)indexof(room->players, session)) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write(&routing, &resp_end, endof(resp), room->players[id].net.version);
			// TODO: selective reordering and repacking of not-set-sent outbound messages
			// TODO: tamper with sync states to fix whatever triggers the game's "broken tracking" bug
			// TODO: more intelligent rate limiting (reduced sync state rates, global load monitoring)
			pkt_write_bytes(*data, &resp_end, endof(resp), room->players[id].net.version, (size_t)(end - *data));
			instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), true);
		}
	} else {
		FOR_EXCLUDING_PLAYER(id, mask, (uint32_t)indexof(room->players, session)) {
			// TODO: investigate fast paths? This block could theoretically be hit upwards of 1.2 million times per second in a fully saturated 254 player lobby
			if(room->players[id].channels.ro.base.backlog) // unreliable transport is only used for sync state deltas, which are safe to drop if rate limiting is needed
				continue;
			uint8_t resp[65536], *resp_end = resp;
			pkt_write(&routing, &resp_end, endof(resp), room->players[id].net.version);
			pkt_write_bytes(*data, &resp_end, endof(resp), room->players[id].net.version, (size_t)(end - *data));
			instance_send(ctx, &room->players[id], resp, (uint32_t)(resp_end - resp), false);
		}
	}
	return routing.connectionId != 127 || routing.encrypted;
}

static void process_message(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, bool reliable, DeliveryMethod channelId) {
	if(!session->net.alive)
		return;
	if(handle_RoutingHeader(ctx, room, session, data, end, reliable, channelId) || !reliable) { // unreliable packets aren't meaningful to the server
		*data = end;
		return;
	}
	while(*data < end) {
		struct SerializeHeader serial;
		if(!pkt_read(&serial, data, end, session->net.version))
			return;
		const uint8_t *sub = *data;
		*data += serial.length;
		if(*data > end) {
			uprintf("Invalid serial length: %u\n", serial.length);
			return;
		}
		struct InternalMessage message = {0};
		if(!pkt_read(&message, &sub, *data, session->net.version)) { // TODO: experiment with packet dropping and reserialization for better bandwidth usage
			if(message.type == InternalMessageType_MultiplayerSession) {
				if(message.multiplayerSession.type == MultiplayerSessionMessageType_MenuRpc)
					uprintf("Error [length=%zd menuRpc=%s]\n", *data - sub, reflect(MenuRpcType, message.multiplayerSession.menuRpc.type));
				else
					uprintf("Error [length=%zd type=%s]\n", *data - sub, reflect(MultiplayerSessionMessageType, message.multiplayerSession.type));
			}
			continue;
		}
		bool validateLength = true;
		switch(message.type) {
			case InternalMessageType_SyncTime: uprintf("BAD TYPE: InternalMessageType_SyncTime\n"); break;
			case InternalMessageType_PlayerConnected: uprintf("BAD TYPE: InternalMessageType_PlayerConnected\n"); break;
			case InternalMessageType_PlayerIdentity: handle_PlayerIdentity(ctx, room, session, &message.playerIdentity); break;
			NOT_IMPLEMENTED(InternalMessageType_PlayerLatencyUpdate);
			case InternalMessageType_PlayerDisconnected: uprintf("BAD TYPE: InternalMessageType_PlayerDisconnected\n"); break;
			case InternalMessageType_PlayerSortOrderUpdate: uprintf("BAD TYPE: InternalMessageType_PlayerSortOrderUpdate\n"); break;
			NOT_IMPLEMENTED(InternalMessageType_Party);
			case InternalMessageType_MultiplayerSession: validateLength = handle_MultiplayerSession(ctx, room, session, &message.multiplayerSession); break;
			case InternalMessageType_KickPlayer: uprintf("BAD TYPE: InternalMessageType_KickPlayer\n"); break;
			case InternalMessageType_PlayerStateUpdate: {
				session->stateHash = message.playerStateUpdate.playerState;
				session_refresh_stateHash(ctx, room, session);
				break;
			}
			NOT_IMPLEMENTED(InternalMessageType_PlayerAvatarUpdate);
			case InternalMessageType_PingMessage: {
				struct InternalMessage r_pong = {
					.type = InternalMessageType_PongMessage,
					.pongMessage.pingTime = message.pingMessage.pingTime,
				};
				struct InternalMessage r_sync = {
					.type = InternalMessageType_SyncTime,
					.syncTime.syncTime = UTimestamp_FromTime(room_get_syncTime(room)),
				};
				uint8_t resp[65536], *resp_end = resp;
				pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
				if(pkt_serialize(&r_pong, &resp_end, endof(resp), session->net.version) &&
				   pkt_serialize(&r_sync, &resp_end, endof(resp), session->net.version))
					instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
				break;
			}
			case InternalMessageType_PongMessage: break;
			default: uprintf("BAD INTERNAL MESSAGE TYPE\n");
		}
		if(validateLength)
			pkt_debug("BAD INTERNAL MESSAGE LENGTH", sub, *data, serial.length, session->net.version);
	}
}

static bool handle_ConnectMessage(const struct ConnectMessage *message, struct Room *room, struct InstanceSession *session, uint8_t protocolId, const uint8_t **data, const uint8_t *end, struct ServerConnectInfo *info_out) {
	struct String baseUserId = session->userId;
	if(message->userId.length < baseUserId.length && baseUserId.data[message->userId.length] == '$') // fast path for annotation stripping
		baseUserId.length = message->userId.length;
	if(!String_eq(message->userId, baseUserId))
		return true;
	session->net.version.netVersion = protocolId;
	session->userName = message->userName;

	bool directDownloads = false;
	while(*data < end) {
		struct ModConnectHeader mod;
		if(!pkt_read(&mod, data, end, session->net.version))
			break;
		const uint8_t *sub = *data;
		*data += mod.length;
		if(*data > end) {
			*data = end;
			uprintf("Invalid mod header length: %u\n", mod.length);
			break;
		}
		if(String_is(mod.name, "BeatUpClient beta0")) {
			uprintf("Outdated BeatUpClient version from user \"%.*s (%.*s)\"\n", session->userName.length, session->userName.data, session->userId.length, session->userId.data);
			return true;
		}
		if(!String_is(mod.name, "BeatUpClient beta1")) {
			uprintf("UNIDENTIFIED MOD: %.*s\n", mod.name.length, mod.name.data);
			continue;
		}
		struct ServerConnectInfo info;
		if(!pkt_read(&info, &sub, *data, session->net.version))
			continue;
		session->net.version.beatUpVersion = (uint8_t)info.base.protocolId;
		directDownloads = info.directDownloads;
		if(indexof(room->players, session) == room->serverOwner) {
			room->shortCountdown = info.countdownDuration * 250;
			room->skipResults = info.skipResults;
			room->perPlayerDifficulty = info.perPlayerDifficulty;
			room->perPlayerModifiers = info.perPlayerModifiers;
		}
		pkt_debug("BAD MOD HEADER LENGTH", sub, *data, mod.length, session->net.version);
	}
	*info_out = (struct ServerConnectInfo){
		.base = {
			.protocolId = session->net.version.beatUpVersion,
			.blockSize = 398,
		},
		.windowSize = 256,
		.countdownDuration = (uint8_t)(room->shortCountdown / 250),
		.directDownloads = directDownloads,
		.skipResults = room->skipResults,
		.perPlayerDifficulty = room->perPlayerDifficulty,
		.perPlayerModifiers = room->perPlayerModifiers,
	};
	return false;
}

static void TryInitiateSession(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session) {
	if(session->state & ServerState_Connected)
		return;

	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 127, false, 0});
	struct InternalMessage r_sync = {
		.type = InternalMessageType_SyncTime,
		.syncTime.syncTime = UTimestamp_FromTime(room_get_syncTime(*room)),
	};
	if(pkt_serialize(&r_sync, &resp_end, endof(resp), session->net.version))
		instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);

	uprintf("connect [slot=(%zu,%zu)@%zu userName=\"%.*s\" userId=(%.*s) protocolVersion=%hhu beatUpVersion=%hhu]\n", indexof(contexts, ctx), indexof(*ctx->rooms, room), indexof((*room)->players, session),
		session->userName.length, session->userName.data, session->userId.length, session->userId.data, session->net.version.protocolVersion, session->net.version.beatUpVersion);

	resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
	FOR_SOME_PLAYERS(id, (*room)->connected,) {
		pkt_serialize((&(struct InternalMessage){
			.type = InternalMessageType_PlayerConnected,
			.playerConnected = {
				.remoteConnectionId = InstanceSession_connectionId((*room)->players, &(*room)->players[id]),
				.userId = (*room)->players[id].userId,
				.userName = (*room)->players[id].userName,
			},
		}), &resp_end, endof(resp), session->net.version);
		pkt_serialize((&(struct InternalMessage){
			.type = InternalMessageType_PlayerSortOrderUpdate,
			.playerSortOrderUpdate = {
				.userId = (*room)->players[id].userId,
				.sortIndex = (int32_t)id,
			},
		}), &resp_end, endof(resp), session->net.version);
	}
	struct InternalMessage r_identity = { // TODO: is this actually needed?
		.type = InternalMessageType_PlayerIdentity,
		.playerIdentity = {
			.playerState.bloomFilter = {
				.d0 = 0x400208001030040,
				.d1 = 0x800400000420001,
			},
			.random.length = 32,
		},
	};
	#ifdef ENABLE_PASSTHROUGH_ENCRYPTION
	memcpy(r_identity.playerIdentity.random.data, NetKeypair_get_random(&(*room)->keys)->raw, 32);
	NetKeypair_write_key(&(*room)->keys, &ctx->net, &r_identity.playerIdentity.publicEncryptionKey);
	#endif
	pkt_serialize(&r_identity, &resp_end, endof(resp), session->net.version);
	instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);

	FOR_SOME_PLAYERS(id, (*room)->connected,) {
		resp_end = resp;
		struct InternalMessage r_identity = {
			.type = InternalMessageType_PlayerIdentity,
			.playerIdentity = {
				.playerState = (*room)->players[id].stateHash,
				.playerAvatars = (*room)->players[id].avatars,
				.random.length = 32,
			},
		};
		#ifdef ENABLE_PASSTHROUGH_ENCRYPTION
		memcpy(r_identity.playerIdentity.random.data, (*room)->players[id].identity.random.raw, sizeof((*room)->players->identity.random.raw));
		r_identity.playerIdentity.publicEncryptionKey = (*room)->players[id].identity.publicEncryptionKey;
		#endif
		pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {InstanceSession_connectionId((*room)->players, &(*room)->players[id]), 0, false, 0});
		pkt_serialize(&r_identity, &resp_end, endof(resp), session->net.version);
		if((*room)->players[id].net.version.gameVersion >= GameVersion_1_29_4) {
			SERIALIZE_MPCORE(&resp_end, endof(resp), session->net.version, {
				.type = String_from("MpPlayerData"),
				.mpPlayerData = {
					.platform = MpPlatform_Unknown, // TODO: platformId, platform
					.gameVersion = GameVersion_toString((*room)->players[id].net.version.gameVersion),
				},
			});
		}
		instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
	}

	session_set_state(ctx, *room, session, ServerState_Synchronizing);
}

static void handle_ConnectRequest(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session, const struct ConnectRequest *req, const uint8_t **data, const uint8_t *end) {
	if(!(session->net.version.direct || String_eq(req->secret, session->secret))) {
		*data = end;
		return;
	}
	{
		struct ServerConnectInfo beatUpInfo;
		if(handle_ConnectMessage(&req->message, *room, session, (uint8_t)req->protocolId, data, end, &beatUpInfo)) {
			*data = end;
			return;
		}
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), session->net.version, NetPacketHeader, {
			.property = PacketProperty_ConnectAccept,
			.connectAccept = {
				.connectTime = req->connectTime,
				.peerId = (int32_t)indexof((*room)->players, session),
				.beatUp = beatUpInfo,
			},
		});
		if(resp_end == resp)
			return;
		net_send_internal(&ctx->net, &session->net, resp, (uint32_t)(resp_end - resp), EncryptMode_BGNet);
	}
	TryInitiateSession(ctx, room, session);
}

static void handle_ENetConnect(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end) {
	struct ConnectMessage message;
	if(!pkt_read(&message, data, end, session->net.version))
		return;
	struct ServerConnectInfo beatUpInfo;
	if(handle_ConnectMessage(&message, *room, session, 11, data, end, &beatUpInfo)) { // TODO: get netVersion from wire
		*data = end;
		return;
	}
	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false, 0});
	SERIALIZE_BEATUP(&resp_end, endof(resp), session->net.version, {
		.type = BeatUpMessageType_ServerConnectInfo,
		.serverConnectInfo = beatUpInfo,
	});
	instance_send(ctx, session, resp, (uint32_t)(resp_end - resp), true);
	TryInitiateSession(ctx, room, session);
}

static void log_players(const struct Room *room, const struct SS *addr, const char *prefix) {
	char addrstr[INET6_ADDRSTRLEN + 8] = {0}, bitText[sizeof(room->playerSort) * 3], *bitText_end = bitText;
	if(addr != NULL)
		net_tostr(addr, addrstr);
	uint32_t playerCount = 0;
	for(uint32_t offset = 0; offset < (uint32_t)room->configuration.maxPlayerCount; offset += 8) {
		uint8_t byte = CounterP_byte(room->playerSort, offset);
		playerCount += (uint32_t)__builtin_popcount(byte);
		*bitText_end++ = (char)(226u);
		*bitText_end++ = (char)(160u | (byte >> 6));
		*bitText_end++ = (char)(128u | (byte & 63));
	}
	uprintf("%s %s | player slots (%u/%d): [%.*s]\n", prefix, addrstr, playerCount, room->configuration.maxPlayerCount, (int)(bitText_end - bitText), bitText);
}

static inline struct Room **instance_get_room(struct InstanceContext *ctx, uint32_t roomID) {
	if(roomID >= sizeof(ctx->rooms) / sizeof(struct Room*))
		return NULL;
	return &ctx->rooms[0][roomID];
}

static void room_free(struct InstanceContext *ctx, struct Room **room) {
	size_t roomID = indexof(*ctx->rooms, room);
	net_keypair_free(&(*room)->keys);
	free(*room);
	*room = NULL;
	uprintf("closing room (%zu,%hu)\n", indexof(contexts, ctx), roomID);
	uint32_t group = (uint32_t)(roomID / lengthof(*ctx->rooms));
	for(uint32_t i = 0; i < lengthof(*ctx->rooms); ++i)
		if(ctx->rooms[group][i])
			return;
	Counter64_clear(&ctx->roomMask, group);
}

static void room_disconnect(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session, bool hold) {
	playerid_t id = (playerid_t)indexof((*room)->players, session);
	CounterP_clear(&(*room)->playerSort, id);
	log_players(*room, NetSession_get_addr(&session->net), hold ? "reconnect" : "disconnect");

	if(id == (*room)->serverOwner) {
		(*room)->serverOwner = 0;
		uint32_t ownerOrder = ~UINT32_C(0);
		FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
			if((*room)->players[id].joinOrder < ownerOrder) {
				ownerOrder = (*room)->players[id].joinOrder;
				(*room)->serverOwner = id;
			}
		}
		struct MenuRpc r_permission = {
			.type = MenuRpcType_SetPermissionConfiguration,
			.setPermissionConfiguration = {
				.base.syncTime = UTimestamp_FromTime(room_get_syncTime(*room)),
				.flags = {true, false, false, false},
				.playersPermissionConfiguration = room_get_permissions(*room, (*room)->connected, NULL),
			},
		};
		struct PlayerLobbyPermissionConfiguration *self = r_permission.setPermissionConfiguration.playersPermissionConfiguration.playersPermission;
		FOR_SOME_PLAYERS(id, (*room)->connected,) {
			self->userId = OwnUserId((*room)->players[id].userId);
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), (*room)->players[id].net.version, RoutingHeader, {0, 0, false, 0});
			SERIALIZE_MENURPC(&resp_end, endof(resp), (*room)->players[id].net.version, r_permission);
			instance_send(ctx, &(*room)->players[id], resp, (uint32_t)(resp_end - resp), true);
			self->userId = (*room)->players[id].userId;
			++self;
		}
	}

	if(!CounterP_isEmpty((*room)->playerSort)) {
		session_set_state(ctx, *room, session, 0);
		if((*room)->state & ServerState_Lobby)
			room_set_state(ctx, *room, ServerState_Lobby_Entitlement);
		hold = true;
	}

	if(session->enet)
		eenet_free(session->enet);
	session->enet = NULL;
	instance_channels_reset(&session->channels);
	NetSession_free(&session->net);
	if(hold)
		return;

	room_free(ctx, room);
	if(ctx->master == NULL)
		return;
	WireLink_send(ctx->master, &(const struct WireMessage){
		.type = WireMessageType_WireRoomCloseNotify,
		.roomCloseNotify.room = (uint32_t)indexof(*ctx->rooms, room),
	});
}

static bool handle_Merged(const uint8_t **data, const uint8_t *end, struct NetPacketHeader *header_out, size_t *length_out, struct PacketContext version) {
	if(*data >= end)
		return false;
	struct MergedHeader merged;
	if(!pkt_read(&merged, data, end, version))
		return false;
	if(end - *data < (int32_t)merged.length) {
		uprintf("OVERFLOW\n");
		return false;
	}
	*length_out = merged.length - pkt_read(header_out, data, &(*data)[merged.length], version);
	return *length_out != merged.length;
}

static inline void handle_packet(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session, const uint8_t *data, const uint8_t *end) {
	struct NetPacketHeader header;
	if(!pkt_read(&header, &data, end, session->net.version))
		return;
	if(session->state == 0 && header.property != PacketProperty_ConnectRequest && header.property != PacketProperty_UnconnectedMessage)
		return;
	size_t length = (size_t)(end - data);
	if(header.property == PacketProperty_Merged /*&& !header.isFragmented*/)
		if(!handle_Merged(&data, end, &header, &length, session->net.version))
			return;
	do {
		if(header.isFragmented && header.property != PacketProperty_Channeled) {
			uprintf("MALFORMED HEADER\n");
			return;
		}
		const uint8_t *sub = data;
		switch(header.property) {
			case PacketProperty_Unreliable: process_message(ctx, *room, session, &sub, &sub[length], false, 0); break;
			case PacketProperty_Channeled: {
					const uint8_t *pkt = NULL;
					DeliveryMethod channelId = header.channeled.channelId;
					size_t pkt_len = handle_Channeled_start(&ctx->net, &session->net, &session->channels, &header, &sub, &sub[length], &pkt);
					while(pkt_len != 0) {
						const uint8_t *pkt_it = pkt;
						process_message(ctx, *room, session, &pkt_it, &pkt[pkt_len], true, channelId);
						pkt_debug("BAD CHANNELED PACKET LENGTH", pkt_it, &pkt[pkt_len], pkt_len, session->net.version);
						channelId = DeliveryMethod_ReliableOrdered;
						pkt_len = handle_Channeled_next(&session->net, &session->channels, &pkt);
					}
					break;
				}
			case PacketProperty_Ack: {
				if(!session->net.version.windowSize) {
					if(!length || length > sizeof(header.ack.data)) {
						uprintf("BAD ACK LENGTH\n");
						room_disconnect(ctx, room, session, false);
						return;
					}
					uprintf("probed window size %u from user \"%.*s (%.*s)\"\n", length * 8, session->userName.length, session->userName.data, session->userId.length, session->userId.data);
					session->net.version.windowSize = (uint16_t)(length * 8);
					instance_channels_flushBacklog(&session->channels, &session->net);
					memcpy(header.ack.data, sub - sizeof(header.ack._pad0), length);
					sub += length;
				}
				handle_Ack(&session->net, &session->channels, &header.ack);
				break;
			}
			case PacketProperty_Ping: handle_Ping(&ctx->net, &session->net, &session->tableTennis, header.ping); break;
			case PacketProperty_Pong: {
				struct InternalMessage r_latency = {
					.type = InternalMessageType_PlayerLatencyUpdate,
					.playerLatencyUpdate.latency = handle_Pong(&ctx->net, &session->net, &session->tableTennis, header.pong),
				};
				if(session->net.version.protocolVersion >= 7 || r_latency.playerLatencyUpdate.latency < 0)
					break;
				FOR_EXCLUDING_PLAYER(id, (*room)->connected, (uint32_t)indexof((*room)->players, session)) {
					uint8_t resp[65536], *resp_end = resp;
					pkt_write_c(&resp_end, endof(resp), (*room)->players[id].net.version, RoutingHeader, {
						.remoteConnectionId = InstanceSession_connectionId((*room)->players, session),
					});
					if(pkt_serialize(&r_latency, &resp_end, endof(resp), (*room)->players[id].net.version))
						instance_send(ctx, &(*room)->players[id], resp, (uint32_t)(resp_end - resp), true);
				}
				break;
			}
			case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, room, session, &header.connectRequest, &sub, &sub[length]); break;
			case PacketProperty_Disconnect: room_disconnect(ctx, room, session, false); return;
			case PacketProperty_UnconnectedMessage: {
				struct MasterSession *const masterSession = MasterContext_lookup(&ctx->base, *NetSession_get_addr(&session->net));
				if(masterSession != NULL)
					MasterContext_handleMessage(&ctx->base, &ctx->net, masterSession, header.unconnectedMessage, sub, &sub[length], NULL);
				sub += length;
				break;
			}
			case PacketProperty_MtuCheck: handle_MtuCheck(&ctx->net, &session->net, &header.mtuCheck); break;
			default: uprintf("Unhandled property [%s]\n", reflect(PacketProperty, header.property));
		}
		data += length;
		pkt_debug("BAD PACKET LENGTH", sub, data, length, session->net.version);
	} while(handle_Merged(&data, end, &header, &length, session->net.version));
}

static struct {
	bool isRemote;
	union {
		struct WireContext *local;
		const char *remote;
	};
} instance_master = {0};
static struct InstanceSession *instance_onGraphAuth(struct NetContext *net, struct NetSession *masterSession, const struct GraphAuthToken *token, struct Room ***room_out);
static void *instance_handler(struct InstanceContext *ctx) {
	net_lock(&ctx->net);
	{
		const struct WireMessage connectMessage = {
			.type = WireMessageType_WireSetAttribs,
			.setAttribs = {
				.capacity = sizeof(ctx->rooms) / sizeof(**ctx->rooms),
				.discover = true,
			},
		};
		if(instance_master.isRemote)
			ctx->master = WireContext_connect(&ctx->wire, instance_master.remote, &connectMessage);
		else if(instance_master.local != NULL)
			ctx->master = WireContext_attach(&ctx->wire, instance_master.local, &connectMessage);
	}
	if(ctx->master == NULL) {
		uprintf("WireContext_connect() failed\n");
		goto unlock;
	}
	uprintf("Started\n");
	uint8_t pkt[1536];
	memset(pkt, 0, sizeof(pkt));
	uint32_t len;
	union {
		struct NetSession *base;
		struct InstanceSession *instance;
		struct MasterSession *master;
	} session;
	for(struct Room **room = NULL; (len = net_recv(&ctx->net, pkt, &session.base, (void**)&room)); room = NULL) {
		struct GraphAuthToken token;
		struct EENetPacket event = {0};
		if(room != NULL) {
			if(session.instance->enet == NULL)
				handle_packet(ctx, room, session.instance, pkt, &pkt[len]);
			else
				eenet_handle(session.instance->enet, pkt, &pkt[len], &event);
		} else if(MasterContext_handle(&ctx->base, &ctx->net, session.master, pkt, &pkt[len], &token)) {
			session.instance = instance_onGraphAuth(&ctx->net, session.base, &token, &room);
			if(session.instance == NULL) {
				eenet_free(token.enet);
			} else if(token.enet) {
				eenet_attach(token.enet, &ctx->net, &session.instance->net);
				session.instance->enet = token.enet;
				event = token.event;
			}
		}
		while(event.type != EENetPacketType_None) {
			const uint8_t *const data_end = &event.data[event.data_len];
			switch(event.type) {
				case EENetPacketType_ConnectMessage: handle_ENetConnect(ctx, room, session.instance, &event.data, data_end); break;
				case EENetPacketType_Disconnect: room_disconnect(ctx, room, session.instance, false); break;
				case EENetPacketType_Reliable: process_message(ctx, *room, session.instance, &event.data, data_end, true, DeliveryMethod_ReliableOrdered); break;
				case EENetPacketType_Unreliable: process_message(ctx, *room, session.instance, &event.data, data_end, false, 0); break;
				default: uprintf("Unexpected ENet packet type: %u\n", event.type); break;
			}
			pkt_debug("BAD ENET PACKET LENGTH", event.data, data_end, event.data_len, session.instance->net.version);
			eenet_handle_next(session.instance->enet, &event);
		}
	}
	WireLink_free(ctx->master);
	ctx->master = NULL;
	unlock: net_unlock(&ctx->net);
	return 0;
}

// TODO: clients aren't guaranteed to use the same IP address when deeplinking from the master server to instances
static struct NetSession *instance_onResolve(struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len, void **userdata_out) {
	*userdata_out = NULL;
	if(packet_len == 0)
		return NULL;
	struct InstanceContext *const ctx = (struct InstanceContext*)net->userptr;
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
			if(!SS_equal(&addr, &(*room)->players[id].net.addr) || (*packet == 0 && (*room)->players[id].enet == NULL)) // filter non-enet unencrypted
				continue;
			*out_len = NetSession_decrypt(&(*room)->players[id].net, packet, packet_len, out);
			*userdata_out = room;
			return &(*room)->players[id].net;
		}
	}
	struct NetSession *const masterSession = MasterContext_onResolve(&ctx->base, net, addr, packet, packet_len, out, out_len);
	if(masterSession != NULL || *packet != 1)
		return masterSession;
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
			struct NetSession *const session = &(*room)->players[id].net;
			if(session->addr.ss.ss_family != AF_UNSPEC || session->version.direct)
				continue;
			*out_len = NetSession_decrypt(session, packet, packet_len, out);
			if(!*out_len)
				continue;
			char addrstr[INET6_ADDRSTRLEN + 8];
			net_tostr(&addr, addrstr);
			uprintf("resolve{Legacy} %s -> (%zu,%zu)@%u\n", addrstr, indexof(contexts, ctx), indexof(*ctx->rooms, room), id);
			session->addr = addr;
			*userdata_out = room;
			return session;
		}
	}
	return NULL;
}

static uint32_t instance_onResend(struct NetContext *net, uint32_t currentTime) {
	struct InstanceContext *const ctx = (struct InstanceContext*)net->userptr;
	uint32_t nextTick = MasterContext_onResend(&ctx->base, net, currentTime);
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
			struct InstanceSession *const session = &(*room)->players[id];
			const int32_t kickTime = (int32_t)(NetSession_get_lastKeepAlive(&session->net) + IDLE_TIMEOUT_MS - currentTime);
			if(kickTime < 0) {
				uprintf("session timeout\n");
				room_disconnect(ctx, room, session, false);
				continue;
			}
			if((uint32_t)kickTime < nextTick)
				nextTick = (uint32_t)kickTime;
			if(session->enet != NULL) {
				eenet_tick(session->enet);
				nextTick = 15;
			} else {
				nextTick = instance_channels_tick(&session->channels, &ctx->net, &session->net, currentTime); // TODO: proper resend timing
			}
		}
		if(!*room)
			continue;

		if((*room)->state & ServerState_Timeout) {
			time_t delta = (*room)->global.timeout - room_get_syncTime(*room);
			if(delta >= 0) {
				if(delta < 10)
					delta = 10;
				if((uint64_t)delta < nextTick)
					nextTick = (uint32_t)delta;
			} else if((*room)->state & ServerState_Game_Results) { // TODO: ServerState_Lobby_Results = ServerState_Lobby_Idle >> 1
				room_set_state(ctx, *room, ServerState_Lobby_Idle);
			} else {
				room_set_state(ctx, *room, (ServerState)((*room)->state << 1));
			}
		}

		FOR_SOME_PLAYERS(id, (*room)->playerSort,)
			net_flush_merged(&ctx->net, &(*room)->players[id].net);
	}
	return nextTick;
}

static const char *instance_domainIPv4 = NULL, *instance_domain = NULL;
static struct IPEndPoint instance_get_endpoint(struct NetContext *net, bool ipv4) {
	struct IPEndPoint out = {
		.address = String_fmt("%s", ipv4 ? instance_domainIPv4 : instance_domain),
	};

	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	getsockname(net_get_sockfd(net), &addr.sa, &addr.len);
	switch(addr.ss.ss_family) {
		case AF_INET: out.port = htons(addr.in.sin_port); break;
		case AF_INET6: out.port = htons(addr.in6.sin6_port); break;
		default:;
	}
	return out;
}

static struct Room **room_open(struct InstanceContext *ctx, uint32_t roomID, struct GameplayServerConfiguration configuration) {
	uprintf("opening room (%zu,%hu)\n", indexof(contexts, ctx), roomID);
	if(instance_get_room(ctx, roomID) == NULL || *instance_get_room(ctx, roomID) != NULL) {
		uprintf("\tBad room ID\n");
		return NULL;
	}
	#ifdef FORCE_LOBBY_SIZE
	configuration.maxPlayerCount = FORCE_LOBBY_SIZE;
	configuration.songSelectionMode = SongSelectionMode_Vote;
	#endif
	// TODO: restore extended lobbies once MpCore patches the 128 player "halt & catch fire" bug
	if(configuration.maxPlayerCount < 1 || (uint32_t)configuration.maxPlayerCount > 126/*bitsize(struct CounterP) - 2*/) {
		uprintf("Requested room size out of range!\n");
		return NULL;
	}
	if(configuration.invitePolicy >= 3 || configuration.invitePolicy < 0)
		configuration.invitePolicy = InvitePolicy_NobodyCanInvite;
	if(instance_mapPool) {
		configuration = (struct GameplayServerConfiguration){
			.maxPlayerCount = configuration.maxPlayerCount,
			.discoveryPolicy = DiscoveryPolicy_Public,
			.invitePolicy = InvitePolicy_AnyoneCanInvite,
			.gameplayServerMode = GameplayServerMode_Managed,
			.songSelectionMode = SongSelectionMode_Random,
			.gameplayServerControlSettings = GameplayServerControlSettings_AllowSpectate,
		};
	}
	// Allocate extra slot for fake player in `SongSelectionMode_Random`
	struct Room *room = malloc(sizeof(struct Room) + ((uint32_t)configuration.maxPlayerCount + 1) * sizeof(*room->players));
	if(room == NULL) {
		uprintf("alloc error\n");
		return NULL;
	}
	net_keypair_init(&ctx->net, &room->keys);
	room->serverOwner = 0;
	room->configuration = configuration;
	if(clock_gettime(CLOCK_MONOTONIC, &room->syncBase))
		room->syncBase = (struct timespec){0};
	room->shortCountdown = 5000;
	room->longCountdown = 15000;
	room->skipResults = false;
	room->perPlayerDifficulty = false;
	room->perPlayerModifiers = false;
	room->joinCount = 0;
	room->connected = COUNTERP_CLEAR;
	room->playerSort = COUNTERP_CLEAR;
	room->state = 0;
	room->global.sessionId[0] = 0;
	room->global.sessionId[1] = 0;
	room->global.inLobby = COUNTERP_CLEAR;
	room->global.isSpectating = COUNTERP_CLEAR;
	room->global.selectedBeatmap = CLEAR_BEATMAP;
	room->global.selectedModifiers = CLEAR_MODIFIERS;
	room->global.roundRobin = 0;
	if(instance_mapPool) {
		room->serverOwner = (playerid_t)room->configuration.maxPlayerCount;
		room->players[room->configuration.maxPlayerCount].userId = String_from("");
	}
	room_set_state(ctx, room, ServerState_Lobby_Idle);
	*instance_get_room(ctx, roomID) = room;
	Counter64_set(&ctx->roomMask, roomID / lengthof(*ctx->rooms));
	return instance_get_room(ctx, roomID);
}

static struct String instance_room_get_managerId(struct Room *room, struct InstanceSession *self) {
	return (&room->players[room->serverOwner] == self) ? OwnUserId(self->userId) : room->players[room->serverOwner].userId;
}

static struct PacketContext instance_room_get_protocol(struct InstanceContext *ctx, uint32_t roomID) {
	struct PacketContext version = PV_LEGACY_DEFAULT;
	struct Room **room = instance_get_room(ctx, roomID);
	if(room == NULL || *room == NULL)
		return (struct PacketContext){0};
	struct CounterP ct = (*room)->playerSort;
	uint32_t id = 0;
	if(CounterP_clear_next(&ct, &id))
		version = (*room)->players[id].net.version;
	return version;
}

static struct WireSessionAllocResp room_resolve_session(struct InstanceContext *ctx, const struct WireSessionAlloc *req) {
	struct Room **const roomPtr = instance_get_room(ctx, req->room);
	struct Room *const room = (roomPtr != NULL) ? *roomPtr : NULL;
	if(room == NULL)
		return (struct WireSessionAllocResp){.result = ConnectToServerResponse_Result_UnknownError};
	if(!AnnotateIDs) {
		FOR_SOME_PLAYERS(id, room->playerSort,)
			if(String_eq(room->players[id].userId, req->userId))
				room_disconnect(ctx, roomPtr, &room->players[id], true);
	}
	struct InstanceSession *session = NULL;
	{
		struct CounterP tmp = room->playerSort;
		uint32_t id = 0;
		if((!CounterP_set_next(&tmp, &id)) || id >= (uint32_t)room->configuration.maxPlayerCount) {
			uprintf("ROOM FULL: %u >= %d\n", id ? id : (uint32_t)bitsize(tmp), room->configuration.maxPlayerCount);
			return (struct WireSessionAllocResp){.result = ConnectToServerResponse_Result_ServerAtCapacity};
		}
		session = &room->players[id];
		room->playerSort = tmp;
	}
	NetSession_init(&session->net, &ctx->net, (struct SS){.ss.ss_family = AF_UNSPEC}, &ctx->base.config);
	session->net.version = req->clientVersion;
	if(!req->clientVersion.direct)
		session->net.clientRandom = req->random;
	*session = (struct InstanceSession){
		.net = session->net,
		.secret = req->secret,
		.userId = AnnotateIDs ? String_fmt("%.*s$%u", req->userId.length, req->userId.data, (uint32_t)indexof(room->players, session)) : req->userId,
		.joinOrder = ++room->joinCount,
	};
	instance_channels_init(&session->channels);

	struct WireSessionAllocResp resp = {
		.result = ConnectToServerResponse_Result_Success,
		.configuration = room->configuration,
		.managerId = instance_room_get_managerId(room, session),
		.endPoint = instance_get_endpoint(&ctx->net, req->ipv4),
		.playerSlot = (uint32_t)indexof(room->players, session),
	};
	if(!req->clientVersion.direct) {
		resp.random = *NetKeypair_get_random(&session->net.keys);
		if(NetKeypair_write_key(&session->net.keys, &ctx->net, &resp.publicKey)) {
			uprintf("Connect to Server Error: NetKeypair_write_key() failed\n");
			return (struct WireSessionAllocResp){.result = ConnectToServerResponse_Result_UnknownError}; // TODO: clean up and remove session data if either of these errors is hit
		}
		if(NetSession_set_remotePublicKey(&session->net, &ctx->net, &req->publicKey, false)) {
			uprintf("Connect to Server Error: NetSession_set_remotePublicKey() failed\n");
			return (struct WireSessionAllocResp){.result = ConnectToServerResponse_Result_UnknownError};
		}
	}
	log_players(room, NULL, "resolve");
	return resp;
}

// TODO: can errors here be reported back to the client?
static struct InstanceSession *instance_onGraphAuth(struct NetContext *net, struct NetSession *masterSession, const struct GraphAuthToken *token, struct Room ***room_out) {
	struct InstanceContext *const ctx = (struct InstanceContext*)net->userptr;
	const char *const sep = memchr(token->base.playerSessionId.data, ',', token->base.playerSessionId.length);
	if(token->base.playerSessionId.length < 11 || memcmp(token->base.playerSessionId.data, "pslot$", 6) || sep == NULL || &token->base.playerSessionId.data[token->base.playerSessionId.length] - sep != 4) {
		uprintf("Auth failed: bad playerSessionId \"%.*s\"\n", token->base.playerSessionId.length, token->base.playerSessionId.data);
		return NULL;
	}
	uint32_t roomID = 0;
	for(const char *it = &token->base.playerSessionId.data[6]; it < sep; ++it) {
		if(*it < '0' || *it > '9') {
			uprintf("Auth failed: bad room ID\n");
			return NULL;
		}
		roomID = roomID * 10 + ((uint32_t)*it - '0');
	}
	struct Room **const roomPtr = instance_get_room(ctx, roomID);
	struct Room *const room = (roomPtr != NULL) ? *roomPtr : NULL;
	if(room == NULL) {
		uprintf("Auth failed: room closed\n");
		return NULL;
	}
	const playerid_t id = ((uint32_t)sep[1] - '0') * 100 | ((uint32_t)sep[2] - '0') * 10 | ((uint32_t)sep[3] - '0');
	if(id >= (uint32_t)room->configuration.maxPlayerCount) {
		uprintf("Auth failed: player slot out of range\n");
		return NULL;
	}
	struct InstanceSession *const session = &room->players[id];
	if(session->net.addr.ss.ss_family != AF_UNSPEC || !session->net.version.direct) {
		uprintf("Auth failed: bad target (%zu,%hu)@%u\n", indexof(contexts, ctx), roomID, id);
		return NULL;
	}
	const struct String targetId = OwnUserId(session->userId);
	if(!String_eq(token->base.userId, targetId)) {
		uprintf("Auth failed: bad user ID (\"%.*s\" != \"%.*s\")\n", token->base.userId.length, token->base.userId.data, targetId.length, targetId.data);
		return NULL;
	}
	const struct PacketContext version = session->net.version;
	NetSession_free(&session->net);
	NetSession_initFrom(&session->net, masterSession);
	session->net.version = version;
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	uprintf("auth{%s} %s -> (%zu,%hu)@%u\n", (token->enet != NULL) ? "ENet" : "Graph", addrstr, indexof(contexts, ctx), roomID, id);
	*room_out = roomPtr;
	return session;
}

static void instance_room_spawn(struct InstanceContext *ctx, struct WireLink *link, uint32_t cookie, const struct WireRoomSpawn *req) {
	struct WireMessage r_alloc = {
		.type = WireMessageType_WireRoomSpawnResp,
		.cookie = cookie,
		.roomSpawnResp.base.result = ConnectToServerResponse_Result_UnknownError,
	};
	struct Room **room = room_open(ctx, req->base.room, req->configuration);
	if(room) {
		r_alloc.roomSpawnResp.base = room_resolve_session(ctx, &req->base);
		if(r_alloc.roomSpawnResp.base.result != ConnectToServerResponse_Result_Success)
			room_free(ctx, room);
	} else {
		uprintf("room_open() failed\n");
	}
	WireLink_send(link, &r_alloc);
}

static void instance_room_join(struct InstanceContext *ctx, struct WireLink *link, uint32_t cookie, const struct WireRoomJoin *req) {
	struct WireMessage r_alloc = {
		.type = WireMessageType_WireRoomJoinResp,
		.cookie = cookie,
	};
	uint32_t roomProtocol = instance_room_get_protocol(ctx, req->base.room).protocolVersion;
	if(roomProtocol == 0) {
		// This condition can happen if a player joins while the WireRoomCloseNotify message still in flight,
		//     or if the instance and master have desynced.
		uprintf("Connect to Server Error: Room closed (room=%u, client=%u)\n", roomProtocol, req->base.clientVersion.protocolVersion);
		r_alloc.roomJoinResp.base.result = ConnectToServerResponse_Result_InvalidCode;
	} else if(roomProtocol != req->base.clientVersion.protocolVersion) {
		uprintf("Connect to Server Error: Version mismatch (room=%u, client=%u)\n", roomProtocol, req->base.clientVersion.protocolVersion);
		r_alloc.roomJoinResp.base.result = ConnectToServerResponse_Result_VersionMismatch;
	} else {
		r_alloc.roomJoinResp.base = room_resolve_session(ctx, &req->base);
	}
	WireLink_send(link, &r_alloc);
}

static void instance_onWireMessage(struct WireContext *wire, struct WireLink *link, const struct WireMessage *message) {
	struct InstanceContext *const ctx = (struct InstanceContext*)wire->userptr;
	net_lock(&ctx->net);
	if(link != ctx->master)
		goto unlock;
	if(message == NULL) {
		ctx->master = NULL;
		goto unlock;
	}
	switch(message->type) {
		case WireMessageType_WireRoomSpawn: instance_room_spawn(ctx, link, message->cookie, &message->roomSpawn); break;
		case WireMessageType_WireRoomJoin: instance_room_join(ctx, link, message->cookie, &message->roomJoin); break;
		default: uprintf("Unhandled wire message [%s]\n", reflect(WireMessageType, message->type));
	}
	unlock: net_unlock(&ctx->net);
}

static uint32_t threads_len = 0;
static pthread_t *threads = NULL;
bool instance_init(const char *domainIPv4, const char *domain, const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, const char *remoteMaster, struct WireContext *localMaster, const char *mapPoolFile, uint32_t count) {
	if(mapPoolFile && *mapPoolFile)
		mapPool_init(mapPoolFile);
	instance_domainIPv4 = domainIPv4;
	instance_domain = domain;
	instance_master.isRemote = (*remoteMaster != 0);
	if(instance_master.isRemote)
		instance_master.remote = remoteMaster;
	else
		instance_master.local = localMaster;
	threads_len = 0;
	contexts = malloc(count * sizeof(*contexts));
	threads = malloc(count * sizeof(*threads));
	if(!contexts || !threads) {
		uprintf("alloc error\n");
		return true;
	}
	for(; threads_len < count; ++threads_len) {
		struct InstanceContext *const ctx = &contexts[threads_len];
		if(net_init(&ctx->net, (uint16_t)(8106 + threads_len))) {
			uprintf("net_init() failed\n");
			return true;
		}
		MasterContext_init(&ctx->base, &ctx->net.ctr_drbg);
		if(MasterContext_setCertificate(&ctx->base, cert, key)) {
			net_cleanup(&ctx->net);
			return true;
		}
		if(WireContext_init(&ctx->wire, ctx, 0)) {
			net_cleanup(&ctx->net);
			MasterContext_cleanup(&ctx->base);
			uprintf("WireContext_init() failed\n");
			return true;
		}
		ctx->net.userptr = ctx;
		ctx->net.onResolve = instance_onResolve;
		ctx->net.onResend = instance_onResend;
		ctx->wire.onMessage = instance_onWireMessage;
		ctx->roomMask = COUNTER64_CLEAR;
		ctx->master = NULL;
		memset(ctx->rooms, 0, sizeof(ctx->rooms));

		if(pthread_create(&threads[threads_len], NULL, (void *(*)(void*))instance_handler, ctx)) {
			threads[threads_len] = NET_THREAD_INVALID;
			WireContext_cleanup(&ctx->wire);
			net_cleanup(&ctx->net);
			MasterContext_cleanup(&ctx->base);
			uprintf("Instance thread creation failed\n");
			return true;
		}
	}
	return false;
}

void instance_cleanup() {
	for(uint32_t i = 0; i < threads_len; ++i) {
		if(threads[i] == NET_THREAD_INVALID)
			continue;
		struct InstanceContext *ctx = &contexts[i];
		net_stop(&ctx->net);
		uprintf("Stopping #%u\n", i);
		pthread_join(threads[i], NULL);
		threads[i] = 0;
		FOR_ALL_ROOMS(ctx, room) {
			FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
				instance_channels_reset(&(*room)->players[id].channels);
				NetSession_free(&(*room)->players[id].net);
			}
			room_free(ctx, room);
		}
		ctx->roomMask = COUNTER64_CLEAR; // should be redundant, but just to be safe
		memset(ctx->rooms, 0, sizeof(ctx->rooms));
		WireContext_cleanup(&ctx->wire);
		net_cleanup(&ctx->net);
		MasterContext_cleanup(&ctx->base);
	}
	free(instance_mapPool);
	free(threads);
	free(contexts);
	instance_mapPool = NULL;
}
