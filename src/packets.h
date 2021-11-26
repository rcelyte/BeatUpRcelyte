/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#include "enum.h"
#include <stdint.h>
ENUM(uint8_t, PacketType, {
	PacketType_Unreliable,
	PacketType_Channeled,
	PacketType_Ack,
	PacketType_Ping,
	PacketType_Pong,
	PacketType_ConnectRequest,
	PacketType_ConnectAccept,
	PacketType_Disconnect,
	PacketType_UnconnectedMessage,
	PacketType_MtuCheck,
	PacketType_MtuOk,
	PacketType_Broadcast,
	PacketType_Merged,
	PacketType_ShutdownOk,
	PacketType_PeerNotFound,
	PacketType_InvalidProtocol,
	PacketType_NatMessage,
	PacketType_Empty,
})
ENUM(uint8_t, PlatformType, {
	PlatformType_Test,
	PlatformType_OculusRift,
	PlatformType_OculusQuest,
	PlatformType_Steam,
	PlatformType_PS4,
	PlatformType_PS4Dev,
	PlatformType_PS4Cert,
	PlatformType_Oculus = 1,
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
	MessageType_UserMessage = 0,
	MessageType_DedicatedServerMessage = 1,
	MessageType_HandshakeMessage = 3192347326,
})
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
struct PacketEncryptionLayer {
	_Bool encrypted;
	uint32_t sequenceId;
};
struct NetPacketHeader {
	PacketType property;
	uint8_t connectionNumber;
	_Bool isFragmented;
	uint16_t sequence;
	uint8_t channelId;
	uint16_t fragmentId;
	uint16_t fragmentPart;
	uint16_t fragmentsTotal;
};
struct ServerCode {
	uint32_t value; // 25 bit value
};
void pkt_writeUint8Array(uint8_t **pkt, uint8_t *in, uint32_t count);
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt);
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in);
struct NetPacketHeader pkt_readNetPacketHeader(uint8_t **pkt);
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in);
struct MessageHeader {
	uint32_t type;
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
	uint8_t platform;
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
	uint8_t difficulties;
	uint32_t modifiers;
	struct SongPackMask songPacks;
};
struct GameplayServerConfiguration {
	int32_t maxPlayerCount;
	int32_t discoveryPolicy;
	int32_t invitePolicy;
	int32_t gameplayServerMode;
	int32_t songSelectionMode;
	int32_t gameplayServerControlSettings;
};
struct PublicServerInfo {
	struct ServerCode code;
	int32_t currentPlayerCount;
};
struct AuthenticateUserRequest {
	struct BaseMasterServerReliableResponse base;
	struct AuthenticationToken authenticationToken;
};
struct AuthenticateUserResponse {
	struct BaseMasterServerReliableResponse base;
	uint8_t result;
};
struct ConnectToServerResponse {
};
struct ConnectToServerRequest {
};
struct UserMessageReceivedAcknowledge {
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
	uint8_t result;
	uint32_t publicServerCount;
	struct PublicServerInfo publicServers[16384];
};
struct AuthenticateDedicatedServerRequest {
};
struct AuthenticateDedicatedServerResponse {
};
struct CreateDedicatedServerInstanceRequest {
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
};
struct ChangeCipherSpecRequest {
};
struct HandshakeMessageReceivedAcknowledge {
};
struct HandshakeMultipartMessage {
	struct BaseMasterServerMultipartMessage base;
};
struct MessageHeader pkt_readMessageHeader(uint8_t **pkt);
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in);
struct SerializeHeader pkt_readSerializeHeader(uint8_t **pkt);
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in);
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(uint8_t **pkt);
void pkt_writeBaseMasterServerReliableRequest(uint8_t **pkt, struct BaseMasterServerReliableRequest in);
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(uint8_t **pkt);
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in);
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(uint8_t **pkt);
void pkt_writeBaseMasterServerReliableResponse(uint8_t **pkt, struct BaseMasterServerReliableResponse in);
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(uint8_t **pkt);
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in);
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(uint8_t **pkt);
void pkt_writeByteArrayNetSerializable(uint8_t **pkt, struct ByteArrayNetSerializable in);
struct String pkt_readString(uint8_t **pkt);
void pkt_writeString(uint8_t **pkt, struct String in);
struct AuthenticationToken pkt_readAuthenticationToken(uint8_t **pkt);
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in);
struct BitMask128 pkt_readBitMask128(uint8_t **pkt);
struct SongPackMask pkt_readSongPackMask(uint8_t **pkt);
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(uint8_t **pkt);
struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(uint8_t **pkt);
void pkt_writePublicServerInfo(uint8_t **pkt, struct PublicServerInfo in);
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(uint8_t **pkt);
void pkt_writeAuthenticateUserResponse(uint8_t **pkt, struct AuthenticateUserResponse in);
struct ConnectToServerResponse pkt_readConnectToServerResponse(uint8_t **pkt);
void pkt_writeConnectToServerResponse(uint8_t **pkt, struct ConnectToServerResponse in);
struct ConnectToServerRequest pkt_readConnectToServerRequest(uint8_t **pkt);
void pkt_writeConnectToServerRequest(uint8_t **pkt, struct ConnectToServerRequest in);
struct UserMessageReceivedAcknowledge pkt_readUserMessageReceivedAcknowledge(uint8_t **pkt);
void pkt_writeUserMessageReceivedAcknowledge(uint8_t **pkt, struct UserMessageReceivedAcknowledge in);
void pkt_writeUserMultipartMessage(uint8_t **pkt, struct UserMultipartMessage in);
struct SessionKeepaliveMessage pkt_readSessionKeepaliveMessage(uint8_t **pkt);
void pkt_writeSessionKeepaliveMessage(uint8_t **pkt, struct SessionKeepaliveMessage in);
struct GetPublicServersRequest pkt_readGetPublicServersRequest(uint8_t **pkt);
void pkt_writeGetPublicServersResponse(uint8_t **pkt, struct GetPublicServersResponse in);
struct AuthenticateDedicatedServerRequest pkt_readAuthenticateDedicatedServerRequest(uint8_t **pkt);
void pkt_writeAuthenticateDedicatedServerRequest(uint8_t **pkt, struct AuthenticateDedicatedServerRequest in);
struct AuthenticateDedicatedServerResponse pkt_readAuthenticateDedicatedServerResponse(uint8_t **pkt);
void pkt_writeAuthenticateDedicatedServerResponse(uint8_t **pkt, struct AuthenticateDedicatedServerResponse in);
struct CreateDedicatedServerInstanceRequest pkt_readCreateDedicatedServerInstanceRequest(uint8_t **pkt);
void pkt_writeCreateDedicatedServerInstanceRequest(uint8_t **pkt, struct CreateDedicatedServerInstanceRequest in);
struct CreateDedicatedServerInstanceResponse pkt_readCreateDedicatedServerInstanceResponse(uint8_t **pkt);
void pkt_writeCreateDedicatedServerInstanceResponse(uint8_t **pkt, struct CreateDedicatedServerInstanceResponse in);
struct DedicatedServerInstanceNoLongerAvailableRequest pkt_readDedicatedServerInstanceNoLongerAvailableRequest(uint8_t **pkt);
void pkt_writeDedicatedServerInstanceNoLongerAvailableRequest(uint8_t **pkt, struct DedicatedServerInstanceNoLongerAvailableRequest in);
struct DedicatedServerHeartbeatRequest pkt_readDedicatedServerHeartbeatRequest(uint8_t **pkt);
void pkt_writeDedicatedServerHeartbeatRequest(uint8_t **pkt, struct DedicatedServerHeartbeatRequest in);
struct DedicatedServerHeartbeatResponse pkt_readDedicatedServerHeartbeatResponse(uint8_t **pkt);
void pkt_writeDedicatedServerHeartbeatResponse(uint8_t **pkt, struct DedicatedServerHeartbeatResponse in);
struct DedicatedServerInstanceStatusUpdateRequest pkt_readDedicatedServerInstanceStatusUpdateRequest(uint8_t **pkt);
void pkt_writeDedicatedServerInstanceStatusUpdateRequest(uint8_t **pkt, struct DedicatedServerInstanceStatusUpdateRequest in);
struct DedicatedServerShutDownRequest pkt_readDedicatedServerShutDownRequest(uint8_t **pkt);
void pkt_writeDedicatedServerShutDownRequest(uint8_t **pkt, struct DedicatedServerShutDownRequest in);
struct DedicatedServerPrepareForConnectionRequest pkt_readDedicatedServerPrepareForConnectionRequest(uint8_t **pkt);
void pkt_writeDedicatedServerPrepareForConnectionRequest(uint8_t **pkt, struct DedicatedServerPrepareForConnectionRequest in);
struct DedicatedServerMessageReceivedAcknowledge pkt_readDedicatedServerMessageReceivedAcknowledge(uint8_t **pkt);
void pkt_writeDedicatedServerMessageReceivedAcknowledge(uint8_t **pkt, struct DedicatedServerMessageReceivedAcknowledge in);
void pkt_writeDedicatedServerMultipartMessage(uint8_t **pkt, struct DedicatedServerMultipartMessage in);
struct DedicatedServerPrepareForConnectionResponse pkt_readDedicatedServerPrepareForConnectionResponse(uint8_t **pkt);
void pkt_writeDedicatedServerPrepareForConnectionResponse(uint8_t **pkt, struct DedicatedServerPrepareForConnectionResponse in);
struct ClientHelloRequest pkt_readClientHelloRequest(uint8_t **pkt);
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in);
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(uint8_t **pkt);
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in);
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in);
struct ServerCertificateResponse pkt_readServerCertificateResponse(uint8_t **pkt);
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(uint8_t **pkt);
void pkt_writeClientKeyExchangeRequest(uint8_t **pkt, struct ClientKeyExchangeRequest in);
struct ChangeCipherSpecRequest pkt_readChangeCipherSpecRequest(uint8_t **pkt);
void pkt_writeChangeCipherSpecRequest(uint8_t **pkt, struct ChangeCipherSpecRequest in);
struct HandshakeMessageReceivedAcknowledge pkt_readHandshakeMessageReceivedAcknowledge(uint8_t **pkt);
void pkt_writeHandshakeMessageReceivedAcknowledge(uint8_t **pkt, struct HandshakeMessageReceivedAcknowledge in);
void pkt_writeHandshakeMultipartMessage(uint8_t **pkt, struct HandshakeMultipartMessage in);
