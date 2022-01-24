namespace CertBypass {
	[HarmonyLib.HarmonyPatch(typeof(MainSettingsModelSO), "Load")]
	internal class MainSettingsModelSO_Load {
		internal static void Prefix(MainSettingsModelSO __instance) {
			Plugin.mainSettingsModel = __instance;
			Plugin.Log?.Debug($"useCustomServerEnvironment: {(bool)__instance.useCustomServerEnvironment}");
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(ClientCertificateValidator), "ValidateCertificateChainInternal")]
	internal class ClientCertificateValidator_ValidateCertificateChainInternal {
		internal static bool Prefix() {
			return !(bool)Plugin.mainSettingsModel?.useCustomServerEnvironment;
		}
	}

	[IPA.Plugin(IPA.RuntimeOptions.DynamicInit)]
	public class Plugin {
		public const string HarmonyId = "org.battletrains.CertBypass";
		public static readonly HarmonyLib.Harmony harmony = new HarmonyLib.Harmony(HarmonyId);
		public static Plugin Instance;
		public static IPA.Logging.Logger Log;
		public static MainSettingsModelSO mainSettingsModel = null;

		[IPA.Init]
		public void Init(IPA.Logging.Logger pluginLogger, IPA.Config.Config conf) {
			Instance = this;
			Plugin.Log = pluginLogger;
			Plugin.Log?.Debug("Logger initialized.");
		}
		[IPA.OnEnable]
		public void OnEnable() {
			try {
				Plugin.Log?.Debug("Applying Harmony patches.");
				harmony.PatchAll(System.Reflection.Assembly.GetExecutingAssembly());
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Error applying Harmony patches: " + ex.Message);
				Plugin.Log?.Debug(ex);
			}
		}
		[IPA.OnDisable]
		public void OnDisable() {
			try {
				harmony.UnpatchSelf();
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Error removing Harmony patches: " + ex.Message);
				Plugin.Log?.Debug(ex);
			}
			mainSettingsModel = null;
		}
	}
}
