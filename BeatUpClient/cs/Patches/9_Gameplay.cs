static partial class BeatUpClient {
	[Patch(PatchType.Postfix, typeof(MultiplayerLocalActivePlayerInGameMenuViewController), nameof(MultiplayerLocalActivePlayerInGameMenuViewController.Start))]
	public static void MultiplayerLocalActivePlayerInGameMenuViewController_Start(MultiplayerLocalActivePlayerInGameMenuViewController __instance) {
		MultiplayerPlayersManager multiplayerPlayersManager = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerPlayersManager>()[0];
		if(connectInfo?.perPlayerDifficulty == true && multiplayerPlayersManager.localPlayerStartSeekSongController is MultiplayerLocalActivePlayerFacade) {
			MenuTransitionsHelper menuTransitionsHelper = UnityEngine.Resources.FindObjectsOfTypeAll<MenuTransitionsHelper>()[0];
			MultiplayerConnectedPlayerSongTimeSyncController audioTimeSyncController = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerConnectedPlayerSongTimeSyncController>()[0];
			PreviewDifficultyBeatmap original = new PreviewDifficultyBeatmap(__instance._localPlayerInGameMenuInitData.previewBeatmapLevel, __instance._localPlayerInGameMenuInitData.beatmapCharacteristic, __instance._localPlayerInGameMenuInitData.beatmapDifficulty);
			PreviewDifficultyBeatmap selectedPreview = original;
			IDifficultyBeatmap? selectedBeatmap = null;
			UnityEngine.RectTransform switchButton = UI.CreateButtonFrom(__instance._resumeButton.gameObject, __instance._resumeButton.transform.parent, "SwitchDifficulty", () => {
				MultiplayerLevelScenesTransitionSetupDataSO setupData = menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData;
				setupData.Init(setupData.gameMode, selectedPreview.beatmapLevel, selectedPreview.beatmapDifficulty, selectedPreview.beatmapCharacteristic, selectedBeatmap, setupData.colorScheme, setupData.gameplayCoreSceneSetupData.gameplayModifiers, setupData.gameplayCoreSceneSetupData.playerSpecificSettings, setupData.gameplayCoreSceneSetupData.practiceSettings, setupData.gameplayCoreSceneSetupData.useTestNoteCutSoundEffects);
				menuTransitionsHelper._gameScenesManager.ReplaceScenes(menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData, null, .35f, null, container => {
					MultiplayerController multiplayerController = container.Resolve<MultiplayerController>();
					multiplayerController._songStartSyncController.syncStartSuccessEvent -= OnSongStart;
					multiplayerController._songStartSyncController.syncStartSuccessEvent += OnSongStart;
					void OnSongStart(float introAnimationStartSyncTime) {
						multiplayerController._songStartSyncController.syncStartSuccessEvent -= OnSongStart;
						multiplayerController._playersManager.activeLocalPlayerFacade._gameSongController._beatmapCallbacksController._startFilterTime = multiplayerController.GetCurrentSongTime(multiplayerController.GetSongStartSyncTime(introAnimationStartSyncTime)) * menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.gameplayCoreSceneSetupData.gameplayModifiers.songSpeedMul + 1;
					}
				});
			});
			switchButton.GetComponentInChildren<Polyglot.LocalizedTextMeshProUGUI>().Key = "BEATUP_SWITCH";
			switchButton.gameObject.SetActive(false);
			DifficultyPanel panel = new DifficultyPanel(__instance._mainBar.transform, 1, -2, __instance._levelBar.transform.Find("BG").GetComponent<HMUI.ImageView>(), true);
			__instance._levelBar.transform.localPosition = new UnityEngine.Vector3(0, 13.25f, 0);
			panel.beatmapCharacteristic.localPosition = new UnityEngine.Vector3(-1, -1.5f, 0);
			panel.beatmapDifficulty.localPosition = new UnityEngine.Vector3(-1, -8.25f, 0);
			void OnSelect(PreviewDifficultyBeatmap preview) {
				selectedPreview = preview;
				selectedBeatmap = (preview == original) ? null : BeatmapLevelDataExtensions.GetDifficultyBeatmap(menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.difficultyBeatmap.level.beatmapLevelData, preview);
				__instance._resumeButton.gameObject.SetActive(selectedBeatmap == null);
				switchButton.gameObject.SetActive(selectedBeatmap != null);
				panel.Update(preview, OnSelect);
			}
			panel.Update(original, OnSelect);
		}
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerConnectedPlayerInstaller), nameof(MultiplayerConnectedPlayerInstaller.InstallBindings))]
	public static void MultiplayerConnectedPlayerInstaller_InstallBindings(GameplayCoreSceneSetupData ____sceneSetupData, IConnectedPlayer ____connectedPlayer) {
		bool zenMode = ____sceneSetupData.gameplayModifiers.zenMode || (BeatUpClient_Config.Instance.HideOtherLevels && !haveMpEx);
		if(connectInfo?.perPlayerModifiers == true && PlayerIndex(____connectedPlayer) < 128) {
			PlayerData.ModifiersWeCareAbout mods = playerData.lockedModifiers[PlayerIndex(____connectedPlayer)];
			____sceneSetupData.gameplayModifiers = ____sceneSetupData.gameplayModifiers.CopyWith(disappearingArrows: mods.disappearingArrows, ghostNotes: mods.ghostNotes, zenMode: zenMode, smallCubes: mods.smallCubes);
		} else {
			____sceneSetupData.gameplayModifiers = ____sceneSetupData.gameplayModifiers.CopyWith(zenMode: zenMode);
		}
	}

	// Hopefully we can get these patches into MultiplayerCore soon for large (~16 or more player) lobbies
	/*[Patch.Generic(PatchType.Prefix, typeof(MultiplayerSessionManager), "Send", typeof(LiteNetLib.Utils.INetSerializable))]
	public static bool MultiplayerSessionManager_Send(MultiplayerSessionManager __instance, LiteNetLib.Utils.INetSerializable message) =>
		(!(message is NodePoseSyncStateNetSerializable || message is StandardScoreSyncStateNetSerializable)).Or(() => __instance.SendUnreliable(message));

	[Patch(PatchType.Postfix, typeof(NodePoseSyncStateManager), "get_deltaUpdateFrequency")]
	[Patch(PatchType.Postfix, typeof(ScoreSyncStateManager), "get_deltaUpdateFrequency")]
	public static void SyncStateManager_get_deltaUpdateFrequency(ref float __result) =>
		__result = float.MaxValue;

	[Patch(PatchType.Postfix, typeof(NodePoseSyncStateManager), "get_fullStateUpdateFrequency")]
	public static void NodePoseSyncStateManager_get_fullStateUpdateFrequency(ref float __result) =>
		__result = 0.01f;

	[Patch(PatchType.Postfix, typeof(ScoreSyncStateManager), "get_fullStateUpdateFrequency")]
	public static void ScoreSyncStateManager_get_fullStateUpdateFrequency(ref float __result) =>
		__result = 0.02f;*/

	[Patch(PatchType.Prefix, typeof(MultiplayerOutroAnimationController), nameof(MultiplayerOutroAnimationController.AnimateOutro))]
	public static bool MultiplayerOutroAnimationController_AnimateOutro(System.Action onCompleted) =>
		(connectInfo?.skipResults != true).Or(() => onCompleted.Invoke());
}
