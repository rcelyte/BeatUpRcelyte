// #define THREAD_COUNT 256
#define THREAD_COUNT 1

#include "../enum_reflection.h"
#include "instance.h"
#include "common.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include "../debug.h"
#include "../pool.h"
#include <mbedtls/error.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define NOT_IMPLEMENTED(type) case type: uprintf(#type " not implemented\n"); abort();

#define COUNTER_VAR CONCAT(_i_,__LINE__)

#define FOR_SOME_PLAYERS(room, id, counter, ...) \
	struct Counter128 COUNTER_VAR = (counter); __VA_ARGS__; for(uint32_t (id) = 0; Counter128_set_next(&COUNTER_VAR, &id, 0); ++id)

#define FOR_EXCLUDING_PLAYER(room, session, id, counter) \
	FOR_SOME_PLAYERS(room, id, counter, Counter128_set(&COUNTER_VAR, indexof((room)->players, session), 0))

#define FOR_ALL_ROOMS(ctx, room) \
	struct Counter16 COUNTER_VAR = (ctx)->blockAlloc; for(uint8_t group; Counter16_set_next(&COUNTER_VAR, &group, 0);) \
		for(struct Room **(room) = (ctx)->rooms[group]; (room) < &(ctx)->rooms[group][lengthof(*(ctx)->rooms)]; ++(room)) \
			if(*room)

struct Counter16 {
	uint16_t bits;
};

struct Counter128 {
	uint32_t bits[4];
};

#define COUNTER16_CLEAR (struct Counter16){0};
#define COUNTER128_CLEAR (struct Counter128){{0, 0, 0, 0}};

_Bool Counter16_set(struct Counter16 *set, uint8_t bit, _Bool state) {
	_Bool prev = (set->bits >> bit) & 1;
	if(state)
		set->bits |= 1 << bit;
	else
		set->bits &= ~(1 << bit);
	return prev;
}

_Bool Counter128_get(struct Counter128 set, uint32_t bit) {
	return (set.bits[bit / 32] >> (bit % 32)) & 1;
}

_Bool Counter128_set(struct Counter128 *set, uint32_t bit, _Bool state) {
	uint32_t i = bit / 32;
	bit %= 32;
	_Bool prev = (set->bits[i] >> bit) & 1;
	if(state)
		set->bits[i] |= 1 << bit;
	else
		set->bits[i] &= ~(1 << bit);
	return prev;
}

_Bool Counter16_set_next(struct Counter16 *set, uint8_t *bit, _Bool state) {
	uint16_t v = state ? ~set->bits : set->bits;
	if(v == 0)
		return 0;
	*bit = __builtin_ctz(v);
	if(state)
		set->bits |= set->bits + 1;
	else
		set->bits &= set->bits - 1;
	return 1;
}

uint32_t Counter128_get_next(struct Counter128 set, uint32_t bit) {
	uint32_t i = bit / 32;
	uint32_t first = set.bits[i] & (~0u >> (bit % 32));
	if(first)
		return i * 32 + __builtin_ctz(first);
	set.bits[i] &= (~0u << (32 - (bit % 32)));
	do {
		i = (i + 1) % lengthof(set.bits);
		if(set.bits[i])
			return i * 32 + __builtin_ctz(set.bits[i]);
	} while(i != bit / 32);
	return 0;
}

_Bool Counter128_set_next_0(struct Counter128 *set, uint32_t *bit) {
	for(uint32_t i = *bit / 32; i < lengthof(set->bits); ++i) {
		if(set->bits[i]) {
			*bit = i * 32 + __builtin_ctz(set->bits[i]);
			set->bits[i] &= set->bits[i] - 1;
			return 1;
		}
	}
	return 0;
}

_Bool Counter128_set_next_1(struct Counter128 *set, uint32_t *bit) {
	for(uint32_t i = *bit / 32; i < lengthof(set->bits); ++i) {
		if(~set->bits[i]) {
			*bit = i * 32 + __builtin_ctz(~set->bits[i]);
			set->bits[i] |= set->bits[i] + 1;
			return 1;
		}
	}
	return 0;
}

#define Counter128_set_next(set, bit, state) Counter128_set_next_##state(set, bit)

_Bool Counter128_eq(struct Counter128 a, struct Counter128 b) {
	for(uint32_t i = 0; i < lengthof(a.bits); ++i)
		if(a.bits[i] != b.bits[i])
			return 0;
	return 1;
}

_Bool Counter128_contains(struct Counter128 set, struct Counter128 subset) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if((set.bits[i] & subset.bits[i]) != subset.bits[i])
			return 0;
	return 1;
}

_Bool Counter128_containsNone(struct Counter128 set, struct Counter128 subset) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if(set.bits[i] & subset.bits[i])
			return 0;
	return 1;
}

_Bool Counter16_isEmpty(struct Counter16 set) {
	return set.bits == 0;
}

_Bool Counter128_isEmpty(struct Counter128 set) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if(set.bits[i])
			return 0;
	return 1;
}

struct Counter128 Counter128_or(struct Counter128 a, struct Counter128 b) {
	for(uint32_t i = 0; i < lengthof(a.bits); ++i)
		a.bits[i] |= b.bits[i];
	return a;
}

struct InstanceSession {
	struct NetSession net;
	struct String secret;
	struct ExString userName;
	struct PlayerLobbyPermissionConfigurationNetSerializable permissions;
	struct PingPong tableTennis;
	struct Channels channels;
	struct PlayerStateHash stateHash;
	struct MultiplayerAvatarData avatar;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicEncryptionKey;
	_Bool sentIdentity, directDownloads;

	ServerState state;
	struct BeatmapIdentifierNetSerializable recommendedBeatmap;
	struct GameplayModifiers recommendedModifiers;
	struct PlayerSpecificSettingsNetSerializable settings;
};
struct Room {
	struct NetKeypair keys;
	struct String managerId;
	struct GameplayServerConfiguration configuration;
	float syncBase, countdownDuration;
	_Bool skipResults, perPlayerDifficulty, perPlayerModifiers;

	ServerState state;
	struct {
		struct Counter128 inLobby;
		struct Counter128 isSpectating;
		struct BeatmapIdentifierNetSerializable selectedBeatmap;
		struct GameplayModifiers selectedModifiers;
		uint32_t roundRobin;
		float timeout;
	} global;

	union {
		struct {
			struct Counter128 isEntitled;
			struct Counter128 isDownloaded;
			struct Counter128 isReady;
			CannotStartGameReason reason;
			struct {
				struct Counter128 missing;
				struct Counter128 deferred;
				struct Counter128 canShare;
				_Bool shareSet;
				uint8_t shareHash[32];
				uint64_t shareSize;
			} entitlement;
		} lobby;
		struct {
			struct Counter128 levelFinished;
			float startTime;
			_Bool showResults;
			union {
				struct {
					struct Counter128 isLoaded;
				} loadingScene;
				struct {
					struct Counter128 isLoaded;
				} loadingSong;
			};
		} game;
	};

	struct Counter128 connected;
	struct Counter128 playerSort;
	struct InstanceSession players[];
};

struct Context {
	struct NetContext net;
	struct Counter16 blockAlloc;
	struct Room *rooms[16][16];
	uint16_t notifyHandle[16];
} static contexts[THREAD_COUNT];

static float room_get_syncTime(struct Room *room) {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec + (now.tv_nsec / 1000) / 1000000.f - room->syncBase;
}

static _Bool PlayerStateHash_contains(struct PlayerStateHash state, const char *key) {
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
			hash ^= key[num3 + 2] << 16;
		case 2:
			hash ^= key[num3 + 1] << 8;
		case 1:
			hash ^= key[num3];
			hash *= 1540483477;
		case 0:
			break;
	}
	hash ^= hash >> 13;
	hash *= 1540483477;
	hash ^= (hash >> 15);
	for(uint_fast8_t i = 0; i < 3; ++i) {
		uint_fast8_t ind = (hash % 128);
		if(!(((ind >= 64) ? state.bloomFilter.d0 >> (ind - 64) : state.bloomFilter.d1 >> ind) & 1))
			return 0;
		hash >>= 8;
	}
	return 1;
}

static _Bool BeatmapIdentifierNetSerializable_eq(const struct BeatmapIdentifierNetSerializable *a, const struct BeatmapIdentifierNetSerializable *b, _Bool ignoreDifficulty) {
	if(!String_eq(a->levelID, b->levelID))
		return 0;
	return ignoreDifficulty || (String_eq(a->beatmapCharacteristicSerializedName, b->beatmapCharacteristicSerializedName) && a->difficulty == b->difficulty);
}

static _Bool GameplayModifiers_eq(const struct GameplayModifiers *a, const struct GameplayModifiers *b, _Bool speedOnly) {
	if(a->songSpeed != b->songSpeed)
		return 0;
	return speedOnly || (a->energyType == b->energyType && a->demoNoFail == b->demoNoFail && a->instaFail == b->instaFail && a->failOnSaberClash == b->failOnSaberClash && a->enabledObstacleType == b->enabledObstacleType && a->demoNoObstacles == b->demoNoObstacles && a->noBombs == b->noBombs && a->fastNotes == b->fastNotes && a->strictAngles == b->strictAngles && a->disappearingArrows == b->disappearingArrows && a->ghostNotes == b->ghostNotes && a->noArrows == b->noArrows && a->noFailOn0Energy == b->noFailOn0Energy && a->proMode == b->proMode && a->zenMode == b->zenMode && a->smallCubes == b->smallCubes);
}

static struct BeatmapIdentifierNetSerializable session_get_beatmap(const struct Room *room, const struct InstanceSession *session) {
	if(room->perPlayerDifficulty && BeatmapIdentifierNetSerializable_eq(&session->recommendedBeatmap, &room->global.selectedBeatmap, 1))
		return session->recommendedBeatmap;
	return room->global.selectedBeatmap;
}

static struct GameplayModifiers session_get_modifiers(const struct Room *room, const struct InstanceSession *session) {
	if(room->perPlayerModifiers && GameplayModifiers_eq(&session->recommendedModifiers, &room->global.selectedModifiers, 1))
		return session->recommendedModifiers;
	return room->global.selectedModifiers;
}

#define STATE_EDGE(from, to, mask) ((to & (mask)) && !(from & (mask)))
static void room_try_finish(struct Context *ctx, struct Room *room);
static void session_set_state(struct Context *ctx, struct Room *room, struct InstanceSession *session, ServerState state) {
	struct RemoteProcedureCall base = {
		.syncTime = room_get_syncTime(room),
	};
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
	uint8_t *start = resp_end;
	if(STATE_EDGE(session->state, state, ServerState_Connected)) {
		Counter128_set(&room->connected, indexof(room->players, session), 1);
	} else if(STATE_EDGE(state, session->state, ServerState_Connected)) {
		Counter128_set(&room->connected, indexof(room->players, session), 0);
		if(room->global.roundRobin == indexof(room->players, session))
			room->global.roundRobin = Counter128_get_next(room->connected, room->global.roundRobin);
		struct PlayerDisconnected r_disconnect = {DisconnectedReason_ClientConnectionClosed};
		FOR_SOME_PLAYERS(room, id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){indexof(room->players, session) + 1, 0, 0});
			SERIALIZE_CUSTOM(room->players[id].net.version, &resp_end, InternalMessageType_PlayerDisconnected)
				pkt_writePlayerDisconnected(room->players[id].net.version, &resp_end, r_disconnect);
			instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}
		if(room->state & ServerState_Game)
			room_try_finish(ctx, room);
	}
	if(state & ServerState_Lobby) {
		_Bool needSetSelectedBeatmap = (state & ServerState_Lobby_Entitlement) != 0;
		if(!(session->state & ServerState_Lobby)) {
			needSetSelectedBeatmap = 1;
			session->recommendedBeatmap = CLEAR_BEATMAP;
			SERIALIZE_GAMEPLAYRPC(session->net.version, &resp_end, ReturnToMenu, (struct ReturnToMenu){base});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetMultiplayerGameState, (struct SetMultiplayerGameState){
				.base = base,
				.flags = {1, 0, 0, 0},
				.lobbyState = MultiplayerGameState_Lobby,
			});
		} else if(STATE_EDGE(state, session->state, ServerState_Lobby_Countdown | ServerState_Lobby_Downloading)) {
			SERIALIZE_MENURPC(session->net.version, &resp_end, CancelCountdown, (struct CancelCountdown){base});
			SERIALIZE_MENURPC(session->net.version, &resp_end, CancelLevelStart, (struct CancelLevelStart){base});
		}
		if(needSetSelectedBeatmap) {
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetSelectedBeatmap, (struct SetSelectedBeatmap){
				.base = base,
				.flags = {1, 0, 0, 0},
				.identifier = session_get_beatmap(room, session),
			});
		}
		if(STATE_EDGE(session->state, state, ServerState_Lobby_Countdown | ServerState_Lobby_Downloading)) {
			SERIALIZE_MENURPC(session->net.version, &resp_end, StartLevel, (struct StartLevel){
				.base = base,
				.flags = {1, 1, 1, 0},
				.beatmapId = session_get_beatmap(room, session),
				.gameplayModifiers = session_get_modifiers(room, session),
				.startTime = room->global.timeout + 1048576,
			});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetCountdownEndTime, (struct SetCountdownEndTime){
				.base = base,
				.flags = {1, 0, 0, 0},
				.newTime = room->global.timeout,
			});
		}
	} else if(STATE_EDGE(session->state, state, ServerState_Game)) {
		SERIALIZE_MENURPC(session->net.version, &resp_end, SetMultiplayerGameState, (struct SetMultiplayerGameState){
			.base = base,
			.flags = {1, 0, 0, 0},
			.lobbyState = MultiplayerGameState_Game,
		});
	}
	switch(state) {
		case ServerState_Lobby_Entitlement: {
			SERIALIZE_MENURPC(session->net.version, &resp_end, GetIsEntitledToLevel, (struct GetIsEntitledToLevel){
				.base = base,
				.flags = {1, 0, 0, 0},
				.levelId = room->global.selectedBeatmap.levelID,
			});
			break;
		}
		case ServerState_Lobby_Idle:
		case ServerState_Lobby_Ready: {
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetIsStartButtonEnabled, (struct SetIsStartButtonEnabled){
				.base = base,
				.flags = {1, 0, 0, 0},
				.reason = room->lobby.reason,
			});
			break;
		}
		case ServerState_Lobby_Countdown: break;
		case ServerState_Lobby_Downloading: break;
		case ServerState_Game_LoadingScene: {
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetStartGameTime, (struct SetStartGameTime){
				.base = base,
				.flags = {1, 0, 0, 0},
				.newTime = base.syncTime,
			});
			SERIALIZE_GAMEPLAYRPC(session->net.version, &resp_end, GetGameplaySceneReady, (struct GetGameplaySceneReady){base});
			break;
		}
		case ServerState_Game_LoadingSong: {
			struct SetGameplaySceneSyncFinish r_sync;
			r_sync.base = base;
			r_sync.flags = (struct RemoteProcedureCallFlags){1, 1, 0, 0};
			r_sync.playersAtGameStart.count = 0;
			FOR_SOME_PLAYERS(room, id, room->connected,)
				r_sync.playersAtGameStart.activePlayerSpecificSettingsAtGameStart[r_sync.playersAtGameStart.count++] = room->players[id].settings;
			r_sync.sessionGameId.isNull = 0;
			r_sync.sessionGameId.length = sprintf(r_sync.sessionGameId.data, "00000000-0000-0000-0000-000000000000");
			SERIALIZE_GAMEPLAYRPC(session->net.version, &resp_end, GetGameplaySongReady, (struct GetGameplaySongReady){base});
			SERIALIZE_GAMEPLAYRPC(session->net.version, &resp_end, SetGameplaySceneSyncFinish, r_sync);
			break;
		}
		case ServerState_Game_Gameplay: {
			SERIALIZE_GAMEPLAYRPC(session->net.version, &resp_end, SetSongStartTime, (struct SetSongStartTime){
				.base = base,
				.flags = {1, 0, 0, 0},
				.startTime = room->game.startTime,
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
		case ServerState_Lobby_Countdown: return "Lobby.Countdown";
		case ServerState_Lobby_Downloading: return "Lobby.Downloading";
		case ServerState_Game_LoadingScene: return "Game.LoadingScene";
		case ServerState_Game_LoadingSong: return "Game.LoadingSong";
		case ServerState_Game_Gameplay: return "Game.Gameplay";
		case ServerState_Game_Results: return "Game.Results";
		default:;
	}
	return "???";
}

static void room_set_state(struct Context *ctx, struct Room *room, ServerState state) {
	uprintf("state %s -> %s\n", ServerState_toString(room->state), ServerState_toString(state));
	if(STATE_EDGE(room->state, state, ServerState_Lobby)) {
		room->global.selectedBeatmap = CLEAR_BEATMAP;
		room->lobby.isEntitled = COUNTER128_CLEAR;
		room->lobby.isDownloaded = COUNTER128_CLEAR;
		room->lobby.isReady = COUNTER128_CLEAR;
		room->lobby.reason = CannotStartGameReason_NoSongSelected;
	} else if(STATE_EDGE(room->state, state, ServerState_Game)) {
		room->game.levelFinished = COUNTER128_CLEAR;
		room->game.showResults = 0;
	}
	switch(state) {
		case ServerState_Lobby_Entitlement: {
			uint32_t select = ~0u;
			switch(room->configuration.songSelectionMode) {
				case SongSelectionMode_Vote: vote_beatmap: {
					uint8_t votes[126], max = 0;
					struct Counter128 mask = COUNTER128_CLEAR;
					FOR_SOME_PLAYERS(room, id, room->connected,) {
						votes[id] = 0;
						if(!(room->players[id].permissions.hasRecommendBeatmapsPermission && room->players[id].recommendedBeatmap.beatmapCharacteristicSerializedName.length))
							continue;
						Counter128_set(&mask, id, 1);
						FOR_SOME_PLAYERS(room, cmp, mask,) {
							if(id == cmp || BeatmapIdentifierNetSerializable_eq(&room->players[id].recommendedBeatmap, &room->players[cmp].recommendedBeatmap, room->perPlayerDifficulty)) {
								if(++votes[cmp] > max) {
									select = cmp;
									max = votes[cmp];
								}
								break;
							}
						}
					}
					break;
				}
				NOT_IMPLEMENTED(SongSelectionMode_Random);
				case SongSelectionMode_OwnerPicks: {
					FOR_SOME_PLAYERS(room, id, room->connected,) {
						if(room->players[id].permissions.isServerOwner) {
							select = id;
							break;
						}
					}
					if(select == (uint32_t)~0u)
						goto vote_beatmap;
					break;
				}
				case SongSelectionMode_RandomPlayerPicks: {
					if(room->players[room->global.roundRobin].recommendedBeatmap.beatmapCharacteristicSerializedName.length)
						select = room->global.roundRobin;
					break;
				}
			}
			if(select == (uint32_t)~0u || room->players[select].recommendedBeatmap.beatmapCharacteristicSerializedName.length == 0) {
				room->lobby.isEntitled = room->connected;
				room->global.selectedBeatmap = CLEAR_BEATMAP;
				room_set_state(ctx, room, ServerState_Lobby_Idle);
				return;
			} else if(Counter128_eq(room->lobby.isEntitled, room->connected) && BeatmapIdentifierNetSerializable_eq(&room->players[select].recommendedBeatmap, &room->global.selectedBeatmap, 0)) {
				return;
			} else {
				room->lobby.isEntitled = COUNTER128_CLEAR;
				room->lobby.isDownloaded = COUNTER128_CLEAR;
				room->global.selectedBeatmap = room->players[select].recommendedBeatmap;
				room->lobby.entitlement.missing = COUNTER128_CLEAR;
				room->lobby.entitlement.deferred = COUNTER128_CLEAR;
				room->lobby.entitlement.canShare = COUNTER128_CLEAR;
				room->lobby.entitlement.shareSet = 0;
				room->lobby.entitlement.shareSize = 0;
			}
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
			if(Counter128_containsNone(room->global.inLobby, room->connected)) {
				room->lobby.reason = CannotStartGameReason_AllPlayersNotInLobby;
				break;
			} else if(Counter128_contains(room->global.isSpectating, room->connected)) {
				room->lobby.reason = CannotStartGameReason_AllPlayersSpectating;
				break;
			} else if(!Counter128_contains(Counter128_or(room->lobby.isReady, room->global.isSpectating), room->connected)) {
				break;
			} else if(room->state & (ServerState_Lobby_Countdown | ServerState_Lobby_Downloading)) {
				return;
			}
			FOR_SOME_PLAYERS(room, id, room->connected,)
				session_set_state(ctx, room, &room->players[id], state);
			room_set_state(ctx, room, ServerState_Lobby_Countdown);
			return;
		}
		case ServerState_Lobby_Countdown: room->global.timeout = room_get_syncTime(room) + room->countdownDuration; break;
		case ServerState_Lobby_Downloading: {
			if(Counter128_contains(room->lobby.isDownloaded, room->connected)) {
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
			room->game.loadingSong.isLoaded = COUNTER128_CLEAR;
			room->global.timeout = room_get_syncTime(room) + LOAD_TIMEOUT;
			break;
		}
		case ServerState_Game_Gameplay: room->game.startTime = room_get_syncTime(room) + .25; break;
		case ServerState_Game_Results: room->global.timeout = room_get_syncTime(room) + 20; break;
	}
	room->state = state;
	FOR_SOME_PLAYERS(room, id, room->connected,)
		session_set_state(ctx, room, &room->players[id], state);
}

static void handle_MenuRpc(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(session->net.version, data);
	switch(rpc.type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: uprintf("BAD TYPE: MenuRpcType_SetPlayersMissingEntitlementsToLevel\n");
		case MenuRpcType_GetIsEntitledToLevel: uprintf("BAD TYPE: MenuRpcType_GetIsEntitledToLevel\n");
		case MenuRpcType_SetIsEntitledToLevel: {
			struct SetIsEntitledToLevel entitlement = pkt_readSetIsEntitledToLevel(session->net.version, data);
			if(!((room->state & ServerState_Lobby) && entitlement.flags.hasValue0 && String_eq(entitlement.levelId, room->global.selectedBeatmap.levelID)))
				break;
			if(!entitlement.flags.hasValue1)
				entitlement.entitlementStatus = EntitlementsStatus_Unknown;
			if(entitlement.entitlementStatus == EntitlementsStatus_Ok)
				if(Counter128_set(&room->lobby.isDownloaded, indexof(room->players, session), 1) == 0)
					if((room->state & ServerState_Lobby_Downloading) && Counter128_contains(room->lobby.isDownloaded, room->connected))
						room_set_state(ctx, room, ServerState_Game_LoadingScene);
			if(!(room->state & ServerState_Lobby_Entitlement))
				break;
			if(Counter128_set(&room->lobby.isEntitled, indexof(room->players, session), 1))
				break;
			if(entitlement.entitlementStatus == EntitlementsStatus_Unknown && session->directDownloads)
				Counter128_set(&room->lobby.entitlement.deferred, indexof(room->players, session), 1);
			else if(entitlement.entitlementStatus != EntitlementsStatus_Ok && entitlement.entitlementStatus != EntitlementsStatus_NotDownloaded)
				Counter128_set(&room->lobby.entitlement.missing, indexof(room->players, session), 1);
			uprintf("entitlement[%.*s]: %s\n", session->userName.base.length, session->userName.base.data, reflect(EntitlementsStatus, entitlement.entitlementStatus));
			if(!Counter128_contains(room->lobby.isEntitled, room->connected))
				break;
			if(Counter128_isEmpty(room->lobby.entitlement.canShare))
				room->lobby.entitlement.missing = Counter128_or(room->lobby.entitlement.missing, room->lobby.entitlement.deferred);
			struct DirectDownloadInfo r_download;
			r_download.base.levelId = room->global.selectedBeatmap.levelID;
			memcpy(r_download.base.levelHash, room->lobby.entitlement.shareHash, sizeof(r_download.base.levelHash));
			r_download.base.fileSize = room->lobby.entitlement.shareSize;
			r_download.count = 0;
			struct SetPlayersMissingEntitlementsToLevel r_missing;
			r_missing.base.syncTime = room_get_syncTime(room);
			r_missing.flags = (struct RemoteProcedureCallFlags){1, 0, 0, 0};
			r_missing.playersMissingEntitlements.count = 0;
			FOR_SOME_PLAYERS(room, id, room->lobby.entitlement.missing,)
				r_missing.playersMissingEntitlements.playersWithoutEntitlements[r_missing.playersMissingEntitlements.count++] = room->players[id].permissions.userId;
			if(r_missing.playersMissingEntitlements.count == 0) {
				FOR_SOME_PLAYERS(room, id, room->lobby.entitlement.canShare,)
					r_download.sourcePlayers[r_download.count++] = room->players[id].permissions.userId;
			}
			FOR_SOME_PLAYERS(room, id, room->connected,) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){0, 127, 0});
				if(room->players[id].directDownloads && r_download.count)
					SERIALIZE_BEATUP(room->players[id].net.version, &resp_end, DirectDownloadInfo, r_download);
				SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, SetPlayersMissingEntitlementsToLevel, r_missing);
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			}
			if(r_missing.playersMissingEntitlements.count == 0)
				room_set_state(ctx, room, ServerState_Lobby_Ready);
			else
				room_set_state(ctx, room, ServerState_Lobby_Idle);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_InvalidateLevelEntitlementStatuses);
		NOT_IMPLEMENTED(MenuRpcType_SelectLevelPack);
		case MenuRpcType_SetSelectedBeatmap: uprintf("BAD TYPE: MenuRpcType_SetSelectedBeatmap\n"); break;
		case MenuRpcType_GetSelectedBeatmap: {
			pkt_readGetSelectedBeatmap(session->net.version, data);
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetSelectedBeatmap, (struct SetSelectedBeatmap){
				.base = base,
				.flags = {1, 0, 0, 0},
				.identifier = session_get_beatmap(room, session),
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_RecommendBeatmap: {
			struct RecommendBeatmap beatmap = pkt_readRecommendBeatmap(session->net.version, data);
			if(!beatmap.flags.hasValue0) {
				if(0) {
					case MenuRpcType_ClearRecommendedBeatmap:
					pkt_readClearRecommendedBeatmap(session->net.version, data);
				}
				beatmap.identifier = CLEAR_BEATMAP;
			}
			if(!((room->state & ServerState_Lobby) && session->permissions.hasRecommendBeatmapsPermission))
				break;
			session->recommendedBeatmap = beatmap.identifier;
			room_set_state(ctx, room, ServerState_Lobby_Entitlement);
			break;
		}
		case MenuRpcType_GetRecommendedBeatmap: pkt_readGetRecommendedBeatmap(session->net.version, data); break;
		case MenuRpcType_SetSelectedGameplayModifiers: uprintf("BAD TYPE: MenuRpcType_SetSelectedGameplayModifiers\n"); break;
		case MenuRpcType_GetSelectedGameplayModifiers: {
			pkt_readGetSelectedGameplayModifiers(session->net.version, data);
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetSelectedGameplayModifiers, (struct SetSelectedGameplayModifiers){
				.base = base,
				.flags = {1, 0, 0, 0},
				.gameplayModifiers = session_get_modifiers(room, session),
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_RecommendGameplayModifiers: {
			struct RecommendGameplayModifiers modifiers = pkt_readRecommendGameplayModifiers(session->net.version, data);
			if(!modifiers.flags.hasValue0) {
				if(0) {
					case MenuRpcType_ClearRecommendedGameplayModifiers:
					pkt_readClearRecommendedGameplayModifiers(session->net.version, data);
				}
				modifiers.gameplayModifiers = CLEAR_MODIFIERS;
			}
			if(!((room->state & ServerState_Lobby) && session->permissions.hasRecommendGameplayModifiersPermission))
				break;
			session->recommendedModifiers = modifiers.gameplayModifiers;
			uint32_t select = ~0u;
			switch(room->configuration.songSelectionMode) {
				case SongSelectionMode_Vote: vote_modifiers: {
					uint8_t votes[126], max = 0;
					struct Counter128 mask = COUNTER128_CLEAR;
					FOR_SOME_PLAYERS(room, id, room->connected,) {
						votes[id] = 0;
						if(!room->players[id].permissions.hasRecommendGameplayModifiersPermission)
							continue;
						Counter128_set(&mask, id, 1);
						FOR_SOME_PLAYERS(room, cmp, mask,) {
							if(id == cmp || GameplayModifiers_eq(&room->players[id].recommendedModifiers, &room->players[cmp].recommendedModifiers, room->perPlayerModifiers)) {
								if(++votes[cmp] > max) {
									select = cmp;
									max = votes[cmp];
								}
								break;
							}
						}
					}
					break;
				}
				NOT_IMPLEMENTED(SongSelectionMode_Random);
				case SongSelectionMode_OwnerPicks: {
					FOR_SOME_PLAYERS(room, id, room->connected,) {
						if(room->players[id].permissions.isServerOwner) {
							select = id;
							break;
						}
					}
					if(select == (uint32_t)~0u)
						goto vote_modifiers;
					break;
				}
				case SongSelectionMode_RandomPlayerPicks: {
					if(!room->players[room->global.roundRobin].permissions.hasRecommendGameplayModifiersPermission)
						goto vote_modifiers;
					select = room->global.roundRobin;
					break;
				}
			}
			if(select == (uint32_t)~0u || GameplayModifiers_eq(&room->players[select].recommendedModifiers, &room->global.selectedModifiers, 0))
				break;
			room->global.selectedModifiers = room->players[select].recommendedModifiers;
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			FOR_SOME_PLAYERS(room, id, room->connected,) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){0, 127, 0});
				SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, SetSelectedGameplayModifiers, (struct SetSelectedGameplayModifiers){
					.base = base,
					.flags = {1, 0, 0, 0},
					.gameplayModifiers = session_get_modifiers(room, session),
				});
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			}
			break;
		}
		case MenuRpcType_GetRecommendedGameplayModifiers: pkt_readGetRecommendedGameplayModifiers(session->net.version, data); break;
		NOT_IMPLEMENTED(MenuRpcType_LevelLoadError);
		NOT_IMPLEMENTED(MenuRpcType_LevelLoadSuccess);
		case MenuRpcType_StartLevel: uprintf("BAD TYPE: MenuRpcType_StartLevel\n"); break;
		case MenuRpcType_GetStartedLevel: {
			pkt_readGetStartedLevel(session->net.version, data);
			if(!(session->state & ServerState_Synchronizing)) {
				break;
			} else if(!(room->state & ServerState_Game)) {
				session_set_state(ctx, room, session, room->state);
				break;
			}
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, StartLevel, (struct StartLevel){
				.base = base,
				.flags = {1, 1, 1, 0},
				.beatmapId = session_get_beatmap(room, session),
				.gameplayModifiers = session_get_modifiers(room, session),
				.startTime = base.syncTime,
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			session_set_state(ctx, room, session, ServerState_Game_LoadingScene);
			break;
		}
		case MenuRpcType_CancelLevelStart: uprintf("BAD TYPE: MenuRpcType_CancelLevelStart\n"); break;
		case MenuRpcType_GetMultiplayerGameState: {
			pkt_readGetMultiplayerGameState(session->net.version, data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetMultiplayerGameState r_state;
			r_state.base.syncTime = room_get_syncTime(room);
			r_state.flags = (struct RemoteProcedureCallFlags){1, 0, 0, 0};
			r_state.lobbyState = (room->state & ServerState_Lobby) ? MultiplayerGameState_Lobby : MultiplayerGameState_Game;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetMultiplayerGameState, r_state);
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetMultiplayerGameState: uprintf("BAD TYPE: MenuRpcType_SetMultiplayerGameState\n"); break;
		case MenuRpcType_GetIsReady: pkt_readGetIsReady(session->net.version, data); break;
		case MenuRpcType_SetIsReady: {
			struct SetIsReady ready = pkt_readSetIsReady(session->net.version, data);
			_Bool isReady = ready.flags.hasValue0 && ready.isReady;
			if(room->state & ServerState_Lobby)
				if(Counter128_set(&room->lobby.isReady, indexof(room->players, session), isReady) != isReady && (room->state & ServerState_Selected))
					room_set_state(ctx, room, ServerState_Lobby_Ready);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_SetStartGameTime);
		NOT_IMPLEMENTED(MenuRpcType_CancelStartGameTime);
		case MenuRpcType_GetIsInLobby: pkt_readGetIsInLobby(session->net.version, data); break;
		case MenuRpcType_SetIsInLobby: {
			struct SetIsInLobby isInLobby = pkt_readSetIsInLobby(session->net.version, data);
			_Bool inLobby = isInLobby.flags.hasValue0 && isInLobby.isBack;
			if(Counter128_set(&room->global.inLobby, indexof(room->players, session), inLobby) != inLobby && (room->state & ServerState_Selected))
				room_set_state(ctx, room, ServerState_Lobby_Ready);
			break;
		}
		case MenuRpcType_GetCountdownEndTime: {
			pkt_readGetCountdownEndTime(session->net.version, data);
			if(!(room->state & ServerState_Lobby))
				break;
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetIsStartButtonEnabled, (struct SetIsStartButtonEnabled){
				.base = base,
				.flags = {1, 0, 0, 0},
				.reason = room->lobby.reason,
			});
			if(room->state & (ServerState_Lobby_Countdown | ServerState_Lobby_Downloading)) {
				SERIALIZE_MENURPC(session->net.version, &resp_end, SetCountdownEndTime, (struct SetCountdownEndTime){
					.base = base,
					.flags = {1, 0, 0, 0},
					.newTime = room->global.timeout,
				});
			}
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetCountdownEndTime: uprintf("BAD TYPE: MenuRpcType_SetCountdownEndTime\n"); break;
		case MenuRpcType_CancelCountdown: uprintf("BAD TYPE: MenuRpcType_CancelCountdown\n"); break;
		NOT_IMPLEMENTED(MenuRpcType_GetOwnedSongPacks);
		case MenuRpcType_SetOwnedSongPacks: pkt_readSetOwnedSongPacks(session->net.version, data); break;
		NOT_IMPLEMENTED(MenuRpcType_RequestKickPlayer);
		case MenuRpcType_GetPermissionConfiguration:  {
			pkt_readGetPermissionConfiguration(session->net.version, data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetPermissionConfiguration r_permission;
			r_permission.base.syncTime = room_get_syncTime(room);
			r_permission.flags = (struct RemoteProcedureCallFlags){1, 0, 0, 0};
			r_permission.playersPermissionConfiguration.count = 0;
			FOR_SOME_PLAYERS(room, id, room->connected,)
				r_permission.playersPermissionConfiguration.playersPermission[r_permission.playersPermissionConfiguration.count++] = room->players[id].permissions;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetPermissionConfiguration, r_permission);
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		NOT_IMPLEMENTED(MenuRpcType_SetPermissionConfiguration);
		case MenuRpcType_GetIsStartButtonEnabled: {
			pkt_readGetIsStartButtonEnabled(session->net.version, data);
			uprintf("GET BUTTON GET BUTTON GET BUTTON GET BUTTON GET BUTTON\n");
			if(!(room->state & ServerState_Lobby))
				break;
			struct RemoteProcedureCall base = {room_get_syncTime(room)};
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(session->net.version, &resp_end, SetIsStartButtonEnabled, (struct SetIsStartButtonEnabled){
				.base = base,
				.flags = {1, 0, 0, 0},
				.reason = room->lobby.reason,
			});
			instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetIsStartButtonEnabled: uprintf("BAD TYPE: MenuRpcType_SetIsStartButtonEnabled\n"); break;
		default: uprintf("BAD MENU RPC TYPE\n");
	}
}

static void room_try_finish(struct Context *ctx, struct Room *room) {
	if(!Counter128_contains(room->game.levelFinished, room->connected))
		return;
	room->global.roundRobin = Counter128_get_next(room->connected, room->global.roundRobin);
	room_set_state(ctx, room, room->game.showResults ? ServerState_Game_Results : ServerState_Lobby_Idle);
}

static void handle_GameplayRpc(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct GameplayRpcHeader rpc = pkt_readGameplayRpcHeader(session->net.version, data);
	switch(rpc.type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: uprintf("BAD TYPE: GameplayRpcType_SetGameplaySceneSyncFinish\n"); break;
		case GameplayRpcType_SetGameplaySceneReady: {
			struct SetGameplaySceneReady ready = pkt_readSetGameplaySceneReady(session->net.version, data);
			if(!(room->state & ServerState_Game_LoadingScene)) {
				if((room->state & ServerState_Game) && room->state > ServerState_Game_LoadingScene)
					session_set_state(ctx, room, session, ServerState_Game_LoadingSong);
				break;
			}
			session->settings = ready.flags.hasValue0 ? ready.playerSpecificSettingsNetSerializable : CLEAR_SETTINGS;
			if(Counter128_set(&room->game.loadingScene.isLoaded, indexof(room->players, session), 1) == 0)
				if(Counter128_contains(room->game.loadingScene.isLoaded, room->connected))
					room_set_state(ctx, room, ServerState_Game_LoadingSong);
			break;
		}
		case GameplayRpcType_GetGameplaySceneReady: uprintf("BAD TYPE: GameplayRpcType_GetGameplaySceneReady\n"); break;
		NOT_IMPLEMENTED(GameplayRpcType_SetActivePlayerFailedToConnect);
		case GameplayRpcType_SetGameplaySongReady: {
			pkt_readSetGameplaySongReady(session->net.version, data);
			if(!(room->state & ServerState_Game_LoadingSong)) {
				if((room->state & ServerState_Game) && room->state > ServerState_Game_LoadingSong)
					session_set_state(ctx, room, session, ServerState_Game_Gameplay);
				break;
			}
			if(Counter128_set(&room->game.loadingSong.isLoaded, indexof(room->players, session), 1) == 0)
				if(Counter128_contains(room->game.loadingSong.isLoaded, room->connected))
					room_set_state(ctx, room, ServerState_Game_Gameplay);
			break;
		}
		case GameplayRpcType_GetGameplaySongReady: uprintf("BAD TYPE: GameplayRpcType_GetGameplaySongReady\n"); break;
		NOT_IMPLEMENTED(GameplayRpcType_SetSongStartTime);
		case GameplayRpcType_NoteCut: pkt_readNoteCut(session->net.version, data); break;
		case GameplayRpcType_NoteMissed: pkt_readNoteMissed(session->net.version, data); break;
		case GameplayRpcType_LevelFinished: {
			struct MultiplayerLevelCompletionResults results = pkt_readLevelFinished(session->net.version, data).results;
			if(!(room->state & ServerState_Game))
				break;
			if(Counter128_set(&room->game.levelFinished, indexof(room->players, session), 1))
				break;
			if(!room->skipResults) {
				if(session->net.version.protocolVersion < 7)
					room->game.showResults |= (results.levelEndState == MultiplayerLevelEndState_Cleared);
				else
					room->game.showResults |= (results.playerLevelEndReason == MultiplayerPlayerLevelEndReason_Cleared);
			}
			room_try_finish(ctx, room);
			break;
		}
		NOT_IMPLEMENTED(GameplayRpcType_ReturnToMenu);
		case GameplayRpcType_RequestReturnToMenu: {
			pkt_readRequestReturnToMenu(session->net.version, data);
			if((room->state & ServerState_Game) && String_eq(session->permissions.userId, room->managerId))
				room_set_state(ctx, room, ServerState_Lobby_Idle);
			break;
		}
		case GameplayRpcType_NoteSpawned: pkt_readNoteSpawned(session->net.version, data); break;
		case GameplayRpcType_ObstacleSpawned: pkt_readObstacleSpawned(session->net.version, data); break;
		case GameplayRpcType_SliderSpawned: pkt_readSliderSpawned(session->net.version, data); break;
		default: uprintf("BAD GAMEPLAY RPC TYPE\n");
	}
}

static void handle_MpCore(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MpCore mpHeader = pkt_readMpCore(session->net.version, data);
	if(String_is(mpHeader.type, "MpBeatmapPacket"))
		pkt_readMpBeatmapPacket(session->net.version, data);
	else if(String_is(mpHeader.type, "MpPlayerData"))
		pkt_readMpPlayerData(session->net.version, data);
	else if(String_is(mpHeader.type, "CustomAvatarPacket"))
		pkt_readCustomAvatarPacket(session->net.version, data);
	else
		uprintf("BAD MPCORE MESSAGE TYPE: '%.*s'\n", mpHeader.type.length, mpHeader.type.data);
}

static void handle_BeatUpMessage(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct BeatUpMessageHeader message = pkt_readBeatUpMessageHeader(session->net.version, data);
	switch(message.type) {
		case BeatUpMessageType_RecommendPreview: pkt_readRecommendPreview(session->net.version, data); break;
		case BeatUpMessageType_SetCanShareBeatmap: {
			struct SetCanShareBeatmap share = pkt_readSetCanShareBeatmap(session->net.version, data);
			if(!((room->state & ServerState_Lobby_Entitlement) && share.base.fileSize && String_eq(share.base.levelId, room->global.selectedBeatmap.levelID)))
				break;
			if(!room->lobby.entitlement.shareSet) {
				room->lobby.entitlement.shareSet = 1;
				memcpy(room->lobby.entitlement.shareHash, share.base.levelHash, sizeof(share.base.levelHash));
				room->lobby.entitlement.shareSize = share.base.fileSize;
			} else if(share.base.fileSize != room->lobby.entitlement.shareSize || memcmp(share.base.levelHash, room->lobby.entitlement.shareHash, sizeof(share.base.levelHash))) {
				if(room->lobby.entitlement.shareSize)
					uprintf("Hash mismatch; Disabling direct downloads\n");
				room->lobby.entitlement.canShare = COUNTER128_CLEAR;
				room->lobby.entitlement.shareSize = 0;
				break;
			}
			if(share.canShare)
				Counter128_set(&room->lobby.entitlement.canShare, indexof(room->players, session), 1);
			break;
		}
		case BeatUpMessageType_DirectDownloadInfo: uprintf("BAD TYPE: BeatUpMessageType_DirectDownloadInfo\n"); break;
		case BeatUpMessageType_LevelFragmentRequest: uprintf("BAD TYPE: BeatUpMessageType_LevelFragmentRequest\n"); break;
		case BeatUpMessageType_LevelFragment: uprintf("BAD TYPE: BeatUpMessageType_LevelFragment\n"); break;
		case BeatUpMessageType_LoadProgress: pkt_readLoadProgress(session->net.version, data); break;
		default: uprintf("BAD BEAT UP MESSAGE TYPE\n");
	}
}

static void handle_MultiplayerSession(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end) {
	struct MultiplayerSessionMessageHeader message = pkt_readMultiplayerSessionMessageHeader(session->net.version, data);
	switch(message.type) {
		case MultiplayerSessionMessageType_MenuRpc: handle_MenuRpc(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_GameplayRpc: handle_GameplayRpc(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: pkt_readNodePoseSyncState(session->net.version, data); break;
		case MultiplayerSessionMessageType_ScoreSyncState: pkt_readScoreSyncState(session->net.version, data); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_readNodePoseSyncStateDelta(session->net.version, data); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_readScoreSyncStateDelta(session->net.version, data); break;
		case MultiplayerSessionMessageType_MpCore: handle_MpCore(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_BeatUpMessage: handle_BeatUpMessage(ctx, room, session, data); break;
		default: uprintf("BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
	}
}

static void session_refresh_stateHash(struct Context *ctx, struct Room *room, struct InstanceSession *session) {
	_Bool isSpectating = !PlayerStateHash_contains(session->stateHash, "wants_to_play_next_level");
	_Bool allPlayersSpectating = Counter128_contains(room->global.isSpectating, room->connected);
	if(Counter128_set(&room->global.isSpectating, indexof(room->players, session), isSpectating) != isSpectating)
		if(Counter128_contains(room->global.isSpectating, room->connected) != allPlayersSpectating && (room->state & ServerState_Selected))
			room_set_state(ctx, room, ServerState_Lobby_Ready);
}

static void handle_PlayerIdentity(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct PlayerIdentity identity = pkt_readPlayerIdentity(session->net.version, data);
	session->stateHash = identity.playerState;
	session_refresh_stateHash(ctx, room, session);
	session->avatar = identity.playerAvatar;
	if(identity.random.length == sizeof(session->random))
		memcpy(session->random, identity.random.data, sizeof(session->random));
	else
		memset(session->random, 0, sizeof(session->random));
	session->publicEncryptionKey = identity.publicEncryptionKey;
	if(session->sentIdentity)
		return;
	session->sentIdentity = 1;

	{
		struct PlayerConnected r_connected;
		r_connected.remoteConnectionId = indexof(room->players, session) + 1;
		r_connected.userId = session->permissions.userId;
		r_connected.userName = session->userName;
		r_connected.isConnectionOwner = 0;
		FOR_EXCLUDING_PLAYER(room, session, id, room->connected) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_CUSTOM(room->players[id].net.version, &resp_end, InternalMessageType_PlayerConnected)
				pkt_writePlayerConnected(room->players[id].net.version, &resp_end, r_connected);
			instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}

		struct PlayerSortOrderUpdate r_sort;
		r_sort.userId = session->permissions.userId;
		r_sort.sortIndex = indexof(room->players, session);
		FOR_SOME_PLAYERS(room, id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_CUSTOM(room->players[id].net.version, &resp_end, InternalMessageType_PlayerSortOrderUpdate)
				pkt_writePlayerSortOrderUpdate(room->players[id].net.version, &resp_end, r_sort);
			instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}

		FOR_SOME_PLAYERS(room, id, room->connected,) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){indexof(room->players, session) + 1, 0, 0});
			SERIALIZE_CUSTOM(room->players[id].net.version, &resp_end, InternalMessageType_PlayerIdentity)
				pkt_writePlayerIdentity(room->players[id].net.version, &resp_end, identity);
			instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}
	}

	struct RemoteProcedureCall base;
	base.syncTime = room_get_syncTime(room);

	FOR_SOME_PLAYERS(room, id, room->connected,) {
		uint8_t resp[65536], *resp_end = resp;
		pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){0, 127, 0});
		SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, GetRecommendedBeatmap, (struct GetRecommendedBeatmap){.base = base});
		SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, GetRecommendedGameplayModifiers, (struct GetRecommendedGameplayModifiers){.base = base});
		SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, GetOwnedSongPacks, (struct GetOwnedSongPacks){.base = base});
		SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, GetIsReady, (struct GetIsReady){.base = base});
		SERIALIZE_MENURPC(room->players[id].net.version, &resp_end, GetIsInLobby, (struct GetIsInLobby){.base = base});
		instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	}
}

static _Bool handle_RoutingHeader(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, _Bool reliable, DeliveryMethod channelId) {
	struct RoutingHeader routing = pkt_readRoutingHeader(session->net.version, data);
	if(routing.connectionId) {
		struct Counter128 mask = room->connected;
		if(routing.connectionId != 127) {
			mask = COUNTER128_CLEAR;
			Counter128_set(&mask, routing.connectionId - 1, 1);
		}
		Counter128_set(&mask, indexof(room->players, session), 0);
		FOR_SOME_PLAYERS(room, id, mask,) {
			uint8_t resp[65536], *resp_end = resp;
			if(!reliable)
				pkt_writeNetPacketHeader(room->players[id].net.version, &resp_end, (struct NetPacketHeader){PacketProperty_Unreliable, 0, 0});
			pkt_writeRoutingHeader(room->players[id].net.version, &resp_end, (struct RoutingHeader){indexof(room->players, session) + 1, routing.connectionId == 127 ? 127 : 0, routing.encrypted});
			pkt_writeUint8Array(room->players[id].net.version, &resp_end, *data, end - *data);
			if(reliable)
				instance_send_channeled(&room->players[id].net, &room->players[id].channels, resp, resp_end - resp, channelId);
			else
				net_send_internal(&ctx->net, &room->players[id].net, resp, resp_end - resp, 1);
		}
		return routing.connectionId != 127 || routing.encrypted;
	}
	return 0;
}

static void log_length_error(const char *msg, const uint8_t *read, const uint8_t *data, size_t length) {
	if(read >= _trap && read < &_trap[sizeof(_trap)])
		uprintf("%s (expected %u)\n", msg, length);
	else
		uprintf("%s (expected %u, read %zu)\n", msg, length, read - (data - length));
	if(read < data) {
		uprintf("\t");
		for(const uint8_t *it = data - length; it < data; ++it)
			uprintf("%02hhx", *it);
		uprintf("\n\t");
		for(const uint8_t *it = data - length; it < read; ++it)
			uprintf("  ");
		uprintf("^ extra data starts here");
	}
	uprintf("\n");
}

static void process_message(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, _Bool reliable, DeliveryMethod channelId) {
	if(handle_RoutingHeader(ctx, room, session, data, end, reliable, channelId)) {
		*data = end;
		return;
	}
	while(*data < end) {
		struct SerializeHeader serial = pkt_readSerializeHeader(session->net.version, data);
		const uint8_t *sub = (*data)--;
		*data += serial.length;
		if(*data > end) {
			uprintf("Invalid serial length: %u\n", serial.length);
			return;
		}
		switch(serial.type) {
			case InternalMessageType_SyncTime: uprintf("BAD TYPE: InternalMessageType_SyncTime\n");
			case InternalMessageType_PlayerConnected: uprintf("BAD TYPE: InternalMessageType_PlayerConnected\n");
			case InternalMessageType_PlayerIdentity: handle_PlayerIdentity(ctx, room, session, &sub); break;
			NOT_IMPLEMENTED(InternalMessageType_PlayerLatencyUpdate);
			case InternalMessageType_PlayerDisconnected: uprintf("BAD TYPE: InternalMessageType_PlayerDisconnected\n");
			case InternalMessageType_PlayerSortOrderUpdate: uprintf("BAD TYPE: InternalMessageType_PlayerSortOrderUpdate\n");
			NOT_IMPLEMENTED(InternalMessageType_Party);
			case InternalMessageType_MultiplayerSession: handle_MultiplayerSession(ctx, room, session, &sub, *data); break;
			NOT_IMPLEMENTED(InternalMessageType_KickPlayer);
			case InternalMessageType_PlayerStateUpdate: {
				session->stateHash = pkt_readPlayerStateUpdate(session->net.version, &sub).playerState;
				session_refresh_stateHash(ctx, room, session);
				break;
			}
			NOT_IMPLEMENTED(InternalMessageType_PlayerAvatarUpdate);
			case InternalMessageType_PingMessage: {
				struct PingMessage ping = pkt_readPingMessage(session->net.version, &sub);

				struct PongMessage r_pong;
				r_pong.pingTime = ping.pingTime;
				struct SyncTime r_sync;
				r_sync.syncTime = room_get_syncTime(room);
				uint8_t resp[65536], *resp_end = resp;
				pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
				SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_PongMessage)
					pkt_writePongMessage(session->net.version, &resp_end, r_pong);
				SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_SyncTime)
					pkt_writeSyncTime(session->net.version, &resp_end, r_sync);
				instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
				break;
			}
			case InternalMessageType_PongMessage: pkt_readPongMessage(session->net.version, &sub); break;
			default: uprintf("BAD INTERNAL MESSAGE TYPE\n");
		}
		if(sub != *data)
			log_length_error("BAD INTERNAL MESSAGE LENGTH", sub, *data, serial.length);
	}
}

static void process_Channeled(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId) {
	process_message(ctx, room, session, data, end, 1, channelId);
}

static void handle_ConnectRequest(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end) {
	struct ConnectRequest req = pkt_readConnectRequest(session->net.version, data);
	session->net.version.netVersion = req.protocolId;
	if(!(String_eq(req.secret, session->secret) && String_eq(req.userId, session->permissions.userId))) {
		*data = end;
		return;
	}
	while(*data < end) {
		struct ModConnectHeader mod = pkt_readModConnectHeader(session->net.version, data);
		const uint8_t *sub = *data;
		*data += mod.length;
		if(String_is(mod.name, "BeatUpClient beta0")) {
			struct BeatUpConnectHeader header = pkt_readBeatUpConnectHeader(session->net.version, &sub);
			session->net.version.beatUpVersion = header.protocolId;
			session->net.version.windowSize = header.windowSize;
			if(session->net.version.windowSize > NET_MAX_WINDOW_SIZE)
				session->net.version.windowSize = NET_MAX_WINDOW_SIZE;
			if(session->net.version.windowSize < 32)
				session->net.version.windowSize = 32;
			session->directDownloads = header.directDownloads;
			if(String_eq(req.userId, room->managerId)) {
				room->countdownDuration = header.countdownDuration / 4.f;
				room->skipResults = header.skipResults;
				room->perPlayerDifficulty = header.perPlayerDifficulty;
				room->perPlayerModifiers = header.perPlayerModifiers;
			}
		} else {
			uprintf("UNIDENTIFIED MOD: %.*s\n", mod.name.length, mod.name.data);
		}
		if(sub != *data)
			log_length_error("BAD MOD HEADER LENGTH", sub, *data, mod.length);
	}
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(session->net.version, &resp_end, (struct NetPacketHeader){PacketProperty_ConnectAccept, 0, 0});
	pkt_writeConnectAccept(session->net.version, &resp_end, (struct ConnectAccept){
		.connectTime = req.connectTime,
		.reusedPeer = 0,
		.connectNum = 0,
		.peerId = indexof(room->players, session),
		.windowSize = session->net.version.windowSize,
		.countdownDuration = room->countdownDuration * 4,
		.directDownloads = session->directDownloads,
		.skipResults = room->skipResults,
		.perPlayerDifficulty = room->perPlayerDifficulty,
		.perPlayerModifiers = room->perPlayerModifiers,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
	if(session->state & ServerState_Connected)
		return;

	resp_end = resp;
	pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 127, 0});
	struct SyncTime r_sync;
	r_sync.syncTime = room_get_syncTime(room);
	SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_SyncTime)
		pkt_writeSyncTime(session->net.version, &resp_end, r_sync);
	instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	uprintf("connect[%zu]: %.*s (%.*s)\n", indexof(room->players, session), session->userName.base.length, session->userName.base.data, session->permissions.userId.length, session->permissions.userId.data);

	FOR_SOME_PLAYERS(room, id, room->connected,) {
		struct PlayerConnected r_connected;
		r_connected.remoteConnectionId = id + 1;
		r_connected.userId = room->players[id].permissions.userId;
		r_connected.userName = room->players[id].userName;
		r_connected.isConnectionOwner = 0;
		resp_end = resp;
		pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
		SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_PlayerConnected)
			pkt_writePlayerConnected(session->net.version, &resp_end, r_connected);
		instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

		struct PlayerSortOrderUpdate r_sort;
		r_sort.userId = room->players[id].permissions.userId;
		r_sort.sortIndex = id;
		resp_end = resp;
		pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
		SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_PlayerSortOrderUpdate)
			pkt_writePlayerSortOrderUpdate(session->net.version, &resp_end, r_sort);
		instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

		struct PlayerIdentity r_identity;
		r_identity.playerState = room->players[id].stateHash;
		r_identity.playerAvatar = room->players[id].avatar;
		r_identity.random.length = 32;
		memcpy(r_identity.random.data, session->random, sizeof(session->random));
		r_identity.publicEncryptionKey = session->publicEncryptionKey;
		uprintf("TODO: do we need to include the encrytion key?\n");
		resp_end = resp;
		pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){id + 1, 0, 0});
		SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_PlayerIdentity)
			pkt_writePlayerIdentity(session->net.version, &resp_end, r_identity);
		instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	}

	resp_end = resp;
	pkt_writeRoutingHeader(session->net.version, &resp_end, (struct RoutingHeader){0, 0, 0});
	struct PlayerIdentity r_identity;
	r_identity.playerState.bloomFilter.d0 = 288266110296588352;
	r_identity.playerState.bloomFilter.d1 = 576531121051926529;
	memset(&r_identity.playerAvatar, 0, sizeof(r_identity.playerAvatar));
	r_identity.random.length = 32;
	memcpy(r_identity.random.data, NetKeypair_get_random(&room->keys), 32);
	r_identity.publicEncryptionKey.length = sizeof(r_identity.publicEncryptionKey.data);
	NetKeypair_write_key(&room->keys, &ctx->net, r_identity.publicEncryptionKey.data, &r_identity.publicEncryptionKey.length);
	SERIALIZE_CUSTOM(session->net.version, &resp_end, InternalMessageType_PlayerIdentity)
		pkt_writePlayerIdentity(session->net.version, &resp_end, r_identity);
	instance_send_channeled(&session->net, &session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	if(room->state & ServerState_Game)
		session_set_state(ctx, room, session, ServerState_Synchronizing);
	else
		session_set_state(ctx, room, session, room->state); // TODO: potential for desync since `StartListeningToGameStart()` hasn't been called at this point
}

static void log_players(const struct Room *room, struct InstanceSession *session, const char *prefix) {
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	uprintf("%sconnect %s\n", prefix, addrstr);
	uprintf("player bits: ");
	for(uint32_t i = 0; i < lengthof(room->playerSort.bits); ++i)
		for(uint32_t b = 0; b < sizeof(*room->playerSort.bits) * 8; ++b)
			uprintf("%u", (room->playerSort.bits[i] >> b) & 1);
	uprintf("\n");
}

enum DisconnectMode {
	DC_RESET = 1,
	DC_NOTIFY = 2,
};
static void room_disconnect(struct Context *ctx, struct Room **room, struct InstanceSession *session, enum DisconnectMode mode) {
	uint32_t id = indexof((*room)->players, session);
	Counter128_set(&(*room)->playerSort, id, 0);
	log_players(*room, session, (mode & DC_RESET) ? "re" : "dis");
	instance_channels_free(&session->channels);
	if(mode & DC_RESET)
		net_session_reset(&ctx->net, &session->net);
	else
		net_session_free(&session->net);

	if(!Counter128_isEmpty((*room)->playerSort)) {
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

	uint16_t group = indexof(*ctx->rooms, room) / lengthof(*ctx->rooms);
	struct RoomHandle handle = {
		.block = ctx->notifyHandle[group],
		.sub = indexof(ctx->rooms[group], room) % lengthof(*ctx->rooms),
	};
	net_unlock(&ctx->net); // avoids deadlock if pool_room_close_notify() internally calls instance_block_release()
	pool_room_close_notify(handle);
	net_lock(&ctx->net);

	net_keypair_free(&(*room)->keys);
	free(*room);
	*room = NULL;
	uprintf("closing room (%hu,%hu,%hhu)\n", indexof(contexts, ctx), handle.block, handle.sub);
}

static void room_close(struct Context *ctx, struct Room **room) {
	FOR_SOME_PLAYERS(*room, id, (*room)->playerSort,)
		room_disconnect(ctx, room, &(*room)->players[id], 0);
}

static inline void handle_packet(struct Context *ctx, struct Room **room, struct InstanceSession *session, const uint8_t *data, const uint8_t *end) {
	struct NetPacketHeader header = pkt_readNetPacketHeader(session->net.version, &data);
	if(data >= end)
		return;
	uint16_t len = end - data;
	const uint8_t *sub = data;
	if(header.property != PacketProperty_Merged)
		goto bypass;
	while(data < end) {
		len = pkt_readUint16(session->net.version, &data);
		sub = data;
		header = pkt_readNetPacketHeader(session->net.version, &sub);
		bypass:;
		data += len;
		if(session->state == 0 && header.property != PacketProperty_ConnectRequest)
			return;
		if(len == 0) {
			uprintf("ZERO LENGTH PACKET\n");
			return;
		}
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
			case PacketProperty_Channeled: handle_Channeled((ChanneledHandler)process_Channeled, &ctx->net, &session->net, &session->channels, ctx, *room, session, &sub, data, header.isFragmented); break;
			case PacketProperty_Ack: handle_Ack(&session->net, &session->channels, &sub); break;
			case PacketProperty_Ping: handle_Ping(&ctx->net, &session->net, &session->tableTennis, &sub); break;
			case PacketProperty_Pong: {
				struct PlayerLatencyUpdate r_latency;
				r_latency.latency = handle_Pong(&ctx->net, &session->net, &session->tableTennis, &sub);
				if(r_latency.latency != 0 && session->net.version.protocolVersion < 7) {
					FOR_EXCLUDING_PLAYER(*room, session, id, (*room)->connected) {
						uint8_t resp[65536], *resp_end = resp;
						pkt_writeRoutingHeader((*room)->players[id].net.version, &resp_end, (struct RoutingHeader){indexof((*room)->players, session) + 1, 0, 0}); // TODO: is this right?
						SERIALIZE_CUSTOM((*room)->players[id].net.version, &resp_end, InternalMessageType_PlayerLatencyUpdate)
							pkt_writePlayerLatencyUpdate((*room)->players[id].net.version, &resp_end, r_latency);
						instance_send_channeled(&(*room)->players[id].net, &(*room)->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
					}
				}
				break;
			}
			case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, *room, session, &sub, data); break;
			case PacketProperty_ConnectAccept: uprintf("BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
			case PacketProperty_Disconnect: {
				pkt_readDisconnect(session->net.version, &sub);
				room_disconnect(ctx, room, session, DC_NOTIFY);
				break;
			}
			case PacketProperty_UnconnectedMessage: uprintf("BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
			case PacketProperty_MtuCheck: handle_MtuCheck(&ctx->net, &session->net, &sub); break;
			NOT_IMPLEMENTED(PacketProperty_MtuOk);
			NOT_IMPLEMENTED(PacketProperty_Broadcast);
			case PacketProperty_Merged: uprintf("BAD TYPE: PacketProperty_Merged\n"); break;
			NOT_IMPLEMENTED(PacketProperty_ShutdownOk);
			NOT_IMPLEMENTED(PacketProperty_PeerNotFound);
			NOT_IMPLEMENTED(PacketProperty_InvalidProtocol);
			NOT_IMPLEMENTED(PacketProperty_NatMessage);
			NOT_IMPLEMENTED(PacketProperty_Empty);
			default: uprintf("BAD PACKET PROPERTY\n");
		}
		if(sub != data)
			uprintf("BAD PACKET LENGTH (expected %u, read %zu)\n", len, len + sub - data);
	}
}

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
instance_handler(struct Context *ctx) {
	net_lock(&ctx->net);
	uprintf("Started\n");
	uint8_t buf[262144];
	memset(buf, 0, sizeof(buf));
	uint32_t len;
	struct Room **room;
	struct InstanceSession *session;
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt, (void**)&room))) {
		#ifdef PACKET_LOGGING_FUNCS
		{
			const uint8_t *read = pkt;
			struct NetPacketHeader header = pkt_readNetPacketHeader(session->net.version, &read);
			if(header.property != PacketProperty_Unreliable && !header.isFragmented) {
				uprintf("recieve[%s]:\n", reflect(PacketProperty, header.property));
				debug_logPacket(read, &pkt[len], header, session->net.version, NULL);
			}
		}
		#endif
		handle_packet(ctx, room, session, pkt, &pkt[len]);
	}
	net_unlock(&ctx->net);
	return 0;
}

static struct NetSession *instance_onResolve(struct Context *ctx, struct SS addr, void **userdata_out) {
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(*room, id, (*room)->playerSort,) {
			if(addrs_are_equal(&addr, NetSession_get_addr(&(*room)->players[id].net))) {
				*userdata_out = room;
				return &(*room)->players[id].net;
			}
		}
	}
	return NULL;
}

static void instance_onResend(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick) {
	FOR_ALL_ROOMS(ctx, room) {
		FOR_SOME_PLAYERS(*room, id, (*room)->playerSort,) {
			struct InstanceSession *session = &(*room)->players[id];
			uint32_t kickTime = NetSession_get_lastKeepAlive(&session->net) + 10000;
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
			} else if((*room)->state & ServerState_Lobby_Countdown) {
				room_set_state(ctx, *room, ServerState_Lobby_Downloading);
			} else {
				room_set_state(ctx, *room, ServerState_Lobby_Idle);
			}
		}

		FOR_SOME_PLAYERS(*room, id, (*room)->playerSort,)
			net_flush_merged(&ctx->net, &(*room)->players[id].net);
	}
}

#ifdef WINDOWS
static HANDLE instance_threads[THREAD_COUNT];
#else
static pthread_t instance_threads[THREAD_COUNT];
#endif
static const char *instance_domain = NULL, *instance_domainIPv4 = NULL;
_Bool instance_init(const char *domain, const char *domainIPv4) {
	instance_domain = domain;
	instance_domainIPv4 = domainIPv4;
	memset(instance_threads, 0, sizeof(instance_threads));
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(net_init(&contexts[i].net, 5000 + i)) {
			uprintf("net_init() failed\n");
			return 1;
		}
		contexts[i].net.user = &contexts[i];
		contexts[i].net.onResolve = instance_onResolve;
		contexts[i].net.onResend = instance_onResend;
		contexts[i].blockAlloc = COUNTER16_CLEAR;
		memset(contexts[i].rooms, 0, sizeof(contexts->rooms));

		#ifdef WINDOWS
		instance_threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)instance_handler, &contexts[i], 0, NULL);
		#else
		if(pthread_create(&instance_threads[i], NULL, (void*(*)(void*))&instance_handler, &contexts[i]))
			instance_threads[i] = 0;
		#endif
		if(!instance_threads[i]) {
			uprintf("Instance thread creation failed\n");
			return 1;
		}
	}
	return 0;
}

void instance_cleanup() {
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(instance_threads[i]) {
			net_stop(&contexts[i].net);
			uprintf("Stopping #%u\n", i);
			#ifdef WINDOWS
			WaitForSingleObject(instance_threads[i], INFINITE);
			#else
			pthread_join(instance_threads[i], NULL);
			#endif
			instance_threads[i] = 0;
			FOR_ALL_ROOMS(&contexts[i], room)
				room_close(&contexts[i], room);
			contexts[i].blockAlloc = COUNTER16_CLEAR;
			memset(contexts[i].rooms, 0, sizeof(contexts->rooms));
			net_cleanup(&contexts[i].net);
		}
	}
}

struct NetContext *instance_get_net(uint16_t thread) {
	return &contexts[thread].net;
}

struct IPEndPoint instance_get_endpoint(_Bool ipv4) {
	struct IPEndPoint out;
	out.address.length = 0;
	out.address.isNull = 0;
	out.port = 0;
	if(ipv4)
		out.address.length = sprintf(out.address.data, "%s", instance_domainIPv4);
	else
		out.address.length = sprintf(out.address.data, "%s", instance_domain);

	struct SS addr = {sizeof(struct sockaddr_storage)};
	getsockname(net_get_sockfd(&contexts[0].net), &addr.sa, &addr.len);
	switch(addr.ss.ss_family) {
		case AF_INET: out.port = htons(addr.in.sin_port); break;
		case AF_INET6: out.port = htons(addr.in6.sin6_port); break;
		default:;
	}
	return out;
}

_Bool instance_block_request(uint16_t thread, uint16_t *group_out, uint16_t notify) {
	uint8_t group;
	net_lock(&contexts[thread].net);
	if(Counter16_set_next(&contexts[thread].blockAlloc, &group, 1)) {
		*group_out = group;
		contexts[thread].notifyHandle[group] = notify;
		uprintf("opening block (%hu,%hu)\n", thread, group);
		net_unlock(&contexts[thread].net);
		return 0;
	}
	net_unlock(&contexts[thread].net);
	uprintf("THREAD FULL\n");
	return 1;
}

void instance_block_release(uint16_t thread, uint16_t group) {
	net_lock(&contexts[thread].net);
	uprintf("closing block (%hu,%hu)\n", thread, group);
	Counter16_set(&contexts[thread].blockAlloc, group, 0);
	net_unlock(&contexts[thread].net);
}

_Bool instance_room_open(uint16_t thread, uint16_t group, uint8_t sub, struct String managerId, struct GameplayServerConfiguration configuration) {
	net_lock(&contexts[thread].net);
	uprintf("opening room (%hu,%hu,%hhu)\n", thread, group, sub);
	if(contexts[thread].rooms[group][sub]) {
		net_unlock(&contexts[thread].net);
		uprintf("Room already open!\n");
		return 1;
	}
	struct Room *room = malloc(sizeof(struct Room) + configuration.maxPlayerCount * sizeof(*room->players));
	if(!room) {
		uprintf("alloc error\n");
		abort();
	}
	net_keypair_init(&room->keys);
	net_keypair_gen(&contexts[0].net, &room->keys);
	room->managerId = managerId;
	room->configuration = configuration;
	room->syncBase = 0;
	room->syncBase = room_get_syncTime(room);
	room->countdownDuration = 5;
	room->perPlayerDifficulty = 0;
	room->perPlayerModifiers = 0;
	room->connected = COUNTER128_CLEAR;
	room->playerSort = COUNTER128_CLEAR;
	room->state = 0;
	room->global.inLobby = COUNTER128_CLEAR;
	room->global.isSpectating = COUNTER128_CLEAR;
	room->global.selectedBeatmap = CLEAR_BEATMAP;
	room->global.selectedModifiers = CLEAR_MODIFIERS;
	room->global.roundRobin = 0;
	room_set_state(&contexts[thread], room, ServerState_Lobby_Idle);
	contexts[thread].rooms[group][sub] = room;
	net_unlock(&contexts[thread].net);
	return 0;
}

void instance_room_close(uint16_t thread, uint16_t group, uint8_t sub) {
	net_lock(&contexts[thread].net);
	room_close(&contexts[thread], &contexts[thread].rooms[group][sub]);
	net_unlock(&contexts[thread].net);
}

struct String instance_room_get_managerId(uint16_t thread, uint16_t group, uint8_t sub) {
	net_lock(&contexts[thread].net);
	struct String out = contexts[thread].rooms[group][sub]->managerId;
	net_unlock(&contexts[thread].net);
	return out;
}

struct GameplayServerConfiguration instance_room_get_configuration(uint16_t thread, uint16_t group, uint8_t sub) {
	net_lock(&contexts[thread].net);
	struct GameplayServerConfiguration out = contexts[thread].rooms[group][sub]->configuration;
	net_unlock(&contexts[thread].net);
	return out;
}

struct PacketContext instance_room_get_protocol(uint16_t thread, uint16_t group, uint8_t sub) {
	struct PacketContext version = PV_LEGACY_DEFAULT;
	net_lock(&contexts[thread].net);
	struct Counter128 ct = contexts[thread].rooms[group][sub]->playerSort;
	uint32_t id = 0;
	if(Counter128_set_next(&ct, &id, 0))
		version = contexts[thread].rooms[group][sub]->players[id].net.version;
	net_unlock(&contexts[thread].net);
	return version;
}

struct NetSession *instance_room_resolve_session(uint16_t thread, uint16_t group, uint8_t sub, struct SS addr, struct String secret, struct String userId, struct ExString userName, struct PacketContext version) {
	struct Context *ctx = &contexts[thread];
	net_lock(&ctx->net);
	struct Room *room = ctx->rooms[group][sub];
	if(!room) {
		net_unlock(&ctx->net);
		return NULL;
	}
	struct InstanceSession *session = NULL;
	FOR_SOME_PLAYERS(*room, id, room->playerSort,) {
		if(addrs_are_equal(&addr, NetSession_get_addr(&room->players[id].net))) {
			session = &room->players[id];
			room_disconnect(ctx, &room, session, DC_RESET | DC_NOTIFY);
			break;
		}
	}
	if(!session) {
		struct Counter128 tmp = room->playerSort;
		uint32_t id = 0;
		if((!Counter128_set_next(&tmp, &id, 1)) || id >= room->configuration.maxPlayerCount) {
			net_unlock(&ctx->net);
			uprintf("ROOM FULL\n");
			return NULL;
		}
		session = &room->players[id];
		net_session_init(&ctx->net, &session->net, addr);
		session->net.version = version;
		room->playerSort = tmp;
	}
	session->secret = secret;
	session->userName = userName;
	session->permissions.userId = userId;
	session->permissions.isServerOwner = String_eq(userId, room->managerId);
	session->permissions.hasRecommendBeatmapsPermission = (room->configuration.songSelectionMode != SongSelectionMode_Random) && (session->permissions.isServerOwner || room->configuration.songSelectionMode != SongSelectionMode_OwnerPicks);
	session->permissions.hasRecommendGameplayModifiersPermission = (room->configuration.gameplayServerControlSettings == GameplayServerControlSettings_AllowModifierSelection || room->configuration.gameplayServerControlSettings == GameplayServerControlSettings_All);
	session->permissions.hasKickVotePermission = session->permissions.isServerOwner;
	session->permissions.hasInvitePermission = (room->configuration.invitePolicy == InvitePolicy_AnyoneCanInvite) || (session->permissions.isServerOwner && room->configuration.invitePolicy == InvitePolicy_OnlyConnectionOwnerCanInvite);
	session->sentIdentity = 0;
	session->directDownloads = 0;
	session->state = 0;
	session->recommendedBeatmap = CLEAR_BEATMAP;
	session->recommendedModifiers = CLEAR_MODIFIERS;

	instance_pingpong_init(&session->tableTennis);
	instance_channels_init(&session->channels);
	session->stateHash.bloomFilter = (struct BitMask128){0, 0};
	session->avatar = CLEAR_AVATARDATA;

	log_players(room, session, "");
	net_unlock(&ctx->net);
	return &session->net;
}
