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
			__instance.customServerHostName.value = "battletrains.org";
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(ClientCertificateValidator), "ValidateCertificateChainInternal")]
	public class ClientCertificateValidator_ValidateCertificateChainInternal {
		public static bool Prefix() =>
			!(Plugin.networkConfig is CustomNetworkConfig);
	}

	/*[HarmonyLib.HarmonyPatch(typeof(Zenject.DiContainer), nameof(Zenject.DiContainer.RegisterProvider))]
	public class DiContainer_RegisterProvider {
		public static void Prefix(Zenject.BindingId bindingId, Zenject.BindingCondition condition, Zenject.IProvider provider, bool nonLazy) {
			Plugin.Log?.Debug($"RegisterProvider: {bindingId.Type}");
			if(bindingId.Type == typeof(MultiplayerLevelSelectionFlowCoordinator))
				Plugin.Log?.Debug(System.Environment.StackTrace);
		}
	}*/

	[HarmonyLib.HarmonyPatch(typeof(MultiplayerLevelSelectionFlowCoordinator), "enableCustomLevels", HarmonyLib.MethodType.Getter)]
	public class MultiplayerLevelSelectionFlowCoordinator_enableCustomLevels {
		public static void Postfix(ref bool __result) {
			__result |= (Plugin.networkConfig is CustomNetworkConfig);
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(LobbySetupViewController), "SetPlayersMissingLevelText")]
	public class LobbySetupViewController_SetPlayersMissingLevelText {
		public static void Prefix(LobbySetupViewController __instance, string playersMissingLevelText, ref UnityEngine.UI.Button ____startGameReadyButton) {
			if (!string.IsNullOrEmpty(playersMissingLevelText) && ____startGameReadyButton.interactable)
				__instance.SetStartGameEnabled(CannotStartGameReason.DoNotOwnSong);
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(BasicConnectionRequestHandler), nameof(BasicConnectionRequestHandler.GetConnectionMessage))]
	public class BasicConnectionRequestHandler_GetConnectionMessage {
		public static void Postfix(LiteNetLib.Utils.NetDataWriter writer) {
			Plugin.Log?.Debug("BasicConnectionRequestHandler_GetConnectionMessage()");
			LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, 16);
			sub.Put((uint)1);
			sub.Put((uint)Config.Instance.windowSize);
			sub.Put((bool)Config.Instance.directDownloads);
			writer.PutVarUInt((uint)sub.Length);
			writer.Put("BeatUpClient");
			writer.Put(sub.CopyData());
		}
	}

	[HarmonyLib.HarmonyPatch(typeof(BeatmapLevelsModel), nameof(BeatmapLevelsModel.GetLevelPreviewForLevelId))]
	public class BeatmapLevelsModel_GetLevelPreviewForLevelId {
		public static System.Func<string, Networking.PacketHandler.NetworkPreviewBeatmapLevel>? resolvePreview = null;
		public static void Postfix(ref IPreviewBeatmapLevel? __result, string levelId) {
			if(__result == null) {
				__result = resolvePreview?.Invoke(levelId);
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
		public static async System.Threading.Tasks.Task ZipData(System.IO.Compression.ZipArchive archive, string text, string name) {
			Plugin.Log?.Debug($"Zipping `{name}`");
			using System.IO.StreamWriter entry = new System.IO.StreamWriter(archive.CreateEntry(name).Open());
			await entry.WriteAsync(text);
		}
		public static async System.Threading.Tasks.Task<byte[]> ZipLevel(CustomBeatmapLevel level) {
			using(System.IO.MemoryStream memoryStream = new System.IO.MemoryStream()) {
				using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(memoryStream, System.IO.Compression.ZipArchiveMode.Create, true)) {
					await ZipFile(archive, level.songAudioClipPath, level.standardLevelInfoSaveData.songFilename);
					await ZipFile(archive, System.IO.Path.Combine(level.customLevelPath, level.standardLevelInfoSaveData.coverImageFilename), level.standardLevelInfoSaveData.coverImageFilename);
					await ZipData(archive, level.standardLevelInfoSaveData.SerializeToJSONString(), "Info.dat");
					for(int i = 0; i < level.beatmapLevelData.difficultyBeatmapSets.Count; ++i)
						for(int j = 0; j < level.beatmapLevelData.difficultyBeatmapSets[i].difficultyBeatmaps.Count; ++j)
							if(level.beatmapLevelData.difficultyBeatmapSets[i].difficultyBeatmaps[j] is CustomDifficultyBeatmap beatmap)
								await ZipData(archive, beatmap.beatmapSaveData.SerializeToJSONString(), level.standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].beatmapFilename);
				}
				memoryStream.Seek(0, System.IO.SeekOrigin.Begin);
				return memoryStream.ToArray();
			}
		}
		public static async System.Threading.Tasks.Task<EntitlementsStatus> ShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> task, string levelId) {
			Plugin.uploadData = null;
			EntitlementsStatus status = await task;
			Plugin.Log?.Debug($"EntitlementsStatus: {status}");
			if(status != EntitlementsStatus.Ok)
				return status;
			BeatmapLevelsModel.GetBeatmapLevelResult result = await LevelLoader.beatmapLevelsModel.GetBeatmapLevelAsync(levelId, default(System.Threading.CancellationToken));
			Plugin.Log?.Debug($"GetBeatmapLevelResult.isError: {result.isError}");
			if(result.isError)
				return EntitlementsStatus.NotOwned;
			if(Config.Instance.directDownloads && result.beatmapLevel is CustomBeatmapLevel level) {
				Plugin.Log?.Debug("Zipping custom level");
				Plugin.uploadData = await ZipLevel(level);
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
		public BeatUpMenuRpcManager(IMultiplayerSessionManager multiplayerSessionManager) : base(multiplayerSessionManager) {}
		void IMenuRpcManager.SetIsEntitledToLevel(string levelId, EntitlementsStatus entitlementStatus) {
			Plugin.Log?.Debug("MenuRpcManager_SetIsEntitledToLevel");
			if(entitlementStatus == EntitlementsStatus.Ok && Plugin.uploadData != null) {
				Plugin.Log?.Debug($"Announcing share for `{levelId}`");
				multiplayerSessionManager.Send(new Networking.PacketHandler.SetCanShareBeatmap(levelId, "1234", (ulong)Plugin.uploadData.Length, true)); // No public method exposes the `onlyFirstDegree` option
			}
			base.SetIsEntitledToLevel(levelId, entitlementStatus);
		}
	}

	[DiJack.ReplaceInterfacesAndSelf(typeof(MultiplayerLevelLoader))]
	public class LevelLoader : MultiplayerLevelLoader {
		public class MemorySpriteLoader : ISpriteAsyncLoader {
			System.IO.Compression.ZipArchiveEntry data;
			public MemorySpriteLoader(System.IO.Compression.ZipArchiveEntry data) {
				this.data = data;
			}
			async System.Threading.Tasks.Task<UnityEngine.Sprite> ISpriteAsyncLoader.LoadSpriteAsync(string path, System.Threading.CancellationToken cancellationToken) {
				using(System.IO.MemoryStream stream = new System.IO.MemoryStream()) {
					await data.Open().CopyToAsync(stream);
					UnityEngine.Texture2D texture = new UnityEngine.Texture2D(2, 2);
					UnityEngine.ImageConversion.LoadImage(texture, stream.ToArray());
					UnityEngine.Rect rect = new UnityEngine.Rect(0, 0, texture.width, texture.height);
					return UnityEngine.Sprite.Create(texture, rect, new UnityEngine.Vector2(0, 0), 0.1f);
				}
			}
		}
		public Networking.PacketHandler handler;
		public readonly IMenuRpcManager rpcManager;
		public BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection;
		public EnvironmentInfoSO defaultEnvironmentInfo;
		public EnvironmentInfoSO defaultAllDirectionsEnvironmentInfo;
		public EnvironmentsListSO environmentSceneInfoCollection;
		public static UnityEngine.Sprite defaultPackCover = null!;
		public static BeatmapLevelsModel beatmapLevelsModel = null!;
		public readonly IMediaAsyncLoader mediaAsyncLoader;
		public System.Func<string, Networking.PacketHandler.NetworkPreviewBeatmapLevel>? resolvePreview = null;
		public LevelLoader(Networking.PacketHandler handler, IMenuRpcManager rpcManager, BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection, CustomLevelLoader customLevelLoader, IMediaAsyncLoader mediaAsyncLoader, BeatmapLevelsModel beatmapLevelsModel) {
			this.handler = handler;
			this.rpcManager = rpcManager;
			this.beatmapCharacteristicCollection = beatmapCharacteristicCollection;
			defaultEnvironmentInfo = (EnvironmentInfoSO)typeof(CustomLevelLoader).GetField("_defaultEnvironmentInfo", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			defaultAllDirectionsEnvironmentInfo = (EnvironmentInfoSO)typeof(CustomLevelLoader).GetField("_defaultAllDirectionsEnvironmentInfo", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			environmentSceneInfoCollection = (EnvironmentsListSO)typeof(CustomLevelLoader).GetField("_environmentSceneInfoCollection", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			defaultPackCover = (UnityEngine.Sprite)typeof(CustomLevelLoader).GetField("_defaultPackCover", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).GetValue(customLevelLoader);
			LevelLoader.beatmapLevelsModel = beatmapLevelsModel;
			this.mediaAsyncLoader = mediaAsyncLoader;
		}
		public EnvironmentInfoSO LoadEnvironmentInfo(string environmentName, EnvironmentInfoSO defaultInfo) {
			EnvironmentInfoSO environmentInfoSO = environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(environmentName);
			if(environmentInfoSO == null)
				environmentInfoSO = defaultInfo;
			return environmentInfoSO;
		}
		public CustomPreviewBeatmapLevel? LoadZippedPreviewBeatmapLevel(string levelID, StandardLevelInfoSaveData standardLevelInfoSaveData, System.IO.Compression.ZipArchive archive) {
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
							difficultyBeatmapSet.difficultyBeatmaps[j].difficulty.BeatmapDifficultyFromSerializedName(out var difficulty);
							diffs[j] = difficulty;
						}
						sets.Add(new PreviewDifficultyBeatmapSet(beatmapCharacteristicBySerializedName, diffs));
					}
				}
				System.IO.Compression.ZipArchiveEntry cover = archive.GetEntry(standardLevelInfoSaveData.coverImageFilename);
				if(cover == null) {
					Plugin.Log?.Error("File not found in archive: " + standardLevelInfoSaveData.coverImageFilename);
					return null;
				}
				return new CustomPreviewBeatmapLevel(defaultPackCover, standardLevelInfoSaveData, "", new MemorySpriteLoader(cover), levelID, standardLevelInfoSaveData.songName, standardLevelInfoSaveData.songSubName, standardLevelInfoSaveData.songAuthorName, standardLevelInfoSaveData.levelAuthorName, standardLevelInfoSaveData.beatsPerMinute, standardLevelInfoSaveData.songTimeOffset, standardLevelInfoSaveData.shuffle, standardLevelInfoSaveData.shufflePeriod, standardLevelInfoSaveData.previewStartTime, standardLevelInfoSaveData.previewDuration, environmentInfo, allDirectionsEnvironmentInfo, sets.ToArray());
			} catch {
				return null;
			}
		}
		public async System.Threading.Tasks.Task<CustomBeatmapLevel?> LoadZippedBeatmapLevelAsync(CustomPreviewBeatmapLevel customPreviewBeatmapLevel, System.IO.Compression.ZipArchive archive, System.Threading.CancellationToken cancellationToken) {
			StandardLevelInfoSaveData standardLevelInfoSaveData = customPreviewBeatmapLevel.standardLevelInfoSaveData;
			string customLevelPath = customPreviewBeatmapLevel.customLevelPath;
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
					using(System.IO.StreamReader reader = new System.IO.StreamReader(file.Open(), System.Text.Encoding.Default)) {
						BeatmapSaveDataVersion3.BeatmapSaveData beatmapSaveData = BeatmapSaveDataVersion3.BeatmapSaveData.DeserializeFromJSONString(reader.ReadToEnd());
						BeatmapDataBasicInfo beatmapDataBasicInfoFromSaveData = BeatmapDataLoader.GetBeatmapDataBasicInfoFromSaveData(beatmapSaveData);
						standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].difficulty.BeatmapDifficultyFromSerializedName(out var difficulty);
						difficultyBeatmaps[j] = new CustomDifficultyBeatmap(customBeatmapLevel, difficultyBeatmapSets[i], difficulty, standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].difficultyRank, standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].noteJumpMovementSpeed, standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].noteJumpStartBeatOffset, standardLevelInfoSaveData.beatsPerMinute, beatmapSaveData, beatmapDataBasicInfoFromSaveData);
					}
				}
				difficultyBeatmapSets[i].SetCustomDifficultyBeatmaps(difficultyBeatmaps);
			}
			System.IO.Compression.ZipArchiveEntry song = archive.GetEntry(standardLevelInfoSaveData.songFilename);
			if(song == null) {
				Plugin.Log?.Error("File not found in archive: " + standardLevelInfoSaveData.songFilename);
				return null;
			}
			string clipPath = System.IO.Path.Combine(System.IO.Path.GetFullPath(CustomLevelPathHelper.baseProjectPath), "BeatUpClient_audio");
			if(!System.IO.Directory.Exists(clipPath))
				System.IO.Directory.CreateDirectory(clipPath);
			clipPath = System.IO.Path.Combine(clipPath, "song.ogg"); // TODO: support `wav` files
			Plugin.Log?.Debug("Path: " + clipPath);
			using(System.IO.FileStream tempSongFile = System.IO.File.OpenWrite(clipPath)) {
				song.Open().CopyTo(tempSongFile);
			}
			UnityEngine.AudioClip? audioClip = await mediaAsyncLoader.LoadAudioClipFromFilePathAsync(clipPath);
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
			if(!result.isError)
				return result;
			Plugin.Log?.Debug("Starting direct download");
			try {
				cancellationToken.ThrowIfCancellationRequested();
				int it = 0;
				ulong off = handler.gaps[0].start;
				IConnectedPlayer victim = handler.GetPlayerByUserId(Plugin.downloadInfo.sourcePlayers[0]);
				while(handler.gaps.Count > 0) {
					if(it >= handler.gaps.Count)
						it = 0;
					for(uint i = 64; i > 0; --i) {
						if(off >= handler.gaps[it].end) {
							if(++it >= handler.gaps.Count)
								it = 0;
							off = handler.gaps[it].start;
						}
						ushort len = (ushort)System.Math.Min(380, handler.gaps[it].end - off);
						handler.SendUnreliableToPlayer(new Networking.PacketHandler.LevelFragmentRequest(off, len), victim);
						off += 380;
					}
					await System.Threading.Tasks.Task.Delay(15, cancellationToken);
					cancellationToken.ThrowIfCancellationRequested();
				}
				Plugin.Log?.Debug($"Finished downloading {handler.buffer.Length} bytes");
				System.IO.File.WriteAllBytes("import.zip", handler.buffer);
				using(System.IO.MemoryStream stream = new System.IO.MemoryStream(handler.buffer)) {
					using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(stream, System.IO.Compression.ZipArchiveMode.Read)) {
						if(archive == null)
							return result;
						System.IO.Compression.ZipArchiveEntry infoFile = archive.GetEntry("Info.dat");
						if(infoFile == null) {
							Plugin.Log?.Error("File not found in archive: Info.dat");
							return result;
						}
						StandardLevelInfoSaveData info;
						using(System.IO.StreamReader reader = new System.IO.StreamReader(infoFile.Open(), System.Text.Encoding.Default)) {
							info = StandardLevelInfoSaveData.DeserializeFromJSONString(reader.ReadToEnd());
						}
						CustomPreviewBeatmapLevel? preview = LoadZippedPreviewBeatmapLevel(gameplaySetupData.beatmapLevel.beatmapLevel.levelID, info, archive);
						if(preview == null)
							return result;
						resolvePreview?.Invoke(gameplaySetupData.beatmapLevel.beatmapLevel.levelID)?.Init(preview);
						CustomBeatmapLevel? level = await LoadZippedBeatmapLevelAsync(preview, archive, cancellationToken);
						if(level == null)
							return result;
						Plugin.Log?.Debug($"level: {level}");
						return new BeatmapLevelsModel.GetBeatmapLevelResult(isError: false, level);
					}
				}
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Direct download error: " + ex.Message);
				Plugin.Log?.Error(ex.StackTrace);
				return result;
			}
		}
		public override void LoadLevel(ILevelGameplaySetupData gameplaySetupData, float initialStartTime) {
			bool pass = (_loaderState != MultiplayerBeatmapLoaderState.NotLoading);
			base.LoadLevel(gameplaySetupData, initialStartTime);
			Plugin.Log?.Debug($"LoadLevel({gameplaySetupData.beatmapLevel.beatmapLevel.levelID})");
			if(pass || gameplaySetupData.beatmapLevel.beatmapLevel.levelID != Plugin.downloadInfo.levelId || !Config.Instance.directDownloads)
				return;
			Plugin.Log?.Debug("DownloadWrapper()");
			_getBeatmapLevelResultTask = DownloadWrapper(_getBeatmapLevelResultTask, gameplaySetupData, _getBeatmapCancellationTokenSource.Token);
		}
		public override void Tick() { // TODO: handle download failure gracefully
			MultiplayerBeatmapLoaderState prevState = _loaderState;
			base.Tick();
			if(prevState == MultiplayerBeatmapLoaderState.LoadingBeatmap && _loaderState == MultiplayerBeatmapLoaderState.WaitingForCountdown)
				rpcManager.SetIsEntitledToLevel(_gameplaySetupData.beatmapLevel.beatmapLevel.levelID, EntitlementsStatus.Ok); // TODO: Don't send this if MpCore already did
		}
	}

	[HarmonyLib.HarmonyPatch]
	public class ConnectedPlayerManager_Send {
		public static System.Collections.Generic.IEnumerable<System.Reflection.MethodBase> TargetMethods() {
			yield return typeof(ConnectedPlayerManager).GetMethod("Send").MakeGenericMethod(typeof(LiteNetLib.Utils.INetSerializable));
		}
		public static bool Prefix(ConnectedPlayerManager __instance, LiteNetLib.Utils.INetSerializable message) {
			if(Config.Instance.unreliableState && (message is NodePoseSyncStateNetSerializable || message is StandardScoreSyncStateNetSerializable)) {
				__instance.SendUnreliable(message);
				return false;
			}
			return true;
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
		};

		public class NetworkPreviewBeatmapLevel : IPreviewBeatmapLevel, LiteNetLib.Utils.INetSerializable {
			public string levelID { get; set; } = "";
			public string songName { get; set; } = "";
			public string songSubName { get; set; } = "";
			public string songAuthorName { get; set; } = "";
			public string levelAuthorName { get; set; } = "";
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
			readonly ByteArrayNetSerializable cover = new ByteArrayNetSerializable("cover", 0, 8192);
			public System.Threading.Tasks.Task<UnityEngine.Sprite> GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) {
				UnityEngine.Sprite sprite = Patches.LevelLoader.defaultPackCover;
				if(cover.data.Length > 0) {
					UnityEngine.Texture2D texture = new UnityEngine.Texture2D(2, 2);
					UnityEngine.ImageConversion.LoadImage(texture, cover.data);
					UnityEngine.Rect rect = new UnityEngine.Rect(0, 0, texture.width, texture.height);
					sprite = UnityEngine.Sprite.Create(texture, rect, new UnityEngine.Vector2(0, 0), 0.1f);
				}
				return System.Threading.Tasks.Task.FromResult<UnityEngine.Sprite>(sprite);
			}
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.Put(levelID);
				writer.Put(songName);
				writer.Put(songSubName);
				writer.Put(songAuthorName);
				writer.Put(levelAuthorName);
				writer.Put(beatsPerMinute);
				writer.Put(songTimeOffset);
				writer.Put(shuffle);
				writer.Put(shufflePeriod);
				writer.Put(previewStartTime);
				writer.Put(previewDuration);
				writer.Put(songDuration);
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
				cover.Deserialize(reader);
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
				environmentInfo = beatmapLevel.environmentInfo;
				allDirectionsEnvironmentInfo = beatmapLevel.allDirectionsEnvironmentInfo;
				Plugin.Log?.Debug($"ASSIGN:");
				Plugin.Log?.Debug($"    beatmapLevel={beatmapLevel}");
				Plugin.Log?.Debug($"    previewDifficultyBeatmapSets={previewDifficultyBeatmapSets}");
			}
			public NetworkPreviewBeatmapLevel() {}
			public NetworkPreviewBeatmapLevel(IPreviewBeatmapLevel beatmapLevel) {
				Init(beatmapLevel);
			}
		}

		public class RecommendPreview : LiteNetLib.Utils.INetSerializable {
			public NetworkPreviewBeatmapLevel preview;
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				Plugin.Log?.Debug($"RecommendPreview.Serialize()");
				preview.Serialize(writer);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				Plugin.Log?.Debug($"RecommendPreview.Deserialize()");
				preview.Deserialize(reader);
				Plugin.Log?.Debug($"RecommendPreview.Deserialize() end");
			}
			public RecommendPreview() {
				this.preview = new NetworkPreviewBeatmapLevel();
			}
			public RecommendPreview(IPreviewBeatmapLevel beatmapLevel) {
				Plugin.Log?.Debug($"RecommendPreview.Init()");
				this.preview = new NetworkPreviewBeatmapLevel(beatmapLevel);
			}
		}

		public class SetCanShareBeatmap : LiteNetLib.Utils.INetSerializable {
			public string levelId = "";
			public string levelHash = "";
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
			public string levelId = "";
			public string levelHash = "";
			public ulong fileSize;
			public string[] sourcePlayers = new string[0];
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.Put(levelId);
				writer.Put(levelHash);
				writer.PutVarULong(fileSize);
				writer.Put((byte)sourcePlayers.Length);
				foreach(string player in sourcePlayers)
					writer.Put(player);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				Plugin.Log?.Debug("DirectDownloadInfo.Deserialize()");
				levelId = reader.GetString();
				levelHash = reader.GetString();
				fileSize = reader.GetVarULong();
				sourcePlayers = new string[reader.GetByte()];
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
				writer.Put((ushort)data.Length);
				writer.Put(data);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				offset = reader.GetVarULong();
				data = new byte[reader.GetUShort()];
				reader.GetBytes(data, data.Length);
			}
			public LevelFragment() {}
			public LevelFragment(ulong offset, byte[] data) {
				this.offset = offset;
				this.data = data;
			}
		}

		public readonly System.Reflection.FieldInfo fi_ConnectedPlayer_connection;
		public readonly System.Reflection.FieldInfo fi_ConnectedPlayer_connectionId;
		public readonly System.Reflection.FieldInfo fi_ConnectedPlayer_remoteConnectionId;
		public readonly System.Reflection.MethodInfo mi_ConnectedPlayerManager_WriteOne;
		public readonly MultiplayerSessionManager multiplayerSessionManager;
		public readonly MultiplayerSessionManager.MessageType messageType = (MultiplayerSessionManager.MessageType)101;
		public readonly NetworkPacketSerializer<MessageType, IConnectedPlayer> serializer = new NetworkPacketSerializer<MessageType, IConnectedPlayer>();

		public PacketHandler(IMultiplayerSessionManager multiplayerSessionManager, INetworkConfig networkConfig) {
			Plugin.Log?.Debug("PacketHandler()");
			System.Type ConnectedPlayer = typeof(ConnectedPlayerManager).Assembly.GetType("ConnectedPlayerManager+ConnectedPlayer");
			fi_ConnectedPlayer_connection = ConnectedPlayer.GetField("_connection", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			fi_ConnectedPlayer_connectionId = ConnectedPlayer.GetField("_connectionId", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			fi_ConnectedPlayer_remoteConnectionId = ConnectedPlayer.GetField("_remoteConnectionId", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			mi_ConnectedPlayerManager_WriteOne = typeof(ConnectedPlayerManager).GetMethod("WriteOne", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic);
			this.multiplayerSessionManager = (MultiplayerSessionManager)multiplayerSessionManager;
			Plugin.networkConfig = networkConfig;
		}

		public void Initialize() {
			if(!Config.Instance.directDownloads)
				return;
			Plugin.Log?.Debug("multiplayerSessionManager.RegisterSerializer()");
			multiplayerSessionManager.RegisterSerializer(messageType, serializer);
			serializer.RegisterCallback<RecommendPreview>(MessageType.RecommendPreview, HandleRecommendPreview);
			serializer.RegisterCallback<SetCanShareBeatmap>(MessageType.SetCanShareBeatmap, HandleSetCanShareBeatmap);
			serializer.RegisterCallback<DirectDownloadInfo>(MessageType.DirectDownloadInfo, HandleDirectDownloadInfo);
			serializer.RegisterCallback<LevelFragmentRequest>(MessageType.LevelFragmentRequest, HandleLevelFragmentRequest);
			serializer.RegisterCallback<LevelFragment>(MessageType.LevelFragment, HandleLevelFragment);
		}

		public void Dispose() {
			if(Config.Instance.directDownloads)
				multiplayerSessionManager.UnregisterSerializer(messageType, serializer);
		}

		public IConnectedPlayer GetPlayerByUserId(string userId) {
			return multiplayerSessionManager.GetPlayerByUserId(userId);
		}

		public void Send<T>(T message) where T : LiteNetLib.Utils.INetSerializable {
			multiplayerSessionManager.Send(message);
		}

		public void SendUnreliableToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable {
			// multiplayerSessionManager.SendUnreliableToPlayer(message, player);
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
			Plugin.downloadInfo = packet;
			buffer = new byte[packet.fileSize];
			gaps.Clear();
			gaps.Add((0, packet.fileSize));
		}

		public void HandleLevelFragmentRequest(LevelFragmentRequest packet, IConnectedPlayer player) {
			if(Plugin.uploadData == null || packet.offset >= (ulong)Plugin.uploadData.Length)
				return;
			byte[] data = new byte[System.Math.Min(packet.maxSize, (ulong)Plugin.uploadData.Length - packet.offset)];
			System.Buffer.BlockCopy(Plugin.uploadData, (int)packet.offset, data, 0, data.Length);
			SendUnreliableToPlayer(new LevelFragment(packet.offset, data), player);
		}

		public void HandleLevelFragment(LevelFragment packet, IConnectedPlayer player) {
			System.Buffer.BlockCopy(packet.data, 0, buffer, (int)packet.offset, packet.data.Length);
			ulong start = packet.offset, end = packet.offset + (ulong)packet.data.Length;
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
	}

	[DiJack.Replace(typeof(LobbyPlayersDataModel))]
	public class PlayersDataModel : LobbyPlayersDataModel, ILobbyPlayersDataModel, System.IDisposable {
		public readonly PacketHandler handler;
		public readonly Patches.LevelLoader loader;
		public System.Collections.Generic.Dictionary<IConnectedPlayer, PacketHandler.NetworkPreviewBeatmapLevel> playerPreviews = new System.Collections.Generic.Dictionary<IConnectedPlayer, PacketHandler.NetworkPreviewBeatmapLevel>();

		public PlayersDataModel(PacketHandler handler, Patches.LevelLoader loader) {
			this.handler = handler;
			this.loader = loader;
		}

		public override void Activate() {
			handler.recommendPreviewEvent += HandleRecommendPreview;
			Patches.BeatmapLevelsModel_GetLevelPreviewForLevelId.resolvePreview += ResolvePreview;
			loader.resolvePreview += ResolvePreview;
			base.Activate();
			_menuRpcManager.getRecommendedBeatmapEvent -= base.HandleMenuRpcManagerGetRecommendedBeatmap;
			_menuRpcManager.getRecommendedBeatmapEvent += this.HandleMenuRpcManagerGetRecommendedBeatmap;
		}

		public override void Deactivate() {
			handler.recommendPreviewEvent -= HandleRecommendPreview;
			Patches.BeatmapLevelsModel_GetLevelPreviewForLevelId.resolvePreview -= ResolvePreview;
			loader.resolvePreview -= ResolvePreview;
			_menuRpcManager.getRecommendedBeatmapEvent -= this.HandleMenuRpcManagerGetRecommendedBeatmap;
			_menuRpcManager.getRecommendedBeatmapEvent += base.HandleMenuRpcManagerGetRecommendedBeatmap;
			base.Deactivate();
		}

		public override void Dispose() {
			Deactivate();
		}

		private void HandleRecommendPreview(PacketHandler.RecommendPreview packet, IConnectedPlayer player) {
			Plugin.Log?.Debug($"PlayersDataModel.HandleRecommendPreview(\"{packet.preview.levelID}\", {player})");
			playerPreviews[player] = packet.preview;
		}

		public PacketHandler.NetworkPreviewBeatmapLevel ResolvePreview(string levelId) {
			return System.Linq.Enumerable.FirstOrDefault(playerPreviews.Values, (PacketHandler.NetworkPreviewBeatmapLevel beatmapLevel) => beatmapLevel.levelID == levelId);
		}

		public override void HandleMenuRpcManagerGetRecommendedBeatmap(string userId) {
			handler.Send(new PacketHandler.RecommendPreview(this[localUserId].beatmapLevel.beatmapLevel));
			base.HandleMenuRpcManagerGetRecommendedBeatmap(userId);
		}

		public override void SetLocalPlayerBeatmapLevel(PreviewDifficultyBeatmap beatmapLevel) {
			handler.Send(new PacketHandler.RecommendPreview(beatmapLevel.beatmapLevel));
			base.SetLocalPlayerBeatmapLevel(beatmapLevel);
		}
	}
}

namespace BeatUpClient {
	public class Config {
		public static Config Instance { get; set; } = new Config();
		public ushort windowSize { get; set; } = 64;
		public bool unreliableState { get; set; } = false;
		public bool directDownloads { get; set; } = false;
	}

	[IPA.Plugin(IPA.RuntimeOptions.SingleStartInit)]
	public class Plugin {
		public const string HarmonyId = "org.battletrains.BeatUpClient";
		public static readonly HarmonyLib.Harmony harmony = new HarmonyLib.Harmony(HarmonyId);
		public static Plugin? Instance;
		public static IPA.Logging.Logger? Log;
		public static Networking.PacketHandler.DirectDownloadInfo downloadInfo = new Networking.PacketHandler.DirectDownloadInfo();
		public static byte[]? uploadData = null;
		public static INetworkConfig? networkConfig;

		static System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> ReliableChannel_ctor(System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> instructions) {
			bool notFound = true;
			foreach(var instruction in instructions) {
				if(notFound && HarmonyLib.CodeInstructionExtensions.LoadsConstant(instruction, 64)) {
					yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Ldc_I4, (int)Config.Instance.windowSize);
					notFound = false;
				} else {
					yield return instruction;
				}
			}
			if(notFound) {
				Plugin.Log?.Error("Failed to override reliable window size");
			} else {
				Plugin.Log?.Info("Overriding reliable window size");
			}
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
			try {
				Plugin.Log?.Debug("Applying patches.");
				DiJack.ResolveInjections(System.Reflection.Assembly.GetExecutingAssembly());
				harmony.PatchAll(System.Reflection.Assembly.GetExecutingAssembly());

				if(Config.Instance.windowSize != 64) {
					System.Reflection.MethodBase original = typeof(LiteNetLib.NetManager).Assembly.GetType("LiteNetLib.ReliableChannel").GetConstructor(new[] {typeof(LiteNetLib.NetPeer), typeof(bool), typeof(byte)});
					System.Reflection.MethodInfo transpiler = typeof(Plugin).GetMethod("ReliableChannel_ctor");
					harmony.Patch(original, transpiler: new HarmonyLib.HarmonyMethod(transpiler));
				}
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Error applying patches: " + ex.Message);
				Plugin.Log?.Debug(ex);
				OnDisable();
			}
		}
		[IPA.OnDisable]
		public void OnDisable() {
			try {
				harmony.UnpatchSelf();
			} catch(System.Exception ex) {
				Plugin.Log?.Error("Error removing patches: " + ex.Message);
				Plugin.Log?.Debug(ex);
			}
		}
	}
}
