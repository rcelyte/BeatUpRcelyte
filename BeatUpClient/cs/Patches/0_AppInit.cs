static partial class BeatUpClient {
	class BeatUpScenesTransitionSetupDataSO : ScenesTransitionSetupDataSO {
		protected override void OnEnable() {
			Init(new string[] {"GameInit"}, System.Array.Empty<SceneSetupData>());
			base.OnEnable();
		}
	}

	static bool initialSceneRegistered = false;
	[Detour(typeof(Zenject.Context), nameof(Zenject.Context.InstallInstallers))]
	static void Context_InstallInstallers(Zenject.Context self, System.Collections.Generic.List<Zenject.InstallerBase> normalInstallers, System.Collections.Generic.List<System.Type> normalInstallerTypes, System.Collections.Generic.List<Zenject.ScriptableObjectInstaller> scriptableObjectInstallers, System.Collections.Generic.List<Zenject.MonoInstaller> installers, System.Collections.Generic.List<Zenject.MonoInstaller> installerPrefabs) {
		initialSceneRegistered |= (self.name == "AppCoreSceneContext");
		Base(self, normalInstallers, normalInstallerTypes, scriptableObjectInstallers, installers, installerPrefabs);
	}

	static BeatUpScenesTransitionSetupDataSO? restartTransitionData = null;
	[Detour(typeof(GameScenesManager), nameof(GameScenesManager.ReplaceScenes))]
	static void GameScenesManager_ReplaceScenes(GameScenesManager self, ScenesTransitionSetupDataSO scenesTransitionSetupData, System.Collections.IEnumerator[] beforeNewScenesActivateRoutines, float minDuration, System.Action afterMinDurationCallback, System.Action<Zenject.DiContainer> finishCallback) {
		if(haveSiraUtil || initialSceneRegistered) {
			Base(self, scenesTransitionSetupData, beforeNewScenesActivateRoutines, minDuration, afterMinDurationCallback, finishCallback);
		} else { // Fallback if SiraUtil isn't installed
			restartTransitionData ??= UnityEngine.ScriptableObject.CreateInstance<BeatUpScenesTransitionSetupDataSO>();
			self.ClearAndOpenScenes(restartTransitionData, finishCallback: container =>
				UnityEngine.SceneManagement.SceneManager.GetSceneByName(self.GetCurrentlyLoadedSceneNames()[0]).GetRootGameObjects()[0].GetComponent<BeatSaberInit>().TransitionToNextSceneAsync());
		}
	}
}
