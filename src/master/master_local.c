#include "master_internal.h"
#include "pool.h"

struct LocalMasterContext {
	struct MasterContext base; // MUST be first
	struct NetContext net;
	struct WireContext wire;
	struct WireLink *status;
};

typedef uint8_t MasterCookieType;
enum {
	MasterCookieType_INVALID,
	MasterCookieType_LocalConnect,
	MasterCookieType_GraphConnect,
};

struct ConnectCookie {
	MasterCookieType cookieType;
	uint32_t room;
};

struct LocalConnectCookie {
	struct ConnectCookie base;
	struct String secret;
	struct SS addr;
	struct BaseMasterServerReliableRequest request;
	struct BeatmapLevelSelectionMask selectionMask;
};

struct GraphConnectCookie {
	struct ConnectCookie base;
	struct String secret;
	const struct WireLink *status;
	WireCookie cookie;
};

static ConnectToServerResponse_Result SendWireSessionAlloc(struct WireSessionAlloc *const allocInfo, struct ConnectCookie *const state, const size_t state_len, const struct GameplayServerConfiguration configuration, const ServerCode code, mbedtls_ctr_drbg_context *const ctr_drbg) {
	state->room = ~UINT32_C(0);
	if(allocInfo->clientVersion.protocolVersion >= 10) {
		uprintf("Connect to Server Error: Game version too new\n");
		return ConnectToServerResponse_Result_VersionMismatch;
	}
	WireCookie cookie = 0;
	bool failed = true;
	struct WireLink *link = NULL;
	if(code == ServerCode_NONE) {
		if(!allocInfo->secret.length) {
			uprintf("Connect to Server Error: Quickplay not supported\n");
			return ConnectToServerResponse_Result_NoAvailableDedicatedServers;
		}
		struct PoolHost *host = pool_handle_new(&state->room, (configuration.discoveryPolicy == DiscoveryPolicy_Public) ? NULL : ctr_drbg);
		if(host == NULL) {
			uprintf("Connect to Server Error: pool_handle_new() failed\n");
			return ConnectToServerResponse_Result_NoAvailableDedicatedServers;
		}
		allocInfo->room = state->room;
		link = pool_host_wire(host);
		cookie = WireLink_makeCookie(link, state, state_len);
		failed = WireLink_send(link, &(struct WireMessage){
			.type = WireMessageType_WireRoomSpawn,
			.cookie = cookie,
			.roomSpawn = {
				.base = *allocInfo,
				.configuration = configuration,
			},
		});
	} else {
		struct PoolHost *host = pool_handle_lookup(&state->room, code);
		if(host == NULL) {
			uprintf("Connect to Server Error: Room '%s' does not exist\n", ServerCodeToString((char[8]){0}, code));
			return ConnectToServerResponse_Result_InvalidCode;
		}
		allocInfo->room = state->room;
		link = pool_host_wire(host);
		cookie = WireLink_makeCookie(link, state, state_len);
		failed = WireLink_send(link, &(struct WireMessage){
			.type = WireMessageType_WireRoomJoin,
			.cookie = cookie,
			.roomJoin.base = *allocInfo,
		});
	}
	if(failed) {
		WireLink_freeCookie(link, cookie);
		return ConnectToServerResponse_Result_UnknownError;
	}
	return ConnectToServerResponse_Result_Success;
}

static struct NetSession *master_onResolve(struct NetContext *net, struct SS addr, const uint8_t packet[static 1536], uint32_t packet_len, uint8_t out[static 1536], uint32_t *out_len, void**) {
	return MasterContext_onResolve(&((struct LocalMasterContext*)net->userptr)->base, net, addr, packet, packet_len, out, out_len);
}

static uint32_t master_onResend(struct NetContext *net, uint32_t currentTime) {
	return MasterContext_onResend(&((struct LocalMasterContext*)net->userptr)->base, net, currentTime);
}

static uint32_t shuffle(uint32_t num, bool dir) {
	static const uint32_t magic[2] = {0x45d9f3b, 0x119de1f3};
	num = ((num >> 16) ^ num) * magic[dir];
	num = ((num >> 16) ^ num) * magic[dir];
	num = (num >> 16) ^ num;
	return num;
}

static bool handle_WireSessionAllocResp_graph(struct LocalMasterContext *ctx, struct PoolHost *host, const struct GraphConnectCookie *state, const struct WireSessionAllocResp *sessionAlloc) {
	if(state->status != ctx->status || state->status == NULL)
		return true;
	struct WireGraphConnectResp resp = {
		.result = MultiplayerPlacementErrorCode_Unknown,
	};
	switch((sessionAlloc != NULL) ? sessionAlloc->result : ConnectToServerResponse_Result_UnknownError) {
		case ConnectToServerResponse_Result_Success: {
			resp = (struct WireGraphConnectResp){
				.result = MultiplayerPlacementErrorCode_Success,
				.configuration = sessionAlloc->configuration,
				.hostId = shuffle(((uint32_t)pool_host_ident(host) << 14) | (state->base.room + 1), false),
				.endPoint = sessionAlloc->endPoint,
				.roomSlot = state->base.room,
				.playerSlot = sessionAlloc->playerSlot,
				.code = pool_handle_code(host, state->base.room),
			};
			uprintf("Sending player to room '%s'\n", ServerCodeToString((char[8]){0}, resp.code));
			break;
		}
		case ConnectToServerResponse_Result_InvalidSecret: [[fallthrough]];
		case ConnectToServerResponse_Result_InvalidCode: resp.result = MultiplayerPlacementErrorCode_ServerDoesNotExist; break;
		case ConnectToServerResponse_Result_InvalidPassword: resp.result = MultiplayerPlacementErrorCode_AuthenticationFailed; break;
		case ConnectToServerResponse_Result_ServerAtCapacity: resp.result = MultiplayerPlacementErrorCode_ServerAtCapacity; break;
		case ConnectToServerResponse_Result_NoAvailableDedicatedServers: resp.result = MultiplayerPlacementErrorCode_ServerDoesNotExist; break;
		default:;
	}
	WireLink_send(ctx->status, &(struct WireMessage){
		.cookie = state->cookie,
		.type = WireMessageType_WireGraphConnectResp,
		.graphConnectResp = resp,
	});
	return resp.result != MultiplayerPlacementErrorCode_Success;
}

static void handle_WireGraphConnect(struct LocalMasterContext *ctx, WireCookie cookie, const struct WireGraphConnect *req) {
	struct GraphConnectCookie state = {
		.base.cookieType = MasterCookieType_GraphConnect,
		.secret = req->secret,
		.status = ctx->status,
		.cookie = cookie,
	};
	struct WireSessionAlloc allocInfo = {
		.secret = req->secret,
		.userId = req->userId,
		.ipv4 = true,
		.clientVersion = PV_LEGACY_DEFAULT,
	};
	allocInfo.clientVersion.direct = true;
	allocInfo.clientVersion.protocolVersion = (uint8_t)req->protocolVersion;
	allocInfo.clientVersion.gameVersion = req->gameVersion;
	const ConnectToServerResponse_Result result = SendWireSessionAlloc(&allocInfo, &state.base, sizeof(state), req->configuration, req->code, &ctx->net.ctr_drbg);
	if(result == ConnectToServerResponse_Result_Success)
		return;
	handle_WireSessionAllocResp_graph(ctx, NULL, &state, &(const struct WireSessionAllocResp){
		.result = result,
	});
}

static void master_onWireMessage_status(struct LocalMasterContext *ctx, struct WireLink *link, const struct WireMessage *message) {
	if(message == NULL) {
		for(struct PoolHost *host = pool_host_iter_start(); host != NULL; host = pool_host_iter_next(host)) {
			struct WireLink *const hostLink = pool_host_wire(host);
			for(WireCookie cookie = 1; cookie <= WireLink_lastCookieIndex(hostLink); ++cookie) {
				const struct DataView view = WireLink_getCookie(hostLink, cookie);
				if(view.length != sizeof(struct GraphConnectCookie) || ((struct GraphConnectCookie*)view.data)->base.cookieType != MasterCookieType_GraphConnect)
					continue;
				if(((struct GraphConnectCookie*)view.data)->status == link)
					((struct GraphConnectCookie*)view.data)->status = NULL;
			}
		}
		if(ctx->status == link)
			ctx->status = NULL;
		return;
	}
	if(message->type == WireMessageType_WireStatusAttach) {
		ctx->status = link;
		return;
	}
	if(link != ctx->status) {
		uprintf("dropping unbound wire message\n");
		return;
	}
	switch(message->type) {
		case WireMessageType_WireGraphConnect: handle_WireGraphConnect(ctx, message->cookie, &message->graphConnect); break;
		default: uprintf("Unhandled wire message [%s]\n", reflect(WireMessageType, message->type));
	}
}

static bool handle_WireSessionAllocResp_local(struct NetContext *net, struct MasterSession *session, struct PoolHost *host, const struct LocalConnectCookie *state, const struct WireSessionAllocResp *sessionAlloc) {
	struct UserMessage r_conn = {
		.type = UserMessageType_ConnectToServerResponse,
		.connectToServerResponse = {
			.base = {
				.requestId = (session != NULL) ? MasterSession_nextRequestId(session) : 0,
				.responseId = state->request.requestId,
			},
			.result = (sessionAlloc != NULL) ? sessionAlloc->result : ConnectToServerResponse_Result_UnknownError,
		},
	};

	if(r_conn.connectToServerResponse.result == ConnectToServerResponse_Result_Success) {
		r_conn.connectToServerResponse = (struct ConnectToServerResponse){
			.base = r_conn.connectToServerResponse.base,
			.result = ConnectToServerResponse_Result_Success,
			.userId = String_fmt("beatupserver:%08x", shuffle(((uint32_t)pool_host_ident(host) << 14) | (state->base.room + 1), false)),
			.userName = String_from(""),
			.secret = state->secret,
			.selectionMask = state->selectionMask,
			.isConnectionOwner = true,
			.isDedicatedServer = true,
			.remoteEndPoint = sessionAlloc->endPoint,
			.random = sessionAlloc->random,
			.publicKey = sessionAlloc->publicKey,
			.code = pool_handle_code(host, state->base.room),
			.configuration = sessionAlloc->configuration,
			.managerId = sessionAlloc->managerId,
		};
		uprintf("Sending player to room '%s'\n", ServerCodeToString((char[8]){0}, r_conn.connectToServerResponse.code));
	}

	if(session != NULL)
		MasterSession_send(net, session, MessageType_UserMessage, &r_conn);
	return r_conn.connectToServerResponse.result != ConnectToServerResponse_Result_Success;
}

static void handle_WireSessionAllocResp(struct LocalMasterContext *const ctx, struct WireLink *const link, struct PoolHost *const host, const WireCookie cookie, const struct WireSessionAllocResp *const sessionAlloc, const bool spawn) {
	const struct DataView view = WireLink_getCookie(link, cookie);
	const struct ConnectCookie *const state = (struct ConnectCookie*)view.data;
	bool dropped = spawn;
	switch((view.length >= sizeof(*state)) ? state->cookieType : MasterCookieType_INVALID) {
		case MasterCookieType_LocalConnect: {
			if(view.length != sizeof(struct LocalConnectCookie))
				break;
			const struct LocalConnectCookie *const localState = (const struct LocalConnectCookie*)view.data;
			dropped &= handle_WireSessionAllocResp_local(&ctx->net, MasterContext_lookup(&ctx->base, localState->addr), host, localState, sessionAlloc);
			return;
		}
		case MasterCookieType_GraphConnect: {
			if(view.length != sizeof(struct GraphConnectCookie))
				break;
			dropped &= handle_WireSessionAllocResp_graph(ctx, host, (const struct GraphConnectCookie*)view.data, sessionAlloc);
			return;
		}
		default: dropped = false;
	}
	if(dropped)
		pool_handle_free(host, state->room);
	uprintf("Connect to Server Error: Malformed wire cookie\n");
}

static void master_onWireMessage(struct WireContext *wire, struct WireLink *link, const struct WireMessage *message) {
	struct LocalMasterContext *const ctx = (struct LocalMasterContext*)wire->userptr;
	net_lock(&ctx->net);
	struct PoolHost **const host = (struct PoolHost**)WireLink_userptr(link);
	if(*host == NULL) {
		if(message == NULL || message->type != WireMessageType_WireInstanceAttach) {
			master_onWireMessage_status(ctx, link, message);
			goto unlock;
		}
		*host = pool_host_attach(link);
		if(*host == NULL) {
			uprintf("pool_host_attach() failed\n");
			goto unlock;
		}
	}
	if(message == NULL) {
		for(WireCookie cookie = 1; cookie <= WireLink_lastCookieIndex(link); ++cookie) {
			const struct DataView view = WireLink_getCookie(link, cookie);
			const MasterCookieType cookieType = (view.length >= sizeof(cookieType)) ? *(MasterCookieType*)view.data : MasterCookieType_INVALID;
			if(cookieType == MasterCookieType_LocalConnect || cookieType == MasterCookieType_GraphConnect)
				handle_WireSessionAllocResp(ctx, link, *host, cookie, NULL, false); // TODO: retry with a different instance if any are still alive
			WireLink_freeCookie(link, cookie);
		}
		pool_host_detach(*host);
		*host = NULL;
		goto unlock;
	}
	switch(message->type) {
		case WireMessageType_WireInstanceAttach: TEMPpool_host_setAttribs(*host, message->instanceAttach.capacity, message->instanceAttach.discover); break;
		case WireMessageType_WireRoomSpawnResp: [[fallthrough]];
		case WireMessageType_WireRoomJoinResp: {
			handle_WireSessionAllocResp(ctx, link, *host, message->cookie, &message->roomJoinResp.base, message->type == WireMessageType_WireRoomSpawnResp);
			WireLink_freeCookie(link, message->cookie);
			break;
		}
		case WireMessageType_WireRoomStatusNotify: [[fallthrough]];
		case WireMessageType_WireRoomCloseNotify: {
			struct WireMessage forward = *message;
			forward.cookie = pool_handle_sequence(*host, message->cookie);
			if(message->type == WireMessageType_WireRoomStatusNotify)
				*(uint32_t*)forward.roomStatusNotify.entry = pool_handle_code(*host, message->cookie);
			else
				pool_handle_free(*host, message->cookie);
			if(ctx->status == NULL)
				break;
			WireLink_send(ctx->status, &forward);
		} break;
		default: uprintf("Unhandled wire message [%s]\n", reflect(WireMessageType, message->type));
	}
	unlock: net_unlock(&ctx->net);
}

// TODO: deduplicate requests
void LocalMasterContext_process_ConnectToServerRequest(struct NetContext *const net, struct MasterSession *const session, const struct ConnectToServerRequest *const req) {
	struct LocalConnectCookie state = {
		.base.cookieType = MasterCookieType_LocalConnect,
		.secret = req->secret,
		.addr = *NetSession_get_addr(&session->net),
		.request = req->base.base,
		.selectionMask = req->selectionMask,
	};
	/*struct BitMask128 customs = get_mask("custom_levelpack_CustomLevels");
	state.selectionMask.songPacks.bloomFilter.d0 |= customs.d0;
	state.selectionMask.songPacks.bloomFilter.d1 |= customs.d1;*/
	struct WireSessionAlloc allocInfo = {
		.secret = req->secret,
		.userId = req->base.userId,
		.ipv4 = (state.addr.ss.ss_family != AF_INET6 || memcmp(state.addr.in6.sin6_addr.s6_addr, (const uint16_t[]){0,0,0,0,0,0xffff}, 12) == 0),
		.clientVersion = PV_LEGACY_DEFAULT,
		.random = req->base.random,
		.publicKey = req->base.publicKey,
	};
	allocInfo.clientVersion.direct = false;
	allocInfo.clientVersion.protocolVersion = session->net.version.protocolVersion;
	switch(session->net.version.protocolVersion) {
		case 6: allocInfo.clientVersion.gameVersion = GameVersion_1_19_0; break;
		case 7: allocInfo.clientVersion.gameVersion = GameVersion_1_19_1; break;
		default: allocInfo.clientVersion.gameVersion = GameVersion_1_20_0;
	}
	const ConnectToServerResponse_Result result = SendWireSessionAlloc(&allocInfo, &state.base, sizeof(state), req->configuration, req->code, &net->ctr_drbg);
	if(result == ConnectToServerResponse_Result_Success)
		return;
	handle_WireSessionAllocResp_local(net, session, NULL, &state, &(const struct WireSessionAllocResp){
		.result = result,
	});
}

static void *master_handler(struct LocalMasterContext *ctx) {
	net_lock(&ctx->net);
	uprintf("Started\n");
	uint8_t pkt[1536];
	memset(pkt, 0, sizeof(pkt));
	uint32_t len;
	struct MasterSession *session;
	while((len = net_recv(&ctx->net, pkt, (struct NetSession**)&session, NULL)))
		MasterContext_handle(&ctx->base, &ctx->net, session, pkt, &pkt[len], NULL);
	net_unlock(&ctx->net);
	return 0;
}

static pthread_t master_thread = NET_THREAD_INVALID;
struct LocalMasterContext LocalMasterContext_Instance = {
	.net = CLEAR_NETCONTEXT,
}; // TODO: This "singleton" can't scale up due to pool API thread safety
struct WireContext *master_init(const mbedtls_x509_crt *cert, const mbedtls_pk_context *key, uint16_t port) {
	if(net_init(&LocalMasterContext_Instance.net, port)) {
		uprintf("net_init() failed\n");
		return NULL;
	}
	if(WireContext_init(&LocalMasterContext_Instance.wire, &LocalMasterContext_Instance, 16)) {
		uprintf("WireContext_init() failed\n");
		goto fail0;
	}
	MasterContext_init(&LocalMasterContext_Instance.base, &LocalMasterContext_Instance.net.ctr_drbg);
	if(MasterContext_setCertificate(&LocalMasterContext_Instance.base, cert, key))
		goto fail1;
	LocalMasterContext_Instance.net.userptr = &LocalMasterContext_Instance;
	LocalMasterContext_Instance.net.onResolve = master_onResolve;
	LocalMasterContext_Instance.net.onResend = master_onResend;
	LocalMasterContext_Instance.wire.onMessage = master_onWireMessage;
	if(pthread_create(&master_thread, NULL, (void *(*)(void*))master_handler, &LocalMasterContext_Instance)) {
		master_thread = NET_THREAD_INVALID;
		goto fail2;
	}
	return &LocalMasterContext_Instance.wire;
	fail2: MasterContext_cleanup(&LocalMasterContext_Instance.base);
	fail1: WireContext_cleanup(&LocalMasterContext_Instance.wire);
	fail0: net_cleanup(&LocalMasterContext_Instance.net);
	return NULL;
}

void master_cleanup() {
	if(master_thread != NET_THREAD_INVALID) {
		net_stop(&LocalMasterContext_Instance.net);
		uprintf("Stopping\n");
		pthread_join(master_thread, NULL);
		master_thread = NET_THREAD_INVALID;
		MasterContext_cleanup(&LocalMasterContext_Instance.base);
	}
	pool_reset();
	WireContext_cleanup(&LocalMasterContext_Instance.wire);
	net_cleanup(&LocalMasterContext_Instance.net);
}
