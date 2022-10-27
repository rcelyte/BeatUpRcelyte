static partial class BeatUpClient {
	static async System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> LoadWrapper(System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> localLoadTask, string? levelId, System.Threading.CancellationToken cancellationToken) {
		BeatmapLevelsModel.GetBeatmapLevelResult result = await localLoadTask;
		if(result.beatmapLevel != null || levelId == null)
			return result;
		System.ArraySegment<byte> data = await ShareCache.Fetch(levelId, progress => Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Downloading, progress)));
		Net.SetLocalProgress(new LoadProgress(LoadState.Loading, 0));
		Log.Debug("Unzipping level");
		CustomBeatmapLevel? level = await UnzipLevel(levelId, data, cancellationToken);
		if(data.Count < 1)
			Log.Debug("Fetch failed");
		else
			Log.Debug("Load " + ((level == null) ? "failed" : "finished"));
		return new BeatmapLevelsModel.GetBeatmapLevelResult(level == null, level);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	public static void MultiplayerLevelLoader_LoadLevel_pre(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, out bool __state) =>
		__state = (____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.NotLoading && !haveMpCore);

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	public static void MultiplayerLevelLoader_LoadLevel_post(bool __state, ILevelGameplaySetupData gameplaySetupData, System.Threading.CancellationTokenSource ____getBeatmapCancellationTokenSource, ref System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> ____getBeatmapLevelResultTask) {
		if(__state)
			____getBeatmapLevelResultTask = LoadWrapper(____getBeatmapLevelResultTask, gameplaySetupData.beatmapLevel.beatmapLevel.levelID, ____getBeatmapCancellationTokenSource.Token);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
	public static void MultiplayerLevelLoader_Tick_pre(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, out bool __state) =>
		__state = (____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.LoadingBeatmap && !haveMpCore);

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
	public static void MultiplayerLevelLoader_Tick_post(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, bool __state, ILevelGameplaySetupData ____gameplaySetupData) {
		if(__state && ____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.WaitingForCountdown)
			Resolve<IMenuRpcManager>()!.SetIsEntitledToLevel(____gameplaySetupData.beatmapLevel.beatmapLevel.levelID, EntitlementsStatus.Ok);
	}
}
