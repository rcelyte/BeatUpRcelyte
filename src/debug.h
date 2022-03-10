#include <stdio.h>

static void debug_logMessage(struct MessageHeader message, struct SerializeHeader serial) {
	fprintf(stderr, "\tMessageHeader.type=%u (%s)\n", message.type, reflect(MessageType, message.type));
	fprintf(stderr, "\tMessageHeader.protocolVersion=%u\n", message.protocolVersion);
	fprintf(stderr, "\tSerializeHeader.length=%u\n", serial.length);
	if(message.type == MessageType_UserMessage)
		fprintf(stderr, "\tSerializeHeader.type=%u (%s)\n", serial.type, reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		fprintf(stderr, "\tSerializeHeader.type=%u (%s)\n", serial.type, reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		fprintf(stderr, "\tSerializeHeader.type=%u (%s)\n", serial.type, reflect(HandshakeMessageType, serial.type));
}

static void debug_logType(struct MessageHeader message, struct SerializeHeader serial) {
	if(message.type == MessageType_UserMessage)
		fprintf(stderr, "recieve UserMessageType_%s\n", reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		fprintf(stderr, "recieve DedicatedServerMessageType_%s\n", reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		fprintf(stderr, "recieve HandshakeMessageType_%s\n", reflect(HandshakeMessageType, serial.type));
}

#ifdef PACKET_LOGGING_FUNCS
/*static void pkt_logUint8Array(const char *name, char *buf, char *it, const uint8_t *in, uint32_t count) {
	fprintf(stderr, "%.*s%s=", (uint32_t)(it - buf), buf, name);
	for(uint32_t i = 0; i < count; ++i)
		fprintf(stderr, "%02hhx", in[i]);
	fprintf(stderr, "\n");
}*/
static const uint8_t *debug_logRouting(const uint8_t *pkt, const uint8_t *data, const uint8_t *end, char *buf, struct PacketContext version) {
	pkt_logRoutingHeader(version, "\tRoutingHeader", buf, buf, pkt_readRoutingHeader(version, &data));
	while(data < end) {
		struct SerializeHeader serial = pkt_readSerializeHeader(version, &data);
		const uint8_t *sub = data--;
		data += serial.length;
		fprintf(stderr, "\tSerializeHeader[@%zu].type=%u (%s)\n", sub - pkt, serial.type, reflect(InternalMessageType, serial.type));
		switch(serial.type) {
			case InternalMessageType_SyncTime: pkt_logSyncTime(version, "\tSyncTime", buf, buf, pkt_readSyncTime(version, &sub)); break;
			case InternalMessageType_PlayerConnected: pkt_logPlayerConnected(version, "\tPlayerConnected", buf, buf, pkt_readPlayerConnected(version, &sub)); break;
			case InternalMessageType_PlayerIdentity: pkt_logPlayerIdentity(version, "\tPlayerIdentity", buf, buf, pkt_readPlayerIdentity(version, &sub)); break;
			case InternalMessageType_PlayerLatencyUpdate: pkt_logPlayerLatencyUpdate(version, "\tPlayerLatencyUpdate", buf, buf, pkt_readPlayerLatencyUpdate(version, &sub)); break;
			case InternalMessageType_PlayerDisconnected: pkt_logPlayerDisconnected(version, "\tPlayerDisconnected", buf, buf, pkt_readPlayerDisconnected(version, &sub)); break;
			case InternalMessageType_PlayerSortOrderUpdate: pkt_logPlayerSortOrderUpdate(version, "\tPlayerSortOrderUpdate", buf, buf, pkt_readPlayerSortOrderUpdate(version, &sub)); break;
			case InternalMessageType_Party: pkt_logParty(version, "\tParty", buf, buf, pkt_readParty(version, &sub)); break;
			case InternalMessageType_MultiplayerSession: {
				struct MultiplayerSessionMessageHeader smsg = pkt_readMultiplayerSessionMessageHeader(version, &sub);
				pkt_logMultiplayerSessionMessageHeader(version, "\tMultiplayerSessionMessageHeader", buf, buf, smsg);
				switch(smsg.type) {
					case MultiplayerSessionMessageType_MenuRpc: {
						struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(version, &sub);
						pkt_logMenuRpcHeader(version, "\tMenuRpcHeader", buf, buf, rpc);
						switch(rpc.type) {
							case MenuRpcType_SetPlayersMissingEntitlementsToLevel: pkt_logSetPlayersMissingEntitlementsToLevel(version, "\tSetPlayersMissingEntitlementsToLevel", buf, buf, pkt_readSetPlayersMissingEntitlementsToLevel(version, &sub)); break;
							case MenuRpcType_GetIsEntitledToLevel: pkt_logGetIsEntitledToLevel(version, "\tGetIsEntitledToLevel", buf, buf, pkt_readGetIsEntitledToLevel(version, &sub)); break;
							case MenuRpcType_SetIsEntitledToLevel: pkt_logSetIsEntitledToLevel(version, "\tSetIsEntitledToLevel", buf, buf, pkt_readSetIsEntitledToLevel(version, &sub)); break;
							case MenuRpcType_InvalidateLevelEntitlementStatuses: pkt_logInvalidateLevelEntitlementStatuses(version, "\tInvalidateLevelEntitlementStatuses", buf, buf, pkt_readInvalidateLevelEntitlementStatuses(version, &sub)); break;
							case MenuRpcType_SelectLevelPack: pkt_logSelectLevelPack(version, "\tSelectLevelPack", buf, buf, pkt_readSelectLevelPack(version, &sub)); break;
							case MenuRpcType_SetSelectedBeatmap: pkt_logSetSelectedBeatmap(version, "\tSetSelectedBeatmap", buf, buf, pkt_readSetSelectedBeatmap(version, &sub)); break;
							case MenuRpcType_GetSelectedBeatmap: pkt_logGetSelectedBeatmap(version, "\tGetSelectedBeatmap", buf, buf, pkt_readGetSelectedBeatmap(version, &sub)); break;
							case MenuRpcType_RecommendBeatmap: pkt_logRecommendBeatmap(version, "\tRecommendBeatmap", buf, buf, pkt_readRecommendBeatmap(version, &sub)); break;
							case MenuRpcType_ClearRecommendedBeatmap: pkt_logClearRecommendedBeatmap(version, "\tClearRecommendedBeatmap", buf, buf, pkt_readClearRecommendedBeatmap(version, &sub)); break;
							case MenuRpcType_GetRecommendedBeatmap: pkt_logGetRecommendedBeatmap(version, "\tGetRecommendedBeatmap", buf, buf, pkt_readGetRecommendedBeatmap(version, &sub)); break;
							case MenuRpcType_SetSelectedGameplayModifiers: pkt_logSetSelectedGameplayModifiers(version, "\tSetSelectedGameplayModifiers", buf, buf, pkt_readSetSelectedGameplayModifiers(version, &sub)); break;
							case MenuRpcType_GetSelectedGameplayModifiers: pkt_logGetSelectedGameplayModifiers(version, "\tGetSelectedGameplayModifiers", buf, buf, pkt_readGetSelectedGameplayModifiers(version, &sub)); break;
							case MenuRpcType_RecommendGameplayModifiers: pkt_logRecommendGameplayModifiers(version, "\tRecommendGameplayModifiers", buf, buf, pkt_readRecommendGameplayModifiers(version, &sub)); break;
							case MenuRpcType_ClearRecommendedGameplayModifiers: pkt_logClearRecommendedGameplayModifiers(version, "\tClearRecommendedGameplayModifiers", buf, buf, pkt_readClearRecommendedGameplayModifiers(version, &sub)); break;
							case MenuRpcType_GetRecommendedGameplayModifiers: pkt_logGetRecommendedGameplayModifiers(version, "\tGetRecommendedGameplayModifiers", buf, buf, pkt_readGetRecommendedGameplayModifiers(version, &sub)); break;
							case MenuRpcType_LevelLoadError: pkt_logLevelLoadError(version, "\tLevelLoadError", buf, buf, pkt_readLevelLoadError(version, &sub)); break;
							case MenuRpcType_LevelLoadSuccess: pkt_logLevelLoadSuccess(version, "\tLevelLoadSuccess", buf, buf, pkt_readLevelLoadSuccess(version, &sub)); break;
							case MenuRpcType_StartLevel: pkt_logStartLevel(version, "\tStartLevel", buf, buf, pkt_readStartLevel(version, &sub)); break;
							case MenuRpcType_GetStartedLevel: pkt_logGetStartedLevel(version, "\tGetStartedLevel", buf, buf, pkt_readGetStartedLevel(version, &sub)); break;
							case MenuRpcType_CancelLevelStart: pkt_logCancelLevelStart(version, "\tCancelLevelStart", buf, buf, pkt_readCancelLevelStart(version, &sub)); break;
							case MenuRpcType_GetMultiplayerGameState: pkt_logGetMultiplayerGameState(version, "\tGetMultiplayerGameState", buf, buf, pkt_readGetMultiplayerGameState(version, &sub)); break;
							case MenuRpcType_SetMultiplayerGameState: pkt_logSetMultiplayerGameState(version, "\tSetMultiplayerGameState", buf, buf, pkt_readSetMultiplayerGameState(version, &sub)); break;
							case MenuRpcType_GetIsReady: pkt_logGetIsReady(version, "\tGetIsReady", buf, buf, pkt_readGetIsReady(version, &sub)); break;
							case MenuRpcType_SetIsReady: pkt_logSetIsReady(version, "\tSetIsReady", buf, buf, pkt_readSetIsReady(version, &sub)); break;
							case MenuRpcType_SetStartGameTime: pkt_logSetStartGameTime(version, "\tSetStartGameTime", buf, buf, pkt_readSetStartGameTime(version, &sub)); break;
							case MenuRpcType_CancelStartGameTime: pkt_logCancelStartGameTime(version, "\tCancelStartGameTime", buf, buf, pkt_readCancelStartGameTime(version, &sub)); break;
							case MenuRpcType_GetIsInLobby: pkt_logGetIsInLobby(version, "\tGetIsInLobby", buf, buf, pkt_readGetIsInLobby(version, &sub)); break;
							case MenuRpcType_SetIsInLobby: pkt_logSetIsInLobby(version, "\tSetIsInLobby", buf, buf, pkt_readSetIsInLobby(version, &sub)); break;
							case MenuRpcType_GetCountdownEndTime: pkt_logGetCountdownEndTime(version, "\tGetCountdownEndTime", buf, buf, pkt_readGetCountdownEndTime(version, &sub)); break;
							case MenuRpcType_SetCountdownEndTime: pkt_logSetCountdownEndTime(version, "\tSetCountdownEndTime", buf, buf, pkt_readSetCountdownEndTime(version, &sub)); break;
							case MenuRpcType_CancelCountdown: pkt_logCancelCountdown(version, "\tCancelCountdown", buf, buf, pkt_readCancelCountdown(version, &sub)); break;
							case MenuRpcType_GetOwnedSongPacks: pkt_logGetOwnedSongPacks(version, "\tGetOwnedSongPacks", buf, buf, pkt_readGetOwnedSongPacks(version, &sub)); break;
							case MenuRpcType_SetOwnedSongPacks: pkt_logSetOwnedSongPacks(version, "\tSetOwnedSongPacks", buf, buf, pkt_readSetOwnedSongPacks(version, &sub)); break;
							case MenuRpcType_RequestKickPlayer: pkt_logRequestKickPlayer(version, "\tRequestKickPlayer", buf, buf, pkt_readRequestKickPlayer(version, &sub)); break;
							case MenuRpcType_GetPermissionConfiguration: pkt_logGetPermissionConfiguration(version, "\tGetPermissionConfiguration", buf, buf, pkt_readGetPermissionConfiguration(version, &sub)); break;
							case MenuRpcType_SetPermissionConfiguration: pkt_logSetPermissionConfiguration(version, "\tSetPermissionConfiguration", buf, buf, pkt_readSetPermissionConfiguration(version, &sub)); break;
							case MenuRpcType_GetIsStartButtonEnabled: pkt_logGetIsStartButtonEnabled(version, "\tGetIsStartButtonEnabled", buf, buf, pkt_readGetIsStartButtonEnabled(version, &sub)); break;
							case MenuRpcType_SetIsStartButtonEnabled: pkt_logSetIsStartButtonEnabled(version, "\tSetIsStartButtonEnabled", buf, buf, pkt_readSetIsStartButtonEnabled(version, &sub)); break;
							default: fprintf(stderr, "BAD MENU RPC TYPE\n");
						}
						break;
					}
					case MultiplayerSessionMessageType_GameplayRpc: {
						struct GameplayRpcHeader rpc = pkt_readGameplayRpcHeader(version, &sub);
						pkt_logGameplayRpcHeader(version, "\tGameplayRpcHeader", buf, buf, rpc);
						switch(rpc.type) {
							case GameplayRpcType_SetGameplaySceneSyncFinish: pkt_logSetGameplaySceneSyncFinish(version, "\tSetGameplaySceneSyncFinish", buf, buf, pkt_readSetGameplaySceneSyncFinish(version, &sub)); break;
							case GameplayRpcType_SetGameplaySceneReady: pkt_logSetGameplaySceneReady(version, "\tSetGameplaySceneReady", buf, buf, pkt_readSetGameplaySceneReady(version, &sub)); break;
							case GameplayRpcType_GetGameplaySceneReady: pkt_logGetGameplaySceneReady(version, "\tGetGameplaySceneReady", buf, buf, pkt_readGetGameplaySceneReady(version, &sub)); break;
							case GameplayRpcType_SetActivePlayerFailedToConnect: pkt_logSetActivePlayerFailedToConnect(version, "\tSetActivePlayerFailedToConnect", buf, buf, pkt_readSetActivePlayerFailedToConnect(version, &sub)); break;
							case GameplayRpcType_SetGameplaySongReady: pkt_logSetGameplaySongReady(version, "\tSetGameplaySongReady", buf, buf, pkt_readSetGameplaySongReady(version, &sub)); break;
							case GameplayRpcType_GetGameplaySongReady: pkt_logGetGameplaySongReady(version, "\tGetGameplaySongReady", buf, buf, pkt_readGetGameplaySongReady(version, &sub)); break;
							case GameplayRpcType_SetSongStartTime: pkt_logSetSongStartTime(version, "\tSetSongStartTime", buf, buf, pkt_readSetSongStartTime(version, &sub)); break;
							case GameplayRpcType_NoteCut: pkt_logNoteCut(version, "\tNoteCut", buf, buf, pkt_readNoteCut(version, &sub)); break;
							case GameplayRpcType_NoteMissed: pkt_logNoteMissed(version, "\tNoteMissed", buf, buf, pkt_readNoteMissed(version, &sub)); break;
							case GameplayRpcType_LevelFinished: pkt_logLevelFinished(version, "\tLevelFinished", buf, buf, pkt_readLevelFinished(version, &sub)); break;
							case GameplayRpcType_ReturnToMenu: pkt_logReturnToMenu(version, "\tReturnToMenu", buf, buf, pkt_readReturnToMenu(version, &sub)); break;
							case GameplayRpcType_RequestReturnToMenu: pkt_logRequestReturnToMenu(version, "\tRequestReturnToMenu", buf, buf, pkt_readRequestReturnToMenu(version, &sub)); break;
							case GameplayRpcType_NoteSpawned: pkt_logNoteSpawned(version, "\tNoteSpawned", buf, buf, pkt_readNoteSpawned(version, &sub)); break;
							case GameplayRpcType_ObstacleSpawned: pkt_logObstacleSpawned(version, "\tObstacleSpawned", buf, buf, pkt_readObstacleSpawned(version, &sub)); break;
							case GameplayRpcType_SliderSpawned: pkt_logSliderSpawned(version, "\tSliderSpawned", buf, buf, pkt_readSliderSpawned(version, &sub)); break;
							default: fprintf(stderr, "BAD GAMEPLAY RPC TYPE\n");
						}
						break;
					}
					#if 0
					case MultiplayerSessionMessageType_NodePoseSyncState: pkt_logNodePoseSyncState(version, "\tNodePoseSyncState", buf, buf, pkt_readNodePoseSyncState(version, &sub)); break;
					case MultiplayerSessionMessageType_ScoreSyncState: pkt_logScoreSyncState(version, "\tScoreSyncState", buf, buf, pkt_readScoreSyncState(version, &sub)); break;
					case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_logNodePoseSyncStateDelta(version, "\tNodePoseSyncStateDelta", buf, buf, pkt_readNodePoseSyncStateDelta(version, &sub)); break;
					case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_logScoreSyncStateDelta(version, "\tScoreSyncStateDelta", buf, buf, pkt_readScoreSyncStateDelta(version, &sub)); break;
					#else
					case MultiplayerSessionMessageType_NodePoseSyncState: pkt_readNodePoseSyncState(version, &sub); break;
					case MultiplayerSessionMessageType_ScoreSyncState: pkt_readScoreSyncState(version, &sub); break;
					case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_readNodePoseSyncStateDelta(version, &sub); break;
					case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_readScoreSyncStateDelta(version, &sub); break;
					#endif
					case MultiplayerSessionMessageType_MpCore: {
						struct MpCore mpHeader = pkt_readMpCore(version, &sub);
						if(mpHeader.packetType.length == 15 && memcmp(mpHeader.packetType.data, "MpBeatmapPacket", 15) == 0) {
							pkt_logMpBeatmapPacket(version, "\tMpBeatmapPacket", buf, buf, pkt_readMpBeatmapPacket(version, &sub));
						} else if(mpHeader.packetType.length == 12 && memcmp(mpHeader.packetType.data, "MpPlayerData", 12) == 0) {
							pkt_logMpPlayerData(version, "\tMpPlayerData", buf, buf, pkt_readMpPlayerData(version, &sub));
						} else {
							fprintf(stderr, "[INSTANCE] BAD MPCORE MESSAGE TYPE: '%.*s'\n", mpHeader.packetType.length, mpHeader.packetType.data);
						}
						break;
					}
					default: fprintf(stderr, "BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
				}
			}  break;
			case InternalMessageType_KickPlayer: pkt_logKickPlayer(version, "\tKickPlayer", buf, buf, pkt_readKickPlayer(version, &sub)); break;
			case InternalMessageType_PlayerStateUpdate: pkt_logPlayerStateUpdate(version, "\tPlayerStateUpdate", buf, buf, pkt_readPlayerStateUpdate(version, &sub)); break;
			case InternalMessageType_PlayerAvatarUpdate: pkt_logPlayerAvatarUpdate(version, "\tPlayerAvatarUpdate", buf, buf, pkt_readPlayerAvatarUpdate(version, &sub)); break;
			case InternalMessageType_PingMessage: pkt_logPingMessage(version, "\tPingMessage", buf, buf, pkt_readPingMessage(version, &sub)); break;
			case InternalMessageType_PongMessage: pkt_logPongMessage(version, "\tPongMessage", buf, buf, pkt_readPongMessage(version, &sub)); break;
			default: fprintf(stderr, "BAD INTERNAL MESSAGE TYPE\n"); continue;
		}
		if(sub != data) {
			fprintf(stderr, "BAD INTERNAL MESSAGE LENGTH (expected %u, read %zu)\n", serial.length, sub - (data - serial.length));
			if(sub < data) {
				fprintf(stderr, "\t");
				for(const uint8_t *it = data - serial.length; it < data; ++it)
					fprintf(stderr, "%02hhx", *it);
				fprintf(stderr, "\n\t");
				for(const uint8_t *it = data - serial.length; it < sub; ++it)
					fprintf(stderr, "  ");
				fprintf(stderr, "^ extra data starts here");
			}
			fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "\tSerializeHeader[@%zu] end\n", data - pkt);
	return data;
}
static void debug_logPacket(const uint8_t *pkt, const uint8_t *end, struct NetPacketHeader header, struct PacketContext version) {
	if(header.isFragmented) {
		fprintf(stderr, "FRAGMENTED\n");
		return;
	}
	const uint8_t *data = pkt;
	char buf[1024*16];
	switch(header.property) {
		case PacketProperty_Unreliable: data = debug_logRouting(pkt, data, end, buf, version); break;
		case PacketProperty_Channeled: {
			pkt_logChanneled(version, "\tChanneled", buf, buf, pkt_readChanneled(version, &data));
			data = debug_logRouting(pkt, data, end, buf, version);
			break;
		}
		case PacketProperty_Ack: pkt_logAck(version, "\tAck", buf, buf, pkt_readAck(version, &data)); break;
		case PacketProperty_Ping: pkt_logPing(version, "\tPing", buf, buf, pkt_readPing(version, &data)); break;
		case PacketProperty_Pong: pkt_logPong(version, "\tPong", buf, buf, pkt_readPong(version, &data)); break;
		case PacketProperty_ConnectRequest: pkt_logConnectRequest(version, "\tConnectRequest", buf, buf, pkt_readConnectRequest(version, &data)); break;
		case PacketProperty_ConnectAccept: pkt_logConnectAccept(version, "\tConnectAccept", buf, buf, pkt_readConnectAccept(version, &data)); break;
		case PacketProperty_Disconnect: pkt_logDisconnect(version, "\tDisconnect", buf, buf, pkt_readDisconnect(version, &data)); break;
		case PacketProperty_UnconnectedMessage: {
			struct MessageHeader message = pkt_readMessageHeader(version, &data);
			struct SerializeHeader serial = pkt_readSerializeHeader(version, &data);
			debug_logMessage(message, serial);
			if(message.type == MessageType_UserMessage) {
				switch(serial.type) {
					case UserMessageType_AuthenticateUserRequest: pkt_logAuthenticateUserRequest(version, "\tAuthenticateUserRequest", buf, buf, pkt_readAuthenticateUserRequest(version, &data)); break;
					case UserMessageType_AuthenticateUserResponse: pkt_logAuthenticateUserResponse(version, "\tAuthenticateUserResponse", buf, buf, pkt_readAuthenticateUserResponse(version, &data)); break;
					case UserMessageType_ConnectToServerResponse: pkt_logConnectToServerResponse(version, "\tConnectToServerResponse", buf, buf, pkt_readConnectToServerResponse(version, &data)); break;
					case UserMessageType_ConnectToServerRequest: pkt_logConnectToServerRequest(version, "\tConnectToServerRequest", buf, buf, pkt_readConnectToServerRequest(version, &data)); break;
					case UserMessageType_UserMessageReceivedAcknowledge: pkt_logUserMessageReceivedAcknowledge(version, "\tUserMessageReceivedAcknowledge", buf, buf, pkt_readUserMessageReceivedAcknowledge(version, &data)); break;
					case UserMessageType_UserMultipartMessage: pkt_logUserMultipartMessage(version, "\tUserMultipartMessage", buf, buf, pkt_readUserMultipartMessage(version, &data)); break;
					case UserMessageType_SessionKeepaliveMessage: pkt_logSessionKeepaliveMessage(version, "\tSessionKeepaliveMessage", buf, buf, pkt_readSessionKeepaliveMessage(version, &data)); break;
					case UserMessageType_GetPublicServersRequest: pkt_logGetPublicServersRequest(version, "\tGetPublicServersRequest", buf, buf, pkt_readGetPublicServersRequest(version, &data)); break;
					case UserMessageType_GetPublicServersResponse: pkt_logGetPublicServersResponse(version, "\tGetPublicServersResponse", buf, buf, pkt_readGetPublicServersResponse(version, &data)); break;
					default: fprintf(stderr, "BAD USER MESSAGE TYPE\n");
				}
			} else if(message.type == MessageType_DedicatedServerMessage) {
				fprintf(stderr, "DedicatedServerMessageType not implemented\n");
			} else if(message.type == MessageType_HandshakeMessage) {
				switch(serial.type) {
					case HandshakeMessageType_ClientHelloRequest: pkt_logClientHelloRequest(version, "\tClientHelloRequest", buf, buf, pkt_readClientHelloRequest(version, &data)); break;
					case HandshakeMessageType_HelloVerifyRequest: pkt_logHelloVerifyRequest(version, "\tHelloVerifyRequest", buf, buf, pkt_readHelloVerifyRequest(version, &data)); break;
					case HandshakeMessageType_ClientHelloWithCookieRequest: pkt_logClientHelloWithCookieRequest(version, "\tClientHelloWithCookieRequest", buf, buf, pkt_readClientHelloWithCookieRequest(version, &data)); break;
					case HandshakeMessageType_ServerHelloRequest: pkt_logServerHelloRequest(version, "\tServerHelloRequest", buf, buf, pkt_readServerHelloRequest(version, &data)); break;
					case HandshakeMessageType_ServerCertificateRequest: pkt_logServerCertificateRequest(version, "\tServerCertificateRequest", buf, buf, pkt_readServerCertificateRequest(version, &data)); break;
					case HandshakeMessageType_ClientKeyExchangeRequest: pkt_logClientKeyExchangeRequest(version, "\tClientKeyExchangeRequest", buf, buf, pkt_readClientKeyExchangeRequest(version, &data)); break;
					case HandshakeMessageType_ChangeCipherSpecRequest: pkt_logChangeCipherSpecRequest(version, "\tChangeCipherSpecRequest", buf, buf, pkt_readChangeCipherSpecRequest(version, &data)); break;
					case HandshakeMessageType_HandshakeMessageReceivedAcknowledge: pkt_logHandshakeMessageReceivedAcknowledge(version, "\tHandshakeMessageReceivedAcknowledge", buf, buf, pkt_readHandshakeMessageReceivedAcknowledge(version, &data)); break;
					case HandshakeMessageType_HandshakeMultipartMessage: pkt_logHandshakeMultipartMessage(version, "\tHandshakeMultipartMessage", buf, buf, pkt_readHandshakeMultipartMessage(version, &data)); break;
					default: fprintf(stderr, "BAD HANDSHAKE MESSAGE TYPE\n");
				}
			} else {
				fprintf(stderr, "BAD MESSAGE TYPE\n");
			}
			break;
		}
		case PacketProperty_MtuCheck: pkt_logMtuCheck(version, "\tMtuCheck", buf, buf, pkt_readMtuCheck(version, &data)); break;
		case PacketProperty_MtuOk: pkt_logMtuOk(version, "\tMtuOk", buf, buf, pkt_readMtuOk(version, &data)); break;
		case PacketProperty_Broadcast: fprintf(stderr, "\tPacketProperty_Broadcast\n"); return;
		case PacketProperty_Merged: {
			for(uint16_t len; data < end; data += len) {
				len = pkt_readUint16(version, &data);
				const uint8_t *sub = data;
				struct NetPacketHeader subheader = pkt_readNetPacketHeader(version, &sub);
				if(sub <= &data[len]) {
					fprintf(stderr, "Merged[@%zu]:\n", sub - pkt);
					debug_logPacket(sub, &data[len], subheader, version);
				}
			}
			fprintf(stderr, "Merged[@%zu] end\n", data - pkt);
			break;
		}
		case PacketProperty_ShutdownOk: fprintf(stderr, "\tPacketProperty_ShutdownOk\n"); return;
		case PacketProperty_PeerNotFound: fprintf(stderr, "\tPacketProperty_PeerNotFound\n"); return;
		case PacketProperty_InvalidProtocol: fprintf(stderr, "\tPacketProperty_InvalidProtocol\n"); return;
		case PacketProperty_NatMessage: fprintf(stderr, "\tPacketProperty_NatMessage\n"); return;
		case PacketProperty_Empty: fprintf(stderr, "\tPacketProperty_Empty\n"); return;
	}
	if(data != end) {
		fprintf(stderr, "BAD PACKET LENGTH (expected %zu, read %zu)\n", end - pkt, data - pkt);
		if(data < end) {
			fprintf(stderr, "\t");
			for(const uint8_t *it = pkt; it < end; ++it)
				fprintf(stderr, "%02hhx", *it);
			fprintf(stderr, "\n\t");
			for(const uint8_t *it = pkt; it < data; ++it)
				fprintf(stderr, "  ");
			fprintf(stderr, "^ extra data starts here\n");
		}
	}
}
#endif
