#include "instance.h"
#include "common.h"
#include "../counter.h"
#include <mbedtls/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>

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
	for(playerid_t (id) = 0; CounterP_clear_next(&COUNTER_VAR, &(id)); ++(id))

#define FOR_EXCLUDING_PLAYER(id, counter, exc) \
	FOR_SOME_PLAYERS(id, counter, CounterP_clear(&COUNTER_VAR, exc))

#define FOR_ALL_ROOMS(ctx, room) \
	struct Counter64 COUNTER_VAR = (ctx)->roomMask; \
	for(uint32_t group; Counter64_clear_next(&COUNTER_VAR, &group);) \
		for(struct Room **(room) = (ctx)->rooms[group]; (room) < endof((ctx)->rooms[group]); ++(room)) \
			if(*room)

struct InstanceSession {
	struct NetSession net;
	struct String secret;
	struct String userName, userId;
	struct PingPong tableTennis;
	struct Channels channels;
	struct PlayerStateHash stateHash;
	struct MultiplayerAvatarData avatar;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicEncryptionKey;
	bool sentIdentity, directDownloads;
	uint32_t joinOrder;

	ServerState state;
	float recommendTime;
	struct BeatmapIdentifierNetSerializable recommendedBeatmap;
	struct GameplayModifiers recommendedModifiers;
	struct PlayerSpecificSettingsNetSerializable settings;
};
struct Room {
	struct NetKeypair keys;
	playerid_t serverOwner;
	struct GameplayServerConfiguration configuration;
	float syncBase, shortCountdown, longCountdown;
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
		float timeout;
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
			float startTime;
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
	struct NetContext net;
	union WireLink *master;
	struct Counter64 roomMask;
	struct Room *rooms[64][8];
};
static struct InstanceContext *contexts = NULL;

static float room_get_syncTime(struct Room *room) {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec + (now.tv_nsec / 1000) / 1000000.f - room->syncBase;
}

static bool PlayerStateHash_contains(struct PlayerStateHash state, const char *key) {
	uint32_t len = strlen(key);
	uint32_t hash = 0x21 ^ len;
	int32_t num3 = 0;
	while(len >= 4) {
		uint32_t num4 = (key[num3 + 3] << 24) | (key[num3 + 2] << 16) | (key[num3 + 1] << 8) | key[num3];
		num4 *= 1540483477;
		num4 ^= num4 >> 24;
		num4 *= 1540483477;
		hash *= 1540483477;
		hash ^= num4;
		num3 += 4;
		len -= 4;
	}
	switch(len) {
		case 3:
			hash ^= key[num3 + 2] << 16; [[fallthrough]];
		case 2:
			hash ^= key[num3 + 1] << 8; [[fallthrough]];
		case 1:
			hash ^= key[num3];
			hash *= 1540483477; [[fallthrough]];
		case 0:
			break;
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
	uint8_t index = indexof(list, session);
	return index + 1 + (index >= 126);
}

static uint8_t ConnectionId_index(uint8_t connectionId) {
	return connectionId - 1 - (connectionId >= 128);
}

static bool BeatmapIdentifierNetSerializable_eq(const struct BeatmapIdentifierNetSerializable *a, const struct BeatmapIdentifierNetSerializable *b, bool ignoreDifficulty) {
	if(!String_eq(a->levelID, b->levelID))
		return false;
	return ignoreDifficulty || (String_eq(a->beatmapCharacteristicSerializedName, b->beatmapCharacteristicSerializedName) && a->difficulty == b->difficulty);
}

static bool GameplayModifiers_eq(const struct GameplayModifiers *a, const struct GameplayModifiers *b, bool optional) {
	struct GameplayModifiers delta = {a->raw ^ b->raw};
	struct GameplayModifiers mask = {REQUIRED_MODIFIER_MASK};
	mask.raw |= optional * ~0u;
	return (delta.raw & mask.raw) == 0;
}

static inline struct PlayerLobbyPermissionConfigurationNetSerializable session_get_permissions(const struct Room *room, const struct InstanceSession *session) {
	bool isServerOwner = (indexof(room->players, session) == room->serverOwner);
	bool canSuggest = (room->configuration.songSelectionMode != SongSelectionMode_Random);
	return (struct PlayerLobbyPermissionConfigurationNetSerializable){
		.userId = session->userId,
		.isServerOwner = isServerOwner,
		.hasRecommendBeatmapsPermission = canSuggest,
		.hasRecommendGameplayModifiersPermission = canSuggest && (room->configuration.gameplayServerControlSettings & GameplayServerControlSettings_AllowModifierSelection),
		.hasKickVotePermission = isServerOwner,
		.hasInvitePermission = (bool[]){true, isServerOwner, false}[room->configuration.invitePolicy],
	};
}

static struct BeatmapIdentifierNetSerializable session_get_beatmap(const struct Room *room, const struct InstanceSession *session) {
	if(room->perPlayerDifficulty && BeatmapIdentifierNetSerializable_eq(&session->recommendedBeatmap, &room->global.selectedBeatmap, 1))
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
	size_t raw_len = ftell(file);
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
	uint16_t formatVersion = raw[4] | raw[5] << 8;
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
		if(!pkt_read(&(struct MpBeatmapPacket){.difficulty=0}, &raw_it, raw_end, (struct PacketContext){12, 6, 0, 64}))
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
		pkt_read(&instance_mapPool[i], &raw_it, raw_end, (struct PacketContext){12, 6, 0, 64});
	return;
	fail:
	fclose(file);
}

static void mapPool_update(struct Room *room) {
	if(!instance_mapPool) {
		uprintf("No map pool to select from!\n");
		return;
	}
	room->lobby.requester = room->configuration.maxPlayerCount;
	room->global.selectedBeatmap = (struct BeatmapIdentifierNetSerializable){
		.levelID = LongString_from("custom_level_"),
		.beatmapCharacteristicSerializedName = instance_mapPool[room->global.roundRobin].characteristic,
		.difficulty = instance_mapPool[room->global.roundRobin].difficulty,
	};
	room->global.selectedModifiers.raw = GameplayModifierFlags_NoFailOn0Energy;
	memcpy(&room->global.selectedBeatmap.levelID.data[13], instance_mapPool[room->global.roundRobin].levelHash.data, instance_mapPool[room->global.roundRobin].levelHash.length);
	room->global.selectedBeatmap.levelID.length += instance_mapPool[room->global.roundRobin].levelHash.length;

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

static float room_get_countdownEnd(const struct Room *room, float defaultTime) {
	switch(room->state) {
		case ServerState_Lobby_LongCountdown: return room->global.timeout + room->shortCountdown;
		case ServerState_Lobby_ShortCountdown: return room->global.timeout;
		default: return defaultTime;
	}
	
}

#define STATE_EDGE(from, to, mask) ((to & (mask)) && !(from & (mask)))
static bool room_try_finish(struct InstanceContext *ctx, struct Room *room);
static void session_set_state(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, ServerState state) {
	struct RemoteProcedureCall base = {
		.syncTime = room_get_syncTime(room),
	};
	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
	uint8_t *start = resp_end;
	if(STATE_EDGE(session->state, state, ServerState_Connected)) {
		CounterP_set(&room->connected, indexof(room->players, session));
	} else if(STATE_EDGE(state, session->state, ServerState_Connected)) {
		CounterP_clear(&room->connected, indexof(room->players, session));
		if(room->configuration.songSelectionMode != SongSelectionMode_Random && room->global.roundRobin == indexof(room->players, session))
			room->global.roundRobin = roundRobin_next(room->global.roundRobin, room->connected);
		struct InternalMessage r_disconnect = {
			.type = InternalMessageType_PlayerDisconnected,
			.playerDisconnected = {
				.disconnectedReason = DisconnectedReason_ClientConnectionClosed,
			},
		};
		FOR_SOME_PLAYERS(id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {InstanceSession_connectionId(room->players, session), 0, false});
			if(pkt_serialize(&r_disconnect, &resp_end, endof(resp), room->players[id].net.version))
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}
		if(room->state & ServerState_Game) {
			CounterP_clear(&room->game.activePlayers, indexof(room->players, session));
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
				.returnToMenu = {
					.base = base,
				},
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
				.cancelCountdown = {base},
			});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_CancelLevelStart,
				.cancelLevelStart = {base},
			});
		}
		if(needSetSelectedBeatmap) {
			if(room->configuration.songSelectionMode == SongSelectionMode_Random && instance_mapPool) {
				SERIALIZE_MPCORE(&resp_end, endof(resp), session->net.version, {
					.type = String_from("MpBeatmapPacket"),
					.mpBeatmapPacket = instance_mapPool[room->global.roundRobin],
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
					.startTime = room->global.timeout + 1048576,
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
				.cancelCountdown = {base},
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
							.mimeType = CLEAR_STRING,
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
					.newTime = base.syncTime,
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
			struct PlayerSpecificSettingsAtStartNetSerializable *playerSettings;
			struct String *id;
			bool active = CounterP_get(room->game.activePlayers, indexof(room->players, session));
			if(active) {
				r_sync = (struct GameplayRpc){
					.type = GameplayRpcType_SetGameplaySceneSyncFinish,
					.setGameplaySceneSyncFinish = {
						.base = base,
						.flags = {true, true, false, false},
						.playersAtGameStart.count = 0,
					},
				};
				playerSettings = &r_sync.setGameplaySceneSyncFinish.playersAtGameStart;
				id = &r_sync.setGameplaySceneSyncFinish.sessionGameId;
			} else {
				r_sync = (struct GameplayRpc){
					.type = GameplayRpcType_SetActivePlayerFailedToConnect,
					.setActivePlayerFailedToConnect = {
						.base = base,
						.flags = {true, true, true, false},
						.failedUserId = session->userId,
						.playersAtGameStart.count = 0,
					},
				};
				playerSettings = &r_sync.setActivePlayerFailedToConnect.playersAtGameStart;
				id = &r_sync.setActivePlayerFailedToConnect.sessionGameId;
			}
			id->length = snprintf(id->data, sizeof(id->data), "%08"PRIx64"-%04"PRIx64"-%04"PRIx64"-%04"PRIx64"-%012"PRIx64, (room->global.sessionId[0] >> 32) & 0xffffffff, (room->global.sessionId[0] >> 16) & 0xffff, room->global.sessionId[0] & 0xffff, (room->global.sessionId[1] >> 48) & 0xffff, room->global.sessionId[1] & 0xffffffffffff);
			FOR_SOME_PLAYERS(id, room->game.activePlayers,)
				playerSettings->activePlayerSpecificSettingsAtGameStart[playerSettings->count++] = room->players[id].settings;
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
					.startTime = room->game.startTime,
				},
			});
			break;
		}
		case ServerState_Game_Results: break;
	}
	if(resp_end != start)
		instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
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

static void room_set_state(struct InstanceContext *ctx, struct Room *room, ServerState state) {
	uprintf("state %s -> %s\n", ServerState_toString(room->state), ServerState_toString(state));
	if(STATE_EDGE(room->state, state, ServerState_Lobby)) {
		room->global.selectedBeatmap = CLEAR_BEATMAP;
		room->global.selectedModifiers = CLEAR_MODIFIERS;
		room->lobby.isEntitled = COUNTER128_CLEAR;
		room->lobby.isDownloaded = COUNTER128_CLEAR;
		room->lobby.isReady = COUNTER128_CLEAR;
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
			playerid_t select = ~0u;
			switch(room->configuration.songSelectionMode) {
				case SongSelectionMode_Vote: vote_beatmap: {
					uint32_t max = 0;
					FOR_SOME_PLAYERS(id, room->connected,) {
						if(!room->players[id].recommendedBeatmap.beatmapCharacteristicSerializedName.length)
							continue;
						uint32_t biasedVotes = (id >= room->global.roundRobin) + 1;
						float requestTime = room->players[id].recommendTime;
						playerid_t firstRequest = id;
						FOR_EXCLUDING_PLAYER(cmp, room->connected, id) { // TODO: this scales horribly
							if(!BeatmapIdentifierNetSerializable_eq(&room->players[id].recommendedBeatmap, &room->players[cmp].recommendedBeatmap, room->perPlayerDifficulty))
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
				case SongSelectionMode_Random: select = room->configuration.maxPlayerCount; break;
				case SongSelectionMode_OwnerPicks: {
					if(!CounterP_get(room->connected, room->serverOwner))
						goto vote_beatmap;
					select = room->serverOwner;
					break;
				}
				case SongSelectionMode_RandomPlayerPicks: {
					if(room->players[room->global.roundRobin].recommendedBeatmap.beatmapCharacteristicSerializedName.length)
						select = room->global.roundRobin;
					break;
				}
			}
			room->lobby.requester = select;
			if(select == (playerid_t)~0u || room->players[select].recommendedBeatmap.beatmapCharacteristicSerializedName.length == 0) {
				uprintf("No map from %u\n", select);
				room->lobby.isEntitled = room->connected;
				room->global.selectedBeatmap = CLEAR_BEATMAP;
				room->global.selectedModifiers = CLEAR_MODIFIERS;
				room_set_state(ctx, room, ServerState_Lobby_Idle);
				return;
			}
			if((room->state & ServerState_Selected) && CounterP_eq(room->lobby.isEntitled, room->connected) && BeatmapIdentifierNetSerializable_eq(&room->players[select].recommendedBeatmap, &room->global.selectedBeatmap, 0))
				return;
			room->lobby.reason = 0;
			room->lobby.isEntitled = COUNTER128_CLEAR;
			room->lobby.isDownloaded = COUNTER128_CLEAR;
			room->global.selectedBeatmap = room->players[select].recommendedBeatmap;
			room->global.selectedModifiers = room->players[select].recommendedModifiers;
			room->lobby.entitlement.missing = COUNTER128_CLEAR;
			break;
		}
		case ServerState_Lobby_Idle: {
			if(room->global.selectedBeatmap.beatmapCharacteristicSerializedName.length == 0)
				room->lobby.reason = CannotStartGameReason_NoSongSelected;
			else
				room->lobby.reason = CannotStartGameReason_DoNotOwnSong;
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
			room->game.loadingScene.isLoaded = COUNTER128_CLEAR;
			room->global.timeout = room_get_syncTime(room) + LOAD_TIMEOUT;
			break;
		}
		case ServerState_Game_LoadingSong: {
			if(room->state & ServerState_Game_LoadingScene) {
				room->game.activePlayers = CounterP_and(room->game.activePlayers, room->game.loadingScene.isLoaded);
				if(room_try_finish(ctx, room))
					return;
			}
			mbedtls_ctr_drbg_random(&ctx->net.ctr_drbg, (uint8_t*)room->global.sessionId, sizeof(room->global.sessionId));
			room->game.loadingSong.isLoaded = COUNTER128_CLEAR;
			room->global.timeout = room_get_syncTime(room) + LOAD_TIMEOUT;
			break;
		}
		case ServerState_Game_Gameplay: {
			if(room->state & ServerState_Game_LoadingSong) {
				room->game.activePlayers = CounterP_and(room->game.activePlayers, room->game.loadingSong.isLoaded);
				if(room_try_finish(ctx, room))
					return;
			}
			room->game.startTime = room_get_syncTime(room) + .25;
			break;
		}
		case ServerState_Game_Results: room->global.timeout = room_get_syncTime(room) + (room->game.showResults ? 20 : 1); break;
	}
	room->state = state;
	FOR_SOME_PLAYERS(id, room->connected,)
		session_set_state(ctx, room, &room->players[id], state);
}

static struct PlayersLobbyPermissionConfigurationNetSerializable room_get_permissions(const struct Room *room) {
	struct PlayersLobbyPermissionConfigurationNetSerializable out = {
		.count = 0,
	};
	FOR_SOME_PLAYERS(id, room->connected,)
		out.playersPermission[out.count++] = session_get_permissions(room, &room->players[id]);
	return out;
}

static void handle_MenuRpc(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct MenuRpc *rpc) {
	switch(rpc->type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: uprintf("BAD TYPE: MenuRpcType_SetPlayersMissingEntitlementsToLevel\n"); break;
		case MenuRpcType_GetIsEntitledToLevel: uprintf("BAD TYPE: MenuRpcType_GetIsEntitledToLevel\n"); break;
		case MenuRpcType_SetIsEntitledToLevel: {
			struct SetIsEntitledToLevel entitlement = rpc->setIsEntitledToLevel;
			if(!((room->state & ServerState_Lobby) && entitlement.flags.hasValue0 && String_eq(entitlement.levelId, room->global.selectedBeatmap.levelID)))
				break;
			if(!entitlement.flags.hasValue1 || entitlement.entitlementStatus == EntitlementsStatus_Unknown) {
				entitlement.entitlementStatus = EntitlementsStatus_NotOwned;
			} else if(entitlement.entitlementStatus == EntitlementsStatus_Ok) {
				if(!PlayerStateHash_contains(session->stateHash, "modded") && entitlement.levelId.length >= 13 && memcmp(entitlement.levelId.data, "custom_level_", 13) == 0)
					entitlement.entitlementStatus = EntitlementsStatus_NotOwned; // Vanilla clients will misreport all custom IDs as owned
				else if(CounterP_set(&room->lobby.isDownloaded, indexof(room->players, session)) == 0)
					if((room->state & ServerState_Lobby_Downloading) && CounterP_contains(room->lobby.isDownloaded, room->connected))
						room_set_state(ctx, room, ServerState_Game_LoadingScene);
			}
			if(!(room->state & ServerState_Lobby_Entitlement))
				break;
			if(CounterP_set(&room->lobby.isEntitled, indexof(room->players, session)))
				break;
			if(entitlement.entitlementStatus != EntitlementsStatus_Ok && entitlement.entitlementStatus != EntitlementsStatus_NotDownloaded)
				CounterP_set(&room->lobby.entitlement.missing, indexof(room->players, session));
			uprintf("entitlement[%.*s]: %s\n", session->userName.length, session->userName.data, reflect(EntitlementsStatus, entitlement.entitlementStatus));
			if(!CounterP_contains(room->lobby.isEntitled, room->connected))
				break;
			struct MenuRpc r_missing = {
				.type = MenuRpcType_SetPlayersMissingEntitlementsToLevel,
				.setPlayersMissingEntitlementsToLevel = {
					.base.syncTime = room_get_syncTime(room),
					.flags = {true, false, false, false},
					.playersMissingEntitlements.count = 0,
				},
			};
			FOR_SOME_PLAYERS(id, room->lobby.entitlement.missing,)
				r_missing.setPlayersMissingEntitlementsToLevel.playersMissingEntitlements.playersWithoutEntitlements[r_missing.setPlayersMissingEntitlementsToLevel.playersMissingEntitlements.count++] = room->players[id].userId;
			FOR_SOME_PLAYERS(id, room->connected,) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 127, false});
				SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, r_missing);
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			}
			if(r_missing.setPlayersMissingEntitlementsToLevel.playersMissingEntitlements.count == 0) {
				room_set_state(ctx, room, ServerState_Lobby_Ready);
			} else if(room->configuration.songSelectionMode == SongSelectionMode_Random) {
				room->global.roundRobin = roundRobin_next(room->global.roundRobin, COUNTER128_CLEAR);
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
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = room->global.selectedBeatmap.levelID.length ? MenuRpcType_SetSelectedBeatmap : MenuRpcType_ClearSelectedBeatmap,
				.setSelectedBeatmap = {
					.base = base,
					.flags = {true, false, false, false},
					.identifier = session_get_beatmap(room, session),
				},
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_RecommendBeatmap: {
			struct RecommendBeatmap beatmap = rpc->recommendBeatmap;
			if(!beatmap.flags.hasValue0) {
				case MenuRpcType_ClearRecommendedBeatmap:
				beatmap.identifier = CLEAR_BEATMAP;
			}
			if(!((room->state & ServerState_Lobby) && session_get_permissions(room, session).hasRecommendBeatmapsPermission))
				break;
			if(!BeatmapIdentifierNetSerializable_eq(&session->recommendedBeatmap, &beatmap.identifier, room->perPlayerDifficulty))
				session->recommendTime = room_get_syncTime(room);
			session->recommendedBeatmap = beatmap.identifier;
			room_set_state(ctx, room, ServerState_Lobby_Entitlement);
			break;
		}
		case MenuRpcType_GetRecommendedBeatmap: break;
		case MenuRpcType_SetSelectedGameplayModifiers: uprintf("BAD TYPE: MenuRpcType_SetSelectedGameplayModifiers\n"); break;
		case MenuRpcType_GetSelectedGameplayModifiers: {
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetSelectedGameplayModifiers,
				.setSelectedGameplayModifiers = {
					.base = base,
					.flags = {true, false, false, false},
					.gameplayModifiers = session_get_modifiers(room, session),
				},
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_RecommendGameplayModifiers: {
			struct RecommendGameplayModifiers modifiers = rpc->recommendGameplayModifiers;
			if(!modifiers.flags.hasValue0) {
				case MenuRpcType_ClearRecommendedGameplayModifiers:
				modifiers.gameplayModifiers = CLEAR_MODIFIERS;
			}
			if(!((room->state & ServerState_Lobby) && session_get_permissions(room, session).hasRecommendGameplayModifiersPermission))
				break;
			session->recommendedModifiers = modifiers.gameplayModifiers;
			if(indexof(room->players, session) != room->lobby.requester)
				break;
			room->global.selectedModifiers = modifiers.gameplayModifiers;
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			FOR_SOME_PLAYERS(id, room->connected,) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 127, false});
				SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
					.type = MenuRpcType_SetSelectedGameplayModifiers,
					.setSelectedGameplayModifiers = {
						.base = base,
						.flags = {true, false, false, false},
						.gameplayModifiers = session_get_modifiers(room, session),
					},
				});
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
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
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_StartLevel,
				.startLevel = {
					.base = base,
					.flags = {true, true, true, false},
					.beatmapId = session_get_beatmap(room, session),
					.gameplayModifiers = session_get_modifiers(room, session),
					.startTime = base.syncTime,
				},
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			session_set_state(ctx, room, session, ServerState_Game_LoadingScene);
			break;
		}
		case MenuRpcType_CancelLevelStart: uprintf("BAD TYPE: MenuRpcType_CancelLevelStart\n"); break;
		case MenuRpcType_GetMultiplayerGameState: {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetMultiplayerGameState,
				.setMultiplayerGameState = {
					.base.syncTime = room_get_syncTime(room),
					.flags = {true, false, false, false},
					.lobbyState = (room->state & ServerState_Lobby) ? MultiplayerGameState_Lobby : MultiplayerGameState_Game,
				},
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetMultiplayerGameState: uprintf("BAD TYPE: MenuRpcType_SetMultiplayerGameState\n"); break;
		case MenuRpcType_GetIsReady: break;
		case MenuRpcType_SetIsReady: {
			bool isReady = rpc->setIsReady.flags.hasValue0 && rpc->setIsReady.isReady;
			if(room->state & ServerState_Lobby)
				if(CounterP_overwrite(&room->lobby.isReady, indexof(room->players, session), isReady) != isReady && (room->state & ServerState_Selected))
					room_set_state(ctx, room, ServerState_Lobby_Ready);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_SetStartGameTime);
		NOT_IMPLEMENTED(MenuRpcType_CancelStartGameTime);
		case MenuRpcType_GetIsInLobby: break;
		case MenuRpcType_SetIsInLobby: {
			bool inLobby = rpc->setIsInLobby.flags.hasValue0 && rpc->setIsInLobby.isBack;
			if(CounterP_overwrite(&room->global.inLobby, indexof(room->players, session), inLobby) != inLobby && (room->state & ServerState_Selected))
				room_set_state(ctx, room, ServerState_Lobby_Ready);
			break;
		}
		case MenuRpcType_GetCountdownEndTime: {
			if(!(room->state & ServerState_Lobby))
				break;
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
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
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetCountdownEndTime: uprintf("BAD TYPE: MenuRpcType_SetCountdownEndTime\n"); break;
		case MenuRpcType_CancelCountdown: uprintf("BAD TYPE: MenuRpcType_CancelCountdown\n"); break;
		NOT_IMPLEMENTED(MenuRpcType_GetOwnedSongPacks);
		case MenuRpcType_SetOwnedSongPacks: break;
		case MenuRpcType_RequestKickPlayer: {
			if(!(session_get_permissions(room, session).hasKickVotePermission && rpc->requestKickPlayer.flags.hasValue0))
				break;
			struct CounterP players = room->playerSort;
			CounterP_clear(&players, indexof(room->players, session));
			FOR_SOME_PLAYERS(id, players,) {
				if(!String_eq(room->players[id].userId, rpc->requestKickPlayer.kickedPlayerId))
					continue;
				uint8_t resp[65536], *resp_end = resp;
				struct InternalMessage r_kick = {
					.type = InternalMessageType_KickPlayer,
					.kickPlayer.disconnectedReason = DisconnectedReason_Kicked,
				};
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 0, false});
				if(pkt_serialize(&r_kick, &resp_end, endof(resp), room->players[id].net.version))
					instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
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
					.base.syncTime = room_get_syncTime(room),
					.flags = {true, false, false, false},
					.playersPermissionConfiguration = room_get_permissions(room),
				},
			};
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, r_permission);
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_SetPermissionConfiguration);
		case MenuRpcType_GetIsStartButtonEnabled: {
			uprintf("GET BUTTON GET BUTTON GET BUTTON GET BUTTON GET BUTTON\n");
			if(!(room->state & ServerState_Lobby))
				break;
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
			SERIALIZE_MENURPC(&resp_end, endof(resp), session->net.version, {
				.type = MenuRpcType_SetIsStartButtonEnabled,
				.setIsStartButtonEnabled = {
					.base = base,
					.flags = {true, false, false, false},
					.reason = room->lobby.reason,
				},
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetIsStartButtonEnabled: uprintf("BAD TYPE: MenuRpcType_SetIsStartButtonEnabled\n"); break;
		default: uprintf("BAD MENU RPC TYPE\n");
	}
}

static bool room_try_finish(struct InstanceContext *ctx, struct Room *room) {
	if(!CounterP_isEmpty(room->game.activePlayers))
		return false;
	room->global.roundRobin = roundRobin_next(room->global.roundRobin, (room->configuration.songSelectionMode == SongSelectionMode_Random) ? COUNTER128_CLEAR : room->connected);
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
			session->settings = rpc->setGameplaySceneReady.flags.hasValue0 ? rpc->setGameplaySceneReady.playerSpecificSettingsNetSerializable : CLEAR_SETTINGS;
			if(CounterP_set(&room->game.loadingScene.isLoaded, indexof(room->players, session)) == 0)
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
			if(CounterP_set(&room->game.loadingSong.isLoaded, indexof(room->players, session)) == 0)
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
			if(!CounterP_clear(&room->game.activePlayers, indexof(room->players, session)))
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

/*static void handle_MpCore(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct MpCore *mpCore) {
	switch(MpCoreType_From(&mpCore->type)) {
		case MpCoreType_MpBeatmapPacket: break;
		case MpCoreType_MpPlayerData: break;
		case MpCoreType_CustomAvatarPacket: break;
		default: uprintf("BAD MPCORE MESSAGE TYPE: '%.*s'\n", mpCore->type.length, mpCore->type.data);
	}
}*/

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

static bool handle_MultiplayerSession(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct MultiplayerSession *message) {
	switch(message->type) {
		case MultiplayerSessionMessageType_MenuRpc: handle_MenuRpc(ctx, room, session, &message->menuRpc); break;
		case MultiplayerSessionMessageType_GameplayRpc: handle_GameplayRpc(ctx, room, session, &message->gameplayRpc); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: break;
		case MultiplayerSessionMessageType_ScoreSyncState: break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: break;
		case MultiplayerSessionMessageType_MpCore: /*handle_MpCore(ctx, room, session, &message->mpCore);*/ break;
		case MultiplayerSessionMessageType_BeatUpMessage: return handle_BeatUpMessage(&message->beatUpMessage);
		default: uprintf("BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
	}
	return true;
}

static void session_refresh_stateHash(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session) {
	bool isSpectating = !PlayerStateHash_contains(session->stateHash, "wants_to_play_next_level");
	if(CounterP_overwrite(&room->global.isSpectating, indexof(room->players, session), isSpectating) != isSpectating && (room->state & ServerState_Selected))
		room_set_state(ctx, room, ServerState_Lobby_Ready);
}

static void handle_PlayerIdentity(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct PlayerIdentity *identity) {
	session->stateHash = identity->playerState;
	session_refresh_stateHash(ctx, room, session);
	session->avatar = identity->playerAvatar;
	if(identity->random.length == sizeof(session->random))
		memcpy(session->random, identity->random.data, sizeof(session->random));
	else
		memset(session->random, 0, sizeof(session->random));
	session->publicEncryptionKey = identity->publicEncryptionKey;
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
				.isConnectionOwner = 0,
			},
		};
		FOR_EXCLUDING_PLAYER(id, room->connected, indexof(room->players, session)) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 0, false});
			if(pkt_serialize(&r_connected, &resp_end, endof(resp), room->players[id].net.version))
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}

		struct InternalMessage r_sort = {
			.type = InternalMessageType_PlayerSortOrderUpdate,
			.playerSortOrderUpdate = {
				.userId = session->userId,
				.sortIndex = indexof(room->players, session),
			},
		};
		FOR_SOME_PLAYERS(id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 0, false});
			if(pkt_serialize(&r_sort, &resp_end, endof(resp), room->players[id].net.version))
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}

		struct InternalMessage r_identity = {
			.type = InternalMessageType_PlayerIdentity,
			.playerIdentity = *identity,
		};
		FOR_SOME_PLAYERS(id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {InstanceSession_connectionId(room->players, session), 0, false});
			if(pkt_serialize(&r_identity, &resp_end, endof(resp), room->players[id].net.version))
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}
	}

	struct RemoteProcedureCall base;
	base.syncTime = room_get_syncTime(room);

	FOR_SOME_PLAYERS(id, room->connected,) {
		uint8_t resp[65536], *resp_end = resp;
		pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {0, 127, false});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetRecommendedBeatmap,
			.getRecommendedBeatmap = {base},
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetRecommendedGameplayModifiers,
			.getRecommendedGameplayModifiers = {base},
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetOwnedSongPacks,
			.getOwnedSongPacks = {base},
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetIsReady,
			.getIsReady = {base},
		});
		SERIALIZE_MENURPC(&resp_end, endof(resp), room->players[id].net.version, {
			.type = MenuRpcType_GetIsInLobby,
			.getIsInLobby = {base},
		});
		instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	}
}

static bool handle_RoutingHeader(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, bool reliable, DeliveryMethod channelId) {
	struct RoutingHeader routing;
	if(room->configuration.maxPlayerCount >= 127) {
		struct BTRoutingHeader extendedRouting;
		if(!pkt_read(&extendedRouting, data, end, session->net.version))
			return true;
		routing = (struct RoutingHeader){extendedRouting.remoteConnectionId, extendedRouting.connectionId, false};
	} else if(!pkt_read(&routing, data, end, session->net.version)) {
		return true;
	}
	if(routing.connectionId) {
		struct CounterP mask = room->connected;
		CounterP_clear(&mask, indexof(room->players, session));
		if(routing.connectionId != 127) {
			struct CounterP single = COUNTER128_CLEAR;
			if(routing.connectionId / 8 < sizeof(struct CounterP))
				CounterP_set(&single, ConnectionId_index(routing.connectionId));
			mask = CounterP_and(mask, single); // Mask out invalid IDs
			if(CounterP_isEmpty(mask))
				uprintf("connectionId %hhu points to nonexistent player!\n", routing.connectionId);
		}
		FOR_SOME_PLAYERS(id, mask,) {
			uint8_t resp[65536], *resp_end = resp;
			if(!reliable)
				pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, NetPacketHeader, {PacketProperty_Unreliable, 0, 0, {{0}}});
			pkt_write_c(&resp_end, endof(resp), room->players[id].net.version, RoutingHeader, {InstanceSession_connectionId(room->players, session), routing.connectionId == 127 ? 127 : 0, routing.encrypted});
			pkt_write_bytes(*data, &resp_end, endof(resp), room->players[id].net.version, end - *data);
			if(reliable)
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, channelId);
			else
				net_send_internal(&ctx->net, &room->players[id].net, resp, resp_end - resp, 1);
		}
		return routing.connectionId != 127 || routing.encrypted;
	}
	return false;
}

static void process_message(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, bool reliable, DeliveryMethod channelId) {
	if(!session->net.alive)
		return;
	if(handle_RoutingHeader(ctx, room, session, data, end, reliable, channelId)) {
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
		struct InternalMessage message;
		if(!pkt_read(&message, &sub, *data, session->net.version)) // TODO: experiment with packet dropping and reserialization for better bandwidth usage
			return;
		/*{
			uprintf("    type=%s\n", reflect(InternalMessageType, message.type));
			if(message.type == InternalMessageType_MultiplayerSession)
				uprintf("        subtype=%s\n", reflect(MultiplayerSessionMessageType, message.multiplayerSession.type));
		}*/
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
					.pongMessage = {
						.pingTime = message.pingMessage.pingTime,
					},
				};
				struct InternalMessage r_sync = {
					.type = InternalMessageType_SyncTime,
					.syncTime.syncTime = room_get_syncTime(room),
				};
				uint8_t resp[65536], *resp_end = resp;
				pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
				if(pkt_serialize(&r_pong, &resp_end, endof(resp), session->net.version) &&
				   pkt_serialize(&r_sync, &resp_end, endof(resp), session->net.version))
					instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
				break;
			}
			case InternalMessageType_PongMessage: break;
			default: uprintf("BAD INTERNAL MESSAGE TYPE\n");
		}
		if(validateLength)
			check_length("BAD INTERNAL MESSAGE LENGTH", sub, *data, serial.length, session->net.version);
	}
}

static void process_Channeled(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId) {
	process_message(ctx, room, session, data, end, 1, channelId);
}

static void handle_ConnectRequest(struct InstanceContext *ctx, struct Room *room, struct InstanceSession *session, const struct ConnectRequest *req, const uint8_t **data, const uint8_t *end) {
	session->net.version.netVersion = req->protocolId;
	if(!(String_eq(req->secret, session->secret) && String_eq(req->userId, session->userId))) {
		*data = end;
		return;
	}
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
			uprintf("Outdated BeatUpClient version from \"%.*s\"\n", session->userName.length, session->userName.data);
			return;
		}
		if(!String_is(mod.name, "BeatUpClient beta1")) {
			uprintf("UNIDENTIFIED MOD: %.*s\n", mod.name.length, mod.name.data);
			continue;
		}
		struct ServerConnectInfo info;
		if(!pkt_read(&info, &sub, *data, session->net.version))
			continue;
		session->net.version.beatUpVersion = info.base.protocolId;
		session->net.version.windowSize = info.windowSize;
		if(session->net.version.windowSize > NET_MAX_WINDOW_SIZE)
			session->net.version.windowSize = NET_MAX_WINDOW_SIZE;
		if(session->net.version.windowSize < 32)
			session->net.version.windowSize = 32;
		session->directDownloads = info.directDownloads;
		if(indexof(room->players, session) == room->serverOwner) {
			room->shortCountdown = info.countdownDuration / 4.f;
			room->skipResults = info.skipResults;
			room->perPlayerDifficulty = info.perPlayerDifficulty;
			room->perPlayerModifiers = info.perPlayerModifiers;
		}
		check_length("BAD MOD HEADER LENGTH", sub, *data, mod.length, session->net.version);
	}
	uint8_t resp[65536], *resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, NetPacketHeader, {
		.property = PacketProperty_ConnectAccept,
		.connectionNumber = 0,
		.isFragmented = false,
		.connectAccept = {
			.connectTime = req->connectTime,
			.reusedPeer = 0,
			.connectNum = 0,
			.peerId = indexof(room->players, session),
			.beatUp = {
				.base = {
					.protocolId = session->net.version.beatUpVersion,
					.blockSize = 398,
				},
				.windowSize = session->net.version.windowSize,
				.countdownDuration = room->shortCountdown * 4,
				.directDownloads = session->directDownloads,
				.skipResults = room->skipResults,
				.perPlayerDifficulty = room->perPlayerDifficulty,
				.perPlayerModifiers = room->perPlayerModifiers,
			},
		},
	});
	if(resp_end == resp)
		return;
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
	if(session->state & ServerState_Connected)
		return;

	resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 127, false});
	struct InternalMessage r_sync = {
		.type = InternalMessageType_SyncTime,
		.syncTime.syncTime = room_get_syncTime(room),
	};
	if(pkt_serialize(&r_sync, &resp_end, endof(resp), session->net.version))
		instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	uprintf("connect[%zu]: %.*s (%.*s)\n", indexof(room->players, session), session->userName.length, session->userName.data, session->userId.length, session->userId.data);

	FOR_SOME_PLAYERS(id, room->connected,) {
		resp_end = resp;
		struct InternalMessage r_connected = {
			.type = InternalMessageType_PlayerConnected,
			.playerConnected = {
				.remoteConnectionId = InstanceSession_connectionId(room->players, &room->players[id]),
				.userId = room->players[id].userId,
				.userName = room->players[id].userName,
				.isConnectionOwner = 0,
			},
		};
		pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
		if(pkt_serialize(&r_connected, &resp_end, endof(resp), session->net.version))
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

		resp_end = resp;
		struct InternalMessage r_sort = {
			.type = InternalMessageType_PlayerSortOrderUpdate,
			.playerSortOrderUpdate = {
				.userId = room->players[id].userId,
				.sortIndex = id,
			},
		};
		pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
		if(pkt_serialize(&r_sort, &resp_end, endof(resp), session->net.version))
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

		resp_end = resp;
		struct InternalMessage r_identity = {
			.type = InternalMessageType_PlayerIdentity,
			.playerIdentity = {
				.playerState = room->players[id].stateHash,
				.playerAvatar = room->players[id].avatar,
				.random.length = 32,
				.publicEncryptionKey = room->players[id].publicEncryptionKey,
			},
		};
		memcpy(r_identity.playerIdentity.random.data, room->players[id].random, sizeof(room->players->random));
		uprintf("TODO: do we need to include the encrytion key?\n");
		pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {InstanceSession_connectionId(room->players, &room->players[id]), 0, false});
		if(pkt_serialize(&r_identity, &resp_end, endof(resp), session->net.version))
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	}

	resp_end = resp;
	pkt_write_c(&resp_end, endof(resp), session->net.version, RoutingHeader, {0, 0, false});
	struct InternalMessage r_identity = {
		.type = InternalMessageType_PlayerIdentity,
		.playerIdentity = {
			.playerState = {
				.bloomFilter = {
					.d0 = 288266110296588352,
					.d1 = 576531121051926529,
				},
			},
			.playerAvatar = CLEAR_AVATARDATA,
			.random.length = 32,
			.publicEncryptionKey.length = sizeof(r_identity.playerIdentity.publicEncryptionKey.data),
		},
	};
	memcpy(r_identity.playerIdentity.random.data, NetKeypair_get_random(&room->keys), 32);
	NetKeypair_write_key(&room->keys, &ctx->net, r_identity.playerIdentity.publicEncryptionKey.data, &r_identity.playerIdentity.publicEncryptionKey.length);
	if(pkt_serialize(&r_identity, &resp_end, endof(resp), session->net.version))
		instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	session_set_state(ctx, room, session, ServerState_Synchronizing);
}

static void log_players(const struct Room *room, struct InstanceSession *session, const char *prefix) {
	char addrstr[INET6_ADDRSTRLEN + 8], bitText[sizeof(room->playerSort) * 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	memset(bitText, '0', sizeof(bitText));
	FOR_SOME_PLAYERS(id, room->playerSort,)
		bitText[id] = '1';
	uprintf("%s %s\nplayer bits: %.*s\n", prefix, addrstr, lengthof(bitText), bitText);
}

static inline struct Room **instance_get_room(struct InstanceContext *ctx, uint16_t roomID) {
	return &ctx->rooms[0][roomID];
}

static void room_free(struct InstanceContext *ctx, struct Room **room) {
	size_t roomID = indexof(*ctx->rooms, room);
	net_keypair_free(&(*room)->keys);
	free(*room);
	*room = NULL;
	uprintf("closing room (%zu,%hu)\n", indexof(contexts, ctx), roomID);
	uint32_t group = roomID / lengthof(*ctx->rooms);
	for(uint32_t i = 0; i < lengthof(*ctx->rooms); ++i)
		if(ctx->rooms[group][i])
			return;
	Counter64_clear(&ctx->roomMask, group);
}

enum DisconnectMode {
	DC_RESET = 1,
	DC_NOTIFY = 2,
};

static void room_disconnect(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session, enum DisconnectMode mode) {
	playerid_t id = indexof((*room)->players, session);
	CounterP_clear(&(*room)->playerSort, id);
	log_players(*room, session, (mode & DC_RESET) ? "reconnect" : "disconnect");
	instance_channels_free(&session->channels);
	if(mode & DC_RESET)
		net_session_reset(&ctx->net, &session->net);
	else
		net_session_free(&session->net);

	if(id == (*room)->serverOwner) {
		(*room)->serverOwner = 0;
		uint32_t ownerOrder = ~0u;
		FOR_SOME_PLAYERS(id, (*room)->playerSort,)
			if((*room)->players[id].joinOrder < ownerOrder)
				(ownerOrder = (*room)->players[id].joinOrder, (*room)->serverOwner = id);
		if(mode & DC_NOTIFY) {
			uint8_t resp[65536], *resp_end = resp;
			struct MenuRpc r_permission = {
				.type = MenuRpcType_SetPermissionConfiguration,
				.setPermissionConfiguration = {
					.base.syncTime = room_get_syncTime(*room),
					.flags = {true, false, false, false},
					.playersPermissionConfiguration = room_get_permissions(*room),
				},
			};
			FOR_SOME_PLAYERS(id, (*room)->connected,) {
				pkt_write_c(&resp_end, endof(resp), (*room)->players[id].net.version, RoutingHeader, {0, 0, false});
				SERIALIZE_MENURPC(&resp_end, endof(resp), (*room)->players[id].net.version, r_permission);
				instance_send_channeled(&(*room)->players[id].net, &(*room)->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			}
		}
	}

	if(!CounterP_isEmpty((*room)->playerSort)) {
		if(mode & DC_NOTIFY) {
			session_set_state(ctx, *room, session, 0);
			if((*room)->state & ServerState_Lobby)
				room_set_state(ctx, *room, ServerState_Lobby_Entitlement);
		} else {
			session->state = 0;
		}
		return;
	} else if(mode & DC_RESET) {
		return;
	}

	room_free(ctx, room);
	wire_send(&ctx->net, ctx->master, &(struct WireMessage){
		.cookie = 0,
		.type = WireMessageType_WireRoomCloseNotify,
		.roomCloseNotify.room = indexof(*ctx->rooms, room),
	});
}

static inline void handle_packet(struct InstanceContext *ctx, struct Room **room, struct InstanceSession *session, const uint8_t *data, const uint8_t *end) {
	struct NetPacketHeader header;
	if(!pkt_read(&header, &data, end, session->net.version))
		return;
	struct MergedHeader merged = {
		.length = end - data,
	};
	const uint8_t *sub = data;
	if(header.property != PacketProperty_Merged)
		goto bypass;
	do {
		if(!pkt_read(&merged, &data, end, session->net.version))
			return;
		sub = data;
		if(!pkt_read(&header, &sub, &data[merged.length], session->net.version))
			return;
		bypass:;
		data += merged.length;
		if(session->state == 0 && header.property != PacketProperty_ConnectRequest)
			return;
		if(data > end) {
			uprintf("OVERFLOW\n");
			return;
		}
		if(header.isFragmented && header.property != PacketProperty_Channeled) {
			uprintf("MALFORMED HEADER\n");
			return;
		}
		switch(header.property) {
			case PacketProperty_Unreliable: process_message(ctx, *room, session, &sub, data, 0, 0); break;
			case PacketProperty_Channeled: handle_Channeled((ChanneledHandler)process_Channeled, &ctx->net, &session->net, &session->channels, ctx, *room, session, &header, &sub, data); break;
			case PacketProperty_Ack: handle_Ack(&session->net, &session->channels, &header.ack); break;
			case PacketProperty_Ping: handle_Ping(&ctx->net, &session->net, &session->tableTennis, header.ping); break;
			case PacketProperty_Pong: {
				struct InternalMessage r_latency = {
					.type = InternalMessageType_PlayerLatencyUpdate,
					.playerLatencyUpdate = {
						.latency = handle_Pong(&ctx->net, &session->net, &session->tableTennis, header.pong),
					},
				};
				if(r_latency.playerLatencyUpdate.latency != 0 && session->net.version.protocolVersion < 7) {
					FOR_EXCLUDING_PLAYER(id, (*room)->connected, indexof((*room)->players, session)) {
						uint8_t resp[65536], *resp_end = resp;
						pkt_write_c(&resp_end, endof(resp), (*room)->players[id].net.version, RoutingHeader, {
							.remoteConnectionId = InstanceSession_connectionId((*room)->players, session),
							.connectionId = 0,
							.encrypted = false,
						});
						if(pkt_serialize(&r_latency, &resp_end, endof(resp), (*room)->players[id].net.version))
							instance_send_channeled(&(*room)->players[id].net, &(*room)->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
					}
				}
				break;
			}
			case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, *room, session, &header.connectRequest, &sub, data); break;
			case PacketProperty_ConnectAccept: uprintf("BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
			case PacketProperty_Disconnect: room_disconnect(ctx, room, session, DC_NOTIFY); return;
			case PacketProperty_UnconnectedMessage: uprintf("BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
			case PacketProperty_MtuCheck: handle_MtuCheck(&ctx->net, &session->net, &header.mtuCheck); break;
			case PacketProperty_Merged: uprintf("BAD TYPE: PacketProperty_Merged\n"); break;
			default: uprintf("BAD PACKET PROPERTY\n");
		}
		check_length("BAD PACKET LENGTH", sub, data, merged.length, session->net.version);
	} while(data < end);
}

static const char *instance_masterAddress = NULL;
static void *instance_handler(struct InstanceContext *ctx) {
	net_lock(&ctx->net);
	if(*instance_masterAddress) {
		ctx->master = wire_connect_remote(&ctx->net, instance_masterAddress);
	} else if(ctx->master) {
		struct NetContext *localMaster = WireLink_cast_local(ctx->master);
		ctx->master = localMaster ? wire_connect_local(&ctx->net, localMaster) : NULL;
	}
	if(!ctx->master) {
		uprintf("wire_connect() failed\n");
		goto fail;
	}
	wire_send(&ctx->net, ctx->master, &(struct WireMessage){
		.cookie = 0,
		.type = WireMessageType_WireSetAttribs,
		.setAttribs = {
			.capacity = sizeof(ctx->rooms) / sizeof(**ctx->rooms),
			.discover = true,
		},
	});
	uprintf("Started\n");
	uint8_t pkt[1536];
	memset(pkt, 0, sizeof(pkt));
	uint32_t len;
	struct Room **room;
	struct InstanceSession *session;
	while((len = net_recv(&ctx->net, pkt, (struct NetSession**)&session, (void**)&room)))
		handle_packet(ctx, room, session, pkt, &pkt[len]);
	if(ctx->master)
		wire_disconnect(&ctx->net, ctx->master);
	fail:
	net_unlock(&ctx->net);
	return 0;
}

// TODO: clients aren't guaranteed to use the same IP address when deeplinking from the master server to instances
static struct NetSession *instance_onResolve(struct InstanceContext *ctx, struct SS addr, void **userdata_out) {
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
			if(SS_equal(&addr, NetSession_get_addr(&(*room)->players[id].net))) {
				*userdata_out = room;
				return &(*room)->players[id].net;
			}
		}
	}
	return NULL;
}

static void instance_onResend(struct InstanceContext *ctx, uint32_t currentTime, uint32_t *nextTick) {
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
			struct InstanceSession *session = &(*room)->players[id];
			uint32_t kickTime = NetSession_get_lastKeepAlive(&session->net) + IDLE_TIMEOUT_MS;
			if(currentTime > kickTime) {
				uprintf("session timeout\n");
				room_disconnect(ctx, room, session, DC_NOTIFY);
			} else {
				if(kickTime < *nextTick)
					*nextTick = kickTime;
				for(; session->channels.ru.base.sendAck; session->channels.ru.base.sendAck = 0)
					flush_ack(&ctx->net, &session->net, &session->channels.ru.base.ack);
				for(; session->channels.ro.base.sendAck; session->channels.ro.base.sendAck = 0)
					flush_ack(&ctx->net, &session->net, &session->channels.ro.base.ack);
				for(uint_fast8_t i = 0; i < 64; ++i)
					try_resend(&ctx->net, &session->net, &session->channels.ru.base.resend[i], currentTime);
				for(uint_fast8_t i = 0; i < 64; ++i)
					try_resend(&ctx->net, &session->net, &session->channels.ro.base.resend[i], currentTime);
				try_resend(&ctx->net, &session->net, &session->channels.rs.resend, currentTime);
				*nextTick = currentTime + 15; // TODO: proper resend timing
			}
		}
		if(!*room)
			continue;

		if((*room)->state & ServerState_Timeout) {
			float delta = (*room)->global.timeout - room_get_syncTime(*room);
			if(delta > 0) {
				uint32_t ms = delta * 1000;
				if(ms < 10)
					ms = 10;
				if(*nextTick - currentTime > ms)
					*nextTick = currentTime + ms;
			} else if((*room)->state & ServerState_Game_Results) { // TODO: ServerState_Lobby_Results = ServerState_Lobby_Idle >> 1
				room_set_state(ctx, *room, ServerState_Lobby_Idle);
			} else {
				room_set_state(ctx, *room, (*room)->state << 1);
			}
		}

		FOR_SOME_PLAYERS(id, (*room)->playerSort,)
			net_flush_merged(&ctx->net, &(*room)->players[id].net);
	}
}

static const char *instance_domainIPv4 = NULL, *instance_domain = NULL;
static struct IPEndPoint instance_get_endpoint(struct NetContext *net, bool ipv4) {
	struct IPEndPoint out;
	out.address.length = 0;
	out.address.isNull = false;
	out.port = 0;
	if(ipv4)
		out.address.length = sprintf(out.address.data, "%s", instance_domainIPv4);
	else
		out.address.length = sprintf(out.address.data, "%s", instance_domain);

	struct SS addr = {.len = sizeof(struct sockaddr_storage)};
	getsockname(net_get_sockfd(net), &addr.sa, &addr.len);
	switch(addr.ss.ss_family) {
		case AF_INET: out.port = htons(addr.in.sin_port); break;
		case AF_INET6: out.port = htons(addr.in6.sin6_port); break;
		default:;
	}
	return out;
}

static struct Room **room_open(struct InstanceContext *ctx, uint16_t roomID, struct GameplayServerConfiguration configuration) {
	uprintf("opening room (%zu,%hu)\n", indexof(contexts, ctx), roomID);
	if(*instance_get_room(ctx, roomID)) {
		uprintf("Room already open!\n");
		return NULL;
	}
	#ifdef FORCE_LOBBY_SIZE
	configuration.maxPlayerCount = FORCE_LOBBY_SIZE;
	configuration.songSelectionMode = SongSelectionMode_Vote;
	#endif
	if(configuration.maxPlayerCount < 1)
		configuration.maxPlayerCount = 1;
	else if((uint32_t)configuration.maxPlayerCount > sizeof(struct CounterP) * 8 - 2)
		configuration.maxPlayerCount = sizeof(struct CounterP) * 8 - 2;
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
	struct Room *room = malloc(sizeof(struct Room) + (configuration.maxPlayerCount + (configuration.songSelectionMode == SongSelectionMode_Random)) * sizeof(*room->players)); // Allocat extra slot for fake player in `SongSelectionMode_Random`
	if(!room) {
		uprintf("alloc error\n");
		return NULL;
	}
	net_keypair_init(&room->keys);
	net_keypair_gen(&ctx->net, &room->keys);
	room->serverOwner = 0;
	room->configuration = configuration;
	room->syncBase = 0;
	room->syncBase = room_get_syncTime(room);
	room->shortCountdown = 5;
	room->longCountdown = 15;
	room->skipResults = false;
	room->perPlayerDifficulty = false;
	room->perPlayerModifiers = false;
	room->joinCount = 0;
	room->connected = COUNTER128_CLEAR;
	room->playerSort = COUNTER128_CLEAR;
	room->state = 0;
	room->global.sessionId[0] = 0;
	room->global.sessionId[1] = 0;
	room->global.inLobby = COUNTER128_CLEAR;
	room->global.isSpectating = COUNTER128_CLEAR;
	room->global.selectedBeatmap = CLEAR_BEATMAP;
	room->global.selectedModifiers = CLEAR_MODIFIERS;
	room->global.roundRobin = 0;
	if(instance_mapPool) {
		room->serverOwner = room->configuration.maxPlayerCount;
		room->players[room->configuration.maxPlayerCount].userId = String_from("");
	}
	room_set_state(ctx, room, ServerState_Lobby_Idle);
	*instance_get_room(ctx, roomID) = room;
	Counter64_set(&ctx->roomMask, roomID / lengthof(*ctx->rooms));
	return instance_get_room(ctx, roomID);
}

static struct String instance_room_get_managerId(struct Room *room) {
	return room->players[room->serverOwner].userId;
}

static struct PacketContext instance_room_get_protocol(struct InstanceContext *ctx, uint16_t roomID) {
	struct PacketContext version = PV_LEGACY_DEFAULT;
	struct Room *room = *instance_get_room(ctx, roomID);
	struct CounterP ct = room->playerSort;
	uint32_t id = 0;
	if(CounterP_clear_next(&ct, &id))
		version = room->players[id].net.version;
	return version;
}

static struct WireSessionAllocResp room_resolve_session(struct InstanceContext *ctx, const struct WireSessionAlloc *req) {
	struct WireSessionAllocResp resp = {
		.result = ConnectToServerResponse_Result_UnknownError,
	};
	struct Room *room = *instance_get_room(ctx, req->room);
	if(!room)
		return resp;
	struct SS addr = {.len = req->address.length};
	memcpy(&addr.ss, req->address.data, req->address.length);
	struct InstanceSession *session = NULL;
	FOR_SOME_PLAYERS(id, room->playerSort,) {
		if(SS_equal(&addr, NetSession_get_addr(&room->players[id].net))) {
			session = &room->players[id];
			room_disconnect(ctx, &room, session, DC_RESET | DC_NOTIFY);
			break;
		}
	}
	if(!session) {
		struct CounterP tmp = room->playerSort;
		uint32_t id = 0;
		if((!CounterP_set_next(&tmp, &id)) || (int32_t)id >= room->configuration.maxPlayerCount) {
			uprintf("ROOM FULL\n");
			return resp;
		}
		session = &room->players[id];
		net_session_init(&ctx->net, &session->net, addr);
		session->net.version = req->version;
		room->playerSort = tmp;
	}
	session->secret = req->secret;
	session->userName = req->userName;
	session->userId = req->userId;
	session->publicEncryptionKey.length = 0;
	session->sentIdentity = false;
	session->directDownloads = false;
	session->joinOrder = ++room->joinCount;
	session->state = 0;
	session->recommendTime = 0;
	session->recommendedBeatmap = CLEAR_BEATMAP;
	session->recommendedModifiers = CLEAR_MODIFIERS;

	instance_pingpong_init(&session->tableTennis);
	instance_channels_init(&session->channels);
	session->stateHash.bloomFilter = (struct BitMask128){0, 0};
	session->avatar = CLEAR_AVATARDATA;

	memcpy(session->net.clientRandom, req->random, 32);
	memcpy(resp.random, NetKeypair_get_random(&session->net.keys), 32);
	resp.publicKey.length = sizeof(resp.publicKey.data);
	if(NetKeypair_write_key(&session->net.keys, &ctx->net, resp.publicKey.data, &resp.publicKey.length)) {
		uprintf("Connect to Server Error: NetKeypair_write_key() failed\n");
		return resp; // TODO: clean up and remove session data if either of these errors is hit
	}
	if(NetSession_set_clientPublicKey(&session->net, &ctx->net, &req->publicKey)) {
		uprintf("Connect to Server Error: NetSession_set_clientPublicKey() failed\n");
		return resp;
	}
	resp.configuration = room->configuration;
	resp.managerId = instance_room_get_managerId(room);
	bool ipv4 = ((struct sockaddr_storage*)req->address.data)->ss_family != AF_INET6 ||
	            memcmp(((struct sockaddr_in6*)req->address.data)->sin6_addr.s6_addr, (const uint8_t[]){0,0,0,0,0,0,0,0,0,0,255,255}, 12) == 0;
	resp.endPoint = instance_get_endpoint(&ctx->net, ipv4);
	resp.result = ConnectToServerResponse_Result_Success;
	log_players(room, session, "connect");
	return resp;
}

static void instance_room_spawn(struct InstanceContext *ctx, union WireLink *link, uint32_t cookie, const struct WireRoomSpawn *req) {
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
	wire_send(&ctx->net, link, &r_alloc);
}

static void instance_room_join(struct InstanceContext *ctx, union WireLink *link, uint32_t cookie, const struct WireRoomJoin *req) {
	struct WireMessage r_alloc = {
		.type = WireMessageType_WireRoomJoinResp,
		.cookie = cookie,
	};
	if(instance_room_get_protocol(ctx, req->base.room).protocolVersion != req->base.version.protocolVersion) {
		uprintf("Connect to Server Error: Version mismatch\n");
		r_alloc.roomJoinResp.base.result = ConnectToServerResponse_Result_VersionMismatch;
	} else {
		r_alloc.roomJoinResp.base = room_resolve_session(ctx, &req->base);
	}
	wire_send(&ctx->net, link, &r_alloc);
}

static void instance_onWireMessage(struct InstanceContext *ctx, union WireLink *link, const struct WireMessage *message) {
	if(link != ctx->master)
		return;
	if(!message) {
		ctx->master = NULL;
		return;
	}
	switch(message->type) {
		case WireMessageType_WireRoomSpawn: instance_room_spawn(ctx, link, message->cookie, &message->roomSpawn); break;
		case WireMessageType_WireRoomJoin: instance_room_join(ctx, link, message->cookie, &message->roomJoin); break;
		default: uprintf("UNHANDLED WIRE MESSAGE [%s]\n", reflect(WireMessageType, message->type));
	}
}

static uint32_t threads_len = 0;
static pthread_t *threads = NULL;
bool instance_init(const char *domainIPv4, const char *domain, const char *remoteMaster, struct NetContext *localMaster, const char *mapPoolFile, uint32_t count) {
	if(mapPoolFile && *mapPoolFile)
		mapPool_init(mapPoolFile);
	instance_domainIPv4 = domainIPv4;
	instance_domain = domain;
	instance_masterAddress = remoteMaster;
	threads_len = 0;
	contexts = malloc(count * sizeof(*contexts));
	threads = malloc(count * sizeof(*threads));
	if(!contexts || !threads) {
		uprintf("alloc error\n");
		return true;
	}
	for(; threads_len < count; ++threads_len) {
		struct InstanceContext *ctx = &contexts[threads_len];
		if(net_init(&ctx->net, 5000 + threads_len, true)) {
			uprintf("net_init() failed\n");
			return true;
		}
		ctx->net.userptr = &contexts[threads_len];
		ctx->net.onResolve = (struct NetSession *(*)(void*, struct SS, void**))instance_onResolve;
		ctx->net.onResend = (void (*)(void*, uint32_t, uint32_t*))instance_onResend;
		ctx->net.onWireMessage = (void (*)(void*, union WireLink*, const struct WireMessage*))instance_onWireMessage;
		ctx->roomMask = COUNTER64_CLEAR;
		ctx->master = (union WireLink*)localMaster;
		memset(ctx->rooms, 0, sizeof(ctx->rooms));

		if(pthread_create(&threads[threads_len], NULL, (void *(*)(void*))instance_handler, ctx))
			threads[threads_len] = 0;
		if(!threads[threads_len]) {
			net_cleanup(&ctx->net);
			uprintf("Instance thread creation failed\n");
			return true;
		}
	}
	return false;
}

void instance_cleanup() {
	for(uint32_t i = 0; i < threads_len; ++i) {
		if(threads[i]) {
			struct InstanceContext *ctx = &contexts[i];
			net_stop(&ctx->net);
			uprintf("Stopping #%u\n", i);
			pthread_join(threads[i], NULL);
			threads[i] = 0;
			FOR_ALL_ROOMS(ctx, room) {
				FOR_SOME_PLAYERS(id, (*room)->playerSort,) {
					room_disconnect(ctx, room, &(*room)->players[id], 0);
				}
			}
			ctx->roomMask = COUNTER64_CLEAR; // should be redundant, but just to be safe
			memset(ctx->rooms, 0, sizeof(ctx->rooms));
			net_cleanup(&ctx->net);
		}
	}
	free(instance_mapPool);
	free(threads);
	free(contexts);
	instance_mapPool = NULL;
}
