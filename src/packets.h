#ifndef PACKETS_H
#define PACKETS_H

/* 
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 * AUTO GENERATED; DO NOT TOUCH
 */

#include "log.h"
#include "enum.h"
#include <stdint.h>

struct PacketContext {
	uint8_t netVersion;
	uint8_t protocolVersion;
	uint8_t windowSize;
	uint8_t beatUpVersion;
};
#define PV_LEGACY_DEFAULT (struct PacketContext){11, 6, 64, 0}

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
struct String {
	uint32_t length;
	_Bool isNull;
	char data[60];
};
struct LongString {
	uint32_t length;
	_Bool isNull;
	char data[4096];
};
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
typedef uint32_t ServerCode;
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
ENUM(uint8_t, DeliveryMethod, {
	DeliveryMethod_ReliableUnordered,
	DeliveryMethod_Sequenced,
	DeliveryMethod_ReliableOrdered,
	DeliveryMethod_ReliableSequenced,
})
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
ENUM(int32_t, GameplayType, {
	GameplayType_Normal,
	GameplayType_Bomb,
	GameplayType_BurstSliderHead,
	GameplayType_BurstSliderElement,
	GameplayType_BurstSliderElementFill,
})
ENUM(int32_t, ScoringType, {
	ScoringType_Ignore = -1,
	ScoringType_NoScore,
	ScoringType_Normal,
	ScoringType_SliderHead,
	ScoringType_SliderTail,
	ScoringType_BurstSliderHead,
	ScoringType_BurstSliderElement,
})
ENUM(int32_t, ColorType, {
	ColorType_ColorA = 0,
	ColorType_ColorB = 1,
	ColorType_None = -1,
})
typedef uint8_t NoteLineLayer;
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
ENUM(uint8_t, ObstacleType, {
	ObstacleType_FullHeight,
	ObstacleType_Top,
})
ENUM(uint8_t, SliderType, {
	SliderType_Normal,
	SliderType_Burst,
})
ENUM(uint8_t, SliderMidAnchorMode, {
	SliderMidAnchorMode_Straight,
	SliderMidAnchorMode_Clockwise,
	SliderMidAnchorMode_CounterClockwise,
})
ENUM(uint8_t, BeatUpMessageType, {
	BeatUpMessageType_RecommendPreview,
	BeatUpMessageType_SetCanShareBeatmap,
	BeatUpMessageType_DirectDownloadInfo,
	BeatUpMessageType_LevelFragmentRequest,
	BeatUpMessageType_LevelFragment,
})
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
	GameplayRpcType_SliderSpawned,
})
ENUM(uint8_t, MultiplayerSessionMessageType, {
	MultiplayerSessionMessageType_MenuRpc,
	MultiplayerSessionMessageType_GameplayRpc,
	MultiplayerSessionMessageType_NodePoseSyncState,
	MultiplayerSessionMessageType_ScoreSyncState,
	MultiplayerSessionMessageType_NodePoseSyncStateDelta,
	MultiplayerSessionMessageType_ScoreSyncStateDelta,
	MultiplayerSessionMessageType_MpCore = 100,
	MultiplayerSessionMessageType_BeatUpMessage = 101,
})
ENUM(int32_t, MpPlatform, {
	MpPlatform_Unknown,
	MpPlatform_Steam,
	MpPlatform_OculusPC,
	MpPlatform_OculusQuest,
	MpPlatform_PS4,
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
	DedicatedServerMessageType_DedicatedServerMessageReceivedAcknowledge = 13,
	DedicatedServerMessageType_DedicatedServerMultipartMessage = 14,
})
ENUM(uint8_t, GameLiftMessageType, {
	GameLiftMessageType_AuthenticateUserRequest,
	GameLiftMessageType_AuthenticateUserResponse,
	GameLiftMessageType_GameLiftMessageReceivedAcknowledge,
	GameLiftMessageType_GameLiftMultipartMessage,
})
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
struct PacketEncryptionLayer {
	_Bool encrypted;
	uint32_t sequenceId;
	uint8_t iv[16];
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
	uint8_t data[8192];
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
	uint64_t d0;
	uint64_t d1;
};
struct SongPackMask {
	struct BitMask128 bloomFilter;
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
struct Channeled {
	uint16_t sequence;
	DeliveryMethod channelId;
};
struct Ack {
	uint16_t sequence;
	DeliveryMethod channelId;
	uint8_t data[16];
	uint8_t _pad0;
};
struct Ping {
	uint16_t sequence;
};
struct Pong {
	uint16_t sequence;
	uint64_t time;
};
struct ConnectRequest {
	uint32_t protocolId;
	uint64_t connectTime;
	int32_t peerId;
	uint8_t addrlen;
	uint8_t address[38];
	struct String secret;
	struct String userId;
	struct String userName;
	_Bool isConnectionOwner;
};
struct ConnectAccept {
	uint64_t connectTime;
	uint8_t connectNum;
	_Bool reusedPeer;
	int32_t peerId;
	uint32_t windowSize;
	_Bool skipResults;
};
struct Disconnect {
	uint8_t _pad0[8];
};
struct MtuCheck {
	uint32_t newMtu0;
	uint8_t pad[1423];
	uint32_t newMtu1;
};
struct MtuOk {
	uint32_t newMtu0;
	uint8_t pad[1423];
	uint32_t newMtu1;
};
struct ModConnectHeader {
	uint32_t length;
	struct String name;
};
struct BeatUpConnectHeader {
	uint32_t protocolId;
	uint32_t windowSize;
	uint8_t countdownDuration;
	_Bool directDownloads;
	_Bool skipResults;
	_Bool perPlayerDifficulty;
	_Bool perPlayerModifiers;
};
struct NetPacketHeader {
	PacketProperty property;
	uint8_t connectionNumber;
	_Bool isFragmented;
};
struct FragmentedHeader {
	uint16_t fragmentId;
	uint16_t fragmentPart;
	uint16_t fragmentsTotal;
};
struct PlayerStateHash {
	struct BitMask128 bloomFilter;
};
struct Color32 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};
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
struct RoutingHeader {
	uint8_t remoteConnectionId;
	uint8_t connectionId;
	_Bool encrypted;
};
struct SyncTime {
	float syncTime;
};
struct PlayerConnected {
	uint8_t remoteConnectionId;
	struct String userId;
	struct String userName;
	_Bool isConnectionOwner;
};
struct PlayerIdentity {
	struct PlayerStateHash playerState;
	struct MultiplayerAvatarData playerAvatar;
	struct ByteArrayNetSerializable random;
	struct ByteArrayNetSerializable publicEncryptionKey;
};
struct PlayerLatencyUpdate {
	float latency;
};
struct PlayerDisconnected {
	DisconnectedReason disconnectedReason;
};
struct PlayerSortOrderUpdate {
	struct String userId;
	int32_t sortIndex;
};
struct PlayerStateUpdate {
	struct PlayerStateHash playerState;
};
struct PingMessage {
	float pingTime;
};
struct PongMessage {
	float pingTime;
};
struct RemoteProcedureCall {
	float syncTime;
};
struct RemoteProcedureCallFlags {
	_Bool hasValue0;
	_Bool hasValue1;
	_Bool hasValue2;
	_Bool hasValue3;
};
struct PlayersMissingEntitlementsNetSerializable {
	int32_t count;
	struct String playersWithoutEntitlements[128];
};
struct BeatmapIdentifierNetSerializable {
	struct LongString levelID;
	struct String beatmapCharacteristicSerializedName;
	BeatmapDifficulty difficulty;
};
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
struct PlayerLobbyPermissionConfigurationNetSerializable {
	struct String userId;
	_Bool isServerOwner;
	_Bool hasRecommendBeatmapsPermission;
	_Bool hasRecommendGameplayModifiersPermission;
	_Bool hasKickVotePermission;
	_Bool hasInvitePermission;
};
struct PlayersLobbyPermissionConfigurationNetSerializable {
	int32_t count;
	struct PlayerLobbyPermissionConfigurationNetSerializable playersPermission[128];
};
struct SyncStateId {
	uint8_t id;
	_Bool same;
};
struct Vector3Serializable {
	int32_t x;
	int32_t y;
	int32_t z;
};
struct QuaternionSerializable {
	int32_t a;
	int32_t b;
	int32_t c;
};
struct PoseSerializable {
	struct Vector3Serializable position;
	struct QuaternionSerializable rotation;
};
struct ColorNoAlphaSerializable {
	float r;
	float g;
	float b;
};
struct ColorSchemeNetSerializable {
	struct ColorNoAlphaSerializable saberAColor;
	struct ColorNoAlphaSerializable saberBColor;
	struct ColorNoAlphaSerializable obstaclesColor;
	struct ColorNoAlphaSerializable environmentColor0;
	struct ColorNoAlphaSerializable environmentColor1;
	struct ColorNoAlphaSerializable environmentColor0Boost;
	struct ColorNoAlphaSerializable environmentColor1Boost;
};
struct PlayerSpecificSettingsNetSerializable {
	struct String userId;
	struct String userName;
	_Bool leftHanded;
	_Bool automaticPlayerHeight;
	float playerHeight;
	float headPosToPlayerHeightOffset;
	struct ColorSchemeNetSerializable colorScheme;
};
struct PlayerSpecificSettingsAtStartNetSerializable {
	int32_t count;
	struct PlayerSpecificSettingsNetSerializable activePlayerSpecificSettingsAtGameStart[128];
};
struct NoteCutInfoNetSerializable {
	_Bool cutWasOk;
	float saberSpeed;
	struct Vector3Serializable saberDir;
	struct Vector3Serializable cutPoint;
	struct Vector3Serializable cutNormal;
	struct Vector3Serializable notePosition;
	struct Vector3Serializable noteScale;
	struct QuaternionSerializable noteRotation;
	GameplayType gameplayType;
	ColorType colorType;
	NoteLineLayer lineLayer;
	int32_t noteLineIndex;
	float noteTime;
	float timeToNextColorNote;
	struct Vector3Serializable moveVec;
};
struct NoteMissInfoNetSerializable {
	ColorType colorType;
	NoteLineLayer lineLayer;
	int32_t noteLineIndex;
	float noteTime;
};
struct LevelCompletionResults {
	struct GameplayModifiers gameplayModifiers;
	int32_t modifiedScore;
	int32_t multipliedScore;
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
	int32_t totalCutScore;
	int32_t goodCutsCountForNotesWithFullScoreScoringType;
	int32_t averageCenterDistanceCutScoreForNotesWithFullScoreScoringType;
	int32_t averageCutScoreForNotesWithFullScoreScoringType;
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
struct MultiplayerLevelCompletionResults {
	MultiplayerLevelEndState levelEndState;
	MultiplayerPlayerLevelEndState playerLevelEndState;
	MultiplayerPlayerLevelEndReason playerLevelEndReason;
	struct LevelCompletionResults levelCompletionResults;
};
struct NodePoseSyncState1 {
	struct PoseSerializable head;
	struct PoseSerializable leftController;
	struct PoseSerializable rightController;
};
struct StandardScoreSyncState {
	int32_t modifiedScore;
	int32_t rawScore;
	int32_t immediateMaxPossibleRawScore;
	int32_t combo;
	int32_t multiplier;
};
struct NoteSpawnInfoNetSerializable {
	float time;
	int32_t lineIndex;
	NoteLineLayer noteLineLayer;
	NoteLineLayer beforeJumpNoteLineLayer;
	GameplayType gameplayType;
	ScoringType scoringType;
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
	float cutSfxVolumeMultiplier;
};
struct ObstacleSpawnInfoNetSerializable {
	float time;
	int32_t lineIndex;
	NoteLineLayer lineLayer;
	ObstacleType obstacleType;
	float duration;
	int32_t width;
	int32_t height;
	struct Vector3Serializable moveStartPos;
	struct Vector3Serializable moveEndPos;
	struct Vector3Serializable jumpEndPos;
	float obstacleHeight;
	float moveDuration;
	float jumpDuration;
	float noteLinesDistance;
	float rotation;
};
struct SliderSpawnInfoNetSerializable {
	ColorType colorType;
	SliderType sliderType;
	_Bool hasHeadNote;
	float headTime;
	int32_t headLineIndex;
	NoteLineLayer headLineLayer;
	NoteLineLayer headBeforeJumpLineLayer;
	float headControlPointLengthMultiplier;
	NoteCutDirection headCutDirection;
	float headCutDirectionAngleOffset;
	_Bool hasTailNote;
	float tailTime;
	int32_t tailLineIndex;
	NoteLineLayer tailLineLayer;
	NoteLineLayer tailBeforeJumpLineLayer;
	float tailControlPointLengthMultiplier;
	NoteCutDirection tailCutDirection;
	float tailCutDirectionAngleOffset;
	SliderMidAnchorMode midAnchorMode;
	int32_t sliceCount;
	float squishAmount;
	struct Vector3Serializable headMoveStartPos;
	struct Vector3Serializable headJumpStartPos;
	struct Vector3Serializable headJumpEndPos;
	float headJumpGravity;
	struct Vector3Serializable tailMoveStartPos;
	struct Vector3Serializable tailJumpStartPos;
	struct Vector3Serializable tailJumpEndPos;
	float tailJumpGravity;
	float moveDuration;
	float jumpDuration;
	float rotation;
};
struct PreviewDifficultyBeatmapSet {
	struct String beatmapCharacteristicSerializedName;
	uint8_t count;
	BeatmapDifficulty difficulties[5];
};
struct NetworkPreviewBeatmapLevel {
	struct LongString levelId;
	struct LongString songName;
	struct LongString songSubName;
	struct LongString songAuthorName;
	struct LongString levelAuthorName;
	float beatsPerMinute;
	float songTimeOffset;
	float shuffle;
	float shufflePeriod;
	float previewStartTime;
	float previewDuration;
	float songDuration;
	uint8_t count;
	struct PreviewDifficultyBeatmapSet previewDifficultyBeatmapSets[8];
	struct ByteArrayNetSerializable cover;
};
struct RecommendPreview {
	struct NetworkPreviewBeatmapLevel preview;
	uint32_t requirements_len;
	struct String requirements[16];
	uint32_t suggestions_len;
	struct String suggestions[16];
};
struct SetCanShareBeatmap {
	struct LongString levelId;
	struct String levelHash;
	uint64_t fileSize;
	_Bool canShare;
};
struct DirectDownloadInfo {
	struct LongString levelId;
	struct String levelHash;
	uint64_t fileSize;
	uint8_t count;
	struct String sourcePlayers[128];
};
struct LevelFragmentRequest {
	uint64_t offset;
	uint16_t maxSize;
};
struct LevelFragment {
	uint64_t offset;
	uint16_t size;
	uint8_t data[1500];
};
struct BeatUpMessageHeader {
	BeatUpMessageType type;
};
struct SetPlayersMissingEntitlementsToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayersMissingEntitlementsNetSerializable playersMissingEntitlements;
};
struct GetIsEntitledToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct LongString levelId;
};
struct SetIsEntitledToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct LongString levelId;
	EntitlementsStatus entitlementStatus;
};
struct SetSelectedBeatmap {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable identifier;
};
struct GetSelectedBeatmap {
	struct RemoteProcedureCall base;
};
struct RecommendBeatmap {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable identifier;
};
struct ClearRecommendedBeatmap {
	struct RemoteProcedureCall base;
};
struct GetRecommendedBeatmap {
	struct RemoteProcedureCall base;
};
struct SetSelectedGameplayModifiers {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct GameplayModifiers gameplayModifiers;
};
struct GetSelectedGameplayModifiers {
	struct RemoteProcedureCall base;
};
struct RecommendGameplayModifiers {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct GameplayModifiers gameplayModifiers;
};
struct ClearRecommendedGameplayModifiers {
	struct RemoteProcedureCall base;
};
struct GetRecommendedGameplayModifiers {
	struct RemoteProcedureCall base;
};
struct StartLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable beatmapId;
	struct GameplayModifiers gameplayModifiers;
	float startTime;
};
struct GetStartedLevel {
	struct RemoteProcedureCall base;
};
struct CancelLevelStart {
	struct RemoteProcedureCall base;
};
struct GetMultiplayerGameState {
	struct RemoteProcedureCall base;
};
struct SetMultiplayerGameState {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	MultiplayerGameState lobbyState;
};
struct GetIsReady {
	struct RemoteProcedureCall base;
};
struct SetIsReady {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	_Bool isReady;
};
struct GetIsInLobby {
	struct RemoteProcedureCall base;
};
struct SetIsInLobby {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	_Bool isBack;
};
struct GetCountdownEndTime {
	struct RemoteProcedureCall base;
};
struct SetCountdownEndTime {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float newTime;
};
struct CancelCountdown {
	struct RemoteProcedureCall base;
};
struct GetOwnedSongPacks {
	struct RemoteProcedureCall base;
};
struct SetOwnedSongPacks {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct SongPackMask songPackMask;
};
struct GetPermissionConfiguration {
	struct RemoteProcedureCall base;
};
struct SetPermissionConfiguration {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayersLobbyPermissionConfigurationNetSerializable playersPermissionConfiguration;
};
struct SetIsStartButtonEnabled {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	CannotStartGameReason reason;
};
struct SetGameplaySceneSyncFinish {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayerSpecificSettingsAtStartNetSerializable playersAtGameStart;
	struct String sessionGameId;
};
struct SetGameplaySceneReady {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayerSpecificSettingsNetSerializable playerSpecificSettingsNetSerializable;
};
struct GetGameplaySceneReady {
	struct RemoteProcedureCall base;
};
struct SetGameplaySongReady {
	struct RemoteProcedureCall base;
};
struct GetGameplaySongReady {
	struct RemoteProcedureCall base;
};
struct SetSongStartTime {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float startTime;
};
struct NoteCut {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteCutInfoNetSerializable noteCutInfo;
};
struct NoteMissed {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteMissInfoNetSerializable noteMissInfo;
};
struct LevelFinished {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct MultiplayerLevelCompletionResults results;
};
struct ReturnToMenu {
	struct RemoteProcedureCall base;
};
struct RequestReturnToMenu {
	struct RemoteProcedureCall base;
};
struct NoteSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteSpawnInfoNetSerializable noteSpawnInfo;
};
struct ObstacleSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct ObstacleSpawnInfoNetSerializable obstacleSpawnInfo;
};
struct SliderSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct SliderSpawnInfoNetSerializable sliderSpawnInfo;
};
struct NodePoseSyncState {
	struct SyncStateId id;
	float time;
	struct NodePoseSyncState1 state;
};
struct ScoreSyncState {
	struct SyncStateId id;
	float time;
	struct StandardScoreSyncState state;
};
struct NodePoseSyncStateDelta {
	struct SyncStateId baseId;
	int32_t timeOffsetMs;
	struct NodePoseSyncState1 delta;
};
struct ScoreSyncStateDelta {
	struct SyncStateId baseId;
	int32_t timeOffsetMs;
	struct StandardScoreSyncState delta;
};
struct MpCore {
	struct String type;
};
struct MultiplayerSessionMessageHeader {
	MultiplayerSessionMessageType type;
};
struct MenuRpcHeader {
	MenuRpcType type;
};
struct GameplayRpcHeader {
	GameplayRpcType type;
};
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
struct MpPlayerData {
	struct String platformId;
	MpPlatform platform;
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
struct ClientKeyExchangeRequest {
	struct BaseMasterServerReliableResponse base;
	struct ByteArrayNetSerializable clientPublicKey;
};
struct ChangeCipherSpecRequest {
	struct BaseMasterServerReliableResponse base;
};
struct MessageHeader {
	MessageType type;
	uint32_t protocolVersion;
};
struct SerializeHeader {
	uint32_t length;
	uint8_t type;
};
#define CONCAT1(a, b) a##b
#define CONCAT0(a, b) CONCAT1(a, b)
#define SERIALIZE_CUSTOM(ctx, pkt, stype) \
	for(uint8_t *start = *pkt; start;) \
		if(*pkt != start) { \
			struct SerializeHeader serial; \
			serial.length = *pkt + 1 - start; \
			serial.type = stype; \
			*pkt = start, start = NULL; \
			pkt_writeSerializeHeader(ctx, pkt, serial); \
			/*uprintf("serialize " #stype "\n");*/ \
			goto CONCAT0(_body_, __LINE__); \
		} else CONCAT0(_body_, __LINE__):
uint8_t pkt_readUint8(struct PacketContext ctx, const uint8_t **pkt);
uint16_t pkt_readUint16(struct PacketContext ctx, const uint8_t **pkt);
uint32_t pkt_readUint32(struct PacketContext ctx, const uint8_t **pkt);
uint64_t pkt_readUint64(struct PacketContext ctx, const uint8_t **pkt);
#define pkt_readInt8(ctx, pkt) (int8_t)pkt_readUint8(ctx, pkt)
#define pkt_readInt16(ctx, pkt) (int16_t)pkt_readUint16(ctx, pkt)
#define pkt_readInt32(ctx, pkt) (int32_t)pkt_readUint32(ctx, pkt)
#define pkt_readInt64(ctx, pkt) (int64_t)pkt_readUint64(ctx, pkt)
uint64_t pkt_readVarUint64(struct PacketContext ctx, const uint8_t **pkt);
uint64_t pkt_readVarUint64(struct PacketContext ctx, const uint8_t **pkt);
int64_t pkt_readVarInt64(struct PacketContext ctx, const uint8_t **pkt);
uint32_t pkt_readVarUint32(struct PacketContext ctx, const uint8_t **pkt);
int32_t pkt_readVarInt32(struct PacketContext ctx, const uint8_t **pkt);
void pkt_readUint8Array(const uint8_t **pkt, uint8_t *out, uint32_t count);
float pkt_readFloat32(struct PacketContext ctx, const uint8_t **pkt);
double pkt_readFloat64(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeUint8(struct PacketContext ctx, uint8_t **pkt, uint8_t v);
void pkt_writeUint16(struct PacketContext ctx, uint8_t **pkt, uint16_t v);
void pkt_writeUint32(struct PacketContext ctx, uint8_t **pkt, uint32_t v);
void pkt_writeUint64(struct PacketContext ctx, uint8_t **pkt, uint64_t v);
#define pkt_writeInt8(ctx, pkt, v) pkt_writeUint8(ctx, pkt, (int8_t)v)
#define pkt_writeInt16(ctx, pkt, v) pkt_writeUint16(ctx, pkt, (int16_t)v)
#define pkt_writeInt32(ctx, pkt, v) pkt_writeUint32(ctx, pkt, (int32_t)v)
#define pkt_writeInt64(ctx, pkt, v) pkt_writeUint64(ctx, pkt, (int64_t)v)
void pkt_writeVarUint64(struct PacketContext ctx, uint8_t **pkt, uint64_t v);
void pkt_writeVarInt64(struct PacketContext ctx, uint8_t **pkt, int64_t v);
void pkt_writeVarUint32(struct PacketContext ctx, uint8_t **pkt, uint32_t v);
void pkt_writeVarInt32(struct PacketContext ctx, uint8_t **pkt, int32_t v);
#define pkt_writeInt8Array(ctx, pkt, out, count) pkt_writeUint8Array(ctx, pkt, (uint8_t*)out, count)
void pkt_writeUint8Array(struct PacketContext ctx, uint8_t **pkt, const uint8_t *in, uint32_t count);
void pkt_writeFloat32(struct PacketContext ctx, uint8_t **pkt, float v);
void pkt_writeFloat64(struct PacketContext ctx, uint8_t **pkt, double v);
struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePacketEncryptionLayer(struct PacketContext ctx, uint8_t **pkt, struct PacketEncryptionLayer in);
struct BaseMasterServerReliableRequest pkt_readBaseMasterServerReliableRequest(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBaseMasterServerReliableRequest(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerReliableRequest in);
struct BaseMasterServerResponse pkt_readBaseMasterServerResponse(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBaseMasterServerResponse(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerResponse in);
struct BaseMasterServerReliableResponse pkt_readBaseMasterServerReliableResponse(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBaseMasterServerReliableResponse(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerReliableResponse in);
struct BaseMasterServerAcknowledgeMessage pkt_readBaseMasterServerAcknowledgeMessage(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBaseMasterServerAcknowledgeMessage(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerAcknowledgeMessage in);
struct ByteArrayNetSerializable pkt_readByteArrayNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeByteArrayNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct ByteArrayNetSerializable in);
struct String pkt_readString(struct PacketContext ctx, const uint8_t **pkt);
struct LongString pkt_readLongString(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeString(struct PacketContext ctx, uint8_t **pkt, struct String in);
void pkt_writeLongString(struct PacketContext ctx, uint8_t **pkt, struct LongString in);
struct AuthenticationToken pkt_readAuthenticationToken(struct PacketContext ctx, const uint8_t **pkt);
struct BaseMasterServerMultipartMessage pkt_readBaseMasterServerMultipartMessage(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBaseMasterServerMultipartMessage(struct PacketContext ctx, uint8_t **pkt, struct BaseMasterServerMultipartMessage in);
struct BitMask128 pkt_readBitMask128(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBitMask128(struct PacketContext ctx, uint8_t **pkt, struct BitMask128 in);
struct SongPackMask pkt_readSongPackMask(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSongPackMask(struct PacketContext ctx, uint8_t **pkt, struct SongPackMask in);
struct BeatmapLevelSelectionMask pkt_readBeatmapLevelSelectionMask(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBeatmapLevelSelectionMask(struct PacketContext ctx, uint8_t **pkt, struct BeatmapLevelSelectionMask in);
struct GameplayServerConfiguration pkt_readGameplayServerConfiguration(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGameplayServerConfiguration(struct PacketContext ctx, uint8_t **pkt, struct GameplayServerConfiguration in);
ServerCode StringToServerCode(const char *in, uint32_t len);
char *ServerCodeToString(char *out, ServerCode in);
void pkt_writeIPEndPoint(struct PacketContext ctx, uint8_t **pkt, struct IPEndPoint in);
struct BaseConnectToServerRequest pkt_readBaseConnectToServerRequest(struct PacketContext ctx, const uint8_t **pkt);
struct Channeled pkt_readChanneled(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeChanneled(struct PacketContext ctx, uint8_t **pkt, struct Channeled in);
struct Ack pkt_readAck(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeAck(struct PacketContext ctx, uint8_t **pkt, struct Ack in);
struct Ping pkt_readPing(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePing(struct PacketContext ctx, uint8_t **pkt, struct Ping in);
struct Pong pkt_readPong(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePong(struct PacketContext ctx, uint8_t **pkt, struct Pong in);
struct ConnectRequest pkt_readConnectRequest(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeConnectAccept(struct PacketContext ctx, uint8_t **pkt, struct ConnectAccept in);
struct Disconnect pkt_readDisconnect(struct PacketContext ctx, const uint8_t **pkt);
struct MtuCheck pkt_readMtuCheck(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeMtuOk(struct PacketContext ctx, uint8_t **pkt, struct MtuOk in);
struct ModConnectHeader pkt_readModConnectHeader(struct PacketContext ctx, const uint8_t **pkt);
struct BeatUpConnectHeader pkt_readBeatUpConnectHeader(struct PacketContext ctx, const uint8_t **pkt);
struct NetPacketHeader pkt_readNetPacketHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeNetPacketHeader(struct PacketContext ctx, uint8_t **pkt, struct NetPacketHeader in);
struct FragmentedHeader pkt_readFragmentedHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeFragmentedHeader(struct PacketContext ctx, uint8_t **pkt, struct FragmentedHeader in);
struct PlayerStateHash pkt_readPlayerStateHash(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePlayerStateHash(struct PacketContext ctx, uint8_t **pkt, struct PlayerStateHash in);
struct Color32 pkt_readColor32(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeColor32(struct PacketContext ctx, uint8_t **pkt, struct Color32 in);
struct MultiplayerAvatarData pkt_readMultiplayerAvatarData(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeMultiplayerAvatarData(struct PacketContext ctx, uint8_t **pkt, struct MultiplayerAvatarData in);
struct RoutingHeader pkt_readRoutingHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeRoutingHeader(struct PacketContext ctx, uint8_t **pkt, struct RoutingHeader in);
void pkt_writeSyncTime(struct PacketContext ctx, uint8_t **pkt, struct SyncTime in);
void pkt_writePlayerConnected(struct PacketContext ctx, uint8_t **pkt, struct PlayerConnected in);
struct PlayerIdentity pkt_readPlayerIdentity(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePlayerIdentity(struct PacketContext ctx, uint8_t **pkt, struct PlayerIdentity in);
void pkt_writePlayerLatencyUpdate(struct PacketContext ctx, uint8_t **pkt, struct PlayerLatencyUpdate in);
void pkt_writePlayerDisconnected(struct PacketContext ctx, uint8_t **pkt, struct PlayerDisconnected in);
void pkt_writePlayerSortOrderUpdate(struct PacketContext ctx, uint8_t **pkt, struct PlayerSortOrderUpdate in);
struct PlayerStateUpdate pkt_readPlayerStateUpdate(struct PacketContext ctx, const uint8_t **pkt);
struct PingMessage pkt_readPingMessage(struct PacketContext ctx, const uint8_t **pkt);
struct PongMessage pkt_readPongMessage(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePongMessage(struct PacketContext ctx, uint8_t **pkt, struct PongMessage in);
struct RemoteProcedureCall pkt_readRemoteProcedureCall(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeRemoteProcedureCall(struct PacketContext ctx, uint8_t **pkt, struct RemoteProcedureCall in);
struct RemoteProcedureCallFlags pkt_readRemoteProcedureCallFlags(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeRemoteProcedureCallFlags(struct PacketContext ctx, uint8_t **pkt, struct RemoteProcedureCallFlags in);
void pkt_writePlayersMissingEntitlementsNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayersMissingEntitlementsNetSerializable in);
struct BeatmapIdentifierNetSerializable pkt_readBeatmapIdentifierNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBeatmapIdentifierNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct BeatmapIdentifierNetSerializable in);
struct GameplayModifiers pkt_readGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct GameplayModifiers in);
void pkt_writePlayerLobbyPermissionConfigurationNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayerLobbyPermissionConfigurationNetSerializable in);
void pkt_writePlayersLobbyPermissionConfigurationNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayersLobbyPermissionConfigurationNetSerializable in);
struct SyncStateId pkt_readSyncStateId(struct PacketContext ctx, const uint8_t **pkt);
struct Vector3Serializable pkt_readVector3Serializable(struct PacketContext ctx, const uint8_t **pkt);
struct QuaternionSerializable pkt_readQuaternionSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct PoseSerializable pkt_readPoseSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct ColorNoAlphaSerializable pkt_readColorNoAlphaSerializable(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeColorNoAlphaSerializable(struct PacketContext ctx, uint8_t **pkt, struct ColorNoAlphaSerializable in);
struct ColorSchemeNetSerializable pkt_readColorSchemeNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeColorSchemeNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct ColorSchemeNetSerializable in);
struct PlayerSpecificSettingsNetSerializable pkt_readPlayerSpecificSettingsNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writePlayerSpecificSettingsNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayerSpecificSettingsNetSerializable in);
void pkt_writePlayerSpecificSettingsAtStartNetSerializable(struct PacketContext ctx, uint8_t **pkt, struct PlayerSpecificSettingsAtStartNetSerializable in);
struct NoteCutInfoNetSerializable pkt_readNoteCutInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct NoteMissInfoNetSerializable pkt_readNoteMissInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct LevelCompletionResults pkt_readLevelCompletionResults(struct PacketContext ctx, const uint8_t **pkt);
struct MultiplayerLevelCompletionResults pkt_readMultiplayerLevelCompletionResults(struct PacketContext ctx, const uint8_t **pkt);
struct NodePoseSyncState1 pkt_readNodePoseSyncState1(struct PacketContext ctx, const uint8_t **pkt);
struct StandardScoreSyncState pkt_readStandardScoreSyncState(struct PacketContext ctx, const uint8_t **pkt);
struct NoteSpawnInfoNetSerializable pkt_readNoteSpawnInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct ObstacleSpawnInfoNetSerializable pkt_readObstacleSpawnInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct SliderSpawnInfoNetSerializable pkt_readSliderSpawnInfoNetSerializable(struct PacketContext ctx, const uint8_t **pkt);
struct PreviewDifficultyBeatmapSet pkt_readPreviewDifficultyBeatmapSet(struct PacketContext ctx, const uint8_t **pkt);
struct NetworkPreviewBeatmapLevel pkt_readNetworkPreviewBeatmapLevel(struct PacketContext ctx, const uint8_t **pkt);
struct RecommendPreview pkt_readRecommendPreview(struct PacketContext ctx, const uint8_t **pkt);
struct SetCanShareBeatmap pkt_readSetCanShareBeatmap(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeDirectDownloadInfo(struct PacketContext ctx, uint8_t **pkt, struct DirectDownloadInfo in);
struct LevelFragmentRequest pkt_readLevelFragmentRequest(struct PacketContext ctx, const uint8_t **pkt);
struct LevelFragment pkt_readLevelFragment(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeLevelFragment(struct PacketContext ctx, uint8_t **pkt, struct LevelFragment in);
struct BeatUpMessageHeader pkt_readBeatUpMessageHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeBeatUpMessageHeader(struct PacketContext ctx, uint8_t **pkt, struct BeatUpMessageHeader in);
void pkt_writeSetPlayersMissingEntitlementsToLevel(struct PacketContext ctx, uint8_t **pkt, struct SetPlayersMissingEntitlementsToLevel in);
void pkt_writeGetIsEntitledToLevel(struct PacketContext ctx, uint8_t **pkt, struct GetIsEntitledToLevel in);
struct SetIsEntitledToLevel pkt_readSetIsEntitledToLevel(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSetSelectedBeatmap(struct PacketContext ctx, uint8_t **pkt, struct SetSelectedBeatmap in);
struct GetSelectedBeatmap pkt_readGetSelectedBeatmap(struct PacketContext ctx, const uint8_t **pkt);
struct RecommendBeatmap pkt_readRecommendBeatmap(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeRecommendBeatmap(struct PacketContext ctx, uint8_t **pkt, struct RecommendBeatmap in);
struct ClearRecommendedBeatmap pkt_readClearRecommendedBeatmap(struct PacketContext ctx, const uint8_t **pkt);
struct GetRecommendedBeatmap pkt_readGetRecommendedBeatmap(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGetRecommendedBeatmap(struct PacketContext ctx, uint8_t **pkt, struct GetRecommendedBeatmap in);
void pkt_writeSetSelectedGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct SetSelectedGameplayModifiers in);
struct GetSelectedGameplayModifiers pkt_readGetSelectedGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt);
struct RecommendGameplayModifiers pkt_readRecommendGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeRecommendGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct RecommendGameplayModifiers in);
struct ClearRecommendedGameplayModifiers pkt_readClearRecommendedGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt);
struct GetRecommendedGameplayModifiers pkt_readGetRecommendedGameplayModifiers(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGetRecommendedGameplayModifiers(struct PacketContext ctx, uint8_t **pkt, struct GetRecommendedGameplayModifiers in);
void pkt_writeStartLevel(struct PacketContext ctx, uint8_t **pkt, struct StartLevel in);
struct GetStartedLevel pkt_readGetStartedLevel(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeCancelLevelStart(struct PacketContext ctx, uint8_t **pkt, struct CancelLevelStart in);
struct GetMultiplayerGameState pkt_readGetMultiplayerGameState(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSetMultiplayerGameState(struct PacketContext ctx, uint8_t **pkt, struct SetMultiplayerGameState in);
struct GetIsReady pkt_readGetIsReady(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGetIsReady(struct PacketContext ctx, uint8_t **pkt, struct GetIsReady in);
struct SetIsReady pkt_readSetIsReady(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSetIsReady(struct PacketContext ctx, uint8_t **pkt, struct SetIsReady in);
struct GetIsInLobby pkt_readGetIsInLobby(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGetIsInLobby(struct PacketContext ctx, uint8_t **pkt, struct GetIsInLobby in);
struct SetIsInLobby pkt_readSetIsInLobby(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSetIsInLobby(struct PacketContext ctx, uint8_t **pkt, struct SetIsInLobby in);
struct GetCountdownEndTime pkt_readGetCountdownEndTime(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSetCountdownEndTime(struct PacketContext ctx, uint8_t **pkt, struct SetCountdownEndTime in);
void pkt_writeCancelCountdown(struct PacketContext ctx, uint8_t **pkt, struct CancelCountdown in);
void pkt_writeGetOwnedSongPacks(struct PacketContext ctx, uint8_t **pkt, struct GetOwnedSongPacks in);
struct SetOwnedSongPacks pkt_readSetOwnedSongPacks(struct PacketContext ctx, const uint8_t **pkt);
struct GetPermissionConfiguration pkt_readGetPermissionConfiguration(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSetPermissionConfiguration(struct PacketContext ctx, uint8_t **pkt, struct SetPermissionConfiguration in);
void pkt_writeSetIsStartButtonEnabled(struct PacketContext ctx, uint8_t **pkt, struct SetIsStartButtonEnabled in);
void pkt_writeSetGameplaySceneSyncFinish(struct PacketContext ctx, uint8_t **pkt, struct SetGameplaySceneSyncFinish in);
struct SetGameplaySceneReady pkt_readSetGameplaySceneReady(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGetGameplaySceneReady(struct PacketContext ctx, uint8_t **pkt, struct GetGameplaySceneReady in);
struct SetGameplaySongReady pkt_readSetGameplaySongReady(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGetGameplaySongReady(struct PacketContext ctx, uint8_t **pkt, struct GetGameplaySongReady in);
void pkt_writeSetSongStartTime(struct PacketContext ctx, uint8_t **pkt, struct SetSongStartTime in);
struct NoteCut pkt_readNoteCut(struct PacketContext ctx, const uint8_t **pkt);
struct NoteMissed pkt_readNoteMissed(struct PacketContext ctx, const uint8_t **pkt);
struct LevelFinished pkt_readLevelFinished(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeReturnToMenu(struct PacketContext ctx, uint8_t **pkt, struct ReturnToMenu in);
struct RequestReturnToMenu pkt_readRequestReturnToMenu(struct PacketContext ctx, const uint8_t **pkt);
struct NoteSpawned pkt_readNoteSpawned(struct PacketContext ctx, const uint8_t **pkt);
struct ObstacleSpawned pkt_readObstacleSpawned(struct PacketContext ctx, const uint8_t **pkt);
struct SliderSpawned pkt_readSliderSpawned(struct PacketContext ctx, const uint8_t **pkt);
struct NodePoseSyncState pkt_readNodePoseSyncState(struct PacketContext ctx, const uint8_t **pkt);
struct ScoreSyncState pkt_readScoreSyncState(struct PacketContext ctx, const uint8_t **pkt);
struct NodePoseSyncStateDelta pkt_readNodePoseSyncStateDelta(struct PacketContext ctx, const uint8_t **pkt);
struct ScoreSyncStateDelta pkt_readScoreSyncStateDelta(struct PacketContext ctx, const uint8_t **pkt);
struct MpCore pkt_readMpCore(struct PacketContext ctx, const uint8_t **pkt);
struct MultiplayerSessionMessageHeader pkt_readMultiplayerSessionMessageHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeMultiplayerSessionMessageHeader(struct PacketContext ctx, uint8_t **pkt, struct MultiplayerSessionMessageHeader in);
struct MenuRpcHeader pkt_readMenuRpcHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeMenuRpcHeader(struct PacketContext ctx, uint8_t **pkt, struct MenuRpcHeader in);
struct GameplayRpcHeader pkt_readGameplayRpcHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeGameplayRpcHeader(struct PacketContext ctx, uint8_t **pkt, struct GameplayRpcHeader in);
struct MpBeatmapPacket pkt_readMpBeatmapPacket(struct PacketContext ctx, const uint8_t **pkt);
struct MpPlayerData pkt_readMpPlayerData(struct PacketContext ctx, const uint8_t **pkt);
struct AuthenticateUserRequest pkt_readAuthenticateUserRequest(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeAuthenticateUserResponse(struct PacketContext ctx, uint8_t **pkt, struct AuthenticateUserResponse in);
void pkt_writeConnectToServerResponse(struct PacketContext ctx, uint8_t **pkt, struct ConnectToServerResponse in);
struct ConnectToServerRequest pkt_readConnectToServerRequest(struct PacketContext ctx, const uint8_t **pkt);
struct ClientHelloRequest pkt_readClientHelloRequest(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeHelloVerifyRequest(struct PacketContext ctx, uint8_t **pkt, struct HelloVerifyRequest in);
struct ClientHelloWithCookieRequest pkt_readClientHelloWithCookieRequest(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeServerHelloRequest(struct PacketContext ctx, uint8_t **pkt, struct ServerHelloRequest in);
void pkt_writeServerCertificateRequest(struct PacketContext ctx, uint8_t **pkt, struct ServerCertificateRequest in);
struct ClientKeyExchangeRequest pkt_readClientKeyExchangeRequest(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeChangeCipherSpecRequest(struct PacketContext ctx, uint8_t **pkt, struct ChangeCipherSpecRequest in);
struct MessageHeader pkt_readMessageHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeMessageHeader(struct PacketContext ctx, uint8_t **pkt, struct MessageHeader in);
struct SerializeHeader pkt_readSerializeHeader(struct PacketContext ctx, const uint8_t **pkt);
void pkt_writeSerializeHeader(struct PacketContext ctx, uint8_t **pkt, struct SerializeHeader in);
#endif // PACKETS_H
