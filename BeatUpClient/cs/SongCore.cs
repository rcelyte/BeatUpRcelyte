using static BeatUpClient;
using static System.Linq.Enumerable;

static class BeatUpClient_SongCore {
	internal static bool MissingRequirements(BeatUpClient.RecommendPreview preview) =>
		!preview.requirements.Where(req => !string.IsNullOrEmpty(req)).All(req => SongCore.Collections.capabilities.Contains(req!));

	[Patch.Overload(PatchType.Prefix, typeof(SongCore.Utilities.Hashing), nameof(SongCore.Utilities.Hashing.GetCustomLevelHash), new[] {typeof(CustomPreviewBeatmapLevel)})]
	[Patch.Overload(PatchType.Prefix, typeof(SongCore.Utilities.Hashing), nameof(SongCore.Utilities.Hashing.GetCustomLevelHash), new[] {typeof(CustomBeatmapLevel)})]
	public static bool Hashing_GetCustomLevelHash(CustomPreviewBeatmapLevel level, ref string __result) {
		if(level is HashedCustomBeatmapLevel hashed) {
			__result = hashed.hash;
			return false;
		}
		return true;
	}
}
