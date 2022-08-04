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
};
[[maybe_unused]] static const char *_reflect_BeatUpMessageType(BeatUpMessageType value) {
	switch(value) {
		case BeatUpMessageType_ConnectInfo: return "ConnectInfo";
		case BeatUpMessageType_RecommendPreview: return "RecommendPreview";
		case BeatUpMessageType_ShareInfo: return "ShareInfo";
		case BeatUpMessageType_DataFragmentRequest: return "DataFragmentRequest";
		case BeatUpMessageType_DataFragment: return "DataFragment";
		case BeatUpMessageType_LoadProgress: return "LoadProgress";
		default: return "???";
	}
}
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
struct BeatUpMessage {
	BeatUpMessageType type;
	union {
		struct ConnectInfo connectInfo;
		struct RecommendPreview recommendPreview;
		struct ShareInfo shareInfo;
		struct DataFragmentRequest dataFragmentRequest;
		struct DataFragment dataFragment;
		struct LoadProgress loadProgress;
	};
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
struct ModConnectHeader {
	uint32_t length;
	struct String name;
};
struct ServerConnectInfo_Prefixed {
	struct String name;
	struct ServerConnectInfo base;
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
void _pkt_ServerConnectInfo_read(struct ServerConnectInfo *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ServerConnectInfo_write(const struct ServerConnectInfo *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ModConnectHeader_read(struct ModConnectHeader *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ModConnectHeader_write(const struct ModConnectHeader *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
void _pkt_ServerConnectInfo_Prefixed_write(const struct ServerConnectInfo_Prefixed *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define reflect(type, value) _reflect_##type(value)
typedef void (*PacketWriteFunc)(const void *restrict, uint8_t**, const uint8_t*, struct PacketContext);
typedef void (*PacketReadFunc)(void *restrict, const uint8_t**, const uint8_t*, struct PacketContext);
size_t _pkt_try_read(PacketReadFunc inner, void *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
size_t _pkt_try_write(PacketWriteFunc inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);
#define pkt_write_c(pkt, end, ctx, type, ...) _pkt_try_write((PacketWriteFunc)_pkt_##type##_write, &(struct type)__VA_ARGS__, pkt, end, ctx)
#define pkt_read(data, ...) _pkt_try_read((PacketReadFunc)_Generic(*(data), struct BeatUpMessage: _pkt_BeatUpMessage_read, struct ServerConnectInfo: _pkt_ServerConnectInfo_read, struct ModConnectHeader: _pkt_ModConnectHeader_read), data, __VA_ARGS__)
#define pkt_write(data, ...) _pkt_try_write((PacketWriteFunc)_Generic(*(data), struct BeatUpMessage: _pkt_BeatUpMessage_write, struct ServerConnectInfo: _pkt_ServerConnectInfo_write, struct ModConnectHeader: _pkt_ModConnectHeader_write, struct ServerConnectInfo_Prefixed: _pkt_ServerConnectInfo_Prefixed_write), data, __VA_ARGS__)
size_t pkt_write_bytes(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx, size_t count);
