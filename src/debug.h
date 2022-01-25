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
static const uint8_t *debug_logRouting(const uint8_t *pkt, const uint8_t *data, const uint8_t *end, char *buf, uint32_t protocolVersion) {
	pkt_logRoutingHeader("\tRoutingHeader", buf, buf, pkt_readRoutingHeader(&data));
	while(data < end) {
		struct SerializeHeader serial = pkt_readSerializeHeader(&data);
		const uint8_t *sub = data--;
		data += serial.length;
		fprintf(stderr, "\tSerializeHeader[@%zu].type=%u (%s)\n", sub - pkt, serial.type, reflect(InternalMessageType, serial.type));
		switch(serial.type) {
			case InternalMessageType_SyncTime: pkt_logSyncTime("\tSyncTime", buf, buf, pkt_readSyncTime(&sub)); break;
			case InternalMessageType_PlayerConnected: pkt_logPlayerConnected("\tPlayerConnected", buf, buf, pkt_readPlayerConnected(&sub)); break;
			case InternalMessageType_PlayerIdentity: pkt_logPlayerIdentity("\tPlayerIdentity", buf, buf, pkt_readPlayerIdentity(&sub)); break;
			case InternalMessageType_PlayerLatencyUpdate: pkt_logPlayerLatencyUpdate("\tPlayerLatencyUpdate", buf, buf, pkt_readPlayerLatencyUpdate(&sub)); break;
			case InternalMessageType_PlayerDisconnected: pkt_logPlayerDisconnected("\tPlayerDisconnected", buf, buf, pkt_readPlayerDisconnected(&sub)); break;
			case InternalMessageType_PlayerSortOrderUpdate: pkt_logPlayerSortOrderUpdate("\tPlayerSortOrderUpdate", buf, buf, pkt_readPlayerSortOrderUpdate(&sub)); break;
			case InternalMessageType_Party: pkt_logParty("\tParty", buf, buf, pkt_readParty(&sub)); break;
			case InternalMessageType_MultiplayerSession: {
				struct MultiplayerSessionMessageHeader smsg = pkt_readMultiplayerSessionMessageHeader(&sub);
				pkt_logMultiplayerSessionMessageHeader("\tMultiplayerSessionMessageHeader", buf, buf, smsg);
				switch(smsg.type) {
					case MultiplayerSessionMessageType_MenuRpc: {
						struct MenuRpcHeader rpc = pkt_readMenuRpcHeader(&sub);
						pkt_logMenuRpcHeader("\tMenuRpcHeader", buf, buf, rpc);
						switch(rpc.type) {
							case MenuRpcType_SetPlayersMissingEntitlementsToLevel: pkt_logSetPlayersMissingEntitlementsToLevel("\tSetPlayersMissingEntitlementsToLevel", buf, buf, pkt_readSetPlayersMissingEntitlementsToLevel(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetIsEntitledToLevel: pkt_logGetIsEntitledToLevel("\tGetIsEntitledToLevel", buf, buf, pkt_readGetIsEntitledToLevel(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetIsEntitledToLevel: pkt_logSetIsEntitledToLevel("\tSetIsEntitledToLevel", buf, buf, pkt_readSetIsEntitledToLevel(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_InvalidateLevelEntitlementStatuses: pkt_logInvalidateLevelEntitlementStatuses("\tInvalidateLevelEntitlementStatuses", buf, buf, pkt_readInvalidateLevelEntitlementStatuses(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SelectLevelPack: pkt_logSelectLevelPack("\tSelectLevelPack", buf, buf, pkt_readSelectLevelPack(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetSelectedBeatmap: pkt_logSetSelectedBeatmap("\tSetSelectedBeatmap", buf, buf, pkt_readSetSelectedBeatmap(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetSelectedBeatmap: pkt_logGetSelectedBeatmap("\tGetSelectedBeatmap", buf, buf, pkt_readGetSelectedBeatmap(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_RecommendBeatmap: pkt_logRecommendBeatmap("\tRecommendBeatmap", buf, buf, pkt_readRecommendBeatmap(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_ClearRecommendedBeatmap: pkt_logClearRecommendedBeatmap("\tClearRecommendedBeatmap", buf, buf, pkt_readClearRecommendedBeatmap(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetRecommendedBeatmap: pkt_logGetRecommendedBeatmap("\tGetRecommendedBeatmap", buf, buf, pkt_readGetRecommendedBeatmap(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetSelectedGameplayModifiers: pkt_logSetSelectedGameplayModifiers("\tSetSelectedGameplayModifiers", buf, buf, pkt_readSetSelectedGameplayModifiers(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetSelectedGameplayModifiers: pkt_logGetSelectedGameplayModifiers("\tGetSelectedGameplayModifiers", buf, buf, pkt_readGetSelectedGameplayModifiers(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_RecommendGameplayModifiers: pkt_logRecommendGameplayModifiers("\tRecommendGameplayModifiers", buf, buf, pkt_readRecommendGameplayModifiers(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_ClearRecommendedGameplayModifiers: pkt_logClearRecommendedGameplayModifiers("\tClearRecommendedGameplayModifiers", buf, buf, pkt_readClearRecommendedGameplayModifiers(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetRecommendedGameplayModifiers: pkt_logGetRecommendedGameplayModifiers("\tGetRecommendedGameplayModifiers", buf, buf, pkt_readGetRecommendedGameplayModifiers(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_LevelLoadError: pkt_logLevelLoadError("\tLevelLoadError", buf, buf, pkt_readLevelLoadError(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_LevelLoadSuccess: pkt_logLevelLoadSuccess("\tLevelLoadSuccess", buf, buf, pkt_readLevelLoadSuccess(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_StartLevel: pkt_logStartLevel("\tStartLevel", buf, buf, pkt_readStartLevel(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetStartedLevel: pkt_logGetStartedLevel("\tGetStartedLevel", buf, buf, pkt_readGetStartedLevel(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_CancelLevelStart: pkt_logCancelLevelStart("\tCancelLevelStart", buf, buf, pkt_readCancelLevelStart(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetMultiplayerGameState: pkt_logGetMultiplayerGameState("\tGetMultiplayerGameState", buf, buf, pkt_readGetMultiplayerGameState(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetMultiplayerGameState: pkt_logSetMultiplayerGameState("\tSetMultiplayerGameState", buf, buf, pkt_readSetMultiplayerGameState(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetIsReady: pkt_logGetIsReady("\tGetIsReady", buf, buf, pkt_readGetIsReady(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetIsReady: pkt_logSetIsReady("\tSetIsReady", buf, buf, pkt_readSetIsReady(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetStartGameTime: pkt_logSetStartGameTime("\tSetStartGameTime", buf, buf, pkt_readSetStartGameTime(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_CancelStartGameTime: pkt_logCancelStartGameTime("\tCancelStartGameTime", buf, buf, pkt_readCancelStartGameTime(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetIsInLobby: pkt_logGetIsInLobby("\tGetIsInLobby", buf, buf, pkt_readGetIsInLobby(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetIsInLobby: pkt_logSetIsInLobby("\tSetIsInLobby", buf, buf, pkt_readSetIsInLobby(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetCountdownEndTime: pkt_logGetCountdownEndTime("\tGetCountdownEndTime", buf, buf, pkt_readGetCountdownEndTime(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetCountdownEndTime: pkt_logSetCountdownEndTime("\tSetCountdownEndTime", buf, buf, pkt_readSetCountdownEndTime(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_CancelCountdown: pkt_logCancelCountdown("\tCancelCountdown", buf, buf, pkt_readCancelCountdown(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetOwnedSongPacks: pkt_logGetOwnedSongPacks("\tGetOwnedSongPacks", buf, buf, pkt_readGetOwnedSongPacks(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetOwnedSongPacks: pkt_logSetOwnedSongPacks("\tSetOwnedSongPacks", buf, buf, pkt_readSetOwnedSongPacks(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_RequestKickPlayer: pkt_logRequestKickPlayer("\tRequestKickPlayer", buf, buf, pkt_readRequestKickPlayer(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetPermissionConfiguration: pkt_logGetPermissionConfiguration("\tGetPermissionConfiguration", buf, buf, pkt_readGetPermissionConfiguration(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetPermissionConfiguration: pkt_logSetPermissionConfiguration("\tSetPermissionConfiguration", buf, buf, pkt_readSetPermissionConfiguration(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_GetIsStartButtonEnabled: pkt_logGetIsStartButtonEnabled("\tGetIsStartButtonEnabled", buf, buf, pkt_readGetIsStartButtonEnabled(&sub, protocolVersion), protocolVersion); break;
							case MenuRpcType_SetIsStartButtonEnabled: pkt_logSetIsStartButtonEnabled("\tSetIsStartButtonEnabled", buf, buf, pkt_readSetIsStartButtonEnabled(&sub, protocolVersion), protocolVersion); break;
							default: fprintf(stderr, "BAD MENU RPC TYPE\n");
						}
						break;
					}
					case MultiplayerSessionMessageType_GameplayRpc: {
						struct GameplayRpcHeader rpc = pkt_readGameplayRpcHeader(&sub);
						pkt_logGameplayRpcHeader("\tGameplayRpcHeader", buf, buf, rpc);
						switch(rpc.type) {
							case GameplayRpcType_SetGameplaySceneSyncFinish: pkt_logSetGameplaySceneSyncFinish("\tSetGameplaySceneSyncFinish", buf, buf, pkt_readSetGameplaySceneSyncFinish(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_SetGameplaySceneReady: pkt_logSetGameplaySceneReady("\tSetGameplaySceneReady", buf, buf, pkt_readSetGameplaySceneReady(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_GetGameplaySceneReady: pkt_logGetGameplaySceneReady("\tGetGameplaySceneReady", buf, buf, pkt_readGetGameplaySceneReady(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_SetActivePlayerFailedToConnect: pkt_logSetActivePlayerFailedToConnect("\tSetActivePlayerFailedToConnect", buf, buf, pkt_readSetActivePlayerFailedToConnect(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_SetGameplaySongReady: pkt_logSetGameplaySongReady("\tSetGameplaySongReady", buf, buf, pkt_readSetGameplaySongReady(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_GetGameplaySongReady: pkt_logGetGameplaySongReady("\tGetGameplaySongReady", buf, buf, pkt_readGetGameplaySongReady(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_SetSongStartTime: pkt_logSetSongStartTime("\tSetSongStartTime", buf, buf, pkt_readSetSongStartTime(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_NoteCut: pkt_logNoteCut("\tNoteCut", buf, buf, pkt_readNoteCut(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_NoteMissed: pkt_logNoteMissed("\tNoteMissed", buf, buf, pkt_readNoteMissed(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_LevelFinished: pkt_logLevelFinished("\tLevelFinished", buf, buf, pkt_readLevelFinished(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_ReturnToMenu: pkt_logReturnToMenu("\tReturnToMenu", buf, buf, pkt_readReturnToMenu(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_RequestReturnToMenu: pkt_logRequestReturnToMenu("\tRequestReturnToMenu", buf, buf, pkt_readRequestReturnToMenu(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_NoteSpawned: pkt_logNoteSpawned("\tNoteSpawned", buf, buf, pkt_readNoteSpawned(&sub, protocolVersion), protocolVersion); break;
							case GameplayRpcType_ObstacleSpawned: pkt_logObstacleSpawned("\tObstacleSpawned", buf, buf, pkt_readObstacleSpawned(&sub, protocolVersion), protocolVersion); break;
							default: fprintf(stderr, "BAD GAMEPLAY RPC TYPE\n");
						}
						break;
					}
					#if 0
					case MultiplayerSessionMessageType_NodePoseSyncState: pkt_logNodePoseSyncState("\tNodePoseSyncState", buf, buf, pkt_readNodePoseSyncState(&sub)); break;
					case MultiplayerSessionMessageType_ScoreSyncState: pkt_logScoreSyncState("\tScoreSyncState", buf, buf, pkt_readScoreSyncState(&sub)); break;
					case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_logNodePoseSyncStateDelta("\tNodePoseSyncStateDelta", buf, buf, pkt_readNodePoseSyncStateDelta(&sub)); break;
					case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_logScoreSyncStateDelta("\tScoreSyncStateDelta", buf, buf, pkt_readScoreSyncStateDelta(&sub)); break;
					#else
					case MultiplayerSessionMessageType_NodePoseSyncState: pkt_readNodePoseSyncState(&sub); break;
					case MultiplayerSessionMessageType_ScoreSyncState: pkt_readScoreSyncState(&sub); break;
					case MultiplayerSessionMessageType_NodePoseSyncStateDelta: pkt_readNodePoseSyncStateDelta(&sub); break;
					case MultiplayerSessionMessageType_ScoreSyncStateDelta: pkt_readScoreSyncStateDelta(&sub); break;
					#endif
					case MultiplayerSessionMessageType_MpCore: {
						struct MpCore mpHeader = pkt_readMpCore(&sub);
						if(mpHeader.packetType.length == 15 && memcmp(mpHeader.packetType.data, "MpBeatmapPacket", 15) == 0) {
							pkt_logMpBeatmapPacket("\tMpBeatmapPacket", buf, buf, pkt_readMpBeatmapPacket(&sub));
						} else if(mpHeader.packetType.length == 12 && memcmp(mpHeader.packetType.data, "MpPlayerData", 12) == 0) {
							pkt_logMpPlayerData("\tMpPlayerData", buf, buf, pkt_readMpPlayerData(&sub));
						} else {
							fprintf(stderr, "[INSTANCE] BAD MPCORE MESSAGE TYPE: '%.*s'\n", mpHeader.packetType.length, mpHeader.packetType.data);
						}
						break;
					}
					default: fprintf(stderr, "BAD MULTIPLAYER SESSION MESSAGE TYPE\n");
				}
			}  break;
			case InternalMessageType_KickPlayer: pkt_logKickPlayer("\tKickPlayer", buf, buf, pkt_readKickPlayer(&sub)); break;
			case InternalMessageType_PlayerStateUpdate: pkt_logPlayerStateUpdate("\tPlayerStateUpdate", buf, buf, pkt_readPlayerStateUpdate(&sub)); break;
			case InternalMessageType_PlayerAvatarUpdate: pkt_logPlayerAvatarUpdate("\tPlayerAvatarUpdate", buf, buf, pkt_readPlayerAvatarUpdate(&sub)); break;
			case InternalMessageType_PingMessage: pkt_logPingMessage("\tPingMessage", buf, buf, pkt_readPingMessage(&sub)); break;
			case InternalMessageType_PongMessage: pkt_logPongMessage("\tPongMessage", buf, buf, pkt_readPongMessage(&sub)); break;
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
static void debug_logPacket(const uint8_t *pkt, const uint8_t *end, struct NetPacketHeader header, uint32_t protocolVersion) {
	if(header.isFragmented) {
		fprintf(stderr, "FRAGMENTED\n");
		return;
	}
	const uint8_t *data = pkt;
	char buf[1024*16];
	switch(header.property) {
		case PacketProperty_Unreliable: data = debug_logRouting(pkt, data, end, buf, protocolVersion); break;
		case PacketProperty_Channeled: {
			pkt_logChanneled("\tChanneled", buf, buf, pkt_readChanneled(&data));
			data = debug_logRouting(pkt, data, end, buf, protocolVersion);
			break;
		}
		case PacketProperty_Ack: pkt_logAck("\tAck", buf, buf, pkt_readAck(&data)); break;
		case PacketProperty_Ping: pkt_logPing("\tPing", buf, buf, pkt_readPing(&data)); break;
		case PacketProperty_Pong: pkt_logPong("\tPong", buf, buf, pkt_readPong(&data)); break;
		case PacketProperty_ConnectRequest: pkt_logConnectRequest("\tConnectRequest", buf, buf, pkt_readConnectRequest(&data)); break;
		case PacketProperty_ConnectAccept: pkt_logConnectAccept("\tConnectAccept", buf, buf, pkt_readConnectAccept(&data)); break;
		case PacketProperty_Disconnect: pkt_logDisconnect("\tDisconnect", buf, buf, pkt_readDisconnect(&data)); break;
		case PacketProperty_UnconnectedMessage: {
			struct MessageHeader message = pkt_readMessageHeader(&data);
			struct SerializeHeader serial = pkt_readSerializeHeader(&data);
			debug_logMessage(message, serial);
			if(message.type == MessageType_UserMessage) {
				switch(serial.type) {
					case UserMessageType_AuthenticateUserRequest: pkt_logAuthenticateUserRequest("\tAuthenticateUserRequest", buf, buf, pkt_readAuthenticateUserRequest(&data)); break;
					case UserMessageType_AuthenticateUserResponse: pkt_logAuthenticateUserResponse("\tAuthenticateUserResponse", buf, buf, pkt_readAuthenticateUserResponse(&data)); break;
					case UserMessageType_ConnectToServerResponse: pkt_logConnectToServerResponse("\tConnectToServerResponse", buf, buf, pkt_readConnectToServerResponse(&data)); break;
					case UserMessageType_ConnectToServerRequest: pkt_logConnectToServerRequest("\tConnectToServerRequest", buf, buf, pkt_readConnectToServerRequest(&data)); break;
					case UserMessageType_UserMessageReceivedAcknowledge: pkt_logUserMessageReceivedAcknowledge("\tUserMessageReceivedAcknowledge", buf, buf, pkt_readUserMessageReceivedAcknowledge(&data)); break;
					case UserMessageType_UserMultipartMessage: pkt_logUserMultipartMessage("\tUserMultipartMessage", buf, buf, pkt_readUserMultipartMessage(&data)); break;
					case UserMessageType_SessionKeepaliveMessage: pkt_logSessionKeepaliveMessage("\tSessionKeepaliveMessage", buf, buf, pkt_readSessionKeepaliveMessage(&data)); break;
					case UserMessageType_GetPublicServersRequest: pkt_logGetPublicServersRequest("\tGetPublicServersRequest", buf, buf, pkt_readGetPublicServersRequest(&data)); break;
					case UserMessageType_GetPublicServersResponse: pkt_logGetPublicServersResponse("\tGetPublicServersResponse", buf, buf, pkt_readGetPublicServersResponse(&data)); break;
					default: fprintf(stderr, "BAD USER MESSAGE TYPE\n");
				}
			} else if(message.type == MessageType_DedicatedServerMessage) {
				fprintf(stderr, "DedicatedServerMessageType not implemented\n");
			} else if(message.type == MessageType_HandshakeMessage) {
				switch(serial.type) {
					case HandshakeMessageType_ClientHelloRequest: pkt_logClientHelloRequest("\tClientHelloRequest", buf, buf, pkt_readClientHelloRequest(&data)); break;
					case HandshakeMessageType_HelloVerifyRequest: pkt_logHelloVerifyRequest("\tHelloVerifyRequest", buf, buf, pkt_readHelloVerifyRequest(&data)); break;
					case HandshakeMessageType_ClientHelloWithCookieRequest: pkt_logClientHelloWithCookieRequest("\tClientHelloWithCookieRequest", buf, buf, pkt_readClientHelloWithCookieRequest(&data)); break;
					case HandshakeMessageType_ServerHelloRequest: pkt_logServerHelloRequest("\tServerHelloRequest", buf, buf, pkt_readServerHelloRequest(&data)); break;
					case HandshakeMessageType_ServerCertificateRequest: pkt_logServerCertificateRequest("\tServerCertificateRequest", buf, buf, pkt_readServerCertificateRequest(&data)); break;
					case HandshakeMessageType_ClientKeyExchangeRequest: pkt_logClientKeyExchangeRequest("\tClientKeyExchangeRequest", buf, buf, pkt_readClientKeyExchangeRequest(&data)); break;
					case HandshakeMessageType_ChangeCipherSpecRequest: pkt_logChangeCipherSpecRequest("\tChangeCipherSpecRequest", buf, buf, pkt_readChangeCipherSpecRequest(&data)); break;
					case HandshakeMessageType_HandshakeMessageReceivedAcknowledge: pkt_logHandshakeMessageReceivedAcknowledge("\tHandshakeMessageReceivedAcknowledge", buf, buf, pkt_readHandshakeMessageReceivedAcknowledge(&data)); break;
					case HandshakeMessageType_HandshakeMultipartMessage: pkt_logHandshakeMultipartMessage("\tHandshakeMultipartMessage", buf, buf, pkt_readHandshakeMultipartMessage(&data)); break;
					default: fprintf(stderr, "BAD HANDSHAKE MESSAGE TYPE\n");
				}
			} else {
				fprintf(stderr, "BAD MESSAGE TYPE\n");
			}
			break;
		}
		case PacketProperty_MtuCheck: pkt_logMtuCheck("\tMtuCheck", buf, buf, pkt_readMtuCheck(&data)); break;
		case PacketProperty_MtuOk: pkt_logMtuOk("\tMtuOk", buf, buf, pkt_readMtuOk(&data)); break;
		case PacketProperty_Broadcast: fprintf(stderr, "\tPacketProperty_Broadcast\n"); return;
		case PacketProperty_Merged: {
			for(uint16_t len; data < end; data += len) {
				len = pkt_readUint16(&data);
				const uint8_t *sub = data;
				struct NetPacketHeader subheader = pkt_readNetPacketHeader(&sub);
				if(sub <= &data[len]) {
					fprintf(stderr, "Merged[@%zu]:\n", sub - pkt);
					debug_logPacket(sub, &data[len], subheader, protocolVersion);
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
