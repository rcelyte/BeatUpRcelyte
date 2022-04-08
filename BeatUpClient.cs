namespace BeatUpClient {
	public class DiJack {
		[System.AttributeUsage(System.AttributeTargets.Class)]
		public class ReplaceAttribute : System.Attribute {
			public System.Type original;
			public ReplaceAttribute(System.Type original) {
				this.original = original;
			}
		}

		[System.AttributeUsage(System.AttributeTargets.Class)]
		public class ReplaceInterfacesAndSelfAttribute : System.Attribute {
			public System.Type original;
			public ReplaceInterfacesAndSelfAttribute(System.Type original) {
				this.original = original;
			}
		}

		public static System.Collections.Generic.Dictionary<System.Type, System.Type> InjectMap = new System.Collections.Generic.Dictionary<System.Type, System.Type>();
		public static void ConcreteBinderNonGeneric_To(Zenject.ConcreteBinderNonGeneric __instance, ref System.Collections.Generic.IEnumerable<System.Type> concreteTypes) {
			System.Type[] newTypes = System.Linq.Enumerable.ToArray(concreteTypes);
			uint i = 0;
			foreach(System.Type type in concreteTypes) {
				if(InjectMap.TryGetValue(type, out System.Type inject)) {
					Plugin.Log?.Debug($"Replacing {type} with {inject}");
					newTypes[i] = inject;
					__instance.BindInfo.ContractTypes.Add(inject);
				}
				++i;
			}
			concreteTypes = newTypes;
		}

		public static System.Collections.Generic.Dictionary<System.Type, System.Type> InterfacesAndSelf_InjectMap = new System.Collections.Generic.Dictionary<System.Type, System.Type>();
		public static bool DiContainer_BindInterfacesAndSelfTo(Zenject.DiContainer __instance, ref Zenject.FromBinderNonGeneric __result, System.Type type) {
			if(InterfacesAndSelf_InjectMap.TryGetValue(type, out System.Type inject)) {
				Plugin.Log?.Debug($"Replacing {type} with {inject}");
				System.Collections.Generic.List<System.Type> types = new System.Collections.Generic.List<System.Type>(ModestTree.TypeExtensions.Interfaces(type));
				types.Add(type);
				types.Add(inject);
				__result = __instance.Bind(types).To(inject);
				return false;
			}
			return true;
		}

		static void Patch<Orig, Patch>(string fn, string patch, params System.Type[] args) {
			System.Reflection.MethodBase original = typeof(Orig).GetMethod(fn, args);
			System.Reflection.MethodInfo prefix = typeof(Patch).GetMethod(patch);
			Plugin.harmony.Patch(original, prefix: new HarmonyLib.HarmonyMethod(prefix));
		}

		public static void ResolveInjections(System.Reflection.Assembly assembly) {
			foreach(System.Type type in assembly.GetTypes()) {
				foreach(System.Attribute attrib in type.GetCustomAttributes(false)) {
					if(attrib is ReplaceAttribute replaceAttribute)
						InjectMap.Add(replaceAttribute.original, type);
					if(attrib is ReplaceInterfacesAndSelfAttribute replaceInterfacesAndSelfAttribute)
						InterfacesAndSelf_InjectMap.Add(replaceInterfacesAndSelfAttribute.original, type);
				}
			}
			Patch<Zenject.ConcreteBinderNonGeneric, DiJack>(nameof(Zenject.ConcreteBinderNonGeneric.To), nameof(ConcreteBinderNonGeneric_To), typeof(System.Collections.Generic.IEnumerable<System.Type>));
			Patch<Zenject.DiContainer, DiJack>(nameof(Zenject.DiContainer.BindInterfacesAndSelfTo), nameof(DiContainer_BindInterfacesAndSelfTo), typeof(System.Type));
		}
	}
}

namespace BeatUpClient.Patches {
	[HarmonyLib.HarmonyPatch(typeof(MainSettingsModelSO), nameof(MainSettingsModelSO.Load))]
	public class MainSettingsModelSO_Load {
		public static void Postfix(ref MainSettingsModelSO __instance) {
			if(__instance.customServerHostName.value.Length != 0)
				return;
			__instance.useCustomServerEnvironment.value = true;
			__instance.forceGameLiftServerEnvironment.value = false;
			__instance.customServerHostName.value = "battletrains.org";
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(PlatformAuthenticationTokenProvider), nameof(PlatformAuthenticationTokenProvider.GetAuthenticationToken))]
	public class PlatformAuthenticationTokenProvider_GetAuthenticationToken { // fix for offline mode
		public static async System.Threading.Tasks.Task<AuthenticationToken> AuthWrapper(System.Threading.Tasks.Task<AuthenticationToken> task, AuthenticationToken.Platform platform, string userId, string userName) {
			try {
				return await task;
			} catch(System.Security.Authentication.AuthenticationException) {
				return new AuthenticationToken(platform, userId, userName, "");
			}
		}
		public static void Postfix(ref System.Threading.Tasks.Task<AuthenticationToken> __result, AuthenticationToken.Platform ____platform, string ____userId, string ____userName) {
			if(Plugin.networkConfig is CustomNetworkConfig)
				__result = AuthWrapper(__result, ____platform, ____userId, ____userName);
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(ClientCertificateValidator), "ValidateCertificateChainInternal")]
	public class ClientCertificateValidator_ValidateCertificateChainInternal {
		public static bool Prefix() =>
			!(Plugin.networkConfig is CustomNetworkConfig);
	}

	[HarmonyLib.HarmonyPatch(typeof(MultiplayerLevelSelectionFlowCoordinator), "enableCustomLevels", HarmonyLib.MethodType.Getter)]
	public class MultiplayerLevelSelectionFlowCoordinator_enableCustomLevels {
		public static void Postfix(ref bool __result) {
			__result |= (Plugin.networkConfig is CustomNetworkConfig);
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(LobbySetupViewController), nameof(LobbySetupViewController.SetPlayersMissingLevelText))]
	public class LobbySetupViewController_SetPlayersMissingLevelText {
		public static void Prefix(LobbySetupViewController __instance, string playersMissingLevelText, ref UnityEngine.UI.Button ____startGameReadyButton) {
			if(!string.IsNullOrEmpty(playersMissingLevelText) && ____startGameReadyButton.interactable)
				__instance.SetStartGameEnabled(CannotStartGameReason.DoNotOwnSong);
		}
	}

	[HarmonyLib.HarmonyPatch]
	public static class MasterServerConnectionManager_HandleConnectToServerSuccess {
		static System.Collections.Generic.IEnumerable<System.Reflection.MethodBase> TargetMethods() {
			yield return typeof(MasterServerConnectionManager).GetMethod("HandleConnectToServerSuccess", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			yield return typeof(GameLiftConnectionManager).GetMethod("HandleConnectToServerSuccess", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
		}
		public static void Prefix(GameplayServerConfiguration configuration) {
			Plugin.playerPreviews = new Networking.PacketHandler.RecommendPreview[configuration.maxPlayerCount];
			Plugin.playerCells = new Plugin.PlayerCell[configuration.maxPlayerCount];
			Plugin.UpdateDifficultyUI(null);
		}
	}
	[HarmonyLib.HarmonyPatch(typeof(MultiplayerSessionManager), "HandlePlayerOrderChanged")]
	public class MultiplayerSessionManager_HandlePlayerOrderChanged {
		public static void Prefix(IConnectedPlayer player) {
			Plugin.playerCells[player.sortIndex].SetData(new Networking.PacketHandler.LoadProgress(Networking.PacketHandler.LoadProgress.LoadState.None, 0, 0));
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(BasicConnectionRequestHandler), nameof(BasicConnectionRequestHandler.GetConnectionMessage))]
	public class BasicConnectionRequestHandler_GetConnectionMessage {
		public static void Postfix(LiteNetLib.Utils.NetDataWriter writer) {
			Plugin.Log?.Debug("BasicConnectionRequestHandler_GetConnectionMessage()");
			LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, 16);
			sub.Put((uint)1);
			sub.Put((uint)Config.Instance.WindowSize);
			sub.Put((byte)(Config.Instance.CountdownDuration * 4));
			byte bits = 0;
			bits |= Config.Instance.DirectDownloads ? (byte)1 : (byte)0;
			bits |= Config.Instance.SkipResults ? (byte)2 : (byte)0;
			bits |= Config.Instance.PerPlayerDifficulty ? (byte)4 : (byte)0;
			bits |= Config.Instance.PerPlayerModifiers ? (byte)8 : (byte)0;
			sub.Put((byte)bits);
			writer.PutVarUInt((uint)sub.Length);
			writer.Put("BeatUpClient");
			writer.Put(sub.CopyData());
			Plugin.windowSize = 64;
			Plugin.directDownloads = false;
			Plugin.skipResults = false;
		}
	}

	[HarmonyLib.HarmonyPatch]
	public class NetConnectAcceptPacket_FromData {
		public static int Size;
		public static System.Reflection.FieldInfo fi_RawData = null!;
		public static System.Reflection.FieldInfo fi_Size = null!;
		public static System.Reflection.MethodBase TargetMethod() {
			System.Type NetPacket = typeof(LiteNetLib.NetManager).Assembly.GetType("LiteNetLib.NetPacket");
			fi_RawData = NetPacket.GetField("RawData", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Public);
			fi_Size = NetPacket.GetField("Size", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Public);
			System.Type NetConnectAcceptPacket = typeof(LiteNetLib.NetManager).Assembly.GetType("LiteNetLib.NetConnectAcceptPacket");
			Size = (int)NetConnectAcceptPacket.GetField("Size", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.Public).GetRawConstantValue();
			return NetConnectAcceptPacket.GetMethod("FromData");
		}
		public static void Prefix(ref object packet) {
			int size = (int)fi_Size.GetValue(packet);
			if(size == Size + 5) {
				Plugin.skipResults = System.BitConverter.ToBoolean((byte[])fi_RawData.GetValue(packet), size -= 1);
				uint windowSize = System.BitConverter.ToUInt32((byte[])fi_RawData.GetValue(packet), size -= 4);
				if(windowSize < 32 || windowSize > 512)
					return;
				Plugin.windowSize = windowSize;
				Plugin.directDownloads = Config.Instance.DirectDownloads;
				Plugin.Log?.Info($"Overriding window size - {windowSize}");
				fi_Size.SetValue(packet, size);
			}
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(BeatmapLevelsModel), nameof(BeatmapLevelsModel.GetLevelPreviewForLevelId))]
	public class BeatmapLevelsModel_GetLevelPreviewForLevelId {
		public class ErrorPreviewBeatmapLevel : IPreviewBeatmapLevel {
			public string levelID { get; set; }
			public string songName { get; set; } = "[ERROR]";
			public string songSubName { get; set; } = System.String.Empty;
			public string songAuthorName { get; set; } = "[ERROR]";
			public string levelAuthorName { get; set; } = System.String.Empty;
			public float beatsPerMinute { get; set; }
			public float songTimeOffset { get; set; }
			public float shuffle { get; set; }
			public float shufflePeriod { get; set; }
			public float previewStartTime { get; set; }
			public float previewDuration { get; set; }
			public float songDuration { get; set; }
			public System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet> previewDifficultyBeatmapSets { get; set; } = null!;
			public EnvironmentInfoSO environmentInfo { get; set; } = null!;
			public EnvironmentInfoSO allDirectionsEnvironmentInfo { get; set; } = null!;
			public System.Threading.Tasks.Task<UnityEngine.Sprite> GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) =>
				System.Threading.Tasks.Task.FromResult<UnityEngine.Sprite>(Plugin.defaultPackCover);
			public ErrorPreviewBeatmapLevel(string levelId) {
				levelID = levelId;
			}
		}
		public static System.Func<string, Networking.PacketHandler.RecommendPreview?>? resolvePreview = null;
		public static void Postfix(ref IPreviewBeatmapLevel? __result, string levelId) {
			if(__result == null) {
				__result = (IPreviewBeatmapLevel?)resolvePreview?.Invoke(levelId)?.preview ?? new ErrorPreviewBeatmapLevel(levelId);
				Plugin.Log?.Debug("Overriding GetLevelPreviewForLevelId()");
			} else {
				Plugin.Log?.Debug("Fail GetLevelPreviewForLevelId()");
			}
			Plugin.Log?.Debug($"    result={__result}");
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(NetworkPlayerEntitlementChecker), "GetEntitlementStatus")]
	public class NetworkPlayerEntitlementChecker_GetEntitlementStatus {
		public static async System.Threading.Tasks.Task ZipFile(System.IO.Compression.ZipArchive archive, string path, string name) {
			Plugin.Log?.Debug($"Zipping `{path}`");
			using System.IO.Stream file = System.IO.File.Open(path, System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.Read);
			using System.IO.Stream entry = archive.CreateEntry(name).Open();
			await file.CopyToAsync(entry);
		}
		public static async System.Threading.Tasks.Task<byte[]> ZipLevel(CustomBeatmapLevel level) {
			using(System.IO.MemoryStream memoryStream = new System.IO.MemoryStream()) {
				using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(memoryStream, System.IO.Compression.ZipArchiveMode.Create, true)) {
					await ZipFile(archive, level.songAudioClipPath, level.standardLevelInfoSaveData.songFilename);
					await ZipFile(archive, System.IO.Path.Combine(level.customLevelPath, "Info.dat"), "Info.dat");
					for(int i = 0; i < level.beatmapLevelData.difficultyBeatmapSets.Count; ++i) {
						for(int j = 0; j < level.beatmapLevelData.difficultyBeatmapSets[i].difficultyBeatmaps.Count; ++j) {
							if(level.beatmapLevelData.difficultyBeatmapSets[i].difficultyBeatmaps[j] is CustomDifficultyBeatmap beatmap) {
								string filename = level.standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].beatmapFilename;
								await ZipFile(archive, System.IO.Path.Combine(level.customLevelPath, filename), filename);
							}
						}
					}
				}
				memoryStream.Seek(0, System.IO.SeekOrigin.Begin);
				return memoryStream.ToArray();
			}
		}
		public static async System.Threading.Tasks.Task<EntitlementsStatus> ShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> task, string levelId) {
			byte[]? lastData = Plugin.uploadData;
			Plugin.uploadData = null;
			EntitlementsStatus status = await task;
			Plugin.Log?.Debug($"EntitlementsStatus: {status}");
			if(status != EntitlementsStatus.Ok) {
				return status;
			}
			BeatmapLevelsModel.GetBeatmapLevelResult result = await LevelLoader.beatmapLevelsModel.GetBeatmapLevelAsync(levelId, default(System.Threading.CancellationToken));
			Plugin.Log?.Debug($"GetBeatmapLevelResult.isError: {result.isError}");
			if(result.isError) {
				if(Plugin.directDownloads && Patches.BeatmapLevelsModel_GetLevelPreviewForLevelId.resolvePreview?.Invoke(levelId) != null)
					return EntitlementsStatus.Unknown;
				else
					return EntitlementsStatus.NotOwned;
			}
			if(Plugin.directDownloads && result.beatmapLevel is CustomBeatmapLevel level) {
				Plugin.Log?.Debug("Zipping custom level");
				if(lastData != null && Plugin.uploadLevel == levelId) {
					Plugin.uploadData = lastData;
				} else {
					Plugin.uploadData = await ZipLevel(level);
					Plugin.uploadLevel = levelId;
				}
				Plugin.Log?.Debug($"Packed {Plugin.uploadData.Length} bytes");
			}
			return EntitlementsStatus.Ok;
		}
		public static void Postfix(ref System.Threading.Tasks.Task<EntitlementsStatus> __result, string levelId) {
			Plugin.Log?.Debug($"NetworkPlayerEntitlementChecker_GetEntitlementStatus");
			__result = ShareWrapper(__result, levelId);
		}
	}

	[DiJack.Replace(typeof(MenuRpcManager))]
	public class BeatUpMenuRpcManager : MenuRpcManager, IMenuRpcManager {
		public Networking.PlayersDataModel? playersDataModel = null;
		string selectedLevelId = System.String.Empty;
		public BeatUpMenuRpcManager(IMultiplayerSessionManager multiplayerSessionManager, INetworkConfig networkConfig) : base(multiplayerSessionManager) {
			Plugin.networkConfig = networkConfig;
			setSelectedBeatmapEvent += HandleSetSelectedBeatmapEvent;
			setIsEntitledToLevelEvent += HandleSetIsEntitledToLevel;
		}
		void HandleSetSelectedBeatmapEvent(string userId, BeatmapIdentifierNetSerializable beatmapId) {
			selectedLevelId = beatmapId.levelID;
			foreach(Plugin.PlayerCell cell in Plugin.playerCells)
				cell.UpdateData(new Networking.PacketHandler.LoadProgress(Networking.PacketHandler.LoadProgress.LoadState.None, 0, 0), true);
		}
		void HandleSetIsEntitledToLevel(string userId, string levelId, EntitlementsStatus entitlementStatus) {
			if(playersDataModel == null)
				return;
			IConnectedPlayer? player = playersDataModel.lobbyStateDataModel.GetPlayerById(userId);
			HandleSetIsEntitledToLevel(player, levelId, entitlementStatus);
		}
		void HandleSetIsEntitledToLevel(IConnectedPlayer? player, string levelId, EntitlementsStatus entitlementStatus) {
			if(player == null || levelId != selectedLevelId)
				return;
			Networking.PacketHandler.LoadProgress.LoadState state = entitlementStatus switch {
				EntitlementsStatus.NotOwned => Networking.PacketHandler.LoadProgress.LoadState.Failed,
				EntitlementsStatus.Ok => Networking.PacketHandler.LoadProgress.LoadState.Done,
				_ => Networking.PacketHandler.LoadProgress.LoadState.None,
			};
			if(state != Networking.PacketHandler.LoadProgress.LoadState.None)
				Plugin.playerCells[player.sortIndex].UpdateData(new Networking.PacketHandler.LoadProgress(state, 65535, 0), true);
		}
		public static bool MissingRequirements(Networking.PacketHandler.RecommendPreview? preview, bool download) {
			if(preview == null) {
				Plugin.Log?.Debug($"Entitlement[good]: no preview");
				return false;
			}
			if(preview.requirements.Length + preview.suggestions.Length >= 1 && download && !Config.Instance.AllowModchartDownloads) {
				Plugin.Log?.Debug($"Entitlement[fail]: blocked by `AllowModchartDownloads`");
				return true;
			}
			if(preview.requirements.Length < 1) {
				Plugin.Log?.Debug($"Entitlement[good]: no requirements");
				return false;
			}
			if(!Plugin.haveSongCore) {
				Plugin.Log?.Debug($"Entitlement[fail]: need SongCore");
				return true;
			}
			if(!System.Linq.Enumerable.All(preview.requirements, x => string.IsNullOrEmpty(x) || SongCore.Collections.capabilities.Contains(x))) {
				Plugin.Log?.Debug($"Entitlement[fail]: missing requirements; need:");
				foreach(string requirement in preview.requirements)
					Plugin.Log?.Debug($"    " + requirement);
				return true;
			}
			Plugin.Log?.Debug($"Entitlement[good]: have all requirements");
			return false;
		}
		void IMenuRpcManager.SetIsEntitledToLevel(string levelId, EntitlementsStatus entitlementStatus) {
			Plugin.Log?.Debug("MenuRpcManager_SetIsEntitledToLevel");
			Networking.PacketHandler.RecommendPreview? preview = playersDataModel?.ResolvePreview(levelId);
			Plugin.Log?.Debug($"entitlementStatus={entitlementStatus}");
			if(MissingRequirements(preview, entitlementStatus == EntitlementsStatus.Unknown)) {
				entitlementStatus = EntitlementsStatus.NotOwned;
			} else if(entitlementStatus == EntitlementsStatus.Ok && Plugin.uploadData != null) {
				Plugin.Log?.Debug($"Announcing share for `{levelId}`");
				multiplayerSessionManager.Send(new Networking.PacketHandler.SetCanShareBeatmap(levelId, "1234", (ulong)Plugin.uploadData.LongLength, true)); // No public method exposes the `onlyFirstDegree` option
			}
			Plugin.Log?.Debug($"entitlementStatus={entitlementStatus}");
			HandleSetIsEntitledToLevel(playersDataModel?.multiplayerSessionManager.localPlayer, levelId, entitlementStatus);
			base.SetIsEntitledToLevel(levelId, entitlementStatus);
		}
	}

	[DiJack.ReplaceInterfacesAndSelf(typeof(MultiplayerLevelLoader))]
	public class LevelLoader : MultiplayerLevelLoader {
		public class MemorySpriteLoader : ISpriteAsyncLoader {
			System.Threading.Tasks.Task<byte[]> dataTask;
			public MemorySpriteLoader(System.Threading.Tasks.Task<byte[]> dataTask) {
				this.dataTask = dataTask;
			}
			public async System.Threading.Tasks.Task<UnityEngine.Sprite?> LoadSpriteAsync(string path, System.Threading.CancellationToken cancellationToken) {
				byte[] data = await dataTask;
				if(data.Length < 1) {
					Plugin.Log?.Debug("Returning default sprite");
					return null;
				}
				Plugin.Log?.Debug("Decoding sprite");
				UnityEngine.Texture2D texture = new UnityEngine.Texture2D(2, 2);
				UnityEngine.ImageConversion.LoadImage(texture, data);
				UnityEngine.Rect rect = new UnityEngine.Rect(0, 0, texture.width, texture.height);
				return UnityEngine.Sprite.Create(texture, rect, new UnityEngine.Vector2(0, 0), 0.1f);
			}
		}
		[Zenject.Inject]
		public Networking.PacketHandler handler = null!;
		[Zenject.Inject]
		public readonly IMenuRpcManager rpcManager = null!;
		[Zenject.Inject]
		public readonly BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection = null!;
		[Zenject.Inject]
		public readonly IMediaAsyncLoader mediaAsyncLoader = null!;
		public EnvironmentInfoSO defaultEnvironmentInfo;
		public EnvironmentInfoSO defaultAllDirectionsEnvironmentInfo;
		public EnvironmentsListSO environmentSceneInfoCollection;
		public static BeatmapLevelsModel beatmapLevelsModel = null!;
		public System.Func<string, Networking.PacketHandler.RecommendPreview?>? resolvePreview = null;
		public readonly string dataPath = System.IO.Path.Combine(System.IO.Path.GetFullPath(CustomLevelPathHelper.baseProjectPath), "BeatUpClient_Data");
		public LevelLoader(CustomLevelLoader customLevelLoader, BeatmapLevelsModel beatmapLevelsModel) {
			defaultEnvironmentInfo = (EnvironmentInfoSO)typeof(CustomLevelLoader).GetField("_defaultEnvironmentInfo", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			defaultAllDirectionsEnvironmentInfo = (EnvironmentInfoSO)typeof(CustomLevelLoader).GetField("_defaultAllDirectionsEnvironmentInfo", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			environmentSceneInfoCollection = (EnvironmentsListSO)typeof(CustomLevelLoader).GetField("_environmentSceneInfoCollection", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			LevelLoader.beatmapLevelsModel = beatmapLevelsModel;
		}
		public bool ValidatedPath(string filename, out string path) {
			path = System.IO.Path.Combine(dataPath, filename);
			if(System.IO.Path.GetDirectoryName(path) != dataPath)
				Plugin.Log?.Error($"PATH MISMATCH: `{System.IO.Path.GetDirectoryName(path)}` != `{dataPath}`");
			else if(System.IO.Path.GetFileName(path) != filename)
				Plugin.Log?.Error($"FILENAME MISMATCH: `{System.IO.Path.GetFileName(path)}` != `{filename}`");
			else
				return true;
			path = System.String.Empty;
			return false;
		}
		public EnvironmentInfoSO LoadEnvironmentInfo(string environmentName, EnvironmentInfoSO defaultInfo) {
			EnvironmentInfoSO environmentInfoSO = environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(environmentName);
			if(environmentInfoSO == null)
				environmentInfoSO = defaultInfo;
			return environmentInfoSO;
		}
		public CustomPreviewBeatmapLevel? LoadZippedPreviewBeatmapLevel(string levelID, StandardLevelInfoSaveData standardLevelInfoSaveData, System.Threading.Tasks.Task<byte[]> cover, System.IO.Compression.ZipArchive archive) {
			try {
				EnvironmentInfoSO environmentInfo = LoadEnvironmentInfo(standardLevelInfoSaveData.environmentName, defaultEnvironmentInfo);
				EnvironmentInfoSO allDirectionsEnvironmentInfo = LoadEnvironmentInfo(standardLevelInfoSaveData.allDirectionsEnvironmentName, defaultAllDirectionsEnvironmentInfo);
				System.Collections.Generic.List<PreviewDifficultyBeatmapSet> sets = new System.Collections.Generic.List<PreviewDifficultyBeatmapSet>();
				StandardLevelInfoSaveData.DifficultyBeatmapSet[] difficultyBeatmapSets = standardLevelInfoSaveData.difficultyBeatmapSets;
				foreach(StandardLevelInfoSaveData.DifficultyBeatmapSet difficultyBeatmapSet in difficultyBeatmapSets) {
					BeatmapCharacteristicSO beatmapCharacteristicBySerializedName = beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(difficultyBeatmapSet.beatmapCharacteristicName);
					if(beatmapCharacteristicBySerializedName != null) {
						BeatmapDifficulty[] diffs = new BeatmapDifficulty[difficultyBeatmapSet.difficultyBeatmaps.Length];
						for(int j = 0; j < difficultyBeatmapSet.difficultyBeatmaps.Length; j++) {
							difficultyBeatmapSet.difficultyBeatmaps[j].difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
							diffs[j] = difficulty;
						}
						sets.Add(new PreviewDifficultyBeatmapSet(beatmapCharacteristicBySerializedName, diffs));
					}
				}
				return new CustomPreviewBeatmapLevel(Plugin.defaultPackCover, standardLevelInfoSaveData, dataPath, new MemorySpriteLoader(cover), levelID, standardLevelInfoSaveData.songName, standardLevelInfoSaveData.songSubName, standardLevelInfoSaveData.songAuthorName, standardLevelInfoSaveData.levelAuthorName, standardLevelInfoSaveData.beatsPerMinute, standardLevelInfoSaveData.songTimeOffset, standardLevelInfoSaveData.shuffle, standardLevelInfoSaveData.shufflePeriod, standardLevelInfoSaveData.previewStartTime, standardLevelInfoSaveData.previewDuration, environmentInfo, allDirectionsEnvironmentInfo, sets.ToArray());
			} catch {
				return null;
			}
		}
		public static async System.Threading.Tasks.Task<UnityEngine.AudioClip?> DecodeAudio(System.IO.Compression.ZipArchiveEntry song, UnityEngine.AudioType type) {
			byte[] songData = new byte[song.Length];
			song.Open().Read(songData, 0, songData.Length);
			System.Net.Sockets.TcpListener host = new System.Net.Sockets.TcpListener(new System.Net.IPEndPoint(System.Net.IPAddress.Any, 0));
			host.Start(1);
			using(UnityEngine.Networking.UnityWebRequest www = UnityEngine.Networking.UnityWebRequestMultimedia.GetAudioClip("http://127.0.0.1:" + ((System.Net.IPEndPoint)host.LocalEndpoint).Port, type)) {
				((UnityEngine.Networking.DownloadHandlerAudioClip)www.downloadHandler).streamAudio = true;
				UnityEngine.AsyncOperation request = www.SendWebRequest();
				System.Net.Sockets.TcpClient client = host.AcceptTcpClient();
				System.Net.Sockets.NetworkStream stream = client.GetStream();
				byte[] resp = System.Text.Encoding.ASCII.GetBytes($"HTTP/1.1 200 \r\naccept-ranges: bytes\r\ncontent-length: {songData.Length}\r\ncontent-type: audio/ogg\r\n\r\n");
				stream.Write(resp, 0, resp.Length);
				stream.Write(songData, 0, songData.Length);
				while(!request.isDone)
					await System.Threading.Tasks.Task.Delay(100);
				client.Close();
				host.Stop();
				if(www.isNetworkError || www.isHttpError) {
					UnityEngine.Debug.Log($"Audio load error: {www.error}");
					return null;
				}
				return UnityEngine.Networking.DownloadHandlerAudioClip.GetContent(www);
			}
		}
		public async System.Threading.Tasks.Task<CustomBeatmapLevel?> LoadZippedBeatmapLevelAsync(CustomPreviewBeatmapLevel customPreviewBeatmapLevel, System.IO.Compression.ZipArchive archive, bool modded, System.Threading.CancellationToken cancellationToken) {
			StandardLevelInfoSaveData standardLevelInfoSaveData = customPreviewBeatmapLevel.standardLevelInfoSaveData;
			CustomBeatmapLevel customBeatmapLevel = new CustomBeatmapLevel(customPreviewBeatmapLevel);
			CustomDifficultyBeatmapSet[] difficultyBeatmapSets = new CustomDifficultyBeatmapSet[standardLevelInfoSaveData.difficultyBeatmapSets.Length];
			for(int i = 0; i < difficultyBeatmapSets.Length; ++i) {
				BeatmapCharacteristicSO beatmapCharacteristicBySerializedName = beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(standardLevelInfoSaveData.difficultyBeatmapSets[i].beatmapCharacteristicName);
				CustomDifficultyBeatmap[] difficultyBeatmaps = new CustomDifficultyBeatmap[standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps.Length];
				difficultyBeatmapSets[i] = new CustomDifficultyBeatmapSet(beatmapCharacteristicBySerializedName);
				for(int j = 0; j < standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps.Length; ++j) {
					string filename = standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].beatmapFilename;
					System.IO.Compression.ZipArchiveEntry file = archive.GetEntry(filename);
					if(file == null) {
						Plugin.Log?.Error("File not found in archive: " + filename);
						return null;
					}
					byte[] rawData = new byte[file.Length];
					file.Open().Read(rawData, 0, rawData.Length);
					if(modded) {
						if(ValidatedPath(filename, out string path))
							System.IO.File.WriteAllBytes(path, rawData); /*await System.IO.File.WriteAllBytesAsync(path, rawData);*/
						else
							return null;
					}
					BeatmapSaveDataVersion3.BeatmapSaveData beatmapSaveData = BeatmapSaveDataVersion3.BeatmapSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(rawData, 0, rawData.Length));
					BeatmapDataBasicInfo beatmapDataBasicInfoFromSaveData = BeatmapDataLoader.GetBeatmapDataBasicInfoFromSaveData(beatmapSaveData);
					standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
					difficultyBeatmaps[j] = new CustomDifficultyBeatmap(customBeatmapLevel, difficultyBeatmapSets[i], difficulty, standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].difficultyRank, standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].noteJumpMovementSpeed, standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].noteJumpStartBeatOffset, standardLevelInfoSaveData.beatsPerMinute, beatmapSaveData, beatmapDataBasicInfoFromSaveData);
				}
				difficultyBeatmapSets[i].SetCustomDifficultyBeatmaps(difficultyBeatmaps);
			}
			System.IO.Compression.ZipArchiveEntry song = archive.GetEntry(standardLevelInfoSaveData.songFilename);
			if(song == null) {
				Plugin.Log?.Error("File not found in archive: " + standardLevelInfoSaveData.songFilename);
				return null;
			}
			UnityEngine.AudioClip? audioClip = await DecodeAudio(song, AudioTypeHelper.GetAudioTypeFromPath(standardLevelInfoSaveData.songFilename));
			if(audioClip == null) {
				Plugin.Log?.Debug("NULL AUDIO CLIP");
				return null;
			}
			Plugin.Log?.Debug("customBeatmapLevel.SetBeatmapLevelData()");
			customBeatmapLevel.SetBeatmapLevelData(new BeatmapLevelData(audioClip, difficultyBeatmapSets));
			return customBeatmapLevel;
		}
		async System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> DownloadWrapper(System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> task, ILevelGameplaySetupData gameplaySetupData, System.Threading.CancellationToken cancellationToken) {
			BeatmapLevelsModel.GetBeatmapLevelResult result = await task;
			if(!result.isError || Plugin.downloadInfo == null || Plugin.downloadPreview == null)
				return result;
			Plugin.Log?.Debug("Starting direct download");
			try {
				cancellationToken.ThrowIfCancellationRequested();
				int it = 0, p = 0;
				uint cycle = 0;
				ulong off = handler.gaps[0].start;
				IConnectedPlayer[] sources = new IConnectedPlayer[Plugin.downloadInfo.sourcePlayers.Length];
				for(uint i = 0; i < sources.Length; ++i)
					sources[i] = handler.GetPlayerByUserId(Plugin.downloadInfo.sourcePlayers[i]);
				while(handler.gaps.Count > 0) {
					if(++cycle == 4) {
						ulong dl = (ulong)handler.buffer.LongLength;
						foreach((ulong start, ulong end) in handler.gaps)
							dl -= (end - start);
						Networking.PacketHandler.LoadProgress packet = new Networking.PacketHandler.LoadProgress(Networking.PacketHandler.LoadProgress.LoadState.Downloading, (ushort)(dl * 65535 / (ulong)handler.buffer.Length));
						handler.HandleLoadProgress(packet, handler.multiplayerSessionManager.localPlayer);
						handler.multiplayerSessionManager.SendUnreliable(packet);
						cycle = 0;
					}
					if(it >= handler.gaps.Count)
						it = 0;
					for(uint i = 64; i > 0; --i) {
						if(off >= handler.gaps[it].end) {
							if(++it >= handler.gaps.Count)
								it = 0;
							off = handler.gaps[it].start;
						}
						ushort len = (ushort)System.Math.Min(380, handler.gaps[it].end - off);
						handler.SendUnreliableToPlayer(new Networking.PacketHandler.LevelFragmentRequest(off, len), sources[p]);
						p = (p + 1) % sources.Length;
						off += 380;
					}
					await System.Threading.Tasks.Task.Delay(7, cancellationToken);
					cancellationToken.ThrowIfCancellationRequested();
				}
				Plugin.Log?.Debug($"Finished downloading {handler.buffer.Length} bytes");
				using(System.IO.MemoryStream stream = new System.IO.MemoryStream(handler.buffer)) {
					using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(stream, System.IO.Compression.ZipArchiveMode.Read)) {
						if(archive == null)
							return result;
						ulong totalLength = 0;
						foreach(System.IO.Compression.ZipArchiveEntry entry in archive.Entries) {
							totalLength += (ulong)entry.Length;
							if((ulong)entry.Length > Plugin.MaxUnzippedSize || totalLength > Plugin.MaxUnzippedSize)
								return result;
						}
						System.IO.Compression.ZipArchiveEntry infoFile = archive.GetEntry("Info.dat");
						if(infoFile == null) {
							Plugin.Log?.Error("File not found in archive: Info.dat");
							return result;
						}
						if(System.IO.Directory.Exists(dataPath)) {
							foreach(System.IO.FileInfo file in new System.IO.DirectoryInfo(dataPath).GetFiles())
								file.Delete();
						} else {
							System.IO.Directory.CreateDirectory(dataPath);
						}
						bool modded = Plugin.downloadPreview.requirements.Length > 0 || Plugin.downloadPreview.suggestions.Length > 0;
						byte[] rawInfo = new byte[infoFile.Length];
						infoFile.Open().Read(rawInfo, 0, rawInfo.Length);
						if(modded) {
							if(ValidatedPath("Info.dat", out string path))
								System.IO.File.WriteAllBytes(path, rawInfo);
							else
								return result;
						}
						StandardLevelInfoSaveData info = StandardLevelInfoSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(rawInfo, 0, rawInfo.Length));
						CustomPreviewBeatmapLevel? previewBeatmapLevel = LoadZippedPreviewBeatmapLevel(gameplaySetupData.beatmapLevel.beatmapLevel.levelID, info, Plugin.downloadPreview.preview.coverRenderTask, archive);
						if(previewBeatmapLevel == null)
							return result;
						Plugin.downloadPreview.preview.Init(previewBeatmapLevel);
						CustomBeatmapLevel? level = await LoadZippedBeatmapLevelAsync(previewBeatmapLevel, archive, modded, cancellationToken);
						if(level == null)
							return result;
						Plugin.Log?.Debug($"level: {level}");
						return new BeatmapLevelsModel.GetBeatmapLevelResult(isError: false, level);
					}
				}
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Direct download error: " + ex);
				Plugin.Log?.Error(ex.StackTrace);
				return result;
			}
		}
		public override void LoadLevel(ILevelGameplaySetupData gameplaySetupData, float initialStartTime) {
			bool pass = (_loaderState != MultiplayerBeatmapLoaderState.NotLoading);
			base.LoadLevel(gameplaySetupData, initialStartTime);
			Plugin.Log?.Debug($"LoadLevel({gameplaySetupData.beatmapLevel.beatmapLevel.levelID})");
			if(pass || Plugin.downloadInfo == null || gameplaySetupData.beatmapLevel.beatmapLevel.levelID != Plugin.downloadInfo.levelId)
				return;
			Plugin.Log?.Debug("DownloadWrapper()");
			_getBeatmapLevelResultTask = DownloadWrapper(_getBeatmapLevelResultTask, gameplaySetupData, _getBeatmapCancellationTokenSource.Token);
		}
		public override void Tick() {
			MultiplayerBeatmapLoaderState prevState = _loaderState;
			base.Tick();
			if(prevState == MultiplayerBeatmapLoaderState.LoadingBeatmap && _loaderState == MultiplayerBeatmapLoaderState.WaitingForCountdown)
				rpcManager.SetIsEntitledToLevel(_gameplaySetupData.beatmapLevel.beatmapLevel.levelID, EntitlementsStatus.Ok);
		}
	}

	[HarmonyLib.HarmonyPatch]
	public class ConnectedPlayerManager_Send {
		public static System.Reflection.MethodBase TargetMethod() {
			return typeof(ConnectedPlayerManager).GetMethod("Send").MakeGenericMethod(typeof(LiteNetLib.Utils.INetSerializable));
		}
		public static bool Prefix(ConnectedPlayerManager __instance, LiteNetLib.Utils.INetSerializable message) {
			if(Config.Instance.UnreliableState && (message is NodePoseSyncStateNetSerializable || message is StandardScoreSyncStateNetSerializable)) {
				__instance.SendUnreliable(message);
				return false;
			}
			return true;
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(MultiplayerOutroAnimationController), nameof(MultiplayerOutroAnimationController.AnimateOutro))]
	public class MultiplayerOutroAnimationController_AnimateOutro {
		public static bool Prefix(System.Action onCompleted) {
			if(Plugin.skipResults)
				onCompleted.Invoke();
			return !Plugin.skipResults;
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(GameServerPlayersTableView), nameof(GameServerPlayersTableView.SetData))]
	public class GameServerPlayersTableView_SetData {
		public static void Postfix(System.Collections.Generic.List<IConnectedPlayer> sortedPlayers, HMUI.TableView ____tableView) {
			for(uint i = 0; i < Plugin.playerCells.Length; ++i)
				Plugin.playerCells[i].transform = null;
			Plugin.Log?.Debug("GameServerPlayersTableView_SetData()");
			foreach(HMUI.TableCell cell in ____tableView.visibleCells) {
				foreach(UnityEngine.Transform child in cell.transform) {
					if(child.gameObject.name == "BeatUpClient_Progress") {
						Plugin.playerCells[sortedPlayers[cell.idx].sortIndex].SetTransform((UnityEngine.RectTransform)child);
						break;
					}
				}
			}
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(GameServerPlayerTableCell), nameof(GameServerPlayerTableCell.Awake))]
	public class GameServerPlayerTableCell_Awake {
		public static void Prefix(GameServerPlayerTableCell __instance, UnityEngine.UI.Image ____localPlayerBackgroundImage) {
			UnityEngine.GameObject bar = UnityEngine.Object.Instantiate(____localPlayerBackgroundImage.gameObject, ____localPlayerBackgroundImage.transform.parent);
			bar.name = "BeatUpClient_Progress";
			HMUI.ImageView image = bar.GetComponent<HMUI.ImageView>();
			image.color = new UnityEngine.Color(0.4782609f, 0.6956522f, 0.02173913f, 0.5434783f);
			image.enabled = false;
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(Zenject.Context), "InstallInstallers", new[] {typeof(System.Collections.Generic.List<Zenject.InstallerBase>), typeof(System.Collections.Generic.List<System.Type>), typeof(System.Collections.Generic.List<Zenject.ScriptableObjectInstaller>), typeof(System.Collections.Generic.List<Zenject.MonoInstaller>), typeof(System.Collections.Generic.List<Zenject.MonoInstaller>)})]
	public class Context_InstallInstallers {
		class MenuInstaller : Zenject.Installer {
			public override void InstallBindings() {
				Plugin.Log?.Debug("MenuInstaller.InstallBindings()");
				Container.BindInterfacesAndSelfTo<Networking.PacketHandler>().AsSingle();
			}
		}

		public static void Prefix(ref Zenject.Context __instance, ref System.Collections.Generic.List<Zenject.InstallerBase> normalInstallers, ref System.Collections.Generic.List<System.Type> normalInstallerTypes, ref System.Collections.Generic.List<Zenject.ScriptableObjectInstaller> scriptableObjectInstallers, ref System.Collections.Generic.List<Zenject.MonoInstaller> installers, ref System.Collections.Generic.List<Zenject.MonoInstaller> installerPrefabs) {
			foreach(Zenject.MonoInstaller installer in installers) {
				if(installer.GetType() == typeof(MultiplayerMenuInstaller)) {
					Plugin.Log?.Debug($"Adding {typeof(MenuInstaller)}");
					normalInstallerTypes.Add(typeof(MenuInstaller));
				}
			}
		}
	}
}

namespace BeatUpClient.Networking {
	public class PacketHandler : Zenject.IInitializable, System.IDisposable {
		public enum MessageType : byte {
			RecommendPreview, // client -> client
			SetCanShareBeatmap, // client -> server
			DirectDownloadInfo, // server -> client
			LevelFragmentRequest, // client -> client (unreliable)
			LevelFragment, // client -> client (unreliable)
			LoadProgress, // client -> client (unreliable)
		};

		public class NetworkPreviewBeatmapLevel : IPreviewBeatmapLevel, LiteNetLib.Utils.INetSerializable {
			public string levelID { get; set; } = System.String.Empty;
			public string songName { get; set; } = System.String.Empty;
			public string songSubName { get; set; } = System.String.Empty;
			public string songAuthorName { get; set; } = System.String.Empty;
			public string levelAuthorName { get; set; } = System.String.Empty;
			public float beatsPerMinute { get; set; }
			public float songTimeOffset { get; set; }
			public float shuffle { get; set; }
			public float shufflePeriod { get; set; }
			public float previewStartTime { get; set; }
			public float previewDuration { get; set; }
			public float songDuration { get; set; }
			public System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet> previewDifficultyBeatmapSets { get; set; } = null!;
			public EnvironmentInfoSO environmentInfo { get; set; } = null!;
			public EnvironmentInfoSO allDirectionsEnvironmentInfo { get; set; } = null!;
			public readonly ByteArrayNetSerializable cover = new ByteArrayNetSerializable("cover", 0, 8192);
			public System.Threading.Tasks.Task<byte[]> coverRenderTask = System.Threading.Tasks.Task.FromResult<byte[]>(new byte[0]);
			public async System.Threading.Tasks.Task<UnityEngine.Sprite> GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) {
				Plugin.Log?.Debug("NetworkPreviewBeatmapLevel.GetCoverImageAsync()");
				return await new Patches.LevelLoader.MemorySpriteLoader(coverRenderTask).LoadSpriteAsync("", cancellationToken) ?? Plugin.defaultPackCover;
			}
			public async System.Threading.Tasks.Task<byte[]> RenderCoverImageAsync(IPreviewBeatmapLevel beatmapLevel) {
				UnityEngine.Sprite fullSprite = await beatmapLevel.GetCoverImageAsync(default(System.Threading.CancellationToken));
				UnityEngine.RenderTexture target = new UnityEngine.RenderTexture(128, 128, 0);
				UnityEngine.Graphics.SetRenderTarget(target);
				UnityEngine.GL.LoadPixelMatrix(0, 1, 1, 0);
				UnityEngine.GL.Clear(true, true, new UnityEngine.Color(0, 0, 0, 0));
				UnityEngine.Graphics.DrawTexture(new UnityEngine.Rect(0, 0, 1, 1), fullSprite.texture);
				UnityEngine.Texture2D pixels = new UnityEngine.Texture2D(128, 128, UnityEngine.TextureFormat.RGB24, false);
				UnityEngine.RenderTexture.active = target;
				pixels.ReadPixels(new UnityEngine.Rect(0, 0, target.width, target.height), 0, 0);
				pixels.Apply();
				for(int quality = 90; quality > 40; quality = quality * 9 / 10) {
					byte[] data = UnityEngine.ImageConversion.EncodeToJPG(pixels, quality);
					if(data.Length <= 8192)
						return data;
				}
				return new byte[0];
			}
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.Put((string)levelID);
				writer.Put((string)songName);
				writer.Put((string)songSubName);
				writer.Put((string)songAuthorName);
				writer.Put((string)levelAuthorName);
				writer.Put((float)beatsPerMinute);
				writer.Put((float)songTimeOffset);
				writer.Put((float)shuffle);
				writer.Put((float)shufflePeriod);
				writer.Put((float)previewStartTime);
				writer.Put((float)previewDuration);
				writer.Put((float)songDuration);
				writer.Put((byte)UpperBound((uint)previewDifficultyBeatmapSets.Count, 8));
				foreach(PreviewDifficultyBeatmapSet previewDifficultyBeatmapSet in previewDifficultyBeatmapSets) {
					writer.Put((string)previewDifficultyBeatmapSet.beatmapCharacteristic.serializedName);
					writer.Put((byte)UpperBound((uint)previewDifficultyBeatmapSet.beatmapDifficulties.Length, 5));
					foreach(BeatmapDifficulty difficulty in previewDifficultyBeatmapSet.beatmapDifficulties)
						writer.PutVarUInt((uint)difficulty);
				}
				coverRenderTask.Wait();
				cover.data = coverRenderTask.Result;
				cover.Serialize(writer);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				levelID = reader.GetString();
				songName = reader.GetString();
				songSubName = reader.GetString();
				songAuthorName = reader.GetString();
				levelAuthorName = reader.GetString();
				beatsPerMinute = reader.GetFloat();
				songTimeOffset = reader.GetFloat();
				shuffle = reader.GetFloat();
				shufflePeriod = reader.GetFloat();
				previewStartTime = reader.GetFloat();
				previewDuration = reader.GetFloat();
				songDuration = reader.GetFloat();
				PreviewDifficultyBeatmapSet[] sets = new PreviewDifficultyBeatmapSet[(int)UpperBound(reader.GetByte(), 8)];
				for(int i = 0; i < sets.Length; ++i) {
					BeatmapCharacteristicSO beatmapCharacteristic = beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(reader.GetString());
					BeatmapDifficulty[] beatmapDifficulties = new BeatmapDifficulty[UpperBound(reader.GetByte(), 5)];
					for(uint j = 0; j < beatmapDifficulties.Length; ++j)
						beatmapDifficulties[j] = (BeatmapDifficulty)reader.GetVarUInt();
					sets[i] = new PreviewDifficultyBeatmapSet(beatmapCharacteristic, beatmapDifficulties);
				}
				previewDifficultyBeatmapSets = sets;
				cover.Deserialize(reader);
				coverRenderTask = System.Threading.Tasks.Task.FromResult<byte[]>(cover.data);
			}
			public void Init(IPreviewBeatmapLevel beatmapLevel) {
				levelID = beatmapLevel.levelID;
				songName = beatmapLevel.songName;
				songSubName = beatmapLevel.songSubName;
				songAuthorName = beatmapLevel.songAuthorName;
				levelAuthorName = beatmapLevel.levelAuthorName;
				beatsPerMinute = beatmapLevel.beatsPerMinute;
				songTimeOffset = beatmapLevel.songTimeOffset;
				shuffle = beatmapLevel.shuffle;
				shufflePeriod = beatmapLevel.shufflePeriod;
				previewStartTime = beatmapLevel.previewStartTime;
				previewDuration = beatmapLevel.previewDuration;
				songDuration = beatmapLevel.songDuration;
				previewDifficultyBeatmapSets = beatmapLevel.previewDifficultyBeatmapSets;
				cover.data = new byte[0];
				coverRenderTask = RenderCoverImageAsync(beatmapLevel);
				environmentInfo = beatmapLevel.environmentInfo;
				allDirectionsEnvironmentInfo = beatmapLevel.allDirectionsEnvironmentInfo;
				Plugin.Log?.Debug($"ASSIGN:");
				Plugin.Log?.Debug($"    beatmapLevel={beatmapLevel}");
				Plugin.Log?.Debug($"    previewDifficultyBeatmapSets={previewDifficultyBeatmapSets}");
			}
			public NetworkPreviewBeatmapLevel() {}
			public NetworkPreviewBeatmapLevel(IPreviewBeatmapLevel beatmapLevel) =>
				Init(beatmapLevel);
		}

		public class RecommendPreview : LiteNetLib.Utils.INetSerializable {
			public NetworkPreviewBeatmapLevel preview = new NetworkPreviewBeatmapLevel();
			public string[] requirements = new string[0];
			public string[] suggestions = new string[0];
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				preview.Serialize(writer);
				writer.PutVarUInt(UpperBound((uint)requirements.Length, 16));
				foreach(string requirement in requirements)
					writer.Put((string)requirement);
				writer.PutVarUInt(UpperBound((uint)suggestions.Length, 16));
				foreach(string suggestion in suggestions)
					writer.Put((string)suggestion);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				preview.Deserialize(reader);
				requirements = new string[UpperBound(reader.GetVarUInt(), 16)];
				for(uint i = 0; i < requirements.Length; ++i)
					requirements[i] = reader.GetString();
				suggestions = new string[UpperBound(reader.GetVarUInt(), 16)];
				for(uint i = 0; i < suggestions.Length; ++i)
					suggestions[i] = reader.GetString();
			}
			public RecommendPreview() {}
			public RecommendPreview(IPreviewBeatmapLevel beatmapLevel, string[] requirements, string[] suggestions) {
				this.preview = new NetworkPreviewBeatmapLevel(beatmapLevel);
				this.requirements = requirements;
				this.suggestions = suggestions;
			}
		}

		public class SetCanShareBeatmap : LiteNetLib.Utils.INetSerializable {
			public string levelId = System.String.Empty;
			public string levelHash = System.String.Empty;
			public ulong fileSize;
			public bool canShare;
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				Plugin.Log?.Debug($"SetCanShareBeatmap.Serialize()");
				writer.Put(levelId);
				writer.Put(levelHash);
				writer.PutVarULong(fileSize);
				writer.Put(canShare);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				levelId = reader.GetString();
				levelHash = reader.GetString();
				fileSize = reader.GetVarULong();
				canShare = reader.GetBool();
			}
			public SetCanShareBeatmap() {}
			public SetCanShareBeatmap(string levelId, string levelHash, ulong fileSize, bool canShare) {
				Plugin.Log?.Debug($"SetCanShareBeatmap()");
				this.levelId = levelId;
				this.levelHash = levelHash;
				this.fileSize = fileSize;
				this.canShare = canShare;
			}
		}

		public class DirectDownloadInfo : LiteNetLib.Utils.INetSerializable {
			public string levelId = System.String.Empty;
			public string levelHash = System.String.Empty;
			public ulong fileSize;
			public string[] sourcePlayers = new string[0];
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.Put(levelId);
				writer.Put(levelHash);
				writer.PutVarULong(fileSize);
				writer.Put((byte)UpperBound((uint)sourcePlayers.Length, 128));
				foreach(string player in sourcePlayers)
					writer.Put(player);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				Plugin.Log?.Debug("DirectDownloadInfo.Deserialize()");
				levelId = reader.GetString();
				levelHash = reader.GetString();
				fileSize = reader.GetVarULong();
				sourcePlayers = new string[UpperBound(reader.GetByte(), 128)];
				for(byte i = 0; i < sourcePlayers.Length; ++i)
					sourcePlayers[i] = reader.GetString();
			}
			public DirectDownloadInfo() {}
		}

		public class LevelFragmentRequest : LiteNetLib.Utils.INetSerializable {
			public ulong offset;
			public ushort maxSize;
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.PutVarULong(offset);
				writer.Put(maxSize);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				offset = reader.GetVarULong();
				maxSize = reader.GetUShort();
			}
			public LevelFragmentRequest() {}
			public LevelFragmentRequest(ulong offset, ushort maxSize) {
				this.offset = offset;
				this.maxSize = maxSize;
			}
		}

		public class LevelFragment : LiteNetLib.Utils.INetSerializable {
			public ulong offset;
			public byte[] data = new byte[0];
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.PutVarULong(offset);
				writer.Put((ushort)UpperBound((uint)data.Length, 1500));
				writer.Put(data);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				offset = reader.GetVarULong();
				data = new byte[UpperBound(reader.GetUShort(), 1500)];
				reader.GetBytes(data, data.Length);
			}
			public LevelFragment() {}
			public LevelFragment(ulong offset, byte[] data) {
				this.offset = offset;
				this.data = data;
			}
		}

		public struct LoadProgress : LiteNetLib.Utils.INetSerializable {
			public enum LoadState : byte {
				None,
				Failed,
				Exporting,
				Downloading,
				Loading,
				Done,
			}
			static uint localSequence = 0;
			public uint sequence;
			public LoadState state;
			public ushort progress;
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.Put((uint)sequence);
				writer.Put((byte)state);
				writer.Put((ushort)progress);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				sequence = reader.GetUInt();
				state = (LoadState)reader.GetByte();
				progress = reader.GetUShort();
			}
			public LoadProgress(LoadState state, ushort progress, uint sequence) {
				this.sequence = sequence;
				this.state = state;
				this.progress = progress;
			}
			public LoadProgress(LoadState state, ushort progress) : this(state, progress, ++localSequence) {}
		}

		public static BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection = null!;
		public readonly System.Reflection.FieldInfo fi_ConnectedPlayer_connection;
		public readonly System.Reflection.FieldInfo fi_ConnectedPlayer_connectionId;
		public readonly System.Reflection.FieldInfo fi_ConnectedPlayer_remoteConnectionId;
		public readonly System.Reflection.MethodInfo mi_ConnectedPlayerManager_WriteOne;
		public readonly MultiplayerSessionManager multiplayerSessionManager;
		public readonly MultiplayerSessionManager.MessageType messageType = (MultiplayerSessionManager.MessageType)101;
		public readonly NetworkPacketSerializer<MessageType, IConnectedPlayer> serializer = new NetworkPacketSerializer<MessageType, IConnectedPlayer>();
		public PlayersDataModel? playersDataModel = null;

		public PacketHandler(BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection, IMultiplayerSessionManager multiplayerSessionManager) {
			Plugin.Log?.Debug("PacketHandler()");
			PacketHandler.beatmapCharacteristicCollection = beatmapCharacteristicCollection;
			System.Type ConnectedPlayer = typeof(ConnectedPlayerManager).Assembly.GetType("ConnectedPlayerManager+ConnectedPlayer");
			fi_ConnectedPlayer_connection = ConnectedPlayer.GetField("_connection", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			fi_ConnectedPlayer_connectionId = ConnectedPlayer.GetField("_connectionId", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			fi_ConnectedPlayer_remoteConnectionId = ConnectedPlayer.GetField("_remoteConnectionId", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			mi_ConnectedPlayerManager_WriteOne = typeof(ConnectedPlayerManager).GetMethod("WriteOne", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			this.multiplayerSessionManager = (MultiplayerSessionManager)multiplayerSessionManager;
		}

		public static uint UpperBound(uint value, uint limit) {
			if(value > limit)
				throw new System.ArgumentOutOfRangeException();
			return value;
		}

		public void Initialize() {
			Plugin.Log?.Debug("multiplayerSessionManager.RegisterSerializer()");
			multiplayerSessionManager.RegisterSerializer(messageType, serializer);
			serializer.RegisterCallback<RecommendPreview>(MessageType.RecommendPreview, HandleRecommendPreview);
			serializer.RegisterCallback<SetCanShareBeatmap>(MessageType.SetCanShareBeatmap, HandleSetCanShareBeatmap);
			serializer.RegisterCallback<DirectDownloadInfo>(MessageType.DirectDownloadInfo, HandleDirectDownloadInfo);
			serializer.RegisterCallback<LevelFragmentRequest>(MessageType.LevelFragmentRequest, HandleLevelFragmentRequest);
			serializer.RegisterCallback<LevelFragment>(MessageType.LevelFragment, HandleLevelFragment);
			serializer.RegisterCallback<LoadProgress>(MessageType.LoadProgress, HandleLoadProgress);
		}

		public void Dispose() {
			multiplayerSessionManager.UnregisterSerializer(messageType, serializer);
		}

		public IConnectedPlayer GetPlayerByUserId(string userId) {
			return multiplayerSessionManager.GetPlayerByUserId(userId);
		}

		public void Send<T>(T message) where T : LiteNetLib.Utils.INetSerializable =>
			multiplayerSessionManager.Send(message);

		public void SendUnreliableToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable {
			ConnectedPlayerManager? connectedPlayerManager = multiplayerSessionManager.connectedPlayerManager;
			if(connectedPlayerManager == null)
				return;
			if(connectedPlayerManager.isConnected)
				((IConnection)fi_ConnectedPlayer_connection.GetValue(player)).Send((LiteNetLib.Utils.NetDataWriter)mi_ConnectedPlayerManager_WriteOne.Invoke(connectedPlayerManager, new[] {(object)fi_ConnectedPlayer_connectionId.GetValue(connectedPlayerManager.localPlayer), (object)fi_ConnectedPlayer_remoteConnectionId.GetValue(player), (object)message}), LiteNetLib.DeliveryMethod.Unreliable);
			else if(message is IPoolablePacket poolable)
				poolable.Release();
		}

		public event System.Action<RecommendPreview, IConnectedPlayer>? recommendPreviewEvent;
		public void HandleRecommendPreview(RecommendPreview packet, IConnectedPlayer player) {
			Plugin.Log?.Debug("PacketHandler.HandleRecommendPreview()");
			Plugin.Log?.Debug($"    recommendPreviewEvent={recommendPreviewEvent}");
			recommendPreviewEvent?.Invoke(packet, player);
		}

		public void HandleSetCanShareBeatmap(SetCanShareBeatmap packet, IConnectedPlayer player) {}

		public byte[] buffer = new byte[0];
		public readonly System.Collections.Generic.List<(ulong start, ulong end)> gaps = new System.Collections.Generic.List<(ulong, ulong)>();
		public void HandleDirectDownloadInfo(DirectDownloadInfo packet, IConnectedPlayer player) {
			Plugin.Log?.Debug($"DirectDownloadInfo:\n    levelId=\"{packet.levelId}\"\n    levelHash=\"{packet.levelHash}\"\n    fileSize={packet.fileSize}\n    source=\"{packet.sourcePlayers[0]}\"");
			RecommendPreview? preview = playersDataModel?.ResolvePreview(packet.levelId);
			if(!Plugin.directDownloads || preview == null || Patches.BeatUpMenuRpcManager.MissingRequirements(preview, true))
				return;
			if(packet.fileSize > Plugin.MaxDownloadSize || packet.sourcePlayers.Length < 1)
				return;
			Plugin.downloadInfo = packet;
			Plugin.downloadPreview = preview;
			buffer = new byte[packet.fileSize];
			gaps.Clear();
			gaps.Add((0, packet.fileSize));
		}

		public void HandleLevelFragmentRequest(LevelFragmentRequest packet, IConnectedPlayer player) {
			if(Plugin.uploadData == null || packet.offset > (ulong)Plugin.uploadData.LongLength)
				return;
			byte[] data = new byte[System.Math.Min(packet.maxSize, (ulong)Plugin.uploadData.LongLength - packet.offset)];
			System.Buffer.BlockCopy(Plugin.uploadData, (int)packet.offset, data, 0, data.Length);
			SendUnreliableToPlayer(new LevelFragment(packet.offset, data), player);
		}

		public void HandleLevelFragment(LevelFragment packet, IConnectedPlayer player) {
			if(buffer == null || packet.offset >= (ulong)buffer.LongLength || packet.offset + (uint)packet.data.LongLength > (ulong)buffer.LongLength)
				return;
			System.Buffer.BlockCopy(packet.data, 0, buffer, (int)packet.offset, packet.data.Length);
			ulong start = packet.offset, end = packet.offset + (ulong)packet.data.LongLength;
			int i = gaps.Count - 1;
			while(i > 0 && gaps[i].start > start)
				--i;
			if(gaps[i].end > end) {
				gaps.Insert(i+1, (System.Math.Max(gaps[i].start, end), gaps[i].end));
			} else {
				++i;
				while(gaps.Count > i) {
					if(gaps[i].end > end)
						break;
					gaps.RemoveAt(i);
				}
				if(gaps.Count > i)
					gaps[i] = (System.Math.Max(gaps[i].start, end), gaps[i].end);
				--i;
			}
			gaps[i] = (gaps[i].start, System.Math.Min(gaps[i].end, start));
			if(gaps[i].end <= gaps[i].start)
				gaps.RemoveAt(i);
		}

		public void HandleLoadProgress(LoadProgress packet, IConnectedPlayer player) =>
			Plugin.playerCells[player.sortIndex].UpdateData(packet);
	}

	[DiJack.Replace(typeof(LobbyPlayersDataModel))]
	public class PlayersDataModel : LobbyPlayersDataModel, ILobbyPlayersDataModel, System.IDisposable {
		[Zenject.Inject]
		public readonly ILobbyStateDataModel lobbyStateDataModel = null!;
		[Zenject.Inject]
		public readonly PacketHandler handler = null!;
		[Zenject.Inject]
		public readonly Patches.LevelLoader loader = null!;
		public readonly MultiplayerSessionManager multiplayerSessionManager;

		public PlayersDataModel(IMultiplayerSessionManager multiplayerSessionManager) {
			this.multiplayerSessionManager = (MultiplayerSessionManager)multiplayerSessionManager;
		}

		public override void Activate() {
			Plugin.Log?.Debug("PlayersDataModel.Activate()");
			((Patches.BeatUpMenuRpcManager)_menuRpcManager).playersDataModel = this;
			handler.playersDataModel = this;
			base.Activate();
			_menuRpcManager.getRecommendedBeatmapEvent -= base.HandleMenuRpcManagerGetRecommendedBeatmap;
			_menuRpcManager.getRecommendedBeatmapEvent += this.HandleMenuRpcManagerGetRecommendedBeatmap;
			handler.recommendPreviewEvent += HandleRecommendPreview;
			Patches.BeatmapLevelsModel_GetLevelPreviewForLevelId.resolvePreview += ResolvePreview;
			loader.resolvePreview += ResolvePreview;
		}

		public override void Deactivate() {
			Plugin.Log?.Debug("PlayersDataModel.Deactivate()");
			loader.resolvePreview -= ResolvePreview;
			Patches.BeatmapLevelsModel_GetLevelPreviewForLevelId.resolvePreview -= ResolvePreview;
			handler.recommendPreviewEvent -= HandleRecommendPreview;
			_menuRpcManager.getRecommendedBeatmapEvent -= this.HandleMenuRpcManagerGetRecommendedBeatmap;
			_menuRpcManager.getRecommendedBeatmapEvent += base.HandleMenuRpcManagerGetRecommendedBeatmap;
			base.Deactivate();
			handler.playersDataModel = null;
			((Patches.BeatUpMenuRpcManager)_menuRpcManager).playersDataModel = null;
		}

		public override void Dispose() {
			Deactivate();
		}

		private void HandleRecommendPreview(PacketHandler.RecommendPreview packet, IConnectedPlayer player) {
			Plugin.Log?.Debug($"PlayersDataModel.HandleRecommendPreview(\"{packet.preview.levelID}\", {player})");
			Plugin.playerPreviews[player.sortIndex] = packet;
		}

		public PacketHandler.RecommendPreview? ResolvePreview(string levelId) =>
			System.Linq.Enumerable.FirstOrDefault(Plugin.playerPreviews, (PacketHandler.RecommendPreview? preview) => preview?.preview.levelID == levelId);

		public override void HandleMenuRpcManagerGetRecommendedBeatmap(string userId) {
			if(Plugin.playerPreviews[multiplayerSessionManager.localPlayer.sortIndex] != null)
				multiplayerSessionManager.Send(Plugin.playerPreviews[multiplayerSessionManager.localPlayer.sortIndex]);
			base.HandleMenuRpcManagerGetRecommendedBeatmap(userId);
		}

		public override void SetLocalPlayerBeatmapLevel(PreviewDifficultyBeatmap? beatmapLevel) {
			if(beatmapLevel != null) {
				PacketHandler.RecommendPreview? localPreview = Plugin.playerPreviews[multiplayerSessionManager.localPlayer.sortIndex];
				if(!(localPreview?.preview.levelID == beatmapLevel.beatmapLevel.levelID)) {
					string[]? requirementArray = null, suggestionArray = null;
					PacketHandler.RecommendPreview? preview = ResolvePreview(beatmapLevel.beatmapLevel.levelID);
					if(preview != null) {
						requirementArray = preview.requirements;
						suggestionArray = preview.suggestions;
					} else if(beatmapLevel.beatmapLevel is CustomPreviewBeatmapLevel previewBeatmapLevel) {
						System.Collections.Generic.HashSet<string> requirementSet = new System.Collections.Generic.HashSet<string>();
						System.Collections.Generic.HashSet<string> suggestionSet = new System.Collections.Generic.HashSet<string>();
						string path = System.IO.Path.Combine(previewBeatmapLevel.customLevelPath, "Info.dat");
						if(System.IO.File.Exists(path)) {
							Newtonsoft.Json.Linq.JObject info = Newtonsoft.Json.Linq.JObject.Parse(System.IO.File.ReadAllText(path));
							if(info.TryGetValue("_difficultyBeatmapSets", out Newtonsoft.Json.Linq.JToken? diffSets_token)) {
								if(diffSets_token is Newtonsoft.Json.Linq.JArray diffSets) {
									foreach(Newtonsoft.Json.Linq.JObject diffSet in diffSets) {
										if(diffSet.TryGetValue("_difficultyBeatmaps", out Newtonsoft.Json.Linq.JToken? diffBeatmaps_token)) {
											if(diffBeatmaps_token is Newtonsoft.Json.Linq.JArray diffBeatmaps) {
												foreach(Newtonsoft.Json.Linq.JObject diffBeatmap in diffBeatmaps) {
													if(diffBeatmap.TryGetValue("_customData", out Newtonsoft.Json.Linq.JToken? customData_token)) {
														if(customData_token is Newtonsoft.Json.Linq.JObject customData) {
															if(customData.TryGetValue("_requirements", out Newtonsoft.Json.Linq.JToken? requirements_token))
																if(requirements_token is Newtonsoft.Json.Linq.JArray requirements)
																	foreach(string? requirement in requirements)
																		if(requirement != null)
																			requirementSet.Add(requirement);
															if(customData.TryGetValue("_suggestions", out Newtonsoft.Json.Linq.JToken? suggestions_token))
																if(suggestions_token is Newtonsoft.Json.Linq.JArray suggestions)
																	foreach(string? suggestion in suggestions)
																		if(suggestion != null)
																			suggestionSet.Add(suggestion);
														}
													}
												}
											}
										}
									}
								}
							}
							requirementArray = new string[requirementSet.Count];
							suggestionArray = new string[suggestionSet.Count];
							requirementSet.CopyTo(requirementArray);
							suggestionSet.CopyTo(suggestionArray);
						}
					}
					if(requirementArray == null)
						requirementArray = new string[0];
					if(suggestionArray == null)
						suggestionArray = new string[0];
					if(requirementArray.Length > 0) {
						Plugin.Log?.Debug($"{requirementArray.Length} requirements for `{beatmapLevel.beatmapLevel.levelID}`:");
						foreach(string req in requirementArray)
							Plugin.Log?.Debug($"    {req}");
					} else {
						Plugin.Log?.Debug($"No requirements for `{beatmapLevel.beatmapLevel.levelID}`");
					}
					Plugin.playerPreviews[multiplayerSessionManager.localPlayer.sortIndex] = new PacketHandler.RecommendPreview(beatmapLevel.beatmapLevel, requirementArray, suggestionArray);
				}
				multiplayerSessionManager.Send(Plugin.playerPreviews[multiplayerSessionManager.localPlayer.sortIndex]);
			}
			Plugin.UpdateDifficultyUI(beatmapLevel, this);
			base.SetLocalPlayerBeatmapLevel(beatmapLevel);
		}

		public override void ClearLocalPlayerBeatmapLevel() {
			Plugin.UpdateDifficultyUI(null);
			base.ClearLocalPlayerBeatmapLevel();
		}
	}
}

namespace BeatUpClient {
	public static class UI {
		public class ToggleSetting : SwitchSettingsController {
			public ValueCB<bool> valueCB = null!;
			public ToggleSetting() {
				UnityEngine.UI.Toggle toggle = gameObject.transform.GetChild(1).gameObject.GetComponent<UnityEngine.UI.Toggle>();
				toggle.onValueChanged.RemoveAllListeners();
				typeof(SwitchSettingsController).GetField("_toggle", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).SetValue(this, toggle);
			}
			protected override bool GetInitValue() => valueCB();
			protected override void ApplyValue(bool value) => valueCB() = value;
		}
		public class ValuePickerSetting : ListSettingsController {
			public byte[] options = null!;
			public ValueCB<float> valueCB = null!;
			public int startIdx = 1;
			public ValuePickerSetting() {
				StepValuePicker stepValuePicker = gameObject.transform.GetChild(1).gameObject.GetComponent<StepValuePicker>();
				typeof(IncDecSettingsController).GetField("_stepValuePicker", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).SetValue(this, stepValuePicker);
			}
			protected override bool GetInitValues(out int idx, out int numberOfElements) {
				idx = System.Array.LastIndexOf(options, (byte)(valueCB() * 4));
				if(idx == 0)
					startIdx = 0;
				else
					--idx;
				numberOfElements = options.Length - startIdx;
				return true;
			}
			protected override void ApplyValue(int idx) => valueCB() = options[idx + startIdx] / 4.0f;
			protected override string TextForValue(int idx) => $"{options[idx + startIdx] / 4.0f}";
		}
		public delegate ref T ValueCB<T>();
		static UnityEngine.GameObject CreateElement(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, string header) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(template, parent);
			((UnityEngine.RectTransform)gameObject.transform).sizeDelta = new UnityEngine.Vector2(90, ((UnityEngine.RectTransform)gameObject.transform).sizeDelta.y);
			gameObject.name = "BeatUpClient_" + name;
			gameObject.SetActive(false);
			UnityEngine.GameObject nameText = gameObject.transform.Find("NameText").gameObject;
			Polyglot.LocalizedTextMeshProUGUI localizedText = nameText.GetComponent<Polyglot.LocalizedTextMeshProUGUI>();
			localizedText.enabled = false;
			localizedText.Key = string.Empty;
			TMPro.TextMeshProUGUI text = nameText.GetComponent<TMPro.TextMeshProUGUI>();
			text.text = header;
			text.richText = true;
			text.overflowMode = TMPro.TextOverflowModes.Ellipsis;
			return gameObject;
		}
		public static UnityEngine.Transform CreateToggle(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, string header, ValueCB<bool> valueCB) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name, header);
			UnityEngine.Object.Destroy(gameObject.GetComponent<BoolSettingsController>());
			ToggleSetting toggleSetting = gameObject.AddComponent<ToggleSetting>();
			toggleSetting.valueCB = valueCB;
			gameObject.SetActive(true);
			return gameObject.transform;
		}
		public static UnityEngine.Transform CreateValuePicker(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, string header, ValueCB<float> valueCB, byte[] options) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name, header);
			UnityEngine.Object.Destroy(gameObject.GetComponent<FormattedFloatListSettingsController>());
			ValuePickerSetting valuePickerSetting = gameObject.AddComponent<ValuePickerSetting>();
			valuePickerSetting.options = options;
			valuePickerSetting.valueCB = valueCB;
			gameObject.SetActive(true);
			return gameObject.transform;
		}
		public static UnityEngine.Transform CreateClone(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, int index) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(template, parent);
			((UnityEngine.RectTransform)gameObject.transform).sizeDelta = new UnityEngine.Vector2(90, ((UnityEngine.RectTransform)gameObject.transform).sizeDelta.y);
			gameObject.name = "BeatUpClient_" + name;
			if(index >= 0)
				gameObject.transform.SetSiblingIndex(index);
			return gameObject.transform;
		}
	}

	public class Config {
		public static Config Instance = new Config();
		public float CountdownDuration = 5;
		public bool SkipResults = false;
		public bool PerPlayerDifficulty = false;
		public bool PerPlayerModifiers = false;
		public ushort WindowSize = 64;
		public bool UnreliableState = false;
		public bool DirectDownloads = true;
		public bool AllowModchartDownloads = false;
	}

	[IPA.Plugin(IPA.RuntimeOptions.SingleStartInit)]
	public class Plugin {
		public struct PlayerCell {
			public UnityEngine.RectTransform? transform;
			public Networking.PacketHandler.LoadProgress data;
			public void SetTransform(UnityEngine.RectTransform transform) {
				this.transform = transform;
				Refresh(transform);
			}
			public void UpdateData(Networking.PacketHandler.LoadProgress data, bool replace = false) {
				if(replace)
					data.sequence = this.data.sequence;
				else if(data.sequence < this.data.sequence)
					return;
				SetData(data);
			}
			public void SetData(Networking.PacketHandler.LoadProgress data) {
				this.data = data;
				if(transform != null)
					Refresh(transform);
			}
			void Refresh(UnityEngine.RectTransform transform) {
				transform.offsetMin = new UnityEngine.Vector2((65535u - data.progress) * 104 / 65535f, transform.offsetMin.y);
				transform.gameObject.GetComponent<HMUI.ImageView>().enabled = (data.state != Networking.PacketHandler.LoadProgress.LoadState.None);
			}
		}

		public const ulong MaxDownloadSize = 268435456;
		public const ulong MaxUnzippedSize = 268435456;
		public const string HarmonyId = "org.battletrains.BeatUpClient";
		public static readonly HarmonyLib.Harmony harmony = new HarmonyLib.Harmony(HarmonyId);
		public static Plugin? Instance;
		public static IPA.Logging.Logger? Log;
		public static UnityEngine.Sprite defaultPackCover = null!;
		public static Networking.PacketHandler.DirectDownloadInfo? downloadInfo = null;
		public static Networking.PacketHandler.RecommendPreview? downloadPreview = null;
		public static string uploadLevel = System.String.Empty;
		public static byte[]? uploadData = null;
		public static INetworkConfig? networkConfig;
		public static Networking.PacketHandler.RecommendPreview?[] playerPreviews = null!;
		public static PlayerCell[] playerCells = null!;
		public static bool haveSongCore = false;
		public static uint windowSize = 64;
		public static bool directDownloads = false;
		public static bool skipResults = false;

		public static System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> ReliableChannel_ctor(System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> instructions) {
			bool notFound = true;
			foreach(HarmonyLib.CodeInstruction instruction in instructions) {
				if(notFound && HarmonyLib.CodeInstructionExtensions.LoadsConstant(instruction, 64)) {
					yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Ldsfld, typeof(Plugin).GetField("windowSize", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.Public));
					notFound = false;
				} else {
					yield return instruction;
				}
			}
			if(notFound)
				Plugin.Log?.Error("Failed to patch reliable window size");
		}

		public static BeatmapCharacteristicSegmentedControlController beatmapCharacteristicSegmentedControlController = null!;
		public static BeatmapDifficultySegmentedControlController beatmapDifficultySegmentedControlController = null!;
		public static System.Reflection.FieldInfo fi_BeatmapCharacteristicSegmentedControlController_didSelectBeatmapCharacteristicEvent = null!;
		public static System.Reflection.FieldInfo fi_BeatmapDifficultySegmentedControlController_didSelectDifficultyEvent = null!;
		public static void UpdateDifficultyUI(PreviewDifficultyBeatmap? beatmapLevel, Networking.PlayersDataModel playersDataModel = null!) {
			fi_BeatmapCharacteristicSegmentedControlController_didSelectBeatmapCharacteristicEvent.SetValue(beatmapCharacteristicSegmentedControlController, (System.Action<BeatmapCharacteristicSegmentedControlController, BeatmapCharacteristicSO>)delegate {});
			fi_BeatmapDifficultySegmentedControlController_didSelectDifficultyEvent.SetValue(beatmapDifficultySegmentedControlController, (System.Action<BeatmapDifficultySegmentedControlController, BeatmapDifficulty>)delegate {});
			if(beatmapLevel == null) {
				beatmapCharacteristicSegmentedControlController.transform.parent.gameObject.SetActive(false);
				beatmapDifficultySegmentedControlController.transform.parent.gameObject.SetActive(false);
				return;
			}
			BeatmapDifficulty[] beatmapDifficulties = null!;
			beatmapCharacteristicSegmentedControlController.SetData(System.Linq.Enumerable.ToList(System.Linq.Enumerable.Select(beatmapLevel.beatmapLevel.previewDifficultyBeatmapSets, set => {
				if(set.beatmapCharacteristic == beatmapLevel.beatmapCharacteristic) {
					beatmapDifficulties = set.beatmapDifficulties;
					beatmapDifficultySegmentedControlController.SetData(System.Linq.Enumerable.ToList(System.Linq.Enumerable.Select(set.beatmapDifficulties, diff => (IDifficultyBeatmap)new CustomDifficultyBeatmap(null, null, diff, 0, 0, 0, 0, null, null))), beatmapLevel.beatmapDifficulty);
				}
				return (IDifficultyBeatmapSet)new CustomDifficultyBeatmapSet(set.beatmapCharacteristic);
			})), beatmapLevel.beatmapCharacteristic);
			beatmapCharacteristicSegmentedControlController.didSelectBeatmapCharacteristicEvent += delegate(BeatmapCharacteristicSegmentedControlController controller, BeatmapCharacteristicSO beatmapCharacteristic) {
				BeatmapDifficulty closestDifficulty = beatmapDifficulties[0];
				foreach(BeatmapDifficulty difficulty in beatmapDifficulties) {
					if(beatmapLevel.beatmapDifficulty < difficulty)
						break;
					closestDifficulty = difficulty;
				}
				playersDataModel.SetLocalPlayerBeatmapLevel(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapCharacteristic, closestDifficulty));
			};
			beatmapDifficultySegmentedControlController.didSelectDifficultyEvent += delegate(BeatmapDifficultySegmentedControlController controller, BeatmapDifficulty difficulty) {
				playersDataModel.SetLocalPlayerBeatmapLevel(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapLevel.beatmapCharacteristic, difficulty));
			};
			beatmapCharacteristicSegmentedControlController.transform.parent.gameObject.SetActive(true);
			beatmapDifficultySegmentedControlController.transform.parent.gameObject.SetActive(true);
		}

		public static void OnSceneLoaded(UnityEngine.SceneManagement.Scene scene, UnityEngine.SceneManagement.LoadSceneMode mode) {
			if(scene.name != "MainMenu")
				return;
			Plugin.Log?.Debug("load MainMenu");
			if(Plugin.networkConfig is CustomNetworkConfig) {
				UnityEngine.GameObject Fullscreen = System.Linq.Enumerable.First(System.Linq.Enumerable.Select(UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.UI.Toggle>(), x => x.transform.parent.gameObject), p => p.name == "Fullscreen");
				UnityEngine.GameObject MaxNumberOfPlayers = System.Linq.Enumerable.First(System.Linq.Enumerable.Select(UnityEngine.Resources.FindObjectsOfTypeAll<StepValuePicker>(), x => x.transform.parent.gameObject), p => p.name == "MaxNumberOfPlayers");
				UnityEngine.Transform CreateServerFormView = MaxNumberOfPlayers.transform.parent;
				UI.CreateValuePicker(MaxNumberOfPlayers, CreateServerFormView, "CountdownDuration", "Countdown Duration", () => ref Config.Instance.CountdownDuration, new byte[] {(byte)(Config.Instance.CountdownDuration * 4), 0, 12, 20, 32, 40, 60});
				UI.CreateToggle(Fullscreen, CreateServerFormView, "SkipResults", "Skip Results Pyramid", () => ref Config.Instance.SkipResults);
				UI.CreateToggle(Fullscreen, CreateServerFormView, "PerPlayerDifficulty", "Per-Player Difficulty", () => ref Config.Instance.PerPlayerDifficulty);
				UI.CreateToggle(Fullscreen, CreateServerFormView, "PerPlayerModifiers", "Per-Player Modifiers", () => ref Config.Instance.PerPlayerModifiers);
				CreateServerFormView.parent.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
				CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
				CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.ContentSizeFitter>().enabled = true;
				CreateServerFormView.parent.parent.gameObject.SetActive(true);
				/*UnityEngine.UI.LayoutRebuilder.ForceRebuildLayoutImmediate(CreateServerFormView.parent.parent as UnityEngine.RectTransform);
				CreateServerFormView.parent.parent.gameObject.SetActive(false);*/
			}

			StandardLevelDetailView LevelDetail = UnityEngine.Resources.FindObjectsOfTypeAll<StandardLevelDetailView>()[0];
			UnityEngine.Transform LobbySetupViewController_Wrapper = UnityEngine.Resources.FindObjectsOfTypeAll<LobbySetupViewController>()[0].transform.GetChild(0);
			UnityEngine.Transform beatmapCharacteristic = UI.CreateClone(((BeatmapCharacteristicSegmentedControlController)typeof(StandardLevelDetailView).GetField("_beatmapCharacteristicSegmentedControlController", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(LevelDetail)).transform.parent.gameObject, LobbySetupViewController_Wrapper, "BeatmapCharacteristic", 2);
			UnityEngine.Transform beatmapDifficulty = UI.CreateClone(((BeatmapDifficultySegmentedControlController)typeof(StandardLevelDetailView).GetField("_beatmapDifficultySegmentedControlController", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(LevelDetail)).transform.parent.gameObject, LobbySetupViewController_Wrapper, "BeatmapDifficulty", 3);
			beatmapCharacteristicSegmentedControlController = beatmapCharacteristic.GetChild(1).gameObject.GetComponent<BeatmapCharacteristicSegmentedControlController>();
			beatmapDifficultySegmentedControlController = beatmapDifficulty.GetChild(1).gameObject.GetComponent<BeatmapDifficultySegmentedControlController>();
			fi_BeatmapCharacteristicSegmentedControlController_didSelectBeatmapCharacteristicEvent = typeof(BeatmapCharacteristicSegmentedControlController).GetField(typeof(BeatmapCharacteristicSegmentedControlController).GetEvent("didSelectBeatmapCharacteristicEvent").Name, System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			fi_BeatmapDifficultySegmentedControlController_didSelectDifficultyEvent = typeof(BeatmapDifficultySegmentedControlController).GetField(typeof(BeatmapDifficultySegmentedControlController).GetEvent("didSelectDifficultyEvent").Name, System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
		}

		[IPA.Init]
		public void Init(IPA.Logging.Logger pluginLogger, IPA.Config.Config conf) {
			Instance = this;
			Plugin.Log = pluginLogger;
			Config.Instance = IPA.Config.Stores.GeneratedStore.Generated<Config>(conf);
			Plugin.Log?.Debug("Logger initialized.");
		}
		[IPA.OnEnable]
		public void OnEnable() {
			haveSongCore = (IPA.Loader.PluginManager.GetPluginFromId("SongCore") != null);
			try {
				Plugin.Log?.Debug("Loading assets");
				defaultPackCover = UnityEngine.AssetBundle.LoadFromStream(System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream("BeatUpClient.cover")).LoadAllAssets<UnityEngine.Sprite>()[0];
				Plugin.Log?.Debug("Applying patches");
				DiJack.ResolveInjections(System.Reflection.Assembly.GetExecutingAssembly());
				harmony.PatchAll(System.Reflection.Assembly.GetExecutingAssembly());
				if(Config.Instance.WindowSize != 64) {
					System.Reflection.MethodBase original = typeof(LiteNetLib.NetManager).Assembly.GetType("LiteNetLib.ReliableChannel").GetConstructor(new[] {typeof(LiteNetLib.NetPeer), typeof(bool), typeof(byte)});
					System.Reflection.MethodInfo transpiler = typeof(Plugin).GetMethod("ReliableChannel_ctor", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.Public);
					harmony.Patch(original, transpiler: new HarmonyLib.HarmonyMethod(transpiler));
				}
				UnityEngine.SceneManagement.SceneManager.sceneLoaded += OnSceneLoaded;
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Error applying patches: " + ex.Message);
				Plugin.Log?.Debug(ex);
				OnDisable();
			}
		}
		[IPA.OnDisable]
		public void OnDisable() {
			try {
				UnityEngine.SceneManagement.SceneManager.sceneLoaded -= OnSceneLoaded;
				harmony.UnpatchSelf();
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Error removing patches: " + ex.Message);
				Plugin.Log?.Debug(ex);
			}
		}
	}
}
