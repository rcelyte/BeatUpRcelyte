static partial class BeatUpClient {
	// TODO: is this still needed in any edge cases
	/*public class ErrorBeatmapLevel : EmptyBeatmapLevel, IPreviewBeatmapLevel {
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
	static void BeatmapLevelsModel_GetLevelPreviewForLevelId(ref IPreviewBeatmapLevel? __result, string levelId) =>
		__result ??= (IPreviewBeatmapLevel?)playerData.ResolvePreview(levelId) ?? new ErrorBeatmapLevel(levelId);*/

	// Failsafe that should've been in the base game; Fixes the player list deleting itself when SongCore isn't installed and a lawless diff is suggested
	[Detour(typeof(BeatmapIdentifierNetSerializableHelper), nameof(BeatmapIdentifierNetSerializableHelper.ToBeatmapKey))]
	static BeatmapKey BeatmapIdentifierNetSerializableHelper_ToBeatmapKey(BeatmapKeyNetSerializable beatmapKeySerializable, BeatmapCharacteristicCollection beatmapCharacteristicCollection) {
		BeatmapKey result = (BeatmapKey)Base(beatmapKeySerializable, beatmapCharacteristicCollection);
		result.beatmapCharacteristic ??= beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName("Standard");
		return result;
	}

	[Patch(PatchType.Prefix, typeof(LobbySetupViewController), nameof(LobbySetupViewController.SetPlayersMissingLevelText))]
	static void LobbySetupViewController_SetPlayersMissingLevelText(LobbySetupViewController __instance, string playersMissingLevelText, ref UnityEngine.UI.Button ____startGameReadyButton) {
		if(!string.IsNullOrEmpty(playersMissingLevelText) && ____startGameReadyButton.interactable)
			__instance.SetStartGameEnabled(CannotStartGameReason.DoNotOwnSong);
	}

	[Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "AddPlayer")]
	static void ConnectedPlayerManager_AddPlayer(ConnectedPlayerManager __instance, IConnectedPlayer? player) {
		playerData.Reset(PlayerIndex(player));
		if(__instance.connectedPlayerCount > 5 && !haveMpCore) // Certified Beat Games™ moment
			__instance.Disconnect((DisconnectedReason)101);
	}

	[Patch(PatchType.Postfix, typeof(DisconnectedReasonMethods), nameof(DisconnectedReasonMethods.LocalizedKey))]
	static void DisconnectedReasonMethods_LocalizedKey(DisconnectedReason connectionFailedReason, ref string __result) {
		if((int)connectionFailedReason == 101)
			__result = "BEATUP_LARGE_LOBBY_AUTOKICK";
	}

	[Patch(PatchType.Prefix, typeof(MenuRpcManager), nameof(MenuRpcManager.GetIsReady))]
	static void MenuRpcManager_GetIsReady() => // TODO: is this too late for the entitlement check?
		Net.Send(new ConnectInfo(LocalBlockSize).Wrap());

	// This callback is bound at the same time MenuRpcManager.GetIsReady() is called
	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.HandleMultiplayerSessionManagerPlayerConnected))]
	static void LobbyPlayersDataModel_HandleMultiplayerSessionManagerPlayerConnected(IConnectedPlayer connectedPlayer) =>
		Net.SendToPlayer(new ConnectInfo(LocalBlockSize).Wrap(), connectedPlayer);

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.HandleMenuRpcManagerGetRecommendedBeatmap))]
	static void LobbyPlayersDataModel_HandleMenuRpcManagerGetRecommendedBeatmap(LobbyPlayersDataModel __instance, string userId) {
		BeatmapKey key = __instance[__instance.localUserId].beatmapKey;
		BeatmapLevel? beatmapLevel = Resolve<BeatmapLevelsModel>()!.GetBeatmapLevelForLevelId(key.levelId);
		RecommendPreview preview = playerData.previews[PlayerIndex(Resolve<MultiplayerSessionManager>()?.localPlayer)];
		if(!string.IsNullOrEmpty(HashForLevelID(beatmapLevel?.levelID)))
			Net.Send(new MpBeatmapPacket(preview, beatmapLevel!, key)); // TODO: maybe avoid alloc?
		Net.Send(preview);
	}

	[Detour(typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.SetLocalPlayerBeatmapLevel))]
	static void LobbyPlayersDataModel_SetLocalPlayerBeatmapLevel(LobbyPlayersDataModel self, ref BeatmapKey beatmapKey) {
		BeatmapLevel? beatmapLevel = Resolve<BeatmapLevelsModel>()!.GetBeatmapLevelForLevelId(beatmapKey.levelId);
		if(beatmapKey.IsValid()) {
			RecommendPreview? preview = playerData.previews[PlayerIndex(Resolve<MultiplayerSessionManager>()?.localPlayer)];
			if(preview.levelID != beatmapKey.levelId) {
				preview = playerData.ResolvePreview(beatmapKey.levelId);
				if(beatmapLevel != null)
					preview ??= new RecommendPreview(beatmapLevel);
				if(preview != null)
					playerData.previews[PlayerIndex(Resolve<MultiplayerSessionManager>()?.localPlayer)] = preview;
			}
			if(preview != null) {
				if(beatmapLevel != null && !string.IsNullOrEmpty(HashForLevelID(beatmapKey.levelId)))
					Net.Send(new MpBeatmapPacket(preview, beatmapLevel, beatmapKey)); // TODO: maybe avoid alloc?
				Net.Send(preview);
			}
		}
		string levelId = beatmapKey.levelId;
		void OnSelect(BeatmapCharacteristicSO newCharacteristic, BeatmapDifficulty newDifficulty) {
			self.SetLocalPlayerBeatmapLevel(new(levelId, newCharacteristic, newDifficulty));
		}
		lobbyDifficultyPanel.Update(beatmapLevel, beatmapKey.beatmapCharacteristic, beatmapKey.difficulty, OnSelect);
		Base(self, PassRef(ref beatmapKey));
	}

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.ClearLocalPlayerBeatmapLevel))]
	static void LobbyPlayersDataModel_ClearLocalPlayerBeatmapLevel() =>
		lobbyDifficultyPanel.Clear();
}
