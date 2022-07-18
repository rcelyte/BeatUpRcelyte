static partial class BeatUpClient {
	class BeatUpScenesTransitionSetupDataSO : ScenesTransitionSetupDataSO {
		protected override void OnEnable() {
			SceneInfo si = CreateInstance<SceneInfo>();
			si._sceneName = "PCInit";
			Init(new SceneInfo[] {si}, System.Array.Empty<SceneSetupData>());
			base.OnEnable();
		}
	}

	static bool initialSceneRegistered = false;
	[Patch.Overload(PatchType.Prefix, typeof(Zenject.Context), "InstallInstallers", new[] {typeof(System.Collections.Generic.List<Zenject.InstallerBase>), typeof(System.Collections.Generic.List<System.Type>), typeof(System.Collections.Generic.List<Zenject.ScriptableObjectInstaller>), typeof(System.Collections.Generic.List<Zenject.MonoInstaller>), typeof(System.Collections.Generic.List<Zenject.MonoInstaller>)})]
	public static void Context_InstallInstallers(Zenject.Context __instance) =>
		initialSceneRegistered |= (__instance.name == "AppCoreSceneContext");

	static BeatUpScenesTransitionSetupDataSO? restartTransitionData = null;
	[Patch(PatchType.Prefix, typeof(GameScenesManager), nameof(GameScenesManager.ReplaceScenes))] // Fallback if SiraUtil isn't installed
	public static bool GameScenesManager_ReplaceScenes(GameScenesManager __instance, ScenesTransitionSetupDataSO scenesTransitionSetupData) {
		return (haveSiraUtil || initialSceneRegistered).Or(() => {
			restartTransitionData ??= UnityEngine.ScriptableObject.CreateInstance<BeatUpScenesTransitionSetupDataSO>();
			__instance.ClearAndOpenScenes(restartTransitionData, finishCallback: container =>
				UnityEngine.SceneManagement.SceneManager.GetSceneByName(__instance.GetCurrentlyLoadedSceneNames()[0]).GetRootGameObjects()[0].GetComponent<PCAppInit>().TransitionToNextScene());
		});
	}
}
