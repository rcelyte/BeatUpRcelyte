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
		System.Threading.CancellationTokenSource? loaderCTS = Resolve<MultiplayerLevelLoader>()?._getBeatmapCancellationTokenSource;
		if(!haveMpCore && loaderCTS != null && cancellationToken == loaderCTS.Token && Resolve<MultiplayerLevelLoader>()!._loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.LoadingBeatmap)
			Resolve<IMenuRpcManager>()?.SetIsEntitledToLevel(preview.levelID, /*(level == null) ? EntitlementsStatus.NotOwned :*/ EntitlementsStatus.Ok);
		return level; // TODO: free zipped data to cut down on memory usage
	}

	[Detour(typeof(CustomLevelLoader), nameof(CustomLevelLoader.LoadCustomBeatmapLevelAsync))]
	static System.Threading.Tasks.Task<CustomBeatmapLevel?> CustomLevelLoader_LoadCustomBeatmapLevelAsync(CustomLevelLoader self, CustomPreviewBeatmapLevel customPreviewBeatmapLevel, System.Threading.CancellationToken cancellationToken) {
		ShareTracker.DownloadPreview? preview = customPreviewBeatmapLevel as ShareTracker.DownloadPreview;
		if(preview == null)
			return (System.Threading.Tasks.Task<CustomBeatmapLevel?>)Base(self, customPreviewBeatmapLevel, cancellationToken);
		System.Threading.CancellationTokenSource? loaderCTS = Resolve<MultiplayerLevelLoader>()?._getBeatmapCancellationTokenSource;
		if(loaderCTS == null || cancellationToken != loaderCTS.Token)
			return System.Threading.Tasks.Task.FromResult<CustomBeatmapLevel?>(null);
		if(waitForMpCore) { // MultiplayerCore causes this method to run twice, discarding the first result
			waitForMpCore = false;
			return System.Threading.Tasks.Task.FromResult<CustomBeatmapLevel?>(null);
		}
		return DownloadLevel(preview, cancellationToken);
	}

	[Patch(PatchType.Finalizer, typeof(BeatmapSaveDataHelpers), nameof(BeatmapSaveDataHelpers.GetVersion))]
	static System.Exception? BeatmapSaveDataHelpers_GetVersion(System.Exception? __exception, ref System.Version __result) {
		if(!(__exception is System.ArgumentException))
			return __exception;
		__result = new System.Version("2.0.0");
		return null;
	}
}
