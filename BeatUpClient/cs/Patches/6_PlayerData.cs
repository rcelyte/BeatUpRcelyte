static partial class BeatUpClient {
	// A failsafe which "should" never apear in practice
	public class ErrorBeatmapLevel : EmptyBeatmapLevel, IPreviewBeatmapLevel {
		string levelId;
		string? IPreviewBeatmapLevel.levelID => levelId;
		string? IPreviewBeatmapLevel.songName => "[ERROR]";
		string? IPreviewBeatmapLevel.songAuthorName => "[ERROR]";
		System.Threading.Tasks.Task<UnityEngine.Sprite> IPreviewBeatmapLevel.GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) =>
			System.Threading.Tasks.Task.FromResult<UnityEngine.Sprite>(defaultPackCover);
		public ErrorBeatmapLevel(string levelId) =>
			this.levelId = levelId;
	}

	[Patch(PatchType.Postfix, typeof(BeatmapLevelsModel), nameof(BeatmapLevelsModel.GetLevelPreviewForLevelId))]
	public static void BeatmapLevelsModel_GetLevelPreviewForLevelId(ref IPreviewBeatmapLevel? __result, string levelId) =>
		__result ??= (IPreviewBeatmapLevel?)playerData.ResolvePreview(levelId) ?? new ErrorBeatmapLevel(levelId);

	// Failsafe that should've been in the base game; Fixes the player list deleting itself when SongCore isn't installed and a lawless diff is suggested
	[Patch(PatchType.Postfix, typeof(BeatmapIdentifierNetSerializableHelper), nameof(BeatmapIdentifierNetSerializableHelper.ToPreviewDifficultyBeatmap))]
	public static void BeatmapIdentifierNetSerializableHelper_ToPreviewDifficultyBeatmap(ref PreviewDifficultyBeatmap? __result) {
		if(__result != null)
			__result.beatmapCharacteristic ??= Resolve<BeatmapCharacteristicCollection>()!.GetBeatmapCharacteristicBySerializedName("Standard");
	}

	[Patch(PatchType.Prefix, typeof(LobbySetupViewController), nameof(LobbySetupViewController.SetPlayersMissingLevelText))]
	public static void LobbySetupViewController_SetPlayersMissingLevelText(LobbySetupViewController __instance, string playersMissingLevelText, ref UnityEngine.UI.Button ____startGameReadyButton) {
		if(!string.IsNullOrEmpty(playersMissingLevelText) && ____startGameReadyButton.interactable)
			__instance.SetStartGameEnabled(CannotStartGameReason.DoNotOwnSong);
	}

	[Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "AddPlayer")]
	public static void ConnectedPlayerManager_AddPlayer(ConnectedPlayerManager __instance, IConnectedPlayer? player) {
		playerData.Reset(PlayerIndex(player));
		if(__instance.connectedPlayerCount > 5 && !haveMpCore) // Certified Beat Games™ moment
			__instance.Disconnect((DisconnectedReason)101);
	}

	[Patch(PatchType.Postfix, typeof(DisconnectedReasonMethods), nameof(DisconnectedReasonMethods.LocalizedKey))]
	public static void DisconnectedReasonMethods_LocalizedKey(DisconnectedReason connectionFailedReason, ref string __result) {
		if((int)connectionFailedReason == 101)
			__result = "BEATUP_LARGE_LOBBY_AUTOKICK";
	}

	[Patch(PatchType.Prefix, typeof(MenuRpcManager), nameof(MenuRpcManager.GetIsReady))]
	public static void MenuRpcManager_GetIsReady() => // TODO: is this too late for the entitlement check?
		Net.Send(new ConnectInfo(LocalBlockSize).Wrap());

	// This callback is bound at the same time MenuRpcManager.GetIsReady() is called
	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.HandleMultiplayerSessionManagerPlayerConnected))]
	public static void LobbyPlayersDataModel_HandleMultiplayerSessionManagerPlayerConnected(IConnectedPlayer connectedPlayer) =>
		Net.SendToPlayer(new ConnectInfo(LocalBlockSize).Wrap(), connectedPlayer);

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.HandleMenuRpcManagerGetRecommendedBeatmap))]
	public static void LobbyPlayersDataModel_HandleMenuRpcManagerGetRecommendedBeatmap(LobbyPlayersDataModel __instance, string userId) {
		PreviewDifficultyBeatmap beatmapLevel = __instance[__instance.localUserId].beatmapLevel;
		RecommendPreview preview = playerData.previews[PlayerIndex(Resolve<MultiplayerSessionManager>()?.localPlayer)];
		if(!string.IsNullOrEmpty(HashForLevelID(beatmapLevel?.beatmapLevel?.levelID)))
			Net.Send(new MpBeatmapPacket(preview, beatmapLevel!)); // TODO: maybe avoid alloc?
		Net.Send(preview);
	}

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.SetLocalPlayerBeatmapLevel))]
	public static void LobbyPlayersDataModel_SetLocalPlayerBeatmapLevel(LobbyPlayersDataModel __instance, PreviewDifficultyBeatmap? beatmapLevel) {
		if(beatmapLevel != null) {
			RecommendPreview? preview = playerData.previews[PlayerIndex(Resolve<MultiplayerSessionManager>()?.localPlayer)];
			if(preview.levelID != beatmapLevel.beatmapLevel.levelID) {
				preview = playerData.ResolvePreview(beatmapLevel.beatmapLevel.levelID);
				if(beatmapLevel.beatmapLevel is CustomPreviewBeatmapLevel previewBeatmapLevel)
					preview ??= new RecommendPreview(previewBeatmapLevel);
				if(preview != null)
					playerData.previews[PlayerIndex(Resolve<MultiplayerSessionManager>()?.localPlayer)] = preview;
			}
			if(preview != null) {
				if(!string.IsNullOrEmpty(HashForLevelID(beatmapLevel.beatmapLevel.levelID)))
					Net.Send(new MpBeatmapPacket(preview, beatmapLevel)); // TODO: maybe avoid alloc?
				Net.Send(preview);
			}
		}
		lobbyDifficultyPanel.Update(beatmapLevel, __instance.SetLocalPlayerBeatmapLevel);
	}

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.ClearLocalPlayerBeatmapLevel))]
	public static void LobbyPlayersDataModel_ClearLocalPlayerBeatmapLevel() =>
		lobbyDifficultyPanel.Clear();
}
