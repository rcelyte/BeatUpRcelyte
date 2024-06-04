using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static System.Threading.Tasks.Task announceTask = System.Threading.Tasks.Task.FromResult<(Hash256, System.ArraySegment<byte>)>((default, default));
	static async System.Threading.Tasks.Task<EntitlementsStatus> AnnounceWrapper(EntitlementsStatus status, string levelId) {
		Log.Debug($"AnnounceWrapper(task.Result={status})");
		ShareInfo info = new ShareInfo(0, new ShareMeta(0), new ShareId {usage = ShareableType.BeatmapSet, mimeType = "application/json", name = levelId});
		if(status == EntitlementsStatus.Ok) {
			IBeatmapLevelData? beatmapLevelData = (await Resolve<BeatmapLevelsModel>()!.LoadBeatmapLevelDataAsync(levelId, System.Threading.CancellationToken.None)).beatmapLevelData;
			if(beatmapLevelData == null) {
				Log.Debug("    Announce: failed to load beatmap");
				status = EntitlementsStatus.NotOwned;
			} else if(beatmapLevelData is FileSystemBeatmapLevelData fileLevel) {
				await announceTask;
				ShareInfo oldInfo = ShareProvider.GetCurrent().info;
				if(oldInfo.id.name == levelId) {
					info = oldInfo;
				} else {
					System.Threading.Tasks.Task<(Hash256, System.ArraySegment<byte>)> zipTask = ZipLevel(fileLevel, System.Threading.CancellationToken.None, progress =>
						Net.SetLocalProgressUnreliable(new LoadProgress(LoadState.Exporting, progress)));
					announceTask = zipTask;
					(Hash256 hash, System.ArraySegment<byte> data) = await zipTask;
					if(data.Count != 0)
						info = ShareProvider.Set(levelId, hash, data);
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
		void OnEntitlement(string userId, string userLevelId, EntitlementsStatus status) {
			if(userLevelId == levelId)
				Remove(userId);
		}
		void OnDisconnect(IConnectedPlayer player) =>
			Remove(player.userId);
		void OnShareInfo(ShareInfo info, IConnectedPlayer player, bool processed) {
			if(info.id.usage != id.usage || info.id.name != id.name)
				return;
			if(processed) {
				Log.Debug($"    User `{player.userId}` can share!");
				canDownload.TrySetResult(true);
			}
			Remove(player.userId);
		}
		MenuRpcManager menuRpcManager = (MenuRpcManager)rpcManager;
		try { // Callbacks must be registered immediately to avoid missing responses
			menuRpcManager.setIsEntitledToLevelEvent += OnEntitlement;
			Net.onDisconnect += OnDisconnect;
			ShareTracker.onProcess += OnShareInfo;
			Remove(string.Empty); // Avoids potential softlock in solo lobbies
			/*System.Collections.Generic.Dictionary<string, IPreviewBeatmapLevel> loadedPreviews = Resolve<BeatmapLevelsModel>()!._loadedPreviewBeatmapLevels;
			if(loadedPreviews.TryGetValue(levelId, out IPreviewBeatmapLevel loaded) && (loaded is ShareTracker.DownloadPreview))
				loadedPreviews.Remove(levelId);*/

			EntitlementsStatus status = await AnnounceWrapper(await task, id.name);
			RecommendPreview? preview = playerData.ResolvePreview(levelId);
			if(MissingRequirements(preview)){
				Log.Debug("    Entitlement: missing requirements");
				return EntitlementsStatus.NotOwned;
			}
			if(status != EntitlementsStatus.NotOwned) {
				Log.Debug("    Entitlement: passthrough");
				return status;
			}
			if(!BeatUpClient_Config.Instance.DirectDownloads) {
				Log.Debug("    Entitlement: direct downloads disabled");
				return status;
			}
			if(preview?.requirements.All(req => SafeMods.Contains(req)) == false) {
				Log.Debug("    Entitlement: untrusted requirements present");
				return status;
			}
			if(await canDownload.Task) {
				Log.Debug("    Entitlement: download available");
				return EntitlementsStatus.NotDownloaded;
			}
			Log.Debug("    Entitlement: no download sources");
			return EntitlementsStatus.NotOwned;
		} catch(System.Exception ex) {
			Log.Critical($"Entitlement check failed: {ex}");
			return EntitlementsStatus.NotOwned;
		} finally {
			Log.Debug("EntitlementWrapper() end");
			ShareTracker.onProcess -= OnShareInfo;
			Net.onDisconnect -= OnDisconnect;
			menuRpcManager.setIsEntitledToLevelEvent -= OnEntitlement;
		}
	}

	internal static async void HandleGetIsEntitledToLevel(System.Threading.Tasks.Task<EntitlementsStatus> task, string levelId, IMenuRpcManager _rpcManager) =>
		_rpcManager.SetIsEntitledToLevel(levelId, await EntitlementWrapper(task, _rpcManager, levelId));

	[Patch(PatchType.Prefix, typeof(NetworkPlayerEntitlementChecker), nameof(NetworkPlayerEntitlementChecker.HandleGetIsEntitledToLevel))]
	public static bool NetworkPlayerEntitlementChecker_HandleGetIsEntitledToLevel(NetworkPlayerEntitlementChecker __instance, string levelId, IMenuRpcManager ____rpcManager) {
		Log.Debug($"NetworkPlayerEntitlementChecker_HandleGetIsEntitledToLevel(levelId=\"{levelId}\")");
		HandleGetIsEntitledToLevel(__instance.GetEntitlementStatus(levelId), levelId, ____rpcManager);
		return false;
	}

	[Detour(typeof(MenuRpcManager), nameof(MenuRpcManager.InvokeSetSelectedBeatmap))]
	public static void MenuRpcManager_InvokeSetSelectedBeatmap(MenuRpcManager self, string userId, BeatmapKeyNetSerializable key) {
		RecommendPreview? preview = playerData.ResolvePreview(key.levelID);
		if(preview != null)
			playerData.previews[PlayerIndex(Net.GetPlayer(userId))] = preview;
		_ = AnnounceWrapper(EntitlementsStatus.Ok, key.levelID);
		Base(self, userId, key);
	}
}
