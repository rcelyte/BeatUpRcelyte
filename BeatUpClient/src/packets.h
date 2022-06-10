#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "packets.h.h"
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
	BeatUpMessageType_RecommendPreview,
	BeatUpMessageType_SetCanShareBeatmap,
	BeatUpMessageType_DirectDownloadInfo,
	BeatUpMessageType_LevelFragmentRequest,
	BeatUpMessageType_LevelFragment,
	BeatUpMessageType_LoadProgress,
};
[[maybe_unused]] static const char *_reflect_BeatUpMessageType(BeatUpMessageType value) {
	switch(value) {
		case BeatUpMessageType_RecommendPreview: return "RecommendPreview";
		case BeatUpMessageType_SetCanShareBeatmap: return "SetCanShareBeatmap";
		case BeatUpMessageType_DirectDownloadInfo: return "DirectDownloadInfo";
		case BeatUpMessageType_LevelFragmentRequest: return "LevelFragmentRequest";
		case BeatUpMessageType_LevelFragment: return "LevelFragment";
		case BeatUpMessageType_LoadProgress: return "LoadProgress";
		default: return "???";
	}
}
struct ByteArrayNetSerializable {
	uint32_t length;
	uint8_t data[8192];
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
	struct NetworkPreviewBeatmapLevel base;
	uint32_t requirements_len;
	struct String requirements[16];
	uint32_t suggestions_len;
	struct String suggestions[16];
};
struct ShareInfo {
	struct LongString levelId;
	uint8_t levelHash[32];
	uint64_t fileSize;
};
struct SetCanShareBeatmap {
	struct ShareInfo base;
	bool canShare;
};
struct DirectDownloadInfo {
	struct ShareInfo base;
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
struct LoadProgress {
	uint32_t sequence;
	LoadState state;
	uint16_t progress;
};
struct BeatUpMessage {
	BeatUpMessageType type;
	union {
		struct RecommendPreview recommendPreview;
		struct SetCanShareBeatmap setCanShareBeatmap;
		struct DirectDownloadInfo directDownloadInfo;
		struct LevelFragmentRequest levelFragmentRequest;
		struct LevelFragment levelFragment;
		struct LoadProgress loadProgress;
	};
};
struct BeatUpConnectInfo {
	uint32_t windowSize;
	uint8_t countdownDuration;
	bool directDownloads;
	bool skipResults;
	bool perPlayerDifficulty;
	bool perPlayerModifiers;
};
struct BeatUpConnectHeader {
	uint32_t protocolId;
	struct BeatUpConnectInfo base;
};
struct ModConnectHeader {
	uint32_t length;
	struct String name;
};
struct BeatUpConnectHeaderFull {
	struct String name;
	struct BeatUpConnectHeader base;
};
struct PacketContext {
	uint8_t netVersion;
	uint8_t protocolVersion;
	uint8_t beatUpVersion;
	uint32_t windowSize;
};
static const struct PacketContext PV_LEGACY_DEFAULT = {
	.netVersion = 11,
	.protocolVersion = 6,
	.beatUpVersion = 0,
	.windowSize = 64,
};
void _pkt_BeatUpMessage_read(struct BeatUpMessage *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpMessage_write(const struct BeatUpMessage *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpConnectInfo_read(struct BeatUpConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpConnectHeader_read(struct BeatUpConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpConnectHeader_write(const struct BeatUpConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ModConnectHeader_write(const struct ModConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_BeatUpConnectHeaderFull_write(const struct BeatUpConnectHeaderFull *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define reflect(type, value) _reflect_##type(value)
typedef void (*PacketWriteFunc)(const void *restrict, uint8_t**, const uint8_t*, struct PacketContext);
typedef void (*PacketReadFunc)(void *restrict, const uint8_t**, const uint8_t*, struct PacketContext);
size_t _pkt_try_read(PacketReadFunc inner, void *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
size_t _pkt_try_write(PacketWriteFunc inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define pkt_write_c(pkt, end, ctx, type, ...) _pkt_try_write((PacketWriteFunc)_pkt_##type##_write, &(struct type)__VA_ARGS__, pkt, end, ctx)
#define pkt_read(data, ...) _pkt_try_read((PacketReadFunc)_Generic(*(data), struct BeatUpMessage: _pkt_BeatUpMessage_read, struct BeatUpConnectInfo: _pkt_BeatUpConnectInfo_read, struct BeatUpConnectHeader: _pkt_BeatUpConnectHeader_read, struct ModConnectHeader: _pkt_ModConnectHeader_read), data, __VA_ARGS__)
#define pkt_write(data, ...) _pkt_try_write((PacketWriteFunc)_Generic(*(data), struct BeatUpMessage: _pkt_BeatUpMessage_write, struct BeatUpConnectHeader: _pkt_BeatUpConnectHeader_write, struct ModConnectHeader: _pkt_ModConnectHeader_write, struct BeatUpConnectHeaderFull: _pkt_BeatUpConnectHeaderFull_write), data, __VA_ARGS__)
size_t pkt_write_bytes(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx, size_t count);
