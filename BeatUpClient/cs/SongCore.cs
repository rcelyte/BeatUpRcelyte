using static BeatUpClient;
using static System.Linq.Enumerable;

static class BeatUpClient_SongCore {
	internal static bool MissingRequirements(BeatUpClient.RecommendPreview preview) =>
		!preview.requirements.Where(req => !string.IsNullOrEmpty(req)).All(req => SongCore.Collections.capabilities.Contains(req!));

	// TODO: check if the downloader still breaks SongCore without this
	/*[Detour(typeof(SongCore.Utilities.Hashing), nameof(SongCore.Utilities.Hashing.GetCustomLevelHash))]
	static string? Hashing_GetCustomLevelHash(BeatmapLevel level) =>
		(level is SharedBeatmapLevel shared) ? shared.hash : (string?)Base(level);*/

	[Detour(typeof(SongCore.Utilities.Utils), nameof(SongCore.Utilities.Utils.GetResource))]
	static byte[] GetResource(System.Reflection.Assembly asm, string resourceName) {
		try {
			return (byte[])Base(asm, resourceName);
		} catch(System.Exception error) {
			Log.Warn($"SongCore GetResource() failed: {error}");
			return new byte[0];
		}
	}
}
