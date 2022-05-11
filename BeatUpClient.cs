using static BeatUpClient;

[IPA.Plugin(IPA.RuntimeOptions.SingleStartInit)]
public class BeatUpClient {
	public static class Reflection {
		public struct FieldProxy<T> {
			System.Reflection.FieldInfo field;
			public object instance;
			public T value {
				get => (T)field.GetValue(instance);
			}
			public FieldProxy(object instance, System.Reflection.FieldInfo field) {
				this.instance = instance;
				this.field = field;
			}
			public void Set(T value) =>
				field.SetValue(instance, value);
			public static implicit operator T(FieldProxy<T> self) =>
				self.value;
		}
		public static FieldProxy<T> Field<T>(object instance, string field) =>
			new FieldProxy<T>(instance, HarmonyLib.AccessTools.Field(instance.GetType(), field));
		// Inline IL considered harmful; prefer runtime codegen (http://www.simplygoodcode.com/2012/08/invoke-base-method-using-reflection/)
		static void DirectInvoke(System.Reflection.MethodInfo methodInfo, object instance, params object?[] arguments) {
			var parameters = methodInfo.GetParameters();
			if(parameters.Length == 0) {
				if(arguments != null && arguments.Length != 0) 
					throw new System.ArgumentException("Arguments cont doesn't match");
			} else {
				if(parameters.Length != arguments.Length)
					throw new System.ArgumentException("Arguments cont doesn't match");
			}
			System.Type type = instance.GetType();
			System.Reflection.Emit.DynamicMethod dynamicMethod = new System.Reflection.Emit.DynamicMethod("", null, new[] {type, typeof(object)}, type);
			System.Reflection.Emit.ILGenerator iLGenerator = dynamicMethod.GetILGenerator();
			iLGenerator.Emit(System.Reflection.Emit.OpCodes.Ldarg_0);
			for (var i = 0; i < parameters.Length; i++) {
				var parameter = parameters[i];
				iLGenerator.Emit(System.Reflection.Emit.OpCodes.Ldarg_1);
				iLGenerator.Emit(System.Reflection.Emit.OpCodes.Ldc_I4_S, i);
				iLGenerator.Emit(System.Reflection.Emit.OpCodes.Ldelem_Ref);
				var parameterType = parameter.ParameterType;
				if(parameterType.IsPrimitive) {
					iLGenerator.Emit(System.Reflection.Emit.OpCodes.Unbox_Any, parameterType);
				} else if(parameterType != typeof(object)) {
					iLGenerator.Emit(System.Reflection.Emit.OpCodes.Castclass, parameterType);
				}
			}
			iLGenerator.Emit(System.Reflection.Emit.OpCodes.Call, methodInfo);
			iLGenerator.Emit(System.Reflection.Emit.OpCodes.Ret);
			dynamicMethod.Invoke(null, new[] {instance, arguments});
		}
		public static System.Action<T> BaseBaseMethod<T>(object instance, string method) {
			System.Reflection.MethodInfo info = instance.GetType().BaseType.BaseType.GetMethod(method);
			return (T arg1) => DirectInvoke(info, instance, arg1);
		}
		public static System.Action<T1, T2> BaseBaseMethod<T1, T2>(object instance, string method) {
			System.Reflection.MethodInfo info = instance.GetType().BaseType.BaseType.GetMethod(method);
			return (T1 arg1, T2 arg2) => DirectInvoke(info, instance, arg1, arg2);
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
		[IPA.Config.Stores.Attributes.NonNullable]
		[IPA.Config.Stores.Attributes.UseConverter(typeof(IPA.Config.Stores.Converters.DictionaryConverter<string>))]
		public virtual System.Collections.Generic.Dictionary<string, string?> Servers { get; set; } = new System.Collections.Generic.Dictionary<string, string?>(new[] {new System.Collections.Generic.KeyValuePair<string, string?>("master.battletrains.org", null)});
		public virtual void Changed() {}
	}

	public enum PatchType : byte {
		Prefix,
		Postfix,
		Transpiler,
	}

	[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
	public class PatchAttribute : System.Attribute {
		System.Reflection.MethodBase? method;
		PatchType patchType;
		public PatchAttribute(PatchType patchType, System.Reflection.MethodBase? method, System.Type? genericType) {
			this.method = (method == null || genericType == null) ? method : ((System.Reflection.MethodInfo)method).MakeGenericMethod(genericType);
			this.patchType = patchType;
		}
		public PatchAttribute(PatchType patchType, System.Type type, string fn, System.Type? genericType = null) : this(patchType, fn == ".ctor" ? (System.Reflection.MethodBase)type.GetConstructors()[0] : HarmonyLib.AccessTools.Method(type, fn), genericType) {}
		public PatchAttribute(PatchType patchType, System.Type refType, string type, string fn) : this(patchType, refType.Assembly.GetType(type), fn) {}
		public void Apply(System.Reflection.MethodInfo self) {
			if(method == null)
				throw new System.ArgumentException($"Missing original method for `{self}`");
			// Log?.Debug($"Patching {method.DeclaringType.FullName} : {method}");
			HarmonyLib.HarmonyMethod hm = new HarmonyLib.HarmonyMethod(self);
			harmony.Patch(method, prefix: patchType == PatchType.Prefix ? hm : null, postfix: patchType == PatchType.Postfix ? hm : null, transpiler: patchType == PatchType.Transpiler ? hm : null);
		}
	}

	[System.AttributeUsage(System.AttributeTargets.Method)]
	public class PatchOverloadAttribute : PatchAttribute {
		public PatchOverloadAttribute(PatchType patchType, System.Type type, string fn, params System.Type[] args) : base(patchType, HarmonyLib.AccessTools.Method(type, fn, args), null) {}
	}

	public static void PatchAll(System.Type type) {
		foreach(System.Reflection.MethodInfo method in type.GetMethods())
			foreach(System.Attribute attrib in method.GetCustomAttributes(false))
				if(attrib is PatchAttribute patch)
					patch.Apply(method);
	}

	public static class NullableStringHelper {
		public static bool IsNullOrEmpty([System.Diagnostics.CodeAnalysis.NotNullWhen(false)] string? str) =>
			string.IsNullOrEmpty(str);
	}

	public class MemorySpriteLoader : ISpriteAsyncLoader {
		System.Threading.Tasks.Task<byte[]> dataTask;
		public MemorySpriteLoader(System.Threading.Tasks.Task<byte[]> dataTask) =>
			this.dataTask = dataTask;
		public async System.Threading.Tasks.Task<UnityEngine.Sprite?> LoadSpriteAsync(string path, System.Threading.CancellationToken cancellationToken) {
			byte[] data = await dataTask;
			if(data.Length < 1) {
				Log?.Debug("Returning default sprite");
				return null;
			}
			Log?.Debug("Decoding sprite");
			UnityEngine.Texture2D texture = new UnityEngine.Texture2D(2, 2);
			UnityEngine.ImageConversion.LoadImage(texture, data);
			UnityEngine.Rect rect = new UnityEngine.Rect(0, 0, texture.width, texture.height);
			return UnityEngine.Sprite.Create(texture, rect, new UnityEngine.Vector2(0, 0), 0.1f);
		}
	}

	public static PacketHandler.RecommendPreview[] playerPreviews = new PacketHandler.RecommendPreview[0];
	static void InitPreviews(int playerCount) {
		playerPreviews = new PacketHandler.RecommendPreview[playerCount];
		System.Array.Fill(playerPreviews, new PacketHandler.RecommendPreview());
	}
	static PacketHandler.RecommendPreview? ResolvePreview(string levelId) =>
		System.Linq.Enumerable.FirstOrDefault(playerPreviews, (PacketHandler.RecommendPreview preview) => preview.preview.levelID == levelId);
	static PacketHandler.RecommendPreview SetPreviewFromLocal(int index, CustomPreviewBeatmapLevel previewBeatmapLevel) {
		System.Collections.Generic.IEnumerable<string?> requirements = new string[0], suggestions = new string[0];
		string path = System.IO.Path.Combine(previewBeatmapLevel.customLevelPath, "Info.dat");
		if(haveSongCore) {
			BeatUpClient_SongCore.GetPreviewInfo(previewBeatmapLevel, ref requirements, ref suggestions);
		} else if(System.IO.File.Exists(path)) {
			Newtonsoft.Json.Linq.JObject info = Newtonsoft.Json.Linq.JObject.Parse(System.IO.File.ReadAllText(path));
			Newtonsoft.Json.Linq.IJEnumerable<Newtonsoft.Json.Linq.JToken> customData = Newtonsoft.Json.Linq.Extensions.Children(info["_difficultyBeatmapSets"]?.Children()["_difficultyBeatmaps"] ?? new Newtonsoft.Json.Linq.JArray())["_customData"];
			requirements = Newtonsoft.Json.Linq.Extensions.Values<string>(customData["_requirements"]);
			suggestions = Newtonsoft.Json.Linq.Extensions.Values<string>(customData["_suggestions"]);
		}
		string?[] requirementArray = System.Linq.Enumerable.ToArray(System.Linq.Enumerable.ToHashSet(requirements));
		string?[] suggestionArray = System.Linq.Enumerable.ToArray(System.Linq.Enumerable.ToHashSet(suggestions));
		if(requirementArray.Length >= 1) {
			Log?.Debug($"{requirementArray.Length} requirements for `{previewBeatmapLevel.levelID}`:");
			foreach(string? req in requirementArray)
				Log?.Debug($"    {req}");
		} else {
			Log?.Debug($"No requirements for `{previewBeatmapLevel.levelID}`");
		}
		return (playerPreviews[index] = new PacketHandler.RecommendPreview(previewBeatmapLevel, requirementArray, suggestionArray));
	}

	public class PacketHandler : System.IDisposable {
		public enum MessageType : byte {
			RecommendPreview, // client -> client
			SetCanShareBeatmap, // client -> server
			DirectDownloadInfo, // server -> client
			LevelFragmentRequest, // client -> client (unreliable)
			LevelFragment, // client -> client (unreliable)
			LoadProgress, // client -> client (unreliable)
		};

		public class NetworkPreviewBeatmapLevel : IPreviewBeatmapLevel, LiteNetLib.Utils.INetSerializable {
			public string? levelID { get; set; } = System.String.Empty;
			public string? songName { get; set; } = System.String.Empty;
			public string? songSubName { get; set; } = System.String.Empty;
			public string? songAuthorName { get; set; } = System.String.Empty;
			public string? levelAuthorName { get; set; } = System.String.Empty;
			public float beatsPerMinute { get; set; }
			public float songTimeOffset { get; set; }
			public float shuffle { get; set; }
			public float shufflePeriod { get; set; }
			public float previewStartTime { get; set; }
			public float previewDuration { get; set; }
			public float songDuration { get; set; }
			public System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet>? previewDifficultyBeatmapSets { get; set; } = null;
			public EnvironmentInfoSO? environmentInfo { get; set; } = null;
			public EnvironmentInfoSO? allDirectionsEnvironmentInfo { get; set; } = null;
			public readonly ByteArrayNetSerializable cover = new ByteArrayNetSerializable("cover", 0, 8192);
			public System.Threading.Tasks.Task<byte[]> coverRenderTask = System.Threading.Tasks.Task.FromResult<byte[]>(new byte[0]);
			public async System.Threading.Tasks.Task<UnityEngine.Sprite> GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) {
				Log?.Debug("NetworkPreviewBeatmapLevel.GetCoverImageAsync()");
				return await new MemorySpriteLoader(coverRenderTask).LoadSpriteAsync("", cancellationToken) ?? defaultPackCover;
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
				writer.Put((string?)levelID);
				writer.Put((string?)songName);
				writer.Put((string?)songSubName);
				writer.Put((string?)songAuthorName);
				writer.Put((string?)levelAuthorName);
				writer.Put((float)beatsPerMinute);
				writer.Put((float)songTimeOffset);
				writer.Put((float)shuffle);
				writer.Put((float)shufflePeriod);
				writer.Put((float)previewStartTime);
				writer.Put((float)previewDuration);
				writer.Put((float)songDuration);
				writer.Put((byte)UpperBound((uint)(previewDifficultyBeatmapSets?.Count ?? 0), 8));
				foreach(PreviewDifficultyBeatmapSet previewDifficultyBeatmapSet in previewDifficultyBeatmapSets ?? new PreviewDifficultyBeatmapSet[0]) {
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
				uint count = UpperBound(reader.GetByte(), 8);
				PreviewDifficultyBeatmapSet[] sets = count >= 1 ? new PreviewDifficultyBeatmapSet[count] : null!;
				for(uint i = 0; i < count; ++i) {
					BeatmapCharacteristicSO? beatmapCharacteristic = handler.beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(reader.GetString());
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
				Log?.Debug($"NetworkPreviewBeatmapLevel.Init(beatmapLevel={beatmapLevel}, previewDifficultyBeatmapSets={previewDifficultyBeatmapSets})");
			}
			public NetworkPreviewBeatmapLevel() {}
			public NetworkPreviewBeatmapLevel(IPreviewBeatmapLevel beatmapLevel) =>
				Init(beatmapLevel);
		}

		public class RecommendPreview : LiteNetLib.Utils.INetSerializable {
			public NetworkPreviewBeatmapLevel preview = new NetworkPreviewBeatmapLevel();
			public string?[] requirements = new string[0];
			public string?[] suggestions = new string[0];
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				preview.Serialize(writer);
				writer.PutVarUInt(UpperBound((uint)requirements.Length, 16));
				foreach(string? requirement in requirements)
					writer.Put((string?)requirement);
				writer.PutVarUInt(UpperBound((uint)suggestions.Length, 16));
				foreach(string? suggestion in suggestions)
					writer.Put((string?)suggestion);
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
			public RecommendPreview(IPreviewBeatmapLevel beatmapLevel, string?[] requirements, string?[] suggestions) {
				this.preview = new NetworkPreviewBeatmapLevel(beatmapLevel);
				this.requirements = requirements;
				this.suggestions = suggestions;
			}
		}

		public abstract class ShareInfo : LiteNetLib.Utils.INetSerializable {
			public string levelId = System.String.Empty;
			public byte[] levelHash = new byte[32];
			public ulong fileSize;
			public virtual void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				Log?.Debug($"ShareInfo.Serialize()");
				writer.Put((string)levelId);
				writer.Put((byte[])levelHash);
				writer.PutVarULong(fileSize);
			}
			public virtual void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				Log?.Debug($"ShareInfo.Deserialize()");
				levelId = reader.GetString();
				reader.GetBytes(levelHash, levelHash.Length);
				fileSize = reader.GetVarULong();
			}
			public ShareInfo() {}
			public ShareInfo(string levelId, byte[] levelHash, ulong fileSize) {
				Log?.Debug($"ShareInfo()");
				this.levelId = levelId;
				System.Buffer.BlockCopy(levelHash, 0, this.levelHash, 0, this.levelHash.Length);
				this.fileSize = fileSize;
			}
		}

		public class SetCanShareBeatmap : ShareInfo {
			public bool canShare;
			public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				Log?.Debug($"SetCanShareBeatmap.Serialize()");
				base.Serialize(writer);
				writer.Put((bool)canShare);
			}
			public override void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				base.Deserialize(reader);
				canShare = reader.GetBool();
			}
			public SetCanShareBeatmap() {}
			public SetCanShareBeatmap(string levelId, byte[] levelHash, ulong fileSize, bool canShare = true) : base(levelId, levelHash, fileSize) {
				Log?.Debug($"SetCanShareBeatmap()");
				this.canShare = canShare;
			}
		}

		public class DirectDownloadInfo : ShareInfo {
			public string[] sourcePlayers = new string[0];
			public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				base.Serialize(writer);
				writer.Put((byte)UpperBound((uint)sourcePlayers.Length, 128));
				foreach(string player in sourcePlayers)
					writer.Put(player);
			}
			public override void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				Log?.Debug("DirectDownloadInfo.Deserialize()");
				base.Deserialize(reader);
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

		public Zenject.DiContainer container;
		public IMenuRpcManager rpcManager;
		public BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection;
		public MultiplayerSessionManager multiplayerSessionManager;
		const MultiplayerSessionManager.MessageType messageType = (MultiplayerSessionManager.MessageType)101;
		public NetworkPacketSerializer<MessageType, IConnectedPlayer> serializer;
		public Downloader? downloader = null;

		public PacketHandler(Zenject.DiContainer container) {
			Log?.Debug("PacketHandler()");
			this.container = container;
			rpcManager = container.TryResolve<IMenuRpcManager>();
			beatmapCharacteristicCollection = container.TryResolve<BeatmapCharacteristicCollectionSO>();
			multiplayerSessionManager = (MultiplayerSessionManager)container.TryResolve<IMultiplayerSessionManager>();
			serializer = new NetworkPacketSerializer<MessageType, IConnectedPlayer>();

			multiplayerSessionManager.SetLocalPlayerState("modded", true);
			multiplayerSessionManager.RegisterSerializer(messageType, serializer);
			serializer.RegisterCallback<RecommendPreview>(MessageType.RecommendPreview, HandleRecommendPreview);
			serializer.RegisterCallback<SetCanShareBeatmap>(MessageType.SetCanShareBeatmap, HandleSetCanShareBeatmap);
			serializer.RegisterCallback<DirectDownloadInfo>(MessageType.DirectDownloadInfo, HandleDirectDownloadInfo);
			serializer.RegisterCallback<LevelFragmentRequest>(MessageType.LevelFragmentRequest, HandleLevelFragmentRequest);
			serializer.RegisterCallback<LevelFragment>(MessageType.LevelFragment, HandleLevelFragment);
			serializer.RegisterCallback<LoadProgress>(MessageType.LoadProgress, HandleLoadProgress);
			rpcManager.setSelectedBeatmapEvent += HandleSetSelectedBeatmapEvent;
			rpcManager.clearSelectedBeatmapEvent += HandleClearSelectedBeatmapEvent;
			rpcManager.setIsEntitledToLevelEvent += HandleSetIsEntitledToLevelRpc;

			container.Bind(typeof(System.IDisposable)).To<PacketHandler>().FromInstance(this);
		}

		void System.IDisposable.Dispose() {
			Log?.Debug("PacketHandler.Dispose()");
			rpcManager.setIsEntitledToLevelEvent -= HandleSetIsEntitledToLevelRpc;
			rpcManager.clearSelectedBeatmapEvent -= HandleClearSelectedBeatmapEvent;
			rpcManager.setSelectedBeatmapEvent -= HandleSetSelectedBeatmapEvent;
			multiplayerSessionManager.UnregisterSerializer(messageType, serializer);
		}

		public static uint UpperBound(uint value, uint limit) {
			if(value > limit)
				throw new System.ArgumentOutOfRangeException();
			return value;
		}

		public static bool MissingRequirements(RecommendPreview? preview, bool download) {
			if(preview == null)
				return false; // Entitlement[good]: no preview
			if(preview.requirements.Length + preview.suggestions.Length >= 1 && download && !Config.Instance.AllowModchartDownloads)
				return true; // Entitlement[fail]: blocked by `AllowModchartDownloads`
			if(preview.requirements.Length < 1)
				return false; // Entitlement[good]: no requirements
			if(!haveSongCore)
				return true; // Entitlement[fail]: need SongCore
			if(!System.Linq.Enumerable.All(preview.requirements, x => NullableStringHelper.IsNullOrEmpty(x) || SongCore.Collections.capabilities.Contains(x)))
				return true; // Entitlement[fail]: missing requirements
			return false; // Entitlement[good]: have all requirements
		}

		static string? selectedLevelId = null;
		static void HandleSetSelectedBeatmapEvent(string userId, BeatmapIdentifierNetSerializable? beatmapId) {
			selectedLevelId = beatmapId?.levelID;
			for(uint i = 0; i < playerCells.Length; ++i)
				playerCells[i].UpdateData(new LoadProgress(LoadProgress.LoadState.None, 0, 0), true);
		}

		static void HandleClearSelectedBeatmapEvent(string userId) =>
			HandleSetSelectedBeatmapEvent(userId, null);

		void HandleSetIsEntitledToLevelRpc(string userId, string levelId, EntitlementsStatus entitlementStatus) =>
			HandleSetIsEntitledToLevel(multiplayerSessionManager.GetPlayerByUserId(userId), levelId, entitlementStatus);

		public static void HandleSetIsEntitledToLevel(IConnectedPlayer? player, string levelId, EntitlementsStatus entitlementStatus) {
			if(player == null || levelId != selectedLevelId)
				return;
			LoadProgress.LoadState state = entitlementStatus switch {
				EntitlementsStatus.NotOwned => LoadProgress.LoadState.Failed,
				EntitlementsStatus.NotDownloaded => LoadProgress.LoadState.Downloading,
				EntitlementsStatus.Ok => LoadProgress.LoadState.Done,
				_ => LoadProgress.LoadState.None,
			};
			if(state != LoadProgress.LoadState.None)
				playerCells[player.sortIndex].UpdateData(new LoadProgress(state, 0, 0), true);
		}

		public void SendUnreliableToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable {
			ConnectedPlayerManager? connectedPlayerManager = multiplayerSessionManager.connectedPlayerManager;
			if(connectedPlayerManager?.isConnected == true && player is ConnectedPlayerManager.ConnectedPlayer connectedPlayer)
				connectedPlayer._connection.Send(connectedPlayerManager.WriteOne(((ConnectedPlayerManager.ConnectedPlayer)connectedPlayerManager.localPlayer)._connectionId, connectedPlayer._remoteConnectionId, message), LiteNetLib.DeliveryMethod.Unreliable);
			else if(message is IPoolablePacket poolable)
				poolable.Release();
		}

		public static void HandleRecommendPreview(RecommendPreview packet, IConnectedPlayer player) {
			Log?.Debug($"HandleRecommendPreview(\"{packet.preview.levelID}\", {player})");
			playerPreviews[player.sortIndex] = packet;
		}

		static void HandleSetCanShareBeatmap(SetCanShareBeatmap packet, IConnectedPlayer player) {}

		void HandleDirectDownloadInfo(DirectDownloadInfo packet, IConnectedPlayer player) {
			Log?.Debug($"DirectDownloadInfo:\n    levelId=\"{packet.levelId}\"\n    levelHash=\"{packet.levelHash}\"\n    fileSize={packet.fileSize}\n    source=\"{packet.sourcePlayers[0]}\"");
			RecommendPreview? preview = ResolvePreview(packet.levelId);
			if(!directDownloads || preview == null || MissingRequirements(preview, true) || packet.fileSize > MaxDownloadSize)
				return;
			if(downloadPending) {
				LoadProgress progress = new LoadProgress(LoadProgress.LoadState.Downloading, 0);
				HandleLoadProgress(progress, multiplayerSessionManager.localPlayer);
				multiplayerSessionManager.Send(progress);
			}
			downloader = new Downloader(packet, preview);
		}

		void HandleLevelFragmentRequest(LevelFragmentRequest packet, IConnectedPlayer player) {
			if(packet.offset >= (ulong)uploadData.Count)
				return;
			byte[] data = new byte[System.Math.Min(packet.maxSize, (ulong)uploadData.Count - packet.offset)];
			System.Buffer.BlockCopy(uploadData.Array, uploadData.Offset + (int)packet.offset, data, 0, data.Length);
			SendUnreliableToPlayer(new LevelFragment(packet.offset, data), player);
		}

		void HandleLevelFragment(LevelFragment packet, IConnectedPlayer player) =>
			downloader?.HandleFragment(packet, player);

		public static void HandleLoadProgress(LoadProgress packet, IConnectedPlayer player) =>
			playerCells[player.sortIndex].UpdateData(packet);
	}
	static PacketHandler handler = null!;

	public class Downloader {
		readonly CustomLevelLoader customLevelLoader;
		readonly BeatmapCharacteristicCollectionSO beatmapCharacteristicCollection;
		readonly string dataPath = System.IO.Path.Combine(System.IO.Path.GetFullPath(CustomLevelPathHelper.baseProjectPath), "BeatUpClient_Data");
		readonly byte[] buffer;
		readonly System.Collections.Generic.List<(ulong start, ulong end)> gaps;
		readonly byte[] levelHash;
		readonly string[] sourcePlayers;
		readonly PacketHandler.RecommendPreview preview;
		System.Collections.Generic.List<IConnectedPlayer>? sources;
		public string? levelId => preview.preview.levelID;
		public Downloader(PacketHandler.DirectDownloadInfo info, PacketHandler.RecommendPreview preview) {
			customLevelLoader = handler.container.Resolve<CustomLevelLoader>();
			beatmapCharacteristicCollection = handler.container.Resolve<BeatmapCharacteristicCollectionSO>();
			buffer = new byte[info.fileSize];
			gaps = new System.Collections.Generic.List<(ulong, ulong)>(new[] {(0LU, info.fileSize)});
			this.levelHash = info.levelHash;
			this.sourcePlayers = info.sourcePlayers;
			this.preview = preview;
		}
		bool ValidatedPath(string filename, out string path) {
			path = System.IO.Path.Combine(dataPath, filename);
			if(System.IO.Path.GetDirectoryName(path) != dataPath)
				Log?.Error($"PATH MISMATCH: `{System.IO.Path.GetDirectoryName(path)}` != `{dataPath}`");
			else if(System.IO.Path.GetFileName(path) != filename)
				Log?.Error($"FILENAME MISMATCH: `{System.IO.Path.GetFileName(path)}` != `{filename}`");
			else
				return true;
			path = System.String.Empty;
			return false;
		}
		EnvironmentInfoSO LoadEnvironmentInfo(string environmentName, EnvironmentInfoSO defaultInfo) {
			EnvironmentInfoSO environmentInfoSO = customLevelLoader._environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(environmentName);
			if(environmentInfoSO == null)
				environmentInfoSO = defaultInfo;
			return environmentInfoSO;
		}
		CustomPreviewBeatmapLevel? LoadZippedPreviewBeatmapLevel(StandardLevelInfoSaveData standardLevelInfoSaveData, System.IO.Compression.ZipArchive archive) {
			try {
				EnvironmentInfoSO environmentInfo = LoadEnvironmentInfo(standardLevelInfoSaveData.environmentName, customLevelLoader._defaultEnvironmentInfo);
				EnvironmentInfoSO allDirectionsEnvironmentInfo = LoadEnvironmentInfo(standardLevelInfoSaveData.allDirectionsEnvironmentName, customLevelLoader._defaultAllDirectionsEnvironmentInfo);
				System.Collections.Generic.List<PreviewDifficultyBeatmapSet> sets = new System.Collections.Generic.List<PreviewDifficultyBeatmapSet>();
				StandardLevelInfoSaveData.DifficultyBeatmapSet[] difficultyBeatmapSets = standardLevelInfoSaveData.difficultyBeatmapSets;
				foreach(StandardLevelInfoSaveData.DifficultyBeatmapSet difficultyBeatmapSet in difficultyBeatmapSets) {
					BeatmapCharacteristicSO? beatmapCharacteristicBySerializedName = beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(difficultyBeatmapSet.beatmapCharacteristicName);
					if(beatmapCharacteristicBySerializedName != null) {
						BeatmapDifficulty[] diffs = new BeatmapDifficulty[difficultyBeatmapSet.difficultyBeatmaps.Length];
						for(int j = 0; j < difficultyBeatmapSet.difficultyBeatmaps.Length; j++) {
							difficultyBeatmapSet.difficultyBeatmaps[j].difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
							diffs[j] = difficulty;
						}
						sets.Add(new PreviewDifficultyBeatmapSet(beatmapCharacteristicBySerializedName, diffs));
					}
				}
				return new CustomPreviewBeatmapLevel(defaultPackCover, standardLevelInfoSaveData, dataPath, new MemorySpriteLoader(preview.preview.coverRenderTask), levelId, standardLevelInfoSaveData.songName, standardLevelInfoSaveData.songSubName, standardLevelInfoSaveData.songAuthorName, standardLevelInfoSaveData.levelAuthorName, standardLevelInfoSaveData.beatsPerMinute, standardLevelInfoSaveData.songTimeOffset, standardLevelInfoSaveData.shuffle, standardLevelInfoSaveData.shufflePeriod, standardLevelInfoSaveData.previewStartTime, standardLevelInfoSaveData.previewDuration, environmentInfo, allDirectionsEnvironmentInfo, sets.ToArray());
			} catch {
				return null;
			}
		}
		static async System.Threading.Tasks.Task<UnityEngine.AudioClip?> DecodeAudio(System.IO.Compression.ZipArchiveEntry song, UnityEngine.AudioType type) {
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
		async System.Threading.Tasks.Task<CustomBeatmapLevel?> LoadZippedBeatmapLevelAsync(CustomPreviewBeatmapLevel customPreviewBeatmapLevel, System.IO.Compression.ZipArchive archive, bool modded, System.Threading.CancellationToken cancellationToken) {
			StandardLevelInfoSaveData standardLevelInfoSaveData = customPreviewBeatmapLevel.standardLevelInfoSaveData;
			CustomBeatmapLevel customBeatmapLevel = new CustomBeatmapLevel(customPreviewBeatmapLevel);
			CustomDifficultyBeatmapSet[] difficultyBeatmapSets = new CustomDifficultyBeatmapSet[standardLevelInfoSaveData.difficultyBeatmapSets.Length];
			for(int i = 0; i < difficultyBeatmapSets.Length; ++i) {
				BeatmapCharacteristicSO? beatmapCharacteristicBySerializedName = beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(standardLevelInfoSaveData.difficultyBeatmapSets[i].beatmapCharacteristicName);
				CustomDifficultyBeatmap[] difficultyBeatmaps = new CustomDifficultyBeatmap[standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps.Length];
				difficultyBeatmapSets[i] = new CustomDifficultyBeatmapSet(beatmapCharacteristicBySerializedName);
				for(int j = 0; j < standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps.Length; ++j) {
					string filename = standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].beatmapFilename;
					System.IO.Compression.ZipArchiveEntry file = archive.GetEntry(filename);
					if(file == null) {
						Log?.Error("File not found in archive: " + filename);
						return null;
					}
					byte[] rawData = new byte[file.Length];
					file.Open().Read(rawData, 0, rawData.Length);
					if(modded) {
						if(ValidatedPath(filename, out string path))
							System.IO.File.WriteAllBytes(path, rawData);
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
				Log?.Error("File not found in archive: " + standardLevelInfoSaveData.songFilename);
				return null;
			}
			UnityEngine.AudioClip? audioClip = await DecodeAudio(song, AudioTypeHelper.GetAudioTypeFromPath(standardLevelInfoSaveData.songFilename));
			if(audioClip == null) {
				Log?.Debug("NULL AUDIO CLIP");
				return null;
			}
			Log?.Debug("customBeatmapLevel.SetBeatmapLevelData()");
			customBeatmapLevel.SetBeatmapLevelData(new BeatmapLevelData(audioClip, difficultyBeatmapSets));
			return customBeatmapLevel;
		}
		public async System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult?> Resolve(System.Action<PacketHandler.LoadProgress> progress, System.Threading.CancellationToken cancellationToken) {
			Log?.Debug("Starting direct download");
			try {
				cancellationToken.ThrowIfCancellationRequested();
				sources ??= System.Linq.Enumerable.ToList(System.Linq.Enumerable.Where(System.Linq.Enumerable.Select(sourcePlayers, userId => handler.multiplayerSessionManager.GetPlayerByUserId(userId)), player => player is ConnectedPlayerManager.ConnectedPlayer));
				int it = 0, p = 0;
				uint cycle = 0;
				ulong off = gaps[0].start;
				while(gaps.Count >= 1) {
					if(++cycle == 4) {
						ulong dl = (ulong)buffer.LongLength;
						foreach((ulong start, ulong end) in gaps)
							dl -= (end - start);
						progress(new PacketHandler.LoadProgress(PacketHandler.LoadProgress.LoadState.Downloading, (ushort)(dl * 65535 / (ulong)buffer.Length)));
						cycle = 0;
					}
					if(it >= gaps.Count)
						it = 0;
					for(uint i = 64; i >= 1; --i) {
						if(off >= gaps[it].end) {
							if(++it >= gaps.Count)
								it = 0;
							off = gaps[it].start;
						}
						ushort len = (ushort)System.Math.Min(380, gaps[it].end - off);
						if(sources.Count < 1) {
							Log?.Error($"No available download sources!");
							return null;
						}
						p = (p + 1) % sources.Count;
						handler.SendUnreliableToPlayer(new PacketHandler.LevelFragmentRequest(off, len), sources[p]);
						off += 380;
					}
					await System.Threading.Tasks.Task.Delay(7, cancellationToken);
					cancellationToken.ThrowIfCancellationRequested();
				}
				Log?.Debug($"Finished downloading {buffer.Length} bytes");
				byte[] hash = await System.Threading.Tasks.Task.Run(() => {
					using System.Security.Cryptography.SHA256 sha256 = System.Security.Cryptography.SHA256.Create();
					return sha256.ComputeHash(buffer);
				});
				if(!System.Linq.Enumerable.SequenceEqual(levelHash, hash)) {
					Log?.Error("Hash mismatch!");
					return null;
				}
				using(System.IO.MemoryStream stream = new System.IO.MemoryStream(buffer)) {
					using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(stream, System.IO.Compression.ZipArchiveMode.Read)) {
						if(archive == null)
							return null;
						ulong totalLength = 0;
						foreach(System.IO.Compression.ZipArchiveEntry entry in archive.Entries) {
							totalLength += (ulong)entry.Length;
							if((ulong)entry.Length > MaxUnzippedSize || totalLength > MaxUnzippedSize)
								return null;
						}
						System.IO.Compression.ZipArchiveEntry infoFile = archive.GetEntry("Info.dat");
						if(infoFile == null) {
							Log?.Error("File not found in archive: Info.dat");
							return null;
						}
						if(System.IO.Directory.Exists(dataPath)) {
							foreach(System.IO.FileInfo file in new System.IO.DirectoryInfo(dataPath).GetFiles())
								file.Delete();
						} else {
							System.IO.Directory.CreateDirectory(dataPath);
						}
						bool modded = preview.requirements.Length >= 1 || preview.suggestions.Length >= 1;
						byte[] rawInfo = new byte[infoFile.Length];
						infoFile.Open().Read(rawInfo, 0, rawInfo.Length);
						if(modded) {
							if(ValidatedPath("Info.dat", out string path))
								System.IO.File.WriteAllBytes(path, rawInfo);
							else
								return null;
						}
						StandardLevelInfoSaveData info = StandardLevelInfoSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(rawInfo, 0, rawInfo.Length));
						CustomPreviewBeatmapLevel? previewBeatmapLevel = LoadZippedPreviewBeatmapLevel(info, archive);
						if(previewBeatmapLevel == null)
							return null;
						preview.preview.Init(previewBeatmapLevel);
						CustomBeatmapLevel? level = await LoadZippedBeatmapLevelAsync(previewBeatmapLevel, archive, modded, cancellationToken);
						if(level == null)
							return null;
						Log?.Debug($"level: {level}");
						return new BeatmapLevelsModel.GetBeatmapLevelResult(isError: false, level);
					}
				}
			} catch(System.Exception ex) {
				Log?.Error("Direct download error: " + ex);
				Log?.Error(ex.StackTrace);
				return null;
			}
		}
		public void HandleFragment(PacketHandler.LevelFragment fragment, IConnectedPlayer player) {
			if(gaps.Count < 1 || sources?.Contains(player) != true)
				return;
			if(fragment.offset >= (ulong)buffer.LongLength || fragment.offset + (uint)fragment.data.LongLength > (ulong)buffer.LongLength || fragment.data.LongLength < 1) {
				sources.Remove(player);
				return;
			}
			System.Buffer.BlockCopy(fragment.data, 0, buffer, (int)fragment.offset, fragment.data.Length);
			ulong start = fragment.offset, end = fragment.offset + (ulong)fragment.data.LongLength;
			int i = gaps.Count - 1;
			while(i >= 1 && gaps[i].start > start)
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

	public static MultiplayerStatusModel? multiplayerStatusModel = null;
	public static QuickPlaySetupModel? quickPlaySetupModel = null;
	public static void InvalidateModels() {
		if(multiplayerStatusModel != null)
			multiplayerStatusModel._request = null;
		if(quickPlaySetupModel != null)
			quickPlaySetupModel._request = null;
		if(mainFlowCoordinator.childFlowCoordinator is MultiplayerModeSelectionFlowCoordinator multiplayerModeSelectionFlowCoordinator) {
			editServerViewController.Dismiss(true);
			mainFlowCoordinator.DismissFlowCoordinator(multiplayerModeSelectionFlowCoordinator, HMUI.ViewController.AnimationDirection.Horizontal, (System.Action)delegate() {
				mainFlowCoordinator.PresentMultiplayerModeSelectionFlowCoordinatorWithDisclaimerAndAvatarCreator();
			}, true);
		}
	}

	public static class UI {
		public class ToggleSetting : SwitchSettingsController {
			public ValueCB<bool> valueCB = null!;
			public ToggleSetting() {
				_toggle = gameObject.transform.GetChild(1).gameObject.GetComponent<UnityEngine.UI.Toggle>();
				_toggle.onValueChanged.RemoveAllListeners();
			}
			protected override bool GetInitValue() => valueCB();
			protected override void ApplyValue(bool value) {
				valueCB() = value;
				Config.Instance.Changed();
			}
		}
		public class ValuePickerSetting : ListSettingsController {
			public byte[] options = null!;
			public ValueCB<float> valueCB = null!;
			public int startIdx = 1;
			public ValuePickerSetting() =>
				_stepValuePicker = gameObject.transform.GetChild(1).gameObject.GetComponent<StepValuePicker>();
			protected override bool GetInitValues(out int idx, out int numberOfElements) {
				idx = System.Array.LastIndexOf(options, (byte)(valueCB() * 4));
				if(idx == 0)
					startIdx = 0;
				else
					--idx;
				numberOfElements = options.Length - startIdx;
				return true;
			}
			protected override void ApplyValue(int idx) {
				valueCB() = options[idx + startIdx] / 4.0f;
				Config.Instance.Changed();
			}
			protected override string TextForValue(int idx) => $"{options[idx + startIdx] / 4.0f}";
		}
		public class DropdownSetting : DropdownSettingsController {
			string[] options = null!;
			public StringSO value = null!;
			public DropdownSetting() =>
				_dropdown = GetComponent<HMUI.SimpleTextDropdown>();
			protected override bool GetInitValues(out int idx, out int numberOfElements) {
				idx = System.Array.IndexOf(options, value.value) + 1;
				numberOfElements = options.Length + 1;
				return true;
			}
			protected override void ApplyValue(int idx) {
				string newValue = idx >= 1 ? options[idx - 1] : "";
				_dropdown.Hide(value.value.Equals(newValue)); // Animation will break if MultiplayerLevelSelectionFlowCoordinator is immediately dismissed
				value.value = newValue;
			}
			protected override string TextForValue(int idx) =>
				idx >= 1 ? options[idx - 1] : "Official Server";
			public void SetOptions(System.Collections.Generic.IEnumerable<string> options) {
				this.options = System.Linq.Enumerable.ToArray(options);
				if(gameObject.activeSelf) {
					gameObject.SetActive(false);
					gameObject.SetActive(true);
				}
			}
		}
		public delegate ref T ValueCB<T>();
		static UnityEngine.GameObject CreateElement(UnityEngine.GameObject template, UnityEngine.Transform parent, string name) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(template, parent);
			gameObject.name = "BeatUpClient_" + name;
			gameObject.SetActive(false);
			return gameObject;
		}
		static UnityEngine.GameObject CreateElementWithText(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, string headerKey) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name);
			((UnityEngine.RectTransform)gameObject.transform).sizeDelta = new UnityEngine.Vector2(90, ((UnityEngine.RectTransform)gameObject.transform).sizeDelta.y);
			UnityEngine.GameObject nameText = gameObject.transform.Find("NameText").gameObject;
			nameText.GetComponent<Polyglot.LocalizedTextMeshProUGUI>().Key = headerKey;
			return gameObject;
		}
		static UnityEngine.GameObject? toggleTemplate = null;
		public static UnityEngine.RectTransform CreateToggle(UnityEngine.Transform parent, string name, string headerKey, ValueCB<bool> valueCB) {
			if(toggleTemplate == null)
				toggleTemplate = System.Linq.Enumerable.First(System.Linq.Enumerable.Select(UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.UI.Toggle>(), x => x.transform.parent.gameObject), p => p.name == "Fullscreen");
			UnityEngine.GameObject gameObject = CreateElementWithText(toggleTemplate, parent, name, headerKey);
			UnityEngine.Object.Destroy(gameObject.GetComponent<BoolSettingsController>());
			ToggleSetting toggleSetting = gameObject.AddComponent<ToggleSetting>();
			toggleSetting.valueCB = valueCB;
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		static UnityEngine.GameObject? pickerTemplate = null;
		public static UnityEngine.RectTransform CreateValuePicker(UnityEngine.Transform parent, string name, string headerKey, ValueCB<float> valueCB, byte[] options) {
			if(pickerTemplate == null)
				pickerTemplate = System.Linq.Enumerable.First(System.Linq.Enumerable.Select(UnityEngine.Resources.FindObjectsOfTypeAll<StepValuePicker>(), x => x.transform.parent.gameObject), p => p.name == "MaxNumberOfPlayers");
			UnityEngine.GameObject gameObject = CreateElementWithText(pickerTemplate, parent, name, headerKey);
			UnityEngine.Object.Destroy(gameObject.GetComponent<FormattedFloatListSettingsController>());
			ValuePickerSetting valuePickerSetting = gameObject.AddComponent<ValuePickerSetting>();
			valuePickerSetting.options = options;
			valuePickerSetting.valueCB = valueCB;
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		static UnityEngine.GameObject? simpleDropdownTemplate = null;
		public static UnityEngine.RectTransform CreateSimpleDropdown(UnityEngine.Transform parent, string name, StringSO value, System.Collections.Generic.IEnumerable<string> options) {
			if(simpleDropdownTemplate == null)
				simpleDropdownTemplate = System.Linq.Enumerable.First(UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.SimpleTextDropdown>(), dropdown => dropdown.GetComponents(typeof(UnityEngine.Component)).Length == 2).gameObject;
			UnityEngine.GameObject gameObject = CreateElement(simpleDropdownTemplate, parent, name);
			DropdownSetting dropdownSetting = gameObject.AddComponent<DropdownSetting>();
			dropdownSetting.value = value;
			dropdownSetting.SetOptions(options);
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		static UnityEngine.GameObject? textboxTemplate = null;
		public static UnityEngine.RectTransform CreateTextbox(UnityEngine.Transform parent, string name, int index, System.Action<HMUI.InputFieldView> callback, string? placeholderKey = null) {
			if(textboxTemplate == null)
				textboxTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<EnterPlayerGuestNameViewController>()[0]._nameInputFieldView.gameObject;
			UnityEngine.GameObject gameObject = CreateElement(textboxTemplate, parent, name);
			HMUI.InputFieldView inputField = gameObject.GetComponent<HMUI.InputFieldView>();
			inputField.onValueChanged = new HMUI.InputFieldView.InputFieldChanged();
			inputField.onValueChanged.AddListener(callback.Invoke);
			Polyglot.LocalizedTextMeshProUGUI localizedText = inputField._placeholderText.GetComponent<Polyglot.LocalizedTextMeshProUGUI>();
			localizedText.enabled = (placeholderKey == null);
			localizedText.Key = placeholderKey ?? string.Empty;
			gameObject.transform.SetSiblingIndex(index);
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.GameObject CreateKey(HMUI.UIKeyboard keyboard, UnityEngine.Transform rightOf, UnityEngine.KeyCode keyCode, char charCode, bool canBeUppercase = false) {
			UnityEngine.GameObject keyTemplate = keyboard.GetComponentInChildren<HMUI.UIKeyboardKey>().gameObject;
			UnityEngine.GameObject key = UnityEngine.Object.Instantiate(keyTemplate, rightOf.parent);
			key.name = "" + keyCode;
			key.transform.localPosition = new UnityEngine.Vector3(rightOf.localPosition.x + 7, rightOf.localPosition.y, rightOf.localPosition.z);
			HMUI.UIKeyboardKey keyboardKey = key.GetComponent<HMUI.UIKeyboardKey>();
			keyboardKey._keyCode = keyCode;
			keyboardKey._overrideText = $"{charCode}";
			keyboardKey._canBeUppercase = canBeUppercase;
			keyboardKey.Awake();
			keyboard._buttonBinder.AddBinding(key.GetComponent<HMUI.NoTransitionsButton>(), delegate {
				System.Action<char>? evt = Reflection.Field<System.Action<char>>(keyboard, typeof(HMUI.UIKeyboard).GetEvent("keyWasPressedEvent").Name);
				if(evt != null)
					foreach(System.Delegate handler in evt.GetInvocationList())
						handler.Method.Invoke(handler.Target, new object[] {charCode});
			});
			return key;
		}
		public static UnityEngine.RectTransform CreateButtonFrom(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, System.Action callback) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name);
			UnityEngine.UI.Button button = gameObject.GetComponent<UnityEngine.UI.Button>();
			button.onClick.RemoveAllListeners();
			button.onClick.AddListener(callback.Invoke);
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateClone(UnityEngine.GameObject template, UnityEngine.Transform parent, string name, int index = -1) {
			UnityEngine.GameObject gameObject = CreateElement(template, parent, name);
			if(index >= 0)
				gameObject.transform.SetSiblingIndex(index);
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
	}

	public class DifficultyPanel {
		static UnityEngine.GameObject characteristicTemplate = null!;
		static UnityEngine.GameObject difficultyTemplate = null!;
		BeatmapCharacteristicSegmentedControlController beatmapCharacteristicSegmentedControlController;
		BeatmapDifficultySegmentedControlController beatmapDifficultySegmentedControlController;
		Reflection.FieldProxy<System.Action<BeatmapCharacteristicSegmentedControlController, BeatmapCharacteristicSO>> didSelectBeatmapCharacteristicEvent;
		Reflection.FieldProxy<System.Action<BeatmapDifficultySegmentedControlController, BeatmapDifficulty>> didSelectDifficultyEvent;
		public UnityEngine.RectTransform beatmapCharacteristic => (UnityEngine.RectTransform)beatmapCharacteristicSegmentedControlController.transform.parent;
		public UnityEngine.RectTransform beatmapDifficulty => (UnityEngine.RectTransform)beatmapDifficultySegmentedControlController.transform.parent;

		static UnityEngine.GameObject Clone(UnityEngine.GameObject template) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(template);
			UnityEngine.RectTransform tr = (UnityEngine.RectTransform)gameObject.transform;
			UnityEngine.RectTransform bg = (UnityEngine.RectTransform)tr.Find("BG");
			bg.pivot = tr.pivot = new UnityEngine.Vector2(.5f, 1);
			bg.localPosition = new UnityEngine.Vector3(0, 0, 0);
			UnityEngine.Object.DontDestroyOnLoad(gameObject);
			return gameObject;
		}
		public static void Init() {
			StandardLevelDetailView LevelDetail = UnityEngine.Resources.FindObjectsOfTypeAll<StandardLevelDetailView>()[0];
			characteristicTemplate = Clone(LevelDetail._beatmapCharacteristicSegmentedControlController.transform.parent.gameObject);
			difficultyTemplate = Clone(LevelDetail._beatmapDifficultySegmentedControlController.transform.parent.gameObject);
		}
		static void ChangeBackground(UnityEngine.RectTransform target, HMUI.ImageView newBackground) {
			HMUI.ImageView bg = target.Find("BG").GetComponent<HMUI.ImageView>();
			bg._skew = newBackground._skew;
			bg.color = newBackground.color;
			bg.color0 = newBackground.color0;
			bg.color1 = newBackground.color1;
			bg._flipGradientColors = newBackground._flipGradientColors;
		}
		public DifficultyPanel(UnityEngine.Transform parent, int index, float width, HMUI.ImageView? background = null) {
			UnityEngine.RectTransform beatmapCharacteristic = UI.CreateClone(characteristicTemplate, parent, "BeatmapCharacteristic", index);
			UnityEngine.RectTransform beatmapDifficulty = UI.CreateClone(difficultyTemplate, parent, "BeatmapDifficulty", index + 1);
			if(background != null) {
				ChangeBackground(beatmapCharacteristic, background);
				ChangeBackground(beatmapDifficulty, background);
			}
			beatmapCharacteristic.sizeDelta = new UnityEngine.Vector2(width, beatmapCharacteristic.sizeDelta.y);
			beatmapDifficulty.sizeDelta = new UnityEngine.Vector2(width, beatmapDifficulty.sizeDelta.y);
			beatmapCharacteristicSegmentedControlController = beatmapCharacteristic.GetComponentInChildren<BeatmapCharacteristicSegmentedControlController>();
			beatmapDifficultySegmentedControlController = beatmapDifficulty.GetComponentInChildren<BeatmapDifficultySegmentedControlController>();
			didSelectBeatmapCharacteristicEvent = Reflection.Field<System.Action<BeatmapCharacteristicSegmentedControlController, BeatmapCharacteristicSO>>(beatmapCharacteristicSegmentedControlController, typeof(BeatmapCharacteristicSegmentedControlController).GetEvent("didSelectBeatmapCharacteristicEvent").Name);
			didSelectDifficultyEvent = Reflection.Field<System.Action<BeatmapDifficultySegmentedControlController, BeatmapDifficulty>>(beatmapDifficultySegmentedControlController, typeof(BeatmapDifficultySegmentedControlController).GetEvent("didSelectDifficultyEvent").Name);
			beatmapCharacteristicSegmentedControlController._segmentedControl._container = new Zenject.DiContainer();
			beatmapCharacteristicSegmentedControlController._segmentedControl._container.Bind<HMUI.HoverHintController>().FromInstance(UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.HoverHintController>()[0]).AsSingle();
			beatmapDifficultySegmentedControlController._difficultySegmentedControl._container = new Zenject.DiContainer();
		}
		public void Clear() {
			beatmapCharacteristicSegmentedControlController.transform.parent.gameObject.SetActive(false);
			beatmapDifficultySegmentedControlController.transform.parent.gameObject.SetActive(false);
		}
		public void Update(PreviewDifficultyBeatmap? beatmapLevel, System.Action<PreviewDifficultyBeatmap> onChange) {
			didSelectBeatmapCharacteristicEvent.Set(delegate {});
			didSelectDifficultyEvent.Set(delegate {});
			if(beatmapLevel == null || beatmapLevel.beatmapLevel.previewDifficultyBeatmapSets == null) {
				Clear();
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
				PreviewDifficultyBeatmapSet set = System.Linq.Enumerable.First(beatmapLevel.beatmapLevel.previewDifficultyBeatmapSets, set => set.beatmapCharacteristic == beatmapCharacteristic);
				BeatmapDifficulty closestDifficulty = set.beatmapDifficulties[0];
				foreach(BeatmapDifficulty difficulty in set.beatmapDifficulties) {
					if(beatmapLevel.beatmapDifficulty < difficulty)
						break;
					closestDifficulty = difficulty;
				}
				onChange(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapCharacteristic, closestDifficulty));
			};
			beatmapDifficultySegmentedControlController.didSelectDifficultyEvent += delegate(BeatmapDifficultySegmentedControlController controller, BeatmapDifficulty difficulty) {
				onChange(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapLevel.beatmapCharacteristic, difficulty));
			};
			beatmapCharacteristicSegmentedControlController.transform.parent.gameObject.SetActive(true);
			beatmapDifficultySegmentedControlController.transform.parent.gameObject.SetActive(true);
		}
	}

	[Patch(PatchType.Prefix, typeof(ClientCertificateValidator), "ValidateCertificateChainInternal")]
	public static bool ClientCertificateValidator_ValidateCertificateChainInternal() =>
		customNetworkConfig == null;

	[Patch(PatchType.Postfix, typeof(MainSettingsModelSO), nameof(MainSettingsModelSO.Load))]
	public static void MainSettingsModelSO_Load(ref MainSettingsModelSO __instance) {
		customServerHostName = __instance.customServerHostName;
		if(!string.IsNullOrEmpty(__instance.customServerHostName))
			Config.Instance.Servers.TryAdd(__instance.customServerHostName, null);
	}

	public static async System.Threading.Tasks.Task<AuthenticationToken> AuthWrapper(System.Threading.Tasks.Task<AuthenticationToken> task, AuthenticationToken.Platform platform, string userId, string userName) {
		try {
			return await task;
		} catch(System.Security.Authentication.AuthenticationException) {
			return new AuthenticationToken(platform, userId, userName, ""); // fix for offline mode
		}
	}

	[Patch(PatchType.Postfix, typeof(PlatformAuthenticationTokenProvider), nameof(PlatformAuthenticationTokenProvider.GetAuthenticationToken))]
	public static void PlatformAuthenticationTokenProvider_GetAuthenticationToken(ref System.Threading.Tasks.Task<AuthenticationToken> __result, AuthenticationToken.Platform ____platform, string ____userId, string ____userName) {
		if(customNetworkConfig != null)
			__result = AuthWrapper(__result, ____platform, ____userId, ____userName);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerModeSelectionFlowCoordinator), nameof(MultiplayerModeSelectionFlowCoordinator.PresentMasterServerUnavailableErrorDialog))]
	public static bool MultiplayerModeSelectionFlowCoordinator_PresentMasterServerUnavailableErrorDialog(MultiplayerModeSelectionFlowCoordinator __instance, MultiplayerModeSelectionViewController ____multiplayerModeSelectionViewController, MultiplayerUnavailableReason reason, long? maintenanceWindowEndTime, string? remoteLocalizedMessage) {
		if(mainFlowCoordinator.childFlowCoordinator != __instance)
			return false;
		____multiplayerModeSelectionViewController.transform.Find("Buttons")?.gameObject.SetActive(false);
		____multiplayerModeSelectionViewController._maintenanceMessageText.text = remoteLocalizedMessage ?? ((reason == MultiplayerUnavailableReason.MaintenanceMode) ? Polyglot.Localization.GetFormat(MultiplayerUnavailableReasonMethods.LocalizedKey(reason), (TimeExtensions.AsUnixTime(maintenanceWindowEndTime.GetValueOrDefault()) - System.DateTime.UtcNow).ToString("h':'mm")) : (Polyglot.Localization.Get(MultiplayerUnavailableReasonMethods.LocalizedKey(reason)) + " (" + MultiplayerUnavailableReasonMethods.ErrorCode(reason) + ")"));
		____multiplayerModeSelectionViewController._maintenanceMessageText.richText = true;
		____multiplayerModeSelectionViewController._maintenanceMessageText.transform.localPosition = new UnityEngine.Vector3(0, 15, 0);
		____multiplayerModeSelectionViewController._maintenanceMessageText.gameObject.SetActive(true);
		__instance.ReplaceTopViewController(____multiplayerModeSelectionViewController, __instance.ProcessDeeplinkingToLobby, HMUI.ViewController.AnimationType.In, HMUI.ViewController.AnimationDirection.Horizontal);
		__instance.SetTitle(Polyglot.Localization.Get("LABEL_CONNECTION_ERROR"), HMUI.ViewController.AnimationType.In);
		return false;
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerModeSelectionViewController), nameof(MultiplayerModeSelectionViewController.SetData))]
	public static void MultiplayerModeSelectionViewController_SetData(MultiplayerModeSelectionViewController __instance, TMPro.TextMeshProUGUI ____maintenanceMessageText) {
		____maintenanceMessageText.richText = false;
		____maintenanceMessageText.transform.localPosition = new UnityEngine.Vector3(0, -5, 0);
		__instance.transform.Find("Buttons")?.gameObject.SetActive(true);
	}

	[Patch(PatchType.Prefix, typeof(LobbySetupViewController), nameof(LobbySetupViewController.SetPlayersMissingLevelText))]
	public static void LobbySetupViewController_SetPlayersMissingLevelText(LobbySetupViewController __instance, string playersMissingLevelText, ref UnityEngine.UI.Button ____startGameReadyButton) {
		if(!string.IsNullOrEmpty(playersMissingLevelText) && ____startGameReadyButton.interactable)
			__instance.SetStartGameEnabled(CannotStartGameReason.DoNotOwnSong);
	}

	static bool enableCustomLevels = false;
	[Patch(PatchType.Prefix, typeof(MasterServerConnectionManager), "HandleConnectToServerSuccess")]
	[Patch(PatchType.Prefix, typeof(GameLiftConnectionManager), "HandleConnectToServerSuccess")]
	public static void MasterServerConnectionManager_HandleConnectToServerSuccess(object __1, BeatmapLevelSelectionMask selectionMask, GameplayServerConfiguration configuration) {
		enableCustomLevels = selectionMask.songPacks.Contains("custom_levelpack_CustomLevels") && __1 is string;
		InitPreviews(configuration.maxPlayerCount);
		playerCells = new PlayerCell[configuration.maxPlayerCount];
		lobbyDifficultyPanel.Clear();
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelSelectionFlowCoordinator), "get_enableCustomLevels")]
	public static void MultiplayerLevelSelectionFlowCoordinator_enableCustomLevels(ref bool __result) =>
		__result |= enableCustomLevels;

	[Patch(PatchType.Prefix, typeof(MultiplayerSessionManager), "HandlePlayerOrderChanged")]
	public static void MultiplayerSessionManager_HandlePlayerOrderChanged(IConnectedPlayer player) =>
		playerCells[player.sortIndex].SetData(new PacketHandler.LoadProgress(PacketHandler.LoadProgress.LoadState.None, 0, 0));

	[Patch(PatchType.Postfix, typeof(BasicConnectionRequestHandler), nameof(BasicConnectionRequestHandler.GetConnectionMessage))]
	public static void BasicConnectionRequestHandler_GetConnectionMessage(LiteNetLib.Utils.NetDataWriter writer) {
		Log?.Debug("BasicConnectionRequestHandler_GetConnectionMessage()");
		LiteNetLib.Utils.NetDataWriter sub = new LiteNetLib.Utils.NetDataWriter(false, 16);
		new BeatUpConnectHeader {
			windowSize = Config.Instance.WindowSize,
			countdownDuration = (byte)(Config.Instance.CountdownDuration * 4),
			directDownloads = Config.Instance.DirectDownloads,
			skipResults = Config.Instance.SkipResults,
			perPlayerDifficulty = Config.Instance.PerPlayerDifficulty,
			perPlayerModifiers = Config.Instance.PerPlayerModifiers,
		}.Serialize(sub);
		writer.PutVarUInt((uint)sub.Length);
		writer.Put("BeatUpClient beta0");
		writer.Put(sub.CopyData());
		windowSize = 64;
		directDownloads = false;
		expectMetadata = false;
		playerTiers.Clear();
		skipResults = false;
		perPlayerDifficulty = false;
	}

	public class BeatUpConnectInfo : LiteNetLib.Utils.INetSerializable {
		public const uint Size = 6;
		public uint windowSize;
		public byte countdownDuration;
		public bool directDownloads;
		public bool skipResults;
		public bool perPlayerDifficulty;
		public bool perPlayerModifiers;
		public virtual void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)windowSize);
			writer.Put((byte)countdownDuration);
			byte bits = 0;
			bits |= directDownloads ? (byte)1 : (byte)0;
			bits |= skipResults ? (byte)2 : (byte)0;
			bits |= perPlayerDifficulty ? (byte)4 : (byte)0;
			bits |= perPlayerModifiers ? (byte)8 : (byte)0;
			writer.Put((byte)bits);
		}
		public virtual void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
			windowSize = reader.GetUInt();
			countdownDuration = reader.GetByte();
			byte bits = reader.GetByte();
			directDownloads = (bits & 1) == 1;
			skipResults = (bits & 2) == 2;
			perPlayerDifficulty = (bits & 4) == 4;
			perPlayerModifiers = (bits & 8) == 8;
		}
	}

	public class BeatUpConnectHeader : BeatUpConnectInfo {
		public uint protocolId = 1;
		public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)protocolId);
			base.Serialize(writer);
		}
		public override void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
			protocolId = reader.GetUInt();
			base.Deserialize(reader);
		}
	}

	[Patch(PatchType.Prefix, typeof(LiteNetLib.NetConnectAcceptPacket), "FromData")]
	public static void NetConnectAcceptPacket_FromData(ref LiteNetLib.NetPacket packet) {
		if(packet.Size == LiteNetLib.NetConnectAcceptPacket.Size + BeatUpConnectInfo.Size) {
			packet.Size = LiteNetLib.NetConnectAcceptPacket.Size;
			BeatUpConnectInfo info = new BeatUpConnectInfo();
			info.Deserialize(new LiteNetLib.Utils.NetDataReader(packet.RawData, packet.Size));
			if(info.windowSize < 32 || info.windowSize > 512)
				return;
			windowSize = info.windowSize;
			directDownloads = Config.Instance.DirectDownloads && info.directDownloads;
			expectMetadata = true;
			skipResults = info.skipResults;
			perPlayerDifficulty = info.perPlayerDifficulty;
			Log?.Info($"Overriding window size - {info.windowSize}");
		}
	}

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.HandleMenuRpcManagerGetRecommendedBeatmap))]
	public static void LobbyPlayersDataModel_HandleMenuRpcManagerGetRecommendedBeatmap(string userId) =>
		handler.multiplayerSessionManager.Send(playerPreviews[handler.multiplayerSessionManager.localPlayer.sortIndex]);

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.SetLocalPlayerBeatmapLevel))]
	public static void LobbyPlayersDataModel_SetLocalPlayerBeatmapLevel(LobbyPlayersDataModel __instance, PreviewDifficultyBeatmap? beatmapLevel) {
		if(beatmapLevel != null) {
			PacketHandler.RecommendPreview? preview = playerPreviews[handler.multiplayerSessionManager.localPlayer.sortIndex];
			if(preview.preview.levelID != beatmapLevel.beatmapLevel.levelID) {
				preview = ResolvePreview(beatmapLevel.beatmapLevel.levelID);
				if(preview != null)
					playerPreviews[handler.multiplayerSessionManager.localPlayer.sortIndex] = preview;
				else if(beatmapLevel.beatmapLevel is CustomPreviewBeatmapLevel previewBeatmapLevel)
					preview = SetPreviewFromLocal(handler.multiplayerSessionManager.localPlayer.sortIndex, previewBeatmapLevel);
			}
			if(preview != null)
				handler.multiplayerSessionManager.Send(preview);
		}
		lobbyDifficultyPanel.Update(beatmapLevel, __instance.SetLocalPlayerBeatmapLevel);
	}

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.ClearLocalPlayerBeatmapLevel))]
	public static void LobbyPlayersDataModel_ClearLocalPlayerBeatmapLevel() =>
		lobbyDifficultyPanel.Clear();

	public class ErrorBeatmapLevel : EmptyBeatmapLevel, IPreviewBeatmapLevel {
		string levelId;
		string? IPreviewBeatmapLevel.levelID => levelId;
		string? IPreviewBeatmapLevel.songName => "[ERROR]";
		string? IPreviewBeatmapLevel.songAuthorName => "[ERROR]";
		System.Threading.Tasks.Task<UnityEngine.Sprite> IPreviewBeatmapLevel.GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) =>
			System.Threading.Tasks.Task.FromResult<UnityEngine.Sprite>(defaultPackCover);
		public ErrorBeatmapLevel(string levelId) =>
			this.levelId = levelId;
	}

	[Patch(PatchType.Postfix, typeof(BeatmapLevelsModel), nameof(BeatmapLevelsModel.GetLevelPreviewForLevelId))]
	public static void BeatmapLevelsModel_GetLevelPreviewForLevelId(ref IPreviewBeatmapLevel? __result, string levelId) =>
		__result ??= (IPreviewBeatmapLevel?)ResolvePreview(levelId)?.preview ?? new ErrorBeatmapLevel(levelId);

	class CallbackStream : System.IO.Stream {
		public readonly System.IO.Stream stream;
		public readonly System.Action<int> onProgress;
		public override bool CanRead {get => stream.CanRead;}
		public override bool CanSeek {get => stream.CanSeek;}
		public override bool CanWrite {get => stream.CanWrite;}
		public override long Length {get => stream.Length;}
		public override long Position {
			get => stream.Position;
			set => stream.Position = value;
		}
		public CallbackStream(System.IO.Stream stream, System.Action<int> onProgress) {
			this.stream = stream;
			this.onProgress = onProgress;
		}
		public override void Flush() => stream.Flush();
		public override long Seek(long offset, System.IO.SeekOrigin origin) => stream.Seek(offset, origin);
		public override void SetLength(long value) => stream.SetLength(value);
		public override int Read(byte[] buffer, int offset, int count) {
			count = stream.Read(buffer, offset, count);
			onProgress(count);
			return count;
		}
		public override void Write(byte[] buffer, int offset, int count) => stream.Write(buffer, offset, count);
		public override async System.Threading.Tasks.Task WriteAsync(byte[] buffer, int offset, int count, System.Threading.CancellationToken cancellationToken) {
			await stream.WriteAsync(buffer, offset, count, cancellationToken);
			onProgress(count);
		}
	}

	public static async System.Threading.Tasks.Task<(System.ArraySegment<byte> data, byte[] hash)> ZipLevel(CustomBeatmapLevel level) {
		System.ArraySegment<byte> buffer = new System.ArraySegment<byte>(new byte[0]);
		byte[] hash = new byte[32];
		using(System.IO.MemoryStream memoryStream = new System.IO.MemoryStream()) {
			ulong progress = 0, total = 0, progressWidth = 53738, progressStart = 0;
			System.Diagnostics.Stopwatch lastUpdate = new System.Diagnostics.Stopwatch();
			lastUpdate.Start();
			void UpdateProgress(ulong p, bool reliable = false) {
				PacketHandler.LoadProgress packet = new PacketHandler.LoadProgress(PacketHandler.LoadProgress.LoadState.Exporting, (ushort)p);
				PacketHandler.HandleLoadProgress(packet, handler.multiplayerSessionManager.localPlayer);
				if(reliable)
					handler.multiplayerSessionManager.Send(packet);
				else
					handler.multiplayerSessionManager.SendUnreliable(packet);
			}
			void HandleProgress(int count) {
				progress += (ulong)count;
				if(lastUpdate.ElapsedMilliseconds > 10) {
					lastUpdate.Restart();
					UpdateProgress(progress * progressWidth / total + progressStart);
				}
			}
			using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(memoryStream, System.IO.Compression.ZipArchiveMode.Create, true)) {
				System.Collections.Generic.List<(string path, string name)> files = new System.Collections.Generic.List<(string, string)>(new[] {
					(level.songAudioClipPath, level.standardLevelInfoSaveData.songFilename),
					(System.IO.Path.Combine(level.customLevelPath, "Info.dat"), "Info.dat"),
				});
				for(int i = 0; i < level.beatmapLevelData.difficultyBeatmapSets.Count; ++i) {
					for(int j = 0; j < level.beatmapLevelData.difficultyBeatmapSets[i].difficultyBeatmaps.Count; ++j) {
						if(level.beatmapLevelData.difficultyBeatmapSets[i].difficultyBeatmaps[j] is CustomDifficultyBeatmap beatmap) {
							string filename = level.standardLevelInfoSaveData.difficultyBeatmapSets[i].difficultyBeatmaps[j].beatmapFilename;
							files.Add((System.IO.Path.Combine(level.customLevelPath, filename), filename));
						}
					}
				}
				total = (ulong)System.Linq.Enumerable.Aggregate(files, 0L, (total, file) => total + new System.IO.FileInfo(file.path).Length);
				if(total > MaxUnzippedSize) {
					Log?.Debug("Level too large!");
					return (buffer, hash);
				}
				UpdateProgress(0);
				foreach((string path, string name) info in files) {
					Log?.Debug($"Zipping `{info.path}`");
					using(System.IO.Stream file = System.IO.File.Open(info.path, System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.Read)) {
						using(System.IO.Stream entry = archive.CreateEntry(info.name).Open()) {
							await file.CopyToAsync(new CallbackStream(entry, HandleProgress));
						}
					}
				}
			}
			if(memoryStream.TryGetBuffer(out buffer)) {
				progress = 0;
				total = (ulong)buffer.Count;
				progressStart = progressWidth;
				progressWidth = 65535 - progressStart;
				memoryStream.Seek(0, System.IO.SeekOrigin.Begin);
				hash = await System.Threading.Tasks.Task.Run(() => {
					using System.Security.Cryptography.SHA256 sha256 = System.Security.Cryptography.SHA256.Create();
					return sha256.ComputeHash(new CallbackStream(memoryStream, HandleProgress));
				});
			}
			UpdateProgress(65535, true);
			return (buffer, hash);
		}
	}

	public static async System.Threading.Tasks.Task<EntitlementsStatus> ShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> task, string levelId) {
		string? lastLevel = uploadLevel;
		uploadLevel = null;
		downloadPending = false;
		EntitlementsStatus status = await task;
		Log?.Debug($"EntitlementsStatus: {status}");
		if(status != EntitlementsStatus.Ok)
			return status;
		BeatmapLevelsModel.GetBeatmapLevelResult result = await beatmapLevelsModel.GetBeatmapLevelAsync(levelId, default(System.Threading.CancellationToken));
		Log?.Debug($"GetBeatmapLevelResult.isError: {result.isError}");
		if(result.isError) {
			if(directDownloads && ResolvePreview(levelId) != null) {
				downloadPending = true;
				return EntitlementsStatus.Unknown;
			} else {
				return EntitlementsStatus.NotOwned;
			}
		}
		if(directDownloads && result.beatmapLevel is CustomBeatmapLevel level) {
			if(lastLevel == levelId) {
				Log?.Debug("Custom level already zipped");
				uploadLevel = lastLevel;
			} else {
				Log?.Debug("Zipping custom level");
				(uploadData, uploadHash) = await ZipLevel(level);
				if(uploadData.Count >= 1) {
					uploadLevel = levelId;
					Log?.Debug($"Packed {uploadData.Count} bytes");
				} else {
					Log?.Debug("Zip failed");
				}
			}
		}
		return EntitlementsStatus.Ok;
	}

	[Patch(PatchType.Postfix, typeof(NetworkPlayerEntitlementChecker), "GetEntitlementStatus")]
	public static void NetworkPlayerEntitlementChecker_GetEntitlementStatus(NetworkPlayerEntitlementChecker __instance, ref System.Threading.Tasks.Task<EntitlementsStatus> __result, string levelId) {
		Log?.Debug($"NetworkPlayerEntitlementChecker_GetEntitlementStatus");
		if(!haveMpCore)
			__result = ShareWrapper(__result, levelId);
	}

	[Patch(PatchType.Prefix, typeof(MenuRpcManager), nameof(MenuRpcManager.SetIsEntitledToLevel))]
	public static void MenuRpcManager_SetIsEntitledToLevel(string levelId, EntitlementsStatus entitlementStatus) {
		Log?.Debug($"MenuRpcManager_SetIsEntitledToLevel(levelId=\"{levelId}\", entitlementStatus={entitlementStatus})");
		PacketHandler.RecommendPreview? preview = ResolvePreview(levelId);
		if(PacketHandler.MissingRequirements(preview, entitlementStatus == EntitlementsStatus.Unknown)) {
			entitlementStatus = EntitlementsStatus.NotOwned;
		} else if(entitlementStatus == EntitlementsStatus.Ok && uploadLevel == levelId) {
			Log?.Debug($"    Announcing share for `{levelId}`");
			handler.multiplayerSessionManager.Send(new PacketHandler.SetCanShareBeatmap(levelId, uploadHash, (ulong)uploadData.Count));
		}
		Log?.Debug($"    entitlementStatus={entitlementStatus}");
		PacketHandler.HandleSetIsEntitledToLevel(handler.multiplayerSessionManager.localPlayer, levelId, entitlementStatus);
	}

	public static void Progress(PacketHandler.LoadProgress packet) {
		PacketHandler.HandleLoadProgress(packet, handler.multiplayerSessionManager.localPlayer);
		handler.multiplayerSessionManager.SendUnreliable(packet);
	}

	static async System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> DownloadWrapper(System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> task, ILevelGameplaySetupData gameplaySetupData, System.Threading.CancellationToken cancellationToken) {
		Log?.Debug("DownloadWrapper()");
		BeatmapLevelsModel.GetBeatmapLevelResult result = await task;
		if(result.isError && handler.downloader != null)
			return (await handler.downloader.Resolve(Progress, cancellationToken)) ?? result;
		return result;
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	public static void MultiplayerLevelLoader_LoadLevel_pre(int ____loaderState, out bool __state) =>
		__state = (____loaderState == (int)MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.NotLoading && !haveMpCore);

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	public static void MultiplayerLevelLoader_LoadLevel_post(MultiplayerLevelLoader __instance, bool __state, ILevelGameplaySetupData gameplaySetupData, float initialStartTime, System.Threading.CancellationTokenSource ____getBeatmapCancellationTokenSource, ref System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> ____getBeatmapLevelResultTask) {
		if(__state && handler.downloader?.levelId == gameplaySetupData.beatmapLevel.beatmapLevel.levelID)
			____getBeatmapLevelResultTask = DownloadWrapper(____getBeatmapLevelResultTask, gameplaySetupData, ____getBeatmapCancellationTokenSource.Token);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
	public static void MultiplayerLevelLoader_Tick_pre(int ____loaderState, out bool __state) =>
		__state = (____loaderState == (int)MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.LoadingBeatmap);

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
	public static void MultiplayerLevelLoader_Tick_post(int ____loaderState, bool __state, ILevelGameplaySetupData ____gameplaySetupData) {
		if(__state && ____loaderState == (int)MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.WaitingForCountdown)
			handler.rpcManager.SetIsEntitledToLevel(____gameplaySetupData.beatmapLevel.beatmapLevel.levelID, EntitlementsStatus.Ok);
	}

	[Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "Send", typeof(LiteNetLib.Utils.INetSerializable))]
	public static bool ConnectedPlayerManager_Send(ConnectedPlayerManager __instance, LiteNetLib.Utils.INetSerializable message) {
		if(Config.Instance.UnreliableState && (message is NodePoseSyncStateNetSerializable || message is StandardScoreSyncStateNetSerializable)) {
			__instance.SendUnreliable(message);
			return false;
		}
		return true;
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerOutroAnimationController), nameof(MultiplayerOutroAnimationController.AnimateOutro))]
	public static bool MultiplayerOutroAnimationController_AnimateOutro(System.Action onCompleted) {
		if(skipResults)
			onCompleted.Invoke();
		return !skipResults;
	}

	[Patch(PatchType.Postfix, typeof(GameServerPlayersTableView), nameof(GameServerPlayersTableView.SetData))]
	public static void GameServerPlayersTableView_SetData(System.Collections.Generic.List<IConnectedPlayer> sortedPlayers, HMUI.TableView ____tableView) {
		for(uint i = 0; i < playerCells.Length; ++i)
			playerCells[i].transform = null;
		foreach(GameServerPlayerTableCell cell in ____tableView.visibleCells) {
			UnityEngine.UI.Image background = cell._localPlayerBackgroundImage;
			foreach(UnityEngine.Transform child in background.transform) {
				if(child.gameObject.name == "BeatUpClient_Progress") {
					IConnectedPlayer player = sortedPlayers[cell.idx];
					playerCells[player.sortIndex].SetBar((UnityEngine.RectTransform)child, background, player.isMe ? new UnityEngine.Color(0.1254902f, 0.7529412f, 1, 0.2509804f) : new UnityEngine.Color(0, 0, 0, 0));
					break;
				}
			}
		}
	}

	[Patch(PatchType.Prefix, typeof(GameServerPlayerTableCell), nameof(GameServerPlayerTableCell.Awake))]
	public static void GameServerPlayerTableCell_Awake(GameServerPlayerTableCell __instance, UnityEngine.UI.Image ____localPlayerBackgroundImage) =>
		UnityEngine.Object.Instantiate(____localPlayerBackgroundImage.gameObject, ____localPlayerBackgroundImage.transform).name = "BeatUpClient_Progress";

	[Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "HandleServerPlayerConnected")]
	public static void ConnectedPlayerManager_HandleServerPlayerConnected(ConnectedPlayerManager.PlayerConnectedPacket packet) {
		if(!expectMetadata)
			return;
		int tier = (int)packet.userName[packet.userName.Length-1] - 17;
		if(tier < -1 || tier >= badges.Length)
			return;
		if(tier >= 0)
			playerTiers[packet.userId] = (byte)tier;
		packet.userName = packet.userName.Substring(0, packet.userName.Length - 1);
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLobbyAvatarManager), nameof(MultiplayerLobbyAvatarManager.AddPlayer))]
	public static void MultiplayerLobbyAvatarManager_AddPlayer(IConnectedPlayer connectedPlayer, System.Collections.Generic.Dictionary<string, MultiplayerLobbyAvatarController> ____playerIdToAvatarMap) {
		if(!connectedPlayer.isMe && expectMetadata && playerTiers.TryGetValue(connectedPlayer.userId, out byte tier))
			UnityEngine.Object.Instantiate(badges[tier], ____playerIdToAvatarMap[connectedPlayer.userId].transform.GetChild(2));
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLocalActivePlayerInGameMenuViewController), nameof(MultiplayerLocalActivePlayerInGameMenuViewController.Start))]
	public static void MultiplayerLocalActivePlayerInGameMenuViewController_Start(MultiplayerLocalActivePlayerInGameMenuViewController __instance) {
		MultiplayerPlayersManager multiplayerPlayersManager = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerPlayersManager>()[0];
		if(perPlayerDifficulty && multiplayerPlayersManager.localPlayerStartSeekSongController is MultiplayerLocalActivePlayerFacade) {
			MenuTransitionsHelper menuTransitionsHelper = UnityEngine.Resources.FindObjectsOfTypeAll<MenuTransitionsHelper>()[0];
			MultiplayerConnectedPlayerSongTimeSyncController audioTimeSyncController = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerConnectedPlayerSongTimeSyncController>()[0];
			PreviewDifficultyBeatmap original = new PreviewDifficultyBeatmap(__instance._localPlayerInGameMenuInitData.previewBeatmapLevel, __instance._localPlayerInGameMenuInitData.beatmapCharacteristic, __instance._localPlayerInGameMenuInitData.beatmapDifficulty);
			PreviewDifficultyBeatmap selectedPreview = original;
			IDifficultyBeatmap? selectedBeatmap = null;
			UnityEngine.RectTransform switchButton = UI.CreateButtonFrom(__instance._resumeButton.gameObject, __instance._resumeButton.transform.parent, "SwitchDifficulty", () => {
				menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.Init(menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.gameMode, selectedPreview.beatmapLevel, selectedPreview.beatmapDifficulty, selectedPreview.beatmapCharacteristic, selectedBeatmap, menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.colorScheme, menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.gameplayCoreSceneSetupData.gameplayModifiers, menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.gameplayCoreSceneSetupData.playerSpecificSettings, menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.gameplayCoreSceneSetupData.practiceSettings, menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData.gameplayCoreSceneSetupData.useTestNoteCutSoundEffects);
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
			switchButton.GetComponentInChildren<Polyglot.LocalizedTextMeshProUGUI>().Key = "BEATUPCLIENT_SWITCH";
			switchButton.gameObject.SetActive(false);
			DifficultyPanel panel = new DifficultyPanel(__instance._mainBar.transform, 1, -2, __instance._levelBar.transform.Find("BG").GetComponent<HMUI.ImageView>());
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

	static readonly BoolSO customServerEnvironmentOverride = UnityEngine.ScriptableObject.CreateInstance<BoolSO>();

	[Patch(PatchType.Transpiler, typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	public static System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> MainSystemInit_InstallBindings(System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> instructions) {
		customServerEnvironmentOverride.value = true;
		System.Reflection.FieldInfo original = HarmonyLib.AccessTools.Field(typeof(MainSettingsModelSO), "useCustomServerEnvironment");
		System.Reflection.FieldInfo replace = HarmonyLib.AccessTools.Field(typeof(BeatUpClient), "customServerEnvironmentOverride");
		foreach(HarmonyLib.CodeInstruction instruction in instructions) {
			if(HarmonyLib.CodeInstructionExtensions.LoadsField(instruction, original)) {
				yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Pop);
				yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Ldsfld, replace);
			} else {
				yield return instruction;
			}
		}
	}

	[Patch(PatchType.Postfix, typeof(MainSystemInit), nameof(MainSystemInit.InstallBindings))]
	public static void MainSystemInit_InstallBindings_post(MainSystemInit __instance, Zenject.DiContainer container) {
		mainSystemInit = __instance;
		handler = new PacketHandler(container);
		multiplayerStatusModel = container.TryResolve<IMultiplayerStatusModel>() as MultiplayerStatusModel;
		quickPlaySetupModel = container.TryResolve<IQuickPlaySetupModel>() as QuickPlaySetupModel;
		customNetworkConfig = container.TryResolve<INetworkConfig>() as CustomNetworkConfig;
		beatmapLevelsModel = container.TryResolve<BeatmapLevelsModel>();
	}

	[Patch(PatchType.Transpiler, typeof(LiteNetLib.ReliableChannel), ".ctor")]
	public static System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> ReliableChannel_ctor(System.Collections.Generic.IEnumerable<HarmonyLib.CodeInstruction> instructions) {
		System.Reflection.FieldInfo replace = HarmonyLib.AccessTools.Field(typeof(BeatUpClient), "windowSize");
		bool notFound = true;
		foreach(HarmonyLib.CodeInstruction instruction in instructions) {
			if(notFound && HarmonyLib.CodeInstructionExtensions.LoadsConstant(instruction, 64)) {
				yield return new HarmonyLib.CodeInstruction(System.Reflection.Emit.OpCodes.Ldsfld, replace);
				notFound = false;
			} else {
				yield return instruction;
			}
		}
		if(notFound)
			Log?.Error("Failed to patch reliable window size");
	}

	class BeatUpScenesTransitionSetupDataSO : ScenesTransitionSetupDataSO {
		protected override void OnEnable() {
			SceneInfo si = CreateInstance<SceneInfo>();
			si._sceneName = "PCInit";
			Init(new SceneInfo[] {si}, System.Array.Empty<SceneSetupData>());
			base.OnEnable();
		}
	}

	static bool initialSceneRegistered = false;
	[PatchOverload(PatchType.Prefix, typeof(Zenject.Context), "InstallInstallers", new[] {typeof(System.Collections.Generic.List<Zenject.InstallerBase>), typeof(System.Collections.Generic.List<System.Type>), typeof(System.Collections.Generic.List<Zenject.ScriptableObjectInstaller>), typeof(System.Collections.Generic.List<Zenject.MonoInstaller>), typeof(System.Collections.Generic.List<Zenject.MonoInstaller>)})]
	public static void Context_InstallInstallers(Zenject.Context __instance) =>
		initialSceneRegistered |= (__instance.name == "AppCoreSceneContext");

	static BeatUpScenesTransitionSetupDataSO? restartTransitionData = null;
	[Patch(PatchType.Prefix, typeof(GameScenesManager), nameof(GameScenesManager.ReplaceScenes))] // Fallback if SiraUtil isn't installed
	public static bool GameScenesManager_ReplaceScenes(GameScenesManager __instance, ScenesTransitionSetupDataSO scenesTransitionSetupData) {
		if(haveSiraUtil || initialSceneRegistered)
			return true;
		if(restartTransitionData == null)
			restartTransitionData = UnityEngine.ScriptableObject.CreateInstance<BeatUpScenesTransitionSetupDataSO>();
		__instance.ClearAndOpenScenes(restartTransitionData, finishCallback: container =>
			UnityEngine.SceneManagement.SceneManager.GetSceneByName(__instance.GetCurrentlyLoadedSceneNames()[0]).GetRootGameObjects()[0].GetComponent<PCAppInit>().TransitionToNextScene());
		return false;
	}

	public struct PlayerCell {
		public UnityEngine.RectTransform? transform;
		UnityEngine.UI.Image background;
		UnityEngine.UI.Image foreground;
		UnityEngine.Color backgroundColor;
		PacketHandler.LoadProgress data;
		public void SetBar(UnityEngine.RectTransform transform, UnityEngine.UI.Image background, UnityEngine.Color backgroundColor) {
			this.transform = transform;
			this.background = background;
			this.backgroundColor = backgroundColor;
			foreground = transform.gameObject.GetComponent<HMUI.ImageView>();
			Refresh(transform);
		}
		public void UpdateData(PacketHandler.LoadProgress data, bool replace = false) {
			if(replace)
				data.sequence = this.data.sequence;
			else if(data.sequence < this.data.sequence)
				return;
			SetData(data);
		}
		public void SetData(PacketHandler.LoadProgress data) {
			this.data = data;
			if(transform != null)
				Refresh(transform);
		}
		void Refresh(UnityEngine.RectTransform transform) {
			float delta = (65535u - data.progress) * -104 / 65534f;
			transform.sizeDelta = new UnityEngine.Vector2(delta, transform.sizeDelta.y);
			transform.localPosition = new UnityEngine.Vector3(delta * (data.state == PacketHandler.LoadProgress.LoadState.Exporting ? .5f : -.5f), 0, 0);
			foreground.color = data.state switch {
				PacketHandler.LoadProgress.LoadState.Exporting => new UnityEngine.Color(0.9130435f, 1, 0.1521739f, 0.5434783f),
				PacketHandler.LoadProgress.LoadState.Downloading => new UnityEngine.Color(0.4782609f, 0.6956522f, 0.02173913f, 0.5434783f),
				_ => new UnityEngine.Color(0, 0, 0, 0),
			};
			background.color = data.state switch {
				PacketHandler.LoadProgress.LoadState.Failed => new UnityEngine.Color(0.7173913f, 0.2608696f, 0.02173913f, 0.7490196f),
				PacketHandler.LoadProgress.LoadState.Exporting => new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f),
				PacketHandler.LoadProgress.LoadState.Downloading => new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f),
				PacketHandler.LoadProgress.LoadState.Done => new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.7490196f),
				_ => backgroundColor,
			};
			foreground.enabled = (data.progress >= 1);
			background.enabled = true;
		}
	}

	public const ulong MaxDownloadSize = 268435456;
	public const ulong MaxUnzippedSize = 268435456;
	public static readonly HarmonyLib.Harmony harmony = new HarmonyLib.Harmony("BeatUpClient");
	public static BeatUpClient? Instance;
	public static IPA.Logging.Logger? Log;
	public static UnityEngine.Sprite defaultPackCover = null!;
	public static UnityEngine.GameObject[] badges = null!;
	public static System.Collections.Generic.Dictionary<string, byte> playerTiers = new System.Collections.Generic.Dictionary<string, byte>();
	public static bool downloadPending = false;
	public static string? uploadLevel = null;
	public static System.ArraySegment<byte> uploadData = new System.ArraySegment<byte>(new byte[0]);
	public static byte[] uploadHash = new byte[32];
	public static StringSO customServerHostName = null!;
	public static CustomNetworkConfig? customNetworkConfig = null;
	public static MainSystemInit mainSystemInit = null!;
	public static MainFlowCoordinator mainFlowCoordinator = null!;
	public static BeatmapLevelsModel beatmapLevelsModel = null!;
	public static PlayerCell[] playerCells = null!;
	public static bool haveSiraUtil = false;
	public static bool haveSongCore = false;
	public static bool haveMpCore = false;
	public static uint windowSize = 64;
	public static bool directDownloads = false;
	public static bool expectMetadata = false;
	public static bool skipResults = false;
	public static bool perPlayerDifficulty = false;

	public static DifficultyPanel lobbyDifficultyPanel = null!;

	static UnityEngine.UI.Button editServerButton = null!;
	static void RefreshNetworkConfig() {
		editServerButton.interactable = false;
		if(customNetworkConfig != null) {
			NetworkConfigSO networkConfigPrefab = mainSystemInit._networkConfig;
			string[] hostname = new[] {networkConfigPrefab.masterServerEndPoint.hostName};
			int port = networkConfigPrefab.masterServerEndPoint.port;
			bool forceGameLift = networkConfigPrefab.forceGameLift;
			string? multiplayerStatusUrl = networkConfigPrefab.multiplayerStatusUrl;
			if(!string.IsNullOrEmpty(customServerHostName)) {
				editServerButton.interactable = true;
				hostname = customServerHostName.value.ToLower().Split(new[] {':'});
				if(hostname.Length >= 2)
					int.TryParse(hostname[1], out port);
				forceGameLift = false;
				if(!Config.Instance.Servers.TryGetValue(customServerHostName.value, out multiplayerStatusUrl))
					multiplayerStatusUrl = null;
			}
			string oldMultiplayerStatusUrl = customNetworkConfig.multiplayerStatusUrl;
			Log?.Debug($"CustomNetworkConfig(customServerHostName=\"{hostname[0]}\", port={port}, forceGameLift={forceGameLift})");
			typeof(CustomNetworkConfig).GetConstructors()[0].Invoke(customNetworkConfig, new object[] {networkConfigPrefab, hostname[0], port, forceGameLift});
			if(!NullableStringHelper.IsNullOrEmpty(multiplayerStatusUrl))
				Reflection.Field<string>(customNetworkConfig, "<multiplayerStatusUrl>k__BackingField").Set(multiplayerStatusUrl);
			if(!forceGameLift)
				Reflection.Field<ServiceEnvironment>(customNetworkConfig, "<serviceEnvironment>k__BackingField").Set(networkConfigPrefab.serviceEnvironment);
			if(customNetworkConfig.multiplayerStatusUrl != oldMultiplayerStatusUrl)
				InvalidateModels();
		}
	}

	public class EditServerViewController : HMUI.ViewController {
		HMUI.FlowCoordinator? flowCoordinator = null;
		bool edit = false;
		VRUIControls.VRInputModule vrInputModule = null!;
		HMUI.InputFieldView editHostnameTextbox = null!;
		HMUI.InputFieldView editStatusTextbox = null!;
		HMUI.CurvedTextMeshPro editStatusPlaceholder = null!;
		HMUI.UIKeyboard keyboard = null!;
		UnityEngine.UI.Button cancelButton = null!;
		string? activeKey = null;

		public static EditServerViewController Create(string name, UnityEngine.Transform parent) {
			UnityEngine.GameObject gameObject = UnityEngine.Object.Instantiate(UnityEngine.Resources.FindObjectsOfTypeAll<ServerCodeEntryViewController>()[0].gameObject, parent);
			gameObject.name = name;
			UnityEngine.Object.Destroy(gameObject.GetComponent<ServerCodeEntryViewController>());
			UnityEngine.Object.Destroy(gameObject.GetComponentInChildren<HMUI.InputFieldView>().gameObject);
			EditServerViewController viewController = gameObject.AddComponent<EditServerViewController>();
			viewController.rectTransform.anchorMin = new UnityEngine.Vector2(0f, 0f);
			viewController.rectTransform.anchorMax = new UnityEngine.Vector2(1f, 1f);
			viewController.rectTransform.sizeDelta = new UnityEngine.Vector2(0f, 0f);
			viewController.rectTransform.anchoredPosition = new UnityEngine.Vector2(0f, 0f);
			viewController.vrInputModule = UnityEngine.Resources.FindObjectsOfTypeAll<VRUIControls.VRInputModule>()[0];

			UnityEngine.Transform Wrapper = gameObject.transform.GetChild(0);
			UnityEngine.RectTransform editHostname = UI.CreateTextbox(Wrapper, "EditHostname", 1, viewController.RefreshStatusPlaceholder, "BEATUPCLIENT_ENTER_HOSTNAME");
			UnityEngine.RectTransform editStatus = UI.CreateTextbox(Wrapper, "EditStatus", 2, viewController.RefreshStatusPlaceholder);
			editHostname.sizeDelta = new UnityEngine.Vector2(80, editHostname.sizeDelta.y);
			editHostname.localPosition = new UnityEngine.Vector3(0, -1.5f, 0);
			editStatus.sizeDelta = new UnityEngine.Vector2(80, editStatus.sizeDelta.y);
			editStatus.localPosition = new UnityEngine.Vector3(0, -8.5f, 0);
			viewController.editHostnameTextbox = editHostname.GetComponent<HMUI.InputFieldView>();
			viewController.editStatusTextbox = editStatus.GetComponent<HMUI.InputFieldView>();
			viewController.editHostnameTextbox._textLengthLimit = 41;
			viewController.editStatusTextbox._textLengthLimit = 41;
			viewController.editStatusPlaceholder = viewController.editStatusTextbox._placeholderText.GetComponent<HMUI.CurvedTextMeshPro>();
			viewController.cancelButton = Wrapper.Find("Buttons/CancelButton").GetComponent<HMUI.NoTransitionsButton>();

			viewController.keyboard = gameObject.GetComponentInChildren<HMUI.UIKeyboard>();
			foreach(UnityEngine.RectTransform tr in new[] {viewController.keyboard.transform.parent, viewController.keyboard.transform})
				tr.sizeDelta = new UnityEngine.Vector2(tr.sizeDelta.x + 7, tr.sizeDelta.y);
			UnityEngine.Transform Letters = viewController.keyboard.transform.GetChild(0);
			UI.CreateKey(viewController.keyboard, Letters.GetChild(0).Find("P"), UnityEngine.KeyCode.Slash, '/');
			UI.CreateKey(viewController.keyboard, Letters.GetChild(1).Find("L"), UnityEngine.KeyCode.Colon, ':');
			UI.CreateKey(viewController.keyboard, Letters.GetChild(2).Find("Del"), UnityEngine.KeyCode.Period, '.');

			gameObject.SetActive(false);
			return viewController;
		}

		protected override void DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
			if(firstActivation)
				base.buttonBinder.AddBinding(cancelButton, () => Dismiss());
			string? status = null;
			if(edit && Config.Instance.Servers.TryGetValue(customServerHostName, out status))
				activeKey = customServerHostName;
			else
				activeKey = null;
			editHostnameTextbox.SetText(activeKey ?? "");
			editHostnameTextbox.onValueChanged.Invoke(editHostnameTextbox);
			editStatusTextbox.SetText(status ?? "");
			editHostnameTextbox.ActivateKeyboard(keyboard);
			editHostnameTextbox.UpdateClearButton();
			keyboard.okButtonWasPressedEvent += HandleOkButtonWasPressed;
			vrInputModule.onProcessMousePressEvent += ProcessMousePress;
		}

		protected override void DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
			vrInputModule.onProcessMousePressEvent -= ProcessMousePress;
			keyboard.okButtonWasPressedEvent -= HandleOkButtonWasPressed;
			editStatusTextbox.DeactivateKeyboard(keyboard);
			editHostnameTextbox.DeactivateKeyboard(keyboard);
			flowCoordinator = null;
		}

		void ProcessMousePress(UnityEngine.GameObject gameObject) {
			HMUI.InputFieldView inputField = gameObject.GetComponent<HMUI.InputFieldView>();
			if(inputField == editHostnameTextbox) {
				editStatusTextbox.DeactivateKeyboard(keyboard);
				editHostnameTextbox.ActivateKeyboard(keyboard);
				editHostnameTextbox.UpdateClearButton();
			} else if(inputField == editStatusTextbox) {
				editHostnameTextbox.DeactivateKeyboard(keyboard);
				editStatusTextbox.ActivateKeyboard(keyboard);
				editStatusTextbox.UpdateClearButton();
			}
		}

		void RefreshStatusPlaceholder(HMUI.InputFieldView textbox) {
			string text = editHostnameTextbox.text.Split(new[] {':'})[0];
			bool enable = !string.IsNullOrEmpty(text);
			if(enable != editStatusTextbox.gameObject.activeSelf) {
				editStatusTextbox.gameObject.SetActive(enable);
				if(!enable)
					editStatusTextbox.DeactivateKeyboard(keyboard);
			}
			if(enable)
				editStatusPlaceholder.text = "https://status." + text;
		}

		public void TryPresent(HMUI.FlowCoordinator flowCoordinator, bool edit) {
			if(this.flowCoordinator == null && flowCoordinator is MultiplayerModeSelectionFlowCoordinator) {
				this.flowCoordinator = flowCoordinator;
				this.edit = edit;
				flowCoordinator.PresentViewController(this, null, HMUI.ViewController.AnimationDirection.Vertical, false);
				flowCoordinator.SetTitle(Polyglot.Localization.Get(edit ? "BEATUPCLIENT_EDIT_SERVER" : "BEATUPCLIENT_ADD_SERVER"), HMUI.ViewController.AnimationType.In);
				flowCoordinator._screenSystem.SetBackButton(false, true);
			}
		}

		public void Dismiss(bool immediately = false) {
			if(flowCoordinator != null)
				flowCoordinator.DismissViewController(this, HMUI.ViewController.AnimationDirection.Vertical, null, immediately);
		}

		void HandleOkButtonWasPressed() {
			if(activeKey != null)
				Config.Instance.Servers.Remove(activeKey);
			string hostname = editHostnameTextbox.text;
			if(hostname.Length >= 1 && hostname.IndexOf(':') != 0)
				Config.Instance.Servers[editHostnameTextbox.text] = editStatusTextbox.text.Length < 1 ? null : editStatusTextbox.text;
			else
				hostname = "";
			if(customServerHostName.value.Equals(hostname))
				RefreshNetworkConfig();
			customServerHostName.value = hostname;
			RefreshDropdown();
			Dismiss();
		}
	}

	public static EditServerViewController editServerViewController = null!;

	static UI.DropdownSetting serverDropdown = null!;
	static void RefreshDropdown() =>
		serverDropdown.SetOptions(Config.Instance.Servers.Keys);

	public static void OnSceneLoaded(UnityEngine.SceneManagement.Scene scene, UnityEngine.SceneManagement.LoadSceneMode mode) {
		if(scene.name != "MainMenu")
			return;
		Log?.Debug("load MainMenu");
		if(customNetworkConfig != null) {
			UnityEngine.Transform CreateServerFormView = UnityEngine.Resources.FindObjectsOfTypeAll<CreateServerFormController>()[0].transform;
			UI.CreateValuePicker(CreateServerFormView, "CountdownDuration", "BEATUPCLIENT_COUNTDOWN_DURATION", () => ref Config.Instance.CountdownDuration, new byte[] {(byte)(Config.Instance.CountdownDuration * 4), 0, 12, 20, 32, 40, 60});
			UI.CreateToggle(CreateServerFormView, "SkipResults", "BEATUPCLIENT_SKIP_RESULTS_PYRAMID", () => ref Config.Instance.SkipResults);
			UI.CreateToggle(CreateServerFormView, "PerPlayerDifficulty", "BEATUPCLIENT_PER_PLAYER_DIFFICULTY", () => ref Config.Instance.PerPlayerDifficulty);
			UI.CreateToggle(CreateServerFormView, "PerPlayerModifiers", "BEATUPCLIENT_PER_PLAYER_MODIFIERS", () => ref Config.Instance.PerPlayerModifiers);
			CreateServerFormView.parent.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
			CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
			CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.ContentSizeFitter>().enabled = true;
			CreateServerFormView.parent.parent.gameObject.SetActive(true);

			mainFlowCoordinator = UnityEngine.Resources.FindObjectsOfTypeAll<MainFlowCoordinator>()[0];
			MultiplayerModeSelectionViewController multiplayerModeSelectionViewController = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerModeSelectionViewController>()[0];
			TMPro.TextMeshProUGUI customServerEndPointText = multiplayerModeSelectionViewController._customServerEndPointText;
			UnityEngine.UI.Button editColorSchemeButton = UnityEngine.Resources.FindObjectsOfTypeAll<ColorsOverrideSettingsPanelController>()[0]._editColorSchemeButton;
			customServerEndPointText.enabled = false;
			if(editServerViewController == null)
				editServerViewController = EditServerViewController.Create("BeatUpClient_EditServerView", multiplayerModeSelectionViewController.transform.parent);
			UnityEngine.RectTransform server = UI.CreateSimpleDropdown(customServerEndPointText.transform, "Server", customServerHostName, Config.Instance.Servers.Keys);
			server.sizeDelta = new UnityEngine.Vector2(80, server.sizeDelta.y);
			server.localPosition = new UnityEngine.Vector3(0, 39.5f, 0);
			foreach(UnityEngine.Transform tr in server)
				tr.localPosition = new UnityEngine.Vector3(0, 0, 0);
			serverDropdown = server.GetComponent<UI.DropdownSetting>();
			UnityEngine.RectTransform addButton = UI.CreateButtonFrom(editColorSchemeButton.gameObject, customServerEndPointText.transform, "AddServer", () =>
				editServerViewController.TryPresent(mainFlowCoordinator.childFlowCoordinator, false));
			addButton.localPosition = new UnityEngine.Vector3(-40, 39.5f, 0);
			addButton.Find("Icon").GetComponent<HMUI.ImageView>().sprite = System.Linq.Enumerable.First(UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.Sprite>(), sprite => sprite.name == "AddIcon");
			UnityEngine.RectTransform editButton = UI.CreateButtonFrom(editColorSchemeButton.gameObject, customServerEndPointText.transform, "EditServer", () =>
				editServerViewController.TryPresent(mainFlowCoordinator.childFlowCoordinator, true));
			editButton.localPosition = new UnityEngine.Vector3(52, 39.5f, 0);
			editServerButton = editButton.GetComponent<UnityEngine.UI.Button>();

			customServerHostName.didChangeEvent -= RefreshNetworkConfig;
			customServerHostName.didChangeEvent += RefreshNetworkConfig;
			RefreshNetworkConfig();
		}

		DifficultyPanel.Init();
		lobbyDifficultyPanel = new DifficultyPanel(UnityEngine.Resources.FindObjectsOfTypeAll<LobbySetupViewController>()[0].transform.GetChild(0), 2, 90);
	}

	static void WarmMethods(System.Type type) {
		// Log?.Debug($"WarmMethods({type})");
		foreach(System.Type nested in type.GetNestedTypes())
			WarmMethods(nested);
		foreach(System.Reflection.MethodInfo method in type.GetMethods(System.Reflection.BindingFlags.DeclaredOnly | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Static))
			if(!method.ContainsGenericParameters)
				method.MethodHandle.GetFunctionPointer(); // This forces compilation, catching `TypeLoadException`s at load time
	}

	[IPA.Init]
	public void Init(IPA.Logging.Logger pluginLogger, IPA.Config.Config conf) {
		Instance = this;
		Log = pluginLogger;
		Config.Instance = IPA.Config.Stores.GeneratedStore.Generated<Config>(conf);
		Log?.Debug("Logger initialized.");
	}
	[IPA.OnEnable]
	public void OnEnable() {
		string localization = "Polyglot\t100\n" +
			"BEATUPCLIENT_COUNTDOWN_DURATION\t\tCountdown Duration\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUPCLIENT_SKIP_RESULTS_PYRAMID\t\tSkip Results Pyramid\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUPCLIENT_PER_PLAYER_DIFFICULTY\t\tPer-Player Difficulty\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUPCLIENT_PER_PLAYER_MODIFIERS\t\tPer-Player Modifiers\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUPCLIENT_ADD_SERVER\t\tAdd Server\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUPCLIENT_EDIT_SERVER\t\tEdit Server\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUPCLIENT_ENTER_HOSTNAME\t\tEnter Hostname\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" + 
			"BEATUPCLIENT_SWITCH\t\tSwitch\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n";
		Polyglot.LocalizationImporter.Import(localization, Polyglot.GoogleDriveDownloadFormat.TSV);

		haveSiraUtil = (IPA.Loader.PluginManager.GetPluginFromId("SiraUtil") != null);
		haveSongCore = (IPA.Loader.PluginManager.GetPluginFromId("SongCore") != null);
		haveMpCore = (IPA.Loader.PluginManager.GetPluginFromId("MultiplayerCore") != null);
		Log?.Debug($"haveSiraUtil={haveSiraUtil}");
		Log?.Debug($"haveSongCore={haveSongCore}");
		Log?.Debug($"haveMpCore={haveMpCore}");

		try {
			System.Collections.Generic.List<System.Type> sections = new System.Collections.Generic.List<System.Type>();
			sections.Add(typeof(BeatUpClient));
			if(haveSongCore)
				sections.Add(typeof(BeatUpClient_SongCore));
			if(haveMpCore) {
				sections.Add(typeof(BeatUpClient_MpCore));
			}
			Log?.Debug("Warming methods");
			foreach(System.Type type in sections)
				WarmMethods(type);
			Log?.Debug("Loading assets");
			UnityEngine.AssetBundle data = UnityEngine.AssetBundle.LoadFromStream(System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream("BeatUpClient.data"));
			defaultPackCover = data.LoadAllAssets<UnityEngine.Sprite>()[0];
			badges = data.LoadAllAssets<UnityEngine.GameObject>();
			Log?.Debug("Applying patches");
			BeatUpClient_MpCore.Patch();
			foreach(System.Type type in sections)
				PatchAll(type);
			UnityEngine.SceneManagement.SceneManager.sceneLoaded += OnSceneLoaded;
		} catch(System.Exception ex) {
			Log?.Error("Error applying patches: " + ex.Message);
			Log?.Debug(ex);
			OnDisable();
		}
	}
	[IPA.OnDisable]
	public void OnDisable() {
		try {
			UnityEngine.SceneManagement.SceneManager.sceneLoaded -= OnSceneLoaded;
			harmony.UnpatchSelf();
			BeatUpClient_MpCore.Unpatch();
		} catch(System.Exception ex) {
			Log?.Error("Error removing patches: " + ex.Message);
			Log?.Debug(ex);
		}
	}
}

static class BeatUpClient_SongCore {
	static string? HashForLevelID(string? levelId) {
		string[] parts = (levelId ?? "").Split('_', ' ');
		if(parts.Length < 3 || parts[2].Length != 40)
			return null;
		return parts[2];
	}
	public static void GetPreviewInfo(CustomPreviewBeatmapLevel previewBeatmapLevel, ref System.Collections.Generic.IEnumerable<string?> requirements, ref System.Collections.Generic.IEnumerable<string?> suggestions) {
		string? levelHash = HashForLevelID(previewBeatmapLevel.levelID);
		if(NullableStringHelper.IsNullOrEmpty(levelHash))
			return;
		SongCore.Data.ExtraSongData? extraSongData = SongCore.Collections.RetrieveExtraSongData(levelHash);
		if(extraSongData == null)
			return;
		requirements = System.Linq.Enumerable.SelectMany(extraSongData._difficulties, diff => diff.additionalDifficultyData._requirements);
		suggestions = System.Linq.Enumerable.SelectMany(extraSongData._difficulties, diff => diff.additionalDifficultyData._suggestions);
	}
}

static class BeatUpClient_MpCore {
	#if MPCORE_SUPPORT
	static class DiJack {
		static System.Collections.Generic.Dictionary<System.Type, System.Type?> InjectMap = new System.Collections.Generic.Dictionary<System.Type, System.Type?>();
		static void ConcreteBinderNonGeneric_To(Zenject.ConcreteBinderNonGeneric __instance, ref System.Collections.Generic.IEnumerable<System.Type> concreteTypes) {
			System.Type[] newTypes = System.Linq.Enumerable.ToArray(concreteTypes);
			uint i = 0;
			foreach(System.Type type in concreteTypes) {
				if(InjectMap.TryGetValue(type, out System.Type? inject)) {
					if(inject == null) {
						Log?.Debug($"Suppressing {type}");
						concreteTypes = new System.Type[0];
						return;
					}
					Log?.Debug($"Replacing {type} with {inject}");
					newTypes[i] = inject;
					__instance.BindInfo.ContractTypes.Add(inject);
				}
				++i;
			}
			concreteTypes = newTypes;
		}

		public static System.Collections.Generic.IEnumerable<System.Type> RegisteredTypes() =>
			InjectMap.Keys;

		public static void Suppress(System.Type original) =>
			InjectMap.Add(original, null);

		public static void Register(System.Type type) =>
			InjectMap.Add(type.BaseType, type);

		public static void UnregisterAll() =>
			InjectMap.Clear();

		public static void Patch() {
			System.Reflection.MethodInfo original = typeof(Zenject.ConcreteBinderNonGeneric).GetMethod(nameof(Zenject.ConcreteBinderNonGeneric.To), new[] {typeof(System.Collections.Generic.IEnumerable<System.Type>)});
			System.Reflection.MethodInfo prefix = typeof(DiJack).GetMethod(nameof(ConcreteBinderNonGeneric_To), System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic);
			harmony.Patch(original, prefix: new HarmonyLib.HarmonyMethod(prefix));
		}
	}

	public static async System.Threading.Tasks.Task<EntitlementsStatus> MpShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> task, string levelId, NetworkPlayerEntitlementChecker checker) {
		EntitlementsStatus status = await task;
		if(status != EntitlementsStatus.NotOwned)
			return status;
		return await checker._additionalContentModel.GetLevelEntitlementStatusAsync(levelId, default(System.Threading.CancellationToken)) switch {
			AdditionalContentModel.EntitlementStatus.Owned => EntitlementsStatus.Ok, 
			AdditionalContentModel.EntitlementStatus.NotOwned => EntitlementsStatus.NotOwned, 
			_ => EntitlementsStatus.Unknown, 
		};
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpEntitlementChecker), "GetEntitlementStatus")]
	public static void MpEntitlementChecker_GetEntitlementStatus(MultiplayerCore.Objects.MpEntitlementChecker __instance, ref System.Threading.Tasks.Task<EntitlementsStatus> __result, string levelId) {
		Log?.Debug($"NetworkPlayerEntitlementChecker_GetEntitlementStatus");
		__result = MpShareWrapper(__result, levelId, __instance);
		__result = ShareWrapper(__result, levelId);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.LoadLevel))]
	public static void MpLevelLoader_LoadLevel_pre(int ____loaderState, out bool __state) =>
		__state = (____loaderState == (int)MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.NotLoading);

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.LoadLevel))]
	public static void MpLevelLoader_LoadLevel_post(MultiplayerCore.Objects.MpLevelLoader __instance, bool __state, ILevelGameplaySetupData gameplaySetupData, float initialStartTime, System.Threading.CancellationTokenSource ____getBeatmapCancellationTokenSource, ref System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> ____getBeatmapLevelResultTask) =>
		MultiplayerLevelLoader_LoadLevel_post(__instance, __state, gameplaySetupData, initialStartTime, ____getBeatmapCancellationTokenSource, ref ____getBeatmapLevelResultTask);

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.Report))]
	public static void MpLevelLoader_Report(double value) =>
		Progress(new PacketHandler.LoadProgress(PacketHandler.LoadProgress.LoadState.Downloading, (ushort)(value * 65535)));

	class PlayersDataModel : MultiplayerCore.Objects.MpPlayersDataModel, ILobbyPlayersDataModel, System.IDisposable {
		public PlayersDataModel(MultiplayerCore.Networking.MpPacketSerializer packetSerializer, MultiplayerCore.Beatmaps.Providers.MpBeatmapLevelProvider beatmapLevelProvider, SiraUtil.Logging.SiraLog logger) : base(packetSerializer, beatmapLevelProvider, logger) {}

		public override void Activate() {
			Log?.Debug("MpPlayersDataModel.Activate()");
			base.Activate();
			_packetSerializer.UnregisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>();
			_packetSerializer.RegisterCallback<MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket>(HandleMpexBeatmapPacket);
			_menuRpcManager.recommendBeatmapEvent -= base.HandleMenuRpcManagerRecommendBeatmap;
			_menuRpcManager.recommendBeatmapEvent += this.HandleRecommendBeatmap;
		}

		public override void Deactivate() {
			Log?.Debug("MpPlayersDataModel.Deactivate()");
			_menuRpcManager.recommendBeatmapEvent -= this.HandleRecommendBeatmap;
			_menuRpcManager.recommendBeatmapEvent += base.HandleMenuRpcManagerRecommendBeatmap;
			base.Deactivate();
		}

		private void HandleMpexBeatmapPacket(MultiplayerCore.Beatmaps.Packets.MpBeatmapPacket packet, IConnectedPlayer player) {
			BeatmapCharacteristicSO? characteristic = _beatmapCharacteristicCollection.GetBeatmapCharacteristicBySerializedName(packet.characteristic);
			IPreviewBeatmapLevel preview = _beatmapLevelProvider.GetBeatmapFromPacket(packet);
			PacketHandler.NetworkPreviewBeatmapLevel current = playerPreviews[player.sortIndex].preview;
			if(preview.levelID != current.levelID || current.previewDifficultyBeatmapSets == null) // Ignore if we already have a BeatUpClient preview for this level
				current.Init(preview);
		}

		void HandleRecommendBeatmap(string userId, BeatmapIdentifierNetSerializable beatmapId) =>
			Reflection.BaseBaseMethod<string, BeatmapIdentifierNetSerializable>(this, "HandleMenuRpcManagerRecommendBeatmap")(userId, beatmapId);

		public override void SetLocalPlayerBeatmapLevel(PreviewDifficultyBeatmap? beatmapLevel) {
			if(beatmapLevel == null)
				Reflection.BaseBaseMethod<PreviewDifficultyBeatmap?>(this, "SetLocalPlayerBeatmapLevel")(beatmapLevel);
			else
				base.SetLocalPlayerBeatmapLevel(beatmapLevel);
		}
	}

	public static void Patch() {
		DiJack.Suppress(typeof(MultiplayerCore.Patchers.CustomLevelsPatcher));
		DiJack.Suppress(typeof(MultiplayerCore.Patchers.ModeSelectionPatcher));
		DiJack.Register(typeof(PlayersDataModel));
		DiJack.Patch();
	}
	public static void Unpatch() =>
		DiJack.UnregisterAll();
	#else
	public static void Patch() =>
		throw new System.Exception("Not built with MultiplayerCore support");
	public static void Unpatch() {}
	#endif
}
