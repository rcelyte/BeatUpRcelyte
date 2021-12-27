/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#pragma once

#include "enum.h"
#include <stdint.h>
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
ENUM(uint8_t, Platform, {
	Platform_Test,
	Platform_OculusRift,
	Platform_OculusQuest,
	Platform_Steam,
	Platform_PS4,
	Platform_PS4Dev,
	Platform_PS4Cert,
	Platform_Oculus = 1,
})
ENUM(uint8_t, BeatmapDifficultyMask, {
	BeatmapDifficultyMask_Easy = 1,
	BeatmapDifficultyMask_Normal = 2,
	BeatmapDifficultyMask_Hard = 4,
	BeatmapDifficultyMask_Expert = 8,
	BeatmapDifficultyMask_ExpertPlus = 16,
	BeatmapDifficultyMask_All = 31,
})
ENUM(uint32_t, GameplayModifierMask, {
	GameplayModifierMask_None = 0,
	GameplayModifierMask_BatteryEnergy = 1,
	GameplayModifierMask_NoFail = 2,
	GameplayModifierMask_InstaFail = 4,
	GameplayModifierMask_NoObstacles = 8,
	GameplayModifierMask_NoBombs = 16,
	GameplayModifierMask_FastNotes = 32,
	GameplayModifierMask_StrictAngles = 64,
	GameplayModifierMask_DisappearingArrows = 128,
	GameplayModifierMask_FasterSong = 256,
	GameplayModifierMask_SlowerSong = 512,
	GameplayModifierMask_NoArrows = 1024,
	GameplayModifierMask_GhostNotes = 2048,
	GameplayModifierMask_SuperFastSong = 4096,
	GameplayModifierMask_ProMode = 8192,
	GameplayModifierMask_ZenMode = 16384,
	GameplayModifierMask_SmallCubes = 32768,
	GameplayModifierMask_All = 65535,
})
ENUM(uint8_t, DiscoveryPolicy, {
	DiscoveryPolicy_Hidden,
	DiscoveryPolicy_WithCode,
	DiscoveryPolicy_Public,
})
ENUM(uint8_t, InvitePolicy, {
	InvitePolicy_OnlyConnectionOwnerCanInvite,
	InvitePolicy_AnyoneCanInvite,
	InvitePolicy_NobodyCanInvite,
})
ENUM(uint8_t, GameplayServerMode, {
	GameplayServerMode_Countdown,
	GameplayServerMode_Managed,
	GameplayServerMode_QuickStartOneSong,
})
ENUM(uint8_t, SongSelectionMode, {
	SongSelectionMode_Vote,
	SongSelectionMode_Random,
	SongSelectionMode_OwnerPicks,
	SongSelectionMode_RandomPlayerPicks,
})
ENUM(uint8_t, GameplayServerControlSettings, {
	GameplayServerControlSettings_None = 0,
	GameplayServerControlSettings_AllowModifierSelection = 1,
	GameplayServerControlSettings_AllowSpectate = 2,
	GameplayServerControlSettings_All = 3,
})
ENUM(uint8_t, AuthenticateUserResponse_Result, {
	AuthenticateUserResponse_Result_Success,
	AuthenticateUserResponse_Result_Failed,
	AuthenticateUserResponse_Result_UnknownError,
})
ENUM(uint8_t, ConnectToServerResponse_Result, {
	ConnectToServerResponse_Result_Success,
	ConnectToServerResponse_Result_InvalidSecret,
	ConnectToServerResponse_Result_InvalidCode,
	ConnectToServerResponse_Result_InvalidPassword,
	ConnectToServerResponse_Result_ServerAtCapacity,
	ConnectToServerResponse_Result_NoAvailableDedicatedServers,
	ConnectToServerResponse_Result_VersionMismatch,
	ConnectToServerResponse_Result_ConfigMismatch,
	ConnectToServerResponse_Result_UnknownError,
})
ENUM(uint8_t, GetPublicServersResponse_Result, {
	GetPublicServersResponse_Result_Success,
	GetPublicServersResponse_Result_UnknownError,
})
ENUM(uint8_t, UserMessageType, {
	UserMessageType_AuthenticateUserRequest,
	UserMessageType_AuthenticateUserResponse,
	UserMessageType_ConnectToServerResponse = 8,
	UserMessageType_ConnectToServerRequest,
	UserMessageType_UserMessageReceivedAcknowledge = 13,
	UserMessageType_UserMultipartMessage,
	UserMessageType_SessionKeepaliveMessage,
	UserMessageType_GetPublicServersRequest,
	UserMessageType_GetPublicServersResponse,
})
ENUM(uint8_t, DedicatedServerMessageType, {
	DedicatedServerMessageType_AuthenticateDedicatedServerRequest,
	DedicatedServerMessageType_AuthenticateDedicatedServerResponse,
	DedicatedServerMessageType_CreateDedicatedServerInstanceRequest = 4,
	DedicatedServerMessageType_CreateDedicatedServerInstanceResponse,
	DedicatedServerMessageType_DedicatedServerInstanceNoLongerAvailableRequest,
	DedicatedServerMessageType_DedicatedServerHeartbeatRequest,
	DedicatedServerMessageType_DedicatedServerHeartbeatResponse,
	DedicatedServerMessageType_DedicatedServerInstanceStatusUpdateRequest = 10,
	DedicatedServerMessageType_DedicatedServerShutDownRequest,
	DedicatedServerMessageType_DedicatedServerPrepareForConnectionRequest,
	DedicatedServerMessageType_DedicatedServerMessageReceivedAcknowledge,
	DedicatedServerMessageType_DedicatedServerMultipartMessage,
	DedicatedServerMessageType_DedicatedServerPrepareForConnectionResponse,
})
ENUM(uint8_t, HandshakeMessageType, {
	HandshakeMessageType_ClientHelloRequest,
	HandshakeMessageType_HelloVerifyRequest,
	HandshakeMessageType_ClientHelloWithCookieRequest,
	HandshakeMessageType_ServerHelloRequest,
	HandshakeMessageType_ServerCertificateRequest,
	HandshakeMessageType_ServerCertificateResponse,
	HandshakeMessageType_ClientKeyExchangeRequest,
	HandshakeMessageType_ChangeCipherSpecRequest,
	HandshakeMessageType_HandshakeMessageReceivedAcknowledge,
	HandshakeMessageType_HandshakeMultipartMessage,
})
ENUM(uint32_t, MessageType, {
	MessageType_UserMessage = 1,
	MessageType_DedicatedServerMessage = 2,
	MessageType_HandshakeMessage = 3192347326,
})
#define SERIALIZE_HEAD(pkt, mtype) { \
	struct MessageHeader _message; \
	_message.type = MessageType_##mtype; \
	_message.protocolVersion = 6; \
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
#define SERIALIZE(pkt, mtype, stype, dtype, data) { \
	SERIALIZE_HEAD(pkt, mtype) \
	SERIALIZE_BODY(pkt, mtype##Type_##stype, dtype, data) \
}
struct NetPacketHeader {
	PacketProperty property;
	uint8_t connectionNumber;
	_Bool isFragmented;
	uint16_t sequence;
	uint8_t channelId;
	uint16_t fragmentId;
	uint16_t fragmentPart;
	uint16_t fragmentsTotal;
};
typedef uint32_t ServerCode;
uint8_t pkt_readUint8(const uint8_t **pkt);
uint16_t pkt_readUint16(const uint8_t **pkt);
uint32_t pkt_readUint32(const uint8_t **pkt);
uint64_t pkt_readUint64(const uint8_t **pkt);
uint64_t pkt_readVarUint64(const uint8_t **pkt);
uint64_t pkt_readVarUint64(const uint8_t **pkt);
int64_t pkt_readVarInt64(const uint8_t **pkt);
uint32_t pkt_readVarUint32(const uint8_t **pkt);
int32_t pkt_readVarInt32(const uint8_t **pkt);
void pkt_readUint8Array(const uint8_t **pkt, uint8_t *out, uint32_t count);
void pkt_writeUint8(uint8_t **pkt, uint8_t v);
void pkt_writeUint16(uint8_t **pkt, uint16_t v);
void pkt_writeUint32(uint8_t **pkt, uint32_t v);
void pkt_writeUint64(uint8_t **pkt, uint64_t v);
void pkt_writeVarUint64(uint8_t **pkt, uint64_t v);
void pkt_writeVarInt64(uint8_t **pkt, int64_t v);
void pkt_writeVarUint32(uint8_t **pkt, uint32_t v);
void pkt_writeVarInt32(uint8_t **pkt, int32_t v);
#define pkt_writeInt8Array(pkt, out, count) pkt_writeUint8Array(pkt, (uint8_t*)out, count)
void pkt_writeUint8Array(uint8_t **pkt, uint8_t *in, uint32_t count);
struct NetPacketHeader pkt_readNetPacketHeader(const uint8_t **pkt);
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in);
struct PacketEncryptionLayer {
	_Bool encrypted;
	uint32_t sequenceId;
	uint8_t iv[16];
};
struct MessageHeader {
	MessageType type;
	uint32_t protocolVersion;
};
struct SerializeHeader {
	uint32_t length;
	uint8_t type;
};
struct BaseMasterServerReliableRequest {
	uint32_t requestId;
};
struct BaseMasterServerResponse {
	uint32_t responseId;
};
struct BaseMasterServerReliableResponse {
	uint32_t requestId;
	uint32_t responseId;
};
struct BaseMasterServerAcknowledgeMessage {
	struct BaseMasterServerResponse base;
	_Bool messageHandled;
};
struct ByteArrayNetSerializable {
	uint32_t length;
	uint8_t data[4096];
};
struct String {
	uint32_t length;
	char data[4096];
};
struct AuthenticationToken {
	Platform platform;
	struct String userId;
	struct String userName;
	struct ByteArrayNetSerializable sessionToken;
};
struct BaseMasterServerMultipartMessage {
	struct BaseMasterServerReliableRequest base;
	uint32_t multipartMessageId;
	uint32_t offset;
	uint32_t length;
	uint32_t totalLength;
	uint8_t data[384];
};
struct BitMask128 {
	uint64_t _d0;
	uint64_t _d1;
};
struct SongPackMask {
	struct BitMask128 _bloomFilter;
};
struct BeatmapLevelSelectionMask {
	BeatmapDifficultyMask difficulties;
	GameplayModifierMask modifiers;
	struct SongPackMask songPacks;
};
struct GameplayServerConfiguration {
	int32_t maxPlayerCount;
	DiscoveryPolicy discoveryPolicy;
	InvitePolicy invitePolicy;
	GameplayServerMode gameplayServerMode;
	SongSelectionMode songSelectionMode;
	GameplayServerControlSettings gameplayServerControlSettings;
};
struct PublicServerInfo {
	ServerCode code;
	int32_t currentPlayerCount;
};
struct IPEndPoint {
	struct String address;
	uint32_t port;
};
struct BaseConnectToServerRequest {
	struct BaseMasterServerReliableRequest base;
	struct String userId;
	struct String userName;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
};
struct AuthenticateUserRequest {
	struct BaseMasterServerReliableResponse base;
	struct AuthenticationToken authenticationToken;
};
struct AuthenticateUserResponse {
	struct BaseMasterServerReliableResponse base;
	AuthenticateUserResponse_Result result;
};
struct ConnectToServerResponse {
	struct BaseMasterServerReliableResponse base;
	ConnectToServerResponse_Result result;
	struct String userId;
	struct String userName;
	struct String secret;
	struct BeatmapLevelSelectionMask selectionMask;
	uint8_t flags;
	struct IPEndPoint remoteEndPoint;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
	ServerCode code;
	struct GameplayServerConfiguration configuration;
	struct String managerId;
};
struct ConnectToServerRequest {
	struct BaseConnectToServerRequest base;
	struct BeatmapLevelSelectionMask selectionMask;
	struct String secret;
	ServerCode code;
	struct GameplayServerConfiguration configuration;
};
struct UserMessageReceivedAcknowledge {
	struct BaseMasterServerAcknowledgeMessage base;
};
struct UserMultipartMessage {
	struct BaseMasterServerMultipartMessage base;
};
struct SessionKeepaliveMessage {
};
struct GetPublicServersRequest {
	struct BaseMasterServerReliableRequest base;
	struct String userId;
	struct String userName;
	int32_t offset;
	int32_t count;
	struct BeatmapLevelSelectionMask selectionMask;
	struct GameplayServerConfiguration configuration;
};
struct GetPublicServersResponse {
	struct BaseMasterServerReliableResponse base;
	GetPublicServersResponse_Result result;
	uint32_t publicServerCount;
	struct PublicServerInfo publicServers[16384];
};
struct AuthenticateDedicatedServerRequest {
	struct BaseMasterServerReliableResponse base;
	struct String dedicatedServerId;
	uint8_t nonce[16];
	uint8_t hash[32];
	uint64_t timestamp;
};
struct AuthenticateDedicatedServerResponse {
};
struct CreateDedicatedServerInstanceRequest {
	struct BaseMasterServerReliableRequest base;
	struct String secret;
	struct BeatmapLevelSelectionMask selectionMask;
	struct String userId;
	struct String userName;
	struct IPEndPoint userEndPoint;
	uint8_t userRandom[32];
	struct ByteArrayNetSerializable userPublicKey;
	struct GameplayServerConfiguration configuration;
};
struct CreateDedicatedServerInstanceResponse {
};
struct DedicatedServerInstanceNoLongerAvailableRequest {
};
struct DedicatedServerHeartbeatRequest {
};
struct DedicatedServerHeartbeatResponse {
};
struct DedicatedServerInstanceStatusUpdateRequest {
};
struct DedicatedServerShutDownRequest {
};
struct DedicatedServerPrepareForConnectionRequest {
};
struct DedicatedServerMessageReceivedAcknowledge {
	struct BaseMasterServerAcknowledgeMessage base;
};
struct DedicatedServerMultipartMessage {
	struct BaseMasterServerMultipartMessage base;
};
struct DedicatedServerPrepareForConnectionResponse {
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
struct ServerHelloRequest {
	struct BaseMasterServerReliableResponse base;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
	struct ByteArrayNetSerializable signature;
};
struct ServerCertificateRequest {
	struct BaseMasterServerReliableResponse base;
	uint32_t certificateCount;
	struct ByteArrayNetSerializable certificateList[10];
};
struct ServerCertificateResponse {
};
struct ClientKeyExchangeRequest {
	struct BaseMasterServerReliableResponse base;
	struct ByteArrayNetSerializable clientPublicKey;
};
struct ChangeCipherSpecRequest {
	struct BaseMasterServerReliableResponse base;
};
struct HandshakeMessageReceivedAcknowledge {
	struct BaseMasterServerAcknowledgeMessage base;
};
struct HandshakeMultipartMessage {
	struct BaseMasterServerMultipartMessage base;
};
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(const uint8_t **pkt);
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in);
struct MessageHeader pkt_readMessageHeader(const uint8_t **pkt);
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in);
struct SerializeHeader pkt_readSerializeHeader(const uint8_t **pkt);
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in);
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(const uint8_t **pkt);
void pkt_writeBaseMasterServerReliableRequest(uint8_t **pkt, struct BaseMasterServerReliableRequest in);
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(const uint8_t **pkt);
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in);
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(const uint8_t **pkt);
void pkt_writeBaseMasterServerReliableResponse(uint8_t **pkt, struct BaseMasterServerReliableResponse in);
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(const uint8_t **pkt);
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in);
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(const uint8_t **pkt);
void pkt_writeByteArrayNetSerializable(uint8_t **pkt, struct ByteArrayNetSerializable in);
struct String pkt_readString(const uint8_t **pkt);
void pkt_writeString(uint8_t **pkt, struct String in);
struct AuthenticationToken pkt_readAuthenticationToken(const uint8_t **pkt);
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in);
struct BitMask128 pkt_readBitMask128(const uint8_t **pkt);
void pkt_writeBitMask128(uint8_t **pkt, struct BitMask128 in);
struct SongPackMask pkt_readSongPackMask(const uint8_t **pkt);
void pkt_writeSongPackMask(uint8_t **pkt, struct SongPackMask in);
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(const uint8_t **pkt);
void pkt_writeBeatmapLevelSelectionMask(uint8_t **pkt, struct BeatmapLevelSelectionMask in);
struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(const uint8_t **pkt);
void pkt_writeGameplayServerConfiguration(uint8_t **pkt, struct GameplayServerConfiguration in);
void pkt_writePublicServerInfo(uint8_t **pkt, struct PublicServerInfo in);
struct IPEndPoint pkt_readIPEndPoint(const uint8_t **pkt);
void pkt_writeIPEndPoint(uint8_t **pkt, struct IPEndPoint in);
struct BaseConnectToServerRequest pkt_readBaseConnectToServerRequest(const uint8_t **pkt);
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(const uint8_t **pkt);
void pkt_writeAuthenticateUserResponse(uint8_t **pkt, struct AuthenticateUserResponse in);
void pkt_writeConnectToServerResponse(uint8_t **pkt, struct ConnectToServerResponse in);
struct ConnectToServerRequest pkt_readConnectToServerRequest(const uint8_t **pkt);
struct UserMessageReceivedAcknowledge pkt_readUserMessageReceivedAcknowledge(const uint8_t **pkt);
void pkt_writeUserMessageReceivedAcknowledge(uint8_t **pkt, struct UserMessageReceivedAcknowledge in);
void pkt_writeUserMultipartMessage(uint8_t **pkt, struct UserMultipartMessage in);
struct SessionKeepaliveMessage pkt_readSessionKeepaliveMessage(const uint8_t **pkt);
void pkt_writeSessionKeepaliveMessage(uint8_t **pkt, struct SessionKeepaliveMessage in);
struct GetPublicServersRequest pkt_readGetPublicServersRequest(const uint8_t **pkt);
void pkt_writeGetPublicServersResponse(uint8_t **pkt, struct GetPublicServersResponse in);
struct AuthenticateDedicatedServerRequest pkt_readAuthenticateDedicatedServerRequest(const uint8_t **pkt);
struct AuthenticateDedicatedServerResponse pkt_readAuthenticateDedicatedServerResponse(const uint8_t **pkt);
void pkt_writeAuthenticateDedicatedServerResponse(uint8_t **pkt, struct AuthenticateDedicatedServerResponse in);
struct CreateDedicatedServerInstanceRequest pkt_readCreateDedicatedServerInstanceRequest(const uint8_t **pkt);
struct CreateDedicatedServerInstanceResponse pkt_readCreateDedicatedServerInstanceResponse(const uint8_t **pkt);
void pkt_writeCreateDedicatedServerInstanceResponse(uint8_t **pkt, struct CreateDedicatedServerInstanceResponse in);
struct DedicatedServerInstanceNoLongerAvailableRequest pkt_readDedicatedServerInstanceNoLongerAvailableRequest(const uint8_t **pkt);
void pkt_writeDedicatedServerInstanceNoLongerAvailableRequest(uint8_t **pkt, struct DedicatedServerInstanceNoLongerAvailableRequest in);
struct DedicatedServerHeartbeatRequest pkt_readDedicatedServerHeartbeatRequest(const uint8_t **pkt);
void pkt_writeDedicatedServerHeartbeatRequest(uint8_t **pkt, struct DedicatedServerHeartbeatRequest in);
struct DedicatedServerHeartbeatResponse pkt_readDedicatedServerHeartbeatResponse(const uint8_t **pkt);
void pkt_writeDedicatedServerHeartbeatResponse(uint8_t **pkt, struct DedicatedServerHeartbeatResponse in);
struct DedicatedServerInstanceStatusUpdateRequest pkt_readDedicatedServerInstanceStatusUpdateRequest(const uint8_t **pkt);
void pkt_writeDedicatedServerInstanceStatusUpdateRequest(uint8_t **pkt, struct DedicatedServerInstanceStatusUpdateRequest in);
struct DedicatedServerShutDownRequest pkt_readDedicatedServerShutDownRequest(const uint8_t **pkt);
void pkt_writeDedicatedServerShutDownRequest(uint8_t **pkt, struct DedicatedServerShutDownRequest in);
struct DedicatedServerPrepareForConnectionRequest pkt_readDedicatedServerPrepareForConnectionRequest(const uint8_t **pkt);
void pkt_writeDedicatedServerPrepareForConnectionRequest(uint8_t **pkt, struct DedicatedServerPrepareForConnectionRequest in);
struct DedicatedServerMessageReceivedAcknowledge pkt_readDedicatedServerMessageReceivedAcknowledge(const uint8_t **pkt);
void pkt_writeDedicatedServerMessageReceivedAcknowledge(uint8_t **pkt, struct DedicatedServerMessageReceivedAcknowledge in);
void pkt_writeDedicatedServerMultipartMessage(uint8_t **pkt, struct DedicatedServerMultipartMessage in);
struct DedicatedServerPrepareForConnectionResponse pkt_readDedicatedServerPrepareForConnectionResponse(const uint8_t **pkt);
void pkt_writeDedicatedServerPrepareForConnectionResponse(uint8_t **pkt, struct DedicatedServerPrepareForConnectionResponse in);
struct ClientHelloRequest pkt_readClientHelloRequest(const uint8_t **pkt);
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in);
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(const uint8_t **pkt);
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in);
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in);
struct ServerCertificateResponse pkt_readServerCertificateResponse(const uint8_t **pkt);
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(const uint8_t **pkt);
void pkt_writeChangeCipherSpecRequest(uint8_t **pkt, struct ChangeCipherSpecRequest in);
struct HandshakeMessageReceivedAcknowledge pkt_readHandshakeMessageReceivedAcknowledge(const uint8_t **pkt);
void pkt_writeHandshakeMessageReceivedAcknowledge(uint8_t **pkt, struct HandshakeMessageReceivedAcknowledge in);
void pkt_writeHandshakeMultipartMessage(uint8_t **pkt, struct HandshakeMultipartMessage in);
