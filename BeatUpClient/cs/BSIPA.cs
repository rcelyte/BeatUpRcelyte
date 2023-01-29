[IPA.Plugin(IPA.RuntimeOptions.SingleStartInit)]
public class BeatUpClient_BSIPA {
	public static BeatUpClient_BSIPA? Instance;
	readonly Hive.Versioning.Version version;

	struct LogWrapper : BeatUpClient.ILogger {
		public IPA.Logging.Logger logger;
		public void Debug(string message) =>
			logger.Debug(message);
		public void Info(string message) =>
			logger.Info(message);
		public void Warn(string message) =>
			logger.Warn(message);
		public void Error(string message) =>
			logger.Error(message);
		public void Critical(string message) =>
			logger.Critical(message);
	}

	internal static Hive.Versioning.Version? GetVersion(string id) =>
		IPA.Loader.PluginManager.GetPluginFromId(id)?.HVersion;

	[IPA.Init]
	public BeatUpClient_BSIPA(IPA.Logging.Logger pluginLogger, IPA.Loader.PluginMetadata metadata, IPA.Config.Config conf) {
		Instance = this;
		BeatUpClient.Log = new LogWrapper {logger = pluginLogger};
		version = metadata.HVersion;
		BeatUpClient_Config.Instance = IPA.Config.Stores.GeneratedStore.Generated<BeatUpClient_Config>(conf);
		BeatUpClient.Log.Debug("Logger initialized.");
	}

	[IPA.OnEnable]
	public void OnEnable() =>
		BeatUpClient.Enable(version, GetVersion);

	[IPA.OnDisable]
	public void OnDisable() =>
		BeatUpClient.Disable();
}
