#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t GameVersion;
enum {
	GameVersion_Unknown,
	GameVersion_1_19_0,
	GameVersion_1_19_1,
	GameVersion_1_20_0,
	GameVersion_1_21_0,
	GameVersion_1_22_0,
	GameVersion_1_22_1,
	GameVersion_1_23_0,
	GameVersion_1_24_0,
	GameVersion_1_24_1,
	GameVersion_1_25_0,
	GameVersion_1_25_1,
	GameVersion_1_26_0,
	GameVersion_1_27_0,
	GameVersion_1_28_0,
	GameVersion_1_29_0,
	GameVersion_1_29_1,
	GameVersion_1_29_4,
	GameVersion_1_30_0,
	GameVersion_1_30_2,
	GameVersion_1_31_0,
	GameVersion_1_31_1,
	GameVersion_1_32_0,
	GameVersion_1_33_0,
	GameVersion_1_34_0,
	GameVersion_1_34_2,
	GameVersion_1_34_4,
	GameVersion_1_34_5,
	GameVersion_1_34_6,
	GameVersion_1_35_0,
	GameVersion_1_36_0,
	GameVersion_1_36_1,
	GameVersion_1_36_2,
};
[[maybe_unused]] static const char *_reflect_GameVersion(GameVersion value) {
	switch(value) {
		case GameVersion_Unknown: return "Unknown";
		case GameVersion_1_19_0: return "1_19_0";
		case GameVersion_1_19_1: return "1_19_1";
		case GameVersion_1_20_0: return "1_20_0";
		case GameVersion_1_21_0: return "1_21_0";
		case GameVersion_1_22_0: return "1_22_0";
		case GameVersion_1_22_1: return "1_22_1";
		case GameVersion_1_23_0: return "1_23_0";
		case GameVersion_1_24_0: return "1_24_0";
		case GameVersion_1_24_1: return "1_24_1";
		case GameVersion_1_25_0: return "1_25_0";
		case GameVersion_1_25_1: return "1_25_1";
		case GameVersion_1_26_0: return "1_26_0";
		case GameVersion_1_27_0: return "1_27_0";
		case GameVersion_1_28_0: return "1_28_0";
		case GameVersion_1_29_0: return "1_29_0";
		case GameVersion_1_29_1: return "1_29_1";
		case GameVersion_1_29_4: return "1_29_4";
		case GameVersion_1_30_0: return "1_30_0";
		case GameVersion_1_30_2: return "1_30_2";
		case GameVersion_1_31_0: return "1_31_0";
		case GameVersion_1_31_1: return "1_31_1";
		case GameVersion_1_32_0: return "1_32_0";
		case GameVersion_1_33_0: return "1_33_0";
		case GameVersion_1_34_0: return "1_34_0";
		case GameVersion_1_34_2: return "1_34_2";
		case GameVersion_1_34_4: return "1_34_4";
		case GameVersion_1_34_5: return "1_34_5";
		case GameVersion_1_34_6: return "1_34_6";
		case GameVersion_1_35_0: return "1_35_0";
		case GameVersion_1_36_0: return "1_36_0";
		case GameVersion_1_36_1: return "1_36_1";
		case GameVersion_1_36_2: return "1_36_2";
		default: return "???";
	}
}
typedef uint32_t BeatmapDifficulty;
enum {
	BeatmapDifficulty_Easy,
	BeatmapDifficulty_Normal,
	BeatmapDifficulty_Hard,
	BeatmapDifficulty_Expert,
	BeatmapDifficulty_ExpertPlus,
};
[[maybe_unused]] static const char *_reflect_BeatmapDifficulty(BeatmapDifficulty value) {
	switch(value) {
		case BeatmapDifficulty_Easy: return "Easy";
		case BeatmapDifficulty_Normal: return "Normal";
		case BeatmapDifficulty_Hard: return "Hard";
		case BeatmapDifficulty_Expert: return "Expert";
		case BeatmapDifficulty_ExpertPlus: return "ExpertPlus";
		default: return "???";
	}
}
typedef uint16_t ShareableType;
enum {
	ShareableType_None,
	ShareableType_Generic,
	ShareableType_BeatmapAudio,
	ShareableType_BeatmapSet,
	ShareableType_Avatar,
};
[[maybe_unused]] static const char *_reflect_ShareableType(ShareableType value) {
	switch(value) {
		case ShareableType_None: return "None";
		case ShareableType_Generic: return "Generic";
		case ShareableType_BeatmapAudio: return "BeatmapAudio";
		case ShareableType_BeatmapSet: return "BeatmapSet";
		case ShareableType_Avatar: return "Avatar";
		default: return "???";
	}
}
typedef uint8_t LoadState;
enum {
	LoadState_None,
	LoadState_Failed,
	LoadState_Exporting,
	LoadState_Downloading,
	LoadState_Loading,
	LoadState_Done,
};
[[maybe_unused]] static const char *_reflect_LoadState(LoadState value) {
	switch(value) {
		case LoadState_None: return "None";
		case LoadState_Failed: return "Failed";
		case LoadState_Exporting: return "Exporting";
		case LoadState_Downloading: return "Downloading";
		case LoadState_Loading: return "Loading";
		case LoadState_Done: return "Done";
		default: return "???";
	}
}
typedef uint8_t BeatUpMessageType;
enum {
	BeatUpMessageType_ConnectInfo,
	BeatUpMessageType_RecommendPreview,
	BeatUpMessageType_ShareInfo,
	BeatUpMessageType_DataFragmentRequest,
	BeatUpMessageType_DataFragment,
	BeatUpMessageType_LoadProgress,
	BeatUpMessageType_ServerConnectInfo,
};
[[maybe_unused]] static const char *_reflect_BeatUpMessageType(BeatUpMessageType value) {
	switch(value) {
		case BeatUpMessageType_ConnectInfo: return "ConnectInfo";
		case BeatUpMessageType_RecommendPreview: return "RecommendPreview";
		case BeatUpMessageType_ShareInfo: return "ShareInfo";
		case BeatUpMessageType_DataFragmentRequest: return "DataFragmentRequest";
		case BeatUpMessageType_DataFragment: return "DataFragment";
		case BeatUpMessageType_LoadProgress: return "LoadProgress";
		case BeatUpMessageType_ServerConnectInfo: return "ServerConnectInfo";
		default: return "???";
	}
}
typedef uint32_t ServerCode;
[[maybe_unused]] static const char *_reflect_ServerCode(ServerCode value) {
	switch(value) {
		default: return "???";
	}
}
typedef int32_t EntitlementsStatus;
enum {
	EntitlementsStatus_Unknown,
	EntitlementsStatus_NotOwned,
	EntitlementsStatus_NotDownloaded,
	EntitlementsStatus_Ok,
};
[[maybe_unused]] static const char *_reflect_EntitlementsStatus(EntitlementsStatus value) {
	switch(value) {
		case EntitlementsStatus_Unknown: return "Unknown";
		case EntitlementsStatus_NotOwned: return "NotOwned";
		case EntitlementsStatus_NotDownloaded: return "NotDownloaded";
		case EntitlementsStatus_Ok: return "Ok";
		default: return "???";
	}
}
typedef uint8_t BeatmapDifficultyMask;
enum {
	BeatmapDifficultyMask_Easy = 1,
	BeatmapDifficultyMask_Normal = 2,
	BeatmapDifficultyMask_Hard = 4,
	BeatmapDifficultyMask_Expert = 8,
	BeatmapDifficultyMask_ExpertPlus = 16,
	BeatmapDifficultyMask_All = 31,
};
[[maybe_unused]] static const char *_reflect_BeatmapDifficultyMask(BeatmapDifficultyMask value) {
	switch(value) {
		case BeatmapDifficultyMask_Easy: return "Easy";
		case BeatmapDifficultyMask_Normal: return "Normal";
		case BeatmapDifficultyMask_Hard: return "Hard";
		case BeatmapDifficultyMask_Expert: return "Expert";
		case BeatmapDifficultyMask_ExpertPlus: return "ExpertPlus";
		case BeatmapDifficultyMask_All: return "All";
		default: return "???";
	}
}
typedef uint32_t GameplayModifierMask;
enum {
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
};
[[maybe_unused]] static const char *_reflect_GameplayModifierMask(GameplayModifierMask value) {
	switch(value) {
		case GameplayModifierMask_None: return "None";
		case GameplayModifierMask_BatteryEnergy: return "BatteryEnergy";
		case GameplayModifierMask_NoFail: return "NoFail";
		case GameplayModifierMask_InstaFail: return "InstaFail";
		case GameplayModifierMask_NoObstacles: return "NoObstacles";
		case GameplayModifierMask_NoBombs: return "NoBombs";
		case GameplayModifierMask_FastNotes: return "FastNotes";
		case GameplayModifierMask_StrictAngles: return "StrictAngles";
		case GameplayModifierMask_DisappearingArrows: return "DisappearingArrows";
		case GameplayModifierMask_FasterSong: return "FasterSong";
		case GameplayModifierMask_SlowerSong: return "SlowerSong";
		case GameplayModifierMask_NoArrows: return "NoArrows";
		case GameplayModifierMask_GhostNotes: return "GhostNotes";
		case GameplayModifierMask_SuperFastSong: return "SuperFastSong";
		case GameplayModifierMask_ProMode: return "ProMode";
		case GameplayModifierMask_ZenMode: return "ZenMode";
		case GameplayModifierMask_SmallCubes: return "SmallCubes";
		case GameplayModifierMask_All: return "All";
		default: return "???";
	}
}
typedef uint8_t EnergyType;
enum {
	EnergyType_Bar,
	EnergyType_Battery,
};
[[maybe_unused]] static const char *_reflect_EnergyType(EnergyType value) {
	switch(value) {
		case EnergyType_Bar: return "Bar";
		case EnergyType_Battery: return "Battery";
		default: return "???";
	}
}
typedef uint8_t EnabledObstacleType;
enum {
	EnabledObstacleType_All,
	EnabledObstacleType_FullHeightOnly,
	EnabledObstacleType_NoObstacles,
};
[[maybe_unused]] static const char *_reflect_EnabledObstacleType(EnabledObstacleType value) {
	switch(value) {
		case EnabledObstacleType_All: return "All";
		case EnabledObstacleType_FullHeightOnly: return "FullHeightOnly";
		case EnabledObstacleType_NoObstacles: return "NoObstacles";
		default: return "???";
	}
}
typedef uint8_t SongSpeed;
enum {
	SongSpeed_Normal,
	SongSpeed_Faster,
	SongSpeed_Slower,
	SongSpeed_SuperFast,
};
[[maybe_unused]] static const char *_reflect_SongSpeed(SongSpeed value) {
	switch(value) {
		case SongSpeed_Normal: return "Normal";
		case SongSpeed_Faster: return "Faster";
		case SongSpeed_Slower: return "Slower";
		case SongSpeed_SuperFast: return "SuperFast";
		default: return "???";
	}
}
typedef int32_t MultiplayerGameState;
enum {
	MultiplayerGameState_None,
	MultiplayerGameState_Lobby,
	MultiplayerGameState_Game,
};
[[maybe_unused]] static const char *_reflect_MultiplayerGameState(MultiplayerGameState value) {
	switch(value) {
		case MultiplayerGameState_None: return "None";
		case MultiplayerGameState_Lobby: return "Lobby";
		case MultiplayerGameState_Game: return "Game";
		default: return "???";
	}
}
typedef int32_t CannotStartGameReason;
enum {
	CannotStartGameReason_None = 1,
	CannotStartGameReason_AllPlayersSpectating,
	CannotStartGameReason_NoSongSelected,
	CannotStartGameReason_AllPlayersNotInLobby,
	CannotStartGameReason_DoNotOwnSong,
};
[[maybe_unused]] static const char *_reflect_CannotStartGameReason(CannotStartGameReason value) {
	switch(value) {
		case CannotStartGameReason_None: return "None";
		case CannotStartGameReason_AllPlayersSpectating: return "AllPlayersSpectating";
		case CannotStartGameReason_NoSongSelected: return "NoSongSelected";
		case CannotStartGameReason_AllPlayersNotInLobby: return "AllPlayersNotInLobby";
		case CannotStartGameReason_DoNotOwnSong: return "DoNotOwnSong";
		default: return "???";
	}
}
typedef uint8_t MenuRpcType;
enum {
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
	MenuRpcType_ClearSelectedBeatmap,
	MenuRpcType_ClearSelectedGameplayModifiers,
};
[[maybe_unused]] static const char *_reflect_MenuRpcType(MenuRpcType value) {
	switch(value) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: return "SetPlayersMissingEntitlementsToLevel";
		case MenuRpcType_GetIsEntitledToLevel: return "GetIsEntitledToLevel";
		case MenuRpcType_SetIsEntitledToLevel: return "SetIsEntitledToLevel";
		case MenuRpcType_InvalidateLevelEntitlementStatuses: return "InvalidateLevelEntitlementStatuses";
		case MenuRpcType_SelectLevelPack: return "SelectLevelPack";
		case MenuRpcType_SetSelectedBeatmap: return "SetSelectedBeatmap";
		case MenuRpcType_GetSelectedBeatmap: return "GetSelectedBeatmap";
		case MenuRpcType_RecommendBeatmap: return "RecommendBeatmap";
		case MenuRpcType_ClearRecommendedBeatmap: return "ClearRecommendedBeatmap";
		case MenuRpcType_GetRecommendedBeatmap: return "GetRecommendedBeatmap";
		case MenuRpcType_SetSelectedGameplayModifiers: return "SetSelectedGameplayModifiers";
		case MenuRpcType_GetSelectedGameplayModifiers: return "GetSelectedGameplayModifiers";
		case MenuRpcType_RecommendGameplayModifiers: return "RecommendGameplayModifiers";
		case MenuRpcType_ClearRecommendedGameplayModifiers: return "ClearRecommendedGameplayModifiers";
		case MenuRpcType_GetRecommendedGameplayModifiers: return "GetRecommendedGameplayModifiers";
		case MenuRpcType_LevelLoadError: return "LevelLoadError";
		case MenuRpcType_LevelLoadSuccess: return "LevelLoadSuccess";
		case MenuRpcType_StartLevel: return "StartLevel";
		case MenuRpcType_GetStartedLevel: return "GetStartedLevel";
		case MenuRpcType_CancelLevelStart: return "CancelLevelStart";
		case MenuRpcType_GetMultiplayerGameState: return "GetMultiplayerGameState";
		case MenuRpcType_SetMultiplayerGameState: return "SetMultiplayerGameState";
		case MenuRpcType_GetIsReady: return "GetIsReady";
		case MenuRpcType_SetIsReady: return "SetIsReady";
		case MenuRpcType_SetStartGameTime: return "SetStartGameTime";
		case MenuRpcType_CancelStartGameTime: return "CancelStartGameTime";
		case MenuRpcType_GetIsInLobby: return "GetIsInLobby";
		case MenuRpcType_SetIsInLobby: return "SetIsInLobby";
		case MenuRpcType_GetCountdownEndTime: return "GetCountdownEndTime";
		case MenuRpcType_SetCountdownEndTime: return "SetCountdownEndTime";
		case MenuRpcType_CancelCountdown: return "CancelCountdown";
		case MenuRpcType_GetOwnedSongPacks: return "GetOwnedSongPacks";
		case MenuRpcType_SetOwnedSongPacks: return "SetOwnedSongPacks";
		case MenuRpcType_RequestKickPlayer: return "RequestKickPlayer";
		case MenuRpcType_GetPermissionConfiguration: return "GetPermissionConfiguration";
		case MenuRpcType_SetPermissionConfiguration: return "SetPermissionConfiguration";
		case MenuRpcType_GetIsStartButtonEnabled: return "GetIsStartButtonEnabled";
		case MenuRpcType_SetIsStartButtonEnabled: return "SetIsStartButtonEnabled";
		case MenuRpcType_ClearSelectedBeatmap: return "ClearSelectedBeatmap";
		case MenuRpcType_ClearSelectedGameplayModifiers: return "ClearSelectedGameplayModifiers";
		default: return "???";
	}
}
typedef int32_t GameplayType;
enum {
	GameplayType_Normal,
	GameplayType_Bomb,
	GameplayType_BurstSliderHead,
	GameplayType_BurstSliderElement,
	GameplayType_BurstSliderElementFill,
};
[[maybe_unused]] static const char *_reflect_GameplayType(GameplayType value) {
	switch(value) {
		case GameplayType_Normal: return "Normal";
		case GameplayType_Bomb: return "Bomb";
		case GameplayType_BurstSliderHead: return "BurstSliderHead";
		case GameplayType_BurstSliderElement: return "BurstSliderElement";
		case GameplayType_BurstSliderElementFill: return "BurstSliderElementFill";
		default: return "???";
	}
}
typedef int32_t ColorType;
enum {
	ColorType_ColorA = 0,
	ColorType_ColorB = 1,
	ColorType_None = -1,
};
[[maybe_unused]] static const char *_reflect_ColorType(ColorType value) {
	switch(value) {
		case ColorType_ColorA: return "ColorA";
		case ColorType_ColorB: return "ColorB";
		case ColorType_None: return "None";
		default: return "???";
	}
}
typedef int32_t NoteLineLayer;
enum {
	NoteLineLayer_Base,
	NoteLineLayer_Upper,
	NoteLineLayer_Top,
};
[[maybe_unused]] static const char *_reflect_NoteLineLayer(NoteLineLayer value) {
	switch(value) {
		case NoteLineLayer_Base: return "Base";
		case NoteLineLayer_Upper: return "Upper";
		case NoteLineLayer_Top: return "Top";
		default: return "???";
	}
}
typedef int32_t MultiplayerLevelEndState;
enum {
	MultiplayerLevelEndState_Cleared,
	MultiplayerLevelEndState_Failed,
	MultiplayerLevelEndState_GivenUp,
	MultiplayerLevelEndState_WasInactive,
	MultiplayerLevelEndState_StartupFailed,
	MultiplayerLevelEndState_HostEndedLevel,
	MultiplayerLevelEndState_ConnectedAfterLevelEnded,
	MultiplayerLevelEndState_Quit,
};
[[maybe_unused]] static const char *_reflect_MultiplayerLevelEndState(MultiplayerLevelEndState value) {
	switch(value) {
		case MultiplayerLevelEndState_Cleared: return "Cleared";
		case MultiplayerLevelEndState_Failed: return "Failed";
		case MultiplayerLevelEndState_GivenUp: return "GivenUp";
		case MultiplayerLevelEndState_WasInactive: return "WasInactive";
		case MultiplayerLevelEndState_StartupFailed: return "StartupFailed";
		case MultiplayerLevelEndState_HostEndedLevel: return "HostEndedLevel";
		case MultiplayerLevelEndState_ConnectedAfterLevelEnded: return "ConnectedAfterLevelEnded";
		case MultiplayerLevelEndState_Quit: return "Quit";
		default: return "???";
	}
}
typedef int32_t MultiplayerPlayerLevelEndState;
enum {
	MultiplayerPlayerLevelEndState_SongFinished,
	MultiplayerPlayerLevelEndState_NotFinished,
	MultiplayerPlayerLevelEndState_NotStarted,
};
[[maybe_unused]] static const char *_reflect_MultiplayerPlayerLevelEndState(MultiplayerPlayerLevelEndState value) {
	switch(value) {
		case MultiplayerPlayerLevelEndState_SongFinished: return "SongFinished";
		case MultiplayerPlayerLevelEndState_NotFinished: return "NotFinished";
		case MultiplayerPlayerLevelEndState_NotStarted: return "NotStarted";
		default: return "???";
	}
}
typedef int32_t MultiplayerPlayerLevelEndReason;
enum {
	MultiplayerPlayerLevelEndReason_Cleared,
	MultiplayerPlayerLevelEndReason_Failed,
	MultiplayerPlayerLevelEndReason_GivenUp,
	MultiplayerPlayerLevelEndReason_Quit,
	MultiplayerPlayerLevelEndReason_HostEndedLevel,
	MultiplayerPlayerLevelEndReason_WasInactive,
	MultiplayerPlayerLevelEndReason_StartupFailed,
	MultiplayerPlayerLevelEndReason_ConnectedAfterLevelEnded,
};
[[maybe_unused]] static const char *_reflect_MultiplayerPlayerLevelEndReason(MultiplayerPlayerLevelEndReason value) {
	switch(value) {
		case MultiplayerPlayerLevelEndReason_Cleared: return "Cleared";
		case MultiplayerPlayerLevelEndReason_Failed: return "Failed";
		case MultiplayerPlayerLevelEndReason_GivenUp: return "GivenUp";
		case MultiplayerPlayerLevelEndReason_Quit: return "Quit";
		case MultiplayerPlayerLevelEndReason_HostEndedLevel: return "HostEndedLevel";
		case MultiplayerPlayerLevelEndReason_WasInactive: return "WasInactive";
		case MultiplayerPlayerLevelEndReason_StartupFailed: return "StartupFailed";
		case MultiplayerPlayerLevelEndReason_ConnectedAfterLevelEnded: return "ConnectedAfterLevelEnded";
		default: return "???";
	}
}
typedef int32_t Rank;
enum {
	Rank_E,
	Rank_D,
	Rank_C,
	Rank_B,
	Rank_A,
	Rank_S,
	Rank_SS,
	Rank_SSS,
};
[[maybe_unused]] static const char *_reflect_Rank(Rank value) {
	switch(value) {
		case Rank_E: return "E";
		case Rank_D: return "D";
		case Rank_C: return "C";
		case Rank_B: return "B";
		case Rank_A: return "A";
		case Rank_S: return "S";
		case Rank_SS: return "SS";
		case Rank_SSS: return "SSS";
		default: return "???";
	}
}
typedef int32_t LevelEndStateType;
enum {
	LevelEndStateType_None,
	LevelEndStateType_Cleared,
	LevelEndStateType_Failed,
};
[[maybe_unused]] static const char *_reflect_LevelEndStateType(LevelEndStateType value) {
	switch(value) {
		case LevelEndStateType_None: return "None";
		case LevelEndStateType_Cleared: return "Cleared";
		case LevelEndStateType_Failed: return "Failed";
		default: return "???";
	}
}
typedef int32_t LevelEndAction;
enum {
	LevelEndAction_None,
	LevelEndAction_Quit,
	LevelEndAction_Restart,
};
[[maybe_unused]] static const char *_reflect_LevelEndAction(LevelEndAction value) {
	switch(value) {
		case LevelEndAction_None: return "None";
		case LevelEndAction_Quit: return "Quit";
		case LevelEndAction_Restart: return "Restart";
		default: return "???";
	}
}
typedef int32_t ScoringType;
enum {
	ScoringType_Ignore = -1,
	ScoringType_NoScore,
	ScoringType_Normal,
	ScoringType_SliderHead,
	ScoringType_SliderTail,
	ScoringType_BurstSliderHead,
	ScoringType_BurstSliderElement,
};
[[maybe_unused]] static const char *_reflect_ScoringType(ScoringType value) {
	switch(value) {
		case ScoringType_Ignore: return "Ignore";
		case ScoringType_NoScore: return "NoScore";
		case ScoringType_Normal: return "Normal";
		case ScoringType_SliderHead: return "SliderHead";
		case ScoringType_SliderTail: return "SliderTail";
		case ScoringType_BurstSliderHead: return "BurstSliderHead";
		case ScoringType_BurstSliderElement: return "BurstSliderElement";
		default: return "???";
	}
}
typedef int32_t NoteCutDirection;
enum {
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
};
[[maybe_unused]] static const char *_reflect_NoteCutDirection(NoteCutDirection value) {
	switch(value) {
		case NoteCutDirection_Up: return "Up";
		case NoteCutDirection_Down: return "Down";
		case NoteCutDirection_Left: return "Left";
		case NoteCutDirection_Right: return "Right";
		case NoteCutDirection_UpLeft: return "UpLeft";
		case NoteCutDirection_UpRight: return "UpRight";
		case NoteCutDirection_DownLeft: return "DownLeft";
		case NoteCutDirection_DownRight: return "DownRight";
		case NoteCutDirection_Any: return "Any";
		case NoteCutDirection_None: return "None";
		default: return "???";
	}
}
typedef int32_t ObstacleType;
enum {
	ObstacleType_FullHeight,
	ObstacleType_Top,
};
[[maybe_unused]] static const char *_reflect_ObstacleType(ObstacleType value) {
	switch(value) {
		case ObstacleType_FullHeight: return "FullHeight";
		case ObstacleType_Top: return "Top";
		default: return "???";
	}
}
typedef int32_t SliderType;
enum {
	SliderType_Normal,
	SliderType_Burst,
};
[[maybe_unused]] static const char *_reflect_SliderType(SliderType value) {
	switch(value) {
		case SliderType_Normal: return "Normal";
		case SliderType_Burst: return "Burst";
		default: return "???";
	}
}
typedef int32_t SliderMidAnchorMode;
enum {
	SliderMidAnchorMode_Straight,
	SliderMidAnchorMode_Clockwise,
	SliderMidAnchorMode_CounterClockwise,
};
[[maybe_unused]] static const char *_reflect_SliderMidAnchorMode(SliderMidAnchorMode value) {
	switch(value) {
		case SliderMidAnchorMode_Straight: return "Straight";
		case SliderMidAnchorMode_Clockwise: return "Clockwise";
		case SliderMidAnchorMode_CounterClockwise: return "CounterClockwise";
		default: return "???";
	}
}
typedef uint8_t GameplayRpcType;
enum {
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
};
[[maybe_unused]] static const char *_reflect_GameplayRpcType(GameplayRpcType value) {
	switch(value) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: return "SetGameplaySceneSyncFinish";
		case GameplayRpcType_SetGameplaySceneReady: return "SetGameplaySceneReady";
		case GameplayRpcType_GetGameplaySceneReady: return "GetGameplaySceneReady";
		case GameplayRpcType_SetActivePlayerFailedToConnect: return "SetActivePlayerFailedToConnect";
		case GameplayRpcType_SetGameplaySongReady: return "SetGameplaySongReady";
		case GameplayRpcType_GetGameplaySongReady: return "GetGameplaySongReady";
		case GameplayRpcType_SetSongStartTime: return "SetSongStartTime";
		case GameplayRpcType_NoteCut: return "NoteCut";
		case GameplayRpcType_NoteMissed: return "NoteMissed";
		case GameplayRpcType_LevelFinished: return "LevelFinished";
		case GameplayRpcType_ReturnToMenu: return "ReturnToMenu";
		case GameplayRpcType_RequestReturnToMenu: return "RequestReturnToMenu";
		case GameplayRpcType_NoteSpawned: return "NoteSpawned";
		case GameplayRpcType_ObstacleSpawned: return "ObstacleSpawned";
		case GameplayRpcType_SliderSpawned: return "SliderSpawned";
		default: return "???";
	}
}
typedef uint32_t MpBeatmapDifficulty;
enum {
	MpBeatmapDifficulty_Easy,
	MpBeatmapDifficulty_Normal,
	MpBeatmapDifficulty_Hard,
	MpBeatmapDifficulty_Expert,
	MpBeatmapDifficulty_ExpertPlus,
};
[[maybe_unused]] static const char *_reflect_MpBeatmapDifficulty(MpBeatmapDifficulty value) {
	switch(value) {
		case MpBeatmapDifficulty_Easy: return "Easy";
		case MpBeatmapDifficulty_Normal: return "Normal";
		case MpBeatmapDifficulty_Hard: return "Hard";
		case MpBeatmapDifficulty_Expert: return "Expert";
		case MpBeatmapDifficulty_ExpertPlus: return "ExpertPlus";
		default: return "???";
	}
}
typedef int32_t MpPlatform;
enum {
	MpPlatform_Unknown,
	MpPlatform_Steam,
	MpPlatform_OculusPC,
	MpPlatform_OculusQuest,
	MpPlatform_PS4,
};
[[maybe_unused]] static const char *_reflect_MpPlatform(MpPlatform value) {
	switch(value) {
		case MpPlatform_Unknown: return "Unknown";
		case MpPlatform_Steam: return "Steam";
		case MpPlatform_OculusPC: return "OculusPC";
		case MpPlatform_OculusQuest: return "OculusQuest";
		case MpPlatform_PS4: return "PS4";
		default: return "???";
	}
}
typedef uint8_t MpCoreType;
enum {
	MpCoreType_MpcTextChatPacket,
	MpCoreType_MpBeatmapPacket,
	MpCoreType_CustomAvatarPacket,
	MpCoreType_MpcCapabilitiesPacket,
	MpCoreType_MpPlayerData,
	MpCoreType_MpexPlayerData,
};
[[maybe_unused]] static const char *_reflect_MpCoreType(MpCoreType value) {
	switch(value) {
		case MpCoreType_MpcTextChatPacket: return "MpcTextChatPacket";
		case MpCoreType_MpBeatmapPacket: return "MpBeatmapPacket";
		case MpCoreType_CustomAvatarPacket: return "CustomAvatarPacket";
		case MpCoreType_MpcCapabilitiesPacket: return "MpcCapabilitiesPacket";
		case MpCoreType_MpPlayerData: return "MpPlayerData";
		case MpCoreType_MpexPlayerData: return "MpexPlayerData";
		default: return "???";
	}
}
typedef int32_t DisconnectedReason;
enum {
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
};
[[maybe_unused]] static const char *_reflect_DisconnectedReason(DisconnectedReason value) {
	switch(value) {
		case DisconnectedReason_Unknown: return "Unknown";
		case DisconnectedReason_UserInitiated: return "UserInitiated";
		case DisconnectedReason_Timeout: return "Timeout";
		case DisconnectedReason_Kicked: return "Kicked";
		case DisconnectedReason_ServerAtCapacity: return "ServerAtCapacity";
		case DisconnectedReason_ServerConnectionClosed: return "ServerConnectionClosed";
		case DisconnectedReason_MasterServerUnreachable: return "MasterServerUnreachable";
		case DisconnectedReason_ClientConnectionClosed: return "ClientConnectionClosed";
		case DisconnectedReason_NetworkDisconnected: return "NetworkDisconnected";
		case DisconnectedReason_ServerTerminated: return "ServerTerminated";
		default: return "???";
	}
}
typedef uint8_t MultiplayerSessionMessageType;
enum {
	MultiplayerSessionMessageType_MenuRpc,
	MultiplayerSessionMessageType_GameplayRpc,
	MultiplayerSessionMessageType_NodePoseSyncState,
	MultiplayerSessionMessageType_ScoreSyncState,
	MultiplayerSessionMessageType_NodePoseSyncStateDelta,
	MultiplayerSessionMessageType_ScoreSyncStateDelta,
	MultiplayerSessionMessageType_MpCore = 100,
	MultiplayerSessionMessageType_BeatUpMessage = 101,
};
[[maybe_unused]] static const char *_reflect_MultiplayerSessionMessageType(MultiplayerSessionMessageType value) {
	switch(value) {
		case MultiplayerSessionMessageType_MenuRpc: return "MenuRpc";
		case MultiplayerSessionMessageType_GameplayRpc: return "GameplayRpc";
		case MultiplayerSessionMessageType_NodePoseSyncState: return "NodePoseSyncState";
		case MultiplayerSessionMessageType_ScoreSyncState: return "ScoreSyncState";
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: return "NodePoseSyncStateDelta";
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: return "ScoreSyncStateDelta";
		case MultiplayerSessionMessageType_MpCore: return "MpCore";
		case MultiplayerSessionMessageType_BeatUpMessage: return "BeatUpMessage";
		default: return "???";
	}
}
typedef uint8_t InternalMessageType;
enum {
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
};
[[maybe_unused]] static const char *_reflect_InternalMessageType(InternalMessageType value) {
	switch(value) {
		case InternalMessageType_SyncTime: return "SyncTime";
		case InternalMessageType_PlayerConnected: return "PlayerConnected";
		case InternalMessageType_PlayerIdentity: return "PlayerIdentity";
		case InternalMessageType_PlayerLatencyUpdate: return "PlayerLatencyUpdate";
		case InternalMessageType_PlayerDisconnected: return "PlayerDisconnected";
		case InternalMessageType_PlayerSortOrderUpdate: return "PlayerSortOrderUpdate";
		case InternalMessageType_Party: return "Party";
		case InternalMessageType_MultiplayerSession: return "MultiplayerSession";
		case InternalMessageType_KickPlayer: return "KickPlayer";
		case InternalMessageType_PlayerStateUpdate: return "PlayerStateUpdate";
		case InternalMessageType_PlayerAvatarUpdate: return "PlayerAvatarUpdate";
		case InternalMessageType_PingMessage: return "PingMessage";
		case InternalMessageType_PongMessage: return "PongMessage";
		default: return "???";
	}
}
typedef uint8_t Platform;
enum {
	Platform_Test,
	Platform_OculusRift,
	Platform_OculusQuest,
	Platform_Steam,
	Platform_PS4,
	Platform_PS4Dev,
	Platform_PS4Cert,
	Platform_Oculus = 1,
};
[[maybe_unused]] static const char *_reflect_Platform(Platform value) {
	switch(value) {
		case Platform_Test: return "Test";
		case Platform_OculusRift: return "OculusRift";
		case Platform_OculusQuest: return "OculusQuest";
		case Platform_Steam: return "Steam";
		case Platform_PS4: return "PS4";
		case Platform_PS4Dev: return "PS4Dev";
		case Platform_PS4Cert: return "PS4Cert";
		default: return "???";
	}
}
typedef uint8_t AuthenticateUserResponse_Result;
enum {
	AuthenticateUserResponse_Result_Success,
	AuthenticateUserResponse_Result_Failed,
	AuthenticateUserResponse_Result_UnknownError,
};
[[maybe_unused]] static const char *_reflect_AuthenticateUserResponse_Result(AuthenticateUserResponse_Result value) {
	switch(value) {
		case AuthenticateUserResponse_Result_Success: return "Success";
		case AuthenticateUserResponse_Result_Failed: return "Failed";
		case AuthenticateUserResponse_Result_UnknownError: return "UnknownError";
		default: return "???";
	}
}
typedef uint8_t ConnectToServerResponse_Result;
enum {
	ConnectToServerResponse_Result_Success,
	ConnectToServerResponse_Result_InvalidSecret,
	ConnectToServerResponse_Result_InvalidCode,
	ConnectToServerResponse_Result_InvalidPassword,
	ConnectToServerResponse_Result_ServerAtCapacity,
	ConnectToServerResponse_Result_NoAvailableDedicatedServers,
	ConnectToServerResponse_Result_VersionMismatch,
	ConnectToServerResponse_Result_ConfigMismatch,
	ConnectToServerResponse_Result_UnknownError,
};
[[maybe_unused]] static const char *_reflect_ConnectToServerResponse_Result(ConnectToServerResponse_Result value) {
	switch(value) {
		case ConnectToServerResponse_Result_Success: return "Success";
		case ConnectToServerResponse_Result_InvalidSecret: return "InvalidSecret";
		case ConnectToServerResponse_Result_InvalidCode: return "InvalidCode";
		case ConnectToServerResponse_Result_InvalidPassword: return "InvalidPassword";
		case ConnectToServerResponse_Result_ServerAtCapacity: return "ServerAtCapacity";
		case ConnectToServerResponse_Result_NoAvailableDedicatedServers: return "NoAvailableDedicatedServers";
		case ConnectToServerResponse_Result_VersionMismatch: return "VersionMismatch";
		case ConnectToServerResponse_Result_ConfigMismatch: return "ConfigMismatch";
		case ConnectToServerResponse_Result_UnknownError: return "UnknownError";
		default: return "???";
	}
}
typedef int32_t DiscoveryPolicy;
enum {
	DiscoveryPolicy_Hidden,
	DiscoveryPolicy_WithCode,
	DiscoveryPolicy_Public,
};
[[maybe_unused]] static const char *_reflect_DiscoveryPolicy(DiscoveryPolicy value) {
	switch(value) {
		case DiscoveryPolicy_Hidden: return "Hidden";
		case DiscoveryPolicy_WithCode: return "WithCode";
		case DiscoveryPolicy_Public: return "Public";
		default: return "???";
	}
}
typedef int32_t InvitePolicy;
enum {
	InvitePolicy_OnlyConnectionOwnerCanInvite,
	InvitePolicy_AnyoneCanInvite,
	InvitePolicy_NobodyCanInvite,
};
[[maybe_unused]] static const char *_reflect_InvitePolicy(InvitePolicy value) {
	switch(value) {
		case InvitePolicy_OnlyConnectionOwnerCanInvite: return "OnlyConnectionOwnerCanInvite";
		case InvitePolicy_AnyoneCanInvite: return "AnyoneCanInvite";
		case InvitePolicy_NobodyCanInvite: return "NobodyCanInvite";
		default: return "???";
	}
}
typedef int32_t GameplayServerMode;
enum {
	GameplayServerMode_Countdown,
	GameplayServerMode_Managed,
	GameplayServerMode_QuickStartOneSong,
};
[[maybe_unused]] static const char *_reflect_GameplayServerMode(GameplayServerMode value) {
	switch(value) {
		case GameplayServerMode_Countdown: return "Countdown";
		case GameplayServerMode_Managed: return "Managed";
		case GameplayServerMode_QuickStartOneSong: return "QuickStartOneSong";
		default: return "???";
	}
}
typedef int32_t SongSelectionMode;
enum {
	SongSelectionMode_Vote,
	SongSelectionMode_Random,
	SongSelectionMode_OwnerPicks,
	SongSelectionMode_RandomPlayerPicks,
};
[[maybe_unused]] static const char *_reflect_SongSelectionMode(SongSelectionMode value) {
	switch(value) {
		case SongSelectionMode_Vote: return "Vote";
		case SongSelectionMode_Random: return "Random";
		case SongSelectionMode_OwnerPicks: return "OwnerPicks";
		case SongSelectionMode_RandomPlayerPicks: return "RandomPlayerPicks";
		default: return "???";
	}
}
typedef int32_t GameplayServerControlSettings;
enum {
	GameplayServerControlSettings_None = 0,
	GameplayServerControlSettings_AllowModifierSelection = 1,
	GameplayServerControlSettings_AllowSpectate = 2,
	GameplayServerControlSettings_All = 3,
};
[[maybe_unused]] static const char *_reflect_GameplayServerControlSettings(GameplayServerControlSettings value) {
	switch(value) {
		case GameplayServerControlSettings_None: return "None";
		case GameplayServerControlSettings_AllowModifierSelection: return "AllowModifierSelection";
		case GameplayServerControlSettings_AllowSpectate: return "AllowSpectate";
		case GameplayServerControlSettings_All: return "All";
		default: return "???";
	}
}
typedef uint8_t GetPublicServersResponse_Result;
enum {
	GetPublicServersResponse_Result_Success,
	GetPublicServersResponse_Result_UnknownError,
};
[[maybe_unused]] static const char *_reflect_GetPublicServersResponse_Result(GetPublicServersResponse_Result value) {
	switch(value) {
		case GetPublicServersResponse_Result_Success: return "Success";
		case GetPublicServersResponse_Result_UnknownError: return "UnknownError";
		default: return "???";
	}
}
typedef uint8_t UserMessageType;
enum {
	UserMessageType_AuthenticateUserRequest,
	UserMessageType_AuthenticateUserResponse,
	UserMessageType_ConnectToServerResponse = 8,
	UserMessageType_ConnectToServerRequest,
	UserMessageType_MessageReceivedAcknowledge = 13,
	UserMessageType_MultipartMessage,
	UserMessageType_SessionKeepaliveMessage,
	UserMessageType_GetPublicServersRequest,
	UserMessageType_GetPublicServersResponse,
};
[[maybe_unused]] static const char *_reflect_UserMessageType(UserMessageType value) {
	switch(value) {
		case UserMessageType_AuthenticateUserRequest: return "AuthenticateUserRequest";
		case UserMessageType_AuthenticateUserResponse: return "AuthenticateUserResponse";
		case UserMessageType_ConnectToServerResponse: return "ConnectToServerResponse";
		case UserMessageType_ConnectToServerRequest: return "ConnectToServerRequest";
		case UserMessageType_MessageReceivedAcknowledge: return "MessageReceivedAcknowledge";
		case UserMessageType_MultipartMessage: return "MultipartMessage";
		case UserMessageType_SessionKeepaliveMessage: return "SessionKeepaliveMessage";
		case UserMessageType_GetPublicServersRequest: return "GetPublicServersRequest";
		case UserMessageType_GetPublicServersResponse: return "GetPublicServersResponse";
		default: return "???";
	}
}
typedef uint8_t GameLiftMessageType;
enum {
	GameLiftMessageType_AuthenticateGameLiftUserRequest,
	GameLiftMessageType_AuthenticateUserResponse,
	GameLiftMessageType_MessageReceivedAcknowledge,
	GameLiftMessageType_MultipartMessage,
};
[[maybe_unused]] static const char *_reflect_GameLiftMessageType(GameLiftMessageType value) {
	switch(value) {
		case GameLiftMessageType_AuthenticateGameLiftUserRequest: return "AuthenticateGameLiftUserRequest";
		case GameLiftMessageType_AuthenticateUserResponse: return "AuthenticateUserResponse";
		case GameLiftMessageType_MessageReceivedAcknowledge: return "MessageReceivedAcknowledge";
		case GameLiftMessageType_MultipartMessage: return "MultipartMessage";
		default: return "???";
	}
}
typedef uint8_t HandshakeMessageType;
enum {
	HandshakeMessageType_ClientHelloRequest,
	HandshakeMessageType_HelloVerifyRequest,
	HandshakeMessageType_ClientHelloWithCookieRequest,
	HandshakeMessageType_ServerHelloRequest,
	HandshakeMessageType_ServerCertificateRequest,
	HandshakeMessageType_ClientKeyExchangeRequest = 6,
	HandshakeMessageType_ChangeCipherSpecRequest,
	HandshakeMessageType_MessageReceivedAcknowledge,
	HandshakeMessageType_MultipartMessage,
};
[[maybe_unused]] static const char *_reflect_HandshakeMessageType(HandshakeMessageType value) {
	switch(value) {
		case HandshakeMessageType_ClientHelloRequest: return "ClientHelloRequest";
		case HandshakeMessageType_HelloVerifyRequest: return "HelloVerifyRequest";
		case HandshakeMessageType_ClientHelloWithCookieRequest: return "ClientHelloWithCookieRequest";
		case HandshakeMessageType_ServerHelloRequest: return "ServerHelloRequest";
		case HandshakeMessageType_ServerCertificateRequest: return "ServerCertificateRequest";
		case HandshakeMessageType_ClientKeyExchangeRequest: return "ClientKeyExchangeRequest";
		case HandshakeMessageType_ChangeCipherSpecRequest: return "ChangeCipherSpecRequest";
		case HandshakeMessageType_MessageReceivedAcknowledge: return "MessageReceivedAcknowledge";
		case HandshakeMessageType_MultipartMessage: return "MultipartMessage";
		default: return "???";
	}
}
typedef uint32_t MessageType;
enum {
	MessageType_UserMessage = 1,
	MessageType_GameLiftMessage = 3,
	#define MessageType_HandshakeMessage 3192347326u
};
[[maybe_unused]] static const char *_reflect_MessageType(MessageType value) {
	switch(value) {
		case MessageType_UserMessage: return "UserMessage";
		case MessageType_GameLiftMessage: return "GameLiftMessage";
		case MessageType_HandshakeMessage: return "HandshakeMessage";
		default: return "???";
	}
}
typedef uint8_t DeliveryMethod;
enum {
	DeliveryMethod_ReliableUnordered,
	DeliveryMethod_Sequenced,
	DeliveryMethod_ReliableOrdered,
	DeliveryMethod_ReliableSequenced,
};
[[maybe_unused]] static const char *_reflect_DeliveryMethod(DeliveryMethod value) {
	switch(value) {
		case DeliveryMethod_ReliableUnordered: return "ReliableUnordered";
		case DeliveryMethod_Sequenced: return "Sequenced";
		case DeliveryMethod_ReliableOrdered: return "ReliableOrdered";
		case DeliveryMethod_ReliableSequenced: return "ReliableSequenced";
		default: return "???";
	}
}
typedef uint8_t PacketProperty;
enum {
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
};
[[maybe_unused]] static const char *_reflect_PacketProperty(PacketProperty value) {
	switch(value) {
		case PacketProperty_Unreliable: return "Unreliable";
		case PacketProperty_Channeled: return "Channeled";
		case PacketProperty_Ack: return "Ack";
		case PacketProperty_Ping: return "Ping";
		case PacketProperty_Pong: return "Pong";
		case PacketProperty_ConnectRequest: return "ConnectRequest";
		case PacketProperty_ConnectAccept: return "ConnectAccept";
		case PacketProperty_Disconnect: return "Disconnect";
		case PacketProperty_UnconnectedMessage: return "UnconnectedMessage";
		case PacketProperty_MtuCheck: return "MtuCheck";
		case PacketProperty_MtuOk: return "MtuOk";
		case PacketProperty_Broadcast: return "Broadcast";
		case PacketProperty_Merged: return "Merged";
		case PacketProperty_ShutdownOk: return "ShutdownOk";
		case PacketProperty_PeerNotFound: return "PeerNotFound";
		case PacketProperty_InvalidProtocol: return "InvalidProtocol";
		case PacketProperty_NatMessage: return "NatMessage";
		case PacketProperty_Empty: return "Empty";
		default: return "???";
	}
}
typedef uint8_t MultiplayerPlacementErrorCode;
enum {
	MultiplayerPlacementErrorCode_Success,
	MultiplayerPlacementErrorCode_Unknown,
	MultiplayerPlacementErrorCode_ConnectionCanceled,
	MultiplayerPlacementErrorCode_ServerDoesNotExist,
	MultiplayerPlacementErrorCode_ServerAtCapacity,
	MultiplayerPlacementErrorCode_AuthenticationFailed,
	MultiplayerPlacementErrorCode_RequestTimeout,
	MultiplayerPlacementErrorCode_MatchmakingTimeout,
};
[[maybe_unused]] static const char *_reflect_MultiplayerPlacementErrorCode(MultiplayerPlacementErrorCode value) {
	switch(value) {
		case MultiplayerPlacementErrorCode_Success: return "Success";
		case MultiplayerPlacementErrorCode_Unknown: return "Unknown";
		case MultiplayerPlacementErrorCode_ConnectionCanceled: return "ConnectionCanceled";
		case MultiplayerPlacementErrorCode_ServerDoesNotExist: return "ServerDoesNotExist";
		case MultiplayerPlacementErrorCode_ServerAtCapacity: return "ServerAtCapacity";
		case MultiplayerPlacementErrorCode_AuthenticationFailed: return "AuthenticationFailed";
		case MultiplayerPlacementErrorCode_RequestTimeout: return "RequestTimeout";
		case MultiplayerPlacementErrorCode_MatchmakingTimeout: return "MatchmakingTimeout";
		default: return "???";
	}
}
typedef uint8_t WireMessageType;
enum {
	WireMessageType_WireInstanceAttach,
	WireMessageType_WireStatusAttach,
	WireMessageType_WireRoomStatusNotify,
	WireMessageType_WireRoomCloseNotify,
	WireMessageType_WireRoomSpawn,
	WireMessageType_WireRoomSpawnResp,
	WireMessageType_WireRoomJoin,
	WireMessageType_WireRoomJoinResp,
	WireMessageType_WireRoomQuery,
	WireMessageType_WireRoomQueryResp,
	WireMessageType_WireGraphConnect,
	WireMessageType_WireGraphConnectResp,
};
[[maybe_unused]] static const char *_reflect_WireMessageType(WireMessageType value) {
	switch(value) {
		case WireMessageType_WireInstanceAttach: return "WireInstanceAttach";
		case WireMessageType_WireStatusAttach: return "WireStatusAttach";
		case WireMessageType_WireRoomStatusNotify: return "WireRoomStatusNotify";
		case WireMessageType_WireRoomCloseNotify: return "WireRoomCloseNotify";
		case WireMessageType_WireRoomSpawn: return "WireRoomSpawn";
		case WireMessageType_WireRoomSpawnResp: return "WireRoomSpawnResp";
		case WireMessageType_WireRoomJoin: return "WireRoomJoin";
		case WireMessageType_WireRoomJoinResp: return "WireRoomJoinResp";
		case WireMessageType_WireRoomQuery: return "WireRoomQuery";
		case WireMessageType_WireRoomQueryResp: return "WireRoomQueryResp";
		case WireMessageType_WireGraphConnect: return "WireGraphConnect";
		case WireMessageType_WireGraphConnectResp: return "WireGraphConnectResp";
		default: return "???";
	}
}
struct PacketContext {
	uint8_t netVersion;
	uint8_t protocolVersion;
	uint8_t beatUpVersion;
	GameVersion gameVersion;
	bool direct;
	uint16_t windowSize;
};
struct ByteArrayNetSerializable {
	uint32_t length;
	uint8_t data[8192];
};
struct ConnectInfo {
	uint32_t protocolId;
	uint16_t blockSize;
};
struct PreviewDifficultyBeatmapSet {
	struct String characteristic;
	uint8_t difficulties_len;
	BeatmapDifficulty difficulties[5];
};
struct PreviewBeatmapLevel {
	struct LongString levelID;
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
	struct String environmentInfo;
	struct String allDirectionsEnvironmentInfo;
	uint8_t beatmapSets_len;
	struct PreviewDifficultyBeatmapSet beatmapSets[8];
	struct ByteArrayNetSerializable cover;
};
struct CustomLabelSet {
	uint8_t difficulties_len;
	struct LongString difficulties[5];
};
struct RecommendPreview {
	struct PreviewBeatmapLevel base;
	struct CustomLabelSet labelSets[8];
	uint32_t requirements_len;
	struct String requirements[16];
	uint32_t suggestions_len;
	struct String suggestions[16];
};
struct ShareMeta {
	uint64_t byteLength;
	uint8_t hash[32];
};
struct ShareId {
	ShareableType usage;
	struct String mimeType;
	struct LongString name;
};
struct ShareInfo {
	uint32_t offset;
	uint16_t blockSize;
	struct ShareMeta meta;
	struct ShareId id;
};
struct DataFragmentRequest {
	uint32_t offset;
	uint8_t count;
};
struct DataFragment {
	uint32_t offset;
};
struct LoadProgress {
	uint32_t sequence;
	LoadState state;
	uint16_t progress;
};
struct ServerConnectInfo {
	struct ConnectInfo base;
	uint32_t windowSize;
	uint8_t countdownDuration;
	bool directDownloads;
	bool skipResults;
	bool perPlayerDifficulty;
	bool perPlayerModifiers;
};
struct BeatUpMessage {
	BeatUpMessageType type;
	union {
		struct ConnectInfo connectInfo;
		struct RecommendPreview recommendPreview;
		struct ShareInfo shareInfo;
		struct DataFragmentRequest dataFragmentRequest;
		struct DataFragment dataFragment;
		struct LoadProgress loadProgress;
		struct ServerConnectInfo serverConnectInfo;
	};
};
struct ModConnectHeader {
	uint32_t length;
	struct String name;
};
struct BitMask128 {
	uint64_t d0;
	uint64_t d1;
};
struct SongPackMask {
	struct BitMask128 bloomFilter;
	struct BitMask128 bloomFilterHi;
};
struct BeatmapLevelSelectionMask {
	BeatmapDifficultyMask difficulties;
	GameplayModifierMask modifiers;
	struct SongPackMask songPacks;
};
struct UTimestamp {
	float legacy;
	uint64_t value;
};
struct STimestamp {
	float legacy;
	int64_t value;
};
struct RemoteProcedureCall {
	struct UTimestamp syncTime;
};
struct SetPlayersMissingEntitlementsToLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	int32_t count;
	struct String players[254];
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
struct InvalidateLevelEntitlementStatuses {
	struct RemoteProcedureCall base;
};
struct SelectLevelPack {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct String levelPackId;
};
struct BeatmapIdentifierNetSerializable {
	struct LongString levelID;
	struct String characteristic;
	BeatmapDifficulty difficulty;
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
struct GameplayModifiers {
	uint32_t raw;
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
struct LevelLoadError {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct LongString levelId;
};
struct LevelLoadSuccess {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct LongString levelId;
};
struct StartLevel {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct BeatmapIdentifierNetSerializable beatmapId;
	struct GameplayModifiers gameplayModifiers;
	struct STimestamp startTime;
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
	bool isReady;
};
struct SetStartGameTime {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct STimestamp newTime;
};
struct CancelStartGameTime {
	struct RemoteProcedureCall base;
};
struct GetIsInLobby {
	struct RemoteProcedureCall base;
};
struct SetIsInLobby {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	bool isBack;
};
struct GetCountdownEndTime {
	struct RemoteProcedureCall base;
};
struct SetCountdownEndTime {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct STimestamp newTime;
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
struct RequestKickPlayer {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct String kickedPlayerId;
};
struct GetPermissionConfiguration {
	struct RemoteProcedureCall base;
};
struct PlayerLobbyPermissionConfiguration {
	struct String userId;
	bool serverOwner;
	bool recommendBeatmaps;
	bool recommendModifiers;
	bool kickVote;
	bool invite;
};
struct PlayersLobbyPermissionConfiguration {
	int32_t count;
	struct PlayerLobbyPermissionConfiguration playersPermission[254];
};
struct SetPermissionConfiguration {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayersLobbyPermissionConfiguration playersPermissionConfiguration;
};
struct GetIsStartButtonEnabled {
	struct RemoteProcedureCall base;
};
struct SetIsStartButtonEnabled {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	CannotStartGameReason reason;
};
struct ClearSelectedBeatmap {
	struct RemoteProcedureCall base;
};
struct ClearSelectedGameplayModifiers {
	struct RemoteProcedureCall base;
};
struct MenuRpc {
	MenuRpcType type;
	union {
		struct SetPlayersMissingEntitlementsToLevel setPlayersMissingEntitlementsToLevel;
		struct GetIsEntitledToLevel getIsEntitledToLevel;
		struct SetIsEntitledToLevel setIsEntitledToLevel;
		struct InvalidateLevelEntitlementStatuses invalidateLevelEntitlementStatuses;
		struct SelectLevelPack selectLevelPack;
		struct SetSelectedBeatmap setSelectedBeatmap;
		struct GetSelectedBeatmap getSelectedBeatmap;
		struct RecommendBeatmap recommendBeatmap;
		struct ClearRecommendedBeatmap clearRecommendedBeatmap;
		struct GetRecommendedBeatmap getRecommendedBeatmap;
		struct SetSelectedGameplayModifiers setSelectedGameplayModifiers;
		struct GetSelectedGameplayModifiers getSelectedGameplayModifiers;
		struct RecommendGameplayModifiers recommendGameplayModifiers;
		struct ClearRecommendedGameplayModifiers clearRecommendedGameplayModifiers;
		struct GetRecommendedGameplayModifiers getRecommendedGameplayModifiers;
		struct LevelLoadError levelLoadError;
		struct LevelLoadSuccess levelLoadSuccess;
		struct StartLevel startLevel;
		struct GetStartedLevel getStartedLevel;
		struct CancelLevelStart cancelLevelStart;
		struct GetMultiplayerGameState getMultiplayerGameState;
		struct SetMultiplayerGameState setMultiplayerGameState;
		struct GetIsReady getIsReady;
		struct SetIsReady setIsReady;
		struct SetStartGameTime setStartGameTime;
		struct CancelStartGameTime cancelStartGameTime;
		struct GetIsInLobby getIsInLobby;
		struct SetIsInLobby setIsInLobby;
		struct GetCountdownEndTime getCountdownEndTime;
		struct SetCountdownEndTime setCountdownEndTime;
		struct CancelCountdown cancelCountdown;
		struct GetOwnedSongPacks getOwnedSongPacks;
		struct SetOwnedSongPacks setOwnedSongPacks;
		struct RequestKickPlayer requestKickPlayer;
		struct GetPermissionConfiguration getPermissionConfiguration;
		struct SetPermissionConfiguration setPermissionConfiguration;
		struct GetIsStartButtonEnabled getIsStartButtonEnabled;
		struct SetIsStartButtonEnabled setIsStartButtonEnabled;
		struct ClearSelectedBeatmap clearSelectedBeatmap;
		struct ClearSelectedGameplayModifiers clearSelectedGameplayModifiers;
	};
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
struct PlayerSpecificSettings {
	struct String userId;
	struct String userName;
	bool leftHanded;
	bool automaticPlayerHeight;
	float playerHeight;
	float headPosToPlayerHeightOffset;
	struct ColorSchemeNetSerializable colorScheme;
};
struct PlayerSpecificSettingsAtStart {
	int32_t count;
	struct PlayerSpecificSettings players[254];
};
struct SetGameplaySceneSyncFinish {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayerSpecificSettingsAtStart settings;
	struct String sessionGameId;
};
struct SetGameplaySceneReady {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct PlayerSpecificSettings playerSpecificSettings;
};
struct GetGameplaySceneReady {
	struct RemoteProcedureCall base;
};
struct SetActivePlayerFailedToConnect {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	struct String failedUserId;
	struct PlayerSpecificSettingsAtStart settings;
	struct String sessionGameId;
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
	struct STimestamp startTime;
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
struct NoteCutInfoNetSerializable {
	bool cutWasOk;
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
struct NoteCut {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteCutInfoNetSerializable noteCutInfo;
};
struct NoteMissInfoNetSerializable {
	ColorType colorType;
	NoteLineLayer lineLayer;
	int32_t noteLineIndex;
	float noteTime;
};
struct NoteMissed {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteMissInfoNetSerializable noteMissInfo;
};
struct LevelCompletionResults {
	struct GameplayModifiers gameplayModifiers;
	int32_t modifiedScore;
	int32_t multipliedScore;
	Rank rank;
	bool fullCombo;
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
struct NoteSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct NoteSpawnInfoNetSerializable noteSpawnInfo;
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
struct ObstacleSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct ObstacleSpawnInfoNetSerializable obstacleSpawnInfo;
};
struct SliderSpawnInfoNetSerializable {
	ColorType colorType;
	SliderType sliderType;
	bool hasHeadNote;
	float headTime;
	int32_t headLineIndex;
	NoteLineLayer headLineLayer;
	NoteLineLayer headBeforeJumpLineLayer;
	float headControlPointLengthMultiplier;
	NoteCutDirection headCutDirection;
	float headCutDirectionAngleOffset;
	bool hasTailNote;
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
struct SliderSpawned {
	struct RemoteProcedureCall base;
	struct RemoteProcedureCallFlags flags;
	float songTime;
	struct SliderSpawnInfoNetSerializable sliderSpawnInfo;
};
struct GameplayRpc {
	GameplayRpcType type;
	union {
		struct SetGameplaySceneSyncFinish setGameplaySceneSyncFinish;
		struct SetGameplaySceneReady setGameplaySceneReady;
		struct GetGameplaySceneReady getGameplaySceneReady;
		struct SetActivePlayerFailedToConnect setActivePlayerFailedToConnect;
		struct SetGameplaySongReady setGameplaySongReady;
		struct GetGameplaySongReady getGameplaySongReady;
		struct SetSongStartTime setSongStartTime;
		struct NoteCut noteCut;
		struct NoteMissed noteMissed;
		struct LevelFinished levelFinished;
		struct ReturnToMenu returnToMenu;
		struct RequestReturnToMenu requestReturnToMenu;
		struct NoteSpawned noteSpawned;
		struct ObstacleSpawned obstacleSpawned;
		struct SliderSpawned sliderSpawned;
	};
};
struct PoseSerializable {
	struct Vector3Serializable position;
	struct QuaternionSerializable rotation;
};
struct NodePoseSyncState1 {
	struct PoseSerializable head;
	struct PoseSerializable leftController;
	struct PoseSerializable rightController;
};
struct SyncStateId {
	uint8_t id;
	bool same;
};
struct NodePoseSyncState {
	struct SyncStateId id;
	struct UTimestamp time;
	struct NodePoseSyncState1 state;
};
struct StandardScoreSyncState {
	int32_t modifiedScore;
	int32_t rawScore;
	int32_t immediateMaxPossibleRawScore;
	int32_t combo;
	int32_t multiplier;
};
struct ScoreSyncState {
	struct SyncStateId id;
	struct UTimestamp time;
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
struct MpRequirementSet {
	uint8_t difficulty;
	uint8_t requirements_len;
	struct String requirements[16];
};
struct MpMapColor {
	uint8_t difficulty;
	bool have_colorLeft;
	bool have_colorRight;
	bool have_envColorLeft;
	bool have_envColorRight;
	bool have_envColorLeftBoost;
	bool have_envColorRightBoost;
	bool have_obstacleColor;
	struct ColorNoAlphaSerializable colorLeft;
	struct ColorNoAlphaSerializable colorRight;
	struct ColorNoAlphaSerializable envColorLeft;
	struct ColorNoAlphaSerializable envColorRight;
	struct ColorNoAlphaSerializable envColorLeftBoost;
	struct ColorNoAlphaSerializable envColorRightBoost;
	struct ColorNoAlphaSerializable obstacleColor;
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
	MpBeatmapDifficulty difficulty;
	uint8_t requirementSets_len;
	struct MpRequirementSet requirementSets[5];
	uint8_t contributors_len;
	struct String contributors[24];
	uint8_t mapColors_len;
	struct MpMapColor mapColors[5];
};
struct MpPlayerData {
	struct String platformId;
	MpPlatform platform;
	struct String gameVersion;
};
struct MpexPlayerData {
	struct String nameColor;
};
struct CustomAvatarPacket {
	struct String hash;
	float scale;
	float floor;
};
struct MpcCapabilitiesPacket {
	uint32_t protocolVersion;
	bool canText;
};
struct MpcTextChatPacket {
	uint32_t protocolVersion;
	struct LongString text;
};
struct MpCore {
	struct String type;
	union {
		struct MpcTextChatPacket mpcTextChat;
		struct MpBeatmapPacket mpBeatmap;
		struct CustomAvatarPacket customAvatar;
		struct MpcCapabilitiesPacket mpcCapabilities;
		struct MpPlayerData mpPlayerData;
		struct MpexPlayerData mpexPlayerData;
	};
};
struct SyncTime {
	struct UTimestamp syncTime;
};
struct PlayerConnected {
	uint8_t remoteConnectionId;
	struct String userId;
	struct String userName;
	bool isConnectionOwner;
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
struct LegacyAvatarData {
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
struct WriterString {
	uint32_t length;
	char data[67];
};
struct WriterColor {
	float r;
	float g;
	float b;
	float a;
};
struct BeatAvatarData {
	struct WriterString headTopId;
	struct WriterColor headTopPrimaryColor;
	struct WriterColor headTopSecondaryColor;
	struct WriterString glassesId;
	struct WriterColor glassesColor;
	struct WriterString facialHairId;
	struct WriterColor facialHairColor;
	struct WriterString handsId;
	struct WriterColor handsColor;
	struct WriterString clothesId;
	struct WriterColor clothesPrimaryColor;
	struct WriterColor clothesSecondaryColor;
	struct WriterColor clothesDetailColor;
	struct WriterString skinColorId;
	struct WriterString eyesId;
	struct WriterString mouthId;
};
struct OpaqueAvatarData {
	uint32_t typeHash;
	uint16_t length;
	uint8_t data[4096];
};
struct MultiplayerAvatarsData {
	struct LegacyAvatarData legacy;
	int32_t count;
	struct OpaqueAvatarData avatars[6];
	struct BitMask128 supportedTypes;
};
struct PlayerIdentity {
	struct PlayerStateHash playerState;
	struct MultiplayerAvatarsData playerAvatars;
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
struct Party {
	uint8_t _empty;
};
struct MultiplayerSession {
	MultiplayerSessionMessageType type;
	union {
		struct MenuRpc menuRpc;
		struct GameplayRpc gameplayRpc;
		struct NodePoseSyncState nodePoseSyncState;
		struct ScoreSyncState scoreSyncState;
		struct NodePoseSyncStateDelta nodePoseSyncStateDelta;
		struct ScoreSyncStateDelta scoreSyncStateDelta;
		struct MpCore mpCore;
		struct BeatUpMessage beatUpMessage;
	};
};
struct KickPlayer {
	DisconnectedReason disconnectedReason;
};
struct PlayerStateUpdate {
	struct PlayerStateHash playerState;
};
struct PlayerAvatarUpdate {
	struct MultiplayerAvatarsData playerAvatars;
};
struct PingMessage {
	struct UTimestamp pingTime;
};
struct PongMessage {
	struct UTimestamp pingTime;
};
struct InternalMessage {
	InternalMessageType type;
	union {
		struct SyncTime syncTime;
		struct PlayerConnected playerConnected;
		struct PlayerIdentity playerIdentity;
		struct PlayerLatencyUpdate playerLatencyUpdate;
		struct PlayerDisconnected playerDisconnected;
		struct PlayerSortOrderUpdate playerSortOrderUpdate;
		struct Party party;
		struct MultiplayerSession multiplayerSession;
		struct KickPlayer kickPlayer;
		struct PlayerStateUpdate playerStateUpdate;
		struct PlayerAvatarUpdate playerAvatarUpdate;
		struct PingMessage pingMessage;
		struct PongMessage pongMessage;
	};
};
struct RoutingHeader {
	uint8_t remoteConnectionId;
	uint8_t connectionId;
	bool encrypted;
	uint8_t packetOptions;
};
struct BTRoutingHeader {
	uint8_t remoteConnectionId;
	uint8_t connectionId;
	uint8_t packetOptions;
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
struct MessageReceivedAcknowledge {
	struct BaseMasterServerResponse base;
	bool messageHandled;
};
struct MessageReceivedAcknowledgeProxy {
	uint8_t type;
	struct MessageReceivedAcknowledge value;
};
struct AuthenticationToken {
	Platform platform;
	struct String userId;
	struct String userName;
	struct ByteArrayNetSerializable sessionToken;
};
struct AuthenticateUserRequest {
	struct BaseMasterServerReliableResponse base;
	struct AuthenticationToken authenticationToken;
};
struct AuthenticateUserResponse {
	struct BaseMasterServerReliableResponse base;
	AuthenticateUserResponse_Result result;
};
struct IPEndPoint {
	struct String address;
	uint32_t port;
};
struct GameplayServerConfiguration {
	int32_t maxPlayerCount;
	DiscoveryPolicy discoveryPolicy;
	InvitePolicy invitePolicy;
	GameplayServerMode gameplayServerMode;
	SongSelectionMode songSelectionMode;
	GameplayServerControlSettings gameplayServerControlSettings;
};
struct ConnectToServerResponse {
	struct BaseMasterServerReliableResponse base;
	ConnectToServerResponse_Result result;
	struct String userId;
	struct String userName;
	struct String secret;
	struct BeatmapLevelSelectionMask selectionMask;
	bool isConnectionOwner;
	bool isDedicatedServer;
	struct IPEndPoint remoteEndPoint;
	struct Cookie32 random;
	struct ByteArrayNetSerializable publicKey;
	ServerCode code;
	struct GameplayServerConfiguration configuration;
	struct String managerId;
};
struct BaseConnectToServerRequest {
	struct BaseMasterServerReliableRequest base;
	struct String userId;
	struct String userName;
	struct Cookie32 random;
	struct ByteArrayNetSerializable publicKey;
};
struct ConnectToServerRequest {
	struct BaseConnectToServerRequest base;
	struct BeatmapLevelSelectionMask selectionMask;
	struct String secret;
	ServerCode code;
	struct GameplayServerConfiguration configuration;
};
struct MultipartMessage {
	struct BaseMasterServerReliableRequest base;
	uint32_t multipartMessageId;
	uint32_t offset;
	uint32_t length;
	uint32_t totalLength;
	uint8_t data[384];
};
struct MultipartMessageProxy {
	uint8_t type;
	struct MultipartMessage value;
};
struct SessionKeepaliveMessage {
	uint8_t _empty;
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
struct PublicServerInfo {
	ServerCode code;
	int32_t currentPlayerCount;
};
struct GetPublicServersResponse {
	struct BaseMasterServerReliableResponse base;
	GetPublicServersResponse_Result result;
	uint32_t publicServerCount;
	struct PublicServerInfo publicServers[20];
};
struct UserMessage {
	UserMessageType type;
	union {
		struct AuthenticateUserRequest authenticateUserRequest;
		struct AuthenticateUserResponse authenticateUserResponse;
		struct ConnectToServerResponse connectToServerResponse;
		struct ConnectToServerRequest connectToServerRequest;
		struct MessageReceivedAcknowledge messageReceivedAcknowledge;
		struct MultipartMessage multipartMessage;
		struct SessionKeepaliveMessage sessionKeepaliveMessage;
		struct GetPublicServersRequest getPublicServersRequest;
		struct GetPublicServersResponse getPublicServersResponse;
	};
};
struct AuthenticateGameLiftUserRequest {
	struct BaseMasterServerReliableResponse base;
	struct String userId;
	struct String userName;
	struct String playerSessionId;
};
struct GameLiftMessage {
	GameLiftMessageType type;
	union {
		struct AuthenticateGameLiftUserRequest authenticateGameLiftUserRequest;
		struct AuthenticateUserResponse authenticateUserResponse;
		struct MessageReceivedAcknowledge messageReceivedAcknowledge;
		struct MultipartMessage multipartMessage;
	};
};
struct ClientHelloRequest {
	struct BaseMasterServerReliableRequest base;
	struct Cookie32 random;
};
struct HelloVerifyRequest {
	struct BaseMasterServerReliableResponse base;
	struct Cookie32 cookie;
};
struct ClientHelloWithCookieRequest {
	struct BaseMasterServerReliableRequest base;
	uint32_t certificateResponseId;
	struct Cookie32 random;
	struct Cookie32 cookie;
};
struct ServerHelloRequest {
	struct BaseMasterServerReliableResponse base;
	struct Cookie32 random;
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
struct HandshakeMessage {
	HandshakeMessageType type;
	union {
		struct ClientHelloRequest clientHelloRequest;
		struct HelloVerifyRequest helloVerifyRequest;
		struct ClientHelloWithCookieRequest clientHelloWithCookieRequest;
		struct ServerHelloRequest serverHelloRequest;
		struct ServerCertificateRequest serverCertificateRequest;
		struct ClientKeyExchangeRequest clientKeyExchangeRequest;
		struct ChangeCipherSpecRequest changeCipherSpecRequest;
		struct MessageReceivedAcknowledge messageReceivedAcknowledge;
		struct MultipartMessage multipartMessage;
	};
};
struct SerializeHeader {
	uint32_t length;
};
struct FragmentedHeader {
	uint16_t fragmentId;
	uint16_t fragmentPart;
	uint16_t fragmentsTotal;
};
struct Unreliable {
	uint8_t _empty;
};
struct Channeled {
	uint16_t sequence;
	DeliveryMethod channelId;
};
struct Ack {
	uint16_t sequence;
	DeliveryMethod channelId;
	uint8_t data[32];
	uint8_t _pad0;
};
struct Ping {
	uint16_t sequence;
};
struct Pong {
	uint16_t sequence;
	uint64_t time;
};
struct ConnectMessage {
	struct String userId;
	struct String userName;
	bool isConnectionOwner;
	struct String playerSessionId;
};
struct ConnectRequest {
	uint32_t protocolId;
	uint64_t connectTime;
	int32_t peerId;
	uint8_t addrlen;
	uint8_t address[38];
	struct String secret;
	struct ConnectMessage message;
};
struct ConnectAccept {
	uint64_t connectTime;
	uint8_t connectNum;
	bool reusedPeer;
	int32_t peerId;
	struct ServerConnectInfo beatUp;
};
struct Disconnect {
	uint8_t _pad0[8];
};
struct UnconnectedMessage {
	MessageType type;
	uint32_t protocolVersion;
};
struct Mtu {
	uint32_t newMtu0;
	uint8_t pad[1423];
	uint32_t newMtu1;
};
struct MtuCheck {
	struct Mtu base;
};
struct MtuOk {
	struct Mtu base;
};
struct MergedHeader {
	uint16_t length;
};
struct Broadcast {
	uint8_t _empty;
};
struct Merged {
	uint8_t _empty;
};
struct ShutdownOk {
	uint8_t _empty;
};
struct PeerNotFound {
	uint8_t _empty;
};
struct InvalidProtocol {
	uint8_t _empty;
};
struct NatMessage {
	uint8_t _empty;
};
struct Empty {
	uint8_t _empty;
};
struct NetPacketHeader {
	PacketProperty property;
	uint8_t connectionNumber;
	bool isFragmented;
	union {
		struct Unreliable unreliable;
		struct Channeled channeled;
		struct Ack ack;
		struct Ping ping;
		struct Pong pong;
		struct ConnectRequest connectRequest;
		struct ConnectAccept connectAccept;
		struct Disconnect disconnect;
		struct UnconnectedMessage unconnectedMessage;
		struct MtuCheck mtuCheck;
		struct MtuOk mtuOk;
		struct Broadcast broadcast;
		struct Merged merged;
		struct ShutdownOk shutdownOk;
		struct PeerNotFound peerNotFound;
		struct InvalidProtocol invalidProtocol;
		struct NatMessage natMessage;
		struct Empty empty;
	};
};
struct MultipartMessageReadbackProxy {
	struct NetPacketHeader header;
	struct SerializeHeader serial;
	uint8_t type;
	struct BaseMasterServerReliableRequest base;
	uint32_t multipartMessageId;
};
struct PacketEncryptionLayer {
	uint8_t encrypted;
	uint32_t sequenceId;
	uint8_t iv[16];
};
struct WireServerConfiguration {
	struct GameplayServerConfiguration base;
	uint32_t shortCountdownMs;
	uint32_t longCountdownMs;
	bool skipResults;
	bool perPlayerDifficulty;
	bool perPlayerModifiers;
};
struct WireAddress {
	uint32_t length;
	uint8_t data[128];
};
struct WireInstanceAttach {
	uint32_t capacity;
	bool discover;
};
struct WireSessionAlloc {
	uint32_t room;
	struct String secret;
	struct String userId;
	bool ipv4;
	struct PacketContext clientVersion;
	struct Cookie32 random;
	struct ByteArrayNetSerializable publicKey;
};
struct WireSessionAllocResp {
	ConnectToServerResponse_Result result;
	struct GameplayServerConfiguration configuration;
	struct String managerId;
	struct IPEndPoint endPoint;
	uint32_t playerSlot;
	struct Cookie32 random;
	struct ByteArrayNetSerializable publicKey;
};
struct WireRoomSpawn {
	struct WireSessionAlloc base;
	struct WireServerConfiguration configuration;
};
struct WireRoomJoin {
	struct WireSessionAlloc base;
};
struct WireRoomSpawnResp {
	struct WireSessionAllocResp base;
};
struct WireRoomJoinResp {
	struct WireSessionAllocResp base;
};
struct WireRoomQuery {
	uint32_t room;
};
struct WireRoomQueryResp_PlayerInfo {
	struct PacketContext version;
	bool modded;
	struct String userName;
	struct Color32 colorScheme[7];
};
struct WireRoomQueryResp {
	struct String levelID;
	uint8_t hostIndex;
	uint8_t players_len;
	struct WireRoomQueryResp_PlayerInfo players[254];
};
struct WireStatusEntry {
	uint32_t code;
	uint8_t protocolVersion;
	uint8_t playerCount;
	uint8_t playerCapacity;
	uint16_t playerNPS;
	uint16_t levelNPS;
	bool public;
	bool quickplay;
	bool skipResults;
	bool perPlayerDifficulty;
	bool perPlayerModifiers;
	SongSelectionMode selectionMode;
	struct String levelName;
	struct String levelID;
	struct ByteArrayNetSerializable levelCover;
};
struct WireRoomStatusNotify {
	uint32_t entry_len;
	uint8_t entry[8384];
};
struct WireRoomCloseNotify {
	uint8_t _empty;
};
struct WireStatusAttach {
	uint8_t _empty;
};
struct WireGraphConnect {
	uint32_t code;
	struct String secret;
	struct String userId;
	struct WireServerConfiguration configuration;
	uint32_t protocolVersion;
	GameVersion gameVersion;
};
struct WireGraphConnectResp {
	MultiplayerPlacementErrorCode result;
	struct GameplayServerConfiguration configuration;
	uint32_t hostId;
	struct IPEndPoint endPoint;
	uint32_t roomSlot;
	uint32_t playerSlot;
	uint32_t code;
};
struct WireMessage {
	uint32_t cookie;
	WireMessageType type;
	union {
		struct WireInstanceAttach instanceAttach;
		struct WireStatusAttach statusAttach;
		struct WireRoomStatusNotify roomStatusNotify;
		struct WireRoomCloseNotify roomCloseNotify;
		struct WireRoomSpawn roomSpawn;
		struct WireRoomSpawnResp roomSpawnResp;
		struct WireRoomJoin roomJoin;
		struct WireRoomJoinResp roomJoinResp;
		struct WireRoomQuery roomQuery;
		struct WireRoomQueryResp roomQueryResp;
		struct WireGraphConnect graphConnect;
		struct WireGraphConnectResp graphConnectResp;
	};
};
static const struct PacketContext PV_LEGACY_DEFAULT = {
	.netVersion = 11,
	.protocolVersion = 6,
	.gameVersion = GameVersion_1_19_0,
};
static const struct PacketContext PV_WIRE = {
	.netVersion = 12,
};
void _pkt_ServerConnectInfo_read(struct ServerConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpMessage_read(struct BeatUpMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpMessage_write(const struct BeatUpMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ModConnectHeader_write(const struct ModConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_MpBeatmapPacket_read(struct MpBeatmapPacket *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatAvatarData_read(struct BeatAvatarData *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatAvatarData_write(const struct BeatAvatarData *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_InternalMessage_read(struct InternalMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_InternalMessage_write(const struct InternalMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_RoutingHeader_read(struct RoutingHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_RoutingHeader_write(const struct RoutingHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BTRoutingHeader_read(struct BTRoutingHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_MessageReceivedAcknowledgeProxy_write(const struct MessageReceivedAcknowledgeProxy *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_MultipartMessageProxy_write(const struct MultipartMessageProxy *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_UserMessage_read(struct UserMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_UserMessage_write(const struct UserMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_GameLiftMessage_read(struct GameLiftMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_GameLiftMessage_write(const struct GameLiftMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_HandshakeMessage_read(struct HandshakeMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_HandshakeMessage_write(const struct HandshakeMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_SerializeHeader_read(struct SerializeHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_SerializeHeader_write(const struct SerializeHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_FragmentedHeader_read(struct FragmentedHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_FragmentedHeader_write(const struct FragmentedHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ConnectMessage_read(struct ConnectMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_UnconnectedMessage_read(struct UnconnectedMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_UnconnectedMessage_write(const struct UnconnectedMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_MergedHeader_read(struct MergedHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_MergedHeader_write(const struct MergedHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_NetPacketHeader_read(struct NetPacketHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_NetPacketHeader_write(const struct NetPacketHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_MultipartMessageReadbackProxy_read(struct MultipartMessageReadbackProxy *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_PacketEncryptionLayer_read(struct PacketEncryptionLayer *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_PacketEncryptionLayer_write(const struct PacketEncryptionLayer *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_WireStatusEntry_read(struct WireStatusEntry *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_WireStatusEntry_write(const struct WireStatusEntry *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_WireMessage_read(struct WireMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_WireMessage_write(const struct WireMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define reflect(type, value) _reflect_##type(value)
typedef void (*PacketWriteFunc)(const void *restrict, uint8_t**, const uint8_t*, struct PacketContext);
typedef void (*PacketReadFunc)(void *restrict, const uint8_t**, const uint8_t*, struct PacketContext);
size_t _pkt_try_read(PacketReadFunc inner, void *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
size_t _pkt_try_write(PacketWriteFunc inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define pkt_write_c(pkt, end, ctx, type, ...) _pkt_try_write((PacketWriteFunc)_pkt_##type##_write, &(struct type)__VA_ARGS__, pkt, end, ctx)
#define _pkt_read_func(data) ((PacketReadFunc)_Generic(*(data), struct ServerConnectInfo: _pkt_ServerConnectInfo_read, struct BeatUpMessage: _pkt_BeatUpMessage_read, struct ModConnectHeader: _pkt_ModConnectHeader_read, struct MpBeatmapPacket: _pkt_MpBeatmapPacket_read, struct BeatAvatarData: _pkt_BeatAvatarData_read, struct InternalMessage: _pkt_InternalMessage_read, struct RoutingHeader: _pkt_RoutingHeader_read, struct BTRoutingHeader: _pkt_BTRoutingHeader_read, struct UserMessage: _pkt_UserMessage_read, struct GameLiftMessage: _pkt_GameLiftMessage_read, struct HandshakeMessage: _pkt_HandshakeMessage_read, struct SerializeHeader: _pkt_SerializeHeader_read, struct FragmentedHeader: _pkt_FragmentedHeader_read, struct ConnectMessage: _pkt_ConnectMessage_read, struct UnconnectedMessage: _pkt_UnconnectedMessage_read, struct MergedHeader: _pkt_MergedHeader_read, struct NetPacketHeader: _pkt_NetPacketHeader_read, struct MultipartMessageReadbackProxy: _pkt_MultipartMessageReadbackProxy_read, struct PacketEncryptionLayer: _pkt_PacketEncryptionLayer_read, struct WireStatusEntry: _pkt_WireStatusEntry_read, struct WireMessage: _pkt_WireMessage_read))
#define _pkt_write_func(data) ((PacketWriteFunc)_Generic(*(data), struct BeatUpMessage: _pkt_BeatUpMessage_write, struct ModConnectHeader: _pkt_ModConnectHeader_write, struct BeatAvatarData: _pkt_BeatAvatarData_write, struct InternalMessage: _pkt_InternalMessage_write, struct RoutingHeader: _pkt_RoutingHeader_write, struct MessageReceivedAcknowledgeProxy: _pkt_MessageReceivedAcknowledgeProxy_write, struct MultipartMessageProxy: _pkt_MultipartMessageProxy_write, struct UserMessage: _pkt_UserMessage_write, struct GameLiftMessage: _pkt_GameLiftMessage_write, struct HandshakeMessage: _pkt_HandshakeMessage_write, struct SerializeHeader: _pkt_SerializeHeader_write, struct FragmentedHeader: _pkt_FragmentedHeader_write, struct UnconnectedMessage: _pkt_UnconnectedMessage_write, struct MergedHeader: _pkt_MergedHeader_write, struct NetPacketHeader: _pkt_NetPacketHeader_write, struct PacketEncryptionLayer: _pkt_PacketEncryptionLayer_write, struct WireStatusEntry: _pkt_WireStatusEntry_write, struct WireMessage: _pkt_WireMessage_write))
#define pkt_read(data, ...) _pkt_try_read(_pkt_read_func(data), data, __VA_ARGS__)
#define pkt_write(data, ...) _pkt_try_write(_pkt_write_func(data), data, __VA_ARGS__)
size_t pkt_write_bytes(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx, size_t count);
