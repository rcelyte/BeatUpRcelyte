using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static System.Threading.Tasks.Task announceTask = System.Threading.Tasks.Task.FromResult<(Hash256, System.ArraySegment<byte>)>((default, default));
	static async System.Threading.Tasks.Task<EntitlementsStatus> AnnounceWrapper(EntitlementsStatus status, string levelId) {
		Log.Debug($"AnnounceWrapper(task.Result={status})");
		ShareInfo info = new ShareInfo(0, new ShareMeta(0), new ShareId {usage = ShareableType.BeatmapSet, mimeType = "application/json", name = levelId});
		if(status == EntitlementsStatus.Ok) {
			BeatmapLevelsModel.GetBeatmapLevelResult result = await Resolve<BeatmapLevelsModel>()!.GetBeatmapLevelAsync(levelId, System.Threading.CancellationToken.None);
			if(result.isError) {
				status = EntitlementsStatus.NotOwned;
			} else if(result.beatmapLevel is CustomBeatmapLevel level) {
				await announceTask;
				if(ShareCache.TryGet(levelId, out ShareData cached)) {
					info = cached.info;
				} else {
					System.Threading.Tasks.Task<(Hash256, System.ArraySegment<byte>)> zipTask = ZipLevel(level, System.Threading.CancellationToken.None, progress =>
						Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Exporting, progress)));
					announceTask = zipTask;
					(Hash256 hash, System.ArraySegment<byte> data) = await zipTask;
					if(data != null)
						info = ShareCache.Add(levelId, hash, data);
				}
			}
		}
		Net.Send(info.Wrap());
		return status;
	}

	static bool MissingRequirements(RecommendPreview? preview) {
		if((preview?.requirements.Length ?? 0) < 1)
			return false;
		if(!haveSongCore)
			return true;
		return BeatUpClient_SongCore.MissingRequirements(preview!);
	}

	internal static async System.Threading.Tasks.Task<EntitlementsStatus> EntitlementWrapper(System.Threading.Tasks.Task<EntitlementsStatus> task, IMenuRpcManager rpcManager, string levelId) {
		ShareId id = new ShareId {
			usage = ShareableType.BeatmapSet,
			mimeType = "application/json",
			name = levelId,
		};
		System.Threading.Tasks.TaskCompletionSource<bool> canDownload = new System.Threading.Tasks.TaskCompletionSource<bool>();
		System.Collections.Generic.HashSet<IConnectedPlayer> potentialSources = Net.beatUpPlayers.Keys.ToHashSet();
		void Remove(string userId) {
			potentialSources.RemoveWhere(player => player.userId == userId);
			if(potentialSources.Count == 0)
				canDownload.TrySetResult(false);
		}
		void OnEntitlement(string userId, string userLevelId, EntitlementsStatus entitlement) {
			if(userLevelId != levelId)
				return;
			Log.Debug($"    No ShareInfo from user `{userId}`");
			Remove(userId);
		}
		void OnDisconnect(IConnectedPlayer player) =>
			Remove(player.userId);
		void OnShareInfo(ShareInfo info, IConnectedPlayer player) {
			if(info.id.usage != id.usage || info.id.name != id.name)
				return;
			if(info.meta.byteLength > 0 && info.id.mimeType == id.mimeType) {
				Log.Debug($"    User `{player.userId}` can share!");
				canDownload.TrySetResult(true);
			} else {
				Log.Debug($"    Invalid ShareInfo from user `{player.userId}`");
				Remove(player.userId);
			}
		}
		MenuRpcManager menuRpcManager = (MenuRpcManager)rpcManager;
		try { // Callbacks must be registered immediately to avoid missing responses
			menuRpcManager.setIsEntitledToLevelEvent += OnEntitlement;
			Net.onDisconnect += OnDisconnect;
			Net.onShareInfo += OnShareInfo;
			EntitlementsStatus status = await AnnounceWrapper(await task, id.name);
			RecommendPreview? preview = playerData.ResolvePreview(levelId);
			if(MissingRequirements(preview))
				return EntitlementsStatus.NotOwned;
			if(status != EntitlementsStatus.NotOwned || !BeatUpClient_Config.Instance.DirectDownloads || preview?.requirements.All(req => SafeMods.Contains(req)) == false)
				return status;
			if(ShareCache.CheckAvailability(levelId) != ShareCache.Status.None)
				return EntitlementsStatus.NotDownloaded;
			return await canDownload.Task ? EntitlementsStatus.NotDownloaded : EntitlementsStatus.NotOwned;
		} catch(System.Exception ex) {
			Log.Critical($"Entitlement check failed: {ex}");
			return EntitlementsStatus.NotOwned;
		} finally {
			Log.Debug("EntitlementWrapper() end");
			Net.onShareInfo -= OnShareInfo;
			Net.onDisconnect -= OnDisconnect;
			menuRpcManager.setIsEntitledToLevelEvent -= OnEntitlement;
		}
	}

	internal static async void HandleGetIsEntitledToLevel<T>(T instance, string levelId, IMenuRpcManager _rpcManager) where T : NetworkPlayerEntitlementChecker =>
		_rpcManager.SetIsEntitledToLevel(levelId, await EntitlementWrapper(instance.GetEntitlementStatus(levelId), _rpcManager, levelId));

	[Patch(PatchType.Prefix, typeof(NetworkPlayerEntitlementChecker), nameof(NetworkPlayerEntitlementChecker.HandleGetIsEntitledToLevel))]
	public static bool NetworkPlayerEntitlementChecker_HandleGetIsEntitledToLevel(NetworkPlayerEntitlementChecker __instance, string levelId, IMenuRpcManager ____rpcManager) {
		Log.Debug($"NetworkPlayerEntitlementChecker_HandleGetIsEntitledToLevel(levelId=\"{levelId}\")");
		HandleGetIsEntitledToLevel(__instance, levelId, ____rpcManager);
		return false;
	}

	[Patch(PatchType.Prefix, typeof(MenuRpcManager), "InvokeSetSelectedBeatmap")]
	public static void MenuRpcManager_InvokeSetSelectedBeatmap(string userId, BeatmapIdentifierNetSerializable identifier) {
		RecommendPreview? preview = playerData.ResolvePreview(identifier.levelID);
		if(preview != null)
			playerData.previews[PlayerIndex(Net.GetPlayer(userId))] = preview;
		_ = AnnounceWrapper(EntitlementsStatus.Ok, identifier.levelID);
	}
}
