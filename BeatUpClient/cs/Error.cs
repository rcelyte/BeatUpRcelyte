static class BeatUpClient_Error {
	static string header = string.Empty, message = string.Empty;
	static bool skip = false;
	[BeatUpClient.Patch(BeatUpClient.PatchType.Prefix, typeof(MainFlowCoordinator), nameof(MainFlowCoordinator.HandleMainMenuViewControllerDidFinish))]
	public static bool HandleMainMenuViewControllerDidFinish(MainFlowCoordinator __instance, MainMenuViewController viewController, MainMenuViewController.MenuButton subMenuType, SimpleDialogPromptViewController ____simpleDialogPromptViewController) {
		return (skip || subMenuType != MainMenuViewController.MenuButton.Multiplayer).Or(() => {
			____simpleDialogPromptViewController.Init(header, message, Polyglot.Localization.Get("BUTTON_OK"), buttonNumber => {
				skip = true;
				__instance.DismissViewController(____simpleDialogPromptViewController, HMUI.ViewController.AnimationDirection.Vertical, () =>
					__instance.HandleMainMenuViewControllerDidFinish(viewController, MainMenuViewController.MenuButton.Multiplayer));
			});
			__instance.PresentViewController(____simpleDialogPromptViewController);
		});
	}
	public static void Init(string header, string message) {
		(BeatUpClient_Error.header, BeatUpClient_Error.message) = (header, message);
		System.Action applyPatches = delegate {}, init = delegate {};
		BeatUpClient.GatherMethods(typeof(BeatUpClient_Error), ref applyPatches, ref init);
		applyPatches();
	}
}
