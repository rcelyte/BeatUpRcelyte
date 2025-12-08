static struct WriterString WriterString_FromString(const struct String from) {
	struct WriterString out = {.length = from.length};
	static_assert(sizeof(out.data) == sizeof(from.data));
	memcpy(out.data, from.data, out.length);
	return out;
}

static struct String String_FromWriterString(const struct WriterString from) {
	struct String out = {.length = from.length};
	static_assert(sizeof(out.data) == sizeof(from.data));
	memcpy(out.data, from.data, out.length);
	return out;
}

static struct WriterColor WriterColor_FromColor32(const struct Color32 from) {
	return (struct WriterColor){from.r / 255.f, from.g / 255.f, from.b / 255.f, from.a / 255.f};
}

static struct Color32 Color32_FromWriterColor(const struct WriterColor from) {
	// TODO: float range check to avoid potential UB
	return (struct Color32){(uint8_t)(from.r * 255), (uint8_t)(from.g * 255), (uint8_t)(from.b * 255), (uint8_t)(from.a * 255)};
}

static const uint32_t BeatAvatarMagic = 0x5610cc60; // TODO: no magic
static struct OpaqueAvatarData OpaqueAvatarData_FromLegacy(const struct LegacyAvatarData *const legacy, const struct PacketContext version) {
	struct OpaqueAvatarData out = {.typeHash = BeatAvatarMagic};
	out.length = (uint16_t)pkt_write_c(&(uint8_t*){out.data}, endof(out.data), version, BeatAvatarData, {
		.headTopId = WriterString_FromString(legacy->headTopId),
		.headTopPrimaryColor = WriterColor_FromColor32(legacy->headTopPrimaryColor),
		.headTopSecondaryColor = WriterColor_FromColor32(legacy->headTopSecondaryColor),
		.glassesId = WriterString_FromString(legacy->glassesId),
		.glassesColor = WriterColor_FromColor32(legacy->glassesColor),
		.facialHairId = WriterString_FromString(legacy->facialHairId),
		.facialHairColor = WriterColor_FromColor32(legacy->facialHairColor),
		.handsId = WriterString_FromString(legacy->handsId),
		.handsColor = WriterColor_FromColor32(legacy->handsColor),
		.clothesId = WriterString_FromString(legacy->clothesId),
		.clothesPrimaryColor = WriterColor_FromColor32(legacy->clothesPrimaryColor),
		.clothesSecondaryColor = WriterColor_FromColor32(legacy->clothesSecondaryColor),
		.clothesDetailColor = WriterColor_FromColor32(legacy->clothesDetailColor),
		.skinColorId = WriterString_FromString(legacy->glassesId), // TODO: check if any part IDs need to be remapped between 1.32.0+ and legacy
		.eyesId = WriterString_FromString(legacy->eyesId),
		.mouthId = WriterString_FromString(legacy->mouthId),
	});
	return out;
}

static struct LegacyAvatarData LegacyAvatarData_FromOpaque(const struct OpaqueAvatarData *const from, const struct PacketContext version) {
	struct BeatAvatarData avatar = {0};
	if(!pkt_read(&avatar, &(const uint8_t*){from->data}, endof(from->data), version))
		return (struct LegacyAvatarData){0};
	return (struct LegacyAvatarData){
		.headTopId = String_FromWriterString(avatar.headTopId),
		.headTopPrimaryColor = Color32_FromWriterColor(avatar.headTopPrimaryColor),
		.handsColor = Color32_FromWriterColor(avatar.handsColor),
		.clothesId = String_FromWriterString(avatar.clothesId),
		.clothesPrimaryColor = Color32_FromWriterColor(avatar.clothesPrimaryColor),
		.clothesSecondaryColor = Color32_FromWriterColor(avatar.clothesSecondaryColor),
		.clothesDetailColor = Color32_FromWriterColor(avatar.clothesDetailColor),
		.eyesId = String_FromWriterString(avatar.eyesId),
		.mouthId = String_FromWriterString(avatar.mouthId),
		.glassesColor = Color32_FromWriterColor(avatar.glassesColor),
		.facialHairColor = Color32_FromWriterColor(avatar.facialHairColor),
		.headTopSecondaryColor = Color32_FromWriterColor(avatar.headTopSecondaryColor),
		.glassesId = String_FromWriterString(avatar.glassesId),
		.facialHairId = String_FromWriterString(avatar.facialHairId),
		.handsId = String_FromWriterString(avatar.handsId),
	};
}
