#ifndef PACKETS_H
#define PACKETS_H

/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#include "enum.h"
#include <stdint.h>

#define CONCAT1(a, b) a##b
#define CONCAT0(a, b) CONCAT1(a, b)
#define SERIALIZE_CUSTOM(pkt, stype) \
	for(uint8_t *start = *pkt; start;) \
		if(*pkt != start) { \
			struct SerializeHeader serial; \
			serial.length = *pkt + 1 - start; \
			serial.type = stype; \
			*pkt = start, start = NULL; \
			pkt_writeSerializeHeader(pkt, serial); \
			fprintf(stderr, "serialize " #stype "\n"); \
			goto CONCAT0(_body_, __LINE__); \
		} else CONCAT0(_body_, __LINE__):
uint8_t pkt_readUint8(const uint8_t **pkt);
uint16_t pkt_readUint16(const uint8_t **pkt);
uint32_t pkt_readUint32(const uint8_t **pkt);
uint64_t pkt_readUint64(const uint8_t **pkt);
#define pkt_readInt8(pkt) (int8_t)pkt_readUint8(pkt)
#define pkt_readInt16(pkt) (int16_t)pkt_readUint16(pkt)
#define pkt_readInt32(pkt) (int32_t)pkt_readUint32(pkt)
#define pkt_readInt64(pkt) (int64_t)pkt_readUint64(pkt)
uint64_t pkt_readVarUint64(const uint8_t **pkt);
uint64_t pkt_readVarUint64(const uint8_t **pkt);
int64_t pkt_readVarInt64(const uint8_t **pkt);
uint32_t pkt_readVarUint32(const uint8_t **pkt);
int32_t pkt_readVarInt32(const uint8_t **pkt);
void pkt_readUint8Array(const uint8_t **pkt, uint8_t *out, uint32_t count);
float pkt_readFloat32(const uint8_t **pkt);
double pkt_readFloat64(const uint8_t **pkt);
void pkt_writeUint8(uint8_t **pkt, uint8_t v);
void pkt_writeUint16(uint8_t **pkt, uint16_t v);
void pkt_writeUint32(uint8_t **pkt, uint32_t v);
void pkt_writeUint64(uint8_t **pkt, uint64_t v);
#define pkt_writeInt8(pkt, v) pkt_writeUint8(pkt, (int8_t)v)
#define pkt_writeInt16(pkt, v) pkt_writeUint16(pkt, (int16_t)v)
#define pkt_writeInt32(pkt, v) pkt_writeUint32(pkt, (int32_t)v)
#define pkt_writeInt64(pkt, v) pkt_writeUint64(pkt, (int64_t)v)
void pkt_writeVarUint64(uint8_t **pkt, uint64_t v);
void pkt_writeVarInt64(uint8_t **pkt, int64_t v);
void pkt_writeVarUint32(uint8_t **pkt, uint32_t v);
void pkt_writeVarInt32(uint8_t **pkt, int32_t v);
#define pkt_writeInt8Array(pkt, out, count) pkt_writeUint8Array(pkt, (uint8_t*)out, count)
void pkt_writeUint8Array(uint8_t **pkt, const uint8_t *in, uint32_t count);
void pkt_writeFloat32(uint8_t **pkt, float v);
void pkt_writeFloat64(uint8_t **pkt, double v);
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
struct PacketEncryptionLayer {
	_Bool encrypted;
	uint32_t sequenceId;
	uint8_t iv[16];
};
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(const uint8_t **pkt);
void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in);
struct BaseMasterServerReliableRequest {
	uint32_t requestId;
};
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(const uint8_t **pkt);
void pkt_writeBaseMasterServerReliableRequest(uint8_t **pkt, struct BaseMasterServerReliableRequest in);
struct BaseMasterServerResponse {
	uint32_t responseId;
};
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(const uint8_t **pkt);
void pkt_writeBaseMasterServerResponse(uint8_t **pkt, struct BaseMasterServerResponse in);
struct BaseMasterServerReliableResponse {
	uint32_t requestId;
	uint32_t responseId;
};
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(const uint8_t **pkt);
void pkt_writeBaseMasterServerReliableResponse(uint8_t **pkt, struct BaseMasterServerReliableResponse in);
struct BaseMasterServerAcknowledgeMessage {
	struct BaseMasterServerResponse base;
	_Bool messageHandled;
};
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(const uint8_t **pkt);
void pkt_writeBaseMasterServerAcknowledgeMessage(uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in);
struct ByteArrayNetSerializable {
	uint32_t length;
	uint8_t data[4096];
};
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(const uint8_t **pkt);
void pkt_writeByteArrayNetSerializable(uint8_t **pkt, struct ByteArrayNetSerializable in);
struct String {
	uint32_t length;
	char data[60];
};
struct String pkt_readString(const uint8_t **pkt);
void pkt_writeString(uint8_t **pkt, struct String in);
struct LongString {
	uint32_t length;
	char data[4096];
};
struct LongString pkt_readLongString(const uint8_t **pkt);
void pkt_writeLongString(uint8_t **pkt, struct LongString in);
struct AuthenticationToken {
	Platform platform;
	struct String userId;
	struct String userName;
	struct ByteArrayNetSerializable sessionToken;
};
struct AuthenticationToken pkt_readAuthenticationToken(const uint8_t **pkt);
struct BaseMasterServerMultipartMessage {
	struct BaseMasterServerReliableRequest base;
	uint32_t multipartMessageId;
	uint32_t offset;
	uint32_t length;
	uint32_t totalLength;
	uint8_t data[384];
};
void pkt_writeBaseMasterServerMultipartMessage(uint8_t **pkt, struct BaseMasterServerMultipartMessage in);
struct BitMask128 {
	uint64_t d0;
	uint64_t d1;
};
struct BitMask128 pkt_readBitMask128(const uint8_t **pkt);
void pkt_writeBitMask128(uint8_t **pkt, struct BitMask128 in);
struct SongPackMask {
	struct BitMask128 bloomFilter;
};
struct SongPackMask pkt_readSongPackMask(const uint8_t **pkt);
void pkt_writeSongPackMask(uint8_t **pkt, struct SongPackMask in);
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
struct BeatmapLevelSelectionMask {
	BeatmapDifficultyMask difficulties;
	GameplayModifierMask modifiers;
	struct SongPackMask songPacks;
};
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(const uint8_t **pkt);
void pkt_writeBeatmapLevelSelectionMask(uint8_t **pkt, struct BeatmapLevelSelectionMask in);
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
struct GameplayServerConfiguration {
	int32_t maxPlayerCount;
	DiscoveryPolicy discoveryPolicy;
	InvitePolicy invitePolicy;
	GameplayServerMode gameplayServerMode;
	SongSelectionMode songSelectionMode;
	GameplayServerControlSettings gameplayServerControlSettings;
};
struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(const uint8_t **pkt);
void pkt_writeGameplayServerConfiguration(uint8_t **pkt, struct GameplayServerConfiguration in);
typedef uint32_t ServerCode;
ServerCode StringToServerCode(const char *in, uint32_t len);
char *ServerCodeToString(char *out, ServerCode in);
struct IPEndPoint {
	struct String address;
	uint32_t port;
};
void pkt_writeIPEndPoint(uint8_t **pkt, struct IPEndPoint in);
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
struct BaseConnectToServerRequest {
	struct BaseMasterServerReliableRequest base;
	struct String userId;
	struct String userName;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
};
struct BaseConnectToServerRequest pkt_readBaseConnectToServerRequest(const uint8_t **pkt);
ENUM(uint8_t, DeliveryMethod, {
	DeliveryMethod_ReliableUnordered,
	DeliveryMethod_Sequenced,
	DeliveryMethod_ReliableOrdered,
	DeliveryMethod_ReliableSequenced,
})
struct Channeled {
	uint16_t sequence;
	DeliveryMethod channelId;
};
struct Channeled pkt_readChanneled(const uint8_t **pkt);
void pkt_writeChanneled(uint8_t **pkt, struct Channeled in);
struct Ack {
	uint16_t sequence;
	DeliveryMethod channelId;
	uint8_t data[9];
};
struct Ack pkt_readAck(const uint8_t **pkt);
void pkt_writeAck(uint8_t **pkt, struct Ack in);
struct Ping {
	uint16_t sequence;
};
struct Ping pkt_readPing(const uint8_t **pkt);
struct Pong {
	uint16_t sequence;
	uint64_t time;
};
void pkt_writePong(uint8_t **pkt, struct Pong in);
struct ConnectRequest {
	uint32_t protocolId;
	uint64_t connectId;
	uint8_t addrlen;
	uint8_t address[38];
	struct String secret;
	struct String userId;
	struct String userName;
	_Bool isConnectionOwner;
};
struct ConnectRequest pkt_readConnectRequest(const uint8_t **pkt);
struct ConnectAccept {
	uint64_t connectId;
	uint8_t connectNum;
	_Bool reusedPeer;
};
void pkt_writeConnectAccept(uint8_t **pkt, struct ConnectAccept in);
struct Disconnect {
	uint8_t _pad0[8];
};
struct Disconnect pkt_readDisconnect(const uint8_t **pkt);
struct MtuCheck {
	uint32_t newMtu0;
	uint8_t pad[1423];
	uint32_t newMtu1;
};
struct MtuCheck pkt_readMtuCheck(const uint8_t **pkt);
struct MtuOk {
	uint32_t newMtu0;
	uint8_t pad[1423];
	uint32_t newMtu1;
};
void pkt_writeMtuOk(uint8_t **pkt, struct MtuOk in);
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
struct NetPacketHeader {
	PacketProperty property;
	uint8_t connectionNumber;
	_Bool isFragmented;
};
struct NetPacketHeader pkt_readNetPacketHeader(const uint8_t **pkt);
void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in);
struct FragmentedHeader {
	uint16_t fragmentId;
	uint16_t fragmentPart;
	uint16_t fragmentsTotal;
};
struct FragmentedHeader pkt_readFragmentedHeader(const uint8_t **pkt);
struct PlayerStateHash {
	struct BitMask128 bloomFilter;
};
struct PlayerStateHash pkt_readPlayerStateHash(const uint8_t **pkt);
void pkt_writePlayerStateHash(uint8_t **pkt, struct PlayerStateHash in);
struct Color32 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};
struct Color32 pkt_readColor32(const uint8_t **pkt);
void pkt_writeColor32(uint8_t **pkt, struct Color32 in);
struct MultiplayerAvatarData {
	struct String headTopId;
	struct Color32 headTopPrimaryColor;
	struct Color32 handsColor;
	struct String clothesId;
	struct Color32 clothesPrimaryColor;
	struct Color32 clothesSecondaryColor;
	struct Color32 clothesDetailColor;
	struct Color32 _unused[2];
	struct String eyesId;
	struct String mouthId;
	struct Color32 glassesColor;
	struct Color32 facialHairColor;
	struct Color32 headTopSecondaryColor;
	struct String glassesId;
	struct String facialHairId;
	struct String handsId;
};
struct MultiplayerAvatarData pkt_readMultiplayerAvatarData(const uint8_t **pkt);
void pkt_writeMultiplayerAvatarData(uint8_t **pkt, struct MultiplayerAvatarData in);
ENUM(uint32_t, DisconnectedReason, {
	DisconnectedReason_Unknown,
	DisconnectedReason_UserInitiated,
	DisconnectedReason_Timeout,
	DisconnectedReason_Kicked,
	DisconnectedReason_ServerAtCapacity,
	DisconnectedReason_ServerConnectionClosed,
	DisconnectedReason_MasterServerUnreachable,
	DisconnectedReason_ClientConnectionClosed,
	DisconnectedReason_NetworkDisconnected,
	DisconnectedReason_ServerTerminated,
})
struct RoutingHeader {
	uint8_t remoteConnectionId;
	uint8_t connectionId;
	_Bool encrypted;
};
struct RoutingHeader pkt_readRoutingHeader(const uint8_t **pkt);
void pkt_writeRoutingHeader(uint8_t **pkt, struct RoutingHeader in);
struct SyncTime {
	float syncTime;
};
void pkt_writeSyncTime(uint8_t **pkt, struct SyncTime in);
struct PlayerConnected {
	uint8_t remoteConnectionId;
	struct String userId;
	struct String userName;
	_Bool isConnectionOwner;
};
void pkt_writePlayerConnected(uint8_t **pkt, struct PlayerConnected in);
struct PlayerIdentity {
	struct PlayerStateHash playerState;
	struct MultiplayerAvatarData playerAvatar;
	struct ByteArrayNetSerializable random;
	struct ByteArrayNetSerializable publicEncryptionKey;
};
struct PlayerIdentity pkt_readPlayerIdentity(const uint8_t **pkt);
void pkt_writePlayerIdentity(uint8_t **pkt, struct PlayerIdentity in);
struct PlayerDisconnected {
	DisconnectedReason disconnectedReason;
};
void pkt_writePlayerDisconnected(uint8_t **pkt, struct PlayerDisconnected in);
struct PlayerSortOrderUpdate {
	struct String userId;
	int32_t sortIndex;
};
void pkt_writePlayerSortOrderUpdate(uint8_t **pkt, struct PlayerSortOrderUpdate in);
struct PlayerStateUpdate {
	struct PlayerStateHash playerState;
};
struct PlayerStateUpdate pkt_readPlayerStateUpdate(const uint8_t **pkt);
struct PingMessage {
	float pingTime;
};
struct PingMessage pkt_readPingMessage(const uint8_t **pkt);
struct PongMessage {
	float pingTime;
};
struct PongMessage pkt_readPongMessage(const uint8_t **pkt);
ENUM(uint8_t, InternalMessageType, {
	InternalMessageType_SyncTime,
	InternalMessageType_PlayerConnected,
	InternalMessageType_PlayerIdentity,
	InternalMessageType_PlayerLatencyUpdate,
	InternalMessageType_PlayerDisconnected,
	InternalMessageType_PlayerSortOrderUpdate,
	InternalMessageType_Party,
	InternalMessageType_MultiplayerSession,
	InternalMessageType_KickPlayer,
	InternalMessageType_PlayerStateUpdate,
	InternalMessageType_PlayerAvatarUpdate,
	InternalMessageType_PingMessage,
	InternalMessageType_PongMessage,
})
struct RemoteProcedureCall {
	float syncTime;
};
struct RemoteProcedureCall pkt_readRemoteProcedureCall(const uint8_t **pkt);
void pkt_writeRemoteProcedureCall(uint8_t **pkt, struct RemoteProcedureCall in);
struct RemoteProcedureCallFlags pkt_readRemoteProcedureCallFlags(const uint8_t **pkt, uint32_t protocolVersion);
struct RemoteProcedureCallFlags {
	_Bool hasValue0;
	_Bool hasValue1;
	_Bool hasValue2;
	_Bool hasValue3;
};
void pkt_writeRemoteProcedureCallFlags(uint8_t **pkt, struct RemoteProcedureCallFlags in, uint32_t protocolVersion);
struct PlayersMissingEntitlementsNetSerializable {
	int32_t count;
	struct String playersWithoutEntitlements[128];
};
void pkt_writePlayersMissingEntitlementsNetSerializable(uint8_t **pkt, struct PlayersMissingEntitlementsNetSerializable in);
ENUM(uint8_t, EntitlementsStatus, {
	EntitlementsStatus_Unknown,
	EntitlementsStatus_NotOwned,
	EntitlementsStatus_NotDownloaded,
	EntitlementsStatus_Ok,
})
ENUM(uint32_t, BeatmapDifficulty, {
	BeatmapDifficulty_Easy,
	BeatmapDifficulty_Normal,
	BeatmapDifficulty_Hard,
	BeatmapDifficulty_Expert,
	BeatmapDifficulty_ExpertPlus,
})
struct BeatmapIdentifierNetSerializable {
	struct LongString levelID;
	struct String beatmapCharacteristicSerializedName;
	BeatmapDifficulty difficulty;
};
struct BeatmapIdentifierNetSerializable pkt_readBeatmapIdentifierNetSerializable(const uint8_t **pkt);
void pkt_writeBeatmapIdentifierNetSerializable(uint8_t **pkt, struct BeatmapIdentifierNetSerializable in);
ENUM(uint8_t, EnabledObstacleType, {
	EnabledObstacleType_All,
	EnabledObstacleType_FullHeightOnly,
	EnabledObstacleType_NoObstacles,
})
ENUM(uint8_t, EnergyType, {
	EnergyType_Bar,
	EnergyType_Battery,
})
ENUM(uint8_t, SongSpeed, {
	SongSpeed_Normal,
	SongSpeed_Faster,
	SongSpeed_Slower,
	SongSpeed_SuperFast,
})
struct GameplayModifiers {
	EnergyType energyType;
	_Bool demoNoFail;
	_Bool instaFail;
	_Bool failOnSaberClash;
	EnabledObstacleType enabledObstacleType;
	_Bool demoNoObstacles;
	_Bool noBombs;
	_Bool fastNotes;
	_Bool strictAngles;
	_Bool disappearingArrows;
	_Bool ghostNotes;
	SongSpeed songSpeed;
	_Bool noArrows;
	_Bool noFailOn0Energy;
	_Bool proMode;
	_Bool zenMode;
	_Bool smallCubes;
};
struct GameplayModifiers pkt_readGameplayModifiers(const uint8_t **pkt);
void pkt_writeGameplayModifiers(uint8_t **pkt, struct GameplayModifiers in);
struct PlayerLobbyPermissionConfigurationNetSerializable {
	struct String userId;
	_Bool isServerOwner;
	_Bool hasRecommendBeatmapsPermission;
	_Bool hasRecommendGameplayModifiersPermission;
	_Bool hasKickVotePermission;
	_Bool hasInvitePermission;
};
void pkt_writePlayerLobbyPermissionConfigurationNetSerializable(uint8_t **pkt, struct PlayerLobbyPermissionConfigurationNetSerializable in);
struct PlayersLobbyPermissionConfigurationNetSerializable {
	int32_t count;
	struct PlayerLobbyPermissionConfigurationNetSerializable playersPermission[128];
};
void pkt_writePlayersLobbyPermissionConfigurationNetSerializable(uint8_t **pkt, struct PlayersLobbyPermissionConfigurationNetSerializable in);
struct SyncStateId {
	uint8_t id;
	_Bool same;
};
struct SyncStateId pkt_readSyncStateId(const uint8_t **pkt);
struct Vector3Serializable {
	int32_t x;
	int32_t y;
	int32_t z;
};
struct Vector3Serializable pkt_readVector3Serializable(const uint8_t **pkt);
struct QuaternionSerializable {
	int32_t a;
	int32_t b;
	int32_t c;
};
struct QuaternionSerializable pkt_readQuaternionSerializable(const uint8_t **pkt);
struct PoseSerializable {
	struct Vector3Serializable position;
	struct QuaternionSerializable rotation;
};
struct PoseSerializable pkt_readPoseSerializable(const uint8_t **pkt);
ENUM(uint8_t, CannotStartGameReason, {
	CannotStartGameReason_None = 1,
	CannotStartGameReason_AllPlayersSpectating,
	CannotStartGameReason_NoSongSelected,
	CannotStartGameReason_AllPlayersNotInLobby,
	CannotStartGameReason_DoNotOwnSong,
})
ENUM(uint8_t, MultiplayerGameState, {
	MultiplayerGameState_None,
	MultiplayerGameState_Lobby,
	MultiplayerGameState_Game,
})
struct ColorNoAlphaSerializable {
	float r;
	float g;
	float b;
};
struct ColorNoAlphaSerializable pkt_readColorNoAlphaSerializable(const uint8_t **pkt);
void pkt_writeColorNoAlphaSerializable(uint8_t **pkt, struct ColorNoAlphaSerializable in);
struct ColorSchemeNetSerializable {
	struct ColorNoAlphaSerializable saberAColor;
	struct ColorNoAlphaSerializable saberBColor;
	struct ColorNoAlphaSerializable obstaclesColor;
	struct ColorNoAlphaSerializable environmentColor0;
	struct ColorNoAlphaSerializable environmentColor1;
	struct ColorNoAlphaSerializable environmentColor0Boost;
	struct ColorNoAlphaSerializable environmentColor1Boost;
};
struct ColorSchemeNetSerializable pkt_readColorSchemeNetSerializable(const uint8_t **pkt);
void pkt_writeColorSchemeNetSerializable(uint8_t **pkt, struct ColorSchemeNetSerializable in);
struct PlayerSpecificSettingsNetSerializable {
	struct String userId;
	struct String userName;
	_Bool leftHanded;
	_Bool automaticPlayerHeight;
	float playerHeight;
	float headPosToPlayerHeightOffset;
	struct ColorSchemeNetSerializable colorScheme;
};
struct PlayerSpecificSettingsNetSerializable pkt_readPlayerSpecificSettingsNetSerializable(const uint8_t **pkt);
void pkt_writePlayerSpecificSettingsNetSerializable(uint8_t **pkt, struct PlayerSpecificSettingsNetSerializable in);
struct PlayerSpecificSettingsAtStartNetSerializable {
	int32_t count;
	struct PlayerSpecificSettingsNetSerializable activePlayerSpecificSettingsAtGameStart[128];
};
void pkt_writePlayerSpecificSettingsAtStartNetSerializable(uint8_t **pkt, struct PlayerSpecificSettingsAtStartNetSerializable in);
ENUM(int32_t, ColorType, {
	ColorType_ColorA = 0,
	ColorType_ColorB = 1,
	ColorType_None = -1,
})
typedef uint8_t NoteLineLayer;
struct NoteCutInfoNetSerializable {
	_Bool cutWasOk;
	float saberSpeed;
	struct Vector3Serializable saberDir;
	struct Vector3Serializable cutPoint;
	struct Vector3Serializable cutNormal;
	struct Vector3Serializable notePosition;
	struct Vector3Serializable noteScale;
	struct QuaternionSerializable noteRotation;
	ColorType colorType;
	NoteLineLayer noteLineLayer;
	int32_t noteLineIndex;
	float noteTime;
	float timeToNextColorNote;
	struct Vector3Serializable moveVec;
};
struct NoteCutInfoNetSerializable pkt_readNoteCutInfoNetSerializable(const uint8_t **pkt);
struct NoteMissInfoNetSerializable {
	ColorType colorType;
	NoteLineLayer noteLineLayer;
	int32_t noteLineIndex;
	float noteTime;
};
struct NoteMissInfoNetSerializable pkt_readNoteMissInfoNetSerializable(const uint8_t **pkt);
ENUM(uint8_t, MultiplayerLevelEndState, {
	MultiplayerLevelEndState_Cleared,
	MultiplayerLevelEndState_Failed,
	MultiplayerLevelEndState_GivenUp,
	MultiplayerLevelEndState_WasInactive,
	MultiplayerLevelEndState_StartupFailed,
	MultiplayerLevelEndState_HostEndedLevel,
	MultiplayerLevelEndState_ConnectedAfterLevelEnded,
	MultiplayerLevelEndState_Quit,
})
ENUM(uint8_t, MultiplayerPlayerLevelEndState, {
	MultiplayerPlayerLevelEndState_SongFinished,
	MultiplayerPlayerLevelEndState_NotFinished,
	MultiplayerPlayerLevelEndState_NotStarted,
})
ENUM(uint8_t, MultiplayerPlayerLevelEndReason, {
	MultiplayerPlayerLevelEndReason_Cleared,
	MultiplayerPlayerLevelEndReason_Failed,
	MultiplayerPlayerLevelEndReason_GivenUp,
	MultiplayerPlayerLevelEndReason_Quit,
	MultiplayerPlayerLevelEndReason_HostEndedLevel,
	MultiplayerPlayerLevelEndReason_WasInactive,
	MultiplayerPlayerLevelEndReason_StartupFailed,
	MultiplayerPlayerLevelEndReason_ConnectedAfterLevelEnded,
})
ENUM(uint8_t, Rank, {
	Rank_E,
	Rank_D,
	Rank_C,
	Rank_B,
	Rank_A,
	Rank_S,
	Rank_SS,
	Rank_SSS,
})
ENUM(uint8_t, LevelEndStateType, {
	LevelEndStateType_None,
	LevelEndStateType_Cleared,
	LevelEndStateType_Failed,
})
ENUM(uint8_t, LevelEndAction, {
	LevelEndAction_None,
	LevelEndAction_Quit,
	LevelEndAction_Restart,
})
struct LevelCompletionResults {
	struct GameplayModifiers gameplayModifiers;
	int32_t modifiedScore;
	int32_t rawScore;
	Rank rank;
	_Bool fullCombo;
	float leftSaberMovementDistance;
	float rightSaberMovementDistance;
	float leftHandMovementDistance;
	float rightHandMovementDistance;
	float songDuration;
	LevelEndStateType levelEndStateType;
	LevelEndAction levelEndAction;
	float energy;
	int32_t goodCutsCount;
	int32_t badCutsCount;
	int32_t missedCount;
	int32_t notGoodCount;
	int32_t okCount;
	int32_t averageCutScore;
	int32_t maxCutScore;
	float averageCutDistanceRawScore;
	int32_t maxCombo;
	float minDirDeviation;
	float maxDirDeviation;
	float averageDirDeviation;
	float minTimeDeviation;
	float maxTimeDeviation;
	float averageTimeDeviation;
	float endSongTime;
};
struct LevelCompletionResults pkt_readLevelCompletionResults(const uint8_t **pkt);
struct MultiplayerLevelCompletionResults {
	MultiplayerLevelEndState levelEndState;
	MultiplayerPlayerLevelEndState playerLevelEndState;
	MultiplayerPlayerLevelEndReason playerLevelEndReason;
	struct LevelCompletionResults levelCompletionResults;
};
struct MultiplayerLevelCompletionResults pkt_readMultiplayerLevelCompletionResults(const uint8_t **pkt, uint32_t protocolVersion);
struct NodePoseSyncState1 {
	struct PoseSerializable head;
	struct PoseSerializable leftController;
	struct PoseSerializable rightController;
};
struct NodePoseSyncState1 pkt_readNodePoseSyncState1(const uint8_t **pkt);
struct StandardScoreSyncState {
	int32_t modifiedScore;
	int32_t rawScore;
	int32_t immediateMaxPossibleRawScore;
	int32_t combo;
	int32_t multiplier;
};
struct StandardScoreSyncState pkt_readStandardScoreSyncState(const uint8_t **pkt);
ENUM(uint8_t, NoteCutDirection, {
	NoteCutDirection_Up,
	NoteCutDirection_Down,
	NoteCutDirection_Left,
	NoteCutDirection_Right,
	NoteCutDirection_UpLeft,
	NoteCutDirection_UpRight,
	NoteCutDirection_DownLeft,
	NoteCutDirection_DownRight,
	NoteCutDirection_Any,
	NoteCutDirection_None,
})
struct NoteSpawnInfoNetSerializable {
	float time;
	int32_t lineIndex;
	NoteLineLayer noteLineLayer;
	NoteLineLayer beforeJumpNoteLineLayer;
	ColorType colorType;
	NoteCutDirection cutDirection;
	float timeToNextColorNote;
	float timeToPrevColorNote;
	int32_t flipLineIndex;
	int32_t flipYSide;
	struct Vector3Serializable moveStartPos;
	struct Vector3Serializable moveEndPos;
	struct Vector3Serializable jumpEndPos;
	float jumpGravity;
	float moveDuration;
	float jumpDuration;
	float rotation;
	float cutDirectionAngleOffset;
};
struct NoteSpawnInfoNetSerializable pkt_readNoteSpawnInfoNetSerializable(const uint8_t **pkt);
ENUM(uint8_t, ObstacleType, {
	ObstacleType_FullHeight,
	ObstacleType_Top,
})
struct ObstacleSpawnInfoNetSerializable {
	float time;
	int32_t lineIndex;
	ObstacleType obstacleType;
	float duration;
	int32_t width;
	struct Vector3Serializable moveStartPos;
	struct Vector3Serializable moveEndPos;
	struct Vector3Serializable jumpEndPos;
	float obstacleHeight;
	float moveDuration;
	float jumpDuration;
	float noteLinesDistance;
	float rotation;
};
struct ObstacleSpawnInfoNetSerializable pkt_readObstacleSpawnInfoNetSerializable(const uint8_t **pkt);
struct SetPlayersMissingEntitlementsToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayersMissingEntitlementsNetSerializable playersMissingEntitlements;
};
void pkt_writeSetPlayersMissingEntitlementsToLevel(uint8_t **pkt, struct SetPlayersMissingEntitlementsToLevel in, uint32_t protocolVersion);
struct GetIsEntitledToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct LongString levelId;
};
void pkt_writeGetIsEntitledToLevel(uint8_t **pkt, struct GetIsEntitledToLevel in, uint32_t protocolVersion);
struct SetIsEntitledToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct LongString levelId;
	EntitlementsStatus entitlementStatus;
};
struct SetIsEntitledToLevel pkt_readSetIsEntitledToLevel(const uint8_t **pkt, uint32_t protocolVersion);
struct SetSelectedBeatmap {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable identifier;
};
void pkt_writeSetSelectedBeatmap(uint8_t **pkt, struct SetSelectedBeatmap in, uint32_t protocolVersion);
struct RecommendBeatmap {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable identifier;
};
struct RecommendBeatmap pkt_readRecommendBeatmap(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeRecommendBeatmap(uint8_t **pkt, struct RecommendBeatmap in, uint32_t protocolVersion);
struct ClearRecommendedBeatmap {
	struct RemoteProcedureCall base;
};
struct ClearRecommendedBeatmap pkt_readClearRecommendedBeatmap(const uint8_t **pkt, uint32_t protocolVersion);
struct GetRecommendedBeatmap {
	struct RemoteProcedureCall base;
};
struct GetRecommendedBeatmap pkt_readGetRecommendedBeatmap(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeGetRecommendedBeatmap(uint8_t **pkt, struct GetRecommendedBeatmap in, uint32_t protocolVersion);
struct SetSelectedGameplayModifiers {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct GameplayModifiers gameplayModifiers;
};
void pkt_writeSetSelectedGameplayModifiers(uint8_t **pkt, struct SetSelectedGameplayModifiers in, uint32_t protocolVersion);
struct RecommendGameplayModifiers {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct GameplayModifiers gameplayModifiers;
};
struct RecommendGameplayModifiers pkt_readRecommendGameplayModifiers(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeRecommendGameplayModifiers(uint8_t **pkt, struct RecommendGameplayModifiers in, uint32_t protocolVersion);
struct ClearRecommendedGameplayModifiers {
	struct RemoteProcedureCall base;
};
struct ClearRecommendedGameplayModifiers pkt_readClearRecommendedGameplayModifiers(const uint8_t **pkt, uint32_t protocolVersion);
struct GetRecommendedGameplayModifiers {
	struct RemoteProcedureCall base;
};
struct GetRecommendedGameplayModifiers pkt_readGetRecommendedGameplayModifiers(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeGetRecommendedGameplayModifiers(uint8_t **pkt, struct GetRecommendedGameplayModifiers in, uint32_t protocolVersion);
struct StartLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable beatmapId;
	struct GameplayModifiers gameplayModifiers;
	float startTime;
};
void pkt_writeStartLevel(uint8_t **pkt, struct StartLevel in, uint32_t protocolVersion);
struct GetStartedLevel {
	struct RemoteProcedureCall base;
};
struct GetStartedLevel pkt_readGetStartedLevel(const uint8_t **pkt, uint32_t protocolVersion);
struct CancelLevelStart {
	struct RemoteProcedureCall base;
};
void pkt_writeCancelLevelStart(uint8_t **pkt, struct CancelLevelStart in, uint32_t protocolVersion);
struct GetMultiplayerGameState {
	struct RemoteProcedureCall base;
};
struct GetMultiplayerGameState pkt_readGetMultiplayerGameState(const uint8_t **pkt, uint32_t protocolVersion);
struct SetMultiplayerGameState {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	MultiplayerGameState lobbyState;
};
void pkt_writeSetMultiplayerGameState(uint8_t **pkt, struct SetMultiplayerGameState in, uint32_t protocolVersion);
struct GetIsReady {
	struct RemoteProcedureCall base;
};
struct GetIsReady pkt_readGetIsReady(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeGetIsReady(uint8_t **pkt, struct GetIsReady in, uint32_t protocolVersion);
struct SetIsReady {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	_Bool isReady;
};
struct SetIsReady pkt_readSetIsReady(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeSetIsReady(uint8_t **pkt, struct SetIsReady in, uint32_t protocolVersion);
struct GetIsInLobby {
	struct RemoteProcedureCall base;
};
struct GetIsInLobby pkt_readGetIsInLobby(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeGetIsInLobby(uint8_t **pkt, struct GetIsInLobby in, uint32_t protocolVersion);
struct SetIsInLobby {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	_Bool isBack;
};
struct SetIsInLobby pkt_readSetIsInLobby(const uint8_t **pkt, uint32_t protocolVersion);
void pkt_writeSetIsInLobby(uint8_t **pkt, struct SetIsInLobby in, uint32_t protocolVersion);
struct GetCountdownEndTime {
	struct RemoteProcedureCall base;
};
struct GetCountdownEndTime pkt_readGetCountdownEndTime(const uint8_t **pkt, uint32_t protocolVersion);
struct SetCountdownEndTime {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float newTime;
};
void pkt_writeSetCountdownEndTime(uint8_t **pkt, struct SetCountdownEndTime in, uint32_t protocolVersion);
struct CancelCountdown {
	struct RemoteProcedureCall base;
};
void pkt_writeCancelCountdown(uint8_t **pkt, struct CancelCountdown in, uint32_t protocolVersion);
struct GetOwnedSongPacks {
	struct RemoteProcedureCall base;
};
void pkt_writeGetOwnedSongPacks(uint8_t **pkt, struct GetOwnedSongPacks in, uint32_t protocolVersion);
struct SetOwnedSongPacks {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct SongPackMask songPackMask;
};
struct SetOwnedSongPacks pkt_readSetOwnedSongPacks(const uint8_t **pkt, uint32_t protocolVersion);
struct GetPermissionConfiguration {
	struct RemoteProcedureCall base;
};
struct GetPermissionConfiguration pkt_readGetPermissionConfiguration(const uint8_t **pkt, uint32_t protocolVersion);
struct SetPermissionConfiguration {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayersLobbyPermissionConfigurationNetSerializable playersPermissionConfiguration;
};
void pkt_writeSetPermissionConfiguration(uint8_t **pkt, struct SetPermissionConfiguration in, uint32_t protocolVersion);
struct SetIsStartButtonEnabled {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	CannotStartGameReason reason;
};
void pkt_writeSetIsStartButtonEnabled(uint8_t **pkt, struct SetIsStartButtonEnabled in, uint32_t protocolVersion);
ENUM(uint8_t, MenuRpcType, {
	MenuRpcType_SetPlayersMissingEntitlementsToLevel,
	MenuRpcType_GetIsEntitledToLevel,
	MenuRpcType_SetIsEntitledToLevel,
	MenuRpcType_InvalidateLevelEntitlementStatuses,
	MenuRpcType_SelectLevelPack,
	MenuRpcType_SetSelectedBeatmap,
	MenuRpcType_GetSelectedBeatmap,
	MenuRpcType_RecommendBeatmap,
	MenuRpcType_ClearRecommendedBeatmap,
	MenuRpcType_GetRecommendedBeatmap,
	MenuRpcType_SetSelectedGameplayModifiers,
	MenuRpcType_GetSelectedGameplayModifiers,
	MenuRpcType_RecommendGameplayModifiers,
	MenuRpcType_ClearRecommendedGameplayModifiers,
	MenuRpcType_GetRecommendedGameplayModifiers,
	MenuRpcType_LevelLoadError,
	MenuRpcType_LevelLoadSuccess,
	MenuRpcType_StartLevel,
	MenuRpcType_GetStartedLevel,
	MenuRpcType_CancelLevelStart,
	MenuRpcType_GetMultiplayerGameState,
	MenuRpcType_SetMultiplayerGameState,
	MenuRpcType_GetIsReady,
	MenuRpcType_SetIsReady,
	MenuRpcType_SetStartGameTime,
	MenuRpcType_CancelStartGameTime,
	MenuRpcType_GetIsInLobby,
	MenuRpcType_SetIsInLobby,
	MenuRpcType_GetCountdownEndTime,
	MenuRpcType_SetCountdownEndTime,
	MenuRpcType_CancelCountdown,
	MenuRpcType_GetOwnedSongPacks,
	MenuRpcType_SetOwnedSongPacks,
	MenuRpcType_RequestKickPlayer,
	MenuRpcType_GetPermissionConfiguration,
	MenuRpcType_SetPermissionConfiguration,
	MenuRpcType_GetIsStartButtonEnabled,
	MenuRpcType_SetIsStartButtonEnabled,
})
struct SetGameplaySceneSyncFinish {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayerSpecificSettingsAtStartNetSerializable playersAtGameStart;
	struct String sessionGameId;
};
void pkt_writeSetGameplaySceneSyncFinish(uint8_t **pkt, struct SetGameplaySceneSyncFinish in, uint32_t protocolVersion);
struct SetGameplaySceneReady {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayerSpecificSettingsNetSerializable playerSpecificSettingsNetSerializable;
};
struct SetGameplaySceneReady pkt_readSetGameplaySceneReady(const uint8_t **pkt, uint32_t protocolVersion);
struct GetGameplaySceneReady {
	struct RemoteProcedureCall base;
};
void pkt_writeGetGameplaySceneReady(uint8_t **pkt, struct GetGameplaySceneReady in, uint32_t protocolVersion);
struct SetGameplaySongReady {
	struct RemoteProcedureCall base;
};
struct SetGameplaySongReady pkt_readSetGameplaySongReady(const uint8_t **pkt, uint32_t protocolVersion);
struct GetGameplaySongReady {
	struct RemoteProcedureCall base;
};
void pkt_writeGetGameplaySongReady(uint8_t **pkt, struct GetGameplaySongReady in, uint32_t protocolVersion);
struct SetSongStartTime {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float startTime;
};
void pkt_writeSetSongStartTime(uint8_t **pkt, struct SetSongStartTime in, uint32_t protocolVersion);
struct NoteCut {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteCutInfoNetSerializable noteCutInfo;
};
struct NoteCut pkt_readNoteCut(const uint8_t **pkt, uint32_t protocolVersion);
struct NoteMissed {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteMissInfoNetSerializable noteMissInfo;
};
struct NoteMissed pkt_readNoteMissed(const uint8_t **pkt, uint32_t protocolVersion);
struct LevelFinished {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct MultiplayerLevelCompletionResults results;
};
struct LevelFinished pkt_readLevelFinished(const uint8_t **pkt, uint32_t protocolVersion);
struct ReturnToMenu {
	struct RemoteProcedureCall base;
};
void pkt_writeReturnToMenu(uint8_t **pkt, struct ReturnToMenu in, uint32_t protocolVersion);
struct RequestReturnToMenu {
	struct RemoteProcedureCall base;
};
struct RequestReturnToMenu pkt_readRequestReturnToMenu(const uint8_t **pkt, uint32_t protocolVersion);
struct NoteSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteSpawnInfoNetSerializable noteSpawnInfo;
};
struct NoteSpawned pkt_readNoteSpawned(const uint8_t **pkt, uint32_t protocolVersion);
struct ObstacleSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct ObstacleSpawnInfoNetSerializable obstacleSpawnInfo;
};
struct ObstacleSpawned pkt_readObstacleSpawned(const uint8_t **pkt, uint32_t protocolVersion);
ENUM(uint8_t, GameplayRpcType, {
	GameplayRpcType_SetGameplaySceneSyncFinish,
	GameplayRpcType_SetGameplaySceneReady,
	GameplayRpcType_GetGameplaySceneReady,
	GameplayRpcType_SetActivePlayerFailedToConnect,
	GameplayRpcType_SetGameplaySongReady,
	GameplayRpcType_GetGameplaySongReady,
	GameplayRpcType_SetSongStartTime,
	GameplayRpcType_NoteCut,
	GameplayRpcType_NoteMissed,
	GameplayRpcType_LevelFinished,
	GameplayRpcType_ReturnToMenu,
	GameplayRpcType_RequestReturnToMenu,
	GameplayRpcType_NoteSpawned,
	GameplayRpcType_ObstacleSpawned,
})
struct NodePoseSyncState {
	struct SyncStateId id;
	float time;
	struct NodePoseSyncState1 state;
};
struct NodePoseSyncState pkt_readNodePoseSyncState(const uint8_t **pkt);
struct ScoreSyncState {
	struct SyncStateId id;
	float time;
	struct StandardScoreSyncState state;
};
struct ScoreSyncState pkt_readScoreSyncState(const uint8_t **pkt);
struct NodePoseSyncStateDelta {
	struct SyncStateId baseId;
	int32_t timeOffsetMs;
	struct NodePoseSyncState1 delta;
};
struct NodePoseSyncStateDelta pkt_readNodePoseSyncStateDelta(const uint8_t **pkt);
struct ScoreSyncStateDelta {
	struct SyncStateId baseId;
	int32_t timeOffsetMs;
	struct StandardScoreSyncState delta;
};
struct ScoreSyncStateDelta pkt_readScoreSyncStateDelta(const uint8_t **pkt);
struct MpCore {
	struct String packetType;
};
struct MpCore pkt_readMpCore(const uint8_t **pkt);
ENUM(uint8_t, MultiplayerSessionMessageType, {
	MultiplayerSessionMessageType_MenuRpc,
	MultiplayerSessionMessageType_GameplayRpc,
	MultiplayerSessionMessageType_NodePoseSyncState,
	MultiplayerSessionMessageType_ScoreSyncState,
	MultiplayerSessionMessageType_NodePoseSyncStateDelta,
	MultiplayerSessionMessageType_ScoreSyncStateDelta,
	MultiplayerSessionMessageType_MpCore = 100,
})
struct MultiplayerSessionMessageHeader {
	MultiplayerSessionMessageType type;
};
struct MultiplayerSessionMessageHeader pkt_readMultiplayerSessionMessageHeader(const uint8_t **pkt);
void pkt_writeMultiplayerSessionMessageHeader(uint8_t **pkt, struct MultiplayerSessionMessageHeader in);
struct MenuRpcHeader {
	MenuRpcType type;
};
struct MenuRpcHeader pkt_readMenuRpcHeader(const uint8_t **pkt);
void pkt_writeMenuRpcHeader(uint8_t **pkt, struct MenuRpcHeader in);
struct GameplayRpcHeader {
	GameplayRpcType type;
};
struct GameplayRpcHeader pkt_readGameplayRpcHeader(const uint8_t **pkt);
void pkt_writeGameplayRpcHeader(uint8_t **pkt, struct GameplayRpcHeader in);
struct MpBeatmapPacket {
	struct String levelHash;
	struct LongString songName;
	struct LongString songSubName;
	struct LongString songAuthorName;
	struct LongString levelAuthorName;
	float beatsPerMinute;
	float songDuration;
	struct String characteristic;
	BeatmapDifficulty difficulty;
};
struct MpBeatmapPacket pkt_readMpBeatmapPacket(const uint8_t **pkt);
ENUM(int32_t, MpPlatform, {
	MpPlatform_Unknown,
	MpPlatform_Steam,
	MpPlatform_OculusPC,
	MpPlatform_OculusQuest,
	MpPlatform_PS4,
})
struct MpPlayerData {
	struct String platformId;
	MpPlatform platform;
};
struct MpPlayerData pkt_readMpPlayerData(const uint8_t **pkt);
struct AuthenticateUserRequest {
	struct BaseMasterServerReliableResponse base;
	struct AuthenticationToken authenticationToken;
};
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(const uint8_t **pkt);
struct AuthenticateUserResponse {
	struct BaseMasterServerReliableResponse base;
	AuthenticateUserResponse_Result result;
};
void pkt_writeAuthenticateUserResponse(uint8_t **pkt, struct AuthenticateUserResponse in);
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
void pkt_writeConnectToServerResponse(uint8_t **pkt, struct ConnectToServerResponse in);
struct ConnectToServerRequest {
	struct BaseConnectToServerRequest base;
	struct BeatmapLevelSelectionMask selectionMask;
	struct String secret;
	ServerCode code;
	struct GameplayServerConfiguration configuration;
};
struct ConnectToServerRequest pkt_readConnectToServerRequest(const uint8_t **pkt);
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
	DedicatedServerMessageType_DedicatedServerMessageReceivedAcknowledge = 13,
	DedicatedServerMessageType_DedicatedServerMultipartMessage = 14,
})
ENUM(uint8_t, GameLiftMessageType, {
	GameLiftMessageType_AuthenticateUserRequest,
	GameLiftMessageType_AuthenticateUserResponse,
	GameLiftMessageType_GameLiftMessageReceivedAcknowledge,
	GameLiftMessageType_GameLiftMultipartMessage,
})
struct ClientHelloRequest {
	struct BaseMasterServerReliableRequest base;
	uint8_t random[32];
};
struct ClientHelloRequest pkt_readClientHelloRequest(const uint8_t **pkt);
struct HelloVerifyRequest {
	struct BaseMasterServerReliableResponse base;
	uint8_t cookie[32];
};
void pkt_writeHelloVerifyRequest(uint8_t **pkt, struct HelloVerifyRequest in);
struct ClientHelloWithCookieRequest {
	struct BaseMasterServerReliableRequest base;
	uint32_t certificateResponseId;
	uint8_t random[32];
	uint8_t cookie[32];
};
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(const uint8_t **pkt);
struct ServerHelloRequest {
	struct BaseMasterServerReliableResponse base;
	uint8_t random[32];
	struct ByteArrayNetSerializable publicKey;
	struct ByteArrayNetSerializable signature;
};
void pkt_writeServerHelloRequest(uint8_t **pkt, struct ServerHelloRequest in);
struct ServerCertificateRequest {
	struct BaseMasterServerReliableResponse base;
	uint32_t certificateCount;
	struct ByteArrayNetSerializable certificateList[10];
};
void pkt_writeServerCertificateRequest(uint8_t **pkt, struct ServerCertificateRequest in);
struct ClientKeyExchangeRequest {
	struct BaseMasterServerReliableResponse base;
	struct ByteArrayNetSerializable clientPublicKey;
};
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(const uint8_t **pkt);
struct ChangeCipherSpecRequest {
	struct BaseMasterServerReliableResponse base;
};
void pkt_writeChangeCipherSpecRequest(uint8_t **pkt, struct ChangeCipherSpecRequest in);
ENUM(uint8_t, HandshakeMessageType, {
	HandshakeMessageType_ClientHelloRequest,
	HandshakeMessageType_HelloVerifyRequest,
	HandshakeMessageType_ClientHelloWithCookieRequest,
	HandshakeMessageType_ServerHelloRequest,
	HandshakeMessageType_ServerCertificateRequest,
	HandshakeMessageType_ClientKeyExchangeRequest = 6,
	HandshakeMessageType_ChangeCipherSpecRequest,
	HandshakeMessageType_HandshakeMessageReceivedAcknowledge,
	HandshakeMessageType_HandshakeMultipartMessage,
})
#ifdef reflect
static const char *const reflect_MessageType = "{MessageType_UserMessage=1,MessageType_DedicatedServerMessage=2,MessageType_GameLiftMessage=3,MessageType_HandshakeMessage=3192347326,}";
#endif
typedef uint32_t MessageType;
#define MessageType_UserMessage 1
#define MessageType_DedicatedServerMessage 2
#define MessageType_GameLiftMessage 3
#define MessageType_HandshakeMessage 3192347326u
struct MessageHeader {
	MessageType type;
	uint32_t protocolVersion;
};
struct MessageHeader pkt_readMessageHeader(const uint8_t **pkt);
void pkt_writeMessageHeader(uint8_t **pkt, struct MessageHeader in);
struct SerializeHeader {
	uint32_t length;
	uint8_t type;
};
struct SerializeHeader pkt_readSerializeHeader(const uint8_t **pkt);
void pkt_writeSerializeHeader(uint8_t **pkt, struct SerializeHeader in);
#endif // PACKETS_H
