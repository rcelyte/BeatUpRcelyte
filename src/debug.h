#include "log.h"

static void debug_logMessage(struct MessageHeader message, struct SerializeHeader serial) {
	uprintf("\tMessageHeader.type=%u (%s)\n", message.type, reflect(MessageType, message.type));
	uprintf("\tMessageHeader.protocolVersion=%u\n", message.protocolVersion);
	uprintf("\tSerializeHeader.length=%u\n", serial.length);
	if(message.type == MessageType_UserMessage)
		uprintf("\tSerializeHeader.type=%u (%s)\n", serial.type, reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		uprintf("\tSerializeHeader.type=%u (%s)\n", serial.type, reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		uprintf("\tSerializeHeader.type=%u (%s)\n", serial.type, reflect(HandshakeMessageType, serial.type));
}

static void debug_logType(struct MessageHeader message, struct SerializeHeader serial) {
	if(message.type == MessageType_UserMessage)
		uprintf("recieve UserMessageType_%s\n", reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		uprintf("recieve DedicatedServerMessageType_%s\n", reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		uprintf("recieve HandshakeMessageType_%s\n", reflect(HandshakeMessageType, serial.type));
}

#ifdef PACKET_LOGGING_FUNCS
#define DEBUG_LOGCASE(type, name, data) case type##_##name: pkt_log##name(version, "\t" #name, buf, buf, pkt_read##name(version, data)); break
/*static void pkt_logUint8Array(const char *name, char *buf, char *it, const uint8_t *in, uint32_t count) {
	uprintf("%.*s%s=", (uint32_t)(it - buf), buf, name);
	for(uint32_t i = 0; i < count; ++i)
		uprintf("%02hhx", in[i]);
	uprintf("\n");
}*/
static const uint8_t *debug_logRouting(const uint8_t *pkt, const uint8_t *data, const uint8_t *end, char *buf, struct PacketContext version) {
	pkt_logRoutingHeader(version, "\tRoutingHeader", buf, buf, pkt_readRoutingHeader(version, &data));
	while(data < end) {
		struct SerializeHeader serial = pkt_readSerializeHeader(version, &data);
		const uint8_t *sub = data--;
		data += serial.length;
		uprintf("\tSerializeHeader[@%zu].type=%u (%s)\n", sub - pkt, serial.type, reflect(InternalMessageType, serial.type));
		switch(serial.type) {
			DEBUG_LOGCASE(InternalMessageType, SyncTime, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerConnected, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerIdentity, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerLatencyUpdate, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerDisconnected, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerSortOrderUpdate, &sub);
			DEBUG_LOGCASE(InternalMessageType, Party, &sub);
			case InternalMessageType_MultiplayerSession: {
				struct MultiplayerSessionMessageHeader smsg = pkt_readMultiplayerSessionMessageHeader(version, &sub);
				pkt_logMultiplayerSessionMessageHeader(version, "\tMultiplayerSessionMessageHeader", buf, buf, smsg);
				switch(smsg.type) {
					case MultiplayerSessionMessageType_MenuRpc: {
						struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(version, &sub);
						pkt_logMenuRpcHeader(version, "\tMenuRpcHeader", buf, buf, rpc);
						switch(rpc.type) {
							DEBUG_LOGCASE(MenuRpcType, SetPlayersMissingEntitlementsToLevel, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetIsEntitledToLevel, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetIsEntitledToLevel, &sub);
							DEBUG_LOGCASE(MenuRpcType, InvalidateLevelEntitlementStatuses, &sub);
							DEBUG_LOGCASE(MenuRpcType, SelectLevelPack, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetSelectedBeatmap, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetSelectedBeatmap, &sub);
							DEBUG_LOGCASE(MenuRpcType, RecommendBeatmap, &sub);
							DEBUG_LOGCASE(MenuRpcType, ClearRecommendedBeatmap, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetRecommendedBeatmap, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetSelectedGameplayModifiers, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetSelectedGameplayModifiers, &sub);
							DEBUG_LOGCASE(MenuRpcType, RecommendGameplayModifiers, &sub);
							DEBUG_LOGCASE(MenuRpcType, ClearRecommendedGameplayModifiers, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetRecommendedGameplayModifiers, &sub);
							DEBUG_LOGCASE(MenuRpcType, LevelLoadError, &sub);
							DEBUG_LOGCASE(MenuRpcType, LevelLoadSuccess, &sub);
							DEBUG_LOGCASE(MenuRpcType, StartLevel, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetStartedLevel, &sub);
							DEBUG_LOGCASE(MenuRpcType, CancelLevelStart, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetMultiplayerGameState, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetMultiplayerGameState, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetIsReady, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetIsReady, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetStartGameTime, &sub);
							DEBUG_LOGCASE(MenuRpcType, CancelStartGameTime, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetIsInLobby, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetIsInLobby, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetCountdownEndTime, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetCountdownEndTime, &sub);
							DEBUG_LOGCASE(MenuRpcType, CancelCountdown, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetOwnedSongPacks, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetOwnedSongPacks, &sub);
							DEBUG_LOGCASE(MenuRpcType, RequestKickPlayer, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetPermissionConfiguration, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetPermissionConfiguration, &sub);
							DEBUG_LOGCASE(MenuRpcType, GetIsStartButtonEnabled, &sub);
							DEBUG_LOGCASE(MenuRpcType, SetIsStartButtonEnabled, &sub);
							default: uprintf("BAD MENU RPC TYPE\n");
						}
						break;
					}
					case MultiplayerSessionMessageType_GameplayRpc: {
						struct GameplayRpcHeader rpc = pkt_readGameplayRpcHeader(version, &sub);
						pkt_logGameplayRpcHeader(version, "\tGameplayRpcHeader", buf, buf, rpc);
						switch(rpc.type) {
							DEBUG_LOGCASE(GameplayRpcType, SetGameplaySceneSyncFinish, &sub);
							DEBUG_LOGCASE(GameplayRpcType, SetGameplaySceneReady, &sub);
							DEBUG_LOGCASE(GameplayRpcType, GetGameplaySceneReady, &sub);
							DEBUG_LOGCASE(GameplayRpcType, SetActivePlayerFailedToConnect, &sub);
							DEBUG_LOGCASE(GameplayRpcType, SetGameplaySongReady, &sub);
							DEBUG_LOGCASE(GameplayRpcType, GetGameplaySongReady, &sub);
							DEBUG_LOGCASE(GameplayRpcType, SetSongStartTime, &sub);
							DEBUG_LOGCASE(GameplayRpcType, NoteCut, &sub);
							DEBUG_LOGCASE(GameplayRpcType, NoteMissed, &sub);
							DEBUG_LOGCASE(GameplayRpcType, LevelFinished, &sub);
							DEBUG_LOGCASE(GameplayRpcType, ReturnToMenu, &sub);
							DEBUG_LOGCASE(GameplayRpcType, RequestReturnToMenu, &sub);
							DEBUG_LOGCASE(GameplayRpcType, NoteSpawned, &sub);
							DEBUG_LOGCASE(GameplayRpcType, ObstacleSpawned, &sub);
							DEBUG_LOGCASE(GameplayRpcType, SliderSpawned, &sub);
							default: uprintf("BAD GAMEPLAY RPC TYPE\n");
						}
						break;
					}
					#if 0
					DEBUG_LOGCASE(MultiplayerSessionMessageType, NodePoseSyncState, &sub);
					DEBUG_LOGCASE(MultiplayerSessionMessageType, ScoreSyncState, &sub);
					DEBUG_LOGCASE(MultiplayerSessionMessageType, NodePoseSyncStateDelta, &sub);
					DEBUG_LOGCASE(MultiplayerSessionMessageType, ScoreSyncStateDelta, &sub);
					#else
					case MultiplayerSessionMessageType_NodePoseSyncState: pkt_readNodePoseSyncState(version, &sub); break;
					case MultiplayerSessionMessageType_ScoreSyncState: pkt_readScoreSyncState(version, &sub); break;
					case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_readNodePoseSyncStateDelta(version, &sub); break;
					case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_readScoreSyncStateDelta(version, &sub); break;
					#endif
					case MultiplayerSessionMessageType_MpCore: {
						struct MpCore mpHeader = pkt_readMpCore(version, &sub);
						if(mpHeader.type.length == 15 && memcmp(mpHeader.type.data, "MpBeatmapPacket", 15) == 0) {
							pkt_logMpBeatmapPacket(version, "\tMpBeatmapPacket", buf, buf, pkt_readMpBeatmapPacket(version, &sub));
						} else if(mpHeader.type.length == 12 && memcmp(mpHeader.type.data, "MpPlayerData", 12) == 0) {
							pkt_logMpPlayerData(version, "\tMpPlayerData", buf, buf, pkt_readMpPlayerData(version, &sub));
						} else {
							uprintf("[INSTANCE] BAD MPCORE MESSAGE TYPE: '%.*s'\n", mpHeader.type.length, mpHeader.type.data);
						}
						break;
					}
					case MultiplayerSessionMessageType_BeatUpMessage: {
						struct BeatUpMessageHeader message = pkt_readBeatUpMessageHeader(version, &sub);
						switch(message.type) {
							DEBUG_LOGCASE(BeatUpMessageType, RecommendPreview, &sub);
							DEBUG_LOGCASE(BeatUpMessageType, SetCanShareBeatmap, &sub);
							DEBUG_LOGCASE(BeatUpMessageType, DirectDownloadInfo, &sub);
							DEBUG_LOGCASE(BeatUpMessageType, LevelFragmentRequest, &sub);
							DEBUG_LOGCASE(BeatUpMessageType, LevelFragment, &sub);
						}
						break;
					}
					default: uprintf("BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
				}
			}  break;
			DEBUG_LOGCASE(InternalMessageType, KickPlayer, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerStateUpdate, &sub);
			DEBUG_LOGCASE(InternalMessageType, PlayerAvatarUpdate, &sub);
			DEBUG_LOGCASE(InternalMessageType, PingMessage, &sub);
			DEBUG_LOGCASE(InternalMessageType, PongMessage, &sub);
			default: uprintf("BAD INTERNAL MESSAGE TYPE\n"); continue;
		}
		if(sub != data) {
			uprintf("BAD INTERNAL MESSAGE LENGTH (expected %u, read %zu)\n", serial.length, sub - (data - serial.length));
			if(sub < data) {
				uprintf("\t");
				for(const uint8_t *it = data - serial.length; it < data; ++it)
					uprintf("%02hhx", *it);
				uprintf("\n\t");
				for(const uint8_t *it = data - serial.length; it < sub; ++it)
					uprintf("  ");
				uprintf("^ extra data starts here");
			}
			uprintf("\n");
		}
	}
	uprintf("\tSerializeHeader[@%zu] end\n", data - pkt);
	return data;
}

static _Bool debug_defaultFilter(PacketProperty property) {
	return 1;
}

static void debug_logPacket(const uint8_t *pkt, const uint8_t *end, struct NetPacketHeader header, struct PacketContext version, _Bool (*filter)(PacketProperty)) {
	if(!filter)
		filter = debug_defaultFilter;
	if(header.isFragmented) {
		uprintf("FRAGMENTED\n");
		return;
	}
	const uint8_t *data = pkt;
	char buf[1024*16];
	if(!(header.property == PacketProperty_Merged || filter(header.property)))
		return;
	switch(header.property) {
		case PacketProperty_Unreliable: data = debug_logRouting(pkt, data, end, buf, version); break;
		case PacketProperty_Channeled: {
			pkt_logChanneled(version, "\tChanneled", buf, buf, pkt_readChanneled(version, &data));
			data = debug_logRouting(pkt, data, end, buf, version);
			break;
		}
		DEBUG_LOGCASE(PacketProperty, Ack, &data);
		DEBUG_LOGCASE(PacketProperty, Ping, &data);
		DEBUG_LOGCASE(PacketProperty, Pong, &data);
		DEBUG_LOGCASE(PacketProperty, ConnectRequest, &data);
		DEBUG_LOGCASE(PacketProperty, ConnectAccept, &data);
		DEBUG_LOGCASE(PacketProperty, Disconnect, &data);
		case PacketProperty_UnconnectedMessage: {
			struct MessageHeader message = pkt_readMessageHeader(version, &data);
			struct SerializeHeader serial = pkt_readSerializeHeader(version, &data);
			debug_logMessage(message, serial);
			if(message.type == MessageType_UserMessage) {
				switch(serial.type) {
					DEBUG_LOGCASE(UserMessageType, AuthenticateUserRequest, &data);
					DEBUG_LOGCASE(UserMessageType, AuthenticateUserResponse, &data);
					DEBUG_LOGCASE(UserMessageType, ConnectToServerResponse, &data);
					DEBUG_LOGCASE(UserMessageType, ConnectToServerRequest, &data);
					DEBUG_LOGCASE(UserMessageType, UserMessageReceivedAcknowledge, &data);
					DEBUG_LOGCASE(UserMessageType, UserMultipartMessage, &data);
					DEBUG_LOGCASE(UserMessageType, SessionKeepaliveMessage, &data);
					DEBUG_LOGCASE(UserMessageType, GetPublicServersRequest, &data);
					DEBUG_LOGCASE(UserMessageType, GetPublicServersResponse, &data);
					default: uprintf("BAD USER MESSAGE TYPE\n");
				}
			} else if(message.type == MessageType_DedicatedServerMessage) {
				uprintf("DedicatedServerMessageType not implemented\n");
			} else if(message.type == MessageType_HandshakeMessage) {
				switch(serial.type) {
					DEBUG_LOGCASE(HandshakeMessageType, ClientHelloRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, HelloVerifyRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, ClientHelloWithCookieRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, ServerHelloRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, ServerCertificateRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, ClientKeyExchangeRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, ChangeCipherSpecRequest, &data);
					DEBUG_LOGCASE(HandshakeMessageType, HandshakeMessageReceivedAcknowledge, &data);
					DEBUG_LOGCASE(HandshakeMessageType, HandshakeMultipartMessage, &data);
					default: uprintf("BAD HANDSHAKE MESSAGE TYPE\n");
				}
			} else {
				uprintf("BAD MESSAGE TYPE\n");
			}
			break;
		}
		DEBUG_LOGCASE(PacketProperty, MtuCheck, &data);
		DEBUG_LOGCASE(PacketProperty, MtuOk, &data);
		case PacketProperty_Broadcast: uprintf("\tPacketProperty_Broadcast\n"); return;
		case PacketProperty_Merged: {
			for(uint16_t len; data < end; data += len) {
				len = pkt_readUint16(version, &data);
				const uint8_t *sub = data;
				struct NetPacketHeader subheader = pkt_readNetPacketHeader(version, &sub);
				if(sub <= &data[len]) {
					if(filter(PacketProperty_Merged))
						uprintf("Merged[@%zu]:\n", sub - pkt);
					debug_logPacket(sub, &data[len], subheader, version, filter);
				}
			}
			if(filter(PacketProperty_Merged))
				uprintf("Merged[@%zu] end\n", data - pkt);
			break;
		}
		case PacketProperty_ShutdownOk: uprintf("\tPacketProperty_ShutdownOk\n"); return;
		case PacketProperty_PeerNotFound: uprintf("\tPacketProperty_PeerNotFound\n"); return;
		case PacketProperty_InvalidProtocol: uprintf("\tPacketProperty_InvalidProtocol\n"); return;
		case PacketProperty_NatMessage: uprintf("\tPacketProperty_NatMessage\n"); return;
		case PacketProperty_Empty: uprintf("\tPacketProperty_Empty\n"); return;
	}
	if(data != end) {
		uprintf("BAD PACKET LENGTH (expected %zu, read %zu)\n", end - pkt, data - pkt);
		if(data < end) {
			uprintf("\t");
			for(const uint8_t *it = pkt; it < end; ++it)
				uprintf("%02hhx", *it);
			uprintf("\n\t");
			for(const uint8_t *it = pkt; it < data; ++it)
				uprintf("  ");
			uprintf("^ extra data starts here\n");
		}
	}
}
#undef DEBUG_LOGCASE
#endif
