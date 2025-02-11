using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static bool SupportedMpCoreVersion(string? v) =>
		v == null || v == "1.5.3" || v == "1.5.4" || v == "1.6.0";

	[System.AttributeUsage(System.AttributeTargets.Method)]
	internal class InitAttribute : System.Attribute {}

	internal static System.Action GatherMethods(System.Type type, ref int patchCount) {
		// Log.Debug($"GatherMethods({type})");
		System.Action applyMethods = delegate {};
		foreach(System.Type nested in type.GetNestedTypes(HarmonyLib.AccessTools.all))
			applyMethods += GatherMethods(nested, ref patchCount);
		if(type.IsInterface)
			return applyMethods;
		foreach(System.Reflection.MethodInfo method in type.GetMethods(System.Reflection.BindingFlags.DeclaredOnly | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Static)) {
			if(method.IsAbstract)
				continue;
			if(!method.ContainsGenericParameters) {
				try {
					method.MethodHandle.GetFunctionPointer(); // Force JIT compilation
				} catch(System.Exception ex) {
					Log.Error($"Failed to preload {method.DeclaringType.FullName} :: {method}");
					Log.Exception(ex);
					patchCount = int.MinValue;
					continue;
				}
			}
			foreach(IPatch patch in method.GetCustomAttributes(typeof(IPatch), false)) {
				// Log.Debug($"    {method}");
				try {
					applyMethods += patch.Bind(method);
				} catch(System.Exception ex) {
					Log.Error($"Failed to load patch {method}");
					Log.Exception(ex);
					patchCount = int.MinValue;
					continue;
				}
				++patchCount;
			}
			if(method.GetCustomAttributes(typeof(InitAttribute), false).Length != 0)
				((System.Action)System.Delegate.CreateDelegate(typeof(System.Action), method))?.Invoke(); // TODO: temp code; inits need to be delayed until just before applying patches
		}
		return applyMethods;
	}

	static void OnSceneLoaded(UnityEngine.SceneManagement.Scene scene, UnityEngine.SceneManagement.LoadSceneMode mode) {
		if(scene.name != "MainMenu")
			return;
		Log.Debug("load MainMenu");
		UnityEngine.Texture2D LoadTexture(string name) { // TODO: get size from file data
			System.IO.Stream resource = System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream(name);
			byte[]? data;
			using(System.IO.BinaryReader reader = new System.IO.BinaryReader(resource)) {
				data = reader.ReadBytes((int)resource.Length);
			}
			UnityEngine.Texture2D texture = new UnityEngine.Texture2D(2, 2);
			UnityEngine.ImageConversion.LoadImage(texture, data);
			return texture;
		}
		UnityEngine.Texture2D coverTexture = LoadTexture("BeatUpClient.cover");
		defaultPackCover = UnityEngine.Sprite.Create(coverTexture, new(0, 0, coverTexture.width, coverTexture.height), new(0, 0), 0.1f);
		spriteSwap = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerModeSelectionViewController>()[0]
			.transform.Find("Buttons/CreateServerButton")?.GetComponent<HMUI.ButtonSpriteSwap>();
		if(spriteSwap != null) {
			createButtonSprites = (spriteSwap._normalStateSprite, spriteSwap._highlightStateSprite,
				spriteSwap._pressedStateSprite, spriteSwap._disabledStateSprite);
			UnityEngine.Texture2D createTexture = LoadTexture("BeatUpClient.create");
			heartButtonSprites.pressed = heartButtonSprites.highlight = UnityEngine.Sprite.Create(createTexture,
				new(0, 0, createTexture.width, createTexture.height / 2), new(0, 0), 0.1f);
			heartButtonSprites.disabled = heartButtonSprites.normal = UnityEngine.Sprite.Create(createTexture,
				new(0, createTexture.height / 2, createTexture.width, createTexture.height / 2), new(0, 0), 0.1f);
		}

		SelectorSetup();
		LobbyUISetup();
	}

	internal static void Enable(string version, System.Func<string, string?> getModVersion) {
		modVersion = version;
		uint protocolVersion = (uint)HarmonyLib.AccessTools.Field(typeof(NetworkConstants), nameof(NetworkConstants.kProtocolVersion)).GetValue(null);
		if(protocolVersion != 9u) {
			BeatUpClient_Error.Init("Incompatible BeatUpClient Version", $"This version of BeatUpClient requires a{((protocolVersion < 9u) ? " newer" : "n older")} version of Beat Saber.");
			return;
		}
		string? mpCoreVersion = getModVersion("MultiplayerCore");
		#if MPCORE_SUPPORT
		if(!SupportedMpCoreVersion(mpCoreVersion)) {
			BeatUpClient_Error.Init("Incompatible BeatUpClient Version", "This version of BeatUpClient only supports MultiplayerCore 1.5.3");
			return;
		}
		#else
		if(mpCoreVersion != null) {
			BeatUpClient_Error.Init("Incompatible BeatUpClient Version", "This version of BeatUpClient is incompatible with MultiplayerCore");
			return;
		}
		#endif
		string? err = null;
		try {
			err = BeatUpClient_Beta.CheckVersion(version);
		} catch(System.Exception ex) {
			Log.Exception(ex);
		}
		if(err != null) {
			if(err.Length != 0)
				BeatUpClient_Error.Init("Unsupported BeatUpClient Version", err);
			else
				BeatUpClient_Error.Init("BeatUpClient Validation Error", "BeatUpClient encountered a critical error. Please message @rcelyte on Discord.");
			return;
		}

		haveSiraUtil = getModVersion("SiraUtil") != null;
		haveSongCore = getModVersion("SongCore") != null;
		haveMpCore = getModVersion("MultiplayerCore") != null;
		haveMpEx = getModVersion("MultiplayerExtensions") != null;
		Log.Debug($"haveSiraUtil={haveSiraUtil}");
		Log.Debug($"haveSongCore={haveSongCore}");
		Log.Debug($"haveMpCore={haveMpCore}");
		Log.Debug($"haveMpEx={haveMpEx}");

		try {
			Log.Debug("Gathering patches");
			int patchCount = 0;
			System.Action applyPatches = new (System.Type type, bool enable)[] {
				(typeof(BeatUpClient), true),
				(typeof(BeatUpClient_SongCore), haveSongCore),
				#if MPCORE_SUPPORT
				(typeof(BeatUpClient_MpCore), haveMpCore),
				#endif
			}.Aggregate((System.Action)delegate {}, (acc, section) => {
				if(section.enable)
					acc += GatherMethods(section.type, ref patchCount);
				return acc;
			});
			if(patchCount >= 0) {
				Log.Debug($"Applying {patchCount} patches");
				applyPatches();
				if(GameLiftRequired)
					BeatUpClient_Migration.Init();
				UnityEngine.SceneManagement.SceneManager.sceneLoaded += OnSceneLoaded;
				return;
			}
		} catch(System.Exception ex) {
			Log.Error($"Error applying patches: {ex}");
		}
		Disable();
	}

	internal static void Disable() {
		try {
			UnityEngine.SceneManagement.SceneManager.sceneLoaded -= OnSceneLoaded;
			harmony.UnpatchSelf();
			DetourAttribute.UnpatchAll();
		} catch(System.Exception ex) {
			Log.Error($"Error removing patches: {ex}");
		}
	}

	private static void NativeEnable(string version) =>
		Enable(version, name => null);
	private static void NativeEnable_BSIPA(string version) =>
		Enable(version, BeatUpClient_BSIPA.GetVersion);
}
