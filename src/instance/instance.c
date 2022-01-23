// #define THREAD_COUNT 256
#define THREAD_COUNT 1

#include "../enum_reflection.h"
#include "instance.h"
#include "common.h"
#include "index.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include "../debug.h"
#include <mbedtls/error.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define FOR_ALL_PLAYERS(room, id) \
	struct Counter128 CONCAT(_i_,__LINE__) = (room)->playerSort; for(uint32_t (id) = 0; Counter128_set_next(&CONCAT(_i_,__LINE__), &id, 0); ++id)

#define FOR_EXCLUDING_PLAYER(room, session, id) \
	FOR_ALL_PLAYERS(room, id) \
		if(id != indexof((room)->players, (session)))

struct Counter128 {
	uint32_t bits[4];
};

void Counter128_clear(struct Counter128 *set) {
	memset(set->bits, 0, sizeof(set->bits));
}

_Bool Counter128_set(struct Counter128 *set, uint32_t bit, _Bool state) {
	uint32_t i = bit / (sizeof(*set->bits) * 8);
	bit %= sizeof(*set->bits) * 8;
	_Bool prev = (set->bits[i] >> bit) & 1;
	if(state)
		set->bits[i] |= 1 << bit;
	else
		set->bits[i] &= ~(1 << bit);
	return prev;
}

_Bool Counter128_set_next(struct Counter128 *set, uint32_t *bit, _Bool state) {
	for(uint32_t i = *bit / (sizeof(*set->bits) * 8); i < lengthof(set->bits); ++i) {
		uint32_t v = state ? ~set->bits[i] : set->bits[i];
		if(v) {
			*bit = i * (sizeof(*set->bits) * 8) +  __builtin_ctz(v);
			if(state)
				set->bits[i] |= set->bits[i] + 1;
			else
				set->bits[i] &= set->bits[i] - 1;
			return 1;
		}
	}
	return 0;
}

_Bool Counter128_isEmpty(struct Counter128 set) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if(set.bits[i])
			return 0;
	return 1;
}

_Bool Counter128_contains(struct Counter128 set, struct Counter128 subset) {
	for(uint32_t i = 0; i < lengthof(set.bits); ++i)
		if((set.bits[i] & subset.bits[i]) != subset.bits[i])
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
	ClientState clientState;
	struct String secret, userName;
	struct PlayerLobbyPermissionConfigurationNetSerializable permissions;
	struct PlayerStateHash state;
	struct MultiplayerAvatarData avatar;
	struct {
		struct SongPackMask ownedSongPacks;
		struct GameplayModifiers recommendedModifiers;
		struct BeatmapIdentifierNetSerializable recommendedBeatmap;
	} menu;
	struct {
		struct PlayerSpecificSettingsNetSerializable settings;
	} game;
	struct Pong pong;
	struct Channels channels;
};
struct Room {
	struct NetKeypair keys;
	struct GameplayServerConfiguration configuration;
	struct String managerId;
	float syncBase, countdownEnd;
	ServerState state;
	struct Counter128 inLobby, ready, entitled, spectating, sceneLoaded, songLoaded, levelFinished;
	struct BeatmapIdentifierNetSerializable selectedBeatmap;
	struct GameplayModifiers selectedModifiers; // TODO: recommend modifiers
	struct PlayersMissingEntitlementsNetSerializable buzzkills;
	_Bool canStart;
	struct Counter128 playerSort;
	struct InstanceSession *players;
};

struct Context {
	struct NetContext net;
	struct Room TEMPglobalRoom; // TODO: room allocation (interface for wire.c)
};

static struct NetSession *instance_onResolve(struct Context *ctx, struct SS addr, void **userdata_out) { // TODO: needs mutex
	struct Room *room = &ctx->TEMPglobalRoom;
	FOR_ALL_PLAYERS(room, id) {
		if(addrs_are_equal(&addr, NetSession_get_addr(&room->players[id].net))) {
			*userdata_out = room;
			return &room->players[id].net;
		}
	}
	return NULL;
}

static float room_get_syncTime(struct Room *room) {
	struct timespec now;
	if(clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec + (now.tv_nsec / 1000) / 1000000.f - room->syncBase;
}

static void refresh_countdown(struct Context *ctx, struct Room *room, _Bool start) {
	if(room->state != ServerState_Lobby)
		return;
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	uint8_t *routing_end = resp_end;
	if(room->canStart && Counter128_contains(Counter128_or(room->ready, room->spectating), room->playerSort)) {
		if(room->countdownEnd != 0) {
			if(!start)
				return;
			Counter128_clear(&room->sceneLoaded);
			Counter128_clear(&room->songLoaded);
			Counter128_clear(&room->levelFinished);
			room->state = ServerState_SceneLoading;

			struct SetSelectedBeatmap r_beatmap;
			struct SetSelectedGameplayModifiers r_modifiers;
			struct StartLevel r_start;
			struct GetGameplaySceneReady r_ready;
			r_ready.base.syncTime = r_start.base.syncTime = r_modifiers.base.syncTime = r_beatmap.base.syncTime = room_get_syncTime(room);
			r_start.beatmapId = r_beatmap.identifier = room->selectedBeatmap;
			r_start.gameplayModifiers = r_modifiers.gameplayModifiers = room->selectedModifiers;
			r_start.startTime = room->countdownEnd;
			FOR_ALL_PLAYERS(room, id) {
				resp_end = routing_end;
				if(PER_PLAYER_DIFFICULTY) {
					if(String_eq(room->players[id].menu.recommendedBeatmap.levelID, room->selectedBeatmap.levelID)) {
						r_start.beatmapId.beatmapCharacteristicSerializedName = room->players[id].menu.recommendedBeatmap.beatmapCharacteristicSerializedName;
						r_start.beatmapId.difficulty = room->players[id].menu.recommendedBeatmap.difficulty;
					} else {
						r_start.beatmapId.beatmapCharacteristicSerializedName = room->selectedBeatmap.beatmapCharacteristicSerializedName;
						r_start.beatmapId.difficulty = room->selectedBeatmap.difficulty;
					}
				}
				SERIALIZE_MENURPC(&resp_end, SetSelectedBeatmap, r_beatmap);
				SERIALIZE_MENURPC(&resp_end, SetSelectedGameplayModifiers, r_modifiers);
				SERIALIZE_MENURPC(&resp_end, StartLevel, r_start);
				SERIALIZE_GAMEPLAYRPC(&resp_end, GetGameplaySceneReady, r_ready);
				instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
				room->players[id].game.settings = (struct PlayerSpecificSettingsNetSerializable){
					.userId = room->players[id].permissions.userId,
					.userName = room->players[id].userName,
					.leftHanded = 0,
					.automaticPlayerHeight = 0,
					.playerHeight = 1.8,
					.headPosToPlayerHeightOffset = .1,
				};
				memset(&room->players[id].game.settings.colorScheme, 0, sizeof(room->players->game.settings.colorScheme));
			}
		} else {
			room->countdownEnd = room_get_syncTime(room) + 5;
			struct SetCountdownEndTime r_countdown;
			r_countdown.base.syncTime = room_get_syncTime(room);
			r_countdown.newTime = room->countdownEnd;
			SERIALIZE_MENURPC(&resp_end, SetCountdownEndTime, r_countdown);
			FOR_ALL_PLAYERS(room, id)
				instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		}
	} else {
		if(room->countdownEnd == 0)
			return;
		room->countdownEnd = 0;
		struct CancelCountdown r_cancelC;
		r_cancelC.base.syncTime = room_get_syncTime(room);
		SERIALIZE_MENURPC(&resp_end, CancelCountdown, r_cancelC);
		FOR_ALL_PLAYERS(room, id)
			instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
		resp_end = routing_end;
		struct CancelLevelStart r_cancelL;
		r_cancelL.base.syncTime = room_get_syncTime(room);
		SERIALIZE_MENURPC(&resp_end, CancelLevelStart, r_cancelL);
		FOR_ALL_PLAYERS(room, id)
			instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	}
}

static void refresh_button(struct Context *ctx, struct Room *room) {
	if(room->state != ServerState_Lobby)
		return;
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	struct SetIsStartButtonEnabled r_button;
	r_button.base.syncTime = room_get_syncTime(room);
	if(Counter128_isEmpty(room->inLobby))
		r_button.reason = CannotStartGameReason_AllPlayersNotInLobby;
	if(!room->selectedBeatmap.beatmapCharacteristicSerializedName.length)
		r_button.reason = CannotStartGameReason_NoSongSelected;
	else if(room->buzzkills.count)
		r_button.reason = CannotStartGameReason_DoNotOwnSong;
	else if(Counter128_contains(room->spectating, room->playerSort))
		r_button.reason = CannotStartGameReason_AllPlayersSpectating;
	else
		r_button.reason = CannotStartGameReason_None;
	SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_button);
	FOR_ALL_PLAYERS(room, id)
		instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	room->canStart = (r_button.reason == CannotStartGameReason_None);
	refresh_countdown(ctx, room, 0);
}

static void swap(uint8_t *a, uint8_t *b) {
	uint8_t c = *a;
	*a = *b, *b = c;
}

static void room_disconnect(struct Context *ctx, struct Room *room, struct InstanceSession *session) {
	if(session->clientState == ClientState_disconnected)
		return;
	session->clientState = ClientState_disconnected;
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	fprintf(stderr, "[INSTANCE] %sconnect %s\n", ctx ? "re" : "dis", addrstr);
	if(ctx)
		net_session_reset(&ctx->net, &session->net);
	else
		net_session_free(&session->net);
	while(session->channels.incomingFragmentsList) {
		struct IncomingFragments *e = session->channels.incomingFragmentsList;
		session->channels.incomingFragmentsList = session->channels.incomingFragmentsList->next;
		free(e);
	}
	uint32_t id = indexof(room->players, session);
	Counter128_set(&room->inLobby, id, 0);
	Counter128_set(&room->ready, id, 0);
	Counter128_set(&room->spectating, id, 0);
	Counter128_set(&room->playerSort, id, 0);
	if(Counter128_set(&room->entitled, id, 0)) {
		fprintf(stderr, "TODO: recount entitlement for selectedBeatmap\n");
	}
	fprintf(stderr, "[INSTANCE] player bits: ");
	for(uint32_t i = 0; i < lengthof(room->playerSort.bits); ++i)
		for(uint32_t b = 0; b < sizeof(*room->playerSort.bits) * 8; ++b)
			fprintf(stderr, "%u", (room->playerSort.bits[i] >> b) & 1);
	fprintf(stderr, "\n");
	refresh_button(ctx, room);
}

static void load_song(struct Context *ctx, struct Room *room) {
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct GetGameplaySongReady r_ready;
	struct SetGameplaySceneSyncFinish r_sync;
	r_sync.base.syncTime = r_ready.base.syncTime = room_get_syncTime(room);
	r_sync.playersAtGameStart.count = 0;
	FOR_ALL_PLAYERS(room, id)
		r_sync.playersAtGameStart.activePlayerSpecificSettingsAtGameStart[r_sync.playersAtGameStart.count++] = room->players[id].game.settings;
	r_sync.sessionGameId.length = sprintf(r_sync.sessionGameId.data, "f85bfe8f-beb5-407d-b07b-4fe5841e6ad0");
	SERIALIZE_GAMEPLAYRPC(&resp_end, GetGameplaySongReady, r_ready);
	SERIALIZE_GAMEPLAYRPC(&resp_end, SetGameplaySceneSyncFinish, r_sync);
	FOR_ALL_PLAYERS(room, id)
		instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	room->state = ServerState_SongLoading;
}

static void start_game(struct Context *ctx, struct Room *room) {
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct SetSongStartTime r_start;
	r_start.base.syncTime = room_get_syncTime(room);
	r_start.startTime = r_start.base.syncTime + .5;
	SERIALIZE_GAMEPLAYRPC(&resp_end, SetSongStartTime, r_start);
	FOR_ALL_PLAYERS(room, id)
		instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
	room->state = ServerState_Game;
}

static void instance_onResend(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick) { // TODO: needs mutex
	struct Room *room = &ctx->TEMPglobalRoom;
	if(Counter128_isEmpty(room->playerSort))
		return;
	FOR_ALL_PLAYERS(room, id) {
		struct InstanceSession *session = &room->players[id];
		uint32_t kickTime = NetSession_get_lastKeepAlive(&session->net) + 10000;
		if(currentTime > kickTime) {
			room_disconnect(NULL, room, session);
		} else {
			if(kickTime < *nextTick)
				*nextTick = kickTime;
			flush_ack(&ctx->net, &session->net, &session->channels.ru.base.ack);
			flush_ack(&ctx->net, &session->net, &session->channels.ro.base.ack);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(&ctx->net, &session->net, &session->channels.ru.base.resend[i], currentTime);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(&ctx->net, &session->net, &session->channels.ro.base.resend[i], currentTime);
			try_resend(&ctx->net, &session->net, &session->channels.rs.resend, currentTime);
		}
	}
	if(room->countdownEnd != 0) {
		float delta = room->countdownEnd - room_get_syncTime(room);
		switch(room->state) {
			case ServerState_Lobby: {
				if(delta > 0) {
					uint32_t ctick = delta * 1000;
					if(ctick < 10)
						ctick = 10;
					if(*nextTick - currentTime > ctick)
						*nextTick = currentTime + ctick;
				} else {
					refresh_countdown(ctx, room, 1);
				}
				break;
			}
			case ServerState_SceneLoading: {
				delta += SCENE_LOAD_TIMEOUT;
				if(delta > 0) {
					uint32_t ctick = delta * 1000;
					if(ctick < 10)
						ctick = 10;
					if(*nextTick - currentTime > ctick)
						*nextTick = currentTime + ctick;
				} else {
					load_song(ctx, room);
				}
				break;
			}
			case ServerState_SongLoading: {
				delta += SONG_LOAD_TIMEOUT;
				if(delta > 0) {
					uint32_t ctick = delta * 1000;
					if(ctick < 10)
						ctick = 10;
					if(*nextTick - currentTime > ctick)
						*nextTick = currentTime + ctick;
				} else {
					start_game(ctx, room);
				}
				break;
			}
			default:;
		}
	}
	FOR_ALL_PLAYERS(room, id)
		net_flush_merged(&ctx->net, &room->players[id].net);
}

static void handle_MenuRpc(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(data);
	switch(rpc.type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: fprintf(stderr, "[INSTANCE] MenuRpcType_SetPlayersMissingEntitlementsToLevel not implemented\n"); abort();
		case MenuRpcType_GetIsEntitledToLevel: fprintf(stderr, "[INSTANCE] MenuRpcType_GetIsEntitledToLevel not implemented\n"); abort();
		case MenuRpcType_SetIsEntitledToLevel: {
			struct SetIsEntitledToLevel entitlement = pkt_readSetIsEntitledToLevel(data);
			if(!String_eq(entitlement.levelId, room->selectedBeatmap.levelID))
				break;
			if(Counter128_set(&room->entitled, indexof(room->players, session), 1))
				break;
			if(entitlement.entitlementStatus != EntitlementsStatus_Ok)
				room->buzzkills.playersWithoutEntitlements[room->buzzkills.count++] = session->permissions.userId;
			if(Counter128_contains(room->entitled, room->playerSort)) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
				struct SetPlayersMissingEntitlementsToLevel r_missing;
				r_missing.base.syncTime = room_get_syncTime(room);
				r_missing.playersMissingEntitlements = room->buzzkills;
				SERIALIZE_MENURPC(&resp_end, SetPlayersMissingEntitlementsToLevel, r_missing);
				FOR_ALL_PLAYERS(room, id)
					instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
				refresh_button(ctx, room);
			}
			break;
		}
		case MenuRpcType_InvalidateLevelEntitlementStatuses: fprintf(stderr, "[INSTANCE] MenuRpcType_InvalidateLevelEntitlementStatuses not implemented\n"); abort();
		case MenuRpcType_SelectLevelPack: fprintf(stderr, "[INSTANCE] MenuRpcType_SelectLevelPack not implemented\n"); abort();
		case MenuRpcType_SetSelectedBeatmap: fprintf(stderr, "[INSTANCE] MenuRpcType_SetSelectedBeatmap not implemented\n"); abort();
		case MenuRpcType_GetSelectedBeatmap: fprintf(stderr, "[INSTANCE] MenuRpcType_GetSelectedBeatmap not implemented\n"); abort();
		case MenuRpcType_RecommendBeatmap: session->menu.recommendedBeatmap = pkt_readRecommendBeatmap(data).identifier;
		case MenuRpcType_ClearRecommendedBeatmap: {
			if(rpc.type == MenuRpcType_ClearRecommendedBeatmap) {
				pkt_readClearRecommendedBeatmap(data);
				session->menu.recommendedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};
			}
			if(session->permissions.hasRecommendBeatmapsPermission) {
				uint32_t select = ~0;
				switch(room->configuration.songSelectionMode) {
					case SongSelectionMode_Vote: {
						uint8_t counter[126], max = 0;
						FOR_ALL_PLAYERS(room, id) {
							counter[id] = 0;
							if(room->players[id].permissions.hasRecommendBeatmapsPermission && room->players[id].menu.recommendedBeatmap.beatmapCharacteristicSerializedName.length) {
								FOR_ALL_PLAYERS(room, cmp) {
									if(!room->players[cmp].permissions.hasRecommendBeatmapsPermission)
										continue;
									if(id == cmp || (String_eq(room->players[id].menu.recommendedBeatmap.levelID, room->players[cmp].menu.recommendedBeatmap.levelID) && String_eq(room->players[id].menu.recommendedBeatmap.beatmapCharacteristicSerializedName, room->players[cmp].menu.recommendedBeatmap.beatmapCharacteristicSerializedName) && (PER_PLAYER_DIFFICULTY || room->players[id].menu.recommendedBeatmap.difficulty == room->players[cmp].menu.recommendedBeatmap.difficulty))) {
										if(++counter[cmp] > max) {
											select = cmp;
											max = counter[cmp];
										}
										break;
									}
								}
							}
						}
						break;
					}
					case SongSelectionMode_Random:
					case SongSelectionMode_OwnerPicks: {
						FOR_ALL_PLAYERS(room, id) {
							if(room->players[id].permissions.isServerOwner) {
								select = id;
								break;
							}
						}
						break;
					}
					case SongSelectionMode_RandomPlayerPicks: {
						uint32_t validPlayerCount = 0, luckyFellow;
						mbedtls_ctr_drbg_random(net_get_ctr_drbg(&ctx->net), (uint8_t*)&luckyFellow, sizeof(luckyFellow));
						FOR_ALL_PLAYERS(room, id)
							validPlayerCount += room->players[id].permissions.hasRecommendBeatmapsPermission;
						if(validPlayerCount) {
							luckyFellow %= validPlayerCount;
							FOR_ALL_PLAYERS(room, id) {
								if(--validPlayerCount == luckyFellow) {
									select = id;
									break;
								}
							}
						}
					}
				}
				room->canStart = 0, room->buzzkills.count = 0;
				if(select != ~0 && room->players[select].menu.recommendedBeatmap.beatmapCharacteristicSerializedName.length) {
					Counter128_clear(&room->entitled);
					room->selectedBeatmap = room->players[select].menu.recommendedBeatmap;
					uint8_t resp[65536], *resp_end = resp;
					struct GetIsEntitledToLevel r_level;
					r_level.base.syncTime = room_get_syncTime(room);
					r_level.levelId = session->menu.recommendedBeatmap.levelID;
					pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
					SERIALIZE_MENURPC(&resp_end, GetIsEntitledToLevel, r_level);
					FOR_ALL_PLAYERS(room, id)
						instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
					refresh_countdown(ctx, room, 0);
				} else {
					room->entitled = room->playerSort;
					room->selectedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};
					refresh_button(ctx, room);
				}
			}
			break;
		}
		case MenuRpcType_GetRecommendedBeatmap: pkt_readGetRecommendedBeatmap(data); break;
		case MenuRpcType_SetSelectedGameplayModifiers: fprintf(stderr, "[INSTANCE] BAD TYPE: MenuRpcType_SetSelectedGameplayModifiers\n"); break;
		case MenuRpcType_GetSelectedGameplayModifiers: fprintf(stderr, "[INSTANCE] MenuRpcType_GetSelectedGameplayModifiers not implemented\n"); abort();
		case MenuRpcType_RecommendGameplayModifiers: {
			struct RecommendGameplayModifiers req = pkt_readRecommendGameplayModifiers(data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetSelectedGameplayModifiers r_modifiers;
			r_modifiers.base.syncTime = room_get_syncTime(room);
			if(session->permissions.hasRecommendGameplayModifiersPermission)
				session->menu.recommendedModifiers = req.gameplayModifiers;
			r_modifiers.gameplayModifiers = room->selectedModifiers;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
			SERIALIZE_MENURPC(&resp_end, SetSelectedGameplayModifiers, r_modifiers);
			instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_ClearRecommendedGameplayModifiers: memset(&session->menu.recommendedModifiers, 0, sizeof(session->menu.recommendedModifiers)); break;
		case MenuRpcType_GetRecommendedGameplayModifiers: pkt_readGetRecommendedGameplayModifiers(data); break;
		case MenuRpcType_LevelLoadError: fprintf(stderr, "[INSTANCE] MenuRpcType_LevelLoadError not implemented\n"); abort();
		case MenuRpcType_LevelLoadSuccess: fprintf(stderr, "[INSTANCE] MenuRpcType_LevelLoadSuccess not implemented\n"); abort();
		case MenuRpcType_StartLevel: fprintf(stderr, "[INSTANCE] MenuRpcType_StartLevel not implemented\n"); abort();
		case MenuRpcType_GetStartedLevel: {
			pkt_readGetStartedLevel(data);
			if(room->state != ServerState_Lobby) {
				fprintf(stderr, "[INSTANCE] MenuRpcType_GetStartedLevel not implemented\n"); abort();
			} else {
				uint8_t resp[65536], *resp_end = resp;
				struct SetIsStartButtonEnabled r_button;
				r_button.base.syncTime = room_get_syncTime(room);
				r_button.reason = CannotStartGameReason_NoSongSelected;
				pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
				SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_button);
				instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			}
			break;
		}
		case MenuRpcType_CancelLevelStart: fprintf(stderr, "[INSTANCE] MenuRpcType_CancelLevelStart not implemented\n"); abort();
		case MenuRpcType_GetMultiplayerGameState: {
			pkt_readGetMultiplayerGameState(data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetMultiplayerGameState r_state;
			r_state.base.syncTime = room_get_syncTime(room);
			if(room->state == ServerState_Lobby)
				r_state.lobbyState = MultiplayerGameState_Lobby;
			else
				r_state.lobbyState = MultiplayerGameState_Game;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
			SERIALIZE_MENURPC(&resp_end, SetMultiplayerGameState, r_state);
			instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetMultiplayerGameState: fprintf(stderr, "[INSTANCE] BAD TYPE: MenuRpcType_SetMultiplayerGameState\n"); break;
		case MenuRpcType_GetIsReady: pkt_readGetIsReady(data); break;
		case MenuRpcType_SetIsReady: {
			_Bool isReady = pkt_readSetIsReady(data).isReady;
			if(Counter128_set(&room->ready, indexof(room->players, session), isReady) != isReady)
				refresh_countdown(ctx, room, 0);
			break;
		}
		case MenuRpcType_SetStartGameTime: fprintf(stderr, "[INSTANCE] MenuRpcType_SetStartGameTime not implemented\n"); abort();
		case MenuRpcType_CancelStartGameTime: fprintf(stderr, "[INSTANCE] MenuRpcType_CancelStartGameTime not implemented\n"); abort();
		case MenuRpcType_GetIsInLobby: pkt_readGetIsInLobby(data); break;
		case MenuRpcType_SetIsInLobby: {
			_Bool inLobby = pkt_readSetIsInLobby(data).isBack;
			if(Counter128_set(&room->inLobby, indexof(room->players, session), inLobby) != inLobby)
				refresh_button(ctx, room);
			break;
		}
		case MenuRpcType_GetCountdownEndTime: {
			pkt_readGetCountdownEndTime(data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetIsStartButtonEnabled r_button;
			r_button.base.syncTime = room_get_syncTime(room);
			r_button.reason = CannotStartGameReason_NoSongSelected;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
			SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_button);
			instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetCountdownEndTime: fprintf(stderr, "[INSTANCE] BAD TYPE: MenuRpcType_SetCountdownEndTime\n"); break;
		case MenuRpcType_CancelCountdown: fprintf(stderr, "[INSTANCE] BAD TYPE: MenuRpcType_CancelCountdown\n"); break;
		case MenuRpcType_GetOwnedSongPacks: fprintf(stderr, "[INSTANCE] MenuRpcType_GetOwnedSongPacks not implemented\n"); abort();
		case MenuRpcType_SetOwnedSongPacks: session->menu.ownedSongPacks = pkt_readSetOwnedSongPacks(data).songPackMask; break;
		case MenuRpcType_RequestKickPlayer: fprintf(stderr, "[INSTANCE] MenuRpcType_RequestKickPlayer not implemented\n"); abort();
		case MenuRpcType_GetPermissionConfiguration:  {
			pkt_readGetPermissionConfiguration(data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetPermissionConfiguration r_permission;
			r_permission.base.syncTime = room_get_syncTime(room);
			r_permission.playersPermissionConfiguration.count = 0;
			FOR_ALL_PLAYERS(room, id)
				r_permission.playersPermissionConfiguration.playersPermission[r_permission.playersPermissionConfiguration.count++] = room->players[id].permissions;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(&resp_end, SetPermissionConfiguration, r_permission);
			instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetPermissionConfiguration: fprintf(stderr, "[INSTANCE] BAD TYPE: MenuRpcType_SetPermissionConfiguration\n"); break;
		case MenuRpcType_GetIsStartButtonEnabled: fprintf(stderr, "[INSTANCE] MenuRpcType_GetIsStartButtonEnabled not implemented\n"); abort();
		case MenuRpcType_SetIsStartButtonEnabled: fprintf(stderr, "[INSTANCE] BAD TYPE: MenuRpcType_SetIsStartButtonEnabled\n"); break;
		default: fprintf(stderr, "[INSTANCE] BAD MENU RPC TYPE\n");
	}
}

static void handle_GameplayRpc(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct GameplayRpcHeader rpc = pkt_readGameplayRpcHeader(data);
	switch(rpc.type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: fprintf(stderr, "[INSTANCE] GameplayRpcType_SetGameplaySceneSyncFinish not implemented\n"); abort();
		case GameplayRpcType_SetGameplaySceneReady: {
			session->game.settings = pkt_readSetGameplaySceneReady(data).playerSpecificSettingsNetSerializable;
			if(room->state != ServerState_SceneLoading)
				break;
			if(!Counter128_set(&room->sceneLoaded, indexof(room->players, session), 1))
				if(Counter128_contains(room->sceneLoaded, room->playerSort))
					load_song(ctx, room);
			break;
		}
		case GameplayRpcType_GetGameplaySceneReady: fprintf(stderr, "[INSTANCE] GameplayRpcType_GetGameplaySceneReady not implemented\n"); abort();
		case GameplayRpcType_SetActivePlayerFailedToConnect: fprintf(stderr, "[INSTANCE] GameplayRpcType_SetActivePlayerFailedToConnect not implemented\n"); abort();
		case GameplayRpcType_SetGameplaySongReady: {
			pkt_readSetGameplaySongReady(data);
			if(room->state != ServerState_SongLoading)
				break;
			if(!Counter128_set(&room->songLoaded, indexof(room->players, session), 1))
				if(Counter128_contains(room->songLoaded, room->playerSort))
					start_game(ctx, room);
			break;
		}
		case GameplayRpcType_GetGameplaySongReady: fprintf(stderr, "[INSTANCE] GameplayRpcType_GetGameplaySongReady not implemented\n"); abort();
		case GameplayRpcType_SetSongStartTime: fprintf(stderr, "[INSTANCE] GameplayRpcType_SetSongStartTime not implemented\n"); abort();
		case GameplayRpcType_NoteCut: pkt_readNoteCut(data); break;
		case GameplayRpcType_NoteMissed: pkt_readNoteMissed(data); break;
		case GameplayRpcType_LevelFinished: {
			pkt_readLevelFinished(data);
			if(room->state == ServerState_Lobby)
				break;
			if(!Counter128_set(&room->levelFinished, indexof(room->players, session), 1)) {
				if(Counter128_contains(room->levelFinished, room->playerSort)) {
					fprintf(stderr, "TODO: wait on results screen\n");
					/*struct ReturnToMenu r_menu;
					r_menu.base.syncTime = room_get_syncTime(room);*/
					room->state = ServerState_Lobby;
				}
			}
			break;
		}
		case GameplayRpcType_ReturnToMenu: fprintf(stderr, "[INSTANCE] GameplayRpcType_ReturnToMenu not implemented\n"); abort();
		case GameplayRpcType_RequestReturnToMenu: fprintf(stderr, "[INSTANCE] GameplayRpcType_RequestReturnToMenu not implemented\n"); abort();
		default: fprintf(stderr, "[INSTANCE] BAD GAMEPLAY RPC TYPE\n");
	}
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

static void update_session_state(struct Context *ctx, struct Room *room, struct InstanceSession *session, struct PlayerStateHash state) {
	_Bool isSpectating = !PlayerStateHash_contains(state, "wants_to_play_next_level");
	_Bool allPlayersSpectating = Counter128_contains(room->spectating, room->playerSort);
	if(Counter128_set(&room->spectating, indexof(room->players, session), isSpectating) != isSpectating)
		if(Counter128_contains(room->spectating, room->playerSort) != allPlayersSpectating)
			refresh_button(ctx, room);
}

static void handle_PlayerIdentity(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct PlayerIdentity identity = pkt_readPlayerIdentity(data);
	update_session_state(ctx, room, session, identity.playerState);
	session->avatar = identity.playerAvatar;
	fprintf(stderr, "TODO: send to other players\n");
	if(session->clientState != ClientState_accepted)
		return;
	session->clientState = ClientState_connected;

	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	uint8_t *routing_end = resp_end;
	struct PlayerConnected r_connected;
	r_connected.remoteConnectionId = indexof(room->players, session) + 1;
	r_connected.userId = session->permissions.userId;
	r_connected.userName = session->userName;
	r_connected.isConnectionOwner = 0;
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_PlayerConnected)
		pkt_writePlayerConnected(&resp_end, r_connected);
	FOR_EXCLUDING_PLAYER(room, session, id)
		instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = routing_end;
	struct PlayerSortOrderUpdate r_sort;
	r_sort.userId = session->permissions.userId;
	r_sort.sortIndex = indexof(room->players, session);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_PlayerSortOrderUpdate)
		pkt_writePlayerSortOrderUpdate(&resp_end, r_sort);
	FOR_ALL_PLAYERS(room, id)
		instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct RemoteProcedureCall base;
	base.syncTime = room_get_syncTime(room);
	SERIALIZE_MENURPC(&resp_end, GetRecommendedBeatmap, (struct GetRecommendedBeatmap){.base = base});
	SERIALIZE_MENURPC(&resp_end, GetRecommendedGameplayModifiers, (struct GetRecommendedGameplayModifiers){.base = base});
	SERIALIZE_MENURPC(&resp_end, GetIsReady, (struct GetIsReady){.base = base});
	SERIALIZE_MENURPC(&resp_end, GetIsInLobby, (struct GetIsInLobby){.base = base});
	instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
}

static void handle_MultiplayerSession(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MultiplayerSessionMessageHeader message = pkt_readMultiplayerSessionMessageHeader(data);
	switch(message.type) {
		case MultiplayerSessionMessageType_MenuRpc: handle_MenuRpc(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_GameplayRpc: handle_GameplayRpc(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: pkt_readNodePoseSyncState(data); break;
		case MultiplayerSessionMessageType_ScoreSyncState: pkt_readScoreSyncState(data); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_readNodePoseSyncStateDelta(data); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_readScoreSyncStateDelta(data); break;
		case MultiplayerSessionMessageType_MpCore: {
			struct MpCore mpHeader = pkt_readMpCore(data);
			if(mpHeader.packetType.length == 15 && memcmp(mpHeader.packetType.data, "MpBeatmapPacket", 15) == 0) {
				fprintf(stderr, "[INSTANCE] 'MpBeatmapPacket' not implemented\n");
				abort();
			} else {
				fprintf(stderr, "[INSTANCE] BAD MPCORE MESSAGE TYPE: '%.*s'\n", mpHeader.packetType.length, mpHeader.packetType.data);
			}
		}
		default: fprintf(stderr, "[INSTANCE] BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
	}
}

static void handle_Unreliable(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end) {
	struct RoutingHeader routing = pkt_readRoutingHeader(data);
	if(routing.connectionId) {
		if(routing.connectionId == 127) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Unreliable, 0, 0});
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){indexof(room->players, session) + 1, 127, 0});
			pkt_writeUint8Array(&resp_end, *data, end - *data);
			FOR_EXCLUDING_PLAYER(room, session, id)
				net_send_internal(&ctx->net, &room->players[id].net, resp, resp_end - resp, 1);
		} else {
			fprintf(stderr, "Routed packets not implemented\n");
			abort();
		}
	}
	while(*data < end) {
		struct SerializeHeader serial = pkt_readSerializeHeader(data);
		const uint8_t *sub = (*data)--;
		*data += serial.length;
		switch(serial.type) {
			case InternalMessageType_SyncTime: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_SyncTime not implemented\n"); abort();
			case InternalMessageType_PlayerConnected: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerConnected not implemented\n"); abort();
			case InternalMessageType_PlayerIdentity: fprintf(stderr, "[INSTANCE] BAD TYPE: InternalMessageType_PlayerIdentity\n"); break;
			case InternalMessageType_PlayerLatencyUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerLatencyUpdate not implemented\n"); abort();
			case InternalMessageType_PlayerDisconnected: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerDisconnected not implemented\n"); abort();
			case InternalMessageType_PlayerSortOrderUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerSortOrderUpdate not implemented\n"); abort();
			case InternalMessageType_Party: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_Party not implemented\n"); abort();
			case InternalMessageType_MultiplayerSession: handle_MultiplayerSession(ctx, room, session, &sub); break;
			case InternalMessageType_KickPlayer: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_KickPlayer not implemented\n"); abort();
			case InternalMessageType_PlayerStateUpdate: fprintf(stderr, "[INSTANCE] BAD TYPE: InternalMessageType_PlayerStateUpdate\n"); break;
			case InternalMessageType_PlayerAvatarUpdate: fprintf(stderr, "[INSTANCE] UNRELIABLE : InternalMessageType_PlayerAvatarUpdate not implemented\n"); abort();
			default: fprintf(stderr, "[INSTANCE] UNRELIABLE : BAD INTERNAL MESSAGE TYPE\n");
		}
		if(sub != *data) {
			fprintf(stderr, "UNRELIABLE : BAD INTERNAL MESSAGE LENGTH (expected %u, read %zu)\n", serial.length, sub - (*data - serial.length));
			if(sub < *data) {
				fprintf(stderr, "\t");
				for(const uint8_t *it = *data - serial.length; it < *data; ++it)
					fprintf(stderr, "%02hhx", *it);
				fprintf(stderr, "\n\t");
				for(const uint8_t *it = *data - serial.length; it < sub; ++it)
					fprintf(stderr, "  ");
				fprintf(stderr, "^ extra data starts here");
			}
			fprintf(stderr, "\n");
		}
	}
}

static void process_Channeled(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId) {
	struct RoutingHeader routing = pkt_readRoutingHeader(data);
	if(routing.connectionId) {
		if(routing.connectionId == 127) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){indexof(room->players, session) + 1, 127, 0});
			pkt_writeUint8Array(&resp_end, *data, end - *data);
			FOR_EXCLUDING_PLAYER(room, session, id)
				instance_send_channeled(&room->players[id].channels, resp, resp_end - resp, channelId);
		} else {
			fprintf(stderr, "Routed packets not implemented\n");
			abort();
		}
	}
	while(*data < end) {
		struct SerializeHeader serial = pkt_readSerializeHeader(data);
		const uint8_t *sub = (*data)--;
		*data += serial.length;
		switch(serial.type) {
			case InternalMessageType_SyncTime: fprintf(stderr, "[INSTANCE] InternalMessageType_SyncTime not implemented\n"); break;
			case InternalMessageType_PlayerConnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerConnected not implemented\n"); break;
			case InternalMessageType_PlayerIdentity: handle_PlayerIdentity(ctx, room, session, &sub); break;
			case InternalMessageType_PlayerLatencyUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerLatencyUpdate not implemented\n"); break;
			case InternalMessageType_PlayerDisconnected: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerDisconnected not implemented\n"); break;
			case InternalMessageType_PlayerSortOrderUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerSortOrderUpdate not implemented\n"); break;
			case InternalMessageType_Party: fprintf(stderr, "[INSTANCE] InternalMessageType_Party not implemented\n"); break;
			case InternalMessageType_MultiplayerSession: handle_MultiplayerSession(ctx, room, session, &sub); break;
			case InternalMessageType_KickPlayer: fprintf(stderr, "[INSTANCE] InternalMessageType_KickPlayer not implemented\n"); break;
			case InternalMessageType_PlayerStateUpdate: update_session_state(ctx, room, session, pkt_readPlayerStateUpdate(&sub).playerState); break;
			case InternalMessageType_PlayerAvatarUpdate: fprintf(stderr, "[INSTANCE] InternalMessageType_PlayerAvatarUpdate not implemented\n"); break;
			default: fprintf(stderr, "[INSTANCE] BAD INTERNAL MESSAGE TYPE\n");
		}
		if(sub != *data) {
			fprintf(stderr, "BAD INTERNAL MESSAGE LENGTH (expected %u, read %zu)\n", serial.length, sub - (*data - serial.length));
			if(sub < *data) {
				fprintf(stderr, "\t");
				for(const uint8_t *it = *data - serial.length; it < *data; ++it)
					fprintf(stderr, "%02hhx", *it);
				fprintf(stderr, "\n\t");
				for(const uint8_t *it = *data - serial.length; it < sub; ++it)
					fprintf(stderr, "  ");
				fprintf(stderr, "^ extra data starts here");
			}
			fprintf(stderr, "\n");
		}
	}
}

static void handle_ConnectRequest(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct ConnectRequest req = pkt_readConnectRequest(data);
	if(!(String_eq(req.secret, session->secret) && String_eq(req.userId, session->permissions.userId)))
		return;
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_ConnectAccept, 0, 0});
	pkt_writeConnectAccept(&resp_end, (struct ConnectAccept){
		.connectId = req.connectId,
		.connectNum = 0,
		.reusedPeer = 0,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);

	if(session->clientState != ClientState_disconnected)
		return;
	session->clientState = ClientState_accepted;

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct SyncTime r_sync;
	r_sync.syncTime = room_get_syncTime(room);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_SyncTime)
		pkt_writeSyncTime(&resp_end, r_sync);
	instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	struct PlayerIdentity r_identity;
	r_identity.playerState.bloomFilter.d0 = 288266110296588352;
	r_identity.playerState.bloomFilter.d1 = 576531121051926529;
	memset(&r_identity.playerAvatar, 0, sizeof(r_identity.playerAvatar));
	r_identity.random.length = 32;
	memcpy(r_identity.random.data, NetKeypair_get_random(&room->keys), 32);
	r_identity.publicEncryptionKey.length = sizeof(r_identity.publicEncryptionKey.data);
	NetKeypair_write_key(&room->keys, &ctx->net, r_identity.publicEncryptionKey.data, &r_identity.publicEncryptionKey.length);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_PlayerIdentity)
		pkt_writePlayerIdentity(&resp_end, r_identity);
	instance_send_channeled(&session->channels, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	fprintf(stderr, "TODO: send PlayerConnected to other players\n");
}
static void handle_Disconnect(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	pkt_readDisconnect(data);
	room_disconnect(NULL, room, session);
}

static void handle_packet(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t *pkt, uint32_t len) {
	const uint8_t *data = pkt, *end = &pkt[len];
	struct NetPacketHeader header = pkt_readNetPacketHeader(&data);
	if(session->clientState == ClientState_disconnected && header.property != PacketProperty_ConnectRequest)
		return;
	if(header.isFragmented && header.property != PacketProperty_Channeled) {
		fprintf(stderr, "MALFORMED HEADER\n");
		return;
	}
	switch(header.property) {
		case PacketProperty_Unreliable: handle_Unreliable(ctx, room, session, &data, end); break;
		case PacketProperty_Channeled: handle_Channeled((ChanneledHandler)process_Channeled, &ctx->net, &session->net, &session->channels, ctx, room, session, &data, end, header.isFragmented); break;
		case PacketProperty_Ack: handle_Ack(&session->channels, &data); break;
		case PacketProperty_Ping: handle_Ping(&ctx->net, &session->net, &session->pong, &data); break;
		case PacketProperty_Pong: fprintf(stderr, "[INSTANCE] PacketProperty_Pong not implemented\n"); break;
		case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, room, session, &data); break;
		case PacketProperty_ConnectAccept: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
		case PacketProperty_Disconnect: handle_Disconnect(ctx, room, session, &data); break;
		case PacketProperty_UnconnectedMessage: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
		case PacketProperty_MtuCheck: handle_MtuCheck(&ctx->net, &session->net, &data); break;
		case PacketProperty_MtuOk: fprintf(stderr, "[INSTANCE] PacketProperty_MtuOk not implemented\n"); break;
		case PacketProperty_Broadcast: fprintf(stderr, "[INSTANCE] PacketProperty_Broadcast not implemented\n"); break;
		case PacketProperty_Merged: {
			for(uint16_t sublen; data < end; data += sublen) {
				sublen = pkt_readUint16(&data);
				const uint8_t *subdata = data;
				handle_packet(ctx, room, session, subdata, sublen);
			}
			break;
		}
		case PacketProperty_ShutdownOk: fprintf(stderr, "[INSTANCE] PacketProperty_ShutdownOk not implemented\n"); break;
		case PacketProperty_PeerNotFound: fprintf(stderr, "[INSTANCE] PacketProperty_PeerNotFound not implemented\n"); break;
		case PacketProperty_InvalidProtocol: fprintf(stderr, "[INSTANCE] PacketProperty_InvalidProtocol not implemented\n"); break;
		case PacketProperty_NatMessage: fprintf(stderr, "[INSTANCE] PacketProperty_NatMessage not implemented\n"); break;
		case PacketProperty_Empty: fprintf(stderr, "[INSTANCE] PacketProperty_Empty not implemented\n"); break;
		default: fprintf(stderr, "[INSTANCE] BAD PACKET PROPERTY\n");
	}
	if(data != end)
		fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %u, read %zu)\n", len, data - pkt);
}

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
instance_handler(struct Context *ctx) {
	fprintf(stderr, "[INSTANCE] Started\n");
	uint8_t buf[262144];
	memset(buf, 0, sizeof(buf));
	uint32_t len;
	struct Room *room;
	struct InstanceSession *session;
	const uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt, (void**)&room))) { // TODO: close instances with zero sessions
		#ifdef PACKET_LOGGING_FUNCS
		{
			const uint8_t *read = pkt;
			struct NetPacketHeader header = pkt_readNetPacketHeader(&read);
			if(!header.isFragmented) {
				fprintf(stderr, "recieve[%s]:\n", reflect(PacketProperty, header.property));
				debug_logPacket(read, &pkt[len], header);
			}
		}
		#endif
		handle_packet(ctx, room, session, pkt, len); // TODO: needs mutex
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE instance_threads[THREAD_COUNT];
#else
static pthread_t instance_threads[THREAD_COUNT];
#endif
static struct Context contexts[THREAD_COUNT];
// static uint32_t instance_count = 1; // ServerCode 0 ("") is not valid
static const char *instance_domain = NULL, *instance_domainIPv4 = NULL;
_Bool instance_init(const char *domain, const char *domainIPv4) {
	instance_domain = domain;
	instance_domainIPv4 = domainIPv4;
	memset(instance_threads, 0, sizeof(instance_threads));
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(net_init(&contexts[i].net, 0)) {
			fprintf(stderr, "net_init() failed\n");
			return 1;
		}
		contexts[i].net.user = &contexts[i];
		contexts[i].net.onResolve = instance_onResolve;
		contexts[i].net.onResend = instance_onResend;
		Counter128_clear(&contexts[i].TEMPglobalRoom.playerSort);

		#ifdef WINDOWS
		instance_threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)instance_handler, &contexts[i], 0, NULL);
		#else
		if(pthread_create(&instance_threads[i], NULL, (void*(*)(void*))&instance_handler, &contexts[i]))
			instance_threads[i] = 0;
		#endif
		if(!instance_threads[i]) {
			fprintf(stderr, "[INSTANCE] Instance thread creation failed\n");
			return 1;
		}
	}
	return index_init();
}

void instance_cleanup() {
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(instance_threads[i]) {
			net_stop(&contexts[i].net);
			fprintf(stderr, "[INSTANCE] Stopping #%u\n", i);
			#ifdef WINDOWS
			WaitForSingleObject(instance_threads[i], INFINITE);
			#else
			pthread_join(instance_threads[i], NULL);
			#endif
			instance_threads[i] = 0;
			FOR_ALL_PLAYERS(&contexts[i].TEMPglobalRoom, id)
				room_disconnect(NULL, &contexts[i].TEMPglobalRoom, &contexts[i].TEMPglobalRoom.players[id]);
			net_cleanup(&contexts[i].net);
		}
	}
	index_cleanup();
}

_Bool instance_get_isopen(ServerCode code, struct String *managerId_out, struct GameplayServerConfiguration *configuration) {
	if(code == StringToServerCode("INDEX", 5))
		return index_get_isopen(managerId_out, configuration);
	if(code == StringToServerCode("HELLO", 5)) { // TODO: temporary hack
		*managerId_out = contexts[0].TEMPglobalRoom.managerId;
		configuration->maxPlayerCount = contexts[0].TEMPglobalRoom.configuration.maxPlayerCount;
		return 1;
	}
	return 0;
}

struct NetSession *instance_resolve_session(ServerCode code, struct SS addr, struct String secret, struct String userId, struct String userName) {
	if(code == StringToServerCode("INDEX", 5))
		return index_create_session(addr, secret, userId, userName);
	if(code != StringToServerCode("HELLO", 5))
		return NULL;
	struct Context *ctx = &contexts[0];
	struct Room *room = &ctx->TEMPglobalRoom;
	struct InstanceSession *session = NULL;
	FOR_ALL_PLAYERS(room, id) {
		if(addrs_are_equal(&addr, NetSession_get_addr(&room->players[id].net))) {
			session = &room->players[id];
			room_disconnect(ctx, room, session);
			break;
		}
	}
	if(!session) {
		struct Counter128 tmp = room->playerSort;
		uint32_t id = 0;
		if((!Counter128_set_next(&tmp, &id, 1)) || id >= room->configuration.maxPlayerCount) {
			fprintf(stderr, "ROOM FULL\n");
			return NULL;
		}
		session = &room->players[id];
		net_session_init(&ctx->net, &session->net, addr);
		room->playerSort = tmp;
	}
	session->clientState = ClientState_disconnected;
	session->pong.sequence = 0;
	instance_channels_init(&session->channels);

	session->secret = secret;
	session->userName = userName;
	session->permissions.userId = userId;
	session->permissions.isServerOwner = String_eq(userId, room->managerId);
	session->permissions.hasRecommendBeatmapsPermission = (room->configuration.songSelectionMode != SongSelectionMode_Random) && (session->permissions.isServerOwner || room->configuration.songSelectionMode != SongSelectionMode_OwnerPicks);
	session->permissions.hasRecommendGameplayModifiersPermission = (room->configuration.gameplayServerControlSettings == GameplayServerControlSettings_AllowModifierSelection || room->configuration.gameplayServerControlSettings == GameplayServerControlSettings_All);
	session->permissions.hasKickVotePermission = session->permissions.isServerOwner;
	session->permissions.hasInvitePermission = (room->configuration.invitePolicy == InvitePolicy_AnyoneCanInvite) || (session->permissions.isServerOwner && room->configuration.invitePolicy == InvitePolicy_OnlyConnectionOwnerCanInvite);
	session->state.bloomFilter = (struct BitMask128){0, 0};
	session->menu.ownedSongPacks.bloomFilter.d0 = 0;
	session->menu.ownedSongPacks.bloomFilter.d1 = 0;
	memset(&session->menu.recommendedModifiers, 0, sizeof(session->menu.recommendedModifiers));
	session->menu.recommendedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	fprintf(stderr, "[INSTANCE] connect %s\n", addrstr);
	fprintf(stderr, "[INSTANCE] player bits: ");
	for(uint32_t i = 0; i < lengthof(room->playerSort.bits); ++i)
		for(uint32_t b = 0; b < sizeof(*room->playerSort.bits) * 8; ++b)
			fprintf(stderr, "%u", (room->playerSort.bits[i] >> b) & 1);
	fprintf(stderr, "\n");

	/*uint8_t punch[128], *punch_end = punch;
	pkt_writeNetPacketHeader(&punch_end, (struct NetPacketHeader){PacketProperty_Ping, 0, 0});
	pkt_writePing(&punch_end, (struct Ping){0});
	net_send_internal(&ctx->net, &session->net, punch, punch_end - punch, 1);*/
	return &session->net;
}

struct NetSession *instance_open(ServerCode *out, struct String managerId, struct GameplayServerConfiguration *configuration, struct SS addr, struct String secret, struct String userId, struct String userName) {
	net_keypair_init(&contexts[0].net, &contexts[0].TEMPglobalRoom.keys);
	contexts[0].TEMPglobalRoom.syncBase = 0;
	contexts[0].TEMPglobalRoom.syncBase = room_get_syncTime(&contexts[0].TEMPglobalRoom);
	contexts[0].TEMPglobalRoom.countdownEnd = 0;
	contexts[0].TEMPglobalRoom.state = ServerState_Lobby;
	Counter128_clear(&contexts[0].TEMPglobalRoom.inLobby);
	Counter128_clear(&contexts[0].TEMPglobalRoom.ready);
	Counter128_clear(&contexts[0].TEMPglobalRoom.entitled);
	Counter128_clear(&contexts[0].TEMPglobalRoom.spectating);
	contexts[0].TEMPglobalRoom.selectedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};
	memset(&contexts[0].TEMPglobalRoom.selectedModifiers, 0, sizeof(contexts->TEMPglobalRoom.selectedModifiers));
	contexts[0].TEMPglobalRoom.buzzkills.count = 0;
	contexts[0].TEMPglobalRoom.canStart = 0;
	contexts[0].TEMPglobalRoom.configuration = *configuration;
	Counter128_clear(&contexts[0].TEMPglobalRoom.playerSort);
	contexts[0].TEMPglobalRoom.players = malloc(contexts[0].TEMPglobalRoom.configuration.maxPlayerCount * sizeof(*contexts->TEMPglobalRoom.players));
	for(uint32_t i = 0; i < contexts[0].TEMPglobalRoom.configuration.maxPlayerCount; ++i) {
		contexts[0].TEMPglobalRoom.players[i].clientState = ClientState_disconnected;
		contexts[0].TEMPglobalRoom.players[i].channels.incomingFragmentsList = NULL;
	}
	contexts[0].TEMPglobalRoom.managerId = managerId;
	*out = StringToServerCode("HELLO", 5); // TODO: temporary hack
	return instance_resolve_session(*out, addr, secret, userId, userName);
}

struct NetContext *instance_get_net(ServerCode code) {
	if(code == StringToServerCode("INDEX", 5))
		return index_get_net();
	return (code == StringToServerCode("HELLO", 5)) ? &contexts[0].net : NULL;
}

struct IPEndPoint instance_get_address(ServerCode code, _Bool ipv4) {
	struct IPEndPoint out;
	out.address.length = 0;
	out.port = 0;
	if(code != StringToServerCode("INDEX", 5) && code != StringToServerCode("HELLO", 5)) // TODO: temporary hack
		return out;
	if(ipv4)
		out.address.length = sprintf(out.address.data, "%s", instance_domainIPv4);
	else
		out.address.length = sprintf(out.address.data, "%s", instance_domain);
	if(code == StringToServerCode("INDEX", 5)) {
		out.port = index_get_port();
	} else {
		struct SS addr = {sizeof(struct sockaddr_storage)};
		getsockname(net_get_sockfd(&contexts[0].net), &addr.sa, &addr.len);
		switch(addr.ss.ss_family) {
			case AF_INET: out.port = htons(addr.in.sin_port); break;
			case AF_INET6: out.port = htons(addr.in6.sin6_port); break;
			default:;
		}
	}
	return out;
}
