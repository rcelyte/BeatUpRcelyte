using static System.Linq.Enumerable;

static partial class BeatUpClient {
	[Detour(typeof(MultiplayerMenuInstaller), nameof(MultiplayerMenuInstaller.InstallBindings))]
	static void MultiplayerMenuInstaller_InstallBindings(MultiplayerMenuInstaller self) {
		Base(self);
		Injected<MultiplayerLevelLoader>.Resolve<MultiplayerLevelLoader>(self.Container);
	}

	internal static bool waitForMpCore = false;
	[Detour(typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	static void MultiplayerLevelLoader_LoadLevel(MultiplayerLevelLoader self, ILevelGameplaySetupData gameplaySetupData, long initialStartTime) {
		waitForMpCore = haveMpCore;
		Base(self, gameplaySetupData, initialStartTime);
	}

	enum MultiplayerBeatmapLoaderState {
		NotLoading,
		LoadingBeatmap,
		WaitingForCountdown
	}

	static async System.Threading.Tasks.Task<LoadBeatmapLevelDataResult> DownloadLevel(ShareTracker.DownloadPreview preview, System.Threading.CancellationToken cancellationToken) {
		byte[]? data = await preview.Fetch(progress => {
			Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Downloading, progress));
		});
		if(!(data?.Length > 0)) {
			Log.Debug("Fetch failed");
			return LoadBeatmapLevelDataResult.Error;
		}
		Net.SetLocalProgress(new LoadProgress(LoadState.Loading, 0));
		Log.Debug("Unzipping level");
		IBeatmapLevelData? level = await UnzipLevel(preview.levelID, data, cancellationToken);
		Log.Debug("Load " + ((level == null) ? "failed" : "finished"));
		System.Threading.CancellationTokenSource? loaderCTS = Resolve<MultiplayerLevelLoader>()?._getBeatmapCancellationTokenSource;
		if(!haveMpCore && loaderCTS != null && cancellationToken == loaderCTS.Token && (int)HarmonyLib.AccessTools.Field(typeof(MultiplayerLevelLoader), "_loaderState").GetValue(Resolve<MultiplayerLevelLoader>()) == (int)MultiplayerBeatmapLoaderState.LoadingBeatmap)
			Resolve<IMenuRpcManager>()?.SetIsEntitledToLevel(preview.levelID, /*(level == null) ? EntitlementsStatus.NotOwned :*/ EntitlementsStatus.Ok);
		return LoadBeatmapLevelDataResult.FromValue(level); // TODO: free zipped data to cut down on memory usage
	}

	[Detour(typeof(BeatmapLevelLoader), nameof(BeatmapLevelLoader.LoadBeatmapLevelDataAsync))]
	static System.Threading.Tasks.Task<LoadBeatmapLevelDataResult> BeatmapLevelLoader_LoadBeatmapLevelDataAsync(BeatmapLevelLoader self, BeatmapLevel beatmapLevel, BeatmapLevelDataVersion beatmapLevelDataVersion, System.Threading.CancellationToken cancellationToken) {
		ShareTracker.DownloadPreview? preview = beatmapLevel as ShareTracker.DownloadPreview;
		if(preview == null)
			return (System.Threading.Tasks.Task<LoadBeatmapLevelDataResult>)Base(self, beatmapLevel, beatmapLevelDataVersion, cancellationToken);
		System.Threading.CancellationTokenSource? loaderCTS = Resolve<MultiplayerLevelLoader>()?._getBeatmapCancellationTokenSource;
		if(loaderCTS == null || cancellationToken != loaderCTS.Token)
			return System.Threading.Tasks.Task.FromResult(LoadBeatmapLevelDataResult.Error);
		if(waitForMpCore) { // MultiplayerCore causes this method to run twice, discarding the first result
			waitForMpCore = false;
			return System.Threading.Tasks.Task.FromResult(LoadBeatmapLevelDataResult.Error);
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
