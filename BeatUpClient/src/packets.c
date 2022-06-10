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
void _pkt_BeatUpMessage_read(struct BeatUpMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
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
void _pkt_BeatUpMessage_write(const struct BeatUpMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
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
void _pkt_BeatUpConnectInfo_read(struct BeatUpConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->windowSize, pkt, end, ctx);
	_pkt_u8_read(&data->countdownDuration, pkt, end, ctx);
	uint8_t bitfield0;
	_pkt_u8_read(&bitfield0, pkt, end, ctx);
	data->directDownloads = bitfield0 >> 0 & 1;
	data->skipResults = bitfield0 >> 1 & 1;
	data->perPlayerDifficulty = bitfield0 >> 2 & 1;
	data->perPlayerModifiers = bitfield0 >> 3 & 1;
}
static void _pkt_BeatUpConnectInfo_write(const struct BeatUpConnectInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->windowSize, pkt, end, ctx);
	_pkt_u8_write(&data->countdownDuration, pkt, end, ctx);
	uint8_t bitfield0 = 0;
	bitfield0 |= (data->directDownloads & 1u) << 0;
	bitfield0 |= (data->skipResults & 1u) << 1;
	bitfield0 |= (data->perPlayerDifficulty & 1u) << 2;
	bitfield0 |= (data->perPlayerModifiers & 1u) << 3;
	_pkt_u8_write(&bitfield0, pkt, end, ctx);
}
void _pkt_BeatUpConnectHeader_read(struct BeatUpConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_read(&data->protocolId, pkt, end, ctx);
	_pkt_BeatUpConnectInfo_read(&data->base, pkt, end, ctx);
}
void _pkt_BeatUpConnectHeader_write(const struct BeatUpConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_u32_write(&data->protocolId, pkt, end, ctx);
	_pkt_BeatUpConnectInfo_write(&data->base, pkt, end, ctx);
}
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_read(&data->length, pkt, end, ctx);
	_pkt_String_read(&data->name, pkt, end, ctx);
}
void _pkt_ModConnectHeader_write(const struct ModConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_vu32_write(&data->length, pkt, end, ctx);
	_pkt_String_write(&data->name, pkt, end, ctx);
}
void _pkt_BeatUpConnectHeaderFull_write(const struct BeatUpConnectHeaderFull *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx) {
	_pkt_String_write(&data->name, pkt, end, ctx);
	_pkt_BeatUpConnectHeader_write(&data->base, pkt, end, ctx);
}
