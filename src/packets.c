#include "packets.h"
#include "packets.c.h"
static void _pkt_ByteArrayNetSerializable_read(struct ByteArrayNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_read(&data->length, pkt, end, ctx);
	_pkt_raw_read(data->data, pkt, end, ctx, check_overflow(data->length, 8192, "ByteArrayNetSerializable.data"));
}
static void _pkt_ByteArrayNetSerializable_write(const struct ByteArrayNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_write(&data->length, pkt, end, ctx);
	_pkt_raw_write(data->data, pkt, end, ctx, check_overflow(data->length, 8192, "ByteArrayNetSerializable.data"));
}
static void _pkt_BitMask128_read(struct BitMask128 *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u64_read(&data->d0, pkt, end, ctx);
	_pkt_u64_read(&data->d1, pkt, end, ctx);
}
static void _pkt_BitMask128_write(const struct BitMask128 *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u64_write(&data->d0, pkt, end, ctx);
	_pkt_u64_write(&data->d1, pkt, end, ctx);
}
static void _pkt_SongPackMask_read(struct SongPackMask *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BitMask128_read(&data->bloomFilter, pkt, end, ctx);
}
static void _pkt_SongPackMask_write(const struct SongPackMask *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BitMask128_write(&data->bloomFilter, pkt, end, ctx);
}
static void _pkt_BeatmapLevelSelectionMask_read(struct BeatmapLevelSelectionMask *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->difficulties, pkt, end, ctx);
	_pkt_u32_read(&data->modifiers, pkt, end, ctx);
	_pkt_SongPackMask_read(&data->songPacks, pkt, end, ctx);
}
static void _pkt_BeatmapLevelSelectionMask_write(const struct BeatmapLevelSelectionMask *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->difficulties, pkt, end, ctx);
	_pkt_u32_write(&data->modifiers, pkt, end, ctx);
	_pkt_SongPackMask_write(&data->songPacks, pkt, end, ctx);
}
static void _pkt_RemoteProcedureCall_read(struct RemoteProcedureCall *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->syncTime, pkt, end, ctx);
}
static void _pkt_RemoteProcedureCall_write(const struct RemoteProcedureCall *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->syncTime, pkt, end, ctx);
}
static void _pkt_PlayersMissingEntitlementsNetSerializable_read(struct PlayersMissingEntitlementsNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_i32_read(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "PlayersMissingEntitlementsNetSerializable.playersWithoutEntitlements"); i < count; ++i)
		_pkt_String_read(&data->playersWithoutEntitlements[i], pkt, end, ctx);
}
static void _pkt_PlayersMissingEntitlementsNetSerializable_write(const struct PlayersMissingEntitlementsNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_i32_write(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "PlayersMissingEntitlementsNetSerializable.playersWithoutEntitlements"); i < count; ++i)
		_pkt_String_write(&data->playersWithoutEntitlements[i], pkt, end, ctx);
}
static void _pkt_SetPlayersMissingEntitlementsToLevel_read(struct SetPlayersMissingEntitlementsToLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayersMissingEntitlementsNetSerializable_read(&data->playersMissingEntitlements, pkt, end, ctx);
	}
}
static void _pkt_SetPlayersMissingEntitlementsToLevel_write(const struct SetPlayersMissingEntitlementsToLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayersMissingEntitlementsNetSerializable_write(&data->playersMissingEntitlements, pkt, end, ctx);
	}
}
static void _pkt_GetIsEntitledToLevel_read(struct GetIsEntitledToLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, pkt, end, ctx);
	}
}
static void _pkt_GetIsEntitledToLevel_write(const struct GetIsEntitledToLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, pkt, end, ctx);
	}
}
static void _pkt_SetIsEntitledToLevel_read(struct SetIsEntitledToLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_vi32_read(&data->entitlementStatus, pkt, end, ctx);
	}
}
static void _pkt_SetIsEntitledToLevel_write(const struct SetIsEntitledToLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_vi32_write(&data->entitlementStatus, pkt, end, ctx);
	}
}
static void _pkt_InvalidateLevelEntitlementStatuses_read(struct InvalidateLevelEntitlementStatuses *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_InvalidateLevelEntitlementStatuses_write(const struct InvalidateLevelEntitlementStatuses *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SelectLevelPack_read(struct SelectLevelPack *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_String_read(&data->levelPackId, pkt, end, ctx);
	}
}
static void _pkt_SelectLevelPack_write(const struct SelectLevelPack *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_String_write(&data->levelPackId, pkt, end, ctx);
	}
}
static void _pkt_BeatmapIdentifierNetSerializable_read(struct BeatmapIdentifierNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_read(&data->levelID, pkt, end, ctx);
	_pkt_String_read(&data->beatmapCharacteristicSerializedName, pkt, end, ctx);
	_pkt_vu32_read(&data->difficulty, pkt, end, ctx);
}
static void _pkt_BeatmapIdentifierNetSerializable_write(const struct BeatmapIdentifierNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_write(&data->levelID, pkt, end, ctx);
	_pkt_String_write(&data->beatmapCharacteristicSerializedName, pkt, end, ctx);
	_pkt_vu32_write(&data->difficulty, pkt, end, ctx);
}
static void _pkt_SetSelectedBeatmap_read(struct SetSelectedBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_read(&data->identifier, pkt, end, ctx);
	}
}
static void _pkt_SetSelectedBeatmap_write(const struct SetSelectedBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_write(&data->identifier, pkt, end, ctx);
	}
}
static void _pkt_GetSelectedBeatmap_read(struct GetSelectedBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetSelectedBeatmap_write(const struct GetSelectedBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_RecommendBeatmap_read(struct RecommendBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_read(&data->identifier, pkt, end, ctx);
	}
}
static void _pkt_RecommendBeatmap_write(const struct RecommendBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_write(&data->identifier, pkt, end, ctx);
	}
}
static void _pkt_ClearRecommendedBeatmap_read(struct ClearRecommendedBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_ClearRecommendedBeatmap_write(const struct ClearRecommendedBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GetRecommendedBeatmap_read(struct GetRecommendedBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetRecommendedBeatmap_write(const struct GetRecommendedBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GameplayModifiers_read(struct GameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint32_t bitfield0;
	_pkt_u32_read(&bitfield0, pkt, end, ctx);
	data->energyType = bitfield0 >> 0 & 15;
	data->_pad0 = bitfield0 >> 4 & 1;
	data->demoNoFail = bitfield0 >> 5 & 1;
	data->instaFail = bitfield0 >> 6 & 1;
	data->failOnSaberClash = bitfield0 >> 7 & 1;
	data->enabledObstacleType = bitfield0 >> 8 & 15;
	data->demoNoObstacles = bitfield0 >> 12 & 1;
	data->noBombs = bitfield0 >> 13 & 1;
	data->fastNotes = bitfield0 >> 14 & 1;
	data->strictAngles = bitfield0 >> 15 & 1;
	data->disappearingArrows = bitfield0 >> 16 & 1;
	data->ghostNotes = bitfield0 >> 17 & 1;
	data->songSpeed = bitfield0 >> 18 & 15;
	data->noArrows = bitfield0 >> 22 & 1;
	data->noFailOn0Energy = bitfield0 >> 23 & 1;
	data->proMode = bitfield0 >> 24 & 1;
	data->zenMode = bitfield0 >> 25 & 1;
	data->smallCubes = bitfield0 >> 26 & 1;
}
static void _pkt_GameplayModifiers_write(const struct GameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint32_t bitfield0 = 0;
	bitfield0 |= (data->energyType & 15u) << 0;
	bitfield0 |= (data->_pad0 & 1u) << 4;
	bitfield0 |= (data->demoNoFail & 1u) << 5;
	bitfield0 |= (data->instaFail & 1u) << 6;
	bitfield0 |= (data->failOnSaberClash & 1u) << 7;
	bitfield0 |= (data->enabledObstacleType & 15u) << 8;
	bitfield0 |= (data->demoNoObstacles & 1u) << 12;
	bitfield0 |= (data->noBombs & 1u) << 13;
	bitfield0 |= (data->fastNotes & 1u) << 14;
	bitfield0 |= (data->strictAngles & 1u) << 15;
	bitfield0 |= (data->disappearingArrows & 1u) << 16;
	bitfield0 |= (data->ghostNotes & 1u) << 17;
	bitfield0 |= (data->songSpeed & 15u) << 18;
	bitfield0 |= (data->noArrows & 1u) << 22;
	bitfield0 |= (data->noFailOn0Energy & 1u) << 23;
	bitfield0 |= (data->proMode & 1u) << 24;
	bitfield0 |= (data->zenMode & 1u) << 25;
	bitfield0 |= (data->smallCubes & 1u) << 26;
	_pkt_u32_write(&bitfield0, pkt, end, ctx);
}
static void _pkt_SetSelectedGameplayModifiers_read(struct SetSelectedGameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_read(&data->gameplayModifiers, pkt, end, ctx);
	}
}
static void _pkt_SetSelectedGameplayModifiers_write(const struct SetSelectedGameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_write(&data->gameplayModifiers, pkt, end, ctx);
	}
}
static void _pkt_GetSelectedGameplayModifiers_read(struct GetSelectedGameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetSelectedGameplayModifiers_write(const struct GetSelectedGameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_RecommendGameplayModifiers_read(struct RecommendGameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_read(&data->gameplayModifiers, pkt, end, ctx);
	}
}
static void _pkt_RecommendGameplayModifiers_write(const struct RecommendGameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_write(&data->gameplayModifiers, pkt, end, ctx);
	}
}
static void _pkt_ClearRecommendedGameplayModifiers_read(struct ClearRecommendedGameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_ClearRecommendedGameplayModifiers_write(const struct ClearRecommendedGameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GetRecommendedGameplayModifiers_read(struct GetRecommendedGameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetRecommendedGameplayModifiers_write(const struct GetRecommendedGameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_LevelLoadError_read(struct LevelLoadError *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, pkt, end, ctx);
	}
}
static void _pkt_LevelLoadError_write(const struct LevelLoadError *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, pkt, end, ctx);
	}
}
static void _pkt_LevelLoadSuccess_read(struct LevelLoadSuccess *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, pkt, end, ctx);
	}
}
static void _pkt_LevelLoadSuccess_write(const struct LevelLoadSuccess *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, pkt, end, ctx);
	}
}
static void _pkt_StartLevel_read(struct StartLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_read(&data->beatmapId, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_GameplayModifiers_read(&data->gameplayModifiers, pkt, end, ctx);
	}
	if(data->flags.hasValue2) {
		_pkt_f32_read(&data->startTime, pkt, end, ctx);
	}
}
static void _pkt_StartLevel_write(const struct StartLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_write(&data->beatmapId, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_GameplayModifiers_write(&data->gameplayModifiers, pkt, end, ctx);
	}
	if(data->flags.hasValue2) {
		_pkt_f32_write(&data->startTime, pkt, end, ctx);
	}
}
static void _pkt_GetStartedLevel_read(struct GetStartedLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetStartedLevel_write(const struct GetStartedLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_CancelLevelStart_read(struct CancelLevelStart *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_CancelLevelStart_write(const struct CancelLevelStart *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GetMultiplayerGameState_read(struct GetMultiplayerGameState *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetMultiplayerGameState_write(const struct GetMultiplayerGameState *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetMultiplayerGameState_read(struct SetMultiplayerGameState *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_vi32_read(&data->lobbyState, pkt, end, ctx);
	}
}
static void _pkt_SetMultiplayerGameState_write(const struct SetMultiplayerGameState *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_vi32_write(&data->lobbyState, pkt, end, ctx);
	}
}
static void _pkt_GetIsReady_read(struct GetIsReady *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetIsReady_write(const struct GetIsReady *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetIsReady_read(struct SetIsReady *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_b_read(&data->isReady, pkt, end, ctx);
	}
}
static void _pkt_SetIsReady_write(const struct SetIsReady *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_b_write(&data->isReady, pkt, end, ctx);
	}
}
static void _pkt_SetStartGameTime_read(struct SetStartGameTime *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->newTime, pkt, end, ctx);
	}
}
static void _pkt_SetStartGameTime_write(const struct SetStartGameTime *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->newTime, pkt, end, ctx);
	}
}
static void _pkt_CancelStartGameTime_read(struct CancelStartGameTime *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_CancelStartGameTime_write(const struct CancelStartGameTime *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GetIsInLobby_read(struct GetIsInLobby *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetIsInLobby_write(const struct GetIsInLobby *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetIsInLobby_read(struct SetIsInLobby *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_b_read(&data->isBack, pkt, end, ctx);
	}
}
static void _pkt_SetIsInLobby_write(const struct SetIsInLobby *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_b_write(&data->isBack, pkt, end, ctx);
	}
}
static void _pkt_GetCountdownEndTime_read(struct GetCountdownEndTime *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetCountdownEndTime_write(const struct GetCountdownEndTime *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetCountdownEndTime_read(struct SetCountdownEndTime *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->newTime, pkt, end, ctx);
	}
}
static void _pkt_SetCountdownEndTime_write(const struct SetCountdownEndTime *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->newTime, pkt, end, ctx);
	}
}
static void _pkt_CancelCountdown_read(struct CancelCountdown *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_CancelCountdown_write(const struct CancelCountdown *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GetOwnedSongPacks_read(struct GetOwnedSongPacks *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetOwnedSongPacks_write(const struct GetOwnedSongPacks *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetOwnedSongPacks_read(struct SetOwnedSongPacks *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_SongPackMask_read(&data->songPackMask, pkt, end, ctx);
	}
}
static void _pkt_SetOwnedSongPacks_write(const struct SetOwnedSongPacks *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_SongPackMask_write(&data->songPackMask, pkt, end, ctx);
	}
}
static void _pkt_RequestKickPlayer_read(struct RequestKickPlayer *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_String_read(&data->kickedPlayerId, pkt, end, ctx);
	}
}
static void _pkt_RequestKickPlayer_write(const struct RequestKickPlayer *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_String_write(&data->kickedPlayerId, pkt, end, ctx);
	}
}
static void _pkt_GetPermissionConfiguration_read(struct GetPermissionConfiguration *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetPermissionConfiguration_write(const struct GetPermissionConfiguration *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_PlayerLobbyPermissionConfigurationNetSerializable_read(struct PlayerLobbyPermissionConfigurationNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->userId, pkt, end, ctx);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->isServerOwner = bitfield0 >> 0 & 1;
	data->hasRecommendBeatmapsPermission = bitfield0 >> 1 & 1;
	data->hasRecommendGameplayModifiersPermission = bitfield0 >> 2 & 1;
	data->hasKickVotePermission = bitfield0 >> 3 & 1;
	data->hasInvitePermission = bitfield0 >> 4 & 1;
}
static void _pkt_PlayerLobbyPermissionConfigurationNetSerializable_write(const struct PlayerLobbyPermissionConfigurationNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->userId, pkt, end, ctx);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->isServerOwner & 1u) << 0;
	bitfield0 |= (data->hasRecommendBeatmapsPermission & 1u) << 1;
	bitfield0 |= (data->hasRecommendGameplayModifiersPermission & 1u) << 2;
	bitfield0 |= (data->hasKickVotePermission & 1u) << 3;
	bitfield0 |= (data->hasInvitePermission & 1u) << 4;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
}
static void _pkt_PlayersLobbyPermissionConfigurationNetSerializable_read(struct PlayersLobbyPermissionConfigurationNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_i32_read(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "PlayersLobbyPermissionConfigurationNetSerializable.playersPermission"); i < count; ++i)
		_pkt_PlayerLobbyPermissionConfigurationNetSerializable_read(&data->playersPermission[i], pkt, end, ctx);
}
static void _pkt_PlayersLobbyPermissionConfigurationNetSerializable_write(const struct PlayersLobbyPermissionConfigurationNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_i32_write(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "PlayersLobbyPermissionConfigurationNetSerializable.playersPermission"); i < count; ++i)
		_pkt_PlayerLobbyPermissionConfigurationNetSerializable_write(&data->playersPermission[i], pkt, end, ctx);
}
static void _pkt_SetPermissionConfiguration_read(struct SetPermissionConfiguration *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayersLobbyPermissionConfigurationNetSerializable_read(&data->playersPermissionConfiguration, pkt, end, ctx);
	}
}
static void _pkt_SetPermissionConfiguration_write(const struct SetPermissionConfiguration *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayersLobbyPermissionConfigurationNetSerializable_write(&data->playersPermissionConfiguration, pkt, end, ctx);
	}
}
static void _pkt_GetIsStartButtonEnabled_read(struct GetIsStartButtonEnabled *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetIsStartButtonEnabled_write(const struct GetIsStartButtonEnabled *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetIsStartButtonEnabled_read(struct SetIsStartButtonEnabled *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_vi32_read(&data->reason, pkt, end, ctx);
	}
}
static void _pkt_SetIsStartButtonEnabled_write(const struct SetIsStartButtonEnabled *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_vi32_write(&data->reason, pkt, end, ctx);
	}
}
static void _pkt_ClearSelectedBeatmap_read(struct ClearSelectedBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_ClearSelectedBeatmap_write(const struct ClearSelectedBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_ClearSelectedGameplayModifiers_read(struct ClearSelectedGameplayModifiers *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_ClearSelectedGameplayModifiers_write(const struct ClearSelectedGameplayModifiers *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_MenuRpc_read(struct MenuRpc *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: _pkt_SetPlayersMissingEntitlementsToLevel_read(&data->setPlayersMissingEntitlementsToLevel, pkt, end, ctx); break;
		case MenuRpcType_GetIsEntitledToLevel: _pkt_GetIsEntitledToLevel_read(&data->getIsEntitledToLevel, pkt, end, ctx); break;
		case MenuRpcType_SetIsEntitledToLevel: _pkt_SetIsEntitledToLevel_read(&data->setIsEntitledToLevel, pkt, end, ctx); break;
		case MenuRpcType_InvalidateLevelEntitlementStatuses: _pkt_InvalidateLevelEntitlementStatuses_read(&data->invalidateLevelEntitlementStatuses, pkt, end, ctx); break;
		case MenuRpcType_SelectLevelPack: _pkt_SelectLevelPack_read(&data->selectLevelPack, pkt, end, ctx); break;
		case MenuRpcType_SetSelectedBeatmap: _pkt_SetSelectedBeatmap_read(&data->setSelectedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_GetSelectedBeatmap: _pkt_GetSelectedBeatmap_read(&data->getSelectedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_RecommendBeatmap: _pkt_RecommendBeatmap_read(&data->recommendBeatmap, pkt, end, ctx); break;
		case MenuRpcType_ClearRecommendedBeatmap: _pkt_ClearRecommendedBeatmap_read(&data->clearRecommendedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_GetRecommendedBeatmap: _pkt_GetRecommendedBeatmap_read(&data->getRecommendedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_SetSelectedGameplayModifiers: _pkt_SetSelectedGameplayModifiers_read(&data->setSelectedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_GetSelectedGameplayModifiers: _pkt_GetSelectedGameplayModifiers_read(&data->getSelectedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_RecommendGameplayModifiers: _pkt_RecommendGameplayModifiers_read(&data->recommendGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_ClearRecommendedGameplayModifiers: _pkt_ClearRecommendedGameplayModifiers_read(&data->clearRecommendedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_GetRecommendedGameplayModifiers: _pkt_GetRecommendedGameplayModifiers_read(&data->getRecommendedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_LevelLoadError: _pkt_LevelLoadError_read(&data->levelLoadError, pkt, end, ctx); break;
		case MenuRpcType_LevelLoadSuccess: _pkt_LevelLoadSuccess_read(&data->levelLoadSuccess, pkt, end, ctx); break;
		case MenuRpcType_StartLevel: _pkt_StartLevel_read(&data->startLevel, pkt, end, ctx); break;
		case MenuRpcType_GetStartedLevel: _pkt_GetStartedLevel_read(&data->getStartedLevel, pkt, end, ctx); break;
		case MenuRpcType_CancelLevelStart: _pkt_CancelLevelStart_read(&data->cancelLevelStart, pkt, end, ctx); break;
		case MenuRpcType_GetMultiplayerGameState: _pkt_GetMultiplayerGameState_read(&data->getMultiplayerGameState, pkt, end, ctx); break;
		case MenuRpcType_SetMultiplayerGameState: _pkt_SetMultiplayerGameState_read(&data->setMultiplayerGameState, pkt, end, ctx); break;
		case MenuRpcType_GetIsReady: _pkt_GetIsReady_read(&data->getIsReady, pkt, end, ctx); break;
		case MenuRpcType_SetIsReady: _pkt_SetIsReady_read(&data->setIsReady, pkt, end, ctx); break;
		case MenuRpcType_SetStartGameTime: _pkt_SetStartGameTime_read(&data->setStartGameTime, pkt, end, ctx); break;
		case MenuRpcType_CancelStartGameTime: _pkt_CancelStartGameTime_read(&data->cancelStartGameTime, pkt, end, ctx); break;
		case MenuRpcType_GetIsInLobby: _pkt_GetIsInLobby_read(&data->getIsInLobby, pkt, end, ctx); break;
		case MenuRpcType_SetIsInLobby: _pkt_SetIsInLobby_read(&data->setIsInLobby, pkt, end, ctx); break;
		case MenuRpcType_GetCountdownEndTime: _pkt_GetCountdownEndTime_read(&data->getCountdownEndTime, pkt, end, ctx); break;
		case MenuRpcType_SetCountdownEndTime: _pkt_SetCountdownEndTime_read(&data->setCountdownEndTime, pkt, end, ctx); break;
		case MenuRpcType_CancelCountdown: _pkt_CancelCountdown_read(&data->cancelCountdown, pkt, end, ctx); break;
		case MenuRpcType_GetOwnedSongPacks: _pkt_GetOwnedSongPacks_read(&data->getOwnedSongPacks, pkt, end, ctx); break;
		case MenuRpcType_SetOwnedSongPacks: _pkt_SetOwnedSongPacks_read(&data->setOwnedSongPacks, pkt, end, ctx); break;
		case MenuRpcType_RequestKickPlayer: _pkt_RequestKickPlayer_read(&data->requestKickPlayer, pkt, end, ctx); break;
		case MenuRpcType_GetPermissionConfiguration: _pkt_GetPermissionConfiguration_read(&data->getPermissionConfiguration, pkt, end, ctx); break;
		case MenuRpcType_SetPermissionConfiguration: _pkt_SetPermissionConfiguration_read(&data->setPermissionConfiguration, pkt, end, ctx); break;
		case MenuRpcType_GetIsStartButtonEnabled: _pkt_GetIsStartButtonEnabled_read(&data->getIsStartButtonEnabled, pkt, end, ctx); break;
		case MenuRpcType_SetIsStartButtonEnabled: _pkt_SetIsStartButtonEnabled_read(&data->setIsStartButtonEnabled, pkt, end, ctx); break;
		case MenuRpcType_ClearSelectedBeatmap: _pkt_ClearSelectedBeatmap_read(&data->clearSelectedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_ClearSelectedGameplayModifiers: _pkt_ClearSelectedGameplayModifiers_read(&data->clearSelectedGameplayModifiers, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `MenuRpcType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_MenuRpc_write(const struct MenuRpc *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: _pkt_SetPlayersMissingEntitlementsToLevel_write(&data->setPlayersMissingEntitlementsToLevel, pkt, end, ctx); break;
		case MenuRpcType_GetIsEntitledToLevel: _pkt_GetIsEntitledToLevel_write(&data->getIsEntitledToLevel, pkt, end, ctx); break;
		case MenuRpcType_SetIsEntitledToLevel: _pkt_SetIsEntitledToLevel_write(&data->setIsEntitledToLevel, pkt, end, ctx); break;
		case MenuRpcType_InvalidateLevelEntitlementStatuses: _pkt_InvalidateLevelEntitlementStatuses_write(&data->invalidateLevelEntitlementStatuses, pkt, end, ctx); break;
		case MenuRpcType_SelectLevelPack: _pkt_SelectLevelPack_write(&data->selectLevelPack, pkt, end, ctx); break;
		case MenuRpcType_SetSelectedBeatmap: _pkt_SetSelectedBeatmap_write(&data->setSelectedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_GetSelectedBeatmap: _pkt_GetSelectedBeatmap_write(&data->getSelectedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_RecommendBeatmap: _pkt_RecommendBeatmap_write(&data->recommendBeatmap, pkt, end, ctx); break;
		case MenuRpcType_ClearRecommendedBeatmap: _pkt_ClearRecommendedBeatmap_write(&data->clearRecommendedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_GetRecommendedBeatmap: _pkt_GetRecommendedBeatmap_write(&data->getRecommendedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_SetSelectedGameplayModifiers: _pkt_SetSelectedGameplayModifiers_write(&data->setSelectedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_GetSelectedGameplayModifiers: _pkt_GetSelectedGameplayModifiers_write(&data->getSelectedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_RecommendGameplayModifiers: _pkt_RecommendGameplayModifiers_write(&data->recommendGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_ClearRecommendedGameplayModifiers: _pkt_ClearRecommendedGameplayModifiers_write(&data->clearRecommendedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_GetRecommendedGameplayModifiers: _pkt_GetRecommendedGameplayModifiers_write(&data->getRecommendedGameplayModifiers, pkt, end, ctx); break;
		case MenuRpcType_LevelLoadError: _pkt_LevelLoadError_write(&data->levelLoadError, pkt, end, ctx); break;
		case MenuRpcType_LevelLoadSuccess: _pkt_LevelLoadSuccess_write(&data->levelLoadSuccess, pkt, end, ctx); break;
		case MenuRpcType_StartLevel: _pkt_StartLevel_write(&data->startLevel, pkt, end, ctx); break;
		case MenuRpcType_GetStartedLevel: _pkt_GetStartedLevel_write(&data->getStartedLevel, pkt, end, ctx); break;
		case MenuRpcType_CancelLevelStart: _pkt_CancelLevelStart_write(&data->cancelLevelStart, pkt, end, ctx); break;
		case MenuRpcType_GetMultiplayerGameState: _pkt_GetMultiplayerGameState_write(&data->getMultiplayerGameState, pkt, end, ctx); break;
		case MenuRpcType_SetMultiplayerGameState: _pkt_SetMultiplayerGameState_write(&data->setMultiplayerGameState, pkt, end, ctx); break;
		case MenuRpcType_GetIsReady: _pkt_GetIsReady_write(&data->getIsReady, pkt, end, ctx); break;
		case MenuRpcType_SetIsReady: _pkt_SetIsReady_write(&data->setIsReady, pkt, end, ctx); break;
		case MenuRpcType_SetStartGameTime: _pkt_SetStartGameTime_write(&data->setStartGameTime, pkt, end, ctx); break;
		case MenuRpcType_CancelStartGameTime: _pkt_CancelStartGameTime_write(&data->cancelStartGameTime, pkt, end, ctx); break;
		case MenuRpcType_GetIsInLobby: _pkt_GetIsInLobby_write(&data->getIsInLobby, pkt, end, ctx); break;
		case MenuRpcType_SetIsInLobby: _pkt_SetIsInLobby_write(&data->setIsInLobby, pkt, end, ctx); break;
		case MenuRpcType_GetCountdownEndTime: _pkt_GetCountdownEndTime_write(&data->getCountdownEndTime, pkt, end, ctx); break;
		case MenuRpcType_SetCountdownEndTime: _pkt_SetCountdownEndTime_write(&data->setCountdownEndTime, pkt, end, ctx); break;
		case MenuRpcType_CancelCountdown: _pkt_CancelCountdown_write(&data->cancelCountdown, pkt, end, ctx); break;
		case MenuRpcType_GetOwnedSongPacks: _pkt_GetOwnedSongPacks_write(&data->getOwnedSongPacks, pkt, end, ctx); break;
		case MenuRpcType_SetOwnedSongPacks: _pkt_SetOwnedSongPacks_write(&data->setOwnedSongPacks, pkt, end, ctx); break;
		case MenuRpcType_RequestKickPlayer: _pkt_RequestKickPlayer_write(&data->requestKickPlayer, pkt, end, ctx); break;
		case MenuRpcType_GetPermissionConfiguration: _pkt_GetPermissionConfiguration_write(&data->getPermissionConfiguration, pkt, end, ctx); break;
		case MenuRpcType_SetPermissionConfiguration: _pkt_SetPermissionConfiguration_write(&data->setPermissionConfiguration, pkt, end, ctx); break;
		case MenuRpcType_GetIsStartButtonEnabled: _pkt_GetIsStartButtonEnabled_write(&data->getIsStartButtonEnabled, pkt, end, ctx); break;
		case MenuRpcType_SetIsStartButtonEnabled: _pkt_SetIsStartButtonEnabled_write(&data->setIsStartButtonEnabled, pkt, end, ctx); break;
		case MenuRpcType_ClearSelectedBeatmap: _pkt_ClearSelectedBeatmap_write(&data->clearSelectedBeatmap, pkt, end, ctx); break;
		case MenuRpcType_ClearSelectedGameplayModifiers: _pkt_ClearSelectedGameplayModifiers_write(&data->clearSelectedGameplayModifiers, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `MenuRpcType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_ColorNoAlphaSerializable_read(struct ColorNoAlphaSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->r, pkt, end, ctx);
	_pkt_f32_read(&data->g, pkt, end, ctx);
	_pkt_f32_read(&data->b, pkt, end, ctx);
}
static void _pkt_ColorNoAlphaSerializable_write(const struct ColorNoAlphaSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->r, pkt, end, ctx);
	_pkt_f32_write(&data->g, pkt, end, ctx);
	_pkt_f32_write(&data->b, pkt, end, ctx);
}
static void _pkt_ColorSchemeNetSerializable_read(struct ColorSchemeNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ColorNoAlphaSerializable_read(&data->saberAColor, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_read(&data->saberBColor, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_read(&data->obstaclesColor, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor0, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor1, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor0Boost, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor1Boost, pkt, end, ctx);
}
static void _pkt_ColorSchemeNetSerializable_write(const struct ColorSchemeNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ColorNoAlphaSerializable_write(&data->saberAColor, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_write(&data->saberBColor, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_write(&data->obstaclesColor, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor0, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor1, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor0Boost, pkt, end, ctx);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor1Boost, pkt, end, ctx);
}
static void _pkt_PlayerSpecificSettingsNetSerializable_read(struct PlayerSpecificSettingsNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_String_read(&data->userName, pkt, end, ctx);
	_pkt_b_read(&data->leftHanded, pkt, end, ctx);
	_pkt_b_read(&data->automaticPlayerHeight, pkt, end, ctx);
	_pkt_f32_read(&data->playerHeight, pkt, end, ctx);
	_pkt_f32_read(&data->headPosToPlayerHeightOffset, pkt, end, ctx);
	_pkt_ColorSchemeNetSerializable_read(&data->colorScheme, pkt, end, ctx);
}
static void _pkt_PlayerSpecificSettingsNetSerializable_write(const struct PlayerSpecificSettingsNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_String_write(&data->userName, pkt, end, ctx);
	_pkt_b_write(&data->leftHanded, pkt, end, ctx);
	_pkt_b_write(&data->automaticPlayerHeight, pkt, end, ctx);
	_pkt_f32_write(&data->playerHeight, pkt, end, ctx);
	_pkt_f32_write(&data->headPosToPlayerHeightOffset, pkt, end, ctx);
	_pkt_ColorSchemeNetSerializable_write(&data->colorScheme, pkt, end, ctx);
}
static void _pkt_PlayerSpecificSettingsAtStartNetSerializable_read(struct PlayerSpecificSettingsAtStartNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_i32_read(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "PlayerSpecificSettingsAtStartNetSerializable.activePlayerSpecificSettingsAtGameStart"); i < count; ++i)
		_pkt_PlayerSpecificSettingsNetSerializable_read(&data->activePlayerSpecificSettingsAtGameStart[i], pkt, end, ctx);
}
static void _pkt_PlayerSpecificSettingsAtStartNetSerializable_write(const struct PlayerSpecificSettingsAtStartNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_i32_write(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "PlayerSpecificSettingsAtStartNetSerializable.activePlayerSpecificSettingsAtGameStart"); i < count; ++i)
		_pkt_PlayerSpecificSettingsNetSerializable_write(&data->activePlayerSpecificSettingsAtGameStart[i], pkt, end, ctx);
}
static void _pkt_SetGameplaySceneSyncFinish_read(struct SetGameplaySceneSyncFinish *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettingsAtStartNetSerializable_read(&data->playersAtGameStart, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_String_read(&data->sessionGameId, pkt, end, ctx);
	}
}
static void _pkt_SetGameplaySceneSyncFinish_write(const struct SetGameplaySceneSyncFinish *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettingsAtStartNetSerializable_write(&data->playersAtGameStart, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_String_write(&data->sessionGameId, pkt, end, ctx);
	}
}
static void _pkt_SetGameplaySceneReady_read(struct SetGameplaySceneReady *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettingsNetSerializable_read(&data->playerSpecificSettingsNetSerializable, pkt, end, ctx);
	}
}
static void _pkt_SetGameplaySceneReady_write(const struct SetGameplaySceneReady *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettingsNetSerializable_write(&data->playerSpecificSettingsNetSerializable, pkt, end, ctx);
	}
}
static void _pkt_GetGameplaySceneReady_read(struct GetGameplaySceneReady *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetGameplaySceneReady_write(const struct GetGameplaySceneReady *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetActivePlayerFailedToConnect_read(struct SetActivePlayerFailedToConnect *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_String_read(&data->failedUserId, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_PlayerSpecificSettingsAtStartNetSerializable_read(&data->playersAtGameStartNetSerializable, pkt, end, ctx);
	}
	if(data->flags.hasValue2) {
		_pkt_String_read(&data->sessionGameId, pkt, end, ctx);
	}
}
static void _pkt_SetActivePlayerFailedToConnect_write(const struct SetActivePlayerFailedToConnect *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_String_write(&data->failedUserId, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_PlayerSpecificSettingsAtStartNetSerializable_write(&data->playersAtGameStartNetSerializable, pkt, end, ctx);
	}
	if(data->flags.hasValue2) {
		_pkt_String_write(&data->sessionGameId, pkt, end, ctx);
	}
}
static void _pkt_SetGameplaySongReady_read(struct SetGameplaySongReady *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_SetGameplaySongReady_write(const struct SetGameplaySongReady *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_GetGameplaySongReady_read(struct GetGameplaySongReady *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_GetGameplaySongReady_write(const struct GetGameplaySongReady *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_SetSongStartTime_read(struct SetSongStartTime *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->startTime, pkt, end, ctx);
	}
}
static void _pkt_SetSongStartTime_write(const struct SetSongStartTime *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->startTime, pkt, end, ctx);
	}
}
static void _pkt_Vector3Serializable_read(struct Vector3Serializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->x, pkt, end, ctx);
	_pkt_vi32_read(&data->y, pkt, end, ctx);
	_pkt_vi32_read(&data->z, pkt, end, ctx);
}
static void _pkt_Vector3Serializable_write(const struct Vector3Serializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->x, pkt, end, ctx);
	_pkt_vi32_write(&data->y, pkt, end, ctx);
	_pkt_vi32_write(&data->z, pkt, end, ctx);
}
static void _pkt_QuaternionSerializable_read(struct QuaternionSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->a, pkt, end, ctx);
	_pkt_vi32_read(&data->b, pkt, end, ctx);
	_pkt_vi32_read(&data->c, pkt, end, ctx);
}
static void _pkt_QuaternionSerializable_write(const struct QuaternionSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->a, pkt, end, ctx);
	_pkt_vi32_write(&data->b, pkt, end, ctx);
	_pkt_vi32_write(&data->c, pkt, end, ctx);
}
static void _pkt_NoteCutInfoNetSerializable_read(struct NoteCutInfoNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->cutWasOk = bitfield0 >> 0 & 1;
	_pkt_f32_read(&data->saberSpeed, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->saberDir, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->cutPoint, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->cutNormal, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->notePosition, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->noteScale, pkt, end, ctx);
	_pkt_QuaternionSerializable_read(&data->noteRotation, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_read(&data->gameplayType, pkt, end, ctx);
	}
	_pkt_vi32_read(&data->colorType, pkt, end, ctx);
	_pkt_vi32_read(&data->lineLayer, pkt, end, ctx);
	_pkt_vi32_read(&data->noteLineIndex, pkt, end, ctx);
	_pkt_f32_read(&data->noteTime, pkt, end, ctx);
	_pkt_f32_read(&data->timeToNextColorNote, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->moveVec, pkt, end, ctx);
}
static void _pkt_NoteCutInfoNetSerializable_write(const struct NoteCutInfoNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->cutWasOk & 1u) << 0;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
	_pkt_f32_write(&data->saberSpeed, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->saberDir, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->cutPoint, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->cutNormal, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->notePosition, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->noteScale, pkt, end, ctx);
	_pkt_QuaternionSerializable_write(&data->noteRotation, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_write(&data->gameplayType, pkt, end, ctx);
	}
	_pkt_vi32_write(&data->colorType, pkt, end, ctx);
	_pkt_vi32_write(&data->lineLayer, pkt, end, ctx);
	_pkt_vi32_write(&data->noteLineIndex, pkt, end, ctx);
	_pkt_f32_write(&data->noteTime, pkt, end, ctx);
	_pkt_f32_write(&data->timeToNextColorNote, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->moveVec, pkt, end, ctx);
}
static void _pkt_NoteCut_read(struct NoteCut *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteCutInfoNetSerializable_read(&data->noteCutInfo, pkt, end, ctx);
	}
}
static void _pkt_NoteCut_write(const struct NoteCut *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteCutInfoNetSerializable_write(&data->noteCutInfo, pkt, end, ctx);
	}
}
static void _pkt_NoteMissInfoNetSerializable_read(struct NoteMissInfoNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->colorType, pkt, end, ctx);
	_pkt_vi32_read(&data->lineLayer, pkt, end, ctx);
	_pkt_vi32_read(&data->noteLineIndex, pkt, end, ctx);
	_pkt_f32_read(&data->noteTime, pkt, end, ctx);
}
static void _pkt_NoteMissInfoNetSerializable_write(const struct NoteMissInfoNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->colorType, pkt, end, ctx);
	_pkt_vi32_write(&data->lineLayer, pkt, end, ctx);
	_pkt_vi32_write(&data->noteLineIndex, pkt, end, ctx);
	_pkt_f32_write(&data->noteTime, pkt, end, ctx);
}
static void _pkt_NoteMissed_read(struct NoteMissed *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteMissInfoNetSerializable_read(&data->noteMissInfo, pkt, end, ctx);
	}
}
static void _pkt_NoteMissed_write(const struct NoteMissed *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteMissInfoNetSerializable_write(&data->noteMissInfo, pkt, end, ctx);
	}
}
static void _pkt_LevelCompletionResults_read(struct LevelCompletionResults *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_GameplayModifiers_read(&data->gameplayModifiers, pkt, end, ctx);
	_pkt_vi32_read(&data->modifiedScore, pkt, end, ctx);
	_pkt_vi32_read(&data->multipliedScore, pkt, end, ctx);
	_pkt_vi32_read(&data->rank, pkt, end, ctx);
	_pkt_b_read(&data->fullCombo, pkt, end, ctx);
	_pkt_f32_read(&data->leftSaberMovementDistance, pkt, end, ctx);
	_pkt_f32_read(&data->rightSaberMovementDistance, pkt, end, ctx);
	_pkt_f32_read(&data->leftHandMovementDistance, pkt, end, ctx);
	_pkt_f32_read(&data->rightHandMovementDistance, pkt, end, ctx);
	if(ctx.protocolVersion < 8) {
		_pkt_f32_read(&data->songDuration, pkt, end, ctx);
	}
	_pkt_vi32_read(&data->levelEndStateType, pkt, end, ctx);
	_pkt_vi32_read(&data->levelEndAction, pkt, end, ctx);
	_pkt_f32_read(&data->energy, pkt, end, ctx);
	_pkt_vi32_read(&data->goodCutsCount, pkt, end, ctx);
	_pkt_vi32_read(&data->badCutsCount, pkt, end, ctx);
	_pkt_vi32_read(&data->missedCount, pkt, end, ctx);
	_pkt_vi32_read(&data->notGoodCount, pkt, end, ctx);
	_pkt_vi32_read(&data->okCount, pkt, end, ctx);
	if(ctx.protocolVersion < 8) {
		_pkt_vi32_read(&data->averageCutScore, pkt, end, ctx);
	}
	_pkt_vi32_read(&data->maxCutScore, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_read(&data->totalCutScore, pkt, end, ctx);
		_pkt_vi32_read(&data->goodCutsCountForNotesWithFullScoreScoringType, pkt, end, ctx);
		_pkt_i32_read(&data->averageCenterDistanceCutScoreForNotesWithFullScoreScoringType, pkt, end, ctx);
		_pkt_i32_read(&data->averageCutScoreForNotesWithFullScoreScoringType, pkt, end, ctx);
	}
	if(ctx.protocolVersion < 8) {
		_pkt_f32_read(&data->averageCutDistanceRawScore, pkt, end, ctx);
	}
	_pkt_vi32_read(&data->maxCombo, pkt, end, ctx);
	if(ctx.protocolVersion < 8) {
		_pkt_f32_read(&data->minDirDeviation, pkt, end, ctx);
		_pkt_f32_read(&data->maxDirDeviation, pkt, end, ctx);
		_pkt_f32_read(&data->averageDirDeviation, pkt, end, ctx);
		_pkt_f32_read(&data->minTimeDeviation, pkt, end, ctx);
		_pkt_f32_read(&data->maxTimeDeviation, pkt, end, ctx);
		_pkt_f32_read(&data->averageTimeDeviation, pkt, end, ctx);
	}
	_pkt_f32_read(&data->endSongTime, pkt, end, ctx);
}
static void _pkt_LevelCompletionResults_write(const struct LevelCompletionResults *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_GameplayModifiers_write(&data->gameplayModifiers, pkt, end, ctx);
	_pkt_vi32_write(&data->modifiedScore, pkt, end, ctx);
	_pkt_vi32_write(&data->multipliedScore, pkt, end, ctx);
	_pkt_vi32_write(&data->rank, pkt, end, ctx);
	_pkt_b_write(&data->fullCombo, pkt, end, ctx);
	_pkt_f32_write(&data->leftSaberMovementDistance, pkt, end, ctx);
	_pkt_f32_write(&data->rightSaberMovementDistance, pkt, end, ctx);
	_pkt_f32_write(&data->leftHandMovementDistance, pkt, end, ctx);
	_pkt_f32_write(&data->rightHandMovementDistance, pkt, end, ctx);
	if(ctx.protocolVersion < 8) {
		_pkt_f32_write(&data->songDuration, pkt, end, ctx);
	}
	_pkt_vi32_write(&data->levelEndStateType, pkt, end, ctx);
	_pkt_vi32_write(&data->levelEndAction, pkt, end, ctx);
	_pkt_f32_write(&data->energy, pkt, end, ctx);
	_pkt_vi32_write(&data->goodCutsCount, pkt, end, ctx);
	_pkt_vi32_write(&data->badCutsCount, pkt, end, ctx);
	_pkt_vi32_write(&data->missedCount, pkt, end, ctx);
	_pkt_vi32_write(&data->notGoodCount, pkt, end, ctx);
	_pkt_vi32_write(&data->okCount, pkt, end, ctx);
	if(ctx.protocolVersion < 8) {
		_pkt_vi32_write(&data->averageCutScore, pkt, end, ctx);
	}
	_pkt_vi32_write(&data->maxCutScore, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_write(&data->totalCutScore, pkt, end, ctx);
		_pkt_vi32_write(&data->goodCutsCountForNotesWithFullScoreScoringType, pkt, end, ctx);
		_pkt_i32_write(&data->averageCenterDistanceCutScoreForNotesWithFullScoreScoringType, pkt, end, ctx);
		_pkt_i32_write(&data->averageCutScoreForNotesWithFullScoreScoringType, pkt, end, ctx);
	}
	if(ctx.protocolVersion < 8) {
		_pkt_f32_write(&data->averageCutDistanceRawScore, pkt, end, ctx);
	}
	_pkt_vi32_write(&data->maxCombo, pkt, end, ctx);
	if(ctx.protocolVersion < 8) {
		_pkt_f32_write(&data->minDirDeviation, pkt, end, ctx);
		_pkt_f32_write(&data->maxDirDeviation, pkt, end, ctx);
		_pkt_f32_write(&data->averageDirDeviation, pkt, end, ctx);
		_pkt_f32_write(&data->minTimeDeviation, pkt, end, ctx);
		_pkt_f32_write(&data->maxTimeDeviation, pkt, end, ctx);
		_pkt_f32_write(&data->averageTimeDeviation, pkt, end, ctx);
	}
	_pkt_f32_write(&data->endSongTime, pkt, end, ctx);
}
static void _pkt_MultiplayerLevelCompletionResults_read(struct MultiplayerLevelCompletionResults *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	if(ctx.protocolVersion < 7) {
		_pkt_vi32_read(&data->levelEndState, pkt, end, ctx);
	}
	if(ctx.protocolVersion >= 7) {
		_pkt_vi32_read(&data->playerLevelEndState, pkt, end, ctx);
		_pkt_vi32_read(&data->playerLevelEndReason, pkt, end, ctx);
	}
	if((ctx.protocolVersion < 7 && data->levelEndState < MultiplayerLevelEndState_GivenUp) || (ctx.protocolVersion >= 7 && data->playerLevelEndState != MultiplayerPlayerLevelEndState_NotStarted)) {
		_pkt_LevelCompletionResults_read(&data->levelCompletionResults, pkt, end, ctx);
	}
}
static void _pkt_MultiplayerLevelCompletionResults_write(const struct MultiplayerLevelCompletionResults *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	if(ctx.protocolVersion < 7) {
		_pkt_vi32_write(&data->levelEndState, pkt, end, ctx);
	}
	if(ctx.protocolVersion >= 7) {
		_pkt_vi32_write(&data->playerLevelEndState, pkt, end, ctx);
		_pkt_vi32_write(&data->playerLevelEndReason, pkt, end, ctx);
	}
	if((ctx.protocolVersion < 7 && data->levelEndState < MultiplayerLevelEndState_GivenUp) || (ctx.protocolVersion >= 7 && data->playerLevelEndState != MultiplayerPlayerLevelEndState_NotStarted)) {
		_pkt_LevelCompletionResults_write(&data->levelCompletionResults, pkt, end, ctx);
	}
}
static void _pkt_LevelFinished_read(struct LevelFinished *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_MultiplayerLevelCompletionResults_read(&data->results, pkt, end, ctx);
	}
}
static void _pkt_LevelFinished_write(const struct LevelFinished *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_MultiplayerLevelCompletionResults_write(&data->results, pkt, end, ctx);
	}
}
static void _pkt_ReturnToMenu_read(struct ReturnToMenu *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_ReturnToMenu_write(const struct ReturnToMenu *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_RequestReturnToMenu_read(struct RequestReturnToMenu *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
}
static void _pkt_RequestReturnToMenu_write(const struct RequestReturnToMenu *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
}
static void _pkt_NoteSpawnInfoNetSerializable_read(struct NoteSpawnInfoNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->time, pkt, end, ctx);
	_pkt_vi32_read(&data->lineIndex, pkt, end, ctx);
	_pkt_vi32_read(&data->noteLineLayer, pkt, end, ctx);
	_pkt_vi32_read(&data->beforeJumpNoteLineLayer, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_read(&data->gameplayType, pkt, end, ctx);
		_pkt_vi32_read(&data->scoringType, pkt, end, ctx);
	}
	_pkt_vi32_read(&data->colorType, pkt, end, ctx);
	_pkt_vi32_read(&data->cutDirection, pkt, end, ctx);
	_pkt_f32_read(&data->timeToNextColorNote, pkt, end, ctx);
	_pkt_f32_read(&data->timeToPrevColorNote, pkt, end, ctx);
	_pkt_vi32_read(&data->flipLineIndex, pkt, end, ctx);
	_pkt_vi32_read(&data->flipYSide, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->moveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->moveEndPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->jumpEndPos, pkt, end, ctx);
	_pkt_f32_read(&data->jumpGravity, pkt, end, ctx);
	_pkt_f32_read(&data->moveDuration, pkt, end, ctx);
	_pkt_f32_read(&data->jumpDuration, pkt, end, ctx);
	_pkt_f32_read(&data->rotation, pkt, end, ctx);
	_pkt_f32_read(&data->cutDirectionAngleOffset, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_f32_read(&data->cutSfxVolumeMultiplier, pkt, end, ctx);
	}
}
static void _pkt_NoteSpawnInfoNetSerializable_write(const struct NoteSpawnInfoNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->time, pkt, end, ctx);
	_pkt_vi32_write(&data->lineIndex, pkt, end, ctx);
	_pkt_vi32_write(&data->noteLineLayer, pkt, end, ctx);
	_pkt_vi32_write(&data->beforeJumpNoteLineLayer, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_write(&data->gameplayType, pkt, end, ctx);
		_pkt_vi32_write(&data->scoringType, pkt, end, ctx);
	}
	_pkt_vi32_write(&data->colorType, pkt, end, ctx);
	_pkt_vi32_write(&data->cutDirection, pkt, end, ctx);
	_pkt_f32_write(&data->timeToNextColorNote, pkt, end, ctx);
	_pkt_f32_write(&data->timeToPrevColorNote, pkt, end, ctx);
	_pkt_vi32_write(&data->flipLineIndex, pkt, end, ctx);
	_pkt_vi32_write(&data->flipYSide, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->moveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->moveEndPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->jumpEndPos, pkt, end, ctx);
	_pkt_f32_write(&data->jumpGravity, pkt, end, ctx);
	_pkt_f32_write(&data->moveDuration, pkt, end, ctx);
	_pkt_f32_write(&data->jumpDuration, pkt, end, ctx);
	_pkt_f32_write(&data->rotation, pkt, end, ctx);
	_pkt_f32_write(&data->cutDirectionAngleOffset, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_f32_write(&data->cutSfxVolumeMultiplier, pkt, end, ctx);
	}
}
static void _pkt_NoteSpawned_read(struct NoteSpawned *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteSpawnInfoNetSerializable_read(&data->noteSpawnInfo, pkt, end, ctx);
	}
}
static void _pkt_NoteSpawned_write(const struct NoteSpawned *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteSpawnInfoNetSerializable_write(&data->noteSpawnInfo, pkt, end, ctx);
	}
}
static void _pkt_ObstacleSpawnInfoNetSerializable_read(struct ObstacleSpawnInfoNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->time, pkt, end, ctx);
	_pkt_vi32_read(&data->lineIndex, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_read(&data->lineLayer, pkt, end, ctx);
	}
	if(ctx.protocolVersion < 8) {
		_pkt_vi32_read(&data->obstacleType, pkt, end, ctx);
	}
	_pkt_f32_read(&data->duration, pkt, end, ctx);
	_pkt_vi32_read(&data->width, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_read(&data->height, pkt, end, ctx);
	}
	_pkt_Vector3Serializable_read(&data->moveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->moveEndPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->jumpEndPos, pkt, end, ctx);
	_pkt_f32_read(&data->obstacleHeight, pkt, end, ctx);
	_pkt_f32_read(&data->moveDuration, pkt, end, ctx);
	_pkt_f32_read(&data->jumpDuration, pkt, end, ctx);
	_pkt_f32_read(&data->noteLinesDistance, pkt, end, ctx);
	_pkt_f32_read(&data->rotation, pkt, end, ctx);
}
static void _pkt_ObstacleSpawnInfoNetSerializable_write(const struct ObstacleSpawnInfoNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->time, pkt, end, ctx);
	_pkt_vi32_write(&data->lineIndex, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_write(&data->lineLayer, pkt, end, ctx);
	}
	if(ctx.protocolVersion < 8) {
		_pkt_vi32_write(&data->obstacleType, pkt, end, ctx);
	}
	_pkt_f32_write(&data->duration, pkt, end, ctx);
	_pkt_vi32_write(&data->width, pkt, end, ctx);
	if(ctx.protocolVersion >= 8) {
		_pkt_vi32_write(&data->height, pkt, end, ctx);
	}
	_pkt_Vector3Serializable_write(&data->moveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->moveEndPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->jumpEndPos, pkt, end, ctx);
	_pkt_f32_write(&data->obstacleHeight, pkt, end, ctx);
	_pkt_f32_write(&data->moveDuration, pkt, end, ctx);
	_pkt_f32_write(&data->jumpDuration, pkt, end, ctx);
	_pkt_f32_write(&data->noteLinesDistance, pkt, end, ctx);
	_pkt_f32_write(&data->rotation, pkt, end, ctx);
}
static void _pkt_ObstacleSpawned_read(struct ObstacleSpawned *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_ObstacleSpawnInfoNetSerializable_read(&data->obstacleSpawnInfo, pkt, end, ctx);
	}
}
static void _pkt_ObstacleSpawned_write(const struct ObstacleSpawned *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_ObstacleSpawnInfoNetSerializable_write(&data->obstacleSpawnInfo, pkt, end, ctx);
	}
}
static void _pkt_SliderSpawnInfoNetSerializable_read(struct SliderSpawnInfoNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->colorType, pkt, end, ctx);
	_pkt_vi32_read(&data->sliderType, pkt, end, ctx);
	_pkt_b_read(&data->hasHeadNote, pkt, end, ctx);
	_pkt_f32_read(&data->headTime, pkt, end, ctx);
	_pkt_vi32_read(&data->headLineIndex, pkt, end, ctx);
	_pkt_vi32_read(&data->headLineLayer, pkt, end, ctx);
	_pkt_vi32_read(&data->headBeforeJumpLineLayer, pkt, end, ctx);
	_pkt_f32_read(&data->headControlPointLengthMultiplier, pkt, end, ctx);
	_pkt_vi32_read(&data->headCutDirection, pkt, end, ctx);
	_pkt_f32_read(&data->headCutDirectionAngleOffset, pkt, end, ctx);
	_pkt_b_read(&data->hasTailNote, pkt, end, ctx);
	_pkt_f32_read(&data->tailTime, pkt, end, ctx);
	_pkt_vi32_read(&data->tailLineIndex, pkt, end, ctx);
	_pkt_vi32_read(&data->tailLineLayer, pkt, end, ctx);
	_pkt_vi32_read(&data->tailBeforeJumpLineLayer, pkt, end, ctx);
	_pkt_f32_read(&data->tailControlPointLengthMultiplier, pkt, end, ctx);
	_pkt_vi32_read(&data->tailCutDirection, pkt, end, ctx);
	_pkt_f32_read(&data->tailCutDirectionAngleOffset, pkt, end, ctx);
	_pkt_vi32_read(&data->midAnchorMode, pkt, end, ctx);
	_pkt_vi32_read(&data->sliceCount, pkt, end, ctx);
	_pkt_f32_read(&data->squishAmount, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->headMoveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->headJumpStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->headJumpEndPos, pkt, end, ctx);
	_pkt_f32_read(&data->headJumpGravity, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->tailMoveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->tailJumpStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_read(&data->tailJumpEndPos, pkt, end, ctx);
	_pkt_f32_read(&data->tailJumpGravity, pkt, end, ctx);
	_pkt_f32_read(&data->moveDuration, pkt, end, ctx);
	_pkt_f32_read(&data->jumpDuration, pkt, end, ctx);
	_pkt_f32_read(&data->rotation, pkt, end, ctx);
}
static void _pkt_SliderSpawnInfoNetSerializable_write(const struct SliderSpawnInfoNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->colorType, pkt, end, ctx);
	_pkt_vi32_write(&data->sliderType, pkt, end, ctx);
	_pkt_b_write(&data->hasHeadNote, pkt, end, ctx);
	_pkt_f32_write(&data->headTime, pkt, end, ctx);
	_pkt_vi32_write(&data->headLineIndex, pkt, end, ctx);
	_pkt_vi32_write(&data->headLineLayer, pkt, end, ctx);
	_pkt_vi32_write(&data->headBeforeJumpLineLayer, pkt, end, ctx);
	_pkt_f32_write(&data->headControlPointLengthMultiplier, pkt, end, ctx);
	_pkt_vi32_write(&data->headCutDirection, pkt, end, ctx);
	_pkt_f32_write(&data->headCutDirectionAngleOffset, pkt, end, ctx);
	_pkt_b_write(&data->hasTailNote, pkt, end, ctx);
	_pkt_f32_write(&data->tailTime, pkt, end, ctx);
	_pkt_vi32_write(&data->tailLineIndex, pkt, end, ctx);
	_pkt_vi32_write(&data->tailLineLayer, pkt, end, ctx);
	_pkt_vi32_write(&data->tailBeforeJumpLineLayer, pkt, end, ctx);
	_pkt_f32_write(&data->tailControlPointLengthMultiplier, pkt, end, ctx);
	_pkt_vi32_write(&data->tailCutDirection, pkt, end, ctx);
	_pkt_f32_write(&data->tailCutDirectionAngleOffset, pkt, end, ctx);
	_pkt_vi32_write(&data->midAnchorMode, pkt, end, ctx);
	_pkt_vi32_write(&data->sliceCount, pkt, end, ctx);
	_pkt_f32_write(&data->squishAmount, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->headMoveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->headJumpStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->headJumpEndPos, pkt, end, ctx);
	_pkt_f32_write(&data->headJumpGravity, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->tailMoveStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->tailJumpStartPos, pkt, end, ctx);
	_pkt_Vector3Serializable_write(&data->tailJumpEndPos, pkt, end, ctx);
	_pkt_f32_write(&data->tailJumpGravity, pkt, end, ctx);
	_pkt_f32_write(&data->moveDuration, pkt, end, ctx);
	_pkt_f32_write(&data->jumpDuration, pkt, end, ctx);
	_pkt_f32_write(&data->rotation, pkt, end, ctx);
}
static void _pkt_SliderSpawned_read(struct SliderSpawned *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_read(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_SliderSpawnInfoNetSerializable_read(&data->sliderSpawnInfo, pkt, end, ctx);
	}
}
static void _pkt_SliderSpawned_write(const struct SliderSpawned *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_RemoteProcedureCall_write(&data->base, pkt, end, ctx);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, pkt, end, ctx);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, pkt, end, ctx);
	}
	if(data->flags.hasValue1) {
		_pkt_SliderSpawnInfoNetSerializable_write(&data->sliderSpawnInfo, pkt, end, ctx);
	}
}
static void _pkt_GameplayRpc_read(struct GameplayRpc *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: _pkt_SetGameplaySceneSyncFinish_read(&data->setGameplaySceneSyncFinish, pkt, end, ctx); break;
		case GameplayRpcType_SetGameplaySceneReady: _pkt_SetGameplaySceneReady_read(&data->setGameplaySceneReady, pkt, end, ctx); break;
		case GameplayRpcType_GetGameplaySceneReady: _pkt_GetGameplaySceneReady_read(&data->getGameplaySceneReady, pkt, end, ctx); break;
		case GameplayRpcType_SetActivePlayerFailedToConnect: _pkt_SetActivePlayerFailedToConnect_read(&data->setActivePlayerFailedToConnect, pkt, end, ctx); break;
		case GameplayRpcType_SetGameplaySongReady: _pkt_SetGameplaySongReady_read(&data->setGameplaySongReady, pkt, end, ctx); break;
		case GameplayRpcType_GetGameplaySongReady: _pkt_GetGameplaySongReady_read(&data->getGameplaySongReady, pkt, end, ctx); break;
		case GameplayRpcType_SetSongStartTime: _pkt_SetSongStartTime_read(&data->setSongStartTime, pkt, end, ctx); break;
		case GameplayRpcType_NoteCut: _pkt_NoteCut_read(&data->noteCut, pkt, end, ctx); break;
		case GameplayRpcType_NoteMissed: _pkt_NoteMissed_read(&data->noteMissed, pkt, end, ctx); break;
		case GameplayRpcType_LevelFinished: _pkt_LevelFinished_read(&data->levelFinished, pkt, end, ctx); break;
		case GameplayRpcType_ReturnToMenu: _pkt_ReturnToMenu_read(&data->returnToMenu, pkt, end, ctx); break;
		case GameplayRpcType_RequestReturnToMenu: _pkt_RequestReturnToMenu_read(&data->requestReturnToMenu, pkt, end, ctx); break;
		case GameplayRpcType_NoteSpawned: _pkt_NoteSpawned_read(&data->noteSpawned, pkt, end, ctx); break;
		case GameplayRpcType_ObstacleSpawned: _pkt_ObstacleSpawned_read(&data->obstacleSpawned, pkt, end, ctx); break;
		case GameplayRpcType_SliderSpawned: _pkt_SliderSpawned_read(&data->sliderSpawned, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `GameplayRpcType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_GameplayRpc_write(const struct GameplayRpc *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: _pkt_SetGameplaySceneSyncFinish_write(&data->setGameplaySceneSyncFinish, pkt, end, ctx); break;
		case GameplayRpcType_SetGameplaySceneReady: _pkt_SetGameplaySceneReady_write(&data->setGameplaySceneReady, pkt, end, ctx); break;
		case GameplayRpcType_GetGameplaySceneReady: _pkt_GetGameplaySceneReady_write(&data->getGameplaySceneReady, pkt, end, ctx); break;
		case GameplayRpcType_SetActivePlayerFailedToConnect: _pkt_SetActivePlayerFailedToConnect_write(&data->setActivePlayerFailedToConnect, pkt, end, ctx); break;
		case GameplayRpcType_SetGameplaySongReady: _pkt_SetGameplaySongReady_write(&data->setGameplaySongReady, pkt, end, ctx); break;
		case GameplayRpcType_GetGameplaySongReady: _pkt_GetGameplaySongReady_write(&data->getGameplaySongReady, pkt, end, ctx); break;
		case GameplayRpcType_SetSongStartTime: _pkt_SetSongStartTime_write(&data->setSongStartTime, pkt, end, ctx); break;
		case GameplayRpcType_NoteCut: _pkt_NoteCut_write(&data->noteCut, pkt, end, ctx); break;
		case GameplayRpcType_NoteMissed: _pkt_NoteMissed_write(&data->noteMissed, pkt, end, ctx); break;
		case GameplayRpcType_LevelFinished: _pkt_LevelFinished_write(&data->levelFinished, pkt, end, ctx); break;
		case GameplayRpcType_ReturnToMenu: _pkt_ReturnToMenu_write(&data->returnToMenu, pkt, end, ctx); break;
		case GameplayRpcType_RequestReturnToMenu: _pkt_RequestReturnToMenu_write(&data->requestReturnToMenu, pkt, end, ctx); break;
		case GameplayRpcType_NoteSpawned: _pkt_NoteSpawned_write(&data->noteSpawned, pkt, end, ctx); break;
		case GameplayRpcType_ObstacleSpawned: _pkt_ObstacleSpawned_write(&data->obstacleSpawned, pkt, end, ctx); break;
		case GameplayRpcType_SliderSpawned: _pkt_SliderSpawned_write(&data->sliderSpawned, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `GameplayRpcType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_PoseSerializable_read(struct PoseSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_Vector3Serializable_read(&data->position, pkt, end, ctx);
	_pkt_QuaternionSerializable_read(&data->rotation, pkt, end, ctx);
}
static void _pkt_PoseSerializable_write(const struct PoseSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_Vector3Serializable_write(&data->position, pkt, end, ctx);
	_pkt_QuaternionSerializable_write(&data->rotation, pkt, end, ctx);
}
static void _pkt_NodePoseSyncState1_read(struct NodePoseSyncState1 *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PoseSerializable_read(&data->head, pkt, end, ctx);
	_pkt_PoseSerializable_read(&data->leftController, pkt, end, ctx);
	_pkt_PoseSerializable_read(&data->rightController, pkt, end, ctx);
}
static void _pkt_NodePoseSyncState1_write(const struct NodePoseSyncState1 *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PoseSerializable_write(&data->head, pkt, end, ctx);
	_pkt_PoseSerializable_write(&data->leftController, pkt, end, ctx);
	_pkt_PoseSerializable_write(&data->rightController, pkt, end, ctx);
}
static void _pkt_SyncStateId_read(struct SyncStateId *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->id = bitfield0 >> 0 & 127;
	data->same = bitfield0 >> 7 & 1;
}
static void _pkt_SyncStateId_write(const struct SyncStateId *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->id & 127u) << 0;
	bitfield0 |= (data->same & 1u) << 7;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
}
static void _pkt_NodePoseSyncState_read(struct NodePoseSyncState *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_read(&data->id, pkt, end, ctx);
	_pkt_f32_read(&data->time, pkt, end, ctx);
	_pkt_NodePoseSyncState1_read(&data->state, pkt, end, ctx);
}
static void _pkt_NodePoseSyncState_write(const struct NodePoseSyncState *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_write(&data->id, pkt, end, ctx);
	_pkt_f32_write(&data->time, pkt, end, ctx);
	_pkt_NodePoseSyncState1_write(&data->state, pkt, end, ctx);
}
static void _pkt_StandardScoreSyncState_read(struct StandardScoreSyncState *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->modifiedScore, pkt, end, ctx);
	_pkt_vi32_read(&data->rawScore, pkt, end, ctx);
	_pkt_vi32_read(&data->immediateMaxPossibleRawScore, pkt, end, ctx);
	_pkt_vi32_read(&data->combo, pkt, end, ctx);
	_pkt_vi32_read(&data->multiplier, pkt, end, ctx);
}
static void _pkt_StandardScoreSyncState_write(const struct StandardScoreSyncState *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->modifiedScore, pkt, end, ctx);
	_pkt_vi32_write(&data->rawScore, pkt, end, ctx);
	_pkt_vi32_write(&data->immediateMaxPossibleRawScore, pkt, end, ctx);
	_pkt_vi32_write(&data->combo, pkt, end, ctx);
	_pkt_vi32_write(&data->multiplier, pkt, end, ctx);
}
static void _pkt_ScoreSyncState_read(struct ScoreSyncState *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_read(&data->id, pkt, end, ctx);
	_pkt_f32_read(&data->time, pkt, end, ctx);
	_pkt_StandardScoreSyncState_read(&data->state, pkt, end, ctx);
}
static void _pkt_ScoreSyncState_write(const struct ScoreSyncState *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_write(&data->id, pkt, end, ctx);
	_pkt_f32_write(&data->time, pkt, end, ctx);
	_pkt_StandardScoreSyncState_write(&data->state, pkt, end, ctx);
}
static void _pkt_NodePoseSyncStateDelta_read(struct NodePoseSyncStateDelta *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_read(&data->baseId, pkt, end, ctx);
	_pkt_vi32_read(&data->timeOffsetMs, pkt, end, ctx);
	if(data->baseId.same == 0) {
		_pkt_NodePoseSyncState1_read(&data->delta, pkt, end, ctx);
	}
}
static void _pkt_NodePoseSyncStateDelta_write(const struct NodePoseSyncStateDelta *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_write(&data->baseId, pkt, end, ctx);
	_pkt_vi32_write(&data->timeOffsetMs, pkt, end, ctx);
	if(data->baseId.same == 0) {
		_pkt_NodePoseSyncState1_write(&data->delta, pkt, end, ctx);
	}
}
static void _pkt_ScoreSyncStateDelta_read(struct ScoreSyncStateDelta *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_read(&data->baseId, pkt, end, ctx);
	_pkt_vi32_read(&data->timeOffsetMs, pkt, end, ctx);
	if(data->baseId.same == 0) {
		_pkt_StandardScoreSyncState_read(&data->delta, pkt, end, ctx);
	}
}
static void _pkt_ScoreSyncStateDelta_write(const struct ScoreSyncStateDelta *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_SyncStateId_write(&data->baseId, pkt, end, ctx);
	_pkt_vi32_write(&data->timeOffsetMs, pkt, end, ctx);
	if(data->baseId.same == 0) {
		_pkt_StandardScoreSyncState_write(&data->delta, pkt, end, ctx);
	}
}
static void _pkt_MpBeatmapPacket_read(struct MpBeatmapPacket *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->levelHash, pkt, end, ctx);
	_pkt_LongString_read(&data->songName, pkt, end, ctx);
	_pkt_LongString_read(&data->songSubName, pkt, end, ctx);
	_pkt_LongString_read(&data->songAuthorName, pkt, end, ctx);
	_pkt_LongString_read(&data->levelAuthorName, pkt, end, ctx);
	_pkt_f32_read(&data->beatsPerMinute, pkt, end, ctx);
	_pkt_f32_read(&data->songDuration, pkt, end, ctx);
	_pkt_String_read(&data->characteristic, pkt, end, ctx);
	_pkt_u32_read(&data->difficulty, pkt, end, ctx);
}
static void _pkt_MpBeatmapPacket_write(const struct MpBeatmapPacket *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->levelHash, pkt, end, ctx);
	_pkt_LongString_write(&data->songName, pkt, end, ctx);
	_pkt_LongString_write(&data->songSubName, pkt, end, ctx);
	_pkt_LongString_write(&data->songAuthorName, pkt, end, ctx);
	_pkt_LongString_write(&data->levelAuthorName, pkt, end, ctx);
	_pkt_f32_write(&data->beatsPerMinute, pkt, end, ctx);
	_pkt_f32_write(&data->songDuration, pkt, end, ctx);
	_pkt_String_write(&data->characteristic, pkt, end, ctx);
	_pkt_u32_write(&data->difficulty, pkt, end, ctx);
}
static void _pkt_MpPlayerData_read(struct MpPlayerData *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->platformId, pkt, end, ctx);
	_pkt_i32_read(&data->platform, pkt, end, ctx);
}
static void _pkt_MpPlayerData_write(const struct MpPlayerData *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->platformId, pkt, end, ctx);
	_pkt_i32_write(&data->platform, pkt, end, ctx);
}
static void _pkt_CustomAvatarPacket_read(struct CustomAvatarPacket *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->hash, pkt, end, ctx);
	_pkt_f32_read(&data->scale, pkt, end, ctx);
	_pkt_f32_read(&data->floor, pkt, end, ctx);
}
static void _pkt_CustomAvatarPacket_write(const struct CustomAvatarPacket *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->hash, pkt, end, ctx);
	_pkt_f32_write(&data->scale, pkt, end, ctx);
	_pkt_f32_write(&data->floor, pkt, end, ctx);
}
static void _pkt_MpCore_read(struct MpCore *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->type, pkt, end, ctx);
	switch(MpCoreType_From(&data->type)) {
		case MpCoreType_MpBeatmapPacket: _pkt_MpBeatmapPacket_read(&data->mpBeatmapPacket, pkt, end, ctx); break;
		case MpCoreType_MpPlayerData: _pkt_MpPlayerData_read(&data->mpPlayerData, pkt, end, ctx); break;
		case MpCoreType_CustomAvatarPacket: _pkt_CustomAvatarPacket_read(&data->customAvatarPacket, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `MpCoreType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_MpCore_write(const struct MpCore *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->type, pkt, end, ctx);
	switch(MpCoreType_From(&data->type)) {
		case MpCoreType_MpBeatmapPacket: _pkt_MpBeatmapPacket_write(&data->mpBeatmapPacket, pkt, end, ctx); break;
		case MpCoreType_MpPlayerData: _pkt_MpPlayerData_write(&data->mpPlayerData, pkt, end, ctx); break;
		case MpCoreType_CustomAvatarPacket: _pkt_CustomAvatarPacket_write(&data->customAvatarPacket, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `MpCoreType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_PreviewDifficultyBeatmapSet_read(struct PreviewDifficultyBeatmapSet *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->beatmapCharacteristicSerializedName, pkt, end, ctx);
	_pkt_u8_read(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 5, "PreviewDifficultyBeatmapSet.difficulties"); i < count; ++i)
		_pkt_vu32_read(&data->difficulties[i], pkt, end, ctx);
}
static void _pkt_PreviewDifficultyBeatmapSet_write(const struct PreviewDifficultyBeatmapSet *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->beatmapCharacteristicSerializedName, pkt, end, ctx);
	_pkt_u8_write(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 5, "PreviewDifficultyBeatmapSet.difficulties"); i < count; ++i)
		_pkt_vu32_write(&data->difficulties[i], pkt, end, ctx);
}
static void _pkt_NetworkPreviewBeatmapLevel_read(struct NetworkPreviewBeatmapLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_read(&data->levelId, pkt, end, ctx);
	_pkt_LongString_read(&data->songName, pkt, end, ctx);
	_pkt_LongString_read(&data->songSubName, pkt, end, ctx);
	_pkt_LongString_read(&data->songAuthorName, pkt, end, ctx);
	_pkt_LongString_read(&data->levelAuthorName, pkt, end, ctx);
	_pkt_f32_read(&data->beatsPerMinute, pkt, end, ctx);
	_pkt_f32_read(&data->songTimeOffset, pkt, end, ctx);
	_pkt_f32_read(&data->shuffle, pkt, end, ctx);
	_pkt_f32_read(&data->shufflePeriod, pkt, end, ctx);
	_pkt_f32_read(&data->previewStartTime, pkt, end, ctx);
	_pkt_f32_read(&data->previewDuration, pkt, end, ctx);
	_pkt_f32_read(&data->songDuration, pkt, end, ctx);
	_pkt_u8_read(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 8, "NetworkPreviewBeatmapLevel.previewDifficultyBeatmapSets"); i < count; ++i)
		_pkt_PreviewDifficultyBeatmapSet_read(&data->previewDifficultyBeatmapSets[i], pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->cover, pkt, end, ctx);
}
static void _pkt_NetworkPreviewBeatmapLevel_write(const struct NetworkPreviewBeatmapLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_write(&data->levelId, pkt, end, ctx);
	_pkt_LongString_write(&data->songName, pkt, end, ctx);
	_pkt_LongString_write(&data->songSubName, pkt, end, ctx);
	_pkt_LongString_write(&data->songAuthorName, pkt, end, ctx);
	_pkt_LongString_write(&data->levelAuthorName, pkt, end, ctx);
	_pkt_f32_write(&data->beatsPerMinute, pkt, end, ctx);
	_pkt_f32_write(&data->songTimeOffset, pkt, end, ctx);
	_pkt_f32_write(&data->shuffle, pkt, end, ctx);
	_pkt_f32_write(&data->shufflePeriod, pkt, end, ctx);
	_pkt_f32_write(&data->previewStartTime, pkt, end, ctx);
	_pkt_f32_write(&data->previewDuration, pkt, end, ctx);
	_pkt_f32_write(&data->songDuration, pkt, end, ctx);
	_pkt_u8_write(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 8, "NetworkPreviewBeatmapLevel.previewDifficultyBeatmapSets"); i < count; ++i)
		_pkt_PreviewDifficultyBeatmapSet_write(&data->previewDifficultyBeatmapSets[i], pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->cover, pkt, end, ctx);
}
static void _pkt_RecommendPreview_read(struct RecommendPreview *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_NetworkPreviewBeatmapLevel_read(&data->base, pkt, end, ctx);
	_pkt_vu32_read(&data->requirements_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->requirements_len, 16, "RecommendPreview.requirements"); i < count; ++i)
		_pkt_String_read(&data->requirements[i], pkt, end, ctx);
	_pkt_vu32_read(&data->suggestions_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->suggestions_len, 16, "RecommendPreview.suggestions"); i < count; ++i)
		_pkt_String_read(&data->suggestions[i], pkt, end, ctx);
}
static void _pkt_RecommendPreview_write(const struct RecommendPreview *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_NetworkPreviewBeatmapLevel_write(&data->base, pkt, end, ctx);
	_pkt_vu32_write(&data->requirements_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->requirements_len, 16, "RecommendPreview.requirements"); i < count; ++i)
		_pkt_String_write(&data->requirements[i], pkt, end, ctx);
	_pkt_vu32_write(&data->suggestions_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->suggestions_len, 16, "RecommendPreview.suggestions"); i < count; ++i)
		_pkt_String_write(&data->suggestions[i], pkt, end, ctx);
}
static void _pkt_ShareInfo_read(struct ShareInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_read(&data->levelId, pkt, end, ctx);
	_pkt_raw_read(data->levelHash, pkt, end, ctx, 32);
	_pkt_vu64_read(&data->fileSize, pkt, end, ctx);
}
static void _pkt_ShareInfo_write(const struct ShareInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_write(&data->levelId, pkt, end, ctx);
	_pkt_raw_write(data->levelHash, pkt, end, ctx, 32);
	_pkt_vu64_write(&data->fileSize, pkt, end, ctx);
}
static void _pkt_SetCanShareBeatmap_read(struct SetCanShareBeatmap *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ShareInfo_read(&data->base, pkt, end, ctx);
	_pkt_b_read(&data->canShare, pkt, end, ctx);
}
static void _pkt_SetCanShareBeatmap_write(const struct SetCanShareBeatmap *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ShareInfo_write(&data->base, pkt, end, ctx);
	_pkt_b_write(&data->canShare, pkt, end, ctx);
}
static void _pkt_DirectDownloadInfo_read(struct DirectDownloadInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ShareInfo_read(&data->base, pkt, end, ctx);
	_pkt_u8_read(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "DirectDownloadInfo.sourcePlayers"); i < count; ++i)
		_pkt_String_read(&data->sourcePlayers[i], pkt, end, ctx);
}
static void _pkt_DirectDownloadInfo_write(const struct DirectDownloadInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ShareInfo_write(&data->base, pkt, end, ctx);
	_pkt_u8_write(&data->count, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->count, 128, "DirectDownloadInfo.sourcePlayers"); i < count; ++i)
		_pkt_String_write(&data->sourcePlayers[i], pkt, end, ctx);
}
static void _pkt_LevelFragmentRequest_read(struct LevelFragmentRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_read(&data->offset, pkt, end, ctx);
	_pkt_u16_read(&data->maxSize, pkt, end, ctx);
}
static void _pkt_LevelFragmentRequest_write(const struct LevelFragmentRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_write(&data->offset, pkt, end, ctx);
	_pkt_u16_write(&data->maxSize, pkt, end, ctx);
}
static void _pkt_LevelFragment_read(struct LevelFragment *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_read(&data->offset, pkt, end, ctx);
	_pkt_u16_read(&data->size, pkt, end, ctx);
	_pkt_raw_read(data->data, pkt, end, ctx, check_overflow(data->size, 1500, "LevelFragment.data"));
}
static void _pkt_LevelFragment_write(const struct LevelFragment *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_write(&data->offset, pkt, end, ctx);
	_pkt_u16_write(&data->size, pkt, end, ctx);
	_pkt_raw_write(data->data, pkt, end, ctx, check_overflow(data->size, 1500, "LevelFragment.data"));
}
static void _pkt_LoadProgress_read(struct LoadProgress *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->sequence, pkt, end, ctx);
	_pkt_u8_read(&data->state, pkt, end, ctx);
	_pkt_u16_read(&data->progress, pkt, end, ctx);
}
static void _pkt_LoadProgress_write(const struct LoadProgress *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->sequence, pkt, end, ctx);
	_pkt_u8_write(&data->state, pkt, end, ctx);
	_pkt_u16_write(&data->progress, pkt, end, ctx);
}
static void _pkt_BeatUpMessage_read(struct BeatUpMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case BeatUpMessageType_RecommendPreview: _pkt_RecommendPreview_read(&data->recommendPreview, pkt, end, ctx); break;
		case BeatUpMessageType_SetCanShareBeatmap: _pkt_SetCanShareBeatmap_read(&data->setCanShareBeatmap, pkt, end, ctx); break;
		case BeatUpMessageType_DirectDownloadInfo: _pkt_DirectDownloadInfo_read(&data->directDownloadInfo, pkt, end, ctx); break;
		case BeatUpMessageType_LevelFragmentRequest: _pkt_LevelFragmentRequest_read(&data->levelFragmentRequest, pkt, end, ctx); break;
		case BeatUpMessageType_LevelFragment: _pkt_LevelFragment_read(&data->levelFragment, pkt, end, ctx); break;
		case BeatUpMessageType_LoadProgress: _pkt_LoadProgress_read(&data->loadProgress, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `BeatUpMessageType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_BeatUpMessage_write(const struct BeatUpMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case BeatUpMessageType_RecommendPreview: _pkt_RecommendPreview_write(&data->recommendPreview, pkt, end, ctx); break;
		case BeatUpMessageType_SetCanShareBeatmap: _pkt_SetCanShareBeatmap_write(&data->setCanShareBeatmap, pkt, end, ctx); break;
		case BeatUpMessageType_DirectDownloadInfo: _pkt_DirectDownloadInfo_write(&data->directDownloadInfo, pkt, end, ctx); break;
		case BeatUpMessageType_LevelFragmentRequest: _pkt_LevelFragmentRequest_write(&data->levelFragmentRequest, pkt, end, ctx); break;
		case BeatUpMessageType_LevelFragment: _pkt_LevelFragment_write(&data->levelFragment, pkt, end, ctx); break;
		case BeatUpMessageType_LoadProgress: _pkt_LoadProgress_write(&data->loadProgress, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `BeatUpMessageType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_SyncTime_read(struct SyncTime *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->syncTime, pkt, end, ctx);
}
static void _pkt_SyncTime_write(const struct SyncTime *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->syncTime, pkt, end, ctx);
}
static void _pkt_PlayerConnected_read(struct PlayerConnected *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->remoteConnectionId, pkt, end, ctx);
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_ExString_read(&data->userName, pkt, end, ctx);
	_pkt_b_read(&data->isConnectionOwner, pkt, end, ctx);
}
static void _pkt_PlayerConnected_write(const struct PlayerConnected *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->remoteConnectionId, pkt, end, ctx);
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_ExString_write(&data->userName, pkt, end, ctx);
	_pkt_b_write(&data->isConnectionOwner, pkt, end, ctx);
}
static void _pkt_PlayerStateHash_read(struct PlayerStateHash *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BitMask128_read(&data->bloomFilter, pkt, end, ctx);
}
static void _pkt_PlayerStateHash_write(const struct PlayerStateHash *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BitMask128_write(&data->bloomFilter, pkt, end, ctx);
}
static void _pkt_Color32_read(struct Color32 *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->r, pkt, end, ctx);
	_pkt_u8_read(&data->g, pkt, end, ctx);
	_pkt_u8_read(&data->b, pkt, end, ctx);
	_pkt_u8_read(&data->a, pkt, end, ctx);
}
static void _pkt_Color32_write(const struct Color32 *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->r, pkt, end, ctx);
	_pkt_u8_write(&data->g, pkt, end, ctx);
	_pkt_u8_write(&data->b, pkt, end, ctx);
	_pkt_u8_write(&data->a, pkt, end, ctx);
}
static void _pkt_MultiplayerAvatarData_read(struct MultiplayerAvatarData *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->headTopId, pkt, end, ctx);
	_pkt_Color32_read(&data->headTopPrimaryColor, pkt, end, ctx);
	_pkt_Color32_read(&data->handsColor, pkt, end, ctx);
	_pkt_String_read(&data->clothesId, pkt, end, ctx);
	_pkt_Color32_read(&data->clothesPrimaryColor, pkt, end, ctx);
	_pkt_Color32_read(&data->clothesSecondaryColor, pkt, end, ctx);
	_pkt_Color32_read(&data->clothesDetailColor, pkt, end, ctx);
	for(uint32_t i = 0, count = 2; i < count; ++i)
		_pkt_Color32_read(&data->_unused[i], pkt, end, ctx);
	_pkt_String_read(&data->eyesId, pkt, end, ctx);
	_pkt_String_read(&data->mouthId, pkt, end, ctx);
	_pkt_Color32_read(&data->glassesColor, pkt, end, ctx);
	_pkt_Color32_read(&data->facialHairColor, pkt, end, ctx);
	_pkt_Color32_read(&data->headTopSecondaryColor, pkt, end, ctx);
	_pkt_String_read(&data->glassesId, pkt, end, ctx);
	_pkt_String_read(&data->facialHairId, pkt, end, ctx);
	_pkt_String_read(&data->handsId, pkt, end, ctx);
}
static void _pkt_MultiplayerAvatarData_write(const struct MultiplayerAvatarData *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->headTopId, pkt, end, ctx);
	_pkt_Color32_write(&data->headTopPrimaryColor, pkt, end, ctx);
	_pkt_Color32_write(&data->handsColor, pkt, end, ctx);
	_pkt_String_write(&data->clothesId, pkt, end, ctx);
	_pkt_Color32_write(&data->clothesPrimaryColor, pkt, end, ctx);
	_pkt_Color32_write(&data->clothesSecondaryColor, pkt, end, ctx);
	_pkt_Color32_write(&data->clothesDetailColor, pkt, end, ctx);
	for(uint32_t i = 0, count = 2; i < count; ++i)
		_pkt_Color32_write(&data->_unused[i], pkt, end, ctx);
	_pkt_String_write(&data->eyesId, pkt, end, ctx);
	_pkt_String_write(&data->mouthId, pkt, end, ctx);
	_pkt_Color32_write(&data->glassesColor, pkt, end, ctx);
	_pkt_Color32_write(&data->facialHairColor, pkt, end, ctx);
	_pkt_Color32_write(&data->headTopSecondaryColor, pkt, end, ctx);
	_pkt_String_write(&data->glassesId, pkt, end, ctx);
	_pkt_String_write(&data->facialHairId, pkt, end, ctx);
	_pkt_String_write(&data->handsId, pkt, end, ctx);
}
static void _pkt_PlayerIdentity_read(struct PlayerIdentity *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PlayerStateHash_read(&data->playerState, pkt, end, ctx);
	_pkt_MultiplayerAvatarData_read(&data->playerAvatar, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->random, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->publicEncryptionKey, pkt, end, ctx);
}
static void _pkt_PlayerIdentity_write(const struct PlayerIdentity *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PlayerStateHash_write(&data->playerState, pkt, end, ctx);
	_pkt_MultiplayerAvatarData_write(&data->playerAvatar, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->random, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->publicEncryptionKey, pkt, end, ctx);
}
static void _pkt_PlayerLatencyUpdate_read(struct PlayerLatencyUpdate *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->latency, pkt, end, ctx);
}
static void _pkt_PlayerLatencyUpdate_write(const struct PlayerLatencyUpdate *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->latency, pkt, end, ctx);
}
static void _pkt_PlayerDisconnected_read(struct PlayerDisconnected *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->disconnectedReason, pkt, end, ctx);
}
static void _pkt_PlayerDisconnected_write(const struct PlayerDisconnected *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->disconnectedReason, pkt, end, ctx);
}
static void _pkt_PlayerSortOrderUpdate_read(struct PlayerSortOrderUpdate *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_vi32_read(&data->sortIndex, pkt, end, ctx);
}
static void _pkt_PlayerSortOrderUpdate_write(const struct PlayerSortOrderUpdate *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_vi32_write(&data->sortIndex, pkt, end, ctx);
}
static void _pkt_Party_read(struct Party *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_Party_write(const struct Party *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_MultiplayerSession_read(struct MultiplayerSession *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case MultiplayerSessionMessageType_MenuRpc: _pkt_MenuRpc_read(&data->menuRpc, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_GameplayRpc: _pkt_GameplayRpc_read(&data->gameplayRpc, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: _pkt_NodePoseSyncState_read(&data->nodePoseSyncState, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_ScoreSyncState: _pkt_ScoreSyncState_read(&data->scoreSyncState, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: _pkt_NodePoseSyncStateDelta_read(&data->nodePoseSyncStateDelta, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: _pkt_ScoreSyncStateDelta_read(&data->scoreSyncStateDelta, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_MpCore: _pkt_MpCore_read(&data->mpCore, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_BeatUpMessage: _pkt_BeatUpMessage_read(&data->beatUpMessage, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `MultiplayerSessionMessageType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_MultiplayerSession_write(const struct MultiplayerSession *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case MultiplayerSessionMessageType_MenuRpc: _pkt_MenuRpc_write(&data->menuRpc, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_GameplayRpc: _pkt_GameplayRpc_write(&data->gameplayRpc, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: _pkt_NodePoseSyncState_write(&data->nodePoseSyncState, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_ScoreSyncState: _pkt_ScoreSyncState_write(&data->scoreSyncState, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: _pkt_NodePoseSyncStateDelta_write(&data->nodePoseSyncStateDelta, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: _pkt_ScoreSyncStateDelta_write(&data->scoreSyncStateDelta, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_MpCore: _pkt_MpCore_write(&data->mpCore, pkt, end, ctx); break;
		case MultiplayerSessionMessageType_BeatUpMessage: _pkt_BeatUpMessage_write(&data->beatUpMessage, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `MultiplayerSessionMessageType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_KickPlayer_read(struct KickPlayer *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->disconnectedReason, pkt, end, ctx);
}
static void _pkt_KickPlayer_write(const struct KickPlayer *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->disconnectedReason, pkt, end, ctx);
}
static void _pkt_PlayerStateUpdate_read(struct PlayerStateUpdate *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PlayerStateHash_read(&data->playerState, pkt, end, ctx);
}
static void _pkt_PlayerStateUpdate_write(const struct PlayerStateUpdate *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PlayerStateHash_write(&data->playerState, pkt, end, ctx);
}
static void _pkt_PlayerAvatarUpdate_read(struct PlayerAvatarUpdate *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_MultiplayerAvatarData_read(&data->playerAvatar, pkt, end, ctx);
}
static void _pkt_PlayerAvatarUpdate_write(const struct PlayerAvatarUpdate *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_MultiplayerAvatarData_write(&data->playerAvatar, pkt, end, ctx);
}
static void _pkt_PingMessage_read(struct PingMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->pingTime, pkt, end, ctx);
}
static void _pkt_PingMessage_write(const struct PingMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->pingTime, pkt, end, ctx);
}
static void _pkt_PongMessage_read(struct PongMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_read(&data->pingTime, pkt, end, ctx);
}
static void _pkt_PongMessage_write(const struct PongMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_f32_write(&data->pingTime, pkt, end, ctx);
}
void _pkt_InternalMessage_read(struct InternalMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case InternalMessageType_SyncTime: _pkt_SyncTime_read(&data->syncTime, pkt, end, ctx); break;
		case InternalMessageType_PlayerConnected: _pkt_PlayerConnected_read(&data->playerConnected, pkt, end, ctx); break;
		case InternalMessageType_PlayerIdentity: _pkt_PlayerIdentity_read(&data->playerIdentity, pkt, end, ctx); break;
		case InternalMessageType_PlayerLatencyUpdate: _pkt_PlayerLatencyUpdate_read(&data->playerLatencyUpdate, pkt, end, ctx); break;
		case InternalMessageType_PlayerDisconnected: _pkt_PlayerDisconnected_read(&data->playerDisconnected, pkt, end, ctx); break;
		case InternalMessageType_PlayerSortOrderUpdate: _pkt_PlayerSortOrderUpdate_read(&data->playerSortOrderUpdate, pkt, end, ctx); break;
		case InternalMessageType_Party: _pkt_Party_read(&data->party, pkt, end, ctx); break;
		case InternalMessageType_MultiplayerSession: _pkt_MultiplayerSession_read(&data->multiplayerSession, pkt, end, ctx); break;
		case InternalMessageType_KickPlayer: _pkt_KickPlayer_read(&data->kickPlayer, pkt, end, ctx); break;
		case InternalMessageType_PlayerStateUpdate: _pkt_PlayerStateUpdate_read(&data->playerStateUpdate, pkt, end, ctx); break;
		case InternalMessageType_PlayerAvatarUpdate: _pkt_PlayerAvatarUpdate_read(&data->playerAvatarUpdate, pkt, end, ctx); break;
		case InternalMessageType_PingMessage: _pkt_PingMessage_read(&data->pingMessage, pkt, end, ctx); break;
		case InternalMessageType_PongMessage: _pkt_PongMessage_read(&data->pongMessage, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `InternalMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_InternalMessage_write(const struct InternalMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case InternalMessageType_SyncTime: _pkt_SyncTime_write(&data->syncTime, pkt, end, ctx); break;
		case InternalMessageType_PlayerConnected: _pkt_PlayerConnected_write(&data->playerConnected, pkt, end, ctx); break;
		case InternalMessageType_PlayerIdentity: _pkt_PlayerIdentity_write(&data->playerIdentity, pkt, end, ctx); break;
		case InternalMessageType_PlayerLatencyUpdate: _pkt_PlayerLatencyUpdate_write(&data->playerLatencyUpdate, pkt, end, ctx); break;
		case InternalMessageType_PlayerDisconnected: _pkt_PlayerDisconnected_write(&data->playerDisconnected, pkt, end, ctx); break;
		case InternalMessageType_PlayerSortOrderUpdate: _pkt_PlayerSortOrderUpdate_write(&data->playerSortOrderUpdate, pkt, end, ctx); break;
		case InternalMessageType_Party: _pkt_Party_write(&data->party, pkt, end, ctx); break;
		case InternalMessageType_MultiplayerSession: _pkt_MultiplayerSession_write(&data->multiplayerSession, pkt, end, ctx); break;
		case InternalMessageType_KickPlayer: _pkt_KickPlayer_write(&data->kickPlayer, pkt, end, ctx); break;
		case InternalMessageType_PlayerStateUpdate: _pkt_PlayerStateUpdate_write(&data->playerStateUpdate, pkt, end, ctx); break;
		case InternalMessageType_PlayerAvatarUpdate: _pkt_PlayerAvatarUpdate_write(&data->playerAvatarUpdate, pkt, end, ctx); break;
		case InternalMessageType_PingMessage: _pkt_PingMessage_write(&data->pingMessage, pkt, end, ctx); break;
		case InternalMessageType_PongMessage: _pkt_PongMessage_write(&data->pongMessage, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `InternalMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_RoutingHeader_read(struct RoutingHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->remoteConnectionId, pkt, end, ctx);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->connectionId = bitfield0 >> 0 & 127;
	data->encrypted = bitfield0 >> 7 & 1;
}
void _pkt_RoutingHeader_write(const struct RoutingHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->remoteConnectionId, pkt, end, ctx);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->connectionId & 127u) << 0;
	bitfield0 |= (data->encrypted & 1u) << 7;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
}
static void _pkt_BaseMasterServerReliableRequest_read(struct BaseMasterServerReliableRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->requestId, pkt, end, ctx);
}
static void _pkt_BaseMasterServerReliableRequest_write(const struct BaseMasterServerReliableRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->requestId, pkt, end, ctx);
}
void _pkt_MasterServerReliableRequestProxy_read(struct MasterServerReliableRequestProxy *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	_pkt_BaseMasterServerReliableRequest_read(&data->value, pkt, end, ctx);
}
static void _pkt_BaseMasterServerResponse_read(struct BaseMasterServerResponse *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->responseId, pkt, end, ctx);
}
static void _pkt_BaseMasterServerResponse_write(const struct BaseMasterServerResponse *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->responseId, pkt, end, ctx);
}
static void _pkt_BaseMasterServerReliableResponse_read(struct BaseMasterServerReliableResponse *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->requestId, pkt, end, ctx);
	_pkt_u32_read(&data->responseId, pkt, end, ctx);
}
static void _pkt_BaseMasterServerReliableResponse_write(const struct BaseMasterServerReliableResponse *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->requestId, pkt, end, ctx);
	_pkt_u32_write(&data->responseId, pkt, end, ctx);
}
static void _pkt_MessageReceivedAcknowledge_read(struct MessageReceivedAcknowledge *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerResponse_read(&data->base, pkt, end, ctx);
	_pkt_b_read(&data->messageHandled, pkt, end, ctx);
}
static void _pkt_MessageReceivedAcknowledge_write(const struct MessageReceivedAcknowledge *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerResponse_write(&data->base, pkt, end, ctx);
	_pkt_b_write(&data->messageHandled, pkt, end, ctx);
}
void _pkt_MessageReceivedAcknowledgeProxy_write(const struct MessageReceivedAcknowledgeProxy *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	_pkt_MessageReceivedAcknowledge_write(&data->value, pkt, end, ctx);
}
static void _pkt_AuthenticationToken_read(struct AuthenticationToken *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->platform, pkt, end, ctx);
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_String_read(&data->userName, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->sessionToken, pkt, end, ctx);
}
static void _pkt_AuthenticationToken_write(const struct AuthenticationToken *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->platform, pkt, end, ctx);
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_String_write(&data->userName, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->sessionToken, pkt, end, ctx);
}
static void _pkt_AuthenticateUserRequest_read(struct AuthenticateUserRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_AuthenticationToken_read(&data->authenticationToken, pkt, end, ctx);
}
static void _pkt_AuthenticateUserRequest_write(const struct AuthenticateUserRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_AuthenticationToken_write(&data->authenticationToken, pkt, end, ctx);
}
static void _pkt_AuthenticateUserResponse_read(struct AuthenticateUserResponse *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_u8_read(&data->result, pkt, end, ctx);
}
static void _pkt_AuthenticateUserResponse_write(const struct AuthenticateUserResponse *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_u8_write(&data->result, pkt, end, ctx);
}
static void _pkt_IPEndPoint_read(struct IPEndPoint *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->address, pkt, end, ctx);
	_pkt_u32_read(&data->port, pkt, end, ctx);
}
static void _pkt_IPEndPoint_write(const struct IPEndPoint *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->address, pkt, end, ctx);
	_pkt_u32_write(&data->port, pkt, end, ctx);
}
static void _pkt_GameplayServerConfiguration_read(struct GameplayServerConfiguration *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_read(&data->maxPlayerCount, pkt, end, ctx);
	_pkt_vi32_read(&data->discoveryPolicy, pkt, end, ctx);
	_pkt_vi32_read(&data->invitePolicy, pkt, end, ctx);
	_pkt_vi32_read(&data->gameplayServerMode, pkt, end, ctx);
	_pkt_vi32_read(&data->songSelectionMode, pkt, end, ctx);
	_pkt_vi32_read(&data->gameplayServerControlSettings, pkt, end, ctx);
}
static void _pkt_GameplayServerConfiguration_write(const struct GameplayServerConfiguration *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vi32_write(&data->maxPlayerCount, pkt, end, ctx);
	_pkt_vi32_write(&data->discoveryPolicy, pkt, end, ctx);
	_pkt_vi32_write(&data->invitePolicy, pkt, end, ctx);
	_pkt_vi32_write(&data->gameplayServerMode, pkt, end, ctx);
	_pkt_vi32_write(&data->songSelectionMode, pkt, end, ctx);
	_pkt_vi32_write(&data->gameplayServerControlSettings, pkt, end, ctx);
}
static void _pkt_ConnectToServerResponse_read(struct ConnectToServerResponse *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_u8_read(&data->result, pkt, end, ctx);
	if(data->result == ConnectToServerResponse_Result_Success) {
		_pkt_String_read(&data->userId, pkt, end, ctx);
		_pkt_String_read(&data->userName, pkt, end, ctx);
		_pkt_String_read(&data->secret, pkt, end, ctx);
		_pkt_BeatmapLevelSelectionMask_read(&data->selectionMask, pkt, end, ctx);
		_pkt_u8_read(&data->flags, pkt, end, ctx);
		_pkt_IPEndPoint_read(&data->remoteEndPoint, pkt, end, ctx);
		_pkt_raw_read(data->random, pkt, end, ctx, 32);
		_pkt_ByteArrayNetSerializable_read(&data->publicKey, pkt, end, ctx);
		_pkt_ServerCode_read(&data->code, pkt, end, ctx);
		_pkt_GameplayServerConfiguration_read(&data->configuration, pkt, end, ctx);
		_pkt_String_read(&data->managerId, pkt, end, ctx);
	}
}
static void _pkt_ConnectToServerResponse_write(const struct ConnectToServerResponse *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_u8_write(&data->result, pkt, end, ctx);
	if(data->result == ConnectToServerResponse_Result_Success) {
		_pkt_String_write(&data->userId, pkt, end, ctx);
		_pkt_String_write(&data->userName, pkt, end, ctx);
		_pkt_String_write(&data->secret, pkt, end, ctx);
		_pkt_BeatmapLevelSelectionMask_write(&data->selectionMask, pkt, end, ctx);
		_pkt_u8_write(&data->flags, pkt, end, ctx);
		_pkt_IPEndPoint_write(&data->remoteEndPoint, pkt, end, ctx);
		_pkt_raw_write(data->random, pkt, end, ctx, 32);
		_pkt_ByteArrayNetSerializable_write(&data->publicKey, pkt, end, ctx);
		_pkt_ServerCode_write(&data->code, pkt, end, ctx);
		_pkt_GameplayServerConfiguration_write(&data->configuration, pkt, end, ctx);
		_pkt_String_write(&data->managerId, pkt, end, ctx);
	}
}
static void _pkt_BaseConnectToServerRequest_read(struct BaseConnectToServerRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_read(&data->base, pkt, end, ctx);
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_String_read(&data->userName, pkt, end, ctx);
	_pkt_raw_read(data->random, pkt, end, ctx, 32);
	_pkt_ByteArrayNetSerializable_read(&data->publicKey, pkt, end, ctx);
}
static void _pkt_BaseConnectToServerRequest_write(const struct BaseConnectToServerRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_write(&data->base, pkt, end, ctx);
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_String_write(&data->userName, pkt, end, ctx);
	_pkt_raw_write(data->random, pkt, end, ctx, 32);
	_pkt_ByteArrayNetSerializable_write(&data->publicKey, pkt, end, ctx);
}
static void _pkt_ConnectToServerRequest_read(struct ConnectToServerRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseConnectToServerRequest_read(&data->base, pkt, end, ctx);
	_pkt_BeatmapLevelSelectionMask_read(&data->selectionMask, pkt, end, ctx);
	_pkt_String_read(&data->secret, pkt, end, ctx);
	_pkt_ServerCode_read(&data->code, pkt, end, ctx);
	_pkt_GameplayServerConfiguration_read(&data->configuration, pkt, end, ctx);
}
static void _pkt_ConnectToServerRequest_write(const struct ConnectToServerRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseConnectToServerRequest_write(&data->base, pkt, end, ctx);
	_pkt_BeatmapLevelSelectionMask_write(&data->selectionMask, pkt, end, ctx);
	_pkt_String_write(&data->secret, pkt, end, ctx);
	_pkt_ServerCode_write(&data->code, pkt, end, ctx);
	_pkt_GameplayServerConfiguration_write(&data->configuration, pkt, end, ctx);
}
static void _pkt_MultipartMessage_read(struct MultipartMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_read(&data->base, pkt, end, ctx);
	_pkt_u32_read(&data->multipartMessageId, pkt, end, ctx);
	_pkt_vu32_read(&data->offset, pkt, end, ctx);
	_pkt_vu32_read(&data->length, pkt, end, ctx);
	_pkt_vu32_read(&data->totalLength, pkt, end, ctx);
	_pkt_raw_read(data->data, pkt, end, ctx, check_overflow(data->length, 384, "MultipartMessage.data"));
}
static void _pkt_MultipartMessage_write(const struct MultipartMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_write(&data->base, pkt, end, ctx);
	_pkt_u32_write(&data->multipartMessageId, pkt, end, ctx);
	_pkt_vu32_write(&data->offset, pkt, end, ctx);
	_pkt_vu32_write(&data->length, pkt, end, ctx);
	_pkt_vu32_write(&data->totalLength, pkt, end, ctx);
	_pkt_raw_write(data->data, pkt, end, ctx, check_overflow(data->length, 384, "MultipartMessage.data"));
}
void _pkt_MultipartMessageProxy_write(const struct MultipartMessageProxy *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	_pkt_MultipartMessage_write(&data->value, pkt, end, ctx);
}
static void _pkt_SessionKeepaliveMessage_read(struct SessionKeepaliveMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_SessionKeepaliveMessage_write(const struct SessionKeepaliveMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_GetPublicServersRequest_read(struct GetPublicServersRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_read(&data->base, pkt, end, ctx);
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_String_read(&data->userName, pkt, end, ctx);
	_pkt_vi32_read(&data->offset, pkt, end, ctx);
	_pkt_vi32_read(&data->count, pkt, end, ctx);
	_pkt_BeatmapLevelSelectionMask_read(&data->selectionMask, pkt, end, ctx);
	_pkt_GameplayServerConfiguration_read(&data->configuration, pkt, end, ctx);
}
static void _pkt_GetPublicServersRequest_write(const struct GetPublicServersRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_write(&data->base, pkt, end, ctx);
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_String_write(&data->userName, pkt, end, ctx);
	_pkt_vi32_write(&data->offset, pkt, end, ctx);
	_pkt_vi32_write(&data->count, pkt, end, ctx);
	_pkt_BeatmapLevelSelectionMask_write(&data->selectionMask, pkt, end, ctx);
	_pkt_GameplayServerConfiguration_write(&data->configuration, pkt, end, ctx);
}
static void _pkt_PublicServerInfo_read(struct PublicServerInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ServerCode_read(&data->code, pkt, end, ctx);
	_pkt_vi32_read(&data->currentPlayerCount, pkt, end, ctx);
}
static void _pkt_PublicServerInfo_write(const struct PublicServerInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ServerCode_write(&data->code, pkt, end, ctx);
	_pkt_vi32_write(&data->currentPlayerCount, pkt, end, ctx);
}
static void _pkt_GetPublicServersResponse_read(struct GetPublicServersResponse *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_u8_read(&data->result, pkt, end, ctx);
	if(data->result == GetPublicServersResponse_Result_Success) {
		_pkt_vu32_read(&data->publicServerCount, pkt, end, ctx);
		for(uint32_t i = 0, count = check_overflow(data->publicServerCount, 8192, "GetPublicServersResponse.publicServers"); i < count; ++i)
			_pkt_PublicServerInfo_read(&data->publicServers[i], pkt, end, ctx);
	}
}
static void _pkt_GetPublicServersResponse_write(const struct GetPublicServersResponse *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_u8_write(&data->result, pkt, end, ctx);
	if(data->result == GetPublicServersResponse_Result_Success) {
		_pkt_vu32_write(&data->publicServerCount, pkt, end, ctx);
		for(uint32_t i = 0, count = check_overflow(data->publicServerCount, 8192, "GetPublicServersResponse.publicServers"); i < count; ++i)
			_pkt_PublicServerInfo_write(&data->publicServers[i], pkt, end, ctx);
	}
}
void _pkt_UserMessage_read(struct UserMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case UserMessageType_AuthenticateUserRequest: _pkt_AuthenticateUserRequest_read(&data->authenticateUserRequest, pkt, end, ctx); break;
		case UserMessageType_AuthenticateUserResponse: _pkt_AuthenticateUserResponse_read(&data->authenticateUserResponse, pkt, end, ctx); break;
		case UserMessageType_ConnectToServerResponse: _pkt_ConnectToServerResponse_read(&data->connectToServerResponse, pkt, end, ctx); break;
		case UserMessageType_ConnectToServerRequest: _pkt_ConnectToServerRequest_read(&data->connectToServerRequest, pkt, end, ctx); break;
		case UserMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_read(&data->messageReceivedAcknowledge, pkt, end, ctx); break;
		case UserMessageType_MultipartMessage: _pkt_MultipartMessage_read(&data->multipartMessage, pkt, end, ctx); break;
		case UserMessageType_SessionKeepaliveMessage: _pkt_SessionKeepaliveMessage_read(&data->sessionKeepaliveMessage, pkt, end, ctx); break;
		case UserMessageType_GetPublicServersRequest: _pkt_GetPublicServersRequest_read(&data->getPublicServersRequest, pkt, end, ctx); break;
		case UserMessageType_GetPublicServersResponse: _pkt_GetPublicServersResponse_read(&data->getPublicServersResponse, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `UserMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_UserMessage_write(const struct UserMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case UserMessageType_AuthenticateUserRequest: _pkt_AuthenticateUserRequest_write(&data->authenticateUserRequest, pkt, end, ctx); break;
		case UserMessageType_AuthenticateUserResponse: _pkt_AuthenticateUserResponse_write(&data->authenticateUserResponse, pkt, end, ctx); break;
		case UserMessageType_ConnectToServerResponse: _pkt_ConnectToServerResponse_write(&data->connectToServerResponse, pkt, end, ctx); break;
		case UserMessageType_ConnectToServerRequest: _pkt_ConnectToServerRequest_write(&data->connectToServerRequest, pkt, end, ctx); break;
		case UserMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_write(&data->messageReceivedAcknowledge, pkt, end, ctx); break;
		case UserMessageType_MultipartMessage: _pkt_MultipartMessage_write(&data->multipartMessage, pkt, end, ctx); break;
		case UserMessageType_SessionKeepaliveMessage: _pkt_SessionKeepaliveMessage_write(&data->sessionKeepaliveMessage, pkt, end, ctx); break;
		case UserMessageType_GetPublicServersRequest: _pkt_GetPublicServersRequest_write(&data->getPublicServersRequest, pkt, end, ctx); break;
		case UserMessageType_GetPublicServersResponse: _pkt_GetPublicServersResponse_write(&data->getPublicServersResponse, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `UserMessageType`\n"); longjmp(fail, 1);
	}
}
static void _pkt_ClientHelloRequest_read(struct ClientHelloRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_read(&data->base, pkt, end, ctx);
	_pkt_raw_read(data->random, pkt, end, ctx, 32);
}
static void _pkt_ClientHelloRequest_write(const struct ClientHelloRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_write(&data->base, pkt, end, ctx);
	_pkt_raw_write(data->random, pkt, end, ctx, 32);
}
static void _pkt_HelloVerifyRequest_read(struct HelloVerifyRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_raw_read(data->cookie, pkt, end, ctx, 32);
}
static void _pkt_HelloVerifyRequest_write(const struct HelloVerifyRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_raw_write(data->cookie, pkt, end, ctx, 32);
}
static void _pkt_ClientHelloWithCookieRequest_read(struct ClientHelloWithCookieRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_read(&data->base, pkt, end, ctx);
	_pkt_u32_read(&data->certificateResponseId, pkt, end, ctx);
	_pkt_raw_read(data->random, pkt, end, ctx, 32);
	_pkt_raw_read(data->cookie, pkt, end, ctx, 32);
}
static void _pkt_ClientHelloWithCookieRequest_write(const struct ClientHelloWithCookieRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableRequest_write(&data->base, pkt, end, ctx);
	_pkt_u32_write(&data->certificateResponseId, pkt, end, ctx);
	_pkt_raw_write(data->random, pkt, end, ctx, 32);
	_pkt_raw_write(data->cookie, pkt, end, ctx, 32);
}
static void _pkt_ServerHelloRequest_read(struct ServerHelloRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_raw_read(data->random, pkt, end, ctx, 32);
	_pkt_ByteArrayNetSerializable_read(&data->publicKey, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->signature, pkt, end, ctx);
}
static void _pkt_ServerHelloRequest_write(const struct ServerHelloRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_raw_write(data->random, pkt, end, ctx, 32);
	_pkt_ByteArrayNetSerializable_write(&data->publicKey, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->signature, pkt, end, ctx);
}
static void _pkt_ServerCertificateRequest_read(struct ServerCertificateRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_vu32_read(&data->certificateCount, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->certificateCount, 10, "ServerCertificateRequest.certificateList"); i < count; ++i)
		_pkt_ByteArrayNetSerializable_read(&data->certificateList[i], pkt, end, ctx);
}
static void _pkt_ServerCertificateRequest_write(const struct ServerCertificateRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_vu32_write(&data->certificateCount, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->certificateCount, 10, "ServerCertificateRequest.certificateList"); i < count; ++i)
		_pkt_ByteArrayNetSerializable_write(&data->certificateList[i], pkt, end, ctx);
}
static void _pkt_ClientKeyExchangeRequest_read(struct ClientKeyExchangeRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->clientPublicKey, pkt, end, ctx);
}
static void _pkt_ClientKeyExchangeRequest_write(const struct ClientKeyExchangeRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->clientPublicKey, pkt, end, ctx);
}
static void _pkt_ChangeCipherSpecRequest_read(struct ChangeCipherSpecRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_read(&data->base, pkt, end, ctx);
}
static void _pkt_ChangeCipherSpecRequest_write(const struct ChangeCipherSpecRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_BaseMasterServerReliableResponse_write(&data->base, pkt, end, ctx);
}
void _pkt_HandshakeMessage_read(struct HandshakeMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case HandshakeMessageType_ClientHelloRequest: _pkt_ClientHelloRequest_read(&data->clientHelloRequest, pkt, end, ctx); break;
		case HandshakeMessageType_HelloVerifyRequest: _pkt_HelloVerifyRequest_read(&data->helloVerifyRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ClientHelloWithCookieRequest: _pkt_ClientHelloWithCookieRequest_read(&data->clientHelloWithCookieRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ServerHelloRequest: _pkt_ServerHelloRequest_read(&data->serverHelloRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ServerCertificateRequest: _pkt_ServerCertificateRequest_read(&data->serverCertificateRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ClientKeyExchangeRequest: _pkt_ClientKeyExchangeRequest_read(&data->clientKeyExchangeRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ChangeCipherSpecRequest: _pkt_ChangeCipherSpecRequest_read(&data->changeCipherSpecRequest, pkt, end, ctx); break;
		case HandshakeMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_read(&data->messageReceivedAcknowledge, pkt, end, ctx); break;
		case HandshakeMessageType_MultipartMessage: _pkt_MultipartMessage_read(&data->multipartMessage, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `HandshakeMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_HandshakeMessage_write(const struct HandshakeMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case HandshakeMessageType_ClientHelloRequest: _pkt_ClientHelloRequest_write(&data->clientHelloRequest, pkt, end, ctx); break;
		case HandshakeMessageType_HelloVerifyRequest: _pkt_HelloVerifyRequest_write(&data->helloVerifyRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ClientHelloWithCookieRequest: _pkt_ClientHelloWithCookieRequest_write(&data->clientHelloWithCookieRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ServerHelloRequest: _pkt_ServerHelloRequest_write(&data->serverHelloRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ServerCertificateRequest: _pkt_ServerCertificateRequest_write(&data->serverCertificateRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ClientKeyExchangeRequest: _pkt_ClientKeyExchangeRequest_write(&data->clientKeyExchangeRequest, pkt, end, ctx); break;
		case HandshakeMessageType_ChangeCipherSpecRequest: _pkt_ChangeCipherSpecRequest_write(&data->changeCipherSpecRequest, pkt, end, ctx); break;
		case HandshakeMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_write(&data->messageReceivedAcknowledge, pkt, end, ctx); break;
		case HandshakeMessageType_MultipartMessage: _pkt_MultipartMessage_write(&data->multipartMessage, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `HandshakeMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_SerializeHeader_read(struct SerializeHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_read(&data->length, pkt, end, ctx);
}
void _pkt_SerializeHeader_write(const struct SerializeHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_write(&data->length, pkt, end, ctx);
}
void _pkt_FragmentedHeader_read(struct FragmentedHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->fragmentId, pkt, end, ctx);
	_pkt_u16_read(&data->fragmentPart, pkt, end, ctx);
	_pkt_u16_read(&data->fragmentsTotal, pkt, end, ctx);
}
void _pkt_FragmentedHeader_write(const struct FragmentedHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->fragmentId, pkt, end, ctx);
	_pkt_u16_write(&data->fragmentPart, pkt, end, ctx);
	_pkt_u16_write(&data->fragmentsTotal, pkt, end, ctx);
}
static void _pkt_Unreliable_read(struct Unreliable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_Unreliable_write(const struct Unreliable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_Channeled_read(struct Channeled *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->sequence, pkt, end, ctx);
	_pkt_u8_read(&data->channelId, pkt, end, ctx);
}
static void _pkt_Channeled_write(const struct Channeled *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->sequence, pkt, end, ctx);
	_pkt_u8_write(&data->channelId, pkt, end, ctx);
}
static void _pkt_Ack_read(struct Ack *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->sequence, pkt, end, ctx);
	_pkt_u8_read(&data->channelId, pkt, end, ctx);
	if(data->channelId % 2 == 0) {
		_pkt_raw_read(data->data, pkt, end, ctx, check_overflow(ctx.windowSize / 8, 16, "Ack.data"));
		_pkt_u8_read(&data->_pad0, pkt, end, ctx);
	}
}
static void _pkt_Ack_write(const struct Ack *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->sequence, pkt, end, ctx);
	_pkt_u8_write(&data->channelId, pkt, end, ctx);
	if(data->channelId % 2 == 0) {
		_pkt_raw_write(data->data, pkt, end, ctx, check_overflow(ctx.windowSize / 8, 16, "Ack.data"));
		_pkt_u8_write(&data->_pad0, pkt, end, ctx);
	}
}
static void _pkt_Ping_read(struct Ping *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->sequence, pkt, end, ctx);
}
static void _pkt_Ping_write(const struct Ping *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->sequence, pkt, end, ctx);
}
static void _pkt_Pong_read(struct Pong *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->sequence, pkt, end, ctx);
	_pkt_u64_read(&data->time, pkt, end, ctx);
}
static void _pkt_Pong_write(const struct Pong *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->sequence, pkt, end, ctx);
	_pkt_u64_write(&data->time, pkt, end, ctx);
}
void _pkt_BeatUpConnectInfo_read(struct BeatUpConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->protocolId, pkt, end, ctx);
	_pkt_u32_read(&data->windowSize, pkt, end, ctx);
	_pkt_u8_read(&data->countdownDuration, pkt, end, ctx);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->directDownloads = bitfield0 >> 0 & 1;
	data->skipResults = bitfield0 >> 1 & 1;
	data->perPlayerDifficulty = bitfield0 >> 2 & 1;
	data->perPlayerModifiers = bitfield0 >> 3 & 1;
}
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_read(&data->length, pkt, end, ctx);
	_pkt_String_read(&data->name, pkt, end, ctx);
}
static void _pkt_ConnectRequest_read(struct ConnectRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->protocolId, pkt, end, ctx);
	_pkt_u64_read(&data->connectTime, pkt, end, ctx);
	if((ctx.netVersion = data->protocolId) >= 12) {
		_pkt_i32_read(&data->peerId, pkt, end, ctx);
	}
	_pkt_u8_read(&data->addrlen, pkt, end, ctx);
	_pkt_raw_read(data->address, pkt, end, ctx, check_overflow(data->addrlen, 38, "ConnectRequest.address"));
	_pkt_String_read(&data->secret, pkt, end, ctx);
	_pkt_String_read(&data->userId, pkt, end, ctx);
	_pkt_String_read(&data->userName, pkt, end, ctx);
	_pkt_b_read(&data->isConnectionOwner, pkt, end, ctx);
}
static void _pkt_ConnectRequest_write(const struct ConnectRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->protocolId, pkt, end, ctx);
	_pkt_u64_write(&data->connectTime, pkt, end, ctx);
	if((ctx.netVersion = data->protocolId) >= 12) {
		_pkt_i32_write(&data->peerId, pkt, end, ctx);
	}
	_pkt_u8_write(&data->addrlen, pkt, end, ctx);
	_pkt_raw_write(data->address, pkt, end, ctx, check_overflow(data->addrlen, 38, "ConnectRequest.address"));
	_pkt_String_write(&data->secret, pkt, end, ctx);
	_pkt_String_write(&data->userId, pkt, end, ctx);
	_pkt_String_write(&data->userName, pkt, end, ctx);
	_pkt_b_write(&data->isConnectionOwner, pkt, end, ctx);
}
static void _pkt_ConnectAccept_read(struct ConnectAccept *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u64_read(&data->connectTime, pkt, end, ctx);
	_pkt_u8_read(&data->connectNum, pkt, end, ctx);
	_pkt_b_read(&data->reusedPeer, pkt, end, ctx);
	if(ctx.netVersion >= 12) {
		_pkt_i32_read(&data->peerId, pkt, end, ctx);
	}
	if(ctx.beatUpVersion) {
		_pkt_u32_read(&data->windowSize, pkt, end, ctx);
		_pkt_u8_read(&data->countdownDuration, pkt, end, ctx);
		uint8_t bitfield0;
		_pkt_u8_read(&bitfield0, pkt, end, ctx);
		data->directDownloads = bitfield0 >> 0 & 1;
		data->skipResults = bitfield0 >> 1 & 1;
		data->perPlayerDifficulty = bitfield0 >> 2 & 1;
		data->perPlayerModifiers = bitfield0 >> 3 & 1;
	}
}
static void _pkt_ConnectAccept_write(const struct ConnectAccept *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u64_write(&data->connectTime, pkt, end, ctx);
	_pkt_u8_write(&data->connectNum, pkt, end, ctx);
	_pkt_b_write(&data->reusedPeer, pkt, end, ctx);
	if(ctx.netVersion >= 12) {
		_pkt_i32_write(&data->peerId, pkt, end, ctx);
	}
	if(ctx.beatUpVersion) {
		_pkt_u32_write(&data->windowSize, pkt, end, ctx);
		_pkt_u8_write(&data->countdownDuration, pkt, end, ctx);
		uint8_t bitfield0 = 0;
		bitfield0 |= (data->directDownloads & 1u) << 0;
		bitfield0 |= (data->skipResults & 1u) << 1;
		bitfield0 |= (data->perPlayerDifficulty & 1u) << 2;
		bitfield0 |= (data->perPlayerModifiers & 1u) << 3;
		_pkt_u8_write(&bitfield0, pkt, end, ctx);
	}
}
static void _pkt_Disconnect_read(struct Disconnect *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_raw_read(data->_pad0, pkt, end, ctx, 8);
}
static void _pkt_Disconnect_write(const struct Disconnect *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_raw_write(data->_pad0, pkt, end, ctx, 8);
}
void _pkt_UnconnectedMessage_read(struct UnconnectedMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->type, pkt, end, ctx);
	_pkt_vu32_read(&data->protocolVersion, pkt, end, ctx);
}
void _pkt_UnconnectedMessage_write(const struct UnconnectedMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->type, pkt, end, ctx);
	_pkt_vu32_write(&data->protocolVersion, pkt, end, ctx);
}
static void _pkt_Mtu_read(struct Mtu *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->newMtu0, pkt, end, ctx);
	_pkt_raw_read(data->pad, pkt, end, ctx, check_overflow(data->newMtu0 - 9, 1423, "Mtu.pad"));
	_pkt_u32_read(&data->newMtu1, pkt, end, ctx);
}
static void _pkt_Mtu_write(const struct Mtu *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->newMtu0, pkt, end, ctx);
	_pkt_raw_write(data->pad, pkt, end, ctx, check_overflow(data->newMtu0 - 9, 1423, "Mtu.pad"));
	_pkt_u32_write(&data->newMtu1, pkt, end, ctx);
}
static void _pkt_MtuCheck_read(struct MtuCheck *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_Mtu_read(&data->base, pkt, end, ctx);
}
static void _pkt_MtuCheck_write(const struct MtuCheck *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_Mtu_write(&data->base, pkt, end, ctx);
}
static void _pkt_MtuOk_read(struct MtuOk *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_Mtu_read(&data->base, pkt, end, ctx);
}
static void _pkt_MtuOk_write(const struct MtuOk *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_Mtu_write(&data->base, pkt, end, ctx);
}
void _pkt_MergedHeader_read(struct MergedHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->length, pkt, end, ctx);
}
void _pkt_MergedHeader_write(const struct MergedHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->length, pkt, end, ctx);
}
static void _pkt_Merged_read(struct Merged *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
static void _pkt_Merged_write(const struct Merged *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
}
void _pkt_NetPacketHeader_read(struct NetPacketHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->property = bitfield0 >> 0 & 31;
	data->connectionNumber = bitfield0 >> 5 & 3;
	data->isFragmented = bitfield0 >> 7 & 1;
	switch(data->property) {
		case PacketProperty_Unreliable: _pkt_Unreliable_read(&data->unreliable, pkt, end, ctx); break;
		case PacketProperty_Channeled: _pkt_Channeled_read(&data->channeled, pkt, end, ctx); break;
		case PacketProperty_Ack: _pkt_Ack_read(&data->ack, pkt, end, ctx); break;
		case PacketProperty_Ping: _pkt_Ping_read(&data->ping, pkt, end, ctx); break;
		case PacketProperty_Pong: _pkt_Pong_read(&data->pong, pkt, end, ctx); break;
		case PacketProperty_ConnectRequest: _pkt_ConnectRequest_read(&data->connectRequest, pkt, end, ctx); break;
		case PacketProperty_ConnectAccept: _pkt_ConnectAccept_read(&data->connectAccept, pkt, end, ctx); break;
		case PacketProperty_Disconnect: _pkt_Disconnect_read(&data->disconnect, pkt, end, ctx); break;
		case PacketProperty_UnconnectedMessage: _pkt_UnconnectedMessage_read(&data->unconnectedMessage, pkt, end, ctx); break;
		case PacketProperty_MtuCheck: _pkt_MtuCheck_read(&data->mtuCheck, pkt, end, ctx); break;
		case PacketProperty_MtuOk: _pkt_MtuOk_read(&data->mtuOk, pkt, end, ctx); break;
		case PacketProperty_Merged: _pkt_Merged_read(&data->merged, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `PacketProperty`\n"); longjmp(fail, 1);
	}
}
void _pkt_NetPacketHeader_write(const struct NetPacketHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->property & 31u) << 0;
	bitfield0 |= (data->connectionNumber & 3u) << 5;
	bitfield0 |= (data->isFragmented & 1u) << 7;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
	switch(data->property) {
		case PacketProperty_Unreliable: _pkt_Unreliable_write(&data->unreliable, pkt, end, ctx); break;
		case PacketProperty_Channeled: _pkt_Channeled_write(&data->channeled, pkt, end, ctx); break;
		case PacketProperty_Ack: _pkt_Ack_write(&data->ack, pkt, end, ctx); break;
		case PacketProperty_Ping: _pkt_Ping_write(&data->ping, pkt, end, ctx); break;
		case PacketProperty_Pong: _pkt_Pong_write(&data->pong, pkt, end, ctx); break;
		case PacketProperty_ConnectRequest: _pkt_ConnectRequest_write(&data->connectRequest, pkt, end, ctx); break;
		case PacketProperty_ConnectAccept: _pkt_ConnectAccept_write(&data->connectAccept, pkt, end, ctx); break;
		case PacketProperty_Disconnect: _pkt_Disconnect_write(&data->disconnect, pkt, end, ctx); break;
		case PacketProperty_UnconnectedMessage: _pkt_UnconnectedMessage_write(&data->unconnectedMessage, pkt, end, ctx); break;
		case PacketProperty_MtuCheck: _pkt_MtuCheck_write(&data->mtuCheck, pkt, end, ctx); break;
		case PacketProperty_MtuOk: _pkt_MtuOk_write(&data->mtuOk, pkt, end, ctx); break;
		case PacketProperty_Merged: _pkt_Merged_write(&data->merged, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `PacketProperty`\n"); longjmp(fail, 1);
	}
}
void _pkt_PacketEncryptionLayer_read(struct PacketEncryptionLayer *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_b_read(&data->encrypted, pkt, end, ctx);
	if(data->encrypted == 1) {
		_pkt_u32_read(&data->sequenceId, pkt, end, ctx);
		_pkt_raw_read(data->iv, pkt, end, ctx, 16);
	}
}
void _pkt_PacketEncryptionLayer_write(const struct PacketEncryptionLayer *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_b_write(&data->encrypted, pkt, end, ctx);
	if(data->encrypted == 1) {
		_pkt_u32_write(&data->sequenceId, pkt, end, ctx);
		_pkt_raw_write(data->iv, pkt, end, ctx, 16);
	}
}
