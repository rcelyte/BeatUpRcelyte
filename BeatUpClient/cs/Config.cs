public class BeatUpClient_Config { // global to avoid `MethodAccessException` in BSIPA
	public static BeatUpClient_Config Instance = new BeatUpClient_Config();
	public static System.Action? onReload = null;
	public virtual float CountdownDuration {get; set;} = 5;
	public virtual bool SkipResults {get; set;} = false;
	public virtual bool PerPlayerDifficulty {get; set;} = false;
	public virtual bool PerPlayerModifiers {get; set;} = false;
	public virtual bool HideOtherLevels {get; set;} = false;
	public virtual bool DirectDownloads {get; set;} = false;
	[IPA.Config.Stores.Attributes.NonNullable]
	[IPA.Config.Stores.Attributes.UseConverter(typeof(IPA.Config.Stores.Converters.DictionaryConverter<string>))]
	public System.Collections.Generic.Dictionary<string, string?> Servers {get; set;} = new System.Collections.Generic.Dictionary<string, string?> {
		["master.battletrains.org"] = null,
		["master.beattogether.systems"] = "http://master.beattogether.systems/status",
	};
	public virtual void Changed() {}
	public virtual void OnReload() =>
		onReload?.Invoke();
}
