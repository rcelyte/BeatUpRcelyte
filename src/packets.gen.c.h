static void _pkt_PacketContext_read(struct PacketContext *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PacketContext");
	_pkt_u8_read(&data->netVersion, state);
	_pkt_u8_read(&data->protocolVersion, state);
	_pkt_u8_read(&data->beatUpVersion, state);
	_pkt_u8_read(&data->gameVersion, state);
	_pkt_b_read(&data->direct, state);
	_pkt_u16_read(&data->windowSize, state);
}
static void _pkt_PacketContext_write(const struct PacketContext *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PacketContext");
	_pkt_u8_write(&data->netVersion, state);
	_pkt_u8_write(&data->protocolVersion, state);
	_pkt_u8_write(&data->beatUpVersion, state);
	_pkt_u8_write(&data->gameVersion, state);
	_pkt_b_write(&data->direct, state);
	_pkt_u16_write(&data->windowSize, state);
}
static void _pkt_ByteArrayNetSerializable_read(struct ByteArrayNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ByteArrayNetSerializable");
	_pkt_vu32_read(&data->length, state);
	_pkt_raw_read(data->data, check_overflow(state, *state.head, (uint32_t)(data->length), 8192, "ByteArrayNetSerializable.data"), state);
}
static void _pkt_ByteArrayNetSerializable_write(const struct ByteArrayNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ByteArrayNetSerializable");
	_pkt_vu32_write(&data->length, state);
	_pkt_raw_write(data->data, check_overflow(state, *state.head, (uint32_t)(data->length), 8192, "ByteArrayNetSerializable.data"), state);
}
static void _pkt_ConnectInfo_read(struct ConnectInfo *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ConnectInfo");
	_pkt_u32_read(&data->protocolId, state);
	_pkt_u16_read(&data->blockSize, state);
}
static void _pkt_ConnectInfo_write(const struct ConnectInfo *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ConnectInfo");
	_pkt_u32_write(&data->protocolId, state);
	_pkt_u16_write(&data->blockSize, state);
}
static void _pkt_PreviewDifficultyBeatmapSet_read(struct PreviewDifficultyBeatmapSet *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PreviewDifficultyBeatmapSet");
	_pkt_String_read(&data->characteristic, state);
	_pkt_u8_read(&data->difficulties_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->difficulties_len), 5, "PreviewDifficultyBeatmapSet.difficulties"); i < count; ++i)
		_pkt_vu32_read(&data->difficulties[i], state);
}
static void _pkt_PreviewDifficultyBeatmapSet_write(const struct PreviewDifficultyBeatmapSet *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PreviewDifficultyBeatmapSet");
	_pkt_String_write(&data->characteristic, state);
	_pkt_u8_write(&data->difficulties_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->difficulties_len), 5, "PreviewDifficultyBeatmapSet.difficulties"); i < count; ++i)
		_pkt_vu32_write(&data->difficulties[i], state);
}
static void _pkt_PreviewBeatmapLevel_read(struct PreviewBeatmapLevel *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PreviewBeatmapLevel");
	_pkt_LongString_read(&data->levelID, state);
	_pkt_LongString_read(&data->songName, state);
	_pkt_LongString_read(&data->songSubName, state);
	_pkt_LongString_read(&data->songAuthorName, state);
	_pkt_LongString_read(&data->levelAuthorName, state);
	_pkt_f32_read(&data->beatsPerMinute, state);
	_pkt_f32_read(&data->songTimeOffset, state);
	_pkt_f32_read(&data->shuffle, state);
	_pkt_f32_read(&data->shufflePeriod, state);
	_pkt_f32_read(&data->previewStartTime, state);
	_pkt_f32_read(&data->previewDuration, state);
	_pkt_f32_read(&data->songDuration, state);
	_pkt_String_read(&data->environmentInfo, state);
	_pkt_String_read(&data->allDirectionsEnvironmentInfo, state);
	_pkt_u8_read(&data->beatmapSets_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->beatmapSets_len), 8, "PreviewBeatmapLevel.beatmapSets"); i < count; ++i)
		_pkt_PreviewDifficultyBeatmapSet_read(&data->beatmapSets[i], state);
	_pkt_ByteArrayNetSerializable_read(&data->cover, state);
}
static void _pkt_PreviewBeatmapLevel_write(const struct PreviewBeatmapLevel *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PreviewBeatmapLevel");
	_pkt_LongString_write(&data->levelID, state);
	_pkt_LongString_write(&data->songName, state);
	_pkt_LongString_write(&data->songSubName, state);
	_pkt_LongString_write(&data->songAuthorName, state);
	_pkt_LongString_write(&data->levelAuthorName, state);
	_pkt_f32_write(&data->beatsPerMinute, state);
	_pkt_f32_write(&data->songTimeOffset, state);
	_pkt_f32_write(&data->shuffle, state);
	_pkt_f32_write(&data->shufflePeriod, state);
	_pkt_f32_write(&data->previewStartTime, state);
	_pkt_f32_write(&data->previewDuration, state);
	_pkt_f32_write(&data->songDuration, state);
	_pkt_String_write(&data->environmentInfo, state);
	_pkt_String_write(&data->allDirectionsEnvironmentInfo, state);
	_pkt_u8_write(&data->beatmapSets_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->beatmapSets_len), 8, "PreviewBeatmapLevel.beatmapSets"); i < count; ++i)
		_pkt_PreviewDifficultyBeatmapSet_write(&data->beatmapSets[i], state);
	_pkt_ByteArrayNetSerializable_write(&data->cover, state);
}
static void _pkt_CustomLabelSet_read(struct CustomLabelSet *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "CustomLabelSet");
	_pkt_u8_read(&data->difficulties_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->difficulties_len), 5, "CustomLabelSet.difficulties"); i < count; ++i)
		_pkt_LongString_read(&data->difficulties[i], state);
}
static void _pkt_CustomLabelSet_write(const struct CustomLabelSet *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "CustomLabelSet");
	_pkt_u8_write(&data->difficulties_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->difficulties_len), 5, "CustomLabelSet.difficulties"); i < count; ++i)
		_pkt_LongString_write(&data->difficulties[i], state);
}
static void _pkt_RecommendPreview_read(struct RecommendPreview *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RecommendPreview");
	_pkt_PreviewBeatmapLevel_read(&data->base, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->base.beatmapSets_len), 8, "RecommendPreview.labelSets"); i < count; ++i)
		_pkt_CustomLabelSet_read(&data->labelSets[i], state);
	_pkt_vu32_read(&data->requirements_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->requirements_len), 16, "RecommendPreview.requirements"); i < count; ++i)
		_pkt_String_read(&data->requirements[i], state);
	_pkt_vu32_read(&data->suggestions_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->suggestions_len), 16, "RecommendPreview.suggestions"); i < count; ++i)
		_pkt_String_read(&data->suggestions[i], state);
}
static void _pkt_RecommendPreview_write(const struct RecommendPreview *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RecommendPreview");
	_pkt_PreviewBeatmapLevel_write(&data->base, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->base.beatmapSets_len), 8, "RecommendPreview.labelSets"); i < count; ++i)
		_pkt_CustomLabelSet_write(&data->labelSets[i], state);
	_pkt_vu32_write(&data->requirements_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->requirements_len), 16, "RecommendPreview.requirements"); i < count; ++i)
		_pkt_String_write(&data->requirements[i], state);
	_pkt_vu32_write(&data->suggestions_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->suggestions_len), 16, "RecommendPreview.suggestions"); i < count; ++i)
		_pkt_String_write(&data->suggestions[i], state);
}
static void _pkt_ShareMeta_read(struct ShareMeta *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ShareMeta");
	_pkt_vu64_read(&data->byteLength, state);
	_pkt_raw_read(data->hash, 32, state);
}
static void _pkt_ShareMeta_write(const struct ShareMeta *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ShareMeta");
	_pkt_vu64_write(&data->byteLength, state);
	_pkt_raw_write(data->hash, 32, state);
}
static void _pkt_ShareId_read(struct ShareId *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ShareId");
	_pkt_u16_read(&data->usage, state);
	if(data->usage != ShareableType_None) {
		_pkt_String_read(&data->mimeType, state);
		_pkt_LongString_read(&data->name, state);
	}
}
static void _pkt_ShareId_write(const struct ShareId *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ShareId");
	_pkt_u16_write(&data->usage, state);
	if(data->usage != ShareableType_None) {
		_pkt_String_write(&data->mimeType, state);
		_pkt_LongString_write(&data->name, state);
	}
}
static void _pkt_ShareInfo_read(struct ShareInfo *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ShareInfo");
	_pkt_u32_read(&data->offset, state);
	_pkt_u16_read(&data->blockSize, state);
	_pkt_ShareMeta_read(&data->meta, state);
	_pkt_ShareId_read(&data->id, state);
}
static void _pkt_ShareInfo_write(const struct ShareInfo *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ShareInfo");
	_pkt_u32_write(&data->offset, state);
	_pkt_u16_write(&data->blockSize, state);
	_pkt_ShareMeta_write(&data->meta, state);
	_pkt_ShareId_write(&data->id, state);
}
static void _pkt_DataFragmentRequest_read(struct DataFragmentRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "DataFragmentRequest");
	_pkt_u32_read(&data->offset, state);
	_pkt_u8_read(&data->count, state);
}
static void _pkt_DataFragmentRequest_write(const struct DataFragmentRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "DataFragmentRequest");
	_pkt_u32_write(&data->offset, state);
	_pkt_u8_write(&data->count, state);
}
static void _pkt_DataFragment_read(struct DataFragment *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "DataFragment");
	_pkt_u32_read(&data->offset, state);
}
static void _pkt_DataFragment_write(const struct DataFragment *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "DataFragment");
	_pkt_u32_write(&data->offset, state);
}
static void _pkt_LoadProgress_read(struct LoadProgress *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "LoadProgress");
	_pkt_u32_read(&data->sequence, state);
	_pkt_u8_read(&data->state, state);
	_pkt_u16_read(&data->progress, state);
}
static void _pkt_LoadProgress_write(const struct LoadProgress *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "LoadProgress");
	_pkt_u32_write(&data->sequence, state);
	_pkt_u8_write(&data->state, state);
	_pkt_u16_write(&data->progress, state);
}
void _pkt_ServerConnectInfo_read(struct ServerConnectInfo *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ServerConnectInfo");
	_pkt_ConnectInfo_read(&data->base, state);
	_pkt_u32_read(&data->windowSize, state);
	_pkt_u8_read(&data->countdownDuration, state);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->directDownloads = bitfield0 >> 0 & 1;
	data->skipResults = bitfield0 >> 1 & 1;
	data->perPlayerDifficulty = bitfield0 >> 2 & 1;
	data->perPlayerModifiers = bitfield0 >> 3 & 1;
}
static void _pkt_ServerConnectInfo_write(const struct ServerConnectInfo *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ServerConnectInfo");
	_pkt_ConnectInfo_write(&data->base, state);
	_pkt_u32_write(&data->windowSize, state);
	_pkt_u8_write(&data->countdownDuration, state);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->directDownloads & 1u) << 0;
	bitfield0 |= (data->skipResults & 1u) << 1;
	bitfield0 |= (data->perPlayerDifficulty & 1u) << 2;
	bitfield0 |= (data->perPlayerModifiers & 1u) << 3;
	_pkt_u8_write(&bitfield0, state);
}
void _pkt_BeatUpMessage_read(struct BeatUpMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BeatUpMessage");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case BeatUpMessageType_ConnectInfo: _pkt_ConnectInfo_read(&data->connectInfo, state); break;
		case BeatUpMessageType_RecommendPreview: _pkt_RecommendPreview_read(&data->recommendPreview, state); break;
		case BeatUpMessageType_ShareInfo: _pkt_ShareInfo_read(&data->shareInfo, state); break;
		case BeatUpMessageType_DataFragmentRequest: _pkt_DataFragmentRequest_read(&data->dataFragmentRequest, state); break;
		case BeatUpMessageType_DataFragment: _pkt_DataFragment_read(&data->dataFragment, state); break;
		case BeatUpMessageType_LoadProgress: _pkt_LoadProgress_read(&data->loadProgress, state); break;
		case BeatUpMessageType_ServerConnectInfo: _pkt_ServerConnectInfo_read(&data->serverConnectInfo, state); break;
		default: uprintf("Invalid value for enum `BeatUpMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_BeatUpMessage_write(const struct BeatUpMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BeatUpMessage");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case BeatUpMessageType_ConnectInfo: _pkt_ConnectInfo_write(&data->connectInfo, state); break;
		case BeatUpMessageType_RecommendPreview: _pkt_RecommendPreview_write(&data->recommendPreview, state); break;
		case BeatUpMessageType_ShareInfo: _pkt_ShareInfo_write(&data->shareInfo, state); break;
		case BeatUpMessageType_DataFragmentRequest: _pkt_DataFragmentRequest_write(&data->dataFragmentRequest, state); break;
		case BeatUpMessageType_DataFragment: _pkt_DataFragment_write(&data->dataFragment, state); break;
		case BeatUpMessageType_LoadProgress: _pkt_LoadProgress_write(&data->loadProgress, state); break;
		case BeatUpMessageType_ServerConnectInfo: _pkt_ServerConnectInfo_write(&data->serverConnectInfo, state); break;
		default: uprintf("Invalid value for enum `BeatUpMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ModConnectHeader");
	_pkt_vu32_read(&data->length, state);
	_pkt_String_read(&data->name, state);
}
void _pkt_ModConnectHeader_write(const struct ModConnectHeader *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ModConnectHeader");
	_pkt_vu32_write(&data->length, state);
	_pkt_String_write(&data->name, state);
}
static void _pkt_BitMask128_read(struct BitMask128 *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BitMask128");
	_pkt_u64_read(&data->d0, state);
	_pkt_u64_read(&data->d1, state);
}
static void _pkt_BitMask128_write(const struct BitMask128 *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BitMask128");
	_pkt_u64_write(&data->d0, state);
	_pkt_u64_write(&data->d1, state);
}
static void _pkt_SongPackMask_read(struct SongPackMask *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SongPackMask");
	_pkt_BitMask128_read(&data->bloomFilter, state);
	if(state.context.gameVersion >= GameVersion_1_34_0) {
		_pkt_BitMask128_read(&data->bloomFilterHi, state);
	}
}
static void _pkt_SongPackMask_write(const struct SongPackMask *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SongPackMask");
	_pkt_BitMask128_write(&data->bloomFilter, state);
	if(state.context.gameVersion >= GameVersion_1_34_0) {
		_pkt_BitMask128_write(&data->bloomFilterHi, state);
	}
}
static void _pkt_BeatmapLevelSelectionMask_read(struct BeatmapLevelSelectionMask *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BeatmapLevelSelectionMask");
	_pkt_u8_read(&data->difficulties, state);
	_pkt_u32_read(&data->modifiers, state);
	_pkt_SongPackMask_read(&data->songPacks, state);
}
static void _pkt_BeatmapLevelSelectionMask_write(const struct BeatmapLevelSelectionMask *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BeatmapLevelSelectionMask");
	_pkt_u8_write(&data->difficulties, state);
	_pkt_u32_write(&data->modifiers, state);
	_pkt_SongPackMask_write(&data->songPacks, state);
}
static void _pkt_UTimestamp_read(struct UTimestamp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "UTimestamp");
	if(state.context.protocolVersion < 9) {
		_pkt_f32_read(&data->legacy, state);
	}
	if(state.context.protocolVersion >= 9) {
		_pkt_vu64_read(&data->value, state);
	}
}
static void _pkt_UTimestamp_write(const struct UTimestamp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "UTimestamp");
	if(state.context.protocolVersion < 9) {
		_pkt_f32_write(&data->legacy, state);
	}
	if(state.context.protocolVersion >= 9) {
		_pkt_vu64_write(&data->value, state);
	}
}
static void _pkt_STimestamp_read(struct STimestamp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "STimestamp");
	if(state.context.protocolVersion < 9) {
		_pkt_f32_read(&data->legacy, state);
	}
	if(state.context.protocolVersion >= 9) {
		_pkt_vi64_read(&data->value, state);
	}
}
static void _pkt_STimestamp_write(const struct STimestamp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "STimestamp");
	if(state.context.protocolVersion < 9) {
		_pkt_f32_write(&data->legacy, state);
	}
	if(state.context.protocolVersion >= 9) {
		_pkt_vi64_write(&data->value, state);
	}
}
static void _pkt_RemoteProcedureCall_read(struct RemoteProcedureCall *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RemoteProcedureCall");
	_pkt_UTimestamp_read(&data->syncTime, state);
}
static void _pkt_RemoteProcedureCall_write(const struct RemoteProcedureCall *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RemoteProcedureCall");
	_pkt_UTimestamp_write(&data->syncTime, state);
}
static void _pkt_SetPlayersMissingEntitlementsToLevel_read(struct SetPlayersMissingEntitlementsToLevel *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetPlayersMissingEntitlementsToLevel");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_i32_read(&data->count, state);
		for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 254, "SetPlayersMissingEntitlementsToLevel.players"); i < count; ++i)
			_pkt_String_read(&data->players[i], state);
	}
}
static void _pkt_SetPlayersMissingEntitlementsToLevel_write(const struct SetPlayersMissingEntitlementsToLevel *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetPlayersMissingEntitlementsToLevel");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_i32_write(&data->count, state);
		for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 254, "SetPlayersMissingEntitlementsToLevel.players"); i < count; ++i)
			_pkt_String_write(&data->players[i], state);
	}
}
static void _pkt_GetIsEntitledToLevel_read(struct GetIsEntitledToLevel *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetIsEntitledToLevel");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, state);
	}
}
static void _pkt_GetIsEntitledToLevel_write(const struct GetIsEntitledToLevel *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetIsEntitledToLevel");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, state);
	}
}
static void _pkt_SetIsEntitledToLevel_read(struct SetIsEntitledToLevel *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetIsEntitledToLevel");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, state);
	}
	if(data->flags.hasValue1) {
		_pkt_vi32_read(&data->entitlementStatus, state);
	}
}
static void _pkt_SetIsEntitledToLevel_write(const struct SetIsEntitledToLevel *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetIsEntitledToLevel");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, state);
	}
	if(data->flags.hasValue1) {
		_pkt_vi32_write(&data->entitlementStatus, state);
	}
}
static void _pkt_InvalidateLevelEntitlementStatuses_read(struct InvalidateLevelEntitlementStatuses *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "InvalidateLevelEntitlementStatuses");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_InvalidateLevelEntitlementStatuses_write(const struct InvalidateLevelEntitlementStatuses *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "InvalidateLevelEntitlementStatuses");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SelectLevelPack_read(struct SelectLevelPack *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SelectLevelPack");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_String_read(&data->levelPackId, state);
	}
}
static void _pkt_SelectLevelPack_write(const struct SelectLevelPack *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SelectLevelPack");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_String_write(&data->levelPackId, state);
	}
}
static void _pkt_BeatmapIdentifierNetSerializable_read(struct BeatmapIdentifierNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BeatmapIdentifierNetSerializable");
	_pkt_LongString_read(&data->levelID, state);
	_pkt_String_read(&data->characteristic, state);
	_pkt_vu32_read(&data->difficulty, state);
}
static void _pkt_BeatmapIdentifierNetSerializable_write(const struct BeatmapIdentifierNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BeatmapIdentifierNetSerializable");
	_pkt_LongString_write(&data->levelID, state);
	_pkt_String_write(&data->characteristic, state);
	_pkt_vu32_write(&data->difficulty, state);
}
static void _pkt_SetSelectedBeatmap_read(struct SetSelectedBeatmap *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetSelectedBeatmap");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_read(&data->identifier, state);
	}
}
static void _pkt_SetSelectedBeatmap_write(const struct SetSelectedBeatmap *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetSelectedBeatmap");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_write(&data->identifier, state);
	}
}
static void _pkt_GetSelectedBeatmap_read(struct GetSelectedBeatmap *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetSelectedBeatmap");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetSelectedBeatmap_write(const struct GetSelectedBeatmap *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetSelectedBeatmap");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_RecommendBeatmap_read(struct RecommendBeatmap *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RecommendBeatmap");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_read(&data->identifier, state);
	}
}
static void _pkt_RecommendBeatmap_write(const struct RecommendBeatmap *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RecommendBeatmap");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_write(&data->identifier, state);
	}
}
static void _pkt_ClearRecommendedBeatmap_read(struct ClearRecommendedBeatmap *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClearRecommendedBeatmap");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_ClearRecommendedBeatmap_write(const struct ClearRecommendedBeatmap *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClearRecommendedBeatmap");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GetRecommendedBeatmap_read(struct GetRecommendedBeatmap *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetRecommendedBeatmap");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetRecommendedBeatmap_write(const struct GetRecommendedBeatmap *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetRecommendedBeatmap");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GameplayModifiers_read(struct GameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GameplayModifiers");
	_pkt_u32_read(&data->raw, state);
}
static void _pkt_GameplayModifiers_write(const struct GameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GameplayModifiers");
	_pkt_u32_write(&data->raw, state);
}
static void _pkt_SetSelectedGameplayModifiers_read(struct SetSelectedGameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetSelectedGameplayModifiers");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_read(&data->gameplayModifiers, state);
	}
}
static void _pkt_SetSelectedGameplayModifiers_write(const struct SetSelectedGameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetSelectedGameplayModifiers");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_write(&data->gameplayModifiers, state);
	}
}
static void _pkt_GetSelectedGameplayModifiers_read(struct GetSelectedGameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetSelectedGameplayModifiers");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetSelectedGameplayModifiers_write(const struct GetSelectedGameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetSelectedGameplayModifiers");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_RecommendGameplayModifiers_read(struct RecommendGameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RecommendGameplayModifiers");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_read(&data->gameplayModifiers, state);
	}
}
static void _pkt_RecommendGameplayModifiers_write(const struct RecommendGameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RecommendGameplayModifiers");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_GameplayModifiers_write(&data->gameplayModifiers, state);
	}
}
static void _pkt_ClearRecommendedGameplayModifiers_read(struct ClearRecommendedGameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClearRecommendedGameplayModifiers");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_ClearRecommendedGameplayModifiers_write(const struct ClearRecommendedGameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClearRecommendedGameplayModifiers");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GetRecommendedGameplayModifiers_read(struct GetRecommendedGameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetRecommendedGameplayModifiers");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetRecommendedGameplayModifiers_write(const struct GetRecommendedGameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetRecommendedGameplayModifiers");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_LevelLoadError_read(struct LevelLoadError *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "LevelLoadError");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, state);
	}
}
static void _pkt_LevelLoadError_write(const struct LevelLoadError *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "LevelLoadError");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, state);
	}
}
static void _pkt_LevelLoadSuccess_read(struct LevelLoadSuccess *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "LevelLoadSuccess");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_read(&data->levelId, state);
	}
}
static void _pkt_LevelLoadSuccess_write(const struct LevelLoadSuccess *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "LevelLoadSuccess");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_LongString_write(&data->levelId, state);
	}
}
static void _pkt_StartLevel_read(struct StartLevel *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "StartLevel");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_read(&data->beatmapId, state);
	}
	if(data->flags.hasValue1) {
		_pkt_GameplayModifiers_read(&data->gameplayModifiers, state);
	}
	if(data->flags.hasValue2) {
		_pkt_STimestamp_read(&data->startTime, state);
	}
}
static void _pkt_StartLevel_write(const struct StartLevel *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "StartLevel");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_BeatmapIdentifierNetSerializable_write(&data->beatmapId, state);
	}
	if(data->flags.hasValue1) {
		_pkt_GameplayModifiers_write(&data->gameplayModifiers, state);
	}
	if(data->flags.hasValue2) {
		_pkt_STimestamp_write(&data->startTime, state);
	}
}
static void _pkt_GetStartedLevel_read(struct GetStartedLevel *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetStartedLevel");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetStartedLevel_write(const struct GetStartedLevel *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetStartedLevel");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_CancelLevelStart_read(struct CancelLevelStart *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "CancelLevelStart");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_CancelLevelStart_write(const struct CancelLevelStart *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "CancelLevelStart");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GetMultiplayerGameState_read(struct GetMultiplayerGameState *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetMultiplayerGameState");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetMultiplayerGameState_write(const struct GetMultiplayerGameState *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetMultiplayerGameState");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetMultiplayerGameState_read(struct SetMultiplayerGameState *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetMultiplayerGameState");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_vi32_read(&data->lobbyState, state);
	}
}
static void _pkt_SetMultiplayerGameState_write(const struct SetMultiplayerGameState *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetMultiplayerGameState");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_vi32_write(&data->lobbyState, state);
	}
}
static void _pkt_GetIsReady_read(struct GetIsReady *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetIsReady");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetIsReady_write(const struct GetIsReady *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetIsReady");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetIsReady_read(struct SetIsReady *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetIsReady");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_b_read(&data->isReady, state);
	}
}
static void _pkt_SetIsReady_write(const struct SetIsReady *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetIsReady");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_b_write(&data->isReady, state);
	}
}
static void _pkt_SetStartGameTime_read(struct SetStartGameTime *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetStartGameTime");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_STimestamp_read(&data->newTime, state);
	}
}
static void _pkt_SetStartGameTime_write(const struct SetStartGameTime *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetStartGameTime");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_STimestamp_write(&data->newTime, state);
	}
}
static void _pkt_CancelStartGameTime_read(struct CancelStartGameTime *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "CancelStartGameTime");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_CancelStartGameTime_write(const struct CancelStartGameTime *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "CancelStartGameTime");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GetIsInLobby_read(struct GetIsInLobby *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetIsInLobby");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetIsInLobby_write(const struct GetIsInLobby *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetIsInLobby");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetIsInLobby_read(struct SetIsInLobby *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetIsInLobby");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_b_read(&data->isBack, state);
	}
}
static void _pkt_SetIsInLobby_write(const struct SetIsInLobby *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetIsInLobby");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_b_write(&data->isBack, state);
	}
}
static void _pkt_GetCountdownEndTime_read(struct GetCountdownEndTime *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetCountdownEndTime");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetCountdownEndTime_write(const struct GetCountdownEndTime *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetCountdownEndTime");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetCountdownEndTime_read(struct SetCountdownEndTime *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetCountdownEndTime");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_STimestamp_read(&data->newTime, state);
	}
}
static void _pkt_SetCountdownEndTime_write(const struct SetCountdownEndTime *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetCountdownEndTime");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_STimestamp_write(&data->newTime, state);
	}
}
static void _pkt_CancelCountdown_read(struct CancelCountdown *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "CancelCountdown");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_CancelCountdown_write(const struct CancelCountdown *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "CancelCountdown");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GetOwnedSongPacks_read(struct GetOwnedSongPacks *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetOwnedSongPacks");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetOwnedSongPacks_write(const struct GetOwnedSongPacks *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetOwnedSongPacks");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetOwnedSongPacks_read(struct SetOwnedSongPacks *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetOwnedSongPacks");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_SongPackMask_read(&data->songPackMask, state);
	}
}
static void _pkt_SetOwnedSongPacks_write(const struct SetOwnedSongPacks *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetOwnedSongPacks");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_SongPackMask_write(&data->songPackMask, state);
	}
}
static void _pkt_RequestKickPlayer_read(struct RequestKickPlayer *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RequestKickPlayer");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_String_read(&data->kickedPlayerId, state);
	}
}
static void _pkt_RequestKickPlayer_write(const struct RequestKickPlayer *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RequestKickPlayer");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_String_write(&data->kickedPlayerId, state);
	}
}
static void _pkt_GetPermissionConfiguration_read(struct GetPermissionConfiguration *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetPermissionConfiguration");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetPermissionConfiguration_write(const struct GetPermissionConfiguration *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetPermissionConfiguration");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_PlayerLobbyPermissionConfiguration_read(struct PlayerLobbyPermissionConfiguration *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerLobbyPermissionConfiguration");
	_pkt_String_read(&data->userId, state);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->serverOwner = bitfield0 >> 0 & 1;
	data->recommendBeatmaps = bitfield0 >> 1 & 1;
	data->recommendModifiers = bitfield0 >> 2 & 1;
	data->kickVote = bitfield0 >> 3 & 1;
	data->invite = bitfield0 >> 4 & 1;
}
static void _pkt_PlayerLobbyPermissionConfiguration_write(const struct PlayerLobbyPermissionConfiguration *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerLobbyPermissionConfiguration");
	_pkt_String_write(&data->userId, state);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->serverOwner & 1u) << 0;
	bitfield0 |= (data->recommendBeatmaps & 1u) << 1;
	bitfield0 |= (data->recommendModifiers & 1u) << 2;
	bitfield0 |= (data->kickVote & 1u) << 3;
	bitfield0 |= (data->invite & 1u) << 4;
	_pkt_u8_write(&bitfield0, state);
}
static void _pkt_PlayersLobbyPermissionConfiguration_read(struct PlayersLobbyPermissionConfiguration *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayersLobbyPermissionConfiguration");
	_pkt_i32_read(&data->count, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 254, "PlayersLobbyPermissionConfiguration.playersPermission"); i < count; ++i)
		_pkt_PlayerLobbyPermissionConfiguration_read(&data->playersPermission[i], state);
}
static void _pkt_PlayersLobbyPermissionConfiguration_write(const struct PlayersLobbyPermissionConfiguration *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayersLobbyPermissionConfiguration");
	_pkt_i32_write(&data->count, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 254, "PlayersLobbyPermissionConfiguration.playersPermission"); i < count; ++i)
		_pkt_PlayerLobbyPermissionConfiguration_write(&data->playersPermission[i], state);
}
static void _pkt_SetPermissionConfiguration_read(struct SetPermissionConfiguration *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetPermissionConfiguration");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_PlayersLobbyPermissionConfiguration_read(&data->playersPermissionConfiguration, state);
	}
}
static void _pkt_SetPermissionConfiguration_write(const struct SetPermissionConfiguration *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetPermissionConfiguration");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_PlayersLobbyPermissionConfiguration_write(&data->playersPermissionConfiguration, state);
	}
}
static void _pkt_GetIsStartButtonEnabled_read(struct GetIsStartButtonEnabled *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetIsStartButtonEnabled");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetIsStartButtonEnabled_write(const struct GetIsStartButtonEnabled *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetIsStartButtonEnabled");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetIsStartButtonEnabled_read(struct SetIsStartButtonEnabled *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetIsStartButtonEnabled");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_vi32_read(&data->reason, state);
	}
}
static void _pkt_SetIsStartButtonEnabled_write(const struct SetIsStartButtonEnabled *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetIsStartButtonEnabled");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_vi32_write(&data->reason, state);
	}
}
static void _pkt_ClearSelectedBeatmap_read(struct ClearSelectedBeatmap *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClearSelectedBeatmap");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_ClearSelectedBeatmap_write(const struct ClearSelectedBeatmap *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClearSelectedBeatmap");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_ClearSelectedGameplayModifiers_read(struct ClearSelectedGameplayModifiers *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClearSelectedGameplayModifiers");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_ClearSelectedGameplayModifiers_write(const struct ClearSelectedGameplayModifiers *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClearSelectedGameplayModifiers");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_MenuRpc_read(struct MenuRpc *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MenuRpc");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: _pkt_SetPlayersMissingEntitlementsToLevel_read(&data->setPlayersMissingEntitlementsToLevel, state); break;
		case MenuRpcType_GetIsEntitledToLevel: _pkt_GetIsEntitledToLevel_read(&data->getIsEntitledToLevel, state); break;
		case MenuRpcType_SetIsEntitledToLevel: _pkt_SetIsEntitledToLevel_read(&data->setIsEntitledToLevel, state); break;
		case MenuRpcType_InvalidateLevelEntitlementStatuses: _pkt_InvalidateLevelEntitlementStatuses_read(&data->invalidateLevelEntitlementStatuses, state); break;
		case MenuRpcType_SelectLevelPack: _pkt_SelectLevelPack_read(&data->selectLevelPack, state); break;
		case MenuRpcType_SetSelectedBeatmap: _pkt_SetSelectedBeatmap_read(&data->setSelectedBeatmap, state); break;
		case MenuRpcType_GetSelectedBeatmap: _pkt_GetSelectedBeatmap_read(&data->getSelectedBeatmap, state); break;
		case MenuRpcType_RecommendBeatmap: _pkt_RecommendBeatmap_read(&data->recommendBeatmap, state); break;
		case MenuRpcType_ClearRecommendedBeatmap: _pkt_ClearRecommendedBeatmap_read(&data->clearRecommendedBeatmap, state); break;
		case MenuRpcType_GetRecommendedBeatmap: _pkt_GetRecommendedBeatmap_read(&data->getRecommendedBeatmap, state); break;
		case MenuRpcType_SetSelectedGameplayModifiers: _pkt_SetSelectedGameplayModifiers_read(&data->setSelectedGameplayModifiers, state); break;
		case MenuRpcType_GetSelectedGameplayModifiers: _pkt_GetSelectedGameplayModifiers_read(&data->getSelectedGameplayModifiers, state); break;
		case MenuRpcType_RecommendGameplayModifiers: _pkt_RecommendGameplayModifiers_read(&data->recommendGameplayModifiers, state); break;
		case MenuRpcType_ClearRecommendedGameplayModifiers: _pkt_ClearRecommendedGameplayModifiers_read(&data->clearRecommendedGameplayModifiers, state); break;
		case MenuRpcType_GetRecommendedGameplayModifiers: _pkt_GetRecommendedGameplayModifiers_read(&data->getRecommendedGameplayModifiers, state); break;
		case MenuRpcType_LevelLoadError: _pkt_LevelLoadError_read(&data->levelLoadError, state); break;
		case MenuRpcType_LevelLoadSuccess: _pkt_LevelLoadSuccess_read(&data->levelLoadSuccess, state); break;
		case MenuRpcType_StartLevel: _pkt_StartLevel_read(&data->startLevel, state); break;
		case MenuRpcType_GetStartedLevel: _pkt_GetStartedLevel_read(&data->getStartedLevel, state); break;
		case MenuRpcType_CancelLevelStart: _pkt_CancelLevelStart_read(&data->cancelLevelStart, state); break;
		case MenuRpcType_GetMultiplayerGameState: _pkt_GetMultiplayerGameState_read(&data->getMultiplayerGameState, state); break;
		case MenuRpcType_SetMultiplayerGameState: _pkt_SetMultiplayerGameState_read(&data->setMultiplayerGameState, state); break;
		case MenuRpcType_GetIsReady: _pkt_GetIsReady_read(&data->getIsReady, state); break;
		case MenuRpcType_SetIsReady: _pkt_SetIsReady_read(&data->setIsReady, state); break;
		case MenuRpcType_SetStartGameTime: _pkt_SetStartGameTime_read(&data->setStartGameTime, state); break;
		case MenuRpcType_CancelStartGameTime: _pkt_CancelStartGameTime_read(&data->cancelStartGameTime, state); break;
		case MenuRpcType_GetIsInLobby: _pkt_GetIsInLobby_read(&data->getIsInLobby, state); break;
		case MenuRpcType_SetIsInLobby: _pkt_SetIsInLobby_read(&data->setIsInLobby, state); break;
		case MenuRpcType_GetCountdownEndTime: _pkt_GetCountdownEndTime_read(&data->getCountdownEndTime, state); break;
		case MenuRpcType_SetCountdownEndTime: _pkt_SetCountdownEndTime_read(&data->setCountdownEndTime, state); break;
		case MenuRpcType_CancelCountdown: _pkt_CancelCountdown_read(&data->cancelCountdown, state); break;
		case MenuRpcType_GetOwnedSongPacks: _pkt_GetOwnedSongPacks_read(&data->getOwnedSongPacks, state); break;
		case MenuRpcType_SetOwnedSongPacks: _pkt_SetOwnedSongPacks_read(&data->setOwnedSongPacks, state); break;
		case MenuRpcType_RequestKickPlayer: _pkt_RequestKickPlayer_read(&data->requestKickPlayer, state); break;
		case MenuRpcType_GetPermissionConfiguration: _pkt_GetPermissionConfiguration_read(&data->getPermissionConfiguration, state); break;
		case MenuRpcType_SetPermissionConfiguration: _pkt_SetPermissionConfiguration_read(&data->setPermissionConfiguration, state); break;
		case MenuRpcType_GetIsStartButtonEnabled: _pkt_GetIsStartButtonEnabled_read(&data->getIsStartButtonEnabled, state); break;
		case MenuRpcType_SetIsStartButtonEnabled: _pkt_SetIsStartButtonEnabled_read(&data->setIsStartButtonEnabled, state); break;
		case MenuRpcType_ClearSelectedBeatmap: _pkt_ClearSelectedBeatmap_read(&data->clearSelectedBeatmap, state); break;
		case MenuRpcType_ClearSelectedGameplayModifiers: _pkt_ClearSelectedGameplayModifiers_read(&data->clearSelectedGameplayModifiers, state); break;
		default: uprintf("Invalid value for enum `MenuRpcType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_MenuRpc_write(const struct MenuRpc *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MenuRpc");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case MenuRpcType_SetPlayersMissingEntitlementsToLevel: _pkt_SetPlayersMissingEntitlementsToLevel_write(&data->setPlayersMissingEntitlementsToLevel, state); break;
		case MenuRpcType_GetIsEntitledToLevel: _pkt_GetIsEntitledToLevel_write(&data->getIsEntitledToLevel, state); break;
		case MenuRpcType_SetIsEntitledToLevel: _pkt_SetIsEntitledToLevel_write(&data->setIsEntitledToLevel, state); break;
		case MenuRpcType_InvalidateLevelEntitlementStatuses: _pkt_InvalidateLevelEntitlementStatuses_write(&data->invalidateLevelEntitlementStatuses, state); break;
		case MenuRpcType_SelectLevelPack: _pkt_SelectLevelPack_write(&data->selectLevelPack, state); break;
		case MenuRpcType_SetSelectedBeatmap: _pkt_SetSelectedBeatmap_write(&data->setSelectedBeatmap, state); break;
		case MenuRpcType_GetSelectedBeatmap: _pkt_GetSelectedBeatmap_write(&data->getSelectedBeatmap, state); break;
		case MenuRpcType_RecommendBeatmap: _pkt_RecommendBeatmap_write(&data->recommendBeatmap, state); break;
		case MenuRpcType_ClearRecommendedBeatmap: _pkt_ClearRecommendedBeatmap_write(&data->clearRecommendedBeatmap, state); break;
		case MenuRpcType_GetRecommendedBeatmap: _pkt_GetRecommendedBeatmap_write(&data->getRecommendedBeatmap, state); break;
		case MenuRpcType_SetSelectedGameplayModifiers: _pkt_SetSelectedGameplayModifiers_write(&data->setSelectedGameplayModifiers, state); break;
		case MenuRpcType_GetSelectedGameplayModifiers: _pkt_GetSelectedGameplayModifiers_write(&data->getSelectedGameplayModifiers, state); break;
		case MenuRpcType_RecommendGameplayModifiers: _pkt_RecommendGameplayModifiers_write(&data->recommendGameplayModifiers, state); break;
		case MenuRpcType_ClearRecommendedGameplayModifiers: _pkt_ClearRecommendedGameplayModifiers_write(&data->clearRecommendedGameplayModifiers, state); break;
		case MenuRpcType_GetRecommendedGameplayModifiers: _pkt_GetRecommendedGameplayModifiers_write(&data->getRecommendedGameplayModifiers, state); break;
		case MenuRpcType_LevelLoadError: _pkt_LevelLoadError_write(&data->levelLoadError, state); break;
		case MenuRpcType_LevelLoadSuccess: _pkt_LevelLoadSuccess_write(&data->levelLoadSuccess, state); break;
		case MenuRpcType_StartLevel: _pkt_StartLevel_write(&data->startLevel, state); break;
		case MenuRpcType_GetStartedLevel: _pkt_GetStartedLevel_write(&data->getStartedLevel, state); break;
		case MenuRpcType_CancelLevelStart: _pkt_CancelLevelStart_write(&data->cancelLevelStart, state); break;
		case MenuRpcType_GetMultiplayerGameState: _pkt_GetMultiplayerGameState_write(&data->getMultiplayerGameState, state); break;
		case MenuRpcType_SetMultiplayerGameState: _pkt_SetMultiplayerGameState_write(&data->setMultiplayerGameState, state); break;
		case MenuRpcType_GetIsReady: _pkt_GetIsReady_write(&data->getIsReady, state); break;
		case MenuRpcType_SetIsReady: _pkt_SetIsReady_write(&data->setIsReady, state); break;
		case MenuRpcType_SetStartGameTime: _pkt_SetStartGameTime_write(&data->setStartGameTime, state); break;
		case MenuRpcType_CancelStartGameTime: _pkt_CancelStartGameTime_write(&data->cancelStartGameTime, state); break;
		case MenuRpcType_GetIsInLobby: _pkt_GetIsInLobby_write(&data->getIsInLobby, state); break;
		case MenuRpcType_SetIsInLobby: _pkt_SetIsInLobby_write(&data->setIsInLobby, state); break;
		case MenuRpcType_GetCountdownEndTime: _pkt_GetCountdownEndTime_write(&data->getCountdownEndTime, state); break;
		case MenuRpcType_SetCountdownEndTime: _pkt_SetCountdownEndTime_write(&data->setCountdownEndTime, state); break;
		case MenuRpcType_CancelCountdown: _pkt_CancelCountdown_write(&data->cancelCountdown, state); break;
		case MenuRpcType_GetOwnedSongPacks: _pkt_GetOwnedSongPacks_write(&data->getOwnedSongPacks, state); break;
		case MenuRpcType_SetOwnedSongPacks: _pkt_SetOwnedSongPacks_write(&data->setOwnedSongPacks, state); break;
		case MenuRpcType_RequestKickPlayer: _pkt_RequestKickPlayer_write(&data->requestKickPlayer, state); break;
		case MenuRpcType_GetPermissionConfiguration: _pkt_GetPermissionConfiguration_write(&data->getPermissionConfiguration, state); break;
		case MenuRpcType_SetPermissionConfiguration: _pkt_SetPermissionConfiguration_write(&data->setPermissionConfiguration, state); break;
		case MenuRpcType_GetIsStartButtonEnabled: _pkt_GetIsStartButtonEnabled_write(&data->getIsStartButtonEnabled, state); break;
		case MenuRpcType_SetIsStartButtonEnabled: _pkt_SetIsStartButtonEnabled_write(&data->setIsStartButtonEnabled, state); break;
		case MenuRpcType_ClearSelectedBeatmap: _pkt_ClearSelectedBeatmap_write(&data->clearSelectedBeatmap, state); break;
		case MenuRpcType_ClearSelectedGameplayModifiers: _pkt_ClearSelectedGameplayModifiers_write(&data->clearSelectedGameplayModifiers, state); break;
		default: uprintf("Invalid value for enum `MenuRpcType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_ColorNoAlphaSerializable_read(struct ColorNoAlphaSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ColorNoAlphaSerializable");
	_pkt_f32_read(&data->r, state);
	_pkt_f32_read(&data->g, state);
	_pkt_f32_read(&data->b, state);
}
static void _pkt_ColorNoAlphaSerializable_write(const struct ColorNoAlphaSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ColorNoAlphaSerializable");
	_pkt_f32_write(&data->r, state);
	_pkt_f32_write(&data->g, state);
	_pkt_f32_write(&data->b, state);
}
static void _pkt_ColorSchemeNetSerializable_read(struct ColorSchemeNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ColorSchemeNetSerializable");
	_pkt_ColorNoAlphaSerializable_read(&data->saberAColor, state);
	_pkt_ColorNoAlphaSerializable_read(&data->saberBColor, state);
	_pkt_ColorNoAlphaSerializable_read(&data->obstaclesColor, state);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor0, state);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor1, state);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor0Boost, state);
	_pkt_ColorNoAlphaSerializable_read(&data->environmentColor1Boost, state);
}
static void _pkt_ColorSchemeNetSerializable_write(const struct ColorSchemeNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ColorSchemeNetSerializable");
	_pkt_ColorNoAlphaSerializable_write(&data->saberAColor, state);
	_pkt_ColorNoAlphaSerializable_write(&data->saberBColor, state);
	_pkt_ColorNoAlphaSerializable_write(&data->obstaclesColor, state);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor0, state);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor1, state);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor0Boost, state);
	_pkt_ColorNoAlphaSerializable_write(&data->environmentColor1Boost, state);
}
static void _pkt_PlayerSpecificSettings_read(struct PlayerSpecificSettings *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerSpecificSettings");
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_b_read(&data->leftHanded, state);
	_pkt_b_read(&data->automaticPlayerHeight, state);
	_pkt_f32_read(&data->playerHeight, state);
	_pkt_f32_read(&data->headPosToPlayerHeightOffset, state);
	_pkt_ColorSchemeNetSerializable_read(&data->colorScheme, state);
}
static void _pkt_PlayerSpecificSettings_write(const struct PlayerSpecificSettings *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerSpecificSettings");
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_b_write(&data->leftHanded, state);
	_pkt_b_write(&data->automaticPlayerHeight, state);
	_pkt_f32_write(&data->playerHeight, state);
	_pkt_f32_write(&data->headPosToPlayerHeightOffset, state);
	_pkt_ColorSchemeNetSerializable_write(&data->colorScheme, state);
}
static void _pkt_PlayerSpecificSettingsAtStart_read(struct PlayerSpecificSettingsAtStart *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerSpecificSettingsAtStart");
	_pkt_i32_read(&data->count, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 254, "PlayerSpecificSettingsAtStart.players"); i < count; ++i)
		_pkt_PlayerSpecificSettings_read(&data->players[i], state);
}
static void _pkt_PlayerSpecificSettingsAtStart_write(const struct PlayerSpecificSettingsAtStart *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerSpecificSettingsAtStart");
	_pkt_i32_write(&data->count, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 254, "PlayerSpecificSettingsAtStart.players"); i < count; ++i)
		_pkt_PlayerSpecificSettings_write(&data->players[i], state);
}
static void _pkt_SetGameplaySceneSyncFinish_read(struct SetGameplaySceneSyncFinish *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetGameplaySceneSyncFinish");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettingsAtStart_read(&data->settings, state);
	}
	if(data->flags.hasValue1) {
		_pkt_String_read(&data->sessionGameId, state);
	}
}
static void _pkt_SetGameplaySceneSyncFinish_write(const struct SetGameplaySceneSyncFinish *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetGameplaySceneSyncFinish");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettingsAtStart_write(&data->settings, state);
	}
	if(data->flags.hasValue1) {
		_pkt_String_write(&data->sessionGameId, state);
	}
}
static void _pkt_SetGameplaySceneReady_read(struct SetGameplaySceneReady *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetGameplaySceneReady");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettings_read(&data->playerSpecificSettings, state);
	}
}
static void _pkt_SetGameplaySceneReady_write(const struct SetGameplaySceneReady *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetGameplaySceneReady");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_PlayerSpecificSettings_write(&data->playerSpecificSettings, state);
	}
}
static void _pkt_GetGameplaySceneReady_read(struct GetGameplaySceneReady *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetGameplaySceneReady");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetGameplaySceneReady_write(const struct GetGameplaySceneReady *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetGameplaySceneReady");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetActivePlayerFailedToConnect_read(struct SetActivePlayerFailedToConnect *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetActivePlayerFailedToConnect");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_String_read(&data->failedUserId, state);
	}
	if(data->flags.hasValue1) {
		_pkt_PlayerSpecificSettingsAtStart_read(&data->settings, state);
	}
	if(data->flags.hasValue2) {
		_pkt_String_read(&data->sessionGameId, state);
	}
}
static void _pkt_SetActivePlayerFailedToConnect_write(const struct SetActivePlayerFailedToConnect *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetActivePlayerFailedToConnect");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_String_write(&data->failedUserId, state);
	}
	if(data->flags.hasValue1) {
		_pkt_PlayerSpecificSettingsAtStart_write(&data->settings, state);
	}
	if(data->flags.hasValue2) {
		_pkt_String_write(&data->sessionGameId, state);
	}
}
static void _pkt_SetGameplaySongReady_read(struct SetGameplaySongReady *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetGameplaySongReady");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_SetGameplaySongReady_write(const struct SetGameplaySongReady *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetGameplaySongReady");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_GetGameplaySongReady_read(struct GetGameplaySongReady *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetGameplaySongReady");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_GetGameplaySongReady_write(const struct GetGameplaySongReady *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetGameplaySongReady");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_SetSongStartTime_read(struct SetSongStartTime *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SetSongStartTime");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_STimestamp_read(&data->startTime, state);
	}
}
static void _pkt_SetSongStartTime_write(const struct SetSongStartTime *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SetSongStartTime");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_STimestamp_write(&data->startTime, state);
	}
}
static void _pkt_Vector3Serializable_read(struct Vector3Serializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Vector3Serializable");
	_pkt_vi32_read(&data->x, state);
	_pkt_vi32_read(&data->y, state);
	_pkt_vi32_read(&data->z, state);
}
static void _pkt_Vector3Serializable_write(const struct Vector3Serializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Vector3Serializable");
	_pkt_vi32_write(&data->x, state);
	_pkt_vi32_write(&data->y, state);
	_pkt_vi32_write(&data->z, state);
}
static void _pkt_QuaternionSerializable_read(struct QuaternionSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "QuaternionSerializable");
	_pkt_vi32_read(&data->a, state);
	_pkt_vi32_read(&data->b, state);
	_pkt_vi32_read(&data->c, state);
}
static void _pkt_QuaternionSerializable_write(const struct QuaternionSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "QuaternionSerializable");
	_pkt_vi32_write(&data->a, state);
	_pkt_vi32_write(&data->b, state);
	_pkt_vi32_write(&data->c, state);
}
static void _pkt_NoteCutInfoNetSerializable_read(struct NoteCutInfoNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NoteCutInfoNetSerializable");
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->cutWasOk = bitfield0 >> 0 & 1;
	_pkt_f32_read(&data->saberSpeed, state);
	_pkt_Vector3Serializable_read(&data->saberDir, state);
	_pkt_Vector3Serializable_read(&data->cutPoint, state);
	_pkt_Vector3Serializable_read(&data->cutNormal, state);
	_pkt_Vector3Serializable_read(&data->notePosition, state);
	_pkt_Vector3Serializable_read(&data->noteScale, state);
	_pkt_QuaternionSerializable_read(&data->noteRotation, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_read(&data->gameplayType, state);
	}
	_pkt_vi32_read(&data->colorType, state);
	_pkt_vi32_read(&data->lineLayer, state);
	_pkt_vi32_read(&data->noteLineIndex, state);
	_pkt_f32_read(&data->noteTime, state);
	_pkt_f32_read(&data->timeToNextColorNote, state);
	_pkt_Vector3Serializable_read(&data->moveVec, state);
}
static void _pkt_NoteCutInfoNetSerializable_write(const struct NoteCutInfoNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NoteCutInfoNetSerializable");
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->cutWasOk & 1u) << 0;
	_pkt_u8_write(&bitfield0, state);
	_pkt_f32_write(&data->saberSpeed, state);
	_pkt_Vector3Serializable_write(&data->saberDir, state);
	_pkt_Vector3Serializable_write(&data->cutPoint, state);
	_pkt_Vector3Serializable_write(&data->cutNormal, state);
	_pkt_Vector3Serializable_write(&data->notePosition, state);
	_pkt_Vector3Serializable_write(&data->noteScale, state);
	_pkt_QuaternionSerializable_write(&data->noteRotation, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_write(&data->gameplayType, state);
	}
	_pkt_vi32_write(&data->colorType, state);
	_pkt_vi32_write(&data->lineLayer, state);
	_pkt_vi32_write(&data->noteLineIndex, state);
	_pkt_f32_write(&data->noteTime, state);
	_pkt_f32_write(&data->timeToNextColorNote, state);
	_pkt_Vector3Serializable_write(&data->moveVec, state);
}
static void _pkt_NoteCut_read(struct NoteCut *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NoteCut");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteCutInfoNetSerializable_read(&data->noteCutInfo, state);
	}
}
static void _pkt_NoteCut_write(const struct NoteCut *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NoteCut");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteCutInfoNetSerializable_write(&data->noteCutInfo, state);
	}
}
static void _pkt_NoteMissInfoNetSerializable_read(struct NoteMissInfoNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NoteMissInfoNetSerializable");
	_pkt_vi32_read(&data->colorType, state);
	_pkt_vi32_read(&data->lineLayer, state);
	_pkt_vi32_read(&data->noteLineIndex, state);
	_pkt_f32_read(&data->noteTime, state);
}
static void _pkt_NoteMissInfoNetSerializable_write(const struct NoteMissInfoNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NoteMissInfoNetSerializable");
	_pkt_vi32_write(&data->colorType, state);
	_pkt_vi32_write(&data->lineLayer, state);
	_pkt_vi32_write(&data->noteLineIndex, state);
	_pkt_f32_write(&data->noteTime, state);
}
static void _pkt_NoteMissed_read(struct NoteMissed *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NoteMissed");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteMissInfoNetSerializable_read(&data->noteMissInfo, state);
	}
}
static void _pkt_NoteMissed_write(const struct NoteMissed *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NoteMissed");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteMissInfoNetSerializable_write(&data->noteMissInfo, state);
	}
}
static void _pkt_LevelCompletionResults_read(struct LevelCompletionResults *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "LevelCompletionResults");
	_pkt_GameplayModifiers_read(&data->gameplayModifiers, state);
	_pkt_vi32_read(&data->modifiedScore, state);
	_pkt_vi32_read(&data->multipliedScore, state);
	_pkt_vi32_read(&data->rank, state);
	_pkt_b_read(&data->fullCombo, state);
	_pkt_f32_read(&data->leftSaberMovementDistance, state);
	_pkt_f32_read(&data->rightSaberMovementDistance, state);
	_pkt_f32_read(&data->leftHandMovementDistance, state);
	_pkt_f32_read(&data->rightHandMovementDistance, state);
	if(state.context.protocolVersion < 8) {
		_pkt_f32_read(&data->songDuration, state);
	}
	_pkt_vi32_read(&data->levelEndStateType, state);
	_pkt_vi32_read(&data->levelEndAction, state);
	_pkt_f32_read(&data->energy, state);
	_pkt_vi32_read(&data->goodCutsCount, state);
	_pkt_vi32_read(&data->badCutsCount, state);
	_pkt_vi32_read(&data->missedCount, state);
	_pkt_vi32_read(&data->notGoodCount, state);
	_pkt_vi32_read(&data->okCount, state);
	if(state.context.protocolVersion < 8) {
		_pkt_vi32_read(&data->averageCutScore, state);
	}
	_pkt_vi32_read(&data->maxCutScore, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_read(&data->totalCutScore, state);
		_pkt_vi32_read(&data->goodCutsCountForNotesWithFullScoreScoringType, state);
		_pkt_i32_read(&data->averageCenterDistanceCutScoreForNotesWithFullScoreScoringType, state);
		_pkt_i32_read(&data->averageCutScoreForNotesWithFullScoreScoringType, state);
	}
	if(state.context.protocolVersion < 8) {
		_pkt_f32_read(&data->averageCutDistanceRawScore, state);
	}
	_pkt_vi32_read(&data->maxCombo, state);
	if(state.context.protocolVersion < 8) {
		_pkt_f32_read(&data->minDirDeviation, state);
		_pkt_f32_read(&data->maxDirDeviation, state);
		_pkt_f32_read(&data->averageDirDeviation, state);
		_pkt_f32_read(&data->minTimeDeviation, state);
		_pkt_f32_read(&data->maxTimeDeviation, state);
		_pkt_f32_read(&data->averageTimeDeviation, state);
	}
	_pkt_f32_read(&data->endSongTime, state);
	if(state.context.gameVersion >= GameVersion_1_37_1) {
		_pkt_b_read(&data->invalidated, state);
	}
}
static void _pkt_LevelCompletionResults_write(const struct LevelCompletionResults *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "LevelCompletionResults");
	_pkt_GameplayModifiers_write(&data->gameplayModifiers, state);
	_pkt_vi32_write(&data->modifiedScore, state);
	_pkt_vi32_write(&data->multipliedScore, state);
	_pkt_vi32_write(&data->rank, state);
	_pkt_b_write(&data->fullCombo, state);
	_pkt_f32_write(&data->leftSaberMovementDistance, state);
	_pkt_f32_write(&data->rightSaberMovementDistance, state);
	_pkt_f32_write(&data->leftHandMovementDistance, state);
	_pkt_f32_write(&data->rightHandMovementDistance, state);
	if(state.context.protocolVersion < 8) {
		_pkt_f32_write(&data->songDuration, state);
	}
	_pkt_vi32_write(&data->levelEndStateType, state);
	_pkt_vi32_write(&data->levelEndAction, state);
	_pkt_f32_write(&data->energy, state);
	_pkt_vi32_write(&data->goodCutsCount, state);
	_pkt_vi32_write(&data->badCutsCount, state);
	_pkt_vi32_write(&data->missedCount, state);
	_pkt_vi32_write(&data->notGoodCount, state);
	_pkt_vi32_write(&data->okCount, state);
	if(state.context.protocolVersion < 8) {
		_pkt_vi32_write(&data->averageCutScore, state);
	}
	_pkt_vi32_write(&data->maxCutScore, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_write(&data->totalCutScore, state);
		_pkt_vi32_write(&data->goodCutsCountForNotesWithFullScoreScoringType, state);
		_pkt_i32_write(&data->averageCenterDistanceCutScoreForNotesWithFullScoreScoringType, state);
		_pkt_i32_write(&data->averageCutScoreForNotesWithFullScoreScoringType, state);
	}
	if(state.context.protocolVersion < 8) {
		_pkt_f32_write(&data->averageCutDistanceRawScore, state);
	}
	_pkt_vi32_write(&data->maxCombo, state);
	if(state.context.protocolVersion < 8) {
		_pkt_f32_write(&data->minDirDeviation, state);
		_pkt_f32_write(&data->maxDirDeviation, state);
		_pkt_f32_write(&data->averageDirDeviation, state);
		_pkt_f32_write(&data->minTimeDeviation, state);
		_pkt_f32_write(&data->maxTimeDeviation, state);
		_pkt_f32_write(&data->averageTimeDeviation, state);
	}
	_pkt_f32_write(&data->endSongTime, state);
	if(state.context.gameVersion >= GameVersion_1_37_1) {
		_pkt_b_write(&data->invalidated, state);
	}
}
static void _pkt_MultiplayerLevelCompletionResults_read(struct MultiplayerLevelCompletionResults *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MultiplayerLevelCompletionResults");
	if(state.context.protocolVersion < 7) {
		_pkt_vi32_read(&data->levelEndState, state);
	}
	if(state.context.protocolVersion >= 7) {
		_pkt_vi32_read(&data->playerLevelEndState, state);
		_pkt_vi32_read(&data->playerLevelEndReason, state);
	}
	if((state.context.protocolVersion < 7) ? (data->levelEndState < MultiplayerLevelEndState_GivenUp) : (data->playerLevelEndState != MultiplayerPlayerLevelEndState_NotStarted)) {
		_pkt_LevelCompletionResults_read(&data->levelCompletionResults, state);
	}
}
static void _pkt_MultiplayerLevelCompletionResults_write(const struct MultiplayerLevelCompletionResults *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MultiplayerLevelCompletionResults");
	if(state.context.protocolVersion < 7) {
		_pkt_vi32_write(&data->levelEndState, state);
	}
	if(state.context.protocolVersion >= 7) {
		_pkt_vi32_write(&data->playerLevelEndState, state);
		_pkt_vi32_write(&data->playerLevelEndReason, state);
	}
	if((state.context.protocolVersion < 7) ? (data->levelEndState < MultiplayerLevelEndState_GivenUp) : (data->playerLevelEndState != MultiplayerPlayerLevelEndState_NotStarted)) {
		_pkt_LevelCompletionResults_write(&data->levelCompletionResults, state);
	}
}
static void _pkt_LevelFinished_read(struct LevelFinished *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "LevelFinished");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_MultiplayerLevelCompletionResults_read(&data->results, state);
	}
}
static void _pkt_LevelFinished_write(const struct LevelFinished *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "LevelFinished");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_MultiplayerLevelCompletionResults_write(&data->results, state);
	}
}
static void _pkt_ReturnToMenu_read(struct ReturnToMenu *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ReturnToMenu");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_ReturnToMenu_write(const struct ReturnToMenu *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ReturnToMenu");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_RequestReturnToMenu_read(struct RequestReturnToMenu *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RequestReturnToMenu");
	_pkt_RemoteProcedureCall_read(&data->base, state);
}
static void _pkt_RequestReturnToMenu_write(const struct RequestReturnToMenu *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RequestReturnToMenu");
	_pkt_RemoteProcedureCall_write(&data->base, state);
}
static void _pkt_NoteSpawnInfoNetSerializable_read(struct NoteSpawnInfoNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NoteSpawnInfoNetSerializable");
	_pkt_f32_read(&data->time, state);
	_pkt_vi32_read(&data->lineIndex, state);
	_pkt_vi32_read(&data->noteLineLayer, state);
	_pkt_vi32_read(&data->beforeJumpNoteLineLayer, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_read(&data->gameplayType, state);
		_pkt_vi32_read(&data->scoringType, state);
	}
	_pkt_vi32_read(&data->colorType, state);
	_pkt_vi32_read(&data->cutDirection, state);
	_pkt_f32_read(&data->timeToNextColorNote, state);
	_pkt_f32_read(&data->timeToPrevColorNote, state);
	_pkt_vi32_read(&data->flipLineIndex, state);
	_pkt_vi32_read(&data->flipYSide, state);
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_Vector3Serializable_read(&data->moveStartPos, state);
		_pkt_Vector3Serializable_read(&data->moveEndPos, state);
		_pkt_Vector3Serializable_read(&data->jumpEndPos, state);
		_pkt_f32_read(&data->jumpGravity, state);
		_pkt_f32_read(&data->moveDuration, state);
		_pkt_f32_read(&data->jumpDuration, state);
	}
	if(state.context.gameVersion >= GameVersion_1_40_0) {
		_pkt_Vector3Serializable_read(&data->moveStartOffset, state);
		_pkt_Vector3Serializable_read(&data->moveEndOffset, state);
		_pkt_Vector3Serializable_read(&data->jumpEndOffset, state);
		_pkt_f32_read(&data->gravityBase, state);
	}
	_pkt_f32_read(&data->rotation, state);
	_pkt_f32_read(&data->cutDirectionAngleOffset, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_f32_read(&data->cutSfxVolumeMultiplier, state);
	}
}
static void _pkt_NoteSpawnInfoNetSerializable_write(const struct NoteSpawnInfoNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NoteSpawnInfoNetSerializable");
	_pkt_f32_write(&data->time, state);
	_pkt_vi32_write(&data->lineIndex, state);
	_pkt_vi32_write(&data->noteLineLayer, state);
	_pkt_vi32_write(&data->beforeJumpNoteLineLayer, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_write(&data->gameplayType, state);
		_pkt_vi32_write(&data->scoringType, state);
	}
	_pkt_vi32_write(&data->colorType, state);
	_pkt_vi32_write(&data->cutDirection, state);
	_pkt_f32_write(&data->timeToNextColorNote, state);
	_pkt_f32_write(&data->timeToPrevColorNote, state);
	_pkt_vi32_write(&data->flipLineIndex, state);
	_pkt_vi32_write(&data->flipYSide, state);
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_Vector3Serializable_write(&data->moveStartPos, state);
		_pkt_Vector3Serializable_write(&data->moveEndPos, state);
		_pkt_Vector3Serializable_write(&data->jumpEndPos, state);
		_pkt_f32_write(&data->jumpGravity, state);
		_pkt_f32_write(&data->moveDuration, state);
		_pkt_f32_write(&data->jumpDuration, state);
	}
	if(state.context.gameVersion >= GameVersion_1_40_0) {
		_pkt_Vector3Serializable_write(&data->moveStartOffset, state);
		_pkt_Vector3Serializable_write(&data->moveEndOffset, state);
		_pkt_Vector3Serializable_write(&data->jumpEndOffset, state);
		_pkt_f32_write(&data->gravityBase, state);
	}
	_pkt_f32_write(&data->rotation, state);
	_pkt_f32_write(&data->cutDirectionAngleOffset, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_f32_write(&data->cutSfxVolumeMultiplier, state);
	}
}
static void _pkt_NoteSpawned_read(struct NoteSpawned *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NoteSpawned");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteSpawnInfoNetSerializable_read(&data->noteSpawnInfo, state);
	}
}
static void _pkt_NoteSpawned_write(const struct NoteSpawned *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NoteSpawned");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_NoteSpawnInfoNetSerializable_write(&data->noteSpawnInfo, state);
	}
}
static void _pkt_ObstacleSpawnInfoNetSerializable_read(struct ObstacleSpawnInfoNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ObstacleSpawnInfoNetSerializable");
	_pkt_f32_read(&data->time, state);
	_pkt_vi32_read(&data->lineIndex, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_read(&data->lineLayer, state);
	}
	if(state.context.protocolVersion < 8) {
		_pkt_vi32_read(&data->obstacleType, state);
	}
	_pkt_f32_read(&data->duration, state);
	_pkt_vi32_read(&data->width, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_read(&data->height, state);
	}
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_Vector3Serializable_read(&data->moveStartPos, state);
		_pkt_Vector3Serializable_read(&data->moveEndPos, state);
		_pkt_Vector3Serializable_read(&data->jumpEndPos, state);
	}
	if(state.context.gameVersion >= GameVersion_1_40_0) {
		_pkt_Vector3Serializable_read(&data->moveOffset, state);
		_pkt_f32_read(&data->obstacleWidth, state);
	}
	_pkt_f32_read(&data->obstacleHeight, state);
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_f32_read(&data->moveDuration, state);
		_pkt_f32_read(&data->jumpDuration, state);
		_pkt_f32_read(&data->noteLinesDistance, state);
	}
	_pkt_f32_read(&data->rotation, state);
}
static void _pkt_ObstacleSpawnInfoNetSerializable_write(const struct ObstacleSpawnInfoNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ObstacleSpawnInfoNetSerializable");
	_pkt_f32_write(&data->time, state);
	_pkt_vi32_write(&data->lineIndex, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_write(&data->lineLayer, state);
	}
	if(state.context.protocolVersion < 8) {
		_pkt_vi32_write(&data->obstacleType, state);
	}
	_pkt_f32_write(&data->duration, state);
	_pkt_vi32_write(&data->width, state);
	if(state.context.protocolVersion >= 8) {
		_pkt_vi32_write(&data->height, state);
	}
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_Vector3Serializable_write(&data->moveStartPos, state);
		_pkt_Vector3Serializable_write(&data->moveEndPos, state);
		_pkt_Vector3Serializable_write(&data->jumpEndPos, state);
	}
	if(state.context.gameVersion >= GameVersion_1_40_0) {
		_pkt_Vector3Serializable_write(&data->moveOffset, state);
		_pkt_f32_write(&data->obstacleWidth, state);
	}
	_pkt_f32_write(&data->obstacleHeight, state);
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_f32_write(&data->moveDuration, state);
		_pkt_f32_write(&data->jumpDuration, state);
		_pkt_f32_write(&data->noteLinesDistance, state);
	}
	_pkt_f32_write(&data->rotation, state);
}
static void _pkt_ObstacleSpawned_read(struct ObstacleSpawned *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ObstacleSpawned");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_ObstacleSpawnInfoNetSerializable_read(&data->obstacleSpawnInfo, state);
	}
}
static void _pkt_ObstacleSpawned_write(const struct ObstacleSpawned *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ObstacleSpawned");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_ObstacleSpawnInfoNetSerializable_write(&data->obstacleSpawnInfo, state);
	}
}
static void _pkt_SliderSpawnInfoNetSerializable_read(struct SliderSpawnInfoNetSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SliderSpawnInfoNetSerializable");
	_pkt_vi32_read(&data->colorType, state);
	_pkt_vi32_read(&data->sliderType, state);
	_pkt_b_read(&data->hasHeadNote, state);
	_pkt_f32_read(&data->headTime, state);
	_pkt_vi32_read(&data->headLineIndex, state);
	_pkt_vi32_read(&data->headLineLayer, state);
	_pkt_vi32_read(&data->headBeforeJumpLineLayer, state);
	_pkt_f32_read(&data->headControlPointLengthMultiplier, state);
	_pkt_vi32_read(&data->headCutDirection, state);
	_pkt_f32_read(&data->headCutDirectionAngleOffset, state);
	_pkt_b_read(&data->hasTailNote, state);
	_pkt_f32_read(&data->tailTime, state);
	_pkt_vi32_read(&data->tailLineIndex, state);
	_pkt_vi32_read(&data->tailLineLayer, state);
	_pkt_vi32_read(&data->tailBeforeJumpLineLayer, state);
	_pkt_f32_read(&data->tailControlPointLengthMultiplier, state);
	_pkt_vi32_read(&data->tailCutDirection, state);
	_pkt_f32_read(&data->tailCutDirectionAngleOffset, state);
	_pkt_vi32_read(&data->midAnchorMode, state);
	_pkt_vi32_read(&data->sliceCount, state);
	_pkt_f32_read(&data->squishAmount, state);
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_Vector3Serializable_read(&data->headMoveStartPos, state);
		_pkt_Vector3Serializable_read(&data->headJumpStartPos, state);
		_pkt_Vector3Serializable_read(&data->headJumpEndPos, state);
		_pkt_f32_read(&data->headJumpGravity, state);
		_pkt_Vector3Serializable_read(&data->tailMoveStartPos, state);
		_pkt_Vector3Serializable_read(&data->tailJumpStartPos, state);
		_pkt_Vector3Serializable_read(&data->tailJumpEndPos, state);
		_pkt_f32_read(&data->tailJumpGravity, state);
		_pkt_f32_read(&data->moveDuration, state);
		_pkt_f32_read(&data->jumpDuration, state);
	}
	if(state.context.gameVersion >= GameVersion_1_40_0) {
		_pkt_Vector3Serializable_read(&data->headNoteOffset, state);
		_pkt_f32_read(&data->headGravityBase, state);
		_pkt_Vector3Serializable_read(&data->tailNoteOffset, state);
		_pkt_f32_read(&data->tailGravityBase, state);
	}
	_pkt_f32_read(&data->rotation, state);
}
static void _pkt_SliderSpawnInfoNetSerializable_write(const struct SliderSpawnInfoNetSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SliderSpawnInfoNetSerializable");
	_pkt_vi32_write(&data->colorType, state);
	_pkt_vi32_write(&data->sliderType, state);
	_pkt_b_write(&data->hasHeadNote, state);
	_pkt_f32_write(&data->headTime, state);
	_pkt_vi32_write(&data->headLineIndex, state);
	_pkt_vi32_write(&data->headLineLayer, state);
	_pkt_vi32_write(&data->headBeforeJumpLineLayer, state);
	_pkt_f32_write(&data->headControlPointLengthMultiplier, state);
	_pkt_vi32_write(&data->headCutDirection, state);
	_pkt_f32_write(&data->headCutDirectionAngleOffset, state);
	_pkt_b_write(&data->hasTailNote, state);
	_pkt_f32_write(&data->tailTime, state);
	_pkt_vi32_write(&data->tailLineIndex, state);
	_pkt_vi32_write(&data->tailLineLayer, state);
	_pkt_vi32_write(&data->tailBeforeJumpLineLayer, state);
	_pkt_f32_write(&data->tailControlPointLengthMultiplier, state);
	_pkt_vi32_write(&data->tailCutDirection, state);
	_pkt_f32_write(&data->tailCutDirectionAngleOffset, state);
	_pkt_vi32_write(&data->midAnchorMode, state);
	_pkt_vi32_write(&data->sliceCount, state);
	_pkt_f32_write(&data->squishAmount, state);
	if(state.context.gameVersion < GameVersion_1_40_0) {
		_pkt_Vector3Serializable_write(&data->headMoveStartPos, state);
		_pkt_Vector3Serializable_write(&data->headJumpStartPos, state);
		_pkt_Vector3Serializable_write(&data->headJumpEndPos, state);
		_pkt_f32_write(&data->headJumpGravity, state);
		_pkt_Vector3Serializable_write(&data->tailMoveStartPos, state);
		_pkt_Vector3Serializable_write(&data->tailJumpStartPos, state);
		_pkt_Vector3Serializable_write(&data->tailJumpEndPos, state);
		_pkt_f32_write(&data->tailJumpGravity, state);
		_pkt_f32_write(&data->moveDuration, state);
		_pkt_f32_write(&data->jumpDuration, state);
	}
	if(state.context.gameVersion >= GameVersion_1_40_0) {
		_pkt_Vector3Serializable_write(&data->headNoteOffset, state);
		_pkt_f32_write(&data->headGravityBase, state);
		_pkt_Vector3Serializable_write(&data->tailNoteOffset, state);
		_pkt_f32_write(&data->tailGravityBase, state);
	}
	_pkt_f32_write(&data->rotation, state);
}
static void _pkt_SliderSpawned_read(struct SliderSpawned *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SliderSpawned");
	_pkt_RemoteProcedureCall_read(&data->base, state);
	_pkt_RemoteProcedureCallFlags_read(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_read(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_SliderSpawnInfoNetSerializable_read(&data->sliderSpawnInfo, state);
	}
}
static void _pkt_SliderSpawned_write(const struct SliderSpawned *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SliderSpawned");
	_pkt_RemoteProcedureCall_write(&data->base, state);
	_pkt_RemoteProcedureCallFlags_write(&data->flags, state);
	if(data->flags.hasValue0) {
		_pkt_f32_write(&data->songTime, state);
	}
	if(data->flags.hasValue1) {
		_pkt_SliderSpawnInfoNetSerializable_write(&data->sliderSpawnInfo, state);
	}
}
static void _pkt_GameplayRpc_read(struct GameplayRpc *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GameplayRpc");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: _pkt_SetGameplaySceneSyncFinish_read(&data->setGameplaySceneSyncFinish, state); break;
		case GameplayRpcType_SetGameplaySceneReady: _pkt_SetGameplaySceneReady_read(&data->setGameplaySceneReady, state); break;
		case GameplayRpcType_GetGameplaySceneReady: _pkt_GetGameplaySceneReady_read(&data->getGameplaySceneReady, state); break;
		case GameplayRpcType_SetActivePlayerFailedToConnect: _pkt_SetActivePlayerFailedToConnect_read(&data->setActivePlayerFailedToConnect, state); break;
		case GameplayRpcType_SetGameplaySongReady: _pkt_SetGameplaySongReady_read(&data->setGameplaySongReady, state); break;
		case GameplayRpcType_GetGameplaySongReady: _pkt_GetGameplaySongReady_read(&data->getGameplaySongReady, state); break;
		case GameplayRpcType_SetSongStartTime: _pkt_SetSongStartTime_read(&data->setSongStartTime, state); break;
		case GameplayRpcType_NoteCut: _pkt_NoteCut_read(&data->noteCut, state); break;
		case GameplayRpcType_NoteMissed: _pkt_NoteMissed_read(&data->noteMissed, state); break;
		case GameplayRpcType_LevelFinished: _pkt_LevelFinished_read(&data->levelFinished, state); break;
		case GameplayRpcType_ReturnToMenu: _pkt_ReturnToMenu_read(&data->returnToMenu, state); break;
		case GameplayRpcType_RequestReturnToMenu: _pkt_RequestReturnToMenu_read(&data->requestReturnToMenu, state); break;
		case GameplayRpcType_NoteSpawned: _pkt_NoteSpawned_read(&data->noteSpawned, state); break;
		case GameplayRpcType_ObstacleSpawned: _pkt_ObstacleSpawned_read(&data->obstacleSpawned, state); break;
		case GameplayRpcType_SliderSpawned: _pkt_SliderSpawned_read(&data->sliderSpawned, state); break;
		default: uprintf("Invalid value for enum `GameplayRpcType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_GameplayRpc_write(const struct GameplayRpc *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GameplayRpc");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case GameplayRpcType_SetGameplaySceneSyncFinish: _pkt_SetGameplaySceneSyncFinish_write(&data->setGameplaySceneSyncFinish, state); break;
		case GameplayRpcType_SetGameplaySceneReady: _pkt_SetGameplaySceneReady_write(&data->setGameplaySceneReady, state); break;
		case GameplayRpcType_GetGameplaySceneReady: _pkt_GetGameplaySceneReady_write(&data->getGameplaySceneReady, state); break;
		case GameplayRpcType_SetActivePlayerFailedToConnect: _pkt_SetActivePlayerFailedToConnect_write(&data->setActivePlayerFailedToConnect, state); break;
		case GameplayRpcType_SetGameplaySongReady: _pkt_SetGameplaySongReady_write(&data->setGameplaySongReady, state); break;
		case GameplayRpcType_GetGameplaySongReady: _pkt_GetGameplaySongReady_write(&data->getGameplaySongReady, state); break;
		case GameplayRpcType_SetSongStartTime: _pkt_SetSongStartTime_write(&data->setSongStartTime, state); break;
		case GameplayRpcType_NoteCut: _pkt_NoteCut_write(&data->noteCut, state); break;
		case GameplayRpcType_NoteMissed: _pkt_NoteMissed_write(&data->noteMissed, state); break;
		case GameplayRpcType_LevelFinished: _pkt_LevelFinished_write(&data->levelFinished, state); break;
		case GameplayRpcType_ReturnToMenu: _pkt_ReturnToMenu_write(&data->returnToMenu, state); break;
		case GameplayRpcType_RequestReturnToMenu: _pkt_RequestReturnToMenu_write(&data->requestReturnToMenu, state); break;
		case GameplayRpcType_NoteSpawned: _pkt_NoteSpawned_write(&data->noteSpawned, state); break;
		case GameplayRpcType_ObstacleSpawned: _pkt_ObstacleSpawned_write(&data->obstacleSpawned, state); break;
		case GameplayRpcType_SliderSpawned: _pkt_SliderSpawned_write(&data->sliderSpawned, state); break;
		default: uprintf("Invalid value for enum `GameplayRpcType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_PoseSerializable_read(struct PoseSerializable *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PoseSerializable");
	_pkt_Vector3Serializable_read(&data->position, state);
	_pkt_QuaternionSerializable_read(&data->rotation, state);
}
static void _pkt_PoseSerializable_write(const struct PoseSerializable *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PoseSerializable");
	_pkt_Vector3Serializable_write(&data->position, state);
	_pkt_QuaternionSerializable_write(&data->rotation, state);
}
static void _pkt_NodePoseSyncState1_read(struct NodePoseSyncState1 *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NodePoseSyncState1");
	_pkt_PoseSerializable_read(&data->head, state);
	_pkt_PoseSerializable_read(&data->leftController, state);
	_pkt_PoseSerializable_read(&data->rightController, state);
}
static void _pkt_NodePoseSyncState1_write(const struct NodePoseSyncState1 *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NodePoseSyncState1");
	_pkt_PoseSerializable_write(&data->head, state);
	_pkt_PoseSerializable_write(&data->leftController, state);
	_pkt_PoseSerializable_write(&data->rightController, state);
}
static void _pkt_SyncStateId_read(struct SyncStateId *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SyncStateId");
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->id = bitfield0 >> 0 & 127;
	data->same = bitfield0 >> 7 & 1;
}
static void _pkt_SyncStateId_write(const struct SyncStateId *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SyncStateId");
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->id & 127u) << 0;
	bitfield0 |= (data->same & 1u) << 7;
	_pkt_u8_write(&bitfield0, state);
}
static void _pkt_NodePoseSyncState_read(struct NodePoseSyncState *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NodePoseSyncState");
	_pkt_SyncStateId_read(&data->id, state);
	_pkt_UTimestamp_read(&data->time, state);
	_pkt_NodePoseSyncState1_read(&data->state, state);
}
static void _pkt_NodePoseSyncState_write(const struct NodePoseSyncState *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NodePoseSyncState");
	_pkt_SyncStateId_write(&data->id, state);
	_pkt_UTimestamp_write(&data->time, state);
	_pkt_NodePoseSyncState1_write(&data->state, state);
}
static void _pkt_StandardScoreSyncState_read(struct StandardScoreSyncState *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "StandardScoreSyncState");
	_pkt_vi32_read(&data->modifiedScore, state);
	_pkt_vi32_read(&data->rawScore, state);
	_pkt_vi32_read(&data->immediateMaxPossibleRawScore, state);
	_pkt_vi32_read(&data->combo, state);
	_pkt_vi32_read(&data->multiplier, state);
}
static void _pkt_StandardScoreSyncState_write(const struct StandardScoreSyncState *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "StandardScoreSyncState");
	_pkt_vi32_write(&data->modifiedScore, state);
	_pkt_vi32_write(&data->rawScore, state);
	_pkt_vi32_write(&data->immediateMaxPossibleRawScore, state);
	_pkt_vi32_write(&data->combo, state);
	_pkt_vi32_write(&data->multiplier, state);
}
static void _pkt_ScoreSyncState_read(struct ScoreSyncState *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ScoreSyncState");
	_pkt_SyncStateId_read(&data->id, state);
	_pkt_UTimestamp_read(&data->time, state);
	_pkt_StandardScoreSyncState_read(&data->state, state);
}
static void _pkt_ScoreSyncState_write(const struct ScoreSyncState *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ScoreSyncState");
	_pkt_SyncStateId_write(&data->id, state);
	_pkt_UTimestamp_write(&data->time, state);
	_pkt_StandardScoreSyncState_write(&data->state, state);
}
static void _pkt_NodePoseSyncStateDelta_read(struct NodePoseSyncStateDelta *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NodePoseSyncStateDelta");
	_pkt_SyncStateId_read(&data->baseId, state);
	_pkt_vi32_read(&data->timeOffsetMs, state);
	if(data->baseId.same == 0) {
		_pkt_NodePoseSyncState1_read(&data->delta, state);
	}
}
static void _pkt_NodePoseSyncStateDelta_write(const struct NodePoseSyncStateDelta *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NodePoseSyncStateDelta");
	_pkt_SyncStateId_write(&data->baseId, state);
	_pkt_vi32_write(&data->timeOffsetMs, state);
	if(data->baseId.same == 0) {
		_pkt_NodePoseSyncState1_write(&data->delta, state);
	}
}
static void _pkt_ScoreSyncStateDelta_read(struct ScoreSyncStateDelta *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ScoreSyncStateDelta");
	_pkt_SyncStateId_read(&data->baseId, state);
	_pkt_vi32_read(&data->timeOffsetMs, state);
	if(data->baseId.same == 0) {
		_pkt_StandardScoreSyncState_read(&data->delta, state);
	}
}
static void _pkt_ScoreSyncStateDelta_write(const struct ScoreSyncStateDelta *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ScoreSyncStateDelta");
	_pkt_SyncStateId_write(&data->baseId, state);
	_pkt_vi32_write(&data->timeOffsetMs, state);
	if(data->baseId.same == 0) {
		_pkt_StandardScoreSyncState_write(&data->delta, state);
	}
}
static void _pkt_MpRequirementSet_read(struct MpRequirementSet *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpRequirementSet");
	_pkt_u8_read(&data->difficulty, state);
	_pkt_u8_read(&data->requirements_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->requirements_len), 16, "MpRequirementSet.requirements"); i < count; ++i)
		_pkt_String_read(&data->requirements[i], state);
}
static void _pkt_MpRequirementSet_write(const struct MpRequirementSet *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpRequirementSet");
	_pkt_u8_write(&data->difficulty, state);
	_pkt_u8_write(&data->requirements_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->requirements_len), 16, "MpRequirementSet.requirements"); i < count; ++i)
		_pkt_String_write(&data->requirements[i], state);
}
static void _pkt_MpMapColor_read(struct MpMapColor *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpMapColor");
	_pkt_u8_read(&data->difficulty, state);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->have_colorLeft = bitfield0 >> 0 & 1;
	data->have_colorRight = bitfield0 >> 1 & 1;
	data->have_envColorLeft = bitfield0 >> 2 & 1;
	data->have_envColorRight = bitfield0 >> 3 & 1;
	data->have_envColorLeftBoost = bitfield0 >> 4 & 1;
	data->have_envColorRightBoost = bitfield0 >> 5 & 1;
	data->have_obstacleColor = bitfield0 >> 6 & 1;
	if(data->have_colorLeft) {
		_pkt_ColorNoAlphaSerializable_read(&data->colorLeft, state);
	}
	if(data->have_colorRight) {
		_pkt_ColorNoAlphaSerializable_read(&data->colorRight, state);
	}
	if(data->have_envColorLeft) {
		_pkt_ColorNoAlphaSerializable_read(&data->envColorLeft, state);
	}
	if(data->have_envColorRight) {
		_pkt_ColorNoAlphaSerializable_read(&data->envColorRight, state);
	}
	if(data->have_envColorLeftBoost) {
		_pkt_ColorNoAlphaSerializable_read(&data->envColorLeftBoost, state);
	}
	if(data->have_envColorRightBoost) {
		_pkt_ColorNoAlphaSerializable_read(&data->envColorRightBoost, state);
	}
	if(data->have_obstacleColor) {
		_pkt_ColorNoAlphaSerializable_read(&data->obstacleColor, state);
	}
}
static void _pkt_MpMapColor_write(const struct MpMapColor *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpMapColor");
	_pkt_u8_write(&data->difficulty, state);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->have_colorLeft & 1u) << 0;
	bitfield0 |= (data->have_colorRight & 1u) << 1;
	bitfield0 |= (data->have_envColorLeft & 1u) << 2;
	bitfield0 |= (data->have_envColorRight & 1u) << 3;
	bitfield0 |= (data->have_envColorLeftBoost & 1u) << 4;
	bitfield0 |= (data->have_envColorRightBoost & 1u) << 5;
	bitfield0 |= (data->have_obstacleColor & 1u) << 6;
	_pkt_u8_write(&bitfield0, state);
	if(data->have_colorLeft) {
		_pkt_ColorNoAlphaSerializable_write(&data->colorLeft, state);
	}
	if(data->have_colorRight) {
		_pkt_ColorNoAlphaSerializable_write(&data->colorRight, state);
	}
	if(data->have_envColorLeft) {
		_pkt_ColorNoAlphaSerializable_write(&data->envColorLeft, state);
	}
	if(data->have_envColorRight) {
		_pkt_ColorNoAlphaSerializable_write(&data->envColorRight, state);
	}
	if(data->have_envColorLeftBoost) {
		_pkt_ColorNoAlphaSerializable_write(&data->envColorLeftBoost, state);
	}
	if(data->have_envColorRightBoost) {
		_pkt_ColorNoAlphaSerializable_write(&data->envColorRightBoost, state);
	}
	if(data->have_obstacleColor) {
		_pkt_ColorNoAlphaSerializable_write(&data->obstacleColor, state);
	}
}
void _pkt_MpBeatmapPacket_read(struct MpBeatmapPacket *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpBeatmapPacket");
	_pkt_String_read(&data->levelHash, state);
	_pkt_LongString_read(&data->songName, state);
	_pkt_LongString_read(&data->songSubName, state);
	_pkt_LongString_read(&data->songAuthorName, state);
	_pkt_LongString_read(&data->levelAuthorName, state);
	_pkt_f32_read(&data->beatsPerMinute, state);
	_pkt_f32_read(&data->songDuration, state);
	_pkt_String_read(&data->characteristic, state);
	_pkt_u32_read(&data->difficulty, state);
	_pkt_u8_read(&data->requirementSets_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->requirementSets_len), 5, "MpBeatmapPacket.requirementSets"); i < count; ++i)
		_pkt_MpRequirementSet_read(&data->requirementSets[i], state);
	_pkt_u8_read(&data->contributors_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->contributors_len*3), 24, "MpBeatmapPacket.contributors"); i < count; ++i)
		_pkt_String_read(&data->contributors[i], state);
	_pkt_u8_read(&data->mapColors_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->mapColors_len), 5, "MpBeatmapPacket.mapColors"); i < count; ++i)
		_pkt_MpMapColor_read(&data->mapColors[i], state);
}
static void _pkt_MpBeatmapPacket_write(const struct MpBeatmapPacket *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpBeatmapPacket");
	_pkt_String_write(&data->levelHash, state);
	_pkt_LongString_write(&data->songName, state);
	_pkt_LongString_write(&data->songSubName, state);
	_pkt_LongString_write(&data->songAuthorName, state);
	_pkt_LongString_write(&data->levelAuthorName, state);
	_pkt_f32_write(&data->beatsPerMinute, state);
	_pkt_f32_write(&data->songDuration, state);
	_pkt_String_write(&data->characteristic, state);
	_pkt_u32_write(&data->difficulty, state);
	_pkt_u8_write(&data->requirementSets_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->requirementSets_len), 5, "MpBeatmapPacket.requirementSets"); i < count; ++i)
		_pkt_MpRequirementSet_write(&data->requirementSets[i], state);
	_pkt_u8_write(&data->contributors_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->contributors_len*3), 24, "MpBeatmapPacket.contributors"); i < count; ++i)
		_pkt_String_write(&data->contributors[i], state);
	_pkt_u8_write(&data->mapColors_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->mapColors_len), 5, "MpBeatmapPacket.mapColors"); i < count; ++i)
		_pkt_MpMapColor_write(&data->mapColors[i], state);
}
static void _pkt_MpPerPlayerPacket_read(struct MpPerPlayerPacket *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpPerPlayerPacket");
	_pkt_b_read(&data->ppdEnabled, state);
	_pkt_b_read(&data->ppmEnabled, state);
}
static void _pkt_MpPerPlayerPacket_write(const struct MpPerPlayerPacket *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpPerPlayerPacket");
	_pkt_b_write(&data->ppdEnabled, state);
	_pkt_b_write(&data->ppmEnabled, state);
}
static void _pkt_GetMpPerPlayerPacket_read(struct GetMpPerPlayerPacket *restrict data, struct PacketRead parent) {
}
static void _pkt_GetMpPerPlayerPacket_write(const struct GetMpPerPlayerPacket *restrict data, struct PacketWrite parent) {
}
static void _pkt_MpPlayerData_read(struct MpPlayerData *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpPlayerData");
	_pkt_String_read(&data->platformId, state);
	_pkt_i32_read(&data->platform, state);
	if(state.context.gameVersion >= GameVersion_1_29_4) {
		_pkt_String_read(&data->gameVersion, state);
	}
}
static void _pkt_MpPlayerData_write(const struct MpPlayerData *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpPlayerData");
	_pkt_String_write(&data->platformId, state);
	_pkt_i32_write(&data->platform, state);
	if(state.context.gameVersion >= GameVersion_1_29_4) {
		_pkt_String_write(&data->gameVersion, state);
	}
}
static void _pkt_MpexPlayerData_read(struct MpexPlayerData *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpexPlayerData");
	_pkt_String_read(&data->nameColor, state);
}
static void _pkt_MpexPlayerData_write(const struct MpexPlayerData *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpexPlayerData");
	_pkt_String_write(&data->nameColor, state);
}
static void _pkt_CustomAvatarPacket_read(struct CustomAvatarPacket *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "CustomAvatarPacket");
	_pkt_String_read(&data->hash, state);
	_pkt_f32_read(&data->scale, state);
	_pkt_f32_read(&data->floor, state);
}
static void _pkt_CustomAvatarPacket_write(const struct CustomAvatarPacket *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "CustomAvatarPacket");
	_pkt_String_write(&data->hash, state);
	_pkt_f32_write(&data->scale, state);
	_pkt_f32_write(&data->floor, state);
}
static void _pkt_MpcCapabilitiesPacket_read(struct MpcCapabilitiesPacket *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpcCapabilitiesPacket");
	_pkt_vu32_read(&data->protocolVersion, state);
	if(data->protocolVersion == 1) {
		_pkt_b_read(&data->canText, state);
	}
}
static void _pkt_MpcCapabilitiesPacket_write(const struct MpcCapabilitiesPacket *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpcCapabilitiesPacket");
	_pkt_vu32_write(&data->protocolVersion, state);
	if(data->protocolVersion == 1) {
		_pkt_b_write(&data->canText, state);
	}
}
static void _pkt_MpcTextChatPacket_read(struct MpcTextChatPacket *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpcTextChatPacket");
	_pkt_vu32_read(&data->protocolVersion, state);
	if(data->protocolVersion == 1) {
		_pkt_LongString_read(&data->text, state);
	}
}
static void _pkt_MpcTextChatPacket_write(const struct MpcTextChatPacket *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpcTextChatPacket");
	_pkt_vu32_write(&data->protocolVersion, state);
	if(data->protocolVersion == 1) {
		_pkt_LongString_write(&data->text, state);
	}
}
static void _pkt_MpCore_read(struct MpCore *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MpCore");
	_pkt_String_read(&data->type, state);
	switch(MpCoreType_From(&data->type)) {
		case MpCoreType_MpcTextChatPacket: _pkt_MpcTextChatPacket_read(&data->mpcTextChat, state); break;
		case MpCoreType_MpBeatmapPacket: _pkt_MpBeatmapPacket_read(&data->mpBeatmap, state); break;
		case MpCoreType_MpPerPlayerPacket: _pkt_MpPerPlayerPacket_read(&data->mpPerPlayer, state); break;
		case MpCoreType_GetMpPerPlayerPacket: _pkt_GetMpPerPlayerPacket_read(&data->getMpPerPlayer, state); break;
		case MpCoreType_CustomAvatarPacket: _pkt_CustomAvatarPacket_read(&data->customAvatar, state); break;
		case MpCoreType_MpcCapabilitiesPacket: _pkt_MpcCapabilitiesPacket_read(&data->mpcCapabilities, state); break;
		case MpCoreType_MpPlayerData: _pkt_MpPlayerData_read(&data->mpPlayerData, state); break;
		case MpCoreType_MpexPlayerData: _pkt_MpexPlayerData_read(&data->mpexPlayerData, state); break;
		default: uprintf("Invalid value for enum `MpCoreType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_MpCore_write(const struct MpCore *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MpCore");
	_pkt_String_write(&data->type, state);
	switch(MpCoreType_From(&data->type)) {
		case MpCoreType_MpcTextChatPacket: _pkt_MpcTextChatPacket_write(&data->mpcTextChat, state); break;
		case MpCoreType_MpBeatmapPacket: _pkt_MpBeatmapPacket_write(&data->mpBeatmap, state); break;
		case MpCoreType_MpPerPlayerPacket: _pkt_MpPerPlayerPacket_write(&data->mpPerPlayer, state); break;
		case MpCoreType_GetMpPerPlayerPacket: _pkt_GetMpPerPlayerPacket_write(&data->getMpPerPlayer, state); break;
		case MpCoreType_CustomAvatarPacket: _pkt_CustomAvatarPacket_write(&data->customAvatar, state); break;
		case MpCoreType_MpcCapabilitiesPacket: _pkt_MpcCapabilitiesPacket_write(&data->mpcCapabilities, state); break;
		case MpCoreType_MpPlayerData: _pkt_MpPlayerData_write(&data->mpPlayerData, state); break;
		case MpCoreType_MpexPlayerData: _pkt_MpexPlayerData_write(&data->mpexPlayerData, state); break;
		default: uprintf("Invalid value for enum `MpCoreType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_SyncTime_read(struct SyncTime *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SyncTime");
	_pkt_UTimestamp_read(&data->syncTime, state);
}
static void _pkt_SyncTime_write(const struct SyncTime *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SyncTime");
	_pkt_UTimestamp_write(&data->syncTime, state);
}
static void _pkt_PlayerConnected_read(struct PlayerConnected *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerConnected");
	_pkt_u8_read(&data->remoteConnectionId, state);
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_b_read(&data->isConnectionOwner, state);
}
static void _pkt_PlayerConnected_write(const struct PlayerConnected *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerConnected");
	_pkt_u8_write(&data->remoteConnectionId, state);
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_b_write(&data->isConnectionOwner, state);
}
static void _pkt_PlayerStateHash_read(struct PlayerStateHash *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerStateHash");
	_pkt_BitMask128_read(&data->bloomFilter, state);
}
static void _pkt_PlayerStateHash_write(const struct PlayerStateHash *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerStateHash");
	_pkt_BitMask128_write(&data->bloomFilter, state);
}
static void _pkt_Color32_read(struct Color32 *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Color32");
	_pkt_u8_read(&data->r, state);
	_pkt_u8_read(&data->g, state);
	_pkt_u8_read(&data->b, state);
	_pkt_u8_read(&data->a, state);
}
static void _pkt_Color32_write(const struct Color32 *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Color32");
	_pkt_u8_write(&data->r, state);
	_pkt_u8_write(&data->g, state);
	_pkt_u8_write(&data->b, state);
	_pkt_u8_write(&data->a, state);
}
static void _pkt_LegacyAvatarData_read(struct LegacyAvatarData *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "LegacyAvatarData");
	_pkt_String_read(&data->headTopId, state);
	_pkt_Color32_read(&data->headTopPrimaryColor, state);
	_pkt_Color32_read(&data->handsColor, state);
	_pkt_String_read(&data->clothesId, state);
	_pkt_Color32_read(&data->clothesPrimaryColor, state);
	_pkt_Color32_read(&data->clothesSecondaryColor, state);
	_pkt_Color32_read(&data->clothesDetailColor, state);
	for(uint32_t i = 0, count = 2; i < count; ++i)
		_pkt_Color32_read(&data->_unused[i], state);
	_pkt_String_read(&data->eyesId, state);
	_pkt_String_read(&data->mouthId, state);
	_pkt_Color32_read(&data->glassesColor, state);
	_pkt_Color32_read(&data->facialHairColor, state);
	_pkt_Color32_read(&data->headTopSecondaryColor, state);
	_pkt_String_read(&data->glassesId, state);
	_pkt_String_read(&data->facialHairId, state);
	_pkt_String_read(&data->handsId, state);
}
static void _pkt_LegacyAvatarData_write(const struct LegacyAvatarData *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "LegacyAvatarData");
	_pkt_String_write(&data->headTopId, state);
	_pkt_Color32_write(&data->headTopPrimaryColor, state);
	_pkt_Color32_write(&data->handsColor, state);
	_pkt_String_write(&data->clothesId, state);
	_pkt_Color32_write(&data->clothesPrimaryColor, state);
	_pkt_Color32_write(&data->clothesSecondaryColor, state);
	_pkt_Color32_write(&data->clothesDetailColor, state);
	for(uint32_t i = 0, count = 2; i < count; ++i)
		_pkt_Color32_write(&data->_unused[i], state);
	_pkt_String_write(&data->eyesId, state);
	_pkt_String_write(&data->mouthId, state);
	_pkt_Color32_write(&data->glassesColor, state);
	_pkt_Color32_write(&data->facialHairColor, state);
	_pkt_Color32_write(&data->headTopSecondaryColor, state);
	_pkt_String_write(&data->glassesId, state);
	_pkt_String_write(&data->facialHairId, state);
	_pkt_String_write(&data->handsId, state);
}
static void _pkt_WriterString_read(struct WriterString *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WriterString");
	_pkt_vu32_read(&data->length, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->length), 67, "WriterString.data"); i < count; ++i)
		_pkt_char_read(&data->data[i], state);
}
static void _pkt_WriterString_write(const struct WriterString *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WriterString");
	_pkt_vu32_write(&data->length, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->length), 67, "WriterString.data"); i < count; ++i)
		_pkt_char_write(&data->data[i], state);
}
static void _pkt_WriterColor_read(struct WriterColor *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WriterColor");
	_pkt_f32_read(&data->r, state);
	_pkt_f32_read(&data->g, state);
	_pkt_f32_read(&data->b, state);
	_pkt_f32_read(&data->a, state);
}
static void _pkt_WriterColor_write(const struct WriterColor *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WriterColor");
	_pkt_f32_write(&data->r, state);
	_pkt_f32_write(&data->g, state);
	_pkt_f32_write(&data->b, state);
	_pkt_f32_write(&data->a, state);
}
void _pkt_BeatAvatarData_read(struct BeatAvatarData *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BeatAvatarData");
	_pkt_WriterString_read(&data->headTopId, state);
	_pkt_WriterColor_read(&data->headTopPrimaryColor, state);
	_pkt_WriterColor_read(&data->headTopSecondaryColor, state);
	_pkt_WriterString_read(&data->glassesId, state);
	_pkt_WriterColor_read(&data->glassesColor, state);
	_pkt_WriterString_read(&data->facialHairId, state);
	_pkt_WriterColor_read(&data->facialHairColor, state);
	_pkt_WriterString_read(&data->handsId, state);
	_pkt_WriterColor_read(&data->handsColor, state);
	_pkt_WriterString_read(&data->clothesId, state);
	_pkt_WriterColor_read(&data->clothesPrimaryColor, state);
	_pkt_WriterColor_read(&data->clothesSecondaryColor, state);
	_pkt_WriterColor_read(&data->clothesDetailColor, state);
	_pkt_WriterString_read(&data->skinColorId, state);
	_pkt_WriterString_read(&data->eyesId, state);
	_pkt_WriterString_read(&data->mouthId, state);
}
void _pkt_BeatAvatarData_write(const struct BeatAvatarData *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BeatAvatarData");
	_pkt_WriterString_write(&data->headTopId, state);
	_pkt_WriterColor_write(&data->headTopPrimaryColor, state);
	_pkt_WriterColor_write(&data->headTopSecondaryColor, state);
	_pkt_WriterString_write(&data->glassesId, state);
	_pkt_WriterColor_write(&data->glassesColor, state);
	_pkt_WriterString_write(&data->facialHairId, state);
	_pkt_WriterColor_write(&data->facialHairColor, state);
	_pkt_WriterString_write(&data->handsId, state);
	_pkt_WriterColor_write(&data->handsColor, state);
	_pkt_WriterString_write(&data->clothesId, state);
	_pkt_WriterColor_write(&data->clothesPrimaryColor, state);
	_pkt_WriterColor_write(&data->clothesSecondaryColor, state);
	_pkt_WriterColor_write(&data->clothesDetailColor, state);
	_pkt_WriterString_write(&data->skinColorId, state);
	_pkt_WriterString_write(&data->eyesId, state);
	_pkt_WriterString_write(&data->mouthId, state);
}
static void _pkt_OpaqueAvatarData_read(struct OpaqueAvatarData *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "OpaqueAvatarData");
	_pkt_u32_read(&data->typeHash, state);
	_pkt_u16_read(&data->length, state);
	_pkt_raw_read(data->data, check_overflow(state, *state.head, (uint32_t)(data->length), 4096, "OpaqueAvatarData.data"), state);
}
static void _pkt_OpaqueAvatarData_write(const struct OpaqueAvatarData *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "OpaqueAvatarData");
	_pkt_u32_write(&data->typeHash, state);
	_pkt_u16_write(&data->length, state);
	_pkt_raw_write(data->data, check_overflow(state, *state.head, (uint32_t)(data->length), 4096, "OpaqueAvatarData.data"), state);
}
static void _pkt_MultiplayerAvatarsData_read(struct MultiplayerAvatarsData *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MultiplayerAvatarsData");
	if(state.context.protocolVersion < 9) {
		_pkt_LegacyAvatarData_read(&data->legacy, state);
	}
	if(state.context.protocolVersion >= 9) {
		_pkt_i32_read(&data->count, state);
		for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 6, "MultiplayerAvatarsData.avatars"); i < count; ++i)
			_pkt_OpaqueAvatarData_read(&data->avatars[i], state);
		_pkt_BitMask128_read(&data->supportedTypes, state);
	}
}
static void _pkt_MultiplayerAvatarsData_write(const struct MultiplayerAvatarsData *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MultiplayerAvatarsData");
	if(state.context.protocolVersion < 9) {
		_pkt_LegacyAvatarData_write(&data->legacy, state);
	}
	if(state.context.protocolVersion >= 9) {
		_pkt_i32_write(&data->count, state);
		for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->count), 6, "MultiplayerAvatarsData.avatars"); i < count; ++i)
			_pkt_OpaqueAvatarData_write(&data->avatars[i], state);
		_pkt_BitMask128_write(&data->supportedTypes, state);
	}
}
static void _pkt_PlayerIdentity_read(struct PlayerIdentity *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerIdentity");
	_pkt_PlayerStateHash_read(&data->playerState, state);
	if(state.context.gameVersion < GameVersion_1_42_0) {
		_pkt_MultiplayerAvatarsData_read(&data->playerAvatars, state);
	}
	_pkt_ByteArrayNetSerializable_read(&data->random, state);
	_pkt_ByteArrayNetSerializable_read(&data->publicEncryptionKey, state);
	if(state.context.gameVersion >= GameVersion_1_42_0) {
		_pkt_MultiplayerAvatarsData_read(&data->playerAvatars, state);
	}
}
static void _pkt_PlayerIdentity_write(const struct PlayerIdentity *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerIdentity");
	_pkt_PlayerStateHash_write(&data->playerState, state);
	if(state.context.gameVersion < GameVersion_1_42_0) {
		_pkt_MultiplayerAvatarsData_write(&data->playerAvatars, state);
	}
	_pkt_ByteArrayNetSerializable_write(&data->random, state);
	_pkt_ByteArrayNetSerializable_write(&data->publicEncryptionKey, state);
	if(state.context.gameVersion >= GameVersion_1_42_0) {
		_pkt_MultiplayerAvatarsData_write(&data->playerAvatars, state);
	}
}
static void _pkt_PlayerLatencyUpdate_read(struct PlayerLatencyUpdate *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerLatencyUpdate");
	_pkt_f32_read(&data->latency, state);
}
static void _pkt_PlayerLatencyUpdate_write(const struct PlayerLatencyUpdate *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerLatencyUpdate");
	_pkt_f32_write(&data->latency, state);
}
static void _pkt_PlayerDisconnected_read(struct PlayerDisconnected *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerDisconnected");
	_pkt_vi32_read(&data->disconnectedReason, state);
}
static void _pkt_PlayerDisconnected_write(const struct PlayerDisconnected *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerDisconnected");
	_pkt_vi32_write(&data->disconnectedReason, state);
}
static void _pkt_PlayerSortOrderUpdate_read(struct PlayerSortOrderUpdate *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerSortOrderUpdate");
	_pkt_String_read(&data->userId, state);
	_pkt_vi32_read(&data->sortIndex, state);
}
static void _pkt_PlayerSortOrderUpdate_write(const struct PlayerSortOrderUpdate *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerSortOrderUpdate");
	_pkt_String_write(&data->userId, state);
	_pkt_vi32_write(&data->sortIndex, state);
}
static void _pkt_Party_read(struct Party *restrict data, struct PacketRead parent) {
}
static void _pkt_Party_write(const struct Party *restrict data, struct PacketWrite parent) {
}
static void _pkt_MultiplayerSession_read(struct MultiplayerSession *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MultiplayerSession");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case MultiplayerSessionMessageType_MenuRpc: _pkt_MenuRpc_read(&data->menuRpc, state); break;
		case MultiplayerSessionMessageType_GameplayRpc: _pkt_GameplayRpc_read(&data->gameplayRpc, state); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: _pkt_NodePoseSyncState_read(&data->nodePoseSyncState, state); break;
		case MultiplayerSessionMessageType_ScoreSyncState: _pkt_ScoreSyncState_read(&data->scoreSyncState, state); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: _pkt_NodePoseSyncStateDelta_read(&data->nodePoseSyncStateDelta, state); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: _pkt_ScoreSyncStateDelta_read(&data->scoreSyncStateDelta, state); break;
		case MultiplayerSessionMessageType_MpCore: _pkt_MpCore_read(&data->mpCore, state); break;
		case MultiplayerSessionMessageType_BeatUpMessage: _pkt_BeatUpMessage_read(&data->beatUpMessage, state); break;
		default: uprintf("Invalid value for enum `MultiplayerSessionMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_MultiplayerSession_write(const struct MultiplayerSession *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MultiplayerSession");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case MultiplayerSessionMessageType_MenuRpc: _pkt_MenuRpc_write(&data->menuRpc, state); break;
		case MultiplayerSessionMessageType_GameplayRpc: _pkt_GameplayRpc_write(&data->gameplayRpc, state); break;
		case MultiplayerSessionMessageType_NodePoseSyncState: _pkt_NodePoseSyncState_write(&data->nodePoseSyncState, state); break;
		case MultiplayerSessionMessageType_ScoreSyncState: _pkt_ScoreSyncState_write(&data->scoreSyncState, state); break;
		case MultiplayerSessionMessageType_NodePoseSyncStateDelta: _pkt_NodePoseSyncStateDelta_write(&data->nodePoseSyncStateDelta, state); break;
		case MultiplayerSessionMessageType_ScoreSyncStateDelta: _pkt_ScoreSyncStateDelta_write(&data->scoreSyncStateDelta, state); break;
		case MultiplayerSessionMessageType_MpCore: _pkt_MpCore_write(&data->mpCore, state); break;
		case MultiplayerSessionMessageType_BeatUpMessage: _pkt_BeatUpMessage_write(&data->beatUpMessage, state); break;
		default: uprintf("Invalid value for enum `MultiplayerSessionMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_KickPlayer_read(struct KickPlayer *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "KickPlayer");
	_pkt_vi32_read(&data->disconnectedReason, state);
}
static void _pkt_KickPlayer_write(const struct KickPlayer *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "KickPlayer");
	_pkt_vi32_write(&data->disconnectedReason, state);
}
static void _pkt_PlayerStateUpdate_read(struct PlayerStateUpdate *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerStateUpdate");
	_pkt_PlayerStateHash_read(&data->playerState, state);
}
static void _pkt_PlayerStateUpdate_write(const struct PlayerStateUpdate *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerStateUpdate");
	_pkt_PlayerStateHash_write(&data->playerState, state);
}
static void _pkt_PlayerAvatarUpdate_read(struct PlayerAvatarUpdate *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PlayerAvatarUpdate");
	_pkt_MultiplayerAvatarsData_read(&data->playerAvatars, state);
}
static void _pkt_PlayerAvatarUpdate_write(const struct PlayerAvatarUpdate *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PlayerAvatarUpdate");
	_pkt_MultiplayerAvatarsData_write(&data->playerAvatars, state);
}
static void _pkt_PingMessage_read(struct PingMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PingMessage");
	_pkt_UTimestamp_read(&data->pingTime, state);
}
static void _pkt_PingMessage_write(const struct PingMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PingMessage");
	_pkt_UTimestamp_write(&data->pingTime, state);
}
static void _pkt_PongMessage_read(struct PongMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PongMessage");
	_pkt_UTimestamp_read(&data->pingTime, state);
}
static void _pkt_PongMessage_write(const struct PongMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PongMessage");
	_pkt_UTimestamp_write(&data->pingTime, state);
}
static void _pkt_GameSpecificMessage_read(struct GameSpecificMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GameSpecificMessage");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case GameSpecificMessageType_PlayerAvatarUpdate: _pkt_PlayerAvatarUpdate_read(&data->playerAvatarUpdate, state); break;
		default: uprintf("Invalid value for enum `GameSpecificMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_GameSpecificMessage_write(const struct GameSpecificMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GameSpecificMessage");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case GameSpecificMessageType_PlayerAvatarUpdate: _pkt_PlayerAvatarUpdate_write(&data->playerAvatarUpdate, state); break;
		default: uprintf("Invalid value for enum `GameSpecificMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_InternalMessage_read(struct InternalMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "InternalMessage");
	_pkt_InternalMessageType_Fixed_read(&data->type, state);
	switch(data->type) {
		case InternalMessageType_SyncTime: _pkt_SyncTime_read(&data->syncTime, state); break;
		case InternalMessageType_PlayerConnected: _pkt_PlayerConnected_read(&data->playerConnected, state); break;
		case InternalMessageType_PlayerIdentity: _pkt_PlayerIdentity_read(&data->playerIdentity, state); break;
		case InternalMessageType_PlayerLatencyUpdate: _pkt_PlayerLatencyUpdate_read(&data->playerLatencyUpdate, state); break;
		case InternalMessageType_PlayerDisconnected: _pkt_PlayerDisconnected_read(&data->playerDisconnected, state); break;
		case InternalMessageType_PlayerSortOrderUpdate: _pkt_PlayerSortOrderUpdate_read(&data->playerSortOrderUpdate, state); break;
		case InternalMessageType_Party: _pkt_Party_read(&data->party, state); break;
		case InternalMessageType_MultiplayerSession: _pkt_MultiplayerSession_read(&data->multiplayerSession, state); break;
		case InternalMessageType_KickPlayer: _pkt_KickPlayer_read(&data->kickPlayer, state); break;
		case InternalMessageType_PlayerStateUpdate: _pkt_PlayerStateUpdate_read(&data->playerStateUpdate, state); break;
		case InternalMessageType_PlayerAvatarUpdate: _pkt_PlayerAvatarUpdate_read(&data->playerAvatarUpdate, state); break;
		case InternalMessageType_PingMessage: _pkt_PingMessage_read(&data->pingMessage, state); break;
		case InternalMessageType_PongMessage: _pkt_PongMessage_read(&data->pongMessage, state); break;
		case InternalMessageType_GameSpecificMessage: _pkt_GameSpecificMessage_read(&data->gameSpecificMessage, state); break;
		default: uprintf("Invalid value for enum `InternalMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_InternalMessage_write(const struct InternalMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "InternalMessage");
	_pkt_InternalMessageType_Fixed_write(&data->type, state);
	switch(data->type) {
		case InternalMessageType_SyncTime: _pkt_SyncTime_write(&data->syncTime, state); break;
		case InternalMessageType_PlayerConnected: _pkt_PlayerConnected_write(&data->playerConnected, state); break;
		case InternalMessageType_PlayerIdentity: _pkt_PlayerIdentity_write(&data->playerIdentity, state); break;
		case InternalMessageType_PlayerLatencyUpdate: _pkt_PlayerLatencyUpdate_write(&data->playerLatencyUpdate, state); break;
		case InternalMessageType_PlayerDisconnected: _pkt_PlayerDisconnected_write(&data->playerDisconnected, state); break;
		case InternalMessageType_PlayerSortOrderUpdate: _pkt_PlayerSortOrderUpdate_write(&data->playerSortOrderUpdate, state); break;
		case InternalMessageType_Party: _pkt_Party_write(&data->party, state); break;
		case InternalMessageType_MultiplayerSession: _pkt_MultiplayerSession_write(&data->multiplayerSession, state); break;
		case InternalMessageType_KickPlayer: _pkt_KickPlayer_write(&data->kickPlayer, state); break;
		case InternalMessageType_PlayerStateUpdate: _pkt_PlayerStateUpdate_write(&data->playerStateUpdate, state); break;
		case InternalMessageType_PlayerAvatarUpdate: _pkt_PlayerAvatarUpdate_write(&data->playerAvatarUpdate, state); break;
		case InternalMessageType_PingMessage: _pkt_PingMessage_write(&data->pingMessage, state); break;
		case InternalMessageType_PongMessage: _pkt_PongMessage_write(&data->pongMessage, state); break;
		case InternalMessageType_GameSpecificMessage: _pkt_GameSpecificMessage_write(&data->gameSpecificMessage, state); break;
		default: uprintf("Invalid value for enum `InternalMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_RoutingHeader_read(struct RoutingHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "RoutingHeader");
	_pkt_u8_read(&data->remoteConnectionId, state);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->connectionId = bitfield0 >> 0 & 127;
	data->encrypted = bitfield0 >> 7 & 1;
	if(state.context.protocolVersion >= 9) {
		_pkt_u8_read(&data->packetOptions, state);
	}
}
void _pkt_RoutingHeader_write(const struct RoutingHeader *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "RoutingHeader");
	_pkt_u8_write(&data->remoteConnectionId, state);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->connectionId & 127u) << 0;
	bitfield0 |= (data->encrypted & 1u) << 7;
	_pkt_u8_write(&bitfield0, state);
	if(state.context.protocolVersion >= 9) {
		_pkt_u8_write(&data->packetOptions, state);
	}
}
void _pkt_BTRoutingHeader_read(struct BTRoutingHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BTRoutingHeader");
	_pkt_u8_read(&data->remoteConnectionId, state);
	_pkt_u8_read(&data->connectionId, state);
	if(state.context.protocolVersion >= 9) {
		_pkt_u8_read(&data->packetOptions, state);
	}
}
static void _pkt_BaseMasterServerReliableRequest_read(struct BaseMasterServerReliableRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BaseMasterServerReliableRequest");
	_pkt_u32_read(&data->requestId, state);
}
static void _pkt_BaseMasterServerReliableRequest_write(const struct BaseMasterServerReliableRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BaseMasterServerReliableRequest");
	_pkt_u32_write(&data->requestId, state);
}
static void _pkt_BaseMasterServerResponse_read(struct BaseMasterServerResponse *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BaseMasterServerResponse");
	_pkt_u32_read(&data->responseId, state);
}
static void _pkt_BaseMasterServerResponse_write(const struct BaseMasterServerResponse *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BaseMasterServerResponse");
	_pkt_u32_write(&data->responseId, state);
}
static void _pkt_BaseMasterServerReliableResponse_read(struct BaseMasterServerReliableResponse *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BaseMasterServerReliableResponse");
	_pkt_u32_read(&data->requestId, state);
	_pkt_u32_read(&data->responseId, state);
}
static void _pkt_BaseMasterServerReliableResponse_write(const struct BaseMasterServerReliableResponse *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BaseMasterServerReliableResponse");
	_pkt_u32_write(&data->requestId, state);
	_pkt_u32_write(&data->responseId, state);
}
static void _pkt_MessageReceivedAcknowledge_read(struct MessageReceivedAcknowledge *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MessageReceivedAcknowledge");
	_pkt_BaseMasterServerResponse_read(&data->base, state);
	_pkt_b_read(&data->messageHandled, state);
}
static void _pkt_MessageReceivedAcknowledge_write(const struct MessageReceivedAcknowledge *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MessageReceivedAcknowledge");
	_pkt_BaseMasterServerResponse_write(&data->base, state);
	_pkt_b_write(&data->messageHandled, state);
}
void _pkt_MessageReceivedAcknowledgeProxy_write(const struct MessageReceivedAcknowledgeProxy *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MessageReceivedAcknowledgeProxy");
	_pkt_u8_write(&data->type, state);
	_pkt_MessageReceivedAcknowledge_write(&data->value, state);
}
static void _pkt_AuthenticationToken_read(struct AuthenticationToken *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "AuthenticationToken");
	_pkt_u8_read(&data->platform, state);
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_ByteArrayNetSerializable_read(&data->sessionToken, state);
}
static void _pkt_AuthenticationToken_write(const struct AuthenticationToken *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "AuthenticationToken");
	_pkt_u8_write(&data->platform, state);
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_ByteArrayNetSerializable_write(&data->sessionToken, state);
}
static void _pkt_AuthenticateUserRequest_read(struct AuthenticateUserRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "AuthenticateUserRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_AuthenticationToken_read(&data->authenticationToken, state);
}
static void _pkt_AuthenticateUserRequest_write(const struct AuthenticateUserRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "AuthenticateUserRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_AuthenticationToken_write(&data->authenticationToken, state);
}
static void _pkt_AuthenticateUserResponse_read(struct AuthenticateUserResponse *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "AuthenticateUserResponse");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_u8_read(&data->result, state);
}
static void _pkt_AuthenticateUserResponse_write(const struct AuthenticateUserResponse *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "AuthenticateUserResponse");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_u8_write(&data->result, state);
}
static void _pkt_IPEndPoint_read(struct IPEndPoint *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "IPEndPoint");
	_pkt_String_read(&data->address, state);
	_pkt_u32_read(&data->port, state);
}
static void _pkt_IPEndPoint_write(const struct IPEndPoint *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "IPEndPoint");
	_pkt_String_write(&data->address, state);
	_pkt_u32_write(&data->port, state);
}
static void _pkt_GameplayServerConfiguration_read(struct GameplayServerConfiguration *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GameplayServerConfiguration");
	_pkt_vi32_read(&data->maxPlayerCount, state);
	_pkt_vi32_read(&data->discoveryPolicy, state);
	_pkt_vi32_read(&data->invitePolicy, state);
	_pkt_vi32_read(&data->gameplayServerMode, state);
	_pkt_vi32_read(&data->songSelectionMode, state);
	_pkt_vi32_read(&data->gameplayServerControlSettings, state);
}
static void _pkt_GameplayServerConfiguration_write(const struct GameplayServerConfiguration *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GameplayServerConfiguration");
	_pkt_vi32_write(&data->maxPlayerCount, state);
	_pkt_vi32_write(&data->discoveryPolicy, state);
	_pkt_vi32_write(&data->invitePolicy, state);
	_pkt_vi32_write(&data->gameplayServerMode, state);
	_pkt_vi32_write(&data->songSelectionMode, state);
	_pkt_vi32_write(&data->gameplayServerControlSettings, state);
}
static void _pkt_ConnectToServerResponse_read(struct ConnectToServerResponse *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ConnectToServerResponse");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_u8_read(&data->result, state);
	if(data->result == ConnectToServerResponse_Result_Success) {
		_pkt_String_read(&data->userId, state);
		_pkt_String_read(&data->userName, state);
		_pkt_String_read(&data->secret, state);
		_pkt_BeatmapLevelSelectionMask_read(&data->selectionMask, state);
		uint8_t bitfield0;
		_pkt_u8_read(&bitfield0, state);
		data->isConnectionOwner = bitfield0 >> 0 & 1;
		data->isDedicatedServer = bitfield0 >> 1 & 1;
		_pkt_IPEndPoint_read(&data->remoteEndPoint, state);
		_pkt_Cookie32_read(&data->random, state);
		_pkt_ByteArrayNetSerializable_read(&data->publicKey, state);
		_pkt_ServerCode_read(&data->code, state);
		_pkt_GameplayServerConfiguration_read(&data->configuration, state);
		_pkt_String_read(&data->managerId, state);
	}
}
static void _pkt_ConnectToServerResponse_write(const struct ConnectToServerResponse *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ConnectToServerResponse");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_u8_write(&data->result, state);
	if(data->result == ConnectToServerResponse_Result_Success) {
		_pkt_String_write(&data->userId, state);
		_pkt_String_write(&data->userName, state);
		_pkt_String_write(&data->secret, state);
		_pkt_BeatmapLevelSelectionMask_write(&data->selectionMask, state);
		uint8_t bitfield0 = 0;
		bitfield0 |= (data->isConnectionOwner & 1u) << 0;
		bitfield0 |= (data->isDedicatedServer & 1u) << 1;
		_pkt_u8_write(&bitfield0, state);
		_pkt_IPEndPoint_write(&data->remoteEndPoint, state);
		_pkt_Cookie32_write(&data->random, state);
		_pkt_ByteArrayNetSerializable_write(&data->publicKey, state);
		_pkt_ServerCode_write(&data->code, state);
		_pkt_GameplayServerConfiguration_write(&data->configuration, state);
		_pkt_String_write(&data->managerId, state);
	}
}
static void _pkt_BaseConnectToServerRequest_read(struct BaseConnectToServerRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "BaseConnectToServerRequest");
	_pkt_BaseMasterServerReliableRequest_read(&data->base, state);
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_Cookie32_read(&data->random, state);
	_pkt_ByteArrayNetSerializable_read(&data->publicKey, state);
}
static void _pkt_BaseConnectToServerRequest_write(const struct BaseConnectToServerRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "BaseConnectToServerRequest");
	_pkt_BaseMasterServerReliableRequest_write(&data->base, state);
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_Cookie32_write(&data->random, state);
	_pkt_ByteArrayNetSerializable_write(&data->publicKey, state);
}
static void _pkt_ConnectToServerRequest_read(struct ConnectToServerRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ConnectToServerRequest");
	_pkt_BaseConnectToServerRequest_read(&data->base, state);
	_pkt_BeatmapLevelSelectionMask_read(&data->selectionMask, state);
	_pkt_String_read(&data->secret, state);
	_pkt_ServerCode_read(&data->code, state);
	_pkt_GameplayServerConfiguration_read(&data->configuration, state);
}
static void _pkt_ConnectToServerRequest_write(const struct ConnectToServerRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ConnectToServerRequest");
	_pkt_BaseConnectToServerRequest_write(&data->base, state);
	_pkt_BeatmapLevelSelectionMask_write(&data->selectionMask, state);
	_pkt_String_write(&data->secret, state);
	_pkt_ServerCode_write(&data->code, state);
	_pkt_GameplayServerConfiguration_write(&data->configuration, state);
}
static void _pkt_MultipartMessage_read(struct MultipartMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MultipartMessage");
	_pkt_BaseMasterServerReliableRequest_read(&data->base, state);
	_pkt_u32_read(&data->multipartMessageId, state);
	_pkt_vu32_read(&data->offset, state);
	_pkt_vu32_read(&data->length, state);
	_pkt_vu32_read(&data->totalLength, state);
	_pkt_raw_read(data->data, check_overflow(state, *state.head, (uint32_t)(data->length), 384, "MultipartMessage.data"), state);
}
static void _pkt_MultipartMessage_write(const struct MultipartMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MultipartMessage");
	_pkt_BaseMasterServerReliableRequest_write(&data->base, state);
	_pkt_u32_write(&data->multipartMessageId, state);
	_pkt_vu32_write(&data->offset, state);
	_pkt_vu32_write(&data->length, state);
	_pkt_vu32_write(&data->totalLength, state);
	_pkt_raw_write(data->data, check_overflow(state, *state.head, (uint32_t)(data->length), 384, "MultipartMessage.data"), state);
}
void _pkt_MultipartMessageProxy_write(const struct MultipartMessageProxy *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MultipartMessageProxy");
	_pkt_u8_write(&data->type, state);
	_pkt_MultipartMessage_write(&data->value, state);
}
static void _pkt_SessionKeepaliveMessage_read(struct SessionKeepaliveMessage *restrict data, struct PacketRead parent) {
}
static void _pkt_SessionKeepaliveMessage_write(const struct SessionKeepaliveMessage *restrict data, struct PacketWrite parent) {
}
static void _pkt_GetPublicServersRequest_read(struct GetPublicServersRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetPublicServersRequest");
	_pkt_BaseMasterServerReliableRequest_read(&data->base, state);
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_vi32_read(&data->offset, state);
	_pkt_vi32_read(&data->count, state);
	_pkt_BeatmapLevelSelectionMask_read(&data->selectionMask, state);
	_pkt_GameplayServerConfiguration_read(&data->configuration, state);
}
static void _pkt_GetPublicServersRequest_write(const struct GetPublicServersRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetPublicServersRequest");
	_pkt_BaseMasterServerReliableRequest_write(&data->base, state);
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_vi32_write(&data->offset, state);
	_pkt_vi32_write(&data->count, state);
	_pkt_BeatmapLevelSelectionMask_write(&data->selectionMask, state);
	_pkt_GameplayServerConfiguration_write(&data->configuration, state);
}
static void _pkt_PublicServerInfo_read(struct PublicServerInfo *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PublicServerInfo");
	_pkt_ServerCode_read(&data->code, state);
	_pkt_vi32_read(&data->currentPlayerCount, state);
}
static void _pkt_PublicServerInfo_write(const struct PublicServerInfo *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PublicServerInfo");
	_pkt_ServerCode_write(&data->code, state);
	_pkt_vi32_write(&data->currentPlayerCount, state);
}
static void _pkt_GetPublicServersResponse_read(struct GetPublicServersResponse *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GetPublicServersResponse");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_u8_read(&data->result, state);
	if(data->result == GetPublicServersResponse_Result_Success) {
		_pkt_vu32_read(&data->publicServerCount, state);
		for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->publicServerCount), 20, "GetPublicServersResponse.publicServers"); i < count; ++i)
			_pkt_PublicServerInfo_read(&data->publicServers[i], state);
	}
}
static void _pkt_GetPublicServersResponse_write(const struct GetPublicServersResponse *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GetPublicServersResponse");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_u8_write(&data->result, state);
	if(data->result == GetPublicServersResponse_Result_Success) {
		_pkt_vu32_write(&data->publicServerCount, state);
		for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->publicServerCount), 20, "GetPublicServersResponse.publicServers"); i < count; ++i)
			_pkt_PublicServerInfo_write(&data->publicServers[i], state);
	}
}
void _pkt_UserMessage_read(struct UserMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "UserMessage");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case UserMessageType_AuthenticateUserRequest: _pkt_AuthenticateUserRequest_read(&data->authenticateUserRequest, state); break;
		case UserMessageType_AuthenticateUserResponse: _pkt_AuthenticateUserResponse_read(&data->authenticateUserResponse, state); break;
		case UserMessageType_ConnectToServerResponse: _pkt_ConnectToServerResponse_read(&data->connectToServerResponse, state); break;
		case UserMessageType_ConnectToServerRequest: _pkt_ConnectToServerRequest_read(&data->connectToServerRequest, state); break;
		case UserMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_read(&data->messageReceivedAcknowledge, state); break;
		case UserMessageType_MultipartMessage: _pkt_MultipartMessage_read(&data->multipartMessage, state); break;
		case UserMessageType_SessionKeepaliveMessage: _pkt_SessionKeepaliveMessage_read(&data->sessionKeepaliveMessage, state); break;
		case UserMessageType_GetPublicServersRequest: _pkt_GetPublicServersRequest_read(&data->getPublicServersRequest, state); break;
		case UserMessageType_GetPublicServersResponse: _pkt_GetPublicServersResponse_read(&data->getPublicServersResponse, state); break;
		default: uprintf("Invalid value for enum `UserMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_UserMessage_write(const struct UserMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "UserMessage");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case UserMessageType_AuthenticateUserRequest: _pkt_AuthenticateUserRequest_write(&data->authenticateUserRequest, state); break;
		case UserMessageType_AuthenticateUserResponse: _pkt_AuthenticateUserResponse_write(&data->authenticateUserResponse, state); break;
		case UserMessageType_ConnectToServerResponse: _pkt_ConnectToServerResponse_write(&data->connectToServerResponse, state); break;
		case UserMessageType_ConnectToServerRequest: _pkt_ConnectToServerRequest_write(&data->connectToServerRequest, state); break;
		case UserMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_write(&data->messageReceivedAcknowledge, state); break;
		case UserMessageType_MultipartMessage: _pkt_MultipartMessage_write(&data->multipartMessage, state); break;
		case UserMessageType_SessionKeepaliveMessage: _pkt_SessionKeepaliveMessage_write(&data->sessionKeepaliveMessage, state); break;
		case UserMessageType_GetPublicServersRequest: _pkt_GetPublicServersRequest_write(&data->getPublicServersRequest, state); break;
		case UserMessageType_GetPublicServersResponse: _pkt_GetPublicServersResponse_write(&data->getPublicServersResponse, state); break;
		default: uprintf("Invalid value for enum `UserMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_AuthenticateGameLiftUserRequest_read(struct AuthenticateGameLiftUserRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "AuthenticateGameLiftUserRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_String_read(&data->playerSessionId, state);
}
static void _pkt_AuthenticateGameLiftUserRequest_write(const struct AuthenticateGameLiftUserRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "AuthenticateGameLiftUserRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_String_write(&data->playerSessionId, state);
}
void _pkt_GameLiftMessage_read(struct GameLiftMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "GameLiftMessage");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case GameLiftMessageType_AuthenticateGameLiftUserRequest: _pkt_AuthenticateGameLiftUserRequest_read(&data->authenticateGameLiftUserRequest, state); break;
		case GameLiftMessageType_AuthenticateUserResponse: _pkt_AuthenticateUserResponse_read(&data->authenticateUserResponse, state); break;
		case GameLiftMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_read(&data->messageReceivedAcknowledge, state); break;
		case GameLiftMessageType_MultipartMessage: _pkt_MultipartMessage_read(&data->multipartMessage, state); break;
		default: uprintf("Invalid value for enum `GameLiftMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_GameLiftMessage_write(const struct GameLiftMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "GameLiftMessage");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case GameLiftMessageType_AuthenticateGameLiftUserRequest: _pkt_AuthenticateGameLiftUserRequest_write(&data->authenticateGameLiftUserRequest, state); break;
		case GameLiftMessageType_AuthenticateUserResponse: _pkt_AuthenticateUserResponse_write(&data->authenticateUserResponse, state); break;
		case GameLiftMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_write(&data->messageReceivedAcknowledge, state); break;
		case GameLiftMessageType_MultipartMessage: _pkt_MultipartMessage_write(&data->multipartMessage, state); break;
		default: uprintf("Invalid value for enum `GameLiftMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
static void _pkt_ClientHelloRequest_read(struct ClientHelloRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClientHelloRequest");
	_pkt_BaseMasterServerReliableRequest_read(&data->base, state);
	_pkt_Cookie32_read(&data->random, state);
}
static void _pkt_ClientHelloRequest_write(const struct ClientHelloRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClientHelloRequest");
	_pkt_BaseMasterServerReliableRequest_write(&data->base, state);
	_pkt_Cookie32_write(&data->random, state);
}
static void _pkt_HelloVerifyRequest_read(struct HelloVerifyRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "HelloVerifyRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_Cookie32_read(&data->cookie, state);
}
static void _pkt_HelloVerifyRequest_write(const struct HelloVerifyRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "HelloVerifyRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_Cookie32_write(&data->cookie, state);
}
static void _pkt_ClientHelloWithCookieRequest_read(struct ClientHelloWithCookieRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClientHelloWithCookieRequest");
	_pkt_BaseMasterServerReliableRequest_read(&data->base, state);
	_pkt_u32_read(&data->certificateResponseId, state);
	_pkt_Cookie32_read(&data->random, state);
	_pkt_Cookie32_read(&data->cookie, state);
}
static void _pkt_ClientHelloWithCookieRequest_write(const struct ClientHelloWithCookieRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClientHelloWithCookieRequest");
	_pkt_BaseMasterServerReliableRequest_write(&data->base, state);
	_pkt_u32_write(&data->certificateResponseId, state);
	_pkt_Cookie32_write(&data->random, state);
	_pkt_Cookie32_write(&data->cookie, state);
}
static void _pkt_ServerHelloRequest_read(struct ServerHelloRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ServerHelloRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_Cookie32_read(&data->random, state);
	_pkt_ByteArrayNetSerializable_read(&data->publicKey, state);
	_pkt_ByteArrayNetSerializable_read(&data->signature, state);
}
static void _pkt_ServerHelloRequest_write(const struct ServerHelloRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ServerHelloRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_Cookie32_write(&data->random, state);
	_pkt_ByteArrayNetSerializable_write(&data->publicKey, state);
	_pkt_ByteArrayNetSerializable_write(&data->signature, state);
}
static void _pkt_ServerCertificateRequest_read(struct ServerCertificateRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ServerCertificateRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_vu32_read(&data->certificateCount, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->certificateCount), 10, "ServerCertificateRequest.certificateList"); i < count; ++i)
		_pkt_ByteArrayNetSerializable_read(&data->certificateList[i], state);
}
static void _pkt_ServerCertificateRequest_write(const struct ServerCertificateRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ServerCertificateRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_vu32_write(&data->certificateCount, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->certificateCount), 10, "ServerCertificateRequest.certificateList"); i < count; ++i)
		_pkt_ByteArrayNetSerializable_write(&data->certificateList[i], state);
}
static void _pkt_ClientKeyExchangeRequest_read(struct ClientKeyExchangeRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ClientKeyExchangeRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
	_pkt_ByteArrayNetSerializable_read(&data->clientPublicKey, state);
}
static void _pkt_ClientKeyExchangeRequest_write(const struct ClientKeyExchangeRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ClientKeyExchangeRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
	_pkt_ByteArrayNetSerializable_write(&data->clientPublicKey, state);
}
static void _pkt_ChangeCipherSpecRequest_read(struct ChangeCipherSpecRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ChangeCipherSpecRequest");
	_pkt_BaseMasterServerReliableResponse_read(&data->base, state);
}
static void _pkt_ChangeCipherSpecRequest_write(const struct ChangeCipherSpecRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ChangeCipherSpecRequest");
	_pkt_BaseMasterServerReliableResponse_write(&data->base, state);
}
void _pkt_HandshakeMessage_read(struct HandshakeMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "HandshakeMessage");
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case HandshakeMessageType_ClientHelloRequest: _pkt_ClientHelloRequest_read(&data->clientHelloRequest, state); break;
		case HandshakeMessageType_HelloVerifyRequest: _pkt_HelloVerifyRequest_read(&data->helloVerifyRequest, state); break;
		case HandshakeMessageType_ClientHelloWithCookieRequest: _pkt_ClientHelloWithCookieRequest_read(&data->clientHelloWithCookieRequest, state); break;
		case HandshakeMessageType_ServerHelloRequest: _pkt_ServerHelloRequest_read(&data->serverHelloRequest, state); break;
		case HandshakeMessageType_ServerCertificateRequest: _pkt_ServerCertificateRequest_read(&data->serverCertificateRequest, state); break;
		case HandshakeMessageType_ClientKeyExchangeRequest: _pkt_ClientKeyExchangeRequest_read(&data->clientKeyExchangeRequest, state); break;
		case HandshakeMessageType_ChangeCipherSpecRequest: _pkt_ChangeCipherSpecRequest_read(&data->changeCipherSpecRequest, state); break;
		case HandshakeMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_read(&data->messageReceivedAcknowledge, state); break;
		case HandshakeMessageType_MultipartMessage: _pkt_MultipartMessage_read(&data->multipartMessage, state); break;
		default: uprintf("Invalid value for enum `HandshakeMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_HandshakeMessage_write(const struct HandshakeMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "HandshakeMessage");
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case HandshakeMessageType_ClientHelloRequest: _pkt_ClientHelloRequest_write(&data->clientHelloRequest, state); break;
		case HandshakeMessageType_HelloVerifyRequest: _pkt_HelloVerifyRequest_write(&data->helloVerifyRequest, state); break;
		case HandshakeMessageType_ClientHelloWithCookieRequest: _pkt_ClientHelloWithCookieRequest_write(&data->clientHelloWithCookieRequest, state); break;
		case HandshakeMessageType_ServerHelloRequest: _pkt_ServerHelloRequest_write(&data->serverHelloRequest, state); break;
		case HandshakeMessageType_ServerCertificateRequest: _pkt_ServerCertificateRequest_write(&data->serverCertificateRequest, state); break;
		case HandshakeMessageType_ClientKeyExchangeRequest: _pkt_ClientKeyExchangeRequest_write(&data->clientKeyExchangeRequest, state); break;
		case HandshakeMessageType_ChangeCipherSpecRequest: _pkt_ChangeCipherSpecRequest_write(&data->changeCipherSpecRequest, state); break;
		case HandshakeMessageType_MessageReceivedAcknowledge: _pkt_MessageReceivedAcknowledge_write(&data->messageReceivedAcknowledge, state); break;
		case HandshakeMessageType_MultipartMessage: _pkt_MultipartMessage_write(&data->multipartMessage, state); break;
		default: uprintf("Invalid value for enum `HandshakeMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_SerializeHeader_read(struct SerializeHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "SerializeHeader");
	_pkt_vu32_read(&data->length, state);
}
void _pkt_SerializeHeader_write(const struct SerializeHeader *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "SerializeHeader");
	_pkt_vu32_write(&data->length, state);
}
void _pkt_FragmentedHeader_read(struct FragmentedHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "FragmentedHeader");
	_pkt_u16_read(&data->fragmentId, state);
	_pkt_u16_read(&data->fragmentPart, state);
	_pkt_u16_read(&data->fragmentsTotal, state);
}
void _pkt_FragmentedHeader_write(const struct FragmentedHeader *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "FragmentedHeader");
	_pkt_u16_write(&data->fragmentId, state);
	_pkt_u16_write(&data->fragmentPart, state);
	_pkt_u16_write(&data->fragmentsTotal, state);
}
static void _pkt_Unreliable_read(struct Unreliable *restrict data, struct PacketRead parent) {
}
static void _pkt_Unreliable_write(const struct Unreliable *restrict data, struct PacketWrite parent) {
}
static void _pkt_Channeled_read(struct Channeled *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Channeled");
	_pkt_u16_read(&data->sequence, state);
	_pkt_u8_read(&data->channelId, state);
}
static void _pkt_Channeled_write(const struct Channeled *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Channeled");
	_pkt_u16_write(&data->sequence, state);
	_pkt_u8_write(&data->channelId, state);
}
static void _pkt_Ack_read(struct Ack *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Ack");
	_pkt_u16_read(&data->sequence, state);
	_pkt_u8_read(&data->channelId, state);
	if(data->channelId % 2 == 0) {
		_pkt_raw_read(data->data, check_overflow(state, *state.head, (uint32_t)((state.context.windowSize + 7) / 8), 32, "Ack.data"), state);
		_pkt_u8_read(&data->_pad0, state);
	}
}
static void _pkt_Ack_write(const struct Ack *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Ack");
	_pkt_u16_write(&data->sequence, state);
	_pkt_u8_write(&data->channelId, state);
	if(data->channelId % 2 == 0) {
		_pkt_raw_write(data->data, check_overflow(state, *state.head, (uint32_t)((state.context.windowSize + 7) / 8), 32, "Ack.data"), state);
		_pkt_u8_write(&data->_pad0, state);
	}
}
static void _pkt_Ping_read(struct Ping *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Ping");
	_pkt_u16_read(&data->sequence, state);
}
static void _pkt_Ping_write(const struct Ping *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Ping");
	_pkt_u16_write(&data->sequence, state);
}
static void _pkt_Pong_read(struct Pong *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Pong");
	_pkt_u16_read(&data->sequence, state);
	_pkt_u64_read(&data->time, state);
}
static void _pkt_Pong_write(const struct Pong *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Pong");
	_pkt_u16_write(&data->sequence, state);
	_pkt_u64_write(&data->time, state);
}
void _pkt_ConnectMessage_read(struct ConnectMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ConnectMessage");
	_pkt_String_read(&data->userId, state);
	_pkt_String_read(&data->userName, state);
	_pkt_b_read(&data->isConnectionOwner, state);
	if(state.context.direct) {
		_pkt_String_read(&data->playerSessionId, state);
	}
	if(state.context.gameVersion >= GameVersion_1_42_0) {
		_pkt_String_read(&data->compatibilityVersion, state);
	}
}
static void _pkt_ConnectMessage_write(const struct ConnectMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ConnectMessage");
	_pkt_String_write(&data->userId, state);
	_pkt_String_write(&data->userName, state);
	_pkt_b_write(&data->isConnectionOwner, state);
	if(state.context.direct) {
		_pkt_String_write(&data->playerSessionId, state);
	}
	if(state.context.gameVersion >= GameVersion_1_42_0) {
		_pkt_String_write(&data->compatibilityVersion, state);
	}
}
static void _pkt_ConnectRequest_read(struct ConnectRequest *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ConnectRequest");
	_pkt_u32_read(&data->protocolId, state);
	_pkt_u64_read(&data->connectTime, state);
	if((state.context.netVersion = (uint8_t)data->protocolId) >= 12) {
		_pkt_i32_read(&data->peerId, state);
	}
	_pkt_u8_read(&data->addrlen, state);
	_pkt_raw_read(data->address, check_overflow(state, *state.head, (uint32_t)(data->addrlen), 38, "ConnectRequest.address"), state);
	if(!state.context.direct) {
		_pkt_String_read(&data->secret, state);
	}
	_pkt_ConnectMessage_read(&data->message, state);
}
static void _pkt_ConnectRequest_write(const struct ConnectRequest *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ConnectRequest");
	_pkt_u32_write(&data->protocolId, state);
	_pkt_u64_write(&data->connectTime, state);
	if((state.context.netVersion = (uint8_t)data->protocolId) >= 12) {
		_pkt_i32_write(&data->peerId, state);
	}
	_pkt_u8_write(&data->addrlen, state);
	_pkt_raw_write(data->address, check_overflow(state, *state.head, (uint32_t)(data->addrlen), 38, "ConnectRequest.address"), state);
	if(!state.context.direct) {
		_pkt_String_write(&data->secret, state);
	}
	_pkt_ConnectMessage_write(&data->message, state);
}
static void _pkt_ConnectAccept_read(struct ConnectAccept *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "ConnectAccept");
	_pkt_u64_read(&data->connectTime, state);
	_pkt_u8_read(&data->connectNum, state);
	_pkt_b_read(&data->reusedPeer, state);
	if(state.context.netVersion >= 12) {
		_pkt_i32_read(&data->peerId, state);
	}
	if(state.context.beatUpVersion) {
		_pkt_ServerConnectInfo_read(&data->beatUp, state);
	}
}
static void _pkt_ConnectAccept_write(const struct ConnectAccept *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "ConnectAccept");
	_pkt_u64_write(&data->connectTime, state);
	_pkt_u8_write(&data->connectNum, state);
	_pkt_b_write(&data->reusedPeer, state);
	if(state.context.netVersion >= 12) {
		_pkt_i32_write(&data->peerId, state);
	}
	if(state.context.beatUpVersion) {
		_pkt_ServerConnectInfo_write(&data->beatUp, state);
	}
}
static void _pkt_Disconnect_read(struct Disconnect *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Disconnect");
	_pkt_raw_read(data->_pad0, 8, state);
}
static void _pkt_Disconnect_write(const struct Disconnect *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Disconnect");
	_pkt_raw_write(data->_pad0, 8, state);
}
void _pkt_UnconnectedMessage_read(struct UnconnectedMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "UnconnectedMessage");
	_pkt_u32_read(&data->type, state);
	_pkt_vu32_read(&data->protocolVersion, state);
}
void _pkt_UnconnectedMessage_write(const struct UnconnectedMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "UnconnectedMessage");
	_pkt_u32_write(&data->type, state);
	_pkt_vu32_write(&data->protocolVersion, state);
}
static void _pkt_Mtu_read(struct Mtu *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "Mtu");
	_pkt_u32_read(&data->newMtu0, state);
	_pkt_raw_read(data->pad, check_overflow(state, *state.head, (uint32_t)(data->newMtu0 - 9), 1423, "Mtu.pad"), state);
	_pkt_u32_read(&data->newMtu1, state);
}
static void _pkt_Mtu_write(const struct Mtu *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "Mtu");
	_pkt_u32_write(&data->newMtu0, state);
	_pkt_raw_write(data->pad, check_overflow(state, *state.head, (uint32_t)(data->newMtu0 - 9), 1423, "Mtu.pad"), state);
	_pkt_u32_write(&data->newMtu1, state);
}
static void _pkt_MtuCheck_read(struct MtuCheck *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MtuCheck");
	_pkt_Mtu_read(&data->base, state);
}
static void _pkt_MtuCheck_write(const struct MtuCheck *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MtuCheck");
	_pkt_Mtu_write(&data->base, state);
}
static void _pkt_MtuOk_read(struct MtuOk *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MtuOk");
	_pkt_Mtu_read(&data->base, state);
}
static void _pkt_MtuOk_write(const struct MtuOk *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MtuOk");
	_pkt_Mtu_write(&data->base, state);
}
void _pkt_MergedHeader_read(struct MergedHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MergedHeader");
	_pkt_u16_read(&data->length, state);
}
void _pkt_MergedHeader_write(const struct MergedHeader *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "MergedHeader");
	_pkt_u16_write(&data->length, state);
}
static void _pkt_Broadcast_read(struct Broadcast *restrict data, struct PacketRead parent) {
}
static void _pkt_Broadcast_write(const struct Broadcast *restrict data, struct PacketWrite parent) {
}
static void _pkt_Merged_read(struct Merged *restrict data, struct PacketRead parent) {
}
static void _pkt_Merged_write(const struct Merged *restrict data, struct PacketWrite parent) {
}
static void _pkt_ShutdownOk_read(struct ShutdownOk *restrict data, struct PacketRead parent) {
}
static void _pkt_ShutdownOk_write(const struct ShutdownOk *restrict data, struct PacketWrite parent) {
}
static void _pkt_PeerNotFound_read(struct PeerNotFound *restrict data, struct PacketRead parent) {
}
static void _pkt_PeerNotFound_write(const struct PeerNotFound *restrict data, struct PacketWrite parent) {
}
static void _pkt_InvalidProtocol_read(struct InvalidProtocol *restrict data, struct PacketRead parent) {
}
static void _pkt_InvalidProtocol_write(const struct InvalidProtocol *restrict data, struct PacketWrite parent) {
}
static void _pkt_NatMessage_read(struct NatMessage *restrict data, struct PacketRead parent) {
}
static void _pkt_NatMessage_write(const struct NatMessage *restrict data, struct PacketWrite parent) {
}
static void _pkt_Empty_read(struct Empty *restrict data, struct PacketRead parent) {
}
static void _pkt_Empty_write(const struct Empty *restrict data, struct PacketWrite parent) {
}
void _pkt_NetPacketHeader_read(struct NetPacketHeader *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "NetPacketHeader");
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->property = bitfield0 >> 0 & 31;
	data->connectionNumber = bitfield0 >> 5 & 3;
	data->isFragmented = bitfield0 >> 7 & 1;
	switch(data->property) {
		case PacketProperty_Unreliable: _pkt_Unreliable_read(&data->unreliable, state); break;
		case PacketProperty_Channeled: _pkt_Channeled_read(&data->channeled, state); break;
		case PacketProperty_Ack: _pkt_Ack_read(&data->ack, state); break;
		case PacketProperty_Ping: _pkt_Ping_read(&data->ping, state); break;
		case PacketProperty_Pong: _pkt_Pong_read(&data->pong, state); break;
		case PacketProperty_ConnectRequest: _pkt_ConnectRequest_read(&data->connectRequest, state); break;
		case PacketProperty_ConnectAccept: _pkt_ConnectAccept_read(&data->connectAccept, state); break;
		case PacketProperty_Disconnect: _pkt_Disconnect_read(&data->disconnect, state); break;
		case PacketProperty_UnconnectedMessage: _pkt_UnconnectedMessage_read(&data->unconnectedMessage, state); break;
		case PacketProperty_MtuCheck: _pkt_MtuCheck_read(&data->mtuCheck, state); break;
		case PacketProperty_MtuOk: _pkt_MtuOk_read(&data->mtuOk, state); break;
		case PacketProperty_Broadcast: _pkt_Broadcast_read(&data->broadcast, state); break;
		case PacketProperty_Merged: _pkt_Merged_read(&data->merged, state); break;
		case PacketProperty_ShutdownOk: _pkt_ShutdownOk_read(&data->shutdownOk, state); break;
		case PacketProperty_PeerNotFound: _pkt_PeerNotFound_read(&data->peerNotFound, state); break;
		case PacketProperty_InvalidProtocol: _pkt_InvalidProtocol_read(&data->invalidProtocol, state); break;
		case PacketProperty_NatMessage: _pkt_NatMessage_read(&data->natMessage, state); break;
		case PacketProperty_Empty: _pkt_Empty_read(&data->empty, state); break;
		default: uprintf("Invalid value for enum `PacketProperty`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_NetPacketHeader_write(const struct NetPacketHeader *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "NetPacketHeader");
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->property & 31u) << 0;
	bitfield0 |= (data->connectionNumber & 3u) << 5;
	bitfield0 |= (data->isFragmented & 1u) << 7;
	_pkt_u8_write(&bitfield0, state);
	switch(data->property) {
		case PacketProperty_Unreliable: _pkt_Unreliable_write(&data->unreliable, state); break;
		case PacketProperty_Channeled: _pkt_Channeled_write(&data->channeled, state); break;
		case PacketProperty_Ack: _pkt_Ack_write(&data->ack, state); break;
		case PacketProperty_Ping: _pkt_Ping_write(&data->ping, state); break;
		case PacketProperty_Pong: _pkt_Pong_write(&data->pong, state); break;
		case PacketProperty_ConnectRequest: _pkt_ConnectRequest_write(&data->connectRequest, state); break;
		case PacketProperty_ConnectAccept: _pkt_ConnectAccept_write(&data->connectAccept, state); break;
		case PacketProperty_Disconnect: _pkt_Disconnect_write(&data->disconnect, state); break;
		case PacketProperty_UnconnectedMessage: _pkt_UnconnectedMessage_write(&data->unconnectedMessage, state); break;
		case PacketProperty_MtuCheck: _pkt_MtuCheck_write(&data->mtuCheck, state); break;
		case PacketProperty_MtuOk: _pkt_MtuOk_write(&data->mtuOk, state); break;
		case PacketProperty_Broadcast: _pkt_Broadcast_write(&data->broadcast, state); break;
		case PacketProperty_Merged: _pkt_Merged_write(&data->merged, state); break;
		case PacketProperty_ShutdownOk: _pkt_ShutdownOk_write(&data->shutdownOk, state); break;
		case PacketProperty_PeerNotFound: _pkt_PeerNotFound_write(&data->peerNotFound, state); break;
		case PacketProperty_InvalidProtocol: _pkt_InvalidProtocol_write(&data->invalidProtocol, state); break;
		case PacketProperty_NatMessage: _pkt_NatMessage_write(&data->natMessage, state); break;
		case PacketProperty_Empty: _pkt_Empty_write(&data->empty, state); break;
		default: uprintf("Invalid value for enum `PacketProperty`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_MultipartMessageReadbackProxy_read(struct MultipartMessageReadbackProxy *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "MultipartMessageReadbackProxy");
	_pkt_NetPacketHeader_read(&data->header, state);
	_pkt_SerializeHeader_read(&data->serial, state);
	_pkt_u8_read(&data->type, state);
	_pkt_BaseMasterServerReliableRequest_read(&data->base, state);
	_pkt_u32_read(&data->multipartMessageId, state);
}
void _pkt_PacketEncryptionLayer_read(struct PacketEncryptionLayer *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "PacketEncryptionLayer");
	_pkt_u8_read(&data->encrypted, state);
	if(data->encrypted == 1) {
		_pkt_u32_read(&data->sequenceId, state);
		_pkt_raw_read(data->iv, 16, state);
	}
}
void _pkt_PacketEncryptionLayer_write(const struct PacketEncryptionLayer *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "PacketEncryptionLayer");
	_pkt_u8_write(&data->encrypted, state);
	if(data->encrypted == 1) {
		_pkt_u32_write(&data->sequenceId, state);
		_pkt_raw_write(data->iv, 16, state);
	}
}
static void _pkt_WireServerConfiguration_read(struct WireServerConfiguration *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireServerConfiguration");
	_pkt_GameplayServerConfiguration_read(&data->base, state);
	_pkt_vu32_read(&data->shortCountdownMs, state);
	_pkt_vu32_read(&data->longCountdownMs, state);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->skipResults = bitfield0 >> 0 & 1;
	data->perPlayerDifficulty = bitfield0 >> 1 & 1;
	data->perPlayerModifiers = bitfield0 >> 2 & 1;
}
static void _pkt_WireServerConfiguration_write(const struct WireServerConfiguration *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireServerConfiguration");
	_pkt_GameplayServerConfiguration_write(&data->base, state);
	_pkt_vu32_write(&data->shortCountdownMs, state);
	_pkt_vu32_write(&data->longCountdownMs, state);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->skipResults & 1u) << 0;
	bitfield0 |= (data->perPlayerDifficulty & 1u) << 1;
	bitfield0 |= (data->perPlayerModifiers & 1u) << 2;
	_pkt_u8_write(&bitfield0, state);
}
static void _pkt_WireInstanceAttach_read(struct WireInstanceAttach *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireInstanceAttach");
	_pkt_u32_read(&data->capacity, state);
	_pkt_b_read(&data->discover, state);
}
static void _pkt_WireInstanceAttach_write(const struct WireInstanceAttach *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireInstanceAttach");
	_pkt_u32_write(&data->capacity, state);
	_pkt_b_write(&data->discover, state);
}
static void _pkt_WireSessionAlloc_read(struct WireSessionAlloc *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireSessionAlloc");
	_pkt_u32_read(&data->room, state);
	_pkt_String_read(&data->secret, state);
	_pkt_String_read(&data->userId, state);
	_pkt_b_read(&data->ipv4, state);
	_pkt_PacketContext_read(&data->clientVersion, state);
	if(!data->clientVersion.direct) {
		_pkt_Cookie32_read(&data->random, state);
		_pkt_ByteArrayNetSerializable_read(&data->publicKey, state);
	}
}
static void _pkt_WireSessionAlloc_write(const struct WireSessionAlloc *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireSessionAlloc");
	_pkt_u32_write(&data->room, state);
	_pkt_String_write(&data->secret, state);
	_pkt_String_write(&data->userId, state);
	_pkt_b_write(&data->ipv4, state);
	_pkt_PacketContext_write(&data->clientVersion, state);
	if(!data->clientVersion.direct) {
		_pkt_Cookie32_write(&data->random, state);
		_pkt_ByteArrayNetSerializable_write(&data->publicKey, state);
	}
}
static void _pkt_WireSessionAllocResp_read(struct WireSessionAllocResp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireSessionAllocResp");
	_pkt_u8_read(&data->result, state);
	if(data->result == ConnectToServerResponse_Result_Success) {
		_pkt_GameplayServerConfiguration_read(&data->configuration, state);
		_pkt_String_read(&data->managerId, state);
		_pkt_IPEndPoint_read(&data->endPoint, state);
		_pkt_u32_read(&data->playerSlot, state);
		_pkt_Cookie32_read(&data->random, state);
		_pkt_ByteArrayNetSerializable_read(&data->publicKey, state);
	}
}
static void _pkt_WireSessionAllocResp_write(const struct WireSessionAllocResp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireSessionAllocResp");
	_pkt_u8_write(&data->result, state);
	if(data->result == ConnectToServerResponse_Result_Success) {
		_pkt_GameplayServerConfiguration_write(&data->configuration, state);
		_pkt_String_write(&data->managerId, state);
		_pkt_IPEndPoint_write(&data->endPoint, state);
		_pkt_u32_write(&data->playerSlot, state);
		_pkt_Cookie32_write(&data->random, state);
		_pkt_ByteArrayNetSerializable_write(&data->publicKey, state);
	}
}
static void _pkt_WireRoomSpawn_read(struct WireRoomSpawn *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomSpawn");
	_pkt_WireSessionAlloc_read(&data->base, state);
	_pkt_WireServerConfiguration_read(&data->configuration, state);
}
static void _pkt_WireRoomSpawn_write(const struct WireRoomSpawn *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomSpawn");
	_pkt_WireSessionAlloc_write(&data->base, state);
	_pkt_WireServerConfiguration_write(&data->configuration, state);
}
static void _pkt_WireRoomJoin_read(struct WireRoomJoin *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomJoin");
	_pkt_WireSessionAlloc_read(&data->base, state);
	_pkt_b_read(&data->managed, state);
}
static void _pkt_WireRoomJoin_write(const struct WireRoomJoin *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomJoin");
	_pkt_WireSessionAlloc_write(&data->base, state);
	_pkt_b_write(&data->managed, state);
}
static void _pkt_WireRoomManagedJoinResp_read(struct WireRoomManagedJoinResp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomManagedJoinResp");
	_pkt_WireSessionAllocResp_read(&data->base, state);
	_pkt_u32_read(&data->room, state);
}
static void _pkt_WireRoomManagedJoinResp_write(const struct WireRoomManagedJoinResp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomManagedJoinResp");
	_pkt_WireSessionAllocResp_write(&data->base, state);
	_pkt_u32_write(&data->room, state);
}
static void _pkt_WireRoomSpawnResp_read(struct WireRoomSpawnResp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomSpawnResp");
	_pkt_WireSessionAllocResp_read(&data->base, state);
}
static void _pkt_WireRoomSpawnResp_write(const struct WireRoomSpawnResp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomSpawnResp");
	_pkt_WireSessionAllocResp_write(&data->base, state);
}
static void _pkt_WireRoomJoinResp_read(struct WireRoomJoinResp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomJoinResp");
	_pkt_WireSessionAllocResp_read(&data->base, state);
}
static void _pkt_WireRoomJoinResp_write(const struct WireRoomJoinResp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomJoinResp");
	_pkt_WireSessionAllocResp_write(&data->base, state);
}
static void _pkt_WireRoomQuery_read(struct WireRoomQuery *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomQuery");
	_pkt_u32_read(&data->room, state);
}
static void _pkt_WireRoomQuery_write(const struct WireRoomQuery *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomQuery");
	_pkt_u32_write(&data->room, state);
}
static void _pkt_WireRoomQueryResp_PlayerInfo_read(struct WireRoomQueryResp_PlayerInfo *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomQueryResp_PlayerInfo");
	_pkt_PacketContext_read(&data->version, state);
	_pkt_b_read(&data->modded, state);
	_pkt_String_read(&data->userName, state);
	for(uint32_t i = 0, count = 7; i < count; ++i)
		_pkt_Color32_read(&data->colorScheme[i], state);
}
static void _pkt_WireRoomQueryResp_PlayerInfo_write(const struct WireRoomQueryResp_PlayerInfo *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomQueryResp_PlayerInfo");
	_pkt_PacketContext_write(&data->version, state);
	_pkt_b_write(&data->modded, state);
	_pkt_String_write(&data->userName, state);
	for(uint32_t i = 0, count = 7; i < count; ++i)
		_pkt_Color32_write(&data->colorScheme[i], state);
}
static void _pkt_WireRoomQueryResp_read(struct WireRoomQueryResp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomQueryResp");
	_pkt_String_read(&data->levelID, state);
	_pkt_u8_read(&data->hostIndex, state);
	_pkt_u8_read(&data->players_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->players_len), 254, "WireRoomQueryResp.players"); i < count; ++i)
		_pkt_WireRoomQueryResp_PlayerInfo_read(&data->players[i], state);
}
static void _pkt_WireRoomQueryResp_write(const struct WireRoomQueryResp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomQueryResp");
	_pkt_String_write(&data->levelID, state);
	_pkt_u8_write(&data->hostIndex, state);
	_pkt_u8_write(&data->players_len, state);
	for(uint32_t i = 0, count = check_overflow(state, *state.head, (uint32_t)(data->players_len), 254, "WireRoomQueryResp.players"); i < count; ++i)
		_pkt_WireRoomQueryResp_PlayerInfo_write(&data->players[i], state);
}
void _pkt_WireRoomStatusNotify_read(struct WireRoomStatusNotify *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireRoomStatusNotify");
	_pkt_u32_read(&data->code, state);
	_pkt_u8_read(&data->protocolVersion, state);
	_pkt_u8_read(&data->playerCount, state);
	_pkt_u8_read(&data->playerCapacity, state);
	_pkt_u16_read(&data->playerNPS, state);
	_pkt_u16_read(&data->levelNPS, state);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, state);
	data->public = bitfield0 >> 0 & 1;
	data->managed = bitfield0 >> 1 & 1;
	data->skipResults = bitfield0 >> 2 & 1;
	data->perPlayerDifficulty = bitfield0 >> 3 & 1;
	data->perPlayerModifiers = bitfield0 >> 4 & 1;
	_pkt_vi32_read(&data->selectionMode, state);
	_pkt_String_read(&data->levelName, state);
	_pkt_String_read(&data->levelID, state);
	_pkt_ByteArrayNetSerializable_read(&data->levelCover, state);
}
void _pkt_WireRoomStatusNotify_write(const struct WireRoomStatusNotify *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireRoomStatusNotify");
	_pkt_u32_write(&data->code, state);
	_pkt_u8_write(&data->protocolVersion, state);
	_pkt_u8_write(&data->playerCount, state);
	_pkt_u8_write(&data->playerCapacity, state);
	_pkt_u16_write(&data->playerNPS, state);
	_pkt_u16_write(&data->levelNPS, state);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->public & 1u) << 0;
	bitfield0 |= (data->managed & 1u) << 1;
	bitfield0 |= (data->skipResults & 1u) << 2;
	bitfield0 |= (data->perPlayerDifficulty & 1u) << 3;
	bitfield0 |= (data->perPlayerModifiers & 1u) << 4;
	_pkt_u8_write(&bitfield0, state);
	_pkt_vi32_write(&data->selectionMode, state);
	_pkt_String_write(&data->levelName, state);
	_pkt_String_write(&data->levelID, state);
	_pkt_ByteArrayNetSerializable_write(&data->levelCover, state);
}
static void _pkt_WireRoomCloseNotify_read(struct WireRoomCloseNotify *restrict data, struct PacketRead parent) {
}
static void _pkt_WireRoomCloseNotify_write(const struct WireRoomCloseNotify *restrict data, struct PacketWrite parent) {
}
static void _pkt_WireStatusAttach_read(struct WireStatusAttach *restrict data, struct PacketRead parent) {
}
static void _pkt_WireStatusAttach_write(const struct WireStatusAttach *restrict data, struct PacketWrite parent) {
}
static void _pkt_WireGraphConnect_read(struct WireGraphConnect *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireGraphConnect");
	_pkt_u32_read(&data->code, state);
	_pkt_String_read(&data->secret, state);
	_pkt_String_read(&data->userId, state);
	_pkt_WireServerConfiguration_read(&data->configuration, state);
	_pkt_u32_read(&data->protocolVersion, state);
	_pkt_u8_read(&data->gameVersion, state);
}
static void _pkt_WireGraphConnect_write(const struct WireGraphConnect *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireGraphConnect");
	_pkt_u32_write(&data->code, state);
	_pkt_String_write(&data->secret, state);
	_pkt_String_write(&data->userId, state);
	_pkt_WireServerConfiguration_write(&data->configuration, state);
	_pkt_u32_write(&data->protocolVersion, state);
	_pkt_u8_write(&data->gameVersion, state);
}
static void _pkt_WireGraphConnectResp_read(struct WireGraphConnectResp *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireGraphConnectResp");
	_pkt_u8_read(&data->result, state);
	if(data->result == MultiplayerPlacementErrorCode_Success) {
		_pkt_GameplayServerConfiguration_read(&data->configuration, state);
		_pkt_u32_read(&data->hostId, state);
		_pkt_IPEndPoint_read(&data->endPoint, state);
		_pkt_u32_read(&data->roomSlot, state);
		_pkt_u32_read(&data->playerSlot, state);
		_pkt_u32_read(&data->code, state);
	}
}
static void _pkt_WireGraphConnectResp_write(const struct WireGraphConnectResp *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireGraphConnectResp");
	_pkt_u8_write(&data->result, state);
	if(data->result == MultiplayerPlacementErrorCode_Success) {
		_pkt_GameplayServerConfiguration_write(&data->configuration, state);
		_pkt_u32_write(&data->hostId, state);
		_pkt_IPEndPoint_write(&data->endPoint, state);
		_pkt_u32_write(&data->roomSlot, state);
		_pkt_u32_write(&data->playerSlot, state);
		_pkt_u32_write(&data->code, state);
	}
}
void _pkt_WireMessage_read(struct WireMessage *restrict data, struct PacketRead parent) {
	struct PacketRead state = scope(parent, "WireMessage");
	_pkt_u32_read(&data->cookie, state);
	_pkt_u8_read(&data->type, state);
	switch(data->type) {
		case WireMessageType_WireInstanceAttach: _pkt_WireInstanceAttach_read(&data->instanceAttach, state); break;
		case WireMessageType_WireStatusAttach: _pkt_WireStatusAttach_read(&data->statusAttach, state); break;
		case WireMessageType_WireRoomStatusNotify: _pkt_WireRoomStatusNotify_read(&data->roomStatusNotify, state); break;
		case WireMessageType_WireRoomCloseNotify: _pkt_WireRoomCloseNotify_read(&data->roomCloseNotify, state); break;
		case WireMessageType_WireRoomManagedJoinResp: _pkt_WireRoomManagedJoinResp_read(&data->roomManagedJoinResp, state); break;
		case WireMessageType_WireRoomSpawn: _pkt_WireRoomSpawn_read(&data->roomSpawn, state); break;
		case WireMessageType_WireRoomSpawnResp: _pkt_WireRoomSpawnResp_read(&data->roomSpawnResp, state); break;
		case WireMessageType_WireRoomJoin: _pkt_WireRoomJoin_read(&data->roomJoin, state); break;
		case WireMessageType_WireRoomJoinResp: _pkt_WireRoomJoinResp_read(&data->roomJoinResp, state); break;
		case WireMessageType_WireRoomQuery: _pkt_WireRoomQuery_read(&data->roomQuery, state); break;
		case WireMessageType_WireRoomQueryResp: _pkt_WireRoomQueryResp_read(&data->roomQueryResp, state); break;
		case WireMessageType_WireGraphConnect: _pkt_WireGraphConnect_read(&data->graphConnect, state); break;
		case WireMessageType_WireGraphConnectResp: _pkt_WireGraphConnectResp_read(&data->graphConnectResp, state); break;
		default: uprintf("Invalid value for enum `WireMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
void _pkt_WireMessage_write(const struct WireMessage *restrict data, struct PacketWrite parent) {
	struct PacketWrite state = scope(parent, "WireMessage");
	_pkt_u32_write(&data->cookie, state);
	_pkt_u8_write(&data->type, state);
	switch(data->type) {
		case WireMessageType_WireInstanceAttach: _pkt_WireInstanceAttach_write(&data->instanceAttach, state); break;
		case WireMessageType_WireStatusAttach: _pkt_WireStatusAttach_write(&data->statusAttach, state); break;
		case WireMessageType_WireRoomStatusNotify: _pkt_WireRoomStatusNotify_write(&data->roomStatusNotify, state); break;
		case WireMessageType_WireRoomCloseNotify: _pkt_WireRoomCloseNotify_write(&data->roomCloseNotify, state); break;
		case WireMessageType_WireRoomManagedJoinResp: _pkt_WireRoomManagedJoinResp_write(&data->roomManagedJoinResp, state); break;
		case WireMessageType_WireRoomSpawn: _pkt_WireRoomSpawn_write(&data->roomSpawn, state); break;
		case WireMessageType_WireRoomSpawnResp: _pkt_WireRoomSpawnResp_write(&data->roomSpawnResp, state); break;
		case WireMessageType_WireRoomJoin: _pkt_WireRoomJoin_write(&data->roomJoin, state); break;
		case WireMessageType_WireRoomJoinResp: _pkt_WireRoomJoinResp_write(&data->roomJoinResp, state); break;
		case WireMessageType_WireRoomQuery: _pkt_WireRoomQuery_write(&data->roomQuery, state); break;
		case WireMessageType_WireRoomQueryResp: _pkt_WireRoomQueryResp_write(&data->roomQueryResp, state); break;
		case WireMessageType_WireGraphConnect: _pkt_WireGraphConnect_write(&data->graphConnect, state); break;
		case WireMessageType_WireGraphConnectResp: _pkt_WireGraphConnectResp_write(&data->graphConnectResp, state); break;
		default: uprintf("Invalid value for enum `WireMessageType`\n"); assert(trace != NULL); longjmp(trace->fail, 1);
	}
}
