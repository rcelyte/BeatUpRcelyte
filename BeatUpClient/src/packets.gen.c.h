static void _pkt_ByteArrayNetSerializable_read(struct ByteArrayNetSerializable *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_read(&data->length, pkt, end, ctx);
	_pkt_raw_read(data->data, pkt, end, ctx, check_overflow(data->length, 8192, "ByteArrayNetSerializable.data"));
}
static void _pkt_ByteArrayNetSerializable_write(const struct ByteArrayNetSerializable *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_write(&data->length, pkt, end, ctx);
	_pkt_raw_write(data->data, pkt, end, ctx, check_overflow(data->length, 8192, "ByteArrayNetSerializable.data"));
}
static void _pkt_ConnectInfo_read(struct ConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->protocolId, pkt, end, ctx);
	_pkt_u16_read(&data->blockSize, pkt, end, ctx);
}
static void _pkt_ConnectInfo_write(const struct ConnectInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->protocolId, pkt, end, ctx);
	_pkt_u16_write(&data->blockSize, pkt, end, ctx);
}
static void _pkt_PreviewDifficultyBeatmapSet_read(struct PreviewDifficultyBeatmapSet *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_read(&data->characteristic, pkt, end, ctx);
	_pkt_u8_read(&data->difficulties_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->difficulties_len, 5, "PreviewDifficultyBeatmapSet.difficulties"); i < count; ++i)
		_pkt_vu32_read(&data->difficulties[i], pkt, end, ctx);
}
static void _pkt_PreviewDifficultyBeatmapSet_write(const struct PreviewDifficultyBeatmapSet *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->characteristic, pkt, end, ctx);
	_pkt_u8_write(&data->difficulties_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->difficulties_len, 5, "PreviewDifficultyBeatmapSet.difficulties"); i < count; ++i)
		_pkt_vu32_write(&data->difficulties[i], pkt, end, ctx);
}
static void _pkt_PreviewBeatmapLevel_read(struct PreviewBeatmapLevel *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_read(&data->levelID, pkt, end, ctx);
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
	_pkt_String_read(&data->environmentInfo, pkt, end, ctx);
	_pkt_String_read(&data->allDirectionsEnvironmentInfo, pkt, end, ctx);
	_pkt_u8_read(&data->beatmapSets_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->beatmapSets_len, 8, "PreviewBeatmapLevel.beatmapSets"); i < count; ++i)
		_pkt_PreviewDifficultyBeatmapSet_read(&data->beatmapSets[i], pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_read(&data->cover, pkt, end, ctx);
}
static void _pkt_PreviewBeatmapLevel_write(const struct PreviewBeatmapLevel *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_LongString_write(&data->levelID, pkt, end, ctx);
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
	_pkt_String_write(&data->environmentInfo, pkt, end, ctx);
	_pkt_String_write(&data->allDirectionsEnvironmentInfo, pkt, end, ctx);
	_pkt_u8_write(&data->beatmapSets_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->beatmapSets_len, 8, "PreviewBeatmapLevel.beatmapSets"); i < count; ++i)
		_pkt_PreviewDifficultyBeatmapSet_write(&data->beatmapSets[i], pkt, end, ctx);
	_pkt_ByteArrayNetSerializable_write(&data->cover, pkt, end, ctx);
}
static void _pkt_CustomLabelSet_read(struct CustomLabelSet *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->difficulties_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->difficulties_len, 5, "CustomLabelSet.difficulties"); i < count; ++i)
		_pkt_LongString_read(&data->difficulties[i], pkt, end, ctx);
}
static void _pkt_CustomLabelSet_write(const struct CustomLabelSet *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->difficulties_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->difficulties_len, 5, "CustomLabelSet.difficulties"); i < count; ++i)
		_pkt_LongString_write(&data->difficulties[i], pkt, end, ctx);
}
static void _pkt_RecommendPreview_read(struct RecommendPreview *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PreviewBeatmapLevel_read(&data->base, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->base.beatmapSets_len, 8, "RecommendPreview.labelSets"); i < count; ++i)
		_pkt_CustomLabelSet_read(&data->labelSets[i], pkt, end, ctx);
	_pkt_vu32_read(&data->requirements_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->requirements_len, 16, "RecommendPreview.requirements"); i < count; ++i)
		_pkt_String_read(&data->requirements[i], pkt, end, ctx);
	_pkt_vu32_read(&data->suggestions_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->suggestions_len, 16, "RecommendPreview.suggestions"); i < count; ++i)
		_pkt_String_read(&data->suggestions[i], pkt, end, ctx);
}
static void _pkt_RecommendPreview_write(const struct RecommendPreview *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_PreviewBeatmapLevel_write(&data->base, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->base.beatmapSets_len, 8, "RecommendPreview.labelSets"); i < count; ++i)
		_pkt_CustomLabelSet_write(&data->labelSets[i], pkt, end, ctx);
	_pkt_vu32_write(&data->requirements_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->requirements_len, 16, "RecommendPreview.requirements"); i < count; ++i)
		_pkt_String_write(&data->requirements[i], pkt, end, ctx);
	_pkt_vu32_write(&data->suggestions_len, pkt, end, ctx);
	for(uint32_t i = 0, count = check_overflow(data->suggestions_len, 16, "RecommendPreview.suggestions"); i < count; ++i)
		_pkt_String_write(&data->suggestions[i], pkt, end, ctx);
}
static void _pkt_ShareMeta_read(struct ShareMeta *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_read(&data->byteLength, pkt, end, ctx);
	_pkt_raw_read(data->hash, pkt, end, ctx, 32);
}
static void _pkt_ShareMeta_write(const struct ShareMeta *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu64_write(&data->byteLength, pkt, end, ctx);
	_pkt_raw_write(data->hash, pkt, end, ctx, 32);
}
static void _pkt_ShareId_read(struct ShareId *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_read(&data->usage, pkt, end, ctx);
	if(data->usage != ShareableType_None) {
		_pkt_String_read(&data->mimeType, pkt, end, ctx);
		_pkt_LongString_read(&data->name, pkt, end, ctx);
	}
}
static void _pkt_ShareId_write(const struct ShareId *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u16_write(&data->usage, pkt, end, ctx);
	if(data->usage != ShareableType_None) {
		_pkt_String_write(&data->mimeType, pkt, end, ctx);
		_pkt_LongString_write(&data->name, pkt, end, ctx);
	}
}
static void _pkt_ShareInfo_read(struct ShareInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->offset, pkt, end, ctx);
	_pkt_u16_read(&data->blockSize, pkt, end, ctx);
	_pkt_ShareMeta_read(&data->meta, pkt, end, ctx);
	_pkt_ShareId_read(&data->id, pkt, end, ctx);
}
static void _pkt_ShareInfo_write(const struct ShareInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->offset, pkt, end, ctx);
	_pkt_u16_write(&data->blockSize, pkt, end, ctx);
	_pkt_ShareMeta_write(&data->meta, pkt, end, ctx);
	_pkt_ShareId_write(&data->id, pkt, end, ctx);
}
static void _pkt_DataFragmentRequest_read(struct DataFragmentRequest *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->offset, pkt, end, ctx);
	_pkt_u8_read(&data->count, pkt, end, ctx);
}
static void _pkt_DataFragmentRequest_write(const struct DataFragmentRequest *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->offset, pkt, end, ctx);
	_pkt_u8_write(&data->count, pkt, end, ctx);
}
static void _pkt_DataFragment_read(struct DataFragment *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->offset, pkt, end, ctx);
}
static void _pkt_DataFragment_write(const struct DataFragment *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->offset, pkt, end, ctx);
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
void _pkt_BeatUpMessage_read(struct BeatUpMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_read(&data->type, pkt, end, ctx);
	switch(data->type) {
		case BeatUpMessageType_ConnectInfo: _pkt_ConnectInfo_read(&data->connectInfo, pkt, end, ctx); break;
		case BeatUpMessageType_RecommendPreview: _pkt_RecommendPreview_read(&data->recommendPreview, pkt, end, ctx); break;
		case BeatUpMessageType_ShareInfo: _pkt_ShareInfo_read(&data->shareInfo, pkt, end, ctx); break;
		case BeatUpMessageType_DataFragmentRequest: _pkt_DataFragmentRequest_read(&data->dataFragmentRequest, pkt, end, ctx); break;
		case BeatUpMessageType_DataFragment: _pkt_DataFragment_read(&data->dataFragment, pkt, end, ctx); break;
		case BeatUpMessageType_LoadProgress: _pkt_LoadProgress_read(&data->loadProgress, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `BeatUpMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_BeatUpMessage_write(const struct BeatUpMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u8_write(&data->type, pkt, end, ctx);
	switch(data->type) {
		case BeatUpMessageType_ConnectInfo: _pkt_ConnectInfo_write(&data->connectInfo, pkt, end, ctx); break;
		case BeatUpMessageType_RecommendPreview: _pkt_RecommendPreview_write(&data->recommendPreview, pkt, end, ctx); break;
		case BeatUpMessageType_ShareInfo: _pkt_ShareInfo_write(&data->shareInfo, pkt, end, ctx); break;
		case BeatUpMessageType_DataFragmentRequest: _pkt_DataFragmentRequest_write(&data->dataFragmentRequest, pkt, end, ctx); break;
		case BeatUpMessageType_DataFragment: _pkt_DataFragment_write(&data->dataFragment, pkt, end, ctx); break;
		case BeatUpMessageType_LoadProgress: _pkt_LoadProgress_write(&data->loadProgress, pkt, end, ctx); break;
		default: uprintf("Invalid value for enum `BeatUpMessageType`\n"); longjmp(fail, 1);
	}
}
void _pkt_ServerConnectInfo_read(struct ServerConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ConnectInfo_read(&data->base, pkt, end, ctx);
	_pkt_u32_read(&data->windowSize, pkt, end, ctx);
	_pkt_u8_read(&data->countdownDuration, pkt, end, ctx);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->directDownloads = bitfield0 >> 0 & 1;
	data->skipResults = bitfield0 >> 1 & 1;
	data->perPlayerDifficulty = bitfield0 >> 2 & 1;
	data->perPlayerModifiers = bitfield0 >> 3 & 1;
}
void _pkt_ServerConnectInfo_write(const struct ServerConnectInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_ConnectInfo_write(&data->base, pkt, end, ctx);
	_pkt_u32_write(&data->windowSize, pkt, end, ctx);
	_pkt_u8_write(&data->countdownDuration, pkt, end, ctx);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->directDownloads & 1u) << 0;
	bitfield0 |= (data->skipResults & 1u) << 1;
	bitfield0 |= (data->perPlayerDifficulty & 1u) << 2;
	bitfield0 |= (data->perPlayerModifiers & 1u) << 3;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
}
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_read(&data->length, pkt, end, ctx);
	_pkt_String_read(&data->name, pkt, end, ctx);
}
void _pkt_ModConnectHeader_write(const struct ModConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_write(&data->length, pkt, end, ctx);
	_pkt_String_write(&data->name, pkt, end, ctx);
}
void _pkt_ServerConnectInfo_Prefixed_write(const struct ServerConnectInfo_Prefixed *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->name, pkt, end, ctx);
	_pkt_ServerConnectInfo_write(&data->base, pkt, end, ctx);
}
