using static System.Linq.Enumerable;

static partial class BeatUpClient {
	[Detour(typeof(MultiplayerMenuInstaller), nameof(MultiplayerMenuInstaller.InstallBindings))]
	static void MultiplayerMenuInstaller_InstallBindings(MultiplayerMenuInstaller self) {
		Base(self);
		Injected<MultiplayerLevelLoader>.Resolve<MultiplayerLevelLoader>(self.Container);
	}

	internal static bool waitForMpCore = false;
	[Detour(typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	static void MultiplayerLevelLoader_LoadLevel(MultiplayerLevelLoader self, ILevelGameplaySetupData gameplaySetupData, float initialStartTime) {
		waitForMpCore = haveMpCore;
		Base(self, gameplaySetupData, initialStartTime);
	}

	static async System.Threading.Tasks.Task<CustomBeatmapLevel?> DownloadLevel(ShareTracker.DownloadPreview preview, System.Threading.CancellationToken cancellationToken) {
		byte[]? data = await preview.Fetch(progress => {
			Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Downloading, progress));
		});
		if(!(data?.Length > 0)) {
			Log.Debug("Fetch failed");
			return null;
		}
		Net.SetLocalProgress(new LoadProgress(LoadState.Loading, 0));
		Log.Debug("Unzipping level");
		CustomBeatmapLevel? level = await UnzipLevel(preview.levelID, data, cancellationToken);
		Log.Debug("Load " + ((level == null) ? "failed" : "finished"));
		return level; // TODO: free zipped data to cut down on memory usage
	}

	[Detour(typeof(CustomLevelLoader), nameof(CustomLevelLoader.LoadCustomBeatmapLevelAsync))]
	static System.Threading.Tasks.Task<CustomBeatmapLevel?> CustomLevelLoader_LoadCustomBeatmapLevelAsync(CustomLevelLoader self, CustomPreviewBeatmapLevel customPreviewBeatmapLevel, System.Threading.CancellationToken cancellationToken) {
		ShareTracker.DownloadPreview? preview = customPreviewBeatmapLevel as ShareTracker.DownloadPreview;
		if(preview == null)
			return (System.Threading.Tasks.Task<CustomBeatmapLevel?>)Base(self, customPreviewBeatmapLevel, cancellationToken);
		if(cancellationToken != Resolve<MultiplayerLevelLoader>()?._getBeatmapCancellationTokenSource.Token)
			return System.Threading.Tasks.Task.FromResult<CustomBeatmapLevel?>(null);
		if(waitForMpCore) { // MultiplayerCore causes this method to run twice, discarding the first result
			waitForMpCore = false;
			return System.Threading.Tasks.Task.FromResult<CustomBeatmapLevel?>(null);
		}
		return DownloadLevel(preview, cancellationToken);
	}
}
