// #define THREAD_COUNT 256
#define THREAD_COUNT 1

#include "../enum_reflection.h"
#include "instance.h"
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

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define indexof(a, e) (((e) - (a)) / sizeof(*(a)))
#define String_eq(a, b) ((a).length == (b).length && memcmp((a).data, (b).data, (b).length) == 0)

#define SERIALIZE_MENURPC(pkt, dtype, data) { \
	SERIALIZE_CUSTOM(pkt, InternalMessageType_MultiplayerSession) { \
		pkt_writeMultiplayerSessionMessageHeader(pkt, (struct MultiplayerSessionMessageHeader){ \
			.type = MultiplayerSessionMessageType_MenuRpc, \
		}); \
		pkt_writeMenuRpcHeader(pkt, (struct MenuRpcHeader){ \
			.type = MenuRpcType_##dtype, \
		}); \
		pkt_write##dtype(pkt, data); \
	} \
}

#define FOR_ALL_PLAYERS(room, id) \
	for(uint32_t i = 0, (id) = (room)->playerSort[0]; i < (room)->playerCount; (id) = (room)->playerSort[++i]) \

#define FOR_EXCLUDING_PLAYER(room, session, id) \
	FOR_ALL_PLAYERS(room, id) \
		if(&(room)->players[id] != (session))

struct InstancePacket {
	uint16_t len;
	_Bool isFragmented;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct InstanceResendPacket {
	uint32_t timeStamp;
	uint16_t len;
	uint8_t data[NET_MAX_PKT_SIZE];
};
struct ReliableChannel {
	struct Ack ack;
	uint16_t localSeqence, remoteSequence;
	uint16_t localWindowStart, remoteWindowStart;
	struct InstanceResendPacket resend[NET_WINDOW_SIZE];
};
struct ReliableUnorderedChannel {
	struct ReliableChannel base;
	_Bool earlyReceived[NET_WINDOW_SIZE];
};
struct ReliableOrderedChannel {
	struct ReliableChannel base;
	struct InstancePacket receivedPackets[NET_WINDOW_SIZE];
};
struct SequencedChannel {
	struct Ack ack;
	uint16_t localSeqence;
	struct InstanceResendPacket resend;
};
typedef uint8_t ClientState;
enum ClientState {
	ClientState_disconnected,
	ClientState_accepted,
	ClientState_connected,
};
struct IncomingFragments {
	struct IncomingFragments *next;
	uint16_t fragmentId;
	DeliveryMethod channelId;
	uint16_t count, total;
	uint32_t size;
	struct InstancePacket fragments[];
};
struct InstanceSession {
	struct NetSession net;
	struct InstanceSession *next;
	ClientState clientState;
	struct String userName;
	struct PlayerLobbyPermissionConfigurationNetSerializable permissions;
	struct PlayerStateHash state;
	struct MultiplayerAvatarData avatar;
	struct {
		_Bool inLobby, isReady, entitlementCounted, isSpectating;
		struct SongPackMask ownedSongPacks;
		struct GameplayModifiers recommendedModifiers;
		struct BeatmapIdentifierNetSerializable recommendedBeatmap;
	} menu;
	struct Pong pong;
	struct ReliableUnorderedChannel ruChannel;
	struct ReliableOrderedChannel roChannel;
	struct SequencedChannel rsChannel;
	struct IncomingFragments *incomingFragmentsList;
};

struct Room {
	struct NetKeypair keys;
	struct GameplayServerConfiguration configuration;
	struct String managerId;
	float syncBase;
	MultiplayerGameState state;
	uint8_t inLobbyCount, readyCount, entitledCount, spectatingCount;
	struct BeatmapIdentifierNetSerializable selectedBeatmap;
	struct PlayersMissingEntitlementsNetSerializable buzzkills;
	uint8_t playerCount, *playerSort;
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

static void instance_send_channeled(struct NetContext *ctx, struct InstanceSession *session, uint8_t *buf, uint32_t len, DeliveryMethod method) {
	if(method != DeliveryMethod_ReliableOrdered) {
		fprintf(stderr, "instance_send_channeled(DeliveryMethod_%s) not implemented\n", reflect(DeliveryMethod, method));
		abort();
	}
	struct ReliableChannel *channel = &session->roChannel.base;
	if(RelativeSequenceNumber(channel->localSeqence, channel->localWindowStart) >= NET_WINDOW_SIZE) {
		fprintf(stderr, "Resend overflow buffer not implemented\n");
		abort();
	}
	channel->resend[channel->localSeqence % NET_WINDOW_SIZE].timeStamp = net_time() - NET_RESEND_DELAY;
	uint8_t *pkt = channel->resend[channel->localSeqence % NET_WINDOW_SIZE].data, *pkt_end = pkt;
	pkt_writeNetPacketHeader(&pkt_end, (struct NetPacketHeader){PacketProperty_Channeled, 0, 0});
	pkt_writeChanneled(&pkt_end, (struct Channeled){
		.sequence = channel->localSeqence,
		.channelId = method,
	});
	if(&pkt_end[len] > &pkt[lengthof(channel->resend->data)]) {
		fprintf(stderr, "Fragmenting not implemented\n");
		abort();
	}
	pkt_writeUint8Array(&pkt_end, buf, len);
	channel->resend[channel->localSeqence % NET_WINDOW_SIZE].len = pkt_end - pkt;
	channel->localSeqence = (channel->localSeqence + 1) % NET_MAX_SEQUENCE;
}

static void refresh_button(struct Context *ctx, struct Room *room) {
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
	struct SetIsStartButtonEnabled r_button;
	r_button.base.syncTime = room_get_syncTime(room);
	if(!room->inLobbyCount)
		r_button.reason = CannotStartGameReason_AllPlayersNotInLobby;
	if(!room->selectedBeatmap.beatmapCharacteristicSerializedName.length)
		r_button.reason = CannotStartGameReason_NoSongSelected;
	else if(room->buzzkills.count)
		r_button.reason = CannotStartGameReason_DoNotOwnSong;
	else if(room->spectatingCount == room->playerCount)
		r_button.reason = CannotStartGameReason_AllPlayersSpectating;
	else
		r_button.reason = CannotStartGameReason_None;
	SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_button);
	FOR_ALL_PLAYERS(room, id)
		instance_send_channeled(&ctx->net, &room->players[id], resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
}

static void swap(uint8_t *a, uint8_t *b) {
	uint8_t c = *a;
	*a = *b, *b = c;
}

static void room_disconnect(struct Context *ctx, struct Room *room, uint32_t sort) {
	struct InstanceSession *session = &room->players[room->playerSort[sort]];
	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(NetSession_get_addr(&session->net), addrstr);
	fprintf(stderr, "[INSTANCE] %sconnect %s\n", ctx ? "re" : "dis", addrstr);
	if(ctx)
		net_session_reset(&ctx->net, &session->net);
	else
		net_session_free(&session->net);
	session->clientState = ClientState_disconnected;
	while(session->incomingFragmentsList) {
		struct IncomingFragments *e = session->incomingFragmentsList;
		session->incomingFragmentsList = session->incomingFragmentsList->next;
		free(e);
	}
	room->inLobbyCount -= session->menu.inLobby;
	room->readyCount -= session->menu.isReady;
	room->spectatingCount -= session->menu.isSpectating;
	swap(&room->playerSort[sort], &room->playerSort[--room->playerCount]);
	if(session->menu.entitlementCounted) {
		session->menu.entitlementCounted = 0;
		fprintf(stderr, "TODO: recount entitlement for selectedBeatmap\n");
	}
	fprintf(stderr, "[INSTANCE] player count: %hhu / %d\n", room->playerCount, room->configuration.maxPlayerCount);
	refresh_button(ctx, room);
}

static void try_resend(struct Context *ctx, struct InstanceSession *session, struct InstanceResendPacket *p, uint32_t currentTime) {
	if(p->len == 0 || currentTime - p->timeStamp < NET_RESEND_DELAY)
		return;
	net_queue_merged(&ctx->net, &session->net, p->data, p->len);
	while(currentTime - p->timeStamp >= NET_RESEND_DELAY)
		p->timeStamp += NET_RESEND_DELAY;
}

static void flush_ack(struct Context *ctx, struct InstanceSession *session, struct Ack *ack) {
	for(uint_fast8_t i = 0; i < lengthof(ack->data); ++i) {
		if(ack->data[i]) {
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ack, 0, 0});
			pkt_writeAck(&resp_end, *ack);
			net_queue_merged(&ctx->net, &session->net, resp, resp_end - resp);
			memset(ack->data, 0, sizeof(ack->data));
			return;
		}
	}
}

static void instance_onResend(struct Context *ctx, uint32_t currentTime, uint32_t *nextTick) { // TODO: needs mutex
	struct Room *room = &ctx->TEMPglobalRoom;
	for(uint32_t i = 0; i < room->playerCount;) {
		struct InstanceSession *session = &room->players[room->playerSort[i]];
		uint32_t kickTime = NetSession_get_lastKeepAlive(&session->net) + 180000;
		if(currentTime > kickTime) {
			room_disconnect(NULL, room, i);
		} else {
			if(kickTime < *nextTick)
				*nextTick = kickTime;
			flush_ack(ctx, session, &session->ruChannel.base.ack);
			flush_ack(ctx, session, &session->roChannel.base.ack);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(ctx, session, &session->ruChannel.base.resend[i], currentTime);
			for(uint_fast8_t i = 0; i < 64; ++i)
				try_resend(ctx, session, &session->roChannel.base.resend[i], currentTime);
			try_resend(ctx, session, &session->rsChannel.resend, currentTime);
			net_flush_merged(&ctx->net, &session->net);
			++i;
		}
	}
}

static void handle_MenuRpc(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(data);
	switch(rpc.type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: fprintf(stderr, "[INSTANCE] MenuRpcType_SetPlayersMissingEntitlementsToLevel not implemented\n"); abort();
		case MenuRpcType_GetIsEntitledToLevel: fprintf(stderr, "[INSTANCE] MenuRpcType_GetIsEntitledToLevel not implemented\n"); abort();
		case MenuRpcType_SetIsEntitledToLevel: {
			struct SetIsEntitledToLevel entitlement = pkt_readSetIsEntitledToLevel(data);
			if(room->entitledCount + room->buzzkills.count == room->playerCount || session->menu.entitlementCounted || !String_eq(entitlement.levelId, room->selectedBeatmap.levelID))
				break;
			session->menu.entitlementCounted = 1;
			if(entitlement.entitlementStatus == EntitlementsStatus_Ok)
				++room->entitledCount;
			else
				room->buzzkills.playersWithoutEntitlements[room->buzzkills.count++] = session->permissions.userId;
			if(room->entitledCount + room->buzzkills.count == room->playerCount) {
				uint8_t resp[65536], *resp_end = resp;
				pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
				struct SetPlayersMissingEntitlementsToLevel r_missing;
				r_missing.base.syncTime = room_get_syncTime(room);
				r_missing.playersMissingEntitlements = room->buzzkills;
				SERIALIZE_MENURPC(&resp_end, SetPlayersMissingEntitlementsToLevel, r_missing);
				FOR_ALL_PLAYERS(room, id) {
					instance_send_channeled(&ctx->net, &room->players[id], resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
					room->players[id].menu.entitlementCounted = 0;
				}
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
				uint8_t select = room->playerCount;
				switch(room->configuration.songSelectionMode) {
					case SongSelectionMode_Vote: {
						uint8_t counter[126], max = 0;
						FOR_ALL_PLAYERS(room, id) {
							counter[id] = 0;
							if(room->players[id].permissions.hasRecommendBeatmapsPermission && room->players[id].menu.recommendedBeatmap.beatmapCharacteristicSerializedName.length) {
								FOR_ALL_PLAYERS(room, cmp) {
									if(!room->players[cmp].permissions.hasRecommendBeatmapsPermission)
										continue;
									if(id == cmp || (String_eq(room->players[id].menu.recommendedBeatmap.levelID, room->players[cmp].menu.recommendedBeatmap.levelID) && String_eq(room->players[id].menu.recommendedBeatmap.beatmapCharacteristicSerializedName, room->players[cmp].menu.recommendedBeatmap.beatmapCharacteristicSerializedName) && room->players[id].menu.recommendedBeatmap.difficulty == room->players[cmp].menu.recommendedBeatmap.difficulty)) {
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
				if(select < room->playerCount && room->players[select].menu.recommendedBeatmap.beatmapCharacteristicSerializedName.length) {
					room->entitledCount = 0, room->buzzkills.count = 0;
					room->selectedBeatmap = room->players[select].menu.recommendedBeatmap;
					uint8_t resp[65536], *resp_end = resp;
					struct GetIsEntitledToLevel r_level;
					r_level.base.syncTime = room_get_syncTime(room);
					r_level.levelId = session->menu.recommendedBeatmap.levelID;
					pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
					SERIALIZE_MENURPC(&resp_end, GetIsEntitledToLevel, r_level);
					FOR_ALL_PLAYERS(room, id)
						instance_send_channeled(&ctx->net, &room->players[id], resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
				} else {
					room->entitledCount = room->playerCount, room->buzzkills.count = 0;
					room->selectedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};
					refresh_button(ctx, room);
				}
			}
			break;
		}
		case MenuRpcType_GetRecommendedBeatmap: pkt_readGetRecommendedBeatmap(data); fprintf(stderr, "[INSTANCE] MenuRpcType_GetRecommendedBeatmap not implemented\n"); break;
		case MenuRpcType_SetSelectedGameplayModifiers: fprintf(stderr, "[INSTANCE] MenuRpcType_SetSelectedGameplayModifiers not implemented\n"); abort();
		case MenuRpcType_GetSelectedGameplayModifiers: fprintf(stderr, "[INSTANCE] MenuRpcType_GetSelectedGameplayModifiers not implemented\n"); abort();
		case MenuRpcType_RecommendGameplayModifiers: {
			uint8_t resp[65536], *resp_end = resp;
			struct SetSelectedGameplayModifiers r_modifiers;
			r_modifiers.base.syncTime = room_get_syncTime(room);
			r_modifiers.gameplayModifiers = pkt_readRecommendGameplayModifiers(data).gameplayModifiers;
			if(session->permissions.hasRecommendGameplayModifiersPermission)
				session->menu.recommendedModifiers = r_modifiers.gameplayModifiers;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
			SERIALIZE_MENURPC(&resp_end, SetSelectedGameplayModifiers, r_modifiers);
			instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_ClearRecommendedGameplayModifiers: memset(&session->menu.recommendedModifiers, 0, sizeof(session->menu.recommendedModifiers)); break;
		case MenuRpcType_GetRecommendedGameplayModifiers: pkt_readGetRecommendedGameplayModifiers(data); fprintf(stderr, "[INSTANCE] MenuRpcType_GetRecommendedGameplayModifiers not implemented\n"); break;
		case MenuRpcType_LevelLoadError: fprintf(stderr, "[INSTANCE] MenuRpcType_LevelLoadError not implemented\n"); abort();
		case MenuRpcType_LevelLoadSuccess: fprintf(stderr, "[INSTANCE] MenuRpcType_LevelLoadSuccess not implemented\n"); abort();
		case MenuRpcType_StartLevel: fprintf(stderr, "[INSTANCE] MenuRpcType_StartLevel not implemented\n"); abort();
		case MenuRpcType_GetStartedLevel: {
			pkt_readGetStartedLevel(data);
			if(room->state == MultiplayerGameState_Game) {
				fprintf(stderr, "[INSTANCE] MenuRpcType_GetStartedLevel not implemented\n"); abort();
			} else {
				uint8_t resp[65536], *resp_end = resp;
				struct SetIsStartButtonEnabled r_button;
				r_button.base.syncTime = room_get_syncTime(room);
				r_button.reason = CannotStartGameReason_NoSongSelected;
				pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
				SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_button);
				instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			}
			break;
		}
		case MenuRpcType_CancelLevelStart: fprintf(stderr, "[INSTANCE] MenuRpcType_CancelLevelStart not implemented\n"); abort();
		case MenuRpcType_GetMultiplayerGameState: {
			pkt_readGetMultiplayerGameState(data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetMultiplayerGameState r_state;
			r_state.base.syncTime = room_get_syncTime(room);
			r_state.lobbyState = room->state;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
			SERIALIZE_MENURPC(&resp_end, SetMultiplayerGameState, r_state);
			instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetMultiplayerGameState: fprintf(stderr, "[INSTANCE] MenuRpcType_SetMultiplayerGameState not implemented\n"); abort();
		case MenuRpcType_GetIsReady: pkt_readGetIsReady(data); break;
		case MenuRpcType_SetIsReady: {
			_Bool isReady = pkt_readSetIsReady(data).isReady;
			if(isReady != session->menu.isReady) {
				room->readyCount = room->readyCount - session->menu.isReady + isReady;
				session->menu.isReady = isReady;
			}
			break;
		}
		case MenuRpcType_SetStartGameTime: fprintf(stderr, "[INSTANCE] MenuRpcType_SetStartGameTime not implemented\n"); abort();
		case MenuRpcType_CancelStartGameTime: fprintf(stderr, "[INSTANCE] MenuRpcType_CancelStartGameTime not implemented\n"); abort();
		case MenuRpcType_GetIsInLobby: pkt_readGetIsInLobby(data); break;
		case MenuRpcType_SetIsInLobby: {
			_Bool inLobby = pkt_readSetIsInLobby(data).isBack;
			if(inLobby != session->menu.inLobby) {
				room->inLobbyCount = room->inLobbyCount - session->menu.inLobby + inLobby;
				session->menu.inLobby = inLobby;
				refresh_button(ctx, room);
			}
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
			instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetCountdownEndTime: fprintf(stderr, "[INSTANCE] MenuRpcType_SetCountdownEndTime not implemented\n"); abort();
		case MenuRpcType_CancelCountdown: fprintf(stderr, "[INSTANCE] MenuRpcType_CancelCountdown not implemented\n"); abort();
		case MenuRpcType_GetOwnedSongPacks: fprintf(stderr, "[INSTANCE] MenuRpcType_GetOwnedSongPacks not implemented\n"); abort();
		case MenuRpcType_SetOwnedSongPacks: session->menu.ownedSongPacks = pkt_readSetOwnedSongPacks(data).songPackMask; break;
		case MenuRpcType_RequestKickPlayer: fprintf(stderr, "[INSTANCE] MenuRpcType_RequestKickPlayer not implemented\n"); abort();
		case MenuRpcType_GetPermissionConfiguration:  {
			pkt_readGetPermissionConfiguration(data);
			uint8_t resp[65536], *resp_end = resp;
			struct SetPermissionConfiguration r_permission;
			r_permission.base.syncTime = room_get_syncTime(room);
			r_permission.playersPermissionConfiguration.count = room->playerCount;
			FOR_ALL_PLAYERS(room, id)
				r_permission.playersPermissionConfiguration.playersPermission[i] = room->players[id].permissions;
			pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 0, 0});
			SERIALIZE_MENURPC(&resp_end, SetPermissionConfiguration, r_permission);
			instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
			break;
		}
		case MenuRpcType_SetPermissionConfiguration: fprintf(stderr, "[INSTANCE] MenuRpcType_SetPermissionConfiguration not implemented\n"); abort();
		case MenuRpcType_GetIsStartButtonEnabled: fprintf(stderr, "[INSTANCE] MenuRpcType_GetIsStartButtonEnabled not implemented\n"); abort();
		case MenuRpcType_SetIsStartButtonEnabled: fprintf(stderr, "[INSTANCE] MenuRpcType_SetIsStartButtonEnabled not implemented\n"); abort();
		default: fprintf(stderr, "BAD MENU RPC TYPE\n");
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
	fprintf(stderr, "TODO: isSpectating should imply isReady\n");
	if(isSpectating != session->menu.isSpectating) {
		_Bool allPlayersSpectating = (room->spectatingCount == room->playerCount);
		room->spectatingCount = room->spectatingCount - session->menu.isSpectating + isSpectating;
		session->menu.isSpectating = isSpectating;
		if((room->spectatingCount == room->playerCount) != allPlayersSpectating)
			refresh_button(ctx, room);
	}
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
		instance_send_channeled(&ctx->net, &room->players[id], resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = routing_end;
	struct PlayerSortOrderUpdate r_sort;
	r_sort.userId = session->permissions.userId;
	r_sort.sortIndex = indexof(room->players, session);
	SERIALIZE_CUSTOM(&resp_end, InternalMessageType_PlayerSortOrderUpdate)
		pkt_writePlayerSortOrderUpdate(&resp_end, r_sort);
	FOR_ALL_PLAYERS(room, id)
		instance_send_channeled(&ctx->net, &room->players[id], resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	resp_end = resp;
	pkt_writeRoutingHeader(&resp_end, (struct RoutingHeader){0, 127, 0});
	struct RemoteProcedureCall base;
	base.syncTime = room_get_syncTime(room);
	struct SetIsStartButtonEnabled r_button;
	r_button.base = base;
	r_button.reason = CannotStartGameReason_AllPlayersNotInLobby;
	SERIALIZE_MENURPC(&resp_end, SetIsStartButtonEnabled, r_button);
	SERIALIZE_MENURPC(&resp_end, GetRecommendedBeatmap, (struct GetRecommendedBeatmap){.base = base});
	SERIALIZE_MENURPC(&resp_end, GetRecommendedGameplayModifiers, (struct GetRecommendedGameplayModifiers){.base = base});
	SERIALIZE_MENURPC(&resp_end, GetIsReady, (struct GetIsReady){.base = base});
	SERIALIZE_MENURPC(&resp_end, GetIsInLobby, (struct GetIsInLobby){.base = base});
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);
}

static void handle_MultiplayerSession(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MultiplayerSessionMessageHeader message = pkt_readMultiplayerSessionMessageHeader(data);
	switch(message.type) {
		case MultiplayerSessionMessageType_MenuRpc: handle_MenuRpc(ctx, room, session, data); break;
		case MultiplayerSessionMessageType_GameplayRpc: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_GameplayRpc not implemented\n"); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: pkt_readNodePoseSyncState(data); break;
		case MultiplayerSessionMessageType_ScoreSyncState: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_ScoreSyncState not implemented\n"); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_readNodePoseSyncStateDelta(data); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: fprintf(stderr, "[INSTANCE] MultiplayerSessionMessageType_ScoreSyncStateDelta not implemented\n"); break;
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
				instance_send_channeled(&ctx->net, &room->players[id], resp, resp_end - resp, channelId);
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
static void process_Reliable(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, DeliveryMethod channelId, _Bool isFragmented) {
	if(!isFragmented) {
		process_Channeled(ctx, room, session, data, end, channelId);
		return;
	}
	struct FragmentedHeader header = pkt_readFragmentedHeader(data);
	struct IncomingFragments **incoming = &session->incomingFragmentsList;
	do {
		if(!*incoming) {
			*incoming = malloc(sizeof(**incoming) + header.fragmentsTotal * sizeof(*(*incoming)->fragments));
			if(!*incoming) {
				fprintf(stderr, "[INSTANCE] alloc error\n");
				abort();
			}
			(*incoming)->fragmentId = header.fragmentId;
			(*incoming)->channelId = channelId;
			(*incoming)->count = 0;
			(*incoming)->total = header.fragmentsTotal;
			(*incoming)->size = 0;
			for(uint32_t i = 0; i < header.fragmentsTotal; ++i)
				(*incoming)->fragments[i].isFragmented = 0;
		} else if((*incoming)->fragmentId != header.fragmentId) {
			incoming = &(*incoming)->next;
			continue;
		}
		if(header.fragmentPart >= (*incoming)->total || (*incoming)->fragments[header.fragmentPart].isFragmented || channelId != (*incoming)->channelId)
			return;
		(*incoming)->size += end - *data;
		if(++(*incoming)->count < (*incoming)->total) {
			(*incoming)->fragments[header.fragmentPart].len = end - *data;
			(*incoming)->fragments[header.fragmentPart].isFragmented = 1;
			pkt_readUint8Array(data, (*incoming)->fragments[header.fragmentPart].data, end - *data);
			return;
		} else {
			uint8_t pkt[(*incoming)->size], *pkt_end = pkt;
			const uint8_t *pkt_it = pkt;
			for(uint32_t i = 0; i < (*incoming)->total; ++i) {
				if(i == header.fragmentPart)
					pkt_writeUint8Array(&pkt_end, *data, end - *data), *data = end;
				else
					pkt_writeUint8Array(&pkt_end, (*incoming)->fragments[i].data, (*incoming)->fragments[i].len);
			}
			#ifdef PACKET_LOGGING_FUNCS
			{
				char buf[1024*16];
				fprintf(stderr, "fragmented\n");
				debug_logRouting(pkt, pkt, pkt_end, buf);
			}
			#endif
			process_Channeled(ctx, room, session, &pkt_it, pkt_end, channelId);
			if(pkt_it != pkt_end)
				fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
			struct IncomingFragments *e = *incoming;
			*incoming = (*incoming)->next;
			free(e);
		}
	} while(0);
}
static void handle_Channeled(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data, const uint8_t *end, _Bool isFragmented) {
	struct Channeled channeled = pkt_readChanneled(data);
	if(channeled.sequence >= NET_MAX_SEQUENCE)
		return;
	struct ReliableChannel *channel = &session->roChannel.base;
	switch(channeled.channelId) {
		case DeliveryMethod_ReliableUnordered: channel = &session->ruChannel.base;
		case DeliveryMethod_ReliableOrdered: {
			if(channeled.sequence >= NET_MAX_SEQUENCE)
				return;
			int32_t relate = RelativeSequenceNumber(channeled.sequence, channel->remoteWindowStart);
			if(RelativeSequenceNumber(channeled.sequence, channel->remoteSequence) > NET_WINDOW_SIZE)
				return;
			if(relate < 0 || relate >= NET_WINDOW_SIZE * 2)
				return;
			if(relate >= NET_WINDOW_SIZE) {
				uint16_t newWindowStart = (channel->remoteWindowStart + relate - NET_WINDOW_SIZE + 1) % NET_MAX_SEQUENCE;
				channel->ack.sequence = newWindowStart;
				while(channel->remoteWindowStart != newWindowStart) {
					uint16_t ackIdx = channel->remoteWindowStart % NET_WINDOW_SIZE;
					channel->ack.data[ackIdx / 8] &= ~(1 << (ackIdx % 8));
					channel->remoteWindowStart = (channel->remoteWindowStart + 1) % NET_MAX_SEQUENCE;
				}
			}
			uint16_t ackIdx = channeled.sequence % NET_WINDOW_SIZE;
			if(channel->ack.data[ackIdx / 8] & (1 << (ackIdx % 8)))
				return;
			channel->ack.data[ackIdx / 8] |= 1 << (ackIdx % 8);
			if(channeled.sequence == channel->remoteSequence) {
				process_Reliable(ctx, room, session, data, end, channeled.channelId, isFragmented);
				channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
				if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
					while(session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE].len) {
						struct InstancePacket *ipkt = &session->roChannel.receivedPackets[channel->remoteSequence % NET_WINDOW_SIZE];
						const uint8_t *pkt = ipkt->data, *pkt_it = pkt;
						const uint8_t *pkt_end = &pkt[ipkt->len];
						ipkt->len = 0;
						process_Reliable(ctx, room, session, &pkt, pkt_end, DeliveryMethod_ReliableOrdered, ipkt->isFragmented);
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
						if(pkt_it != pkt_end)
							fprintf(stderr, "[INSTANCE] BAD PACKET LENGTH (expected %zu, read %zu)\n", pkt_end - pkt, pkt_it - pkt);
					}
				} else {
					while(session->ruChannel.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE]) {
						session->ruChannel.earlyReceived[channel->remoteSequence % NET_WINDOW_SIZE] = 0;
						channel->remoteSequence = (channel->remoteSequence + 1) % NET_MAX_SEQUENCE;
					}
				}
			} else if(channeled.channelId == DeliveryMethod_ReliableOrdered) {
				session->roChannel.receivedPackets[ackIdx].len = end - *data;
				session->roChannel.receivedPackets[ackIdx].isFragmented = isFragmented;
				memcpy(session->roChannel.receivedPackets[ackIdx].data, *data, end - *data);
				*data = end;
			} else {
				session->ruChannel.earlyReceived[ackIdx] = 1;
				process_Reliable(ctx, room, session, data, end, DeliveryMethod_ReliableUnordered, isFragmented);
			}
		}
		case DeliveryMethod_Sequenced: return;
		case DeliveryMethod_ReliableSequenced: {
			if(isFragmented) {
				fprintf(stderr, "MALFORMED PACKET\n");
				return;
			}
			int32_t relative = RelativeSequenceNumber(channeled.sequence, session->rsChannel.ack.sequence);
			if(channeled.sequence < NET_MAX_SEQUENCE && relative > 0) {
				session->rsChannel.ack.sequence = channeled.sequence;
				process_Channeled(ctx, room, session, data, end, DeliveryMethod_ReliableSequenced);
			}
			uint8_t resp[65536], *resp_end = resp;
			pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Ack, 0, 0});
			pkt_writeAck(&resp_end, session->rsChannel.ack);
			net_queue_merged(&ctx->net, &session->net, resp, resp_end - resp);
		}
		default:;
	}
	return;
}
static void handle_Ack(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct Ack ack = pkt_readAck(data);
	/*if(packet.Size != _outgoingAcks.Size) // [PA]Invalid acks packet size
		return;*/
	if(ack.channelId == DeliveryMethod_ReliableSequenced) {
		if(ack.sequence == session->rsChannel.localSeqence)
			session->rsChannel.resend.len = 0;
		return;
	}
	struct ReliableChannel *channel = (ack.channelId == DeliveryMethod_ReliableUnordered) ? &session->ruChannel.base : &session->roChannel.base;
	int32_t windowRel = RelativeSequenceNumber(channel->localWindowStart, ack.sequence);
	if(ack.sequence >= NET_MAX_SEQUENCE || windowRel < 0 || windowRel >= NET_WINDOW_SIZE) // [PA]Bad window start
		return;
	for(uint16_t pendingSeq = channel->localWindowStart; pendingSeq != channel->localSeqence; pendingSeq = (pendingSeq + 1) % NET_MAX_SEQUENCE) {
		if(RelativeSequenceNumber(pendingSeq, ack.sequence) >= NET_WINDOW_SIZE)
			break;
		uint16_t pendingIdx = pendingSeq % NET_WINDOW_SIZE;
		if((ack.data[pendingIdx / 8] & (1 << (pendingIdx % 8))) == 0) //Skip false ack
			continue;
		if(pendingSeq == channel->localWindowStart) //Move window
			channel->localWindowStart = (channel->localWindowStart + 1) % NET_MAX_SEQUENCE;
		channel->resend[pendingIdx].len = 0;
	}
}
static void handle_Ping(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct Ping ping = pkt_readPing(data);
	if(RelativeSequenceNumber(ping.sequence, session->pong.sequence) > 0) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		session->pong.sequence = ping.sequence;
		session->pong.time = (uint64_t)now.tv_sec * 10000000LLU + (uint64_t)now.tv_nsec / 100LLU;
		uint8_t resp[65536], *resp_end = resp;
		pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_Pong, 0, 0});
		pkt_writePong(&resp_end, session->pong);
		net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
	}
}
static void handle_ConnectRequest(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct ConnectRequest req = pkt_readConnectRequest(data);
	if(!String_eq(req.userId, session->permissions.userId))
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
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

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
	instance_send_channeled(&ctx->net, session, resp, resp_end - resp, DeliveryMethod_ReliableOrdered);

	fprintf(stderr, "TODO: send PlayerConnected to other players\n");
}
static void handle_Disconnect(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	pkt_readDisconnect(data);
	FOR_ALL_PLAYERS(room, id) {
		if(&room->players[id] == session) {
			room_disconnect(NULL, room, i);
			return;
		}
	}
}
static void handle_MtuCheck(struct Context *ctx, struct Room *room, struct InstanceSession *session, const uint8_t **data) {
	struct MtuCheck req = pkt_readMtuCheck(data);
	uint8_t resp[65536], *resp_end = resp;
	pkt_writeNetPacketHeader(&resp_end, (struct NetPacketHeader){PacketProperty_MtuOk, 0, 0});
	pkt_writeMtuOk(&resp_end, (struct MtuOk){
		.newMtu0 = req.newMtu0,
		.newMtu1 = req.newMtu1,
	});
	net_send_internal(&ctx->net, &session->net, resp, resp_end - resp, 1);
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
		case PacketProperty_Channeled: handle_Channeled(ctx, room, session, &data, end, header.isFragmented); break;
		case PacketProperty_Ack: handle_Ack(ctx, room, session, &data); break;
		case PacketProperty_Ping: handle_Ping(ctx, room, session, &data); break;
		case PacketProperty_Pong: fprintf(stderr, "[INSTANCE] PacketProperty_Pong not implemented\n"); break;
		case PacketProperty_ConnectRequest: handle_ConnectRequest(ctx, room, session, &data); break;
		case PacketProperty_ConnectAccept: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_ConnectAccept\n"); break;
		case PacketProperty_Disconnect: handle_Disconnect(ctx, room, session, &data); break;
		case PacketProperty_UnconnectedMessage: fprintf(stderr, "[INSTANCE] BAD PROPERTY: PacketProperty_UnconnectedMessage\n"); break;
		case PacketProperty_MtuCheck: handle_MtuCheck(ctx, room, session, &data); break;
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
	while((len = net_recv(&ctx->net, buf, sizeof(buf), (struct NetSession**)&session, &pkt, (void**)&room))) { // TODO: close instances with zero sessions using `onDisconnect`
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
static const char *instance_domain = NULL;
_Bool instance_init(const char *domain) {
	instance_domain = domain;
	memset(instance_threads, 0, sizeof(instance_threads));
	for(uint32_t i = 0; i < lengthof(instance_threads); ++i) {
		if(net_init(&contexts[i].net, 0)) {
			fprintf(stderr, "net_init() failed\n");
			return 1;
		}
		contexts[i].net.user = &contexts[i];
		contexts[i].net.onResolve = instance_onResolve;
		contexts[i].net.onResend = instance_onResend;
		contexts[i].TEMPglobalRoom.playerCount = 0;

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
	return 0;
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
			while(contexts[i].TEMPglobalRoom.playerCount)
				room_disconnect(NULL, &contexts[i].TEMPglobalRoom, contexts[i].TEMPglobalRoom.playerSort[0]);
			net_cleanup(&contexts[i].net);
		}
	}
}

_Bool instance_get_isopen(ServerCode code, struct String *managerId_out, struct GameplayServerConfiguration *configuration) {
	if(code == StringToServerCode("HELLO", 5)) { // TODO: temporary hack
		*managerId_out = contexts[0].TEMPglobalRoom.managerId;
		configuration->maxPlayerCount = contexts[0].TEMPglobalRoom.configuration.maxPlayerCount;
		return 1;
	}
	return 0;
}
_Bool instance_open(ServerCode *out, struct String managerId, struct GameplayServerConfiguration *configuration) {
	if(net_keypair_init(&contexts[0].net, &contexts[0].TEMPglobalRoom.keys))
		return 1;
	contexts[0].TEMPglobalRoom.syncBase = 0;
	contexts[0].TEMPglobalRoom.syncBase = room_get_syncTime(&contexts[0].TEMPglobalRoom);
	contexts[0].TEMPglobalRoom.state = MultiplayerGameState_Lobby;
	contexts[0].TEMPglobalRoom.inLobbyCount = 0;
	contexts[0].TEMPglobalRoom.readyCount = 0;
	contexts[0].TEMPglobalRoom.entitledCount = 0;
	contexts[0].TEMPglobalRoom.spectatingCount = 0;
	contexts[0].TEMPglobalRoom.selectedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};
	contexts[0].TEMPglobalRoom.buzzkills.count = 0;
	contexts[0].TEMPglobalRoom.playerCount = 0;
	contexts[0].TEMPglobalRoom.configuration = *configuration;
	size_t sortLen = (contexts[0].TEMPglobalRoom.configuration.maxPlayerCount * sizeof(*contexts[0].TEMPglobalRoom.playerSort) + 7) & ~7; // aligned
	contexts[0].TEMPglobalRoom.playerSort = malloc(sortLen + contexts[0].TEMPglobalRoom.configuration.maxPlayerCount * sizeof(*contexts[0].TEMPglobalRoom.players));
	contexts[0].TEMPglobalRoom.players = (struct InstanceSession*)&((uint8_t*)contexts[0].TEMPglobalRoom.playerSort)[sortLen];
	for(uint32_t i = 0; i < contexts[0].TEMPglobalRoom.configuration.maxPlayerCount; ++i) {
		contexts[0].TEMPglobalRoom.playerSort[i] = i;
		contexts[0].TEMPglobalRoom.players[i].clientState = ClientState_disconnected;
		contexts[0].TEMPglobalRoom.players[i].incomingFragmentsList = NULL;
	}
	contexts[0].TEMPglobalRoom.managerId = managerId;
	*out = StringToServerCode("HELLO", 5); // TODO: temporary hack
	return 0;
}

struct NetContext *instance_get_net(ServerCode code) {
	return (code == StringToServerCode("HELLO", 5)) ? &contexts[0].net : NULL;
}

struct NetSession *instance_resolve_session(ServerCode code, struct SS addr, struct String userId, struct String userName) {
	if(code != StringToServerCode("HELLO", 5))
		return NULL;
	struct Context *ctx = &contexts[0];
	struct Room *room;
	struct InstanceSession *session = (struct InstanceSession*)instance_onResolve(ctx, addr, (void**)&room); // TODO: This may connect the user to previous rooms they haven't successfully disconnected from
	if(session) {
		for(uint32_t i = 0; i < room->playerCount; ++i) {
			if(room->playerSort[i] == indexof(room->players, session)) {
				room_disconnect(ctx, room, i);
				break;
			}
		}
		/*if(net_session_reset(&ctx->net, &session->net))
			return NULL;
		while(session->incomingFragmentsList) {
			struct IncomingFragments *e = session->incomingFragmentsList;
			session->incomingFragmentsList = session->incomingFragmentsList->next;
			free(e);
		}*/
	} else {
		room = &ctx->TEMPglobalRoom;
		if(room->playerCount >= room->configuration.maxPlayerCount) {
			fprintf(stderr, "ROOM FULL\n");
			return NULL;
		}
		session = &room->players[room->playerSort[room->playerCount]];
		if(net_session_init(&ctx->net, &session->net, addr))
			return NULL;
		++room->playerCount;
	}
	session->clientState = ClientState_disconnected;
	session->pong.sequence = 0;
	memset(&session->ruChannel, 0, sizeof(session->ruChannel));
	memset(&session->roChannel, 0, sizeof(session->roChannel));
	memset(&session->rsChannel, 0, sizeof(session->rsChannel));
	session->ruChannel.base.ack.channelId = DeliveryMethod_ReliableUnordered;
	session->roChannel.base.ack.channelId = DeliveryMethod_ReliableOrdered;
	session->rsChannel.ack.channelId = DeliveryMethod_ReliableSequenced;

	session->userName = userName;
	session->permissions.userId = userId;
	session->permissions.isServerOwner = String_eq(userId, room->managerId);
	session->permissions.hasRecommendBeatmapsPermission = (room->configuration.songSelectionMode != SongSelectionMode_Random) && (session->permissions.isServerOwner || room->configuration.songSelectionMode != SongSelectionMode_OwnerPicks);
	session->permissions.hasRecommendGameplayModifiersPermission = (room->configuration.gameplayServerControlSettings == GameplayServerControlSettings_AllowModifierSelection || room->configuration.gameplayServerControlSettings == GameplayServerControlSettings_All);
	session->permissions.hasKickVotePermission = session->permissions.isServerOwner;
	session->permissions.hasInvitePermission = (room->configuration.invitePolicy == InvitePolicy_AnyoneCanInvite) || (session->permissions.isServerOwner && room->configuration.invitePolicy == InvitePolicy_OnlyConnectionOwnerCanInvite);
	session->state.bloomFilter = (struct BitMask128){0, 0};
	session->menu.inLobby = 0;
	session->menu.isReady = 0;
	session->menu.entitlementCounted = 0;
	session->menu.isSpectating = 0;
	session->menu.ownedSongPacks.bloomFilter.d0 = 0;
	session->menu.ownedSongPacks.bloomFilter.d1 = 0;
	memset(&session->menu.recommendedModifiers, 0, sizeof(session->menu.recommendedModifiers));
	session->menu.recommendedBeatmap = (struct BeatmapIdentifierNetSerializable){{0}, {0}, 0};

	char addrstr[INET6_ADDRSTRLEN + 8];
	net_tostr(&addr, addrstr);
	fprintf(stderr, "[INSTANCE] connect %s\n", addrstr);
	fprintf(stderr, "[INSTANCE] player count: %hhu / %d\n", room->playerCount, room->configuration.maxPlayerCount);
	return &session->net;
}

struct IPEndPoint instance_get_address(ServerCode code) {
	struct IPEndPoint out;
	out.address.length = 0;
	out.port = 0;
	if(code != StringToServerCode("HELLO", 5)) // TODO: temporary hack
		return out;
	out.address.length = sprintf(out.address.data, "%s", instance_domain);
	struct SS addr = {sizeof(struct sockaddr_storage)};
	getsockname(net_get_sockfd(&contexts[0].net), &addr.sa, &addr.len);
	switch(addr.ss.ss_family) {
		case AF_INET: {
			out.port = htons(addr.in.sin_port);
			break;
		}
		case AF_INET6: {
			out.port = htons(addr.in6.sin6_port);
			break;
		}
		default:;
	}
	return out;
}
