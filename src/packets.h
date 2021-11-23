#include <stdint.h>
#include <stdbool.h>
#include "enum.h"

ENUM(uint8_t, PacketProperty, {
	PacketProperty_Unreliable,
	PacketProperty_Channeled,
	PacketProperty_Ack,
	PacketProperty_Ping,
	PacketProperty_Pong,
	PacketProperty_ConnectRequest,
	PacketProperty_ConnectAccept,
	PacketProperty_Disconnect,
	PacketProperty_UnconnectedMessage,
	PacketProperty_MtuCheck,
	PacketProperty_MtuOk,
	PacketProperty_Broadcast,
	PacketProperty_Merged,
	PacketProperty_ShutdownOk,
	PacketProperty_PeerNotFound,
	PacketProperty_InvalidProtocol,
	PacketProperty_NatMessage,
	PacketProperty_Empty,
})
ENUM(uint32_t, MessageType, {
	MessageType_UserMessage = 1,
	MessageType_DedicatedServerMessage = 2,
	MessageType_HandshakeMessage = 3192347326u,
})
ENUM(uint8_t, HandshakeMessageType, {
	HandshakeMessageType_ClientHelloRequest = 0,
	HandshakeMessageType_HelloVerifyRequest = 1,
	HandshakeMessageType_ClientHelloWithCookieRequest = 2, // recv reliable
	HandshakeMessageType_ServerHelloRequest = 3, // send reliable
	HandshakeMessageType_ServerCertificateRequest = 4, // send reliable
	HandshakeMessageType_ServerCertificateResponse = 5,
	HandshakeMessageType_ClientKeyExchangeRequest = 6, // send reliable
	HandshakeMessageType_ChangeCipherSpecRequest = 7, // send reliable
	HandshakeMessageType_MessageReceivedAcknowledge = 8,
	HandshakeMessageType_MultipartMessage = 9, // send/recv reliable
})
ENUM(uint8_t, UserMessageType, {
	UserMessageType_AuthenticateUserRequest, // reliable
	UserMessageType_AuthenticateUserResponse, // reliable
	UserMessageType_ConnectToServerResponse = 8, // reliable
	UserMessageType_ConnectToServerRequest, // reliable
	UserMessageType_MessageReceivedAcknowledge = 13,
	UserMessageType_MultipartMessage, // send/recv reliable
	UserMessageType_SessionKeepaliveMessage,
	UserMessageType_GetPublicServersRequest, // reliable
	UserMessageType_GetPublicServersResponse, // reliable
})
ENUM(uint8_t, DedicatedServerMessageType, {
	DedicatedServerMessageType_AuthenticateDedicatedServerRequest, // reliable
	DedicatedServerMessageType_AuthenticateDedicatedServerResponse, // reliable
	DedicatedServerMessageType_CreateDedicatedServerInstanceRequest = 4, // reliable
	DedicatedServerMessageType_CreateDedicatedServerInstanceResponse, // reliable
	DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest, // reliable
	DedicatedServerMessageType_DedicatedServerHeartbeatRequest,
	DedicatedServerMessageType_DedicatedServerHeartbeatResponse,
	DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest = 10, // reliable
	DedicatedServerMessageType_DedicatedServerShutDownRequest,
	DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest, // reliable
	DedicatedServerMessageType_MessageReceivedAcknowledge,
	DedicatedServerMessageType_MultipartMessage, // send/recv reliable
	DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse, // reliable
})
/*ENUM(uint8_t, DiscoveryPolicy, {
	DiscoveryPolicy_Hidden = 0,
	DiscoveryPolicy_WithCode = 1,
	DiscoveryPolicy_Public = 2
})*/
/*ENUM(uint8_t, InvitePolicy, {
	InvitePolicy_OnlyConnectionOwnerCanInvite = 0,
	InvitePolicy_AnyoneCanInvite = 1,
})*/
/*ENUM(uint8_t, GameplayServerMode, {
	GameplayServerMode_Countdown = 0,
	GameplayServerMode_Managed = 1,
	GameplayServerMode_QuickStartOneSong = 2,
})*/
/*ENUM(uint8_t, SongSelectionMode, {
	SongSelectionMode_Vote = 0,
	SongSelectionMode_Random = 1,
	SongSelectionMode_OwnerPicks = 2,
	SongSelectionMode_RandomPlayerPicks = 3,
})*/
/*ENUM(uint8_t, GameplayServerControlSettings, {
	GameplayServerControlSettings_None = 0,
	GameplayServerControlSettings_AllowModifierSelection = 1,
	GameplayServerControlSettings_AllowSpectate = 2,
	GameplayServerControlSettings_All = 3,
})*/
/*ENUM(uint8_t, BeatmapDifficultyMask, {
	BeatmapDifficultyMask_Easy = 1,
	BeatmapDifficultyMask_Normal = 2,
	BeatmapDifficultyMask_Hard = 4,
	BeatmapDifficultyMask_Expert = 8,
	BeatmapDifficultyMask_ExpertPlus = 16,
	BeatmapDifficultyMask_All = 31,
})*/
/*ENUM(uint16_t, GameplayModifiersMask, {
	GameplayModifiersMask_None = 0,
	GameplayModifiersMask_BatteryEnergy = 1,
	GameplayModifiersMask_NoFail = 2,
	GameplayModifiersMask_InstaFail = 4,
	GameplayModifiersMask_NoObstacles = 8,
	GameplayModifiersMask_NoBombs = 16,
	GameplayModifiersMask_FastNotes = 32,
	GameplayModifiersMask_StrictAngles = 64,
	GameplayModifiersMask_DisappearingArrows = 128,
	GameplayModifiersMask_FasterSong = 256,
	GameplayModifiersMask_SlowerSong = 512,
	GameplayModifiersMask_NoArrows = 1024,
	GameplayModifiersMask_GhostNotes = 2048,
	GameplayModifiersMask_SuperFastSong = 4096,
	GameplayModifiersMask_ProMode = 8192,
	GameplayModifiersMask_ZenMode = 16384,
	GameplayModifiersMask_SmallCubes = 32768,
	GameplayModifiersMask_All = 65535,
})*/
struct NetPacketHeader {
	PacketProperty property;
	uint8_t connectionNumber;
	bool isFragmented;
	uint16_t sequence;
	uint8_t channelId;
	uint16_t fragmentId;
	uint16_t fragmentPart;
	uint16_t fragmentsTotal;
};
struct MessageHeader {
	MessageType type;
	uint32_t protocolVersion;
};
struct SerializeHeader {
	uint32_t length;
	uint8_t type;
};
struct PacketEncryptionLayer {
	_Bool encrypted;
	uint32_t sequenceId;
};
struct ByteArrayNetSerializable {
	uint32_t length;
	uint8_t data[4096];
};
/*struct String {
	uint32_t length;
	uint8_t data[32768];
};*/
struct BaseMasterServerReliableRequest {
	uint32_t requestId;
};
struct BaseMasterServerReliableResponse {
	uint32_t requestId;
	uint32_t responseId;
};
struct ClientHelloRequest {
	struct BaseMasterServerReliableRequest base;
	uint8_t random[32];
};
struct HelloVerifyRequest {
	struct BaseMasterServerReliableResponse base;
	uint8_t cookie[32];
};
struct ClientHelloWithCookieRequest {
	struct BaseMasterServerReliableRequest base;
	uint32_t certificateResponseId;
	uint8_t random[32];
	uint8_t cookie[32];
};
struct ServerCertificateRequest {
	struct BaseMasterServerReliableResponse base;
	uint32_t certificateCount;
	struct ByteArrayNetSerializable certificateList[10];
};
struct ServerHelloRequest {
	struct BaseMasterServerReliableResponse base;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
	struct ByteArrayNetSerializable signature;
};
// struct ClientKeyExchangeRequest;
// struct ChangeCipherSpecRequest;
struct BaseMasterServerResponse {
	uint32_t responseId;
};
struct BaseMasterServerAcknowledgeMessage {
	struct BaseMasterServerResponse base;
	bool messageHandled;
};
struct BaseMasterServerMultipartMessage {
	struct BaseMasterServerReliableRequest base;
	uint32_t multipartMessageId;
	uint32_t offset;
	uint32_t length;
	uint32_t totalLength;
	uint8_t data[384];
};
/*struct SongPackMask {
	uint64_t top;
	uint64_t bottom;
};*/
/*struct BeatmapLevelSelectionMask {
	BeatmapDifficultyMask beatmapDifficultyMask;
	GameplayModifiersMask gameplayModifiersMask;
	struct SongPackMask songPackMask;
};*/
/*struct GameplayServerConfiguration {
	int32_t maxPlayerCount;
	DiscoveryPolicy discoveryPolicy;
	InvitePolicy invitePolicy;
	GameplayServerMode gameplayServerMode;
	SongSelectionMode songSelectionMode;
	GameplayServerControlSettings gameplayServerControlSettings;
};*/
/*struct BaseConnectToServerRequest {
	struct BaseMasterServerReliableRequest base;
	struct String userId;
	struct String userName;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
};*/
/*struct ConnectToServerRequest {
	struct BaseConnectToServerRequest base;
	struct BeatmapLevelSelectionMask selectionMask;
	struct String secret;
	struct String code;
	struct GameplayServerConfiguration configuration;
};*/

struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt);
struct NetPacketHeader pkt_readNetPacketHeader(uint8_t **pkt);
struct MessageHeader pkt_readMessageHeader(uint8_t **pkt);
struct SerializeHeader pkt_readSerializeHeader(uint8_t **pkt);
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(uint8_t **pkt);
struct ClientHelloRequest pkt_readClientHelloRequest(uint8_t **pkt);
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(uint8_t **pkt);

void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in);
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in);
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in);
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in);
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in);
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in);
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in);
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in);
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in);
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in);
#define SERIALIZE_HEAD(pkt, msg, mtype) { \
	struct MessageHeader _message = (msg); \
	_message.type = MessageType_##mtype; \
	pkt_writeMessageHeader(pkt, _message); \
}
#define SERIALIZE_BODY(pkt, stype, dtype, data) { \
	fprintf(stderr, "serialize " #stype "\n"); \
	uint8_t *_end = *(pkt); \
	pkt_write##dtype(&_end, data); \
	struct SerializeHeader _serial; \
	_serial.length = _end + 1 - *(pkt); \
	_serial.type = stype; \
	pkt_writeSerializeHeader(pkt, _serial); \
	pkt_write##dtype(pkt, data); \
}
#define SERIALIZE(pkt, msg, mtype, stype, dtype, data) { \
	fprintf(stderr, "serialize " #mtype "Type_" #stype "\n"); \
	uint8_t *_end = *(pkt); \
	pkt_write##dtype(&_end, data); \
	struct MessageHeader _message = (msg); \
	_message.type = MessageType_##mtype; \
	struct SerializeHeader _serial; \
	_serial.length = _end + 1 - *(pkt); \
	_serial.type = mtype##Type_##stype; \
	pkt_writeMessageHeader(pkt, _message); \
	pkt_writeSerializeHeader(pkt, _serial); \
	pkt_write##dtype(pkt, data); \
}
