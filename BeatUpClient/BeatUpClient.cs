#define ENABLE_ZIP_PROGRESS
using static System.Linq.Enumerable;
using static BeatUpClient;
using static Extensions;
#nullable enable
namespace System.Diagnostics.CodeAnalysis {
	[AttributeUsage(AttributeTargets.Parameter, Inherited = false)]
	internal sealed class NotNullWhenAttribute : Attribute {
		public bool ReturnValue {get;}
		public NotNullWhenAttribute(bool returnValue) =>
			ReturnValue = returnValue;
	}
}

static class Extensions {
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	public static bool Or(this bool res, System.Action func) {
		if(!res)
			func();
		return res;
	}
}

[IPA.Plugin(IPA.RuntimeOptions.SingleStartInit)]
public class BeatUpClient {
	public const ulong MaxDownloadSize = 268435456;
	public const ulong MaxUnzippedSize = 268435456;

	public class Config {
		public static Config Instance = new Config();
		public static System.Action? onReload = null;
		public virtual float CountdownDuration {get; set;} = 5;
		public virtual bool SkipResults {get; set;} = false;
		public virtual bool PerPlayerDifficulty {get; set;} = false;
		public virtual bool PerPlayerModifiers {get; set;} = false;
		public virtual bool HideOtherLevels {get; set;} = false;
		public ushort WindowSize {get; set;} = LiteNetLib.NetConstants.DefaultWindowSize;
		public bool UnreliableState {get; set;} = false;
		public bool DirectDownloads {get; set;} = true;
		public bool AllowModchartDownloads {get; set;} = false;
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

	public class BeatUpConnectInfo : LiteNetLib.Utils.INetSerializable {
		public const uint Size = 6;
		public uint windowSize = LiteNetLib.NetConstants.DefaultWindowSize;
		public byte countdownDuration = 20;
		public bool directDownloads = false;
		public bool skipResults = false;
		public bool perPlayerDifficulty = false;
		public bool perPlayerModifiers = false;
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
	static BeatUpConnectInfo? connectInfo = null;
	public static uint windowSize = LiteNetLib.NetConstants.DefaultWindowSize; // Needed by transpiler
	static UnityEngine.GameObject infoText = null!;

	public enum PatchType : byte {
		Prefix,
		Postfix,
		Transpiler,
	}

	delegate T CreateArrayCallback<T>(uint i);
	static T[] CreateArray<T>(uint count, CreateArrayCallback<T> cb) {
		T[] arr = new T[count];
		for(uint i = 0; i < count; ++i)
			arr[(int)i] = cb(i);
		return arr;
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

	[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
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
			PersistentSingleton<SharedCoroutineStarter>.instance.StartCoroutine(Refresh(this));
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
				PersistentSingleton<SharedCoroutineStarter>.instance.StartCoroutine(Refresh(this));
		}
		static System.Collections.IEnumerator Refresh(PlayerCell cell) { // Creates a copy of `cell`
			yield return null; // Avoids threading issues that lead to memory corruption in the Unity UI
			float delta = (65535u - cell.data.progress) * -104 / 65534f;
			cell.transform!.sizeDelta = new UnityEngine.Vector2(delta, cell.transform.sizeDelta.y);
			cell.transform.localPosition = new UnityEngine.Vector3(delta * (cell.data.state == PacketHandler.LoadProgress.LoadState.Exporting ? .5f : -.5f), 0, 0);
			cell.foreground.color = cell.data.state switch {
				PacketHandler.LoadProgress.LoadState.Exporting => new UnityEngine.Color(0.9130435f, 1, 0.1521739f, 0.5434783f),
				PacketHandler.LoadProgress.LoadState.Downloading => new UnityEngine.Color(0.4782609f, 0.6956522f, 0.02173913f, 0.5434783f),
				_ => new UnityEngine.Color(0, 0, 0, 0),
			};
			cell.background.color = cell.data.state switch {
				PacketHandler.LoadProgress.LoadState.Failed => new UnityEngine.Color(0.7173913f, 0.2608696f, 0.02173913f, 0.7490196f),
				PacketHandler.LoadProgress.LoadState.Exporting => new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f),
				PacketHandler.LoadProgress.LoadState.Downloading => new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f),
				PacketHandler.LoadProgress.LoadState.Done => new UnityEngine.Color(0.2608696f, 0.7173913f, 0.02173913f, 0.7490196f),
				_ => cell.backgroundColor,
			};
			cell.foreground.enabled = (cell.data.progress >= 1);
			cell.background.enabled = true;
		}
	}

	public struct PlayerData {
		public struct ModifiersWeCareAbout {
			public GameplayModifiers.SongSpeed songSpeed;
			public readonly bool disappearingArrows;
			public readonly bool ghostNotes;
			public readonly bool smallCubes;
			public ModifiersWeCareAbout(GameplayModifiers src) =>
				(songSpeed, disappearingArrows, ghostNotes, smallCubes) = (src.songSpeed, src.disappearingArrows, src.ghostNotes, src.smallCubes);
		}
		public readonly PacketHandler.RecommendPreview[] previews;
		public readonly PlayerCell[] cells;
		public readonly ModifiersWeCareAbout[] modifiers;
		public readonly ModifiersWeCareAbout[] lockedModifiers;
		public readonly System.Collections.Generic.Dictionary<string, byte> tiers;
		public PlayerData(int playerCount) {
			previews = new PacketHandler.RecommendPreview[playerCount];
			cells = new PlayerCell[playerCount];
			modifiers = new ModifiersWeCareAbout[playerCount];
			lockedModifiers = new ModifiersWeCareAbout[playerCount];
			tiers = new System.Collections.Generic.Dictionary<string, byte>();
			System.Array.Fill(previews, new PacketHandler.RecommendPreview());
			System.Array.Fill(modifiers, new ModifiersWeCareAbout {songSpeed = (GameplayModifiers.SongSpeed)255});
			System.Array.Fill(lockedModifiers, new ModifiersWeCareAbout {songSpeed = (GameplayModifiers.SongSpeed)255});
		}
		public void Reset(int index) {
			cells[index].SetData(new PacketHandler.LoadProgress(PacketHandler.LoadProgress.LoadState.None, 0, 0));
			lockedModifiers[index].songSpeed = modifiers[index].songSpeed = (GameplayModifiers.SongSpeed)255;
		}
		public PacketHandler.RecommendPreview? ResolvePreview(string levelId) =>
			previews.FirstOrDefault((PacketHandler.RecommendPreview preview) => preview.levelID == levelId);
		public PacketHandler.RecommendPreview SetPreviewFromLocal(int index, CustomPreviewBeatmapLevel previewBeatmapLevel) {
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
			string?[] requirementArray = requirements.ToHashSet().ToArray();
			string?[] suggestionArray = suggestions.ToHashSet().ToArray();
			if(requirementArray.Length >= 1)
				Log?.Debug(requirementArray.Aggregate($"{requirementArray.Length} requirements for `{previewBeatmapLevel.levelID}`:", (str, req) => $"{str}\n    {req}"));
			else
				Log?.Debug($"No requirements for `{previewBeatmapLevel.levelID}`");
			return (previews[index] = new PacketHandler.RecommendPreview(previewBeatmapLevel, requirementArray, suggestionArray));
		}
	}
	public static PlayerData playerData = new PlayerData(0);

	public static string? HashForLevelID(string? levelId) {
		string[] parts = (levelId ?? "").Split('_', ' ');
		if(parts.Length < 3 || parts[2].Length != 40)
			return null;
		return parts[2];
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
			bool mpCore;
			public string? levelID {get; set;} = string.Empty;
			public string? songName {get; set;} = string.Empty;
			public string? songSubName {get; set;} = string.Empty;
			public string? songAuthorName {get; set;} = string.Empty;
			public string? levelAuthorName {get; set;} = string.Empty;
			public float beatsPerMinute {get; set;}
			public float songTimeOffset {get; set;}
			public float shuffle {get; set;}
			public float shufflePeriod {get; set;}
			public float previewStartTime {get; set;}
			public float previewDuration {get; set;}
			public float songDuration {get; set;}
			public System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet>? previewDifficultyBeatmapSets {get; set;} = null;
			public EnvironmentInfoSO? environmentInfo {get; set;} = null;
			public EnvironmentInfoSO? allDirectionsEnvironmentInfo {get; set;} = null;
			public readonly ByteArrayNetSerializable cover = new ByteArrayNetSerializable("cover", 0, 8192);
			public System.Threading.Tasks.Task<byte[]> coverRenderTask = System.Threading.Tasks.Task.FromResult<byte[]>(new byte[0]);
			public async System.Threading.Tasks.Task<UnityEngine.Sprite> GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) {
				Log?.Debug("NetworkPreviewBeatmapLevel.GetCoverImageAsync()");
				return await new MemorySpriteLoader(coverRenderTask).LoadSpriteAsync("", cancellationToken) ?? defaultPackCover;
			}
			public async System.Threading.Tasks.Task<byte[]> RenderCoverImageAsync(IPreviewBeatmapLevel beatmapLevel) {
				UnityEngine.Sprite fullSprite = await beatmapLevel.GetCoverImageAsync(System.Threading.CancellationToken.None);
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
				writer.Put((string?)(mpCore ? HashForLevelID(levelID) : levelID));
				writer.Put((string?)songName);
				writer.Put((string?)songSubName);
				writer.Put((string?)songAuthorName);
				writer.Put((string?)levelAuthorName);
				writer.Put((float)beatsPerMinute);
				if(!mpCore) {
					writer.Put((float)songTimeOffset);
					writer.Put((float)shuffle);
					writer.Put((float)shufflePeriod);
					writer.Put((float)previewStartTime);
					writer.Put((float)previewDuration);
				}
				writer.Put((float)songDuration);
				if(!mpCore) {
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
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				levelID = reader.GetString();
				if(mpCore)
					levelID = $"custom_level_{levelID}";
				songName = reader.GetString();
				songSubName = reader.GetString();
				songAuthorName = reader.GetString();
				levelAuthorName = reader.GetString();
				beatsPerMinute = reader.GetFloat();
				if(!mpCore) {
					songTimeOffset = reader.GetFloat();
					shuffle = reader.GetFloat();
					shufflePeriod = reader.GetFloat();
					previewStartTime = reader.GetFloat();
					previewDuration = reader.GetFloat();
				}
				songDuration = reader.GetFloat();
				if(!mpCore) {
					uint count = UpperBound(reader.GetByte(), 8);
					previewDifficultyBeatmapSets = (count < 1) ? null : CreateArray(count, i =>
						new PreviewDifficultyBeatmapSet(handler.characteristics.GetBeatmapCharacteristicBySerializedName(reader.GetString()), CreateArray(UpperBound(reader.GetByte(), 5), i =>
							(BeatmapDifficulty)reader.GetVarUInt())));
					cover.Deserialize(reader);
					coverRenderTask = System.Threading.Tasks.Task.FromResult<byte[]>(cover.data);
				}
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
				if(!mpCore)
					coverRenderTask = RenderCoverImageAsync(beatmapLevel);
				environmentInfo = beatmapLevel.environmentInfo;
				allDirectionsEnvironmentInfo = beatmapLevel.allDirectionsEnvironmentInfo;
				Log?.Debug($"NetworkPreviewBeatmapLevel.Init(beatmapLevel={beatmapLevel}, previewDifficultyBeatmapSets={previewDifficultyBeatmapSets}, mpCore={mpCore})");
			}
			public NetworkPreviewBeatmapLevel(bool mpCore) =>
				this.mpCore = mpCore;
			public NetworkPreviewBeatmapLevel(IPreviewBeatmapLevel beatmapLevel, bool mpCore) : this(mpCore) =>
				Init(beatmapLevel);
		}

		public class RecommendPreview : NetworkPreviewBeatmapLevel, LiteNetLib.Utils.INetSerializable {
			public string?[] requirements = new string[0];
			public string?[] suggestions = new string[0];
			void LiteNetLib.Utils.INetSerializable.Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				base.Serialize(writer);
				writer.PutVarUInt(UpperBound((uint)requirements.Length, 16));
				foreach(string? requirement in requirements)
					writer.Put((string?)requirement);
				writer.PutVarUInt(UpperBound((uint)suggestions.Length, 16));
				foreach(string? suggestion in suggestions)
					writer.Put((string?)suggestion);
			}
			void LiteNetLib.Utils.INetSerializable.Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				base.Deserialize(reader);
				requirements = CreateArray(UpperBound(reader.GetVarUInt(), 16), i => reader.GetString());
				suggestions = CreateArray(UpperBound(reader.GetVarUInt(), 16), i => reader.GetString());
			}
			public RecommendPreview() : base(false) {}
			public RecommendPreview(IPreviewBeatmapLevel beatmapLevel, string?[] requirements, string?[] suggestions) : base(beatmapLevel, false) =>
				(this.requirements, this.suggestions) =  (requirements, suggestions);
		}

		public abstract class ShareInfo : LiteNetLib.Utils.INetSerializable {
			public string levelId = string.Empty;
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
			public ShareInfo(string levelId, System.ReadOnlySpan<byte> levelHash, ulong fileSize) {
				Log?.Debug($"ShareInfo()");
				this.levelId = levelId;
				levelHash.CopyTo(this.levelHash);
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
			public SetCanShareBeatmap(string levelId, System.ReadOnlySpan<byte> levelHash, ulong fileSize, bool canShare = true) : base(levelId, levelHash, fileSize) {
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
				sourcePlayers = CreateArray(UpperBound(reader.GetByte(), 128), i => reader.GetString());
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
			public LevelFragmentRequest(ulong offset, ushort maxSize) =>
				(this.offset, this.maxSize) = (offset, maxSize);
		}

		public class LevelFragment : LiteNetLib.Utils.INetSerializable {
			public ulong offset;
			public System.ArraySegment<byte> data;
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				writer.PutVarULong(offset);
				writer.Put((ushort)UpperBound((uint)data.Count, 1500));
				writer.Put(data.Array, data.Offset, data.Count);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				offset = reader.GetVarULong();
				data = new System.ArraySegment<byte>(new byte[UpperBound(reader.GetUShort(), 1500)]);
				reader.GetBytes(data.Array, data.Offset, data.Count);
			}
			public LevelFragment() {}
			public LevelFragment(ulong offset, System.ArraySegment<byte> data) =>
				(this.offset, this.data) = (offset, data);
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
			public LoadProgress(LoadState state, ushort progress, uint sequence) =>
				(this.sequence, this.state, this.progress) = (sequence, state, progress);
			public LoadProgress(LoadState state, ushort progress) : this(state, progress, ++localSequence) {}
		}

		public class MpBeatmapPacket : NetworkPreviewBeatmapLevel, LiteNetLib.Utils.INetSerializable {
			public string? characteristic = string.Empty;
			public BeatmapDifficulty difficulty;
			void LiteNetLib.Utils.INetSerializable.Serialize(LiteNetLib.Utils.NetDataWriter writer) {
				base.Serialize(writer);
				writer.Put((string?)characteristic);
				writer.Put((uint)difficulty);
			}
			void LiteNetLib.Utils.INetSerializable.Deserialize(LiteNetLib.Utils.NetDataReader reader) {
				base.Deserialize(reader);
				characteristic = reader.GetString();
				difficulty = (BeatmapDifficulty)reader.GetUInt();
			}
			public MpBeatmapPacket() : base(true) {}
			public MpBeatmapPacket(PreviewDifficultyBeatmap beatmap) : base(beatmap.beatmapLevel, true) =>
				(characteristic, difficulty) = (beatmap.beatmapCharacteristic.serializedName, beatmap.beatmapDifficulty);
		}

		class MpFallbackSerializer : INetworkPacketSubSerializer<IConnectedPlayer> {
			public void Serialize(LiteNetLib.Utils.NetDataWriter writer, LiteNetLib.Utils.INetSerializable packet) {
				writer.Put((string)packet.GetType().Name);
				packet.Serialize(writer);
			}
			public void Deserialize(LiteNetLib.Utils.NetDataReader reader, int length, IConnectedPlayer player) {
				int end = reader.Position + length;
				if(reader.GetString() == "MpBeatmapPacket") {
					MpBeatmapPacket packet = new MpBeatmapPacket();
					packet.Deserialize(reader);
					PacketHandler.RecommendPreview current = playerData.previews[player.sortIndex];
					if(packet.levelID != current.levelID || current.previewDifficultyBeatmapSets == null) // Ignore if we already have a BeatUpClient preview for this level
						current.Init(packet);
				}
				reader.SkipBytes(end - reader.Position);
			}
			public bool HandlesType(System.Type type) =>
				type == typeof(MpBeatmapPacket);
		}

		const MultiplayerSessionManager.MessageType messageType = (MultiplayerSessionManager.MessageType)101;
		const MultiplayerSessionManager.MessageType mpMessageType = (MultiplayerSessionManager.MessageType)100;
		public Zenject.DiContainer container;
		public IMenuRpcManager rpcManager;
		public BeatmapCharacteristicCollectionSO characteristics;
		public MultiplayerSessionManager multiplayerSessionManager;
		NetworkPacketSerializer<MessageType, IConnectedPlayer> serializer;
		MpFallbackSerializer? mpSerializer = null;
		public Downloader? downloader = null;

		public PacketHandler(Zenject.DiContainer container) {
			Log?.Debug("PacketHandler()");
			this.container = container;
			rpcManager = container.Resolve<IMenuRpcManager>();
			characteristics = container.Resolve<BeatmapCharacteristicCollectionSO>();
			multiplayerSessionManager = (MultiplayerSessionManager)container.Resolve<IMultiplayerSessionManager>();
			serializer = new NetworkPacketSerializer<MessageType, IConnectedPlayer>();

			multiplayerSessionManager.SetLocalPlayerState("modded", true);
			multiplayerSessionManager.RegisterSerializer(messageType, serializer);
			serializer.RegisterCallback<RecommendPreview>(MessageType.RecommendPreview, HandleRecommendPreview);
			serializer.RegisterCallback<SetCanShareBeatmap>(MessageType.SetCanShareBeatmap, HandleSetCanShareBeatmap);
			serializer.RegisterCallback<DirectDownloadInfo>(MessageType.DirectDownloadInfo, HandleDirectDownloadInfo);
			serializer.RegisterCallback<LevelFragmentRequest>(MessageType.LevelFragmentRequest, HandleLevelFragmentRequest);
			serializer.RegisterCallback<LevelFragment>(MessageType.LevelFragment, HandleLevelFragment);
			serializer.RegisterCallback<LoadProgress>(MessageType.LoadProgress, HandleLoadProgress);

			if(!haveMpCore) {
				mpSerializer = new MpFallbackSerializer();
				multiplayerSessionManager.RegisterSerializer(mpMessageType, mpSerializer);
			}

			rpcManager.setSelectedBeatmapEvent += HandleSetSelectedBeatmapEvent;
			rpcManager.clearSelectedBeatmapEvent += HandleClearSelectedBeatmapEvent;
			rpcManager.setIsEntitledToLevelEvent += HandleSetIsEntitledToLevelRpc;
			rpcManager.recommendGameplayModifiersEvent += HandleRecommendModifiers;
			rpcManager.startedLevelEvent += HandleLevelStart;

			container.Bind(typeof(System.IDisposable)).To<PacketHandler>().FromInstance(this);
		}

		void System.IDisposable.Dispose() {
			Log?.Debug("PacketHandler.Dispose()");
			rpcManager.startedLevelEvent -= HandleLevelStart;
			rpcManager.recommendGameplayModifiersEvent -= HandleRecommendModifiers;
			rpcManager.setIsEntitledToLevelEvent -= HandleSetIsEntitledToLevelRpc;
			rpcManager.clearSelectedBeatmapEvent -= HandleClearSelectedBeatmapEvent;
			rpcManager.setSelectedBeatmapEvent -= HandleSetSelectedBeatmapEvent;
			if(mpSerializer != null)
				multiplayerSessionManager.UnregisterSerializer(mpMessageType, mpSerializer);
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
			if(!preview.requirements.All(x => NullableStringHelper.IsNullOrEmpty(x) || SongCore.Collections.capabilities.Contains(x)))
				return true; // Entitlement[fail]: missing requirements
			return false; // Entitlement[good]: have all requirements
		}

		// Mono.Cecil likes to break nested types, so we need to avoid referencing `ConnectedPlayerManager.ConnectedPlayer`
		public IConnectedPlayer? GetPlayer(string userId) {
			for(int i = 0; i < multiplayerSessionManager.connectedPlayerManager.connectedPlayerCount; i++) {
				IConnectedPlayer player = multiplayerSessionManager.connectedPlayerManager.GetConnectedPlayer(i);
				if(object.Equals(player.userId, userId))
					return player;
			}
			return null;
		}

		static string? selectedLevelId = null;
		static void HandleSetSelectedBeatmapEvent(string userId, BeatmapIdentifierNetSerializable? beatmapId) {
			selectedLevelId = beatmapId?.levelID;
			for(uint i = 0; i < playerData.cells.Length; ++i)
				playerData.cells[i].UpdateData(new LoadProgress(LoadProgress.LoadState.None, 0, 0), true);
		}

		static void HandleClearSelectedBeatmapEvent(string userId) =>
			HandleSetSelectedBeatmapEvent(userId, null);

		void HandleSetIsEntitledToLevelRpc(string userId, string levelId, EntitlementsStatus entitlementStatus) =>
			HandleSetIsEntitledToLevel(GetPlayer(userId), levelId, entitlementStatus);

		void HandleRecommendModifiers(string userId, GameplayModifiers gameplayModifiers) {
			IConnectedPlayer? player = GetPlayer(userId);
			if(player != null)
				playerData.modifiers[player.sortIndex] = new PlayerData.ModifiersWeCareAbout(gameplayModifiers);
		}

		static void HandleLevelStart(string userId, BeatmapIdentifierNetSerializable beatmapId, GameplayModifiers gameplayModifiers, float startTime) {
			for(int i = 0; i < playerData.modifiers.Length; ++i)
				playerData.lockedModifiers[i] = (playerData.modifiers[i].songSpeed == gameplayModifiers.songSpeed) ? playerData.modifiers[i] : new PlayerData.ModifiersWeCareAbout(gameplayModifiers);
		}

		public static void HandleSetIsEntitledToLevel(IConnectedPlayer? player, string levelId, EntitlementsStatus entitlementStatus) {
			if(player == null || levelId != selectedLevelId)
				return;
			LoadProgress.LoadState state;
			switch(entitlementStatus) {
				case EntitlementsStatus.NotOwned: state = LoadProgress.LoadState.Failed; break;
				case EntitlementsStatus.NotDownloaded: state = LoadProgress.LoadState.Downloading; break;
				case EntitlementsStatus.Ok: state = LoadProgress.LoadState.Done; break;
				default: return;
			};
			playerData.cells[player.sortIndex].UpdateData(new LoadProgress(state, 0, 0), true);
		}

		public void SendUnreliableToPlayer<T>(T message, IConnectedPlayer player) where T : LiteNetLib.Utils.INetSerializable {
			ConnectedPlayerManager? connectedPlayerManager = multiplayerSessionManager.connectedPlayerManager;
			if(connectedPlayerManager?.isConnected == true && player is ConnectedPlayerManager.ConnectedPlayer connectedPlayer)
				connectedPlayer._connection.Send(connectedPlayerManager.WriteOne(((ConnectedPlayerManager.ConnectedPlayer)connectedPlayerManager.localPlayer)._connectionId, connectedPlayer._remoteConnectionId, message), LiteNetLib.DeliveryMethod.Unreliable);
			else if(message is IPoolablePacket poolable)
				poolable.Release();
		}

		public static void HandleRecommendPreview(RecommendPreview packet, IConnectedPlayer player) {
			Log?.Debug($"HandleRecommendPreview(\"{packet.levelID}\", {player})");
			playerData.previews[player.sortIndex] = packet;
		}

		static void HandleSetCanShareBeatmap(SetCanShareBeatmap packet, IConnectedPlayer player) {}

		void HandleDirectDownloadInfo(DirectDownloadInfo packet, IConnectedPlayer player) {
			Log?.Debug($"DirectDownloadInfo:\n    levelId=\"{packet.levelId}\"\n    levelHash=\"{packet.levelHash}\"\n    fileSize={packet.fileSize}\n    source=\"{packet.sourcePlayers[0]}\"");
			RecommendPreview? preview = playerData.ResolvePreview(packet.levelId);
			if(connectInfo?.directDownloads != true || preview == null || MissingRequirements(preview, true) || packet.fileSize > MaxDownloadSize)
				return;
			beatmapLevelsModel.GetBeatmapLevelAsync(packet.levelId, System.Threading.CancellationToken.None).ContinueWith(result => {
				if(!result.Result.isError)
					return;
				LoadProgress progress = new LoadProgress(LoadProgress.LoadState.Downloading, 0);
				HandleLoadProgress(progress, multiplayerSessionManager.localPlayer);
				multiplayerSessionManager.Send(progress);
			});
			downloader = new Downloader(packet, preview);
		}

		void HandleLevelFragmentRequest(LevelFragmentRequest packet, IConnectedPlayer player) {
			if(packet.offset >= (ulong)uploadLevel.data.Count)
				return;
			int length = (int)System.Math.Min(packet.maxSize, (ulong)uploadLevel.data.Count - packet.offset);
			System.ArraySegment<byte> data = new System.ArraySegment<byte>(uploadLevel.data.Array, uploadLevel.data.Offset + (int)packet.offset, length);
			SendUnreliableToPlayer(new LevelFragment(packet.offset, data), player);
		}

		void HandleLevelFragment(LevelFragment packet, IConnectedPlayer player) =>
			downloader?.HandleFragment(packet, player);

		public static void HandleLoadProgress(LoadProgress packet, IConnectedPlayer player) =>
			playerData.cells[player.sortIndex].UpdateData(packet);
	}
	static PacketHandler handler = null!;

	public class Downloader {
		readonly CustomLevelLoader customLevelLoader = handler.container.Resolve<CustomLevelLoader>();
		readonly string dataPath = System.IO.Path.Combine(System.IO.Path.GetFullPath(CustomLevelPathHelper.baseProjectPath), "BeatUpClient_Data");
		readonly byte[] levelHash;
		readonly string[] sourcePlayers;
		readonly PacketHandler.RecommendPreview preview;
		readonly byte[] buffer;
		readonly System.Collections.Generic.List<(ulong start, ulong end)> gaps;
		System.Collections.Generic.List<IConnectedPlayer>? sources;
		public string? levelId => preview.levelID;
		public Downloader(PacketHandler.DirectDownloadInfo info, PacketHandler.RecommendPreview preview) {
			(this.levelHash, this.sourcePlayers, this.preview, buffer) = (info.levelHash, info.sourcePlayers, preview, new byte[info.fileSize]);
			gaps = new System.Collections.Generic.List<(ulong, ulong)>(new[] {(0LU, info.fileSize)});
		}
		string ValidatedPath(string filename) {
			string path = System.IO.Path.Combine(dataPath, filename);
			if(System.IO.Path.GetDirectoryName(path) != dataPath)
				throw new System.Security.SecurityException($"Path mismatch (`{System.IO.Path.GetDirectoryName(path)}` != `{dataPath}`)");
			if(System.IO.Path.GetFileName(path) != filename)
				throw new System.Security.SecurityException($"Filename mismatch (`{System.IO.Path.GetFileName(path)}` != `{filename}`)");
			return path;
		}
		EnvironmentInfoSO LoadEnvironmentInfo(string environmentName, EnvironmentInfoSO defaultInfo) =>
			customLevelLoader._environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(environmentName) ?? defaultInfo;
		CustomPreviewBeatmapLevel LoadZippedPreviewBeatmapLevel(StandardLevelInfoSaveData info, System.IO.Compression.ZipArchive archive) {
			EnvironmentInfoSO envInfo = LoadEnvironmentInfo(info.environmentName, customLevelLoader._defaultEnvironmentInfo);
			EnvironmentInfoSO envInfo360 = LoadEnvironmentInfo(info.allDirectionsEnvironmentName, customLevelLoader._defaultAllDirectionsEnvironmentInfo);
			PreviewDifficultyBeatmapSet[] sets = info.difficultyBeatmapSets.Select(difficultyBeatmapSet => {
				BeatmapCharacteristicSO beatmapCharacteristicBySerializedName = handler.characteristics.GetBeatmapCharacteristicBySerializedName(difficultyBeatmapSet.beatmapCharacteristicName);
				if(beatmapCharacteristicBySerializedName == null)
					return null;
				return new PreviewDifficultyBeatmapSet(beatmapCharacteristicBySerializedName, difficultyBeatmapSet.difficultyBeatmaps.Select(diff => {
					diff.difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
					return difficulty;
				}).ToArray());
			}).Where(preview => preview != null).Select(preview => preview!).ToArray();
			return new CustomPreviewBeatmapLevel(defaultPackCover, info, dataPath, new MemorySpriteLoader(preview.coverRenderTask), levelId, info.songName, info.songSubName, info.songAuthorName, info.levelAuthorName, info.beatsPerMinute, info.songTimeOffset, info.shuffle, info.shufflePeriod, info.previewStartTime, info.previewDuration, envInfo, envInfo360, sets);
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
		async System.Threading.Tasks.Task<CustomBeatmapLevel> LoadZippedBeatmapLevelAsync(CustomPreviewBeatmapLevel customPreviewBeatmapLevel, System.IO.Compression.ZipArchive archive, bool modded, System.Threading.CancellationToken cancellationToken) {
			StandardLevelInfoSaveData info = customPreviewBeatmapLevel.standardLevelInfoSaveData;
			System.IO.Compression.ZipArchiveEntry? song = archive.GetEntry(info.songFilename) ?? throw new System.IO.FileNotFoundException("File not found in archive: " + info.songFilename);
			UnityEngine.AudioClip audioClip = await DecodeAudio(song, AudioTypeHelper.GetAudioTypeFromPath(info.songFilename)) ?? throw new System.InvalidOperationException("Null audio clip");
			CustomBeatmapLevel customBeatmapLevel = new CustomBeatmapLevel(customPreviewBeatmapLevel);
			customBeatmapLevel.SetBeatmapLevelData(new BeatmapLevelData(audioClip, info.difficultyBeatmapSets.Select(setInfo => {
				BeatmapCharacteristicSO? beatmapCharacteristicBySerializedName = handler.characteristics.GetBeatmapCharacteristicBySerializedName(setInfo.beatmapCharacteristicName);
				CustomDifficultyBeatmapSet beatmapSet = new CustomDifficultyBeatmapSet(beatmapCharacteristicBySerializedName);
				beatmapSet.SetCustomDifficultyBeatmaps(setInfo.difficultyBeatmaps.Select(difficultyInfo => {
					string filename = difficultyInfo.beatmapFilename;
					System.IO.Compression.ZipArchiveEntry file = archive.GetEntry(filename) ?? throw new System.IO.FileNotFoundException("File not found in archive: " + filename);
					byte[] rawData = new byte[file.Length];
					file.Open().Read(rawData, 0, rawData.Length);
					if(modded)
						System.IO.File.WriteAllBytes(ValidatedPath(filename), rawData);
					BeatmapSaveDataVersion3.BeatmapSaveData saveData = BeatmapSaveDataVersion3.BeatmapSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(rawData, 0, rawData.Length));
					BeatmapDataBasicInfo basicInfo = BeatmapDataLoader.GetBeatmapDataBasicInfoFromSaveData(saveData);
					difficultyInfo.difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
					return new CustomDifficultyBeatmap(customBeatmapLevel, beatmapSet, difficulty, difficultyInfo.difficultyRank, difficultyInfo.noteJumpMovementSpeed, difficultyInfo.noteJumpStartBeatOffset, info.beatsPerMinute, saveData, basicInfo);
				}).ToArray());
				return beatmapSet;
			}).ToArray()));
			return customBeatmapLevel;
		}
		public async System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult?> Resolve(System.Action<PacketHandler.LoadProgress> progress, System.Threading.CancellationToken cancellationToken) {
			Log?.Debug("Starting direct download");
			try {
				cancellationToken.ThrowIfCancellationRequested();
				sources ??= sourcePlayers.Select(userId => handler.GetPlayer(userId)!).Where(player => player is ConnectedPlayerManager.ConnectedPlayer).ToList();
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
				if(!levelHash.SequenceEqual(hash))
					throw new System.IO.InvalidDataException("Hash mismatch");
				using(System.IO.MemoryStream stream = new System.IO.MemoryStream(buffer)) {
					using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(stream, System.IO.Compression.ZipArchiveMode.Read) ?? throw new System.InvalidOperationException()) {
						archive.Entries.Aggregate(0LU, (total, entry) => {
							total += (ulong)entry.Length;
							if((ulong)entry.Length > MaxUnzippedSize || total > MaxUnzippedSize)
								throw new System.InvalidOperationException("Unzipped size too large");
							return total;
						});
						System.IO.Compression.ZipArchiveEntry infoFile = archive.GetEntry("Info.dat") ?? throw new System.IO.FileNotFoundException("File not found in archive: Info.dat");
						if(System.IO.Directory.Exists(dataPath)) {
							foreach(System.IO.FileInfo file in new System.IO.DirectoryInfo(dataPath).GetFiles())
								file.Delete();
						} else {
							System.IO.Directory.CreateDirectory(dataPath);
						}
						bool modded = preview.requirements.Length >= 1 || preview.suggestions.Length >= 1;
						byte[] rawInfo = new byte[infoFile.Length];
						infoFile.Open().Read(rawInfo, 0, rawInfo.Length);
						if(modded)
							System.IO.File.WriteAllBytes(ValidatedPath("Info.dat"), rawInfo);
						StandardLevelInfoSaveData info = StandardLevelInfoSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(rawInfo, 0, rawInfo.Length));
						CustomPreviewBeatmapLevel previewBeatmapLevel = LoadZippedPreviewBeatmapLevel(info, archive);
						preview.Init(previewBeatmapLevel);
						CustomBeatmapLevel level = await LoadZippedBeatmapLevelAsync(previewBeatmapLevel, archive, modded, cancellationToken);
						Log?.Debug($"level: {level}");
						return new BeatmapLevelsModel.GetBeatmapLevelResult(isError: false, level);
					}
				}
			} catch(System.Threading.Tasks.TaskCanceledException) {
			} catch(System.Exception ex) {
				Log?.Error("Direct download error: " + ex);
			}
			return null;
		}
		public void HandleFragment(PacketHandler.LevelFragment fragment, IConnectedPlayer player) {
			if(gaps.Count < 1 || sources?.Contains(player) != true)
				return;
			if(fragment.offset >= (ulong)buffer.LongLength || fragment.offset + (uint)fragment.data.Count > (ulong)buffer.LongLength || fragment.data.Count < 1) {
				sources.Remove(player);
				return;
			}
			((System.Span<byte>)fragment.data).CopyTo(new System.Span<byte>(buffer, (int)fragment.offset, fragment.data.Count));
			ulong start = fragment.offset, end = fragment.offset + (uint)fragment.data.Count;
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

	public static class UI {
		public class ToggleSetting : SwitchSettingsController {
			public IValue<bool> setting = null!;
			public ToggleSetting() {
				_toggle = gameObject.transform.GetChild(1).gameObject.GetComponent<UnityEngine.UI.Toggle>();
				_toggle.onValueChanged.RemoveAllListeners();
				Config.onReload += OnEnable;
			}
			protected override bool GetInitValue() => setting.value;
			protected override void ApplyValue(bool value) =>
				setting.value = value;
		}
		public class ValuePickerSetting : ListSettingsController {
			public byte[] options = null!;
			public IValue<float> setting = null!;
			public int startIdx = 1;
			public ValuePickerSetting() {
				_stepValuePicker = gameObject.transform.GetChild(1).gameObject.GetComponent<StepValuePicker>();
				Config.onReload += OnEnable;
			}
			protected override bool GetInitValues(out int idx, out int numberOfElements) {
				idx = System.Array.LastIndexOf(options, (byte)(setting.value * 4));
				if(idx == 0)
					startIdx = 0;
				else
					--idx;
				numberOfElements = options.Length - startIdx;
				return true;
			}
			protected override void ApplyValue(int idx) =>
				setting.value = options[idx + startIdx] / 4.0f;
			protected override string TextForValue(int idx) => $"{options[idx + startIdx] / 4.0f}";
		}
		public class ServerDropdown : DropdownSettingsController {
			string[] options = null!;
			public ServerDropdown() {
				_dropdown = GetComponent<HMUI.SimpleTextDropdown>();
				Refresh();
			}
			protected override bool GetInitValues(out int idx, out int numberOfElements) {
				idx = System.Array.IndexOf(options, customServerHostName.value) + 1;
				numberOfElements = options.Length + 1;
				return true;
			}
			protected override void ApplyValue(int idx) {
				string newValue = idx >= 1 ? options[idx - 1] : "";
				_dropdown.Hide(customServerHostName.value.Equals(newValue)); // Animation will break if MultiplayerLevelSelectionFlowCoordinator is immediately dismissed
				customServerHostName.value = newValue;
			}
			protected override string TextForValue(int idx) =>
				idx >= 1 ? options[idx - 1] : "Official Server";
			public void Refresh() {
				this.options = Config.Instance.Servers.Keys.ToArray();
				if(gameObject.activeSelf) {
					gameObject.SetActive(false);
					gameObject.SetActive(true);
				}
			}
		}
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
		public static UnityEngine.RectTransform CreateToggle(UnityEngine.Transform parent, string name, string headerKey, IValue<bool> value) {
			UnityEngine.GameObject toggleTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.UI.Toggle>().Select(x => x.transform.parent.gameObject).First(p => p.name == "Fullscreen");
			UnityEngine.GameObject gameObject = CreateElementWithText(toggleTemplate, parent, name, headerKey);
			UnityEngine.Object.Destroy(gameObject.GetComponent<BoolSettingsController>());
			ToggleSetting toggleSetting = gameObject.AddComponent<ToggleSetting>();
			toggleSetting.setting = value;
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateValuePicker(UnityEngine.Transform parent, string name, string headerKey, IValue<float> value, byte[] options) {
			UnityEngine.GameObject pickerTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<StepValuePicker>().Select(x => x.transform.parent.gameObject).First(p => p.name == "MaxNumberOfPlayers");
			UnityEngine.GameObject gameObject = CreateElementWithText(pickerTemplate, parent, name, headerKey);
			UnityEngine.Object.Destroy(gameObject.GetComponent<FormattedFloatListSettingsController>());
			ValuePickerSetting valuePickerSetting = gameObject.AddComponent<ValuePickerSetting>();
			valuePickerSetting.options = options;
			valuePickerSetting.setting = value;
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateSimpleDropdown(UnityEngine.Transform parent, string name, IValue<string> value, System.Collections.Generic.IEnumerable<string> options) {
			UnityEngine.GameObject simpleDropdownTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.SimpleTextDropdown>().First(dropdown => dropdown.GetComponents(typeof(UnityEngine.Component)).Length == 2).gameObject;
			UnityEngine.GameObject gameObject = CreateElement(simpleDropdownTemplate, parent, name);
			gameObject.AddComponent<ServerDropdown>();
			gameObject.SetActive(true);
			return (UnityEngine.RectTransform)gameObject.transform;
		}
		public static UnityEngine.RectTransform CreateTextbox(UnityEngine.Transform parent, string name, int index, System.Action<HMUI.InputFieldView> callback, string? placeholderKey = null) {
			UnityEngine.GameObject textboxTemplate = UnityEngine.Resources.FindObjectsOfTypeAll<EnterPlayerGuestNameViewController>()[0]._nameInputFieldView.gameObject;
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
		public static UnityEngine.GameObject AddKey(HMUI.UIKeyboard keyboard, bool numpad, int row, int col, UnityEngine.KeyCode keyCode, char charCode, bool canBeUppercase = false) {
			UnityEngine.Transform parent = keyboard.transform.Find(numpad ? "Numpad" : "Letters").GetChild(row);
			UnityEngine.Transform refKey = parent.GetChild(numpad ? 0 : 1);
			UnityEngine.GameObject key = UnityEngine.Object.Instantiate(refKey.gameObject, parent);
			key.name = "" + keyCode;
			if(col < 0)
				col += parent.childCount;
			key.transform.SetSiblingIndex(col);
			if(numpad) {
				parent.GetComponent<UnityEngine.UI.HorizontalLayoutGroup>().enabled = true;
			} else {
				UnityEngine.Vector3 refPosition = refKey.localPosition;
				for(int i = col, count = parent.childCount; i < count; ++i)
					parent.GetChild(i).localPosition = new UnityEngine.Vector3(refPosition.x + i * 7 - 7, refPosition.y, refPosition.z);
			}
			HMUI.UIKeyboardKey keyboardKey = key.GetComponent<HMUI.UIKeyboardKey>();
			keyboardKey._keyCode = keyCode;
			keyboardKey._overrideText = $"{charCode}";
			keyboardKey._canBeUppercase = canBeUppercase;
			keyboardKey.Awake();
			keyboard._buttonBinder.AddBinding(key.GetComponent<HMUI.NoTransitionsButton>(), () =>
				keyboard.keyWasPressedEvent(charCode));
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

	// Ugly hack for avoiding BSIPA's overcomplicated and limiting config system
	public struct Property<T> : IValue<T> {
		System.Func<T> get;
		System.Action<T> set;
		public T value {get => get(); set => set(value);}
		public Property(object instance, string propName) {
			System.Reflection.PropertyInfo member = instance.GetType().GetProperty(propName);
			get = (System.Func<T>)System.Delegate.CreateDelegate(typeof(System.Func<T>), instance, member.GetGetMethod());
			set = (System.Action<T>)System.Delegate.CreateDelegate(typeof(System.Action<T>), instance, member.GetSetMethod());
		}
	}

	public class DifficultyPanel {
		static UnityEngine.GameObject characteristicTemplate = null!;
		static UnityEngine.GameObject difficultyTemplate = null!;
		BeatmapCharacteristicSegmentedControlController characteristicSelector;
		BeatmapDifficultySegmentedControlController difficultySelector;
		bool hideHints;
		public UnityEngine.RectTransform beatmapCharacteristic => (UnityEngine.RectTransform)characteristicSelector.transform.parent;
		public UnityEngine.RectTransform beatmapDifficulty => (UnityEngine.RectTransform)difficultySelector.transform.parent;

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
		static void ChangeBackground(UnityEngine.RectTransform target, HMUI.ImageView newBG) {
			HMUI.ImageView bg = target.Find("BG").GetComponent<HMUI.ImageView>();
			(bg._skew, bg.color, bg.color0, bg.color1, bg._flipGradientColors) = (newBG._skew, newBG.color, newBG.color0, newBG.color1, newBG._flipGradientColors);
		}
		public DifficultyPanel(UnityEngine.Transform parent, int index, float width, HMUI.ImageView? background = null, bool hideHints = false) {
			UnityEngine.RectTransform beatmapCharacteristic = UI.CreateClone(characteristicTemplate, parent, "BeatmapCharacteristic", index);
			UnityEngine.RectTransform beatmapDifficulty = UI.CreateClone(difficultyTemplate, parent, "BeatmapDifficulty", index + 1);
			if(background != null) {
				ChangeBackground(beatmapCharacteristic, background);
				ChangeBackground(beatmapDifficulty, background);
			}
			beatmapCharacteristic.sizeDelta = new UnityEngine.Vector2(width, beatmapCharacteristic.sizeDelta.y);
			beatmapDifficulty.sizeDelta = new UnityEngine.Vector2(width, beatmapDifficulty.sizeDelta.y);
			characteristicSelector = beatmapCharacteristic.GetComponentInChildren<BeatmapCharacteristicSegmentedControlController>();
			difficultySelector = beatmapDifficulty.GetComponentInChildren<BeatmapDifficultySegmentedControlController>();
			characteristicSelector._segmentedControl._container = new Zenject.DiContainer();
			characteristicSelector._segmentedControl._container.Bind<HMUI.HoverHintController>().FromInstance(UnityEngine.Resources.FindObjectsOfTypeAll<HMUI.HoverHintController>()[0]).AsSingle();
			difficultySelector._difficultySegmentedControl._container = new Zenject.DiContainer();
			this.hideHints = hideHints;
		}
		public bool Clear() {
			characteristicSelector.transform.parent.gameObject.SetActive(false);
			difficultySelector.transform.parent.gameObject.SetActive(false);
			return false;
		}
		public bool Update(PreviewDifficultyBeatmap? beatmapLevel, System.Action<PreviewDifficultyBeatmap> onChange) {
			characteristicSelector.didSelectBeatmapCharacteristicEvent = delegate {};
			difficultySelector.didSelectDifficultyEvent = delegate {};
			if(beatmapLevel == null)
				return Clear();
			System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet>? previewDifficultyBeatmapSets = beatmapLevel.beatmapLevel.previewDifficultyBeatmapSets;
			if(previewDifficultyBeatmapSets == null)
				return Clear();
			BeatmapCharacteristicSO[] beatmapCharacteristics = PreviewDifficultyBeatmapSetExtensions.GetBeatmapCharacteristics(previewDifficultyBeatmapSets.ToArray());
			for(int i = 0; i < beatmapCharacteristics.Length; ++i)
				if(beatmapCharacteristics[i] == beatmapLevel.beatmapCharacteristic)
					difficultySelector.SetData(previewDifficultyBeatmapSets[i].beatmapDifficulties.Select(diff => (IDifficultyBeatmap)new CustomDifficultyBeatmap(null, null, diff, 0, 0, 0, 0, null, null)).ToList(), beatmapLevel.beatmapDifficulty);
			characteristicSelector.SetData(beatmapCharacteristics.Select(ch => (IDifficultyBeatmapSet)new CustomDifficultyBeatmapSet(ch)).ToList(), beatmapLevel.beatmapCharacteristic);
			if(hideHints)
				foreach(HMUI.HoverHint hint in characteristicSelector.GetComponentsInChildren<HMUI.HoverHint>())
					hint.enabled = false;
			characteristicSelector.didSelectBeatmapCharacteristicEvent += (controller, beatmapCharacteristic) => {
				PreviewDifficultyBeatmapSet set = previewDifficultyBeatmapSets.First(set => set.beatmapCharacteristic == beatmapCharacteristic);
				BeatmapDifficulty closestDifficulty = set.beatmapDifficulties[0];
				foreach(BeatmapDifficulty difficulty in set.beatmapDifficulties) {
					if(beatmapLevel.beatmapDifficulty < difficulty)
						break;
					closestDifficulty = difficulty;
				}
				onChange(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapCharacteristic, closestDifficulty));
			};
			difficultySelector.didSelectDifficultyEvent += (controller, difficulty) =>
				onChange(new PreviewDifficultyBeatmap(beatmapLevel.beatmapLevel, beatmapLevel.beatmapCharacteristic, difficulty));
			characteristicSelector.transform.parent.gameObject.SetActive(true);
			difficultySelector.transform.parent.gameObject.SetActive(true);
			return false;
		}
	}

	[Patch(PatchType.Prefix, typeof(ClientCertificateValidator), "ValidateCertificateChainInternal")]
	public static bool ClientCertificateValidator_ValidateCertificateChainInternal() =>
		customNetworkConfig == null;

	[Patch(PatchType.Postfix, typeof(MainSettingsModelSO), nameof(MainSettingsModelSO.Load))]
	public static void MainSettingsModelSO_Load(ref MainSettingsModelSO __instance) =>
		customServerHostName = __instance.customServerHostName;

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

	static UnityEngine.GameObject? prevLoading = null;
	static void ShowLoading(UnityEngine.GameObject? loading) {
		if(prevLoading != null)
			UnityEngine.Object.Destroy(prevLoading);
		prevLoading = loading;
		if(loading != null) {
			loading.name = "BeatUpClient_LoadingControl";
			loading.transform.localPosition = new UnityEngine.Vector3(0, 12, 0);
			loading.GetComponent<LoadingControl>().ShowLoading(Polyglot.Localization.Get("LABEL_CHECKING_SERVER_STATUS"));
		}
	}

	[Patch(PatchType.Prefix, typeof(HMUI.FlowCoordinator), "ProvideInitialViewControllers")]
	public static void FlowCoordinator_ProvideInitialViewControllers(HMUI.FlowCoordinator __instance, ref HMUI.ViewController mainViewController) {
		if(mainViewController is JoiningLobbyViewController originalView && __instance is MultiplayerModeSelectionFlowCoordinator flowCoordinator) {
			mainViewController = flowCoordinator._multiplayerModeSelectionViewController;
			flowCoordinator._multiplayerModeSelectionViewController.transform.Find("Buttons")?.gameObject.SetActive(false);
			flowCoordinator._multiplayerModeSelectionViewController._maintenanceMessageText.gameObject.SetActive(false);
			flowCoordinator.showBackButton = true;
			ShowLoading(UnityEngine.Object.Instantiate(originalView._loadingControl.gameObject, mainViewController.transform));
		}
	}

	// This callback triggers twice if `_multiplayerStatusModel.GetMultiplayerStatusAsync()` was cancelled by pressing the back button
	[Patch(PatchType.Prefix, typeof(MainFlowCoordinator), nameof(MainFlowCoordinator.HandleMultiplayerModeSelectionFlowCoordinatorDidFinish))]
	public static bool MainFlowCoordinator_HandleMultiplayerModeSelectionFlowCoordinatorDidFinish(MultiplayerModeSelectionFlowCoordinator multiplayerModeSelectionFlowCoordinator) =>
		multiplayerModeSelectionFlowCoordinator._parentFlowCoordinator != null;

	// The UI deletes itself at the end of `MultiplayerModeSelectionFlowCoordinator.TryShowModeSelection()` without this
	[PatchOverload(PatchType.Prefix, typeof(HMUI.FlowCoordinator), "ReplaceTopViewController", new[] {typeof(HMUI.ViewController), typeof(System.Action), typeof(HMUI.ViewController.AnimationType), typeof(HMUI.ViewController.AnimationDirection)})]
	public static bool FlowCoordinator_ReplaceTopViewController(HMUI.FlowCoordinator __instance, HMUI.ViewController viewController, System.Action finishedCallback, HMUI.ViewController.AnimationType animationType) {
		return (!(viewController is MultiplayerModeSelectionViewController)).Or(() => {
			__instance.TopViewControllerWillChange(viewController, viewController, animationType);
			finishedCallback();
		});
	}

	static string GetMaintenanceMessage(MultiplayerUnavailableReason reason, long? maintenanceWindowEndTime) {
		if(reason == MultiplayerUnavailableReason.MaintenanceMode)
			return Polyglot.Localization.GetFormat(MultiplayerUnavailableReasonMethods.LocalizedKey(reason), (TimeExtensions.AsUnixTime(maintenanceWindowEndTime.GetValueOrDefault()) - System.DateTime.UtcNow).ToString("h':'mm"));
		return $"{Polyglot.Localization.Get(MultiplayerUnavailableReasonMethods.LocalizedKey(reason))} ({MultiplayerUnavailableReasonMethods.ErrorCode(reason)})";
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerModeSelectionFlowCoordinator), nameof(MultiplayerModeSelectionFlowCoordinator.PresentMasterServerUnavailableErrorDialog))]
	public static bool MultiplayerModeSelectionFlowCoordinator_PresentMasterServerUnavailableErrorDialog(MultiplayerModeSelectionFlowCoordinator __instance, MultiplayerModeSelectionViewController ____multiplayerModeSelectionViewController, MultiplayerUnavailableReason reason, long? maintenanceWindowEndTime, string? remoteLocalizedMessage) {
		if(mainFlowCoordinator.childFlowCoordinator != __instance)
			return false;
		ShowLoading(null);
		TMPro.TextMeshProUGUI message = ____multiplayerModeSelectionViewController._maintenanceMessageText;
		message.text = remoteLocalizedMessage ?? GetMaintenanceMessage(reason, maintenanceWindowEndTime);
		message.richText = true;
		message.transform.localPosition = new UnityEngine.Vector3(0, 15, 0);
		message.gameObject.SetActive(true);
		__instance.SetTitle(Polyglot.Localization.Get("LABEL_CONNECTION_ERROR"), HMUI.ViewController.AnimationType.In);
		return false;
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerModeSelectionViewController), nameof(MultiplayerModeSelectionViewController.SetData))]
	public static void MultiplayerModeSelectionViewController_SetData(MultiplayerModeSelectionViewController __instance, TMPro.TextMeshProUGUI ____maintenanceMessageText) {
		ShowLoading(null);
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
		playerData = new PlayerData(configuration.maxPlayerCount);
		lobbyDifficultyPanel.Clear();
		connectInfo = null;
		windowSize = LiteNetLib.NetConstants.DefaultWindowSize;
		infoText.SetActive(false);
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelSelectionFlowCoordinator), "get_enableCustomLevels")]
	public static void MultiplayerLevelSelectionFlowCoordinator_enableCustomLevels(ref bool __result) =>
		__result |= enableCustomLevels;

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelSelectionFlowCoordinator), "get_notAllowedCharacteristics")]
	public static void MultiplayerLevelSelectionFlowCoordinator_notAllowedCharacteristics(ref BeatmapCharacteristicSO[] __result) =>
		__result = new BeatmapCharacteristicSO[0];

	[Patch(PatchType.Prefix, typeof(MultiplayerSessionManager), "HandlePlayerOrderChanged")]
	public static void MultiplayerSessionManager_HandlePlayerOrderChanged(IConnectedPlayer player) =>
		playerData.Reset(player.sortIndex);

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

	// `windowSize` MUST be set before LiteNetLib constructs any `ReliableChannel`s
	[Patch(PatchType.Prefix, typeof(LiteNetLib.NetConnectAcceptPacket), "FromData")]
	public static void NetConnectAcceptPacket_FromData(ref LiteNetLib.NetPacket packet) {
		if(packet.Size == LiteNetLib.NetConnectAcceptPacket.Size + BeatUpConnectInfo.Size) {
			packet.Size = LiteNetLib.NetConnectAcceptPacket.Size;
			BeatUpConnectInfo info = new BeatUpConnectInfo();
			info.Deserialize(new LiteNetLib.Utils.NetDataReader(packet.RawData, packet.Size));
			if(info.windowSize < 32 || info.windowSize > 512)
				return;
			info.directDownloads = info.directDownloads && Config.Instance.DirectDownloads;
			connectInfo = info;
			windowSize = info.windowSize;
			infoText.SetActive(true);
			Log?.Info($"Overriding window size - {info.windowSize}");
		}
		Polyglot.LocalizedTextMeshProUGUI? SuggestedModifiers = UnityEngine.Resources.FindObjectsOfTypeAll<GameServerPlayersTableView>()[0].transform.Find("ServerPlayersTableHeader/Labels/SuggestedModifiers")?.GetComponent<Polyglot.LocalizedTextMeshProUGUI>();
		if(SuggestedModifiers != null)
			SuggestedModifiers.Key = (connectInfo?.perPlayerModifiers == false) ? "BEATUP_VOTE_MODIFIERS" : "SUGGESTED_MODIFIERS";
	}

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.HandleMenuRpcManagerGetRecommendedBeatmap))]
	public static void LobbyPlayersDataModel_HandleMenuRpcManagerGetRecommendedBeatmap(string userId) =>
		handler.multiplayerSessionManager.Send(playerData.previews[handler.multiplayerSessionManager.localPlayer.sortIndex]);

	[Patch(PatchType.Prefix, typeof(LobbyPlayersDataModel), nameof(LobbyPlayersDataModel.SetLocalPlayerBeatmapLevel))]
	public static void LobbyPlayersDataModel_SetLocalPlayerBeatmapLevel(LobbyPlayersDataModel __instance, PreviewDifficultyBeatmap? beatmapLevel) {
		if(beatmapLevel != null) {
			PacketHandler.RecommendPreview? preview = playerData.previews[handler.multiplayerSessionManager.localPlayer.sortIndex];
			if(preview.levelID != beatmapLevel.beatmapLevel.levelID) {
				preview = playerData.ResolvePreview(beatmapLevel.beatmapLevel.levelID);
				if(preview != null)
					playerData.previews[handler.multiplayerSessionManager.localPlayer.sortIndex] = preview;
				else if(beatmapLevel.beatmapLevel is CustomPreviewBeatmapLevel previewBeatmapLevel)
					preview = playerData.SetPreviewFromLocal(handler.multiplayerSessionManager.localPlayer.sortIndex, previewBeatmapLevel);
			}
			if(preview != null) {
				if(!haveMpCore)
					handler.multiplayerSessionManager.Send(new PacketHandler.MpBeatmapPacket(beatmapLevel));
				handler.multiplayerSessionManager.Send(preview);
			}
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
		__result ??= (IPreviewBeatmapLevel?)playerData.ResolvePreview(levelId) ?? new ErrorBeatmapLevel(levelId);

	class CallbackStream : System.IO.Stream {
		public readonly System.IO.Stream stream;
		public readonly System.Action<int> onProgress;
		public override bool CanRead => stream.CanRead;
		public override bool CanSeek => stream.CanSeek;
		public override bool CanWrite => stream.CanWrite;
		public override long Length => stream.Length;
		public override long Position {
			get => stream.Position;
			set => stream.Position = value;
		}
		public CallbackStream(System.IO.Stream stream, System.Action<int> onProgress) =>
			(this.stream, this.onProgress) = (stream, onProgress);
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

	readonly struct ShareLevel {
		public readonly string id;
		public readonly System.ArraySegment<byte> data;
		public readonly byte[] hash;
		public ShareLevel(string id, System.ArraySegment<byte> data, byte[] hash) =>
			(this.id, this.data, this.hash) = (id, data, hash);
	}

	static async System.Threading.Tasks.Task<ShareLevel?> ZipLevel(CustomBeatmapLevel level, System.Threading.CancellationToken cancellationToken) {
		#if ENABLE_ZIP_PROGRESS
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
		#else
		ulong total = 0;
		void UpdateProgress(ulong p, bool reliable = false) {}
		void HandleProgress(int count) {}
		#endif
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
		total = (ulong)files.Aggregate(0L, (total, file) => total + new System.IO.FileInfo(file.path).Length);
		if(total > MaxUnzippedSize) {
			Log?.Debug("Level too large!");
			return null;
		}
		using System.IO.MemoryStream memoryStream = new System.IO.MemoryStream();
		using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(memoryStream, System.IO.Compression.ZipArchiveMode.Create, true)) {
			UpdateProgress(0);
			foreach((string path, string name) info in files) {
				Log?.Debug($"    Compressing `{info.path}`");
				using(System.IO.Stream file = System.IO.File.Open(info.path, System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.Read)) {
					using(System.IO.Stream entry = archive.CreateEntry(info.name).Open()) {
						await file.CopyToAsync(new CallbackStream(entry, HandleProgress), 81920, cancellationToken);
					}
				}
			}
		}
		if(memoryStream.TryGetBuffer(out System.ArraySegment<byte> buffer)) {
			#if ENABLE_ZIP_PROGRESS
			progress = 0;
			total = (ulong)buffer.Count;
			progressStart = progressWidth;
			progressWidth = 65535 - progressStart;
			#endif
			memoryStream.Seek(0, System.IO.SeekOrigin.Begin);
			byte[] hash = await System.Threading.Tasks.Task.Run(() => {
				using System.Security.Cryptography.SHA256 sha256 = System.Security.Cryptography.SHA256.Create();
				return sha256.ComputeHash(new CallbackStream(memoryStream, HandleProgress)); // TODO: need `cancellationToken` here
			});
			UpdateProgress(65535, true);
			return new ShareLevel(level.levelID, buffer, hash);
		}
		UpdateProgress(65535, true);
		return null;
	}

	class InterlockedTask {
		public class Handle : System.IDisposable {
			readonly System.Threading.CancellationTokenSource source = new System.Threading.CancellationTokenSource();
			readonly System.Threading.Mutex mutex;
			public Handle(System.Threading.Mutex mutex) =>
				this.mutex = mutex;
			public System.Threading.CancellationToken Token =>
				source.Token;
			public void Cancel() =>
				source.Cancel();
			public void Dispose() =>
				mutex.ReleaseMutex();
		}
		System.Threading.Mutex mutex = new System.Threading.Mutex();
		Handle? currentTask = null;
		public Handle Aquire() {
			currentTask?.Cancel();
			if(!mutex.WaitOne()) {
				mutex.ReleaseMutex();
				throw new System.InvalidOperationException("Failed to aquire lock");
			}
			currentTask = new Handle(mutex);
			return currentTask;
		}
	}

	static ShareLevel uploadLevel = new ShareLevel("", default(System.ArraySegment<byte>), new byte[32]);
	static InterlockedTask shareTask = new InterlockedTask();
	public static async System.Threading.Tasks.Task<EntitlementsStatus> ShareTask(string levelId) {
		Log?.Debug($"ShareTask(levelId=\"{levelId}\")");
		BeatmapLevelsModel.GetBeatmapLevelResult result = await beatmapLevelsModel.GetBeatmapLevelAsync(levelId, System.Threading.CancellationToken.None);
		Log?.Debug($"GetBeatmapLevelResult.isError: {result.isError}");
		if(result.isError)
			return (connectInfo?.directDownloads == true && playerData.ResolvePreview(levelId) != null) ? EntitlementsStatus.Unknown : EntitlementsStatus.NotOwned;
		try {
			using InterlockedTask.Handle handle = shareTask.Aquire();
			if(!(connectInfo?.directDownloads == true && result.beatmapLevel is CustomBeatmapLevel level))
				return EntitlementsStatus.Ok;
			if(uploadLevel.id == levelId) {
				Log?.Debug("Custom level already zipped");
				return EntitlementsStatus.Ok;
			}
			Log?.Debug("Zipping custom level");
			ShareLevel? share = await ZipLevel(level, handle.Token);
			if(share == null) {
				Log?.Debug("Zip failed");
				return EntitlementsStatus.Ok;
			}
			uploadLevel = (ShareLevel)share;
			Log?.Debug($"Packed {uploadLevel.data.Count} bytes");
		} catch(System.Threading.Tasks.TaskCanceledException) {
		} catch(System.Exception ex) {
			Log?.Warn($"ShareTask() failed: {ex}");
		}
		return EntitlementsStatus.Ok;
	}

	static async System.Threading.Tasks.Task<EntitlementsStatus> ShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> status, string levelId) =>
		(await status != EntitlementsStatus.Ok) ? status.Result : await ShareTask(levelId);

	[Patch(PatchType.Postfix, typeof(NetworkPlayerEntitlementChecker), "GetEntitlementStatus")]
	public static void NetworkPlayerEntitlementChecker_GetEntitlementStatus(NetworkPlayerEntitlementChecker __instance, ref System.Threading.Tasks.Task<EntitlementsStatus> __result, string levelId) {
		Log?.Debug($"NetworkPlayerEntitlementChecker_GetEntitlementStatus(levelId=\"{levelId}\")");
		if(haveMpCore)
			return;
		__result = ShareWrapper(__result, levelId);
	}

	[Patch(PatchType.Prefix, typeof(MenuRpcManager), nameof(MenuRpcManager.SetIsEntitledToLevel))]
	public static void MenuRpcManager_SetIsEntitledToLevel(string levelId, EntitlementsStatus entitlementStatus) {
		Log?.Debug($"MenuRpcManager_SetIsEntitledToLevel(levelId=\"{levelId}\", entitlementStatus={entitlementStatus})");
		PacketHandler.RecommendPreview? preview = playerData.ResolvePreview(levelId);
		if(PacketHandler.MissingRequirements(preview, entitlementStatus == EntitlementsStatus.Unknown)) {
			entitlementStatus = EntitlementsStatus.NotOwned;
		} else if(entitlementStatus == EntitlementsStatus.Ok && uploadLevel.id == levelId) {
			Log?.Debug($"    Announcing share for `{levelId}`");
			handler.multiplayerSessionManager.Send(new PacketHandler.SetCanShareBeatmap(uploadLevel.id, uploadLevel.hash, (ulong)uploadLevel.data.Count));
		}
		Log?.Debug($"    entitlementStatus={entitlementStatus}");
		PacketHandler.HandleSetIsEntitledToLevel(handler.multiplayerSessionManager.localPlayer, levelId, entitlementStatus);
	}

	public static void Progress(PacketHandler.LoadProgress packet) {
		PacketHandler.HandleLoadProgress(packet, handler.multiplayerSessionManager.localPlayer);
		handler.multiplayerSessionManager.SendUnreliable(packet);
	}

	static async System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> DownloadWrapper(System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> task, Downloader downloader, System.Threading.CancellationToken cancellationToken) {
		Log?.Debug("DownloadWrapper()");
		BeatmapLevelsModel.GetBeatmapLevelResult result = await task;
		if(result.isError)
			return (await downloader.Resolve(Progress, cancellationToken)) ?? result;
		return result;
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	public static void MultiplayerLevelLoader_LoadLevel_pre(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, out bool __state) =>
		__state = (____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.NotLoading && !haveMpCore);

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
	public static void MultiplayerLevelLoader_LoadLevel_post(MultiplayerLevelLoader __instance, bool __state, ILevelGameplaySetupData gameplaySetupData, float initialStartTime, System.Threading.CancellationTokenSource ____getBeatmapCancellationTokenSource, ref System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> ____getBeatmapLevelResultTask) {
		if(__state && handler.downloader?.levelId == gameplaySetupData.beatmapLevel.beatmapLevel.levelID)
			____getBeatmapLevelResultTask = DownloadWrapper(____getBeatmapLevelResultTask, handler.downloader, ____getBeatmapCancellationTokenSource.Token);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
	public static void MultiplayerLevelLoader_Tick_pre(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, out bool __state) =>
		__state = (____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.LoadingBeatmap && !haveMpCore);

	[Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
	public static void MultiplayerLevelLoader_Tick_post(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, bool __state, ILevelGameplaySetupData ____gameplaySetupData) {
		if(__state && ____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.WaitingForCountdown)
			handler.rpcManager.SetIsEntitledToLevel(____gameplaySetupData.beatmapLevel.beatmapLevel.levelID, EntitlementsStatus.Ok);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerConnectedPlayerInstaller), nameof(MultiplayerConnectedPlayerInstaller.InstallBindings))]
	public static void MultiplayerConnectedPlayerInstaller_InstallBindings(MultiplayerConnectedPlayerInstaller __instance, GameplayCoreSceneSetupData ____sceneSetupData) {
		bool zenMode = ____sceneSetupData.gameplayModifiers.zenMode || (Config.Instance.HideOtherLevels && !haveMpEx);
		if(connectInfo?.perPlayerModifiers == true) {
			PlayerData.ModifiersWeCareAbout mods = playerData.lockedModifiers[__instance.Container.Resolve<IConnectedPlayer>().sortIndex];
			____sceneSetupData.gameplayModifiers = ____sceneSetupData.gameplayModifiers.CopyWith(disappearingArrows: mods.disappearingArrows, ghostNotes: mods.ghostNotes, zenMode: zenMode, smallCubes: mods.smallCubes);
		} else {
			____sceneSetupData.gameplayModifiers = ____sceneSetupData.gameplayModifiers.CopyWith(zenMode: zenMode);
		}
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerSessionManager), "Send", typeof(LiteNetLib.Utils.INetSerializable))]
	public static bool MultiplayerSessionManager_Send(MultiplayerSessionManager __instance, LiteNetLib.Utils.INetSerializable message) =>
		(!(Config.Instance.UnreliableState && (message is NodePoseSyncStateNetSerializable || message is StandardScoreSyncStateNetSerializable))).Or(() => __instance.SendUnreliable(message));

	[Patch(PatchType.Prefix, typeof(MultiplayerOutroAnimationController), nameof(MultiplayerOutroAnimationController.AnimateOutro))]
	public static bool MultiplayerOutroAnimationController_AnimateOutro(System.Action onCompleted) =>
		(connectInfo?.skipResults != true).Or(() => onCompleted.Invoke());

	[Patch(PatchType.Postfix, typeof(GameServerPlayersTableView), nameof(GameServerPlayersTableView.SetData))]
	public static void GameServerPlayersTableView_SetData(System.Collections.Generic.List<IConnectedPlayer> sortedPlayers, HMUI.TableView ____tableView) {
		for(uint i = 0; i < playerData.cells.Length; ++i)
			playerData.cells[i].transform = null;
		foreach(GameServerPlayerTableCell cell in ____tableView.visibleCells) {
			UnityEngine.UI.Image background = cell._localPlayerBackgroundImage;
			foreach(UnityEngine.Transform child in background.transform) {
				if(child.gameObject.name == "BeatUpClient_Progress") {
					IConnectedPlayer player = sortedPlayers[cell.idx];
					playerData.cells[player.sortIndex].SetBar((UnityEngine.RectTransform)child, background, player.isMe ? new UnityEngine.Color(0.1254902f, 0.7529412f, 1, 0.2509804f) : new UnityEngine.Color(0, 0, 0, 0));
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
		if(connectInfo == null)
			return;
		int tier = (int)packet.userName[packet.userName.Length-1] - 17;
		if(tier < -1 || tier >= badges.Length)
			return;
		if(tier >= 0)
			playerData.tiers[packet.userId] = (byte)tier;
		packet.userName = packet.userName.Substring(0, packet.userName.Length - 1);
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLobbyAvatarManager), nameof(MultiplayerLobbyAvatarManager.AddPlayer))]
	public static void MultiplayerLobbyAvatarManager_AddPlayer(IConnectedPlayer connectedPlayer, System.Collections.Generic.Dictionary<string, MultiplayerLobbyAvatarController> ____playerIdToAvatarMap) {
		if(!connectedPlayer.isMe && connectInfo != null && playerData.tiers.TryGetValue(connectedPlayer.userId, out byte tier))
			UnityEngine.Object.Instantiate(badges[tier], ____playerIdToAvatarMap[connectedPlayer.userId].transform.GetChild(2));
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerLocalActivePlayerInGameMenuViewController), nameof(MultiplayerLocalActivePlayerInGameMenuViewController.Start))]
	public static void MultiplayerLocalActivePlayerInGameMenuViewController_Start(MultiplayerLocalActivePlayerInGameMenuViewController __instance) {
		MultiplayerPlayersManager multiplayerPlayersManager = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerPlayersManager>()[0];
		if(connectInfo?.perPlayerDifficulty == true && multiplayerPlayersManager.localPlayerStartSeekSongController is MultiplayerLocalActivePlayerFacade) {
			MenuTransitionsHelper menuTransitionsHelper = UnityEngine.Resources.FindObjectsOfTypeAll<MenuTransitionsHelper>()[0];
			MultiplayerConnectedPlayerSongTimeSyncController audioTimeSyncController = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerConnectedPlayerSongTimeSyncController>()[0];
			PreviewDifficultyBeatmap original = new PreviewDifficultyBeatmap(__instance._localPlayerInGameMenuInitData.previewBeatmapLevel, __instance._localPlayerInGameMenuInitData.beatmapCharacteristic, __instance._localPlayerInGameMenuInitData.beatmapDifficulty);
			PreviewDifficultyBeatmap selectedPreview = original;
			IDifficultyBeatmap? selectedBeatmap = null;
			UnityEngine.RectTransform switchButton = UI.CreateButtonFrom(__instance._resumeButton.gameObject, __instance._resumeButton.transform.parent, "SwitchDifficulty", () => {
				MultiplayerLevelScenesTransitionSetupDataSO setupData = menuTransitionsHelper._multiplayerLevelScenesTransitionSetupData;
				setupData.Init(setupData.gameMode, selectedPreview.beatmapLevel, selectedPreview.beatmapDifficulty, selectedPreview.beatmapCharacteristic, selectedBeatmap, setupData.colorScheme, setupData.gameplayCoreSceneSetupData.gameplayModifiers, setupData.gameplayCoreSceneSetupData.playerSpecificSettings, setupData.gameplayCoreSceneSetupData.practiceSettings, setupData.gameplayCoreSceneSetupData.useTestNoteCutSoundEffects);
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
			switchButton.GetComponentInChildren<Polyglot.LocalizedTextMeshProUGUI>().Key = "BEATUP_SWITCH";
			switchButton.gameObject.SetActive(false);
			DifficultyPanel panel = new DifficultyPanel(__instance._mainBar.transform, 1, -2, __instance._levelBar.transform.Find("BG").GetComponent<HMUI.ImageView>(), true);
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
			if(notFound && HarmonyLib.CodeInstructionExtensions.LoadsConstant(instruction, LiteNetLib.NetConstants.DefaultWindowSize)) {
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
		return (haveSiraUtil || initialSceneRegistered).Or(() => {
			restartTransitionData ??= UnityEngine.ScriptableObject.CreateInstance<BeatUpScenesTransitionSetupDataSO>();
			__instance.ClearAndOpenScenes(restartTransitionData, finishCallback: container =>
				UnityEngine.SceneManagement.SceneManager.GetSceneByName(__instance.GetCurrentlyLoadedSceneNames()[0]).GetRootGameObjects()[0].GetComponent<PCAppInit>().TransitionToNextScene());
		});
	}

	public static readonly HarmonyLib.Harmony harmony = new HarmonyLib.Harmony("BeatUpClient");
	public static BeatUpClient? Instance;
	public static IPA.Logging.Logger? Log;
	public static UnityEngine.Sprite defaultPackCover = null!;
	public static UnityEngine.GameObject[] badges = null!;
	public static StringSO customServerHostName = null!;
	public static CustomNetworkConfig? customNetworkConfig = null;
	public static MainSystemInit mainSystemInit = null!;
	public static MainFlowCoordinator mainFlowCoordinator = null!;
	public static BeatmapLevelsModel beatmapLevelsModel = null!;
	public static bool haveSiraUtil = false;
	public static bool haveSongCore = false;
	public static bool haveMpCore = false;
	public static bool haveMpEx = false;

	public static DifficultyPanel lobbyDifficultyPanel = null!;

	public static MultiplayerStatusModel? multiplayerStatusModel = null;
	public static QuickPlaySetupModel? quickPlaySetupModel = null;
	public static void InvalidateModels() {
		if(multiplayerStatusModel != null)
			multiplayerStatusModel._request = null;
		if(quickPlaySetupModel != null)
			quickPlaySetupModel._request = null;
		if(mainFlowCoordinator.childFlowCoordinator is MultiplayerModeSelectionFlowCoordinator multiplayerModeSelectionFlowCoordinator) {
			editServerViewController.Dismiss(true);
			mainFlowCoordinator.DismissFlowCoordinator(multiplayerModeSelectionFlowCoordinator, HMUI.ViewController.AnimationDirection.Horizontal, () => {
				mainFlowCoordinator.PresentMultiplayerModeSelectionFlowCoordinatorWithDisclaimerAndAvatarCreator();
			}, true);
		}
	}

	static UnityEngine.UI.Button editServerButton = null!;
	static void RefreshNetworkConfig() {
		editServerButton.interactable = false;
		if(customNetworkConfig == null)
			return;
		NetworkConfigSO networkConfigPrefab = mainSystemInit._networkConfig;
		string[] hostname = new[] {networkConfigPrefab.masterServerEndPoint.hostName};
		int port = networkConfigPrefab.masterServerEndPoint.port;
		bool forceGameLift = networkConfigPrefab.forceGameLift;
		string? multiplayerStatusUrl = networkConfigPrefab.multiplayerStatusUrl;
		if(!NullableStringHelper.IsNullOrEmpty(customServerHostName.value)) {
			editServerButton.interactable = true;
			hostname = customServerHostName.value.ToLower().Split(new[] {':'});
			if(hostname.Length >= 2)
				int.TryParse(hostname[1], out port);
			forceGameLift = false;
			if(!Config.Instance.Servers.TryGetValue(customServerHostName.value, out multiplayerStatusUrl))
				multiplayerStatusUrl = null;
		}
		string oldMultiplayerStatusUrl = customNetworkConfig.multiplayerStatusUrl;
		Log?.Debug($"CustomNetworkConfig(customServerHostName=\"{hostname[0]}\", port={port}, forceGameLift={forceGameLift}), multiplayerStatusUrl={multiplayerStatusUrl}");
		typeof(CustomNetworkConfig).GetConstructors()[0].Invoke(customNetworkConfig, new object[] {networkConfigPrefab, hostname[0], port, forceGameLift});
		if(!NullableStringHelper.IsNullOrEmpty(multiplayerStatusUrl))
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<multiplayerStatusUrl>k__BackingField").SetValue(customNetworkConfig, (string)multiplayerStatusUrl);
		if(!forceGameLift)
			HarmonyLib.AccessTools.Field(typeof(CustomNetworkConfig), "<serviceEnvironment>k__BackingField").SetValue(customNetworkConfig, (ServiceEnvironment)networkConfigPrefab.serviceEnvironment);
		if(customNetworkConfig.multiplayerStatusUrl != oldMultiplayerStatusUrl)
			InvalidateModels();
	}

	public static void UpdateNetworkConfig(string hostname, string? statusUrl = null) {
		bool statusChanged = true;
		if(!NullableStringHelper.IsNullOrEmpty(hostname)) {
			if(Config.Instance.Servers.TryGetValue(hostname, out string? oldStatusUrl))
				statusChanged = (statusUrl != oldStatusUrl);
			Config.Instance.Servers[hostname] = statusUrl;
			Config.Instance.Changed();
			serverDropdown.Refresh();
		}
		if(!customServerHostName.value.Equals(hostname))
			customServerHostName.value = hostname;
		else if(statusChanged)
			RefreshNetworkConfig();
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
			UnityEngine.RectTransform editHostname = UI.CreateTextbox(Wrapper, "EditHostname", 1, viewController.RefreshStatusPlaceholder, "BEATUP_ENTER_HOSTNAME");
			UnityEngine.RectTransform editStatus = UI.CreateTextbox(Wrapper, "EditStatus", 2, viewController.RefreshStatusPlaceholder);
			editHostname.sizeDelta = new UnityEngine.Vector2(80, editHostname.sizeDelta.y);
			editHostname.localPosition = new UnityEngine.Vector3(0, -1.5f, 0);
			editStatus.sizeDelta = new UnityEngine.Vector2(80, editStatus.sizeDelta.y);
			editStatus.localPosition = new UnityEngine.Vector3(0, -8.5f, 0);
			viewController.editHostnameTextbox = editHostname.GetComponent<HMUI.InputFieldView>();
			viewController.editStatusTextbox = editStatus.GetComponent<HMUI.InputFieldView>();
			viewController.editHostnameTextbox._textLengthLimit = 180;
			viewController.editStatusTextbox._textLengthLimit = 180;
			viewController.editStatusPlaceholder = viewController.editStatusTextbox._placeholderText.GetComponent<HMUI.CurvedTextMeshPro>();
			viewController.cancelButton = Wrapper.Find("Buttons/CancelButton").GetComponent<HMUI.NoTransitionsButton>();

			viewController.keyboard = gameObject.GetComponentInChildren<HMUI.UIKeyboard>();
			foreach(UnityEngine.RectTransform tr in new[] {viewController.keyboard.transform.parent, viewController.keyboard.transform})
				tr.sizeDelta = new UnityEngine.Vector2(tr.sizeDelta.x + 7, tr.sizeDelta.y);
			UI.AddKey(viewController.keyboard, false, 0, -1, UnityEngine.KeyCode.Underscore, '_');
			UI.AddKey(viewController.keyboard, false, 1, -1, UnityEngine.KeyCode.Colon, ':');
			UI.AddKey(viewController.keyboard, false, 2, -2, UnityEngine.KeyCode.Slash, '/');
			UnityEngine.GameObject dot = UI.AddKey(viewController.keyboard, true, 3, -1, UnityEngine.KeyCode.Period, '.');
			((UnityEngine.RectTransform)dot.transform.parent.GetChild(0)).sizeDelta = new UnityEngine.Vector2(14, 7);

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
			RefreshStatusPlaceholder(editStatusTextbox);
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
				flowCoordinator.SetTitle(Polyglot.Localization.Get(edit ? "BEATUP_EDIT_SERVER" : "BEATUP_ADD_SERVER"), HMUI.ViewController.AnimationType.In);
				flowCoordinator._screenSystem.SetBackButton(false, true);
			}
		}

		public void Dismiss(bool immediately = false) =>
			flowCoordinator?.DismissViewController(this, HMUI.ViewController.AnimationDirection.Vertical, null, immediately);

		void HandleOkButtonWasPressed() {
			if(activeKey != null)
				Config.Instance.Servers.Remove(activeKey);
			string? hostname = editHostnameTextbox.text;
			string? status = null;
			if(hostname.Length < 1 || hostname.IndexOf(':') == 0)
				hostname = null;
			else if(editStatusTextbox.text.Length >= 1)
				status = editStatusTextbox.text;
			UpdateNetworkConfig(hostname ?? string.Empty, status);
			Dismiss();
		}
	}

	public static EditServerViewController editServerViewController = null!;

	static UI.ServerDropdown serverDropdown = null!;

	class InstantHoverHint : HMUI.HoverHint {
		public override void OnPointerEnter(UnityEngine.EventSystems.PointerEventData eventData) {
			_hoverHintController._isHiding = false;
			_hoverHintController.StopAllCoroutines();
			_hoverHintController.SetupAndShowHintPanel(this);
		}
	}

	public static void OnSceneLoaded(UnityEngine.SceneManagement.Scene scene, UnityEngine.SceneManagement.LoadSceneMode mode) {
		if(scene.name != "MainMenu")
			return;
		Log?.Debug("load MainMenu");
		if(customNetworkConfig != null) {
			UnityEngine.Transform CreateServerFormView = UnityEngine.Resources.FindObjectsOfTypeAll<CreateServerFormController>()[0].transform;
			UI.CreateValuePicker(CreateServerFormView, "CountdownDuration", "BEATUP_COUNTDOWN_DURATION", new Property<float>(Config.Instance, nameof(Config.CountdownDuration)), new byte[] {(byte)(Config.Instance.CountdownDuration * 4), 0, 12, 20, 32, 40, 60});
			UI.CreateToggle(CreateServerFormView, "SkipResults", "BEATUP_SKIP_RESULTS_PYRAMID", new Property<bool>(Config.Instance, nameof(Config.SkipResults)));
			UI.CreateToggle(CreateServerFormView, "PerPlayerDifficulty", "BEATUP_PER_PLAYER_DIFFICULTY", new Property<bool>(Config.Instance, nameof(Config.PerPlayerDifficulty)));
			UI.CreateToggle(CreateServerFormView, "PerPlayerModifiers", "BEATUP_PER_PLAYER_MODIFIERS", new Property<bool>(Config.Instance, nameof(Config.PerPlayerModifiers)));
			CreateServerFormView.parent.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
			CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.VerticalLayoutGroup>().enabled = true;
			CreateServerFormView.gameObject.GetComponent<UnityEngine.UI.ContentSizeFitter>().enabled = true;
			CreateServerFormView.parent.parent.gameObject.SetActive(true);

			mainFlowCoordinator = UnityEngine.Resources.FindObjectsOfTypeAll<MainFlowCoordinator>()[0];
			MultiplayerModeSelectionViewController multiplayerModeSelectionViewController = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerModeSelectionViewController>()[0];
			TMPro.TextMeshProUGUI customServerEndPointText = multiplayerModeSelectionViewController._customServerEndPointText;
			UnityEngine.UI.Button editColorSchemeButton = UnityEngine.Resources.FindObjectsOfTypeAll<ColorsOverrideSettingsPanelController>()[0]._editColorSchemeButton;
			customServerEndPointText.enabled = false;
			editServerViewController ??= EditServerViewController.Create("BeatUpClient_EditServerView", multiplayerModeSelectionViewController.transform.parent);
			UnityEngine.RectTransform server = UI.CreateSimpleDropdown(customServerEndPointText.transform, "Server", customServerHostName, Config.Instance.Servers.Keys);
			server.sizeDelta = new UnityEngine.Vector2(80, server.sizeDelta.y);
			server.localPosition = new UnityEngine.Vector3(0, 39.5f, 0);
			foreach(UnityEngine.Transform tr in server)
				tr.localPosition = new UnityEngine.Vector3(0, 0, 0);
			serverDropdown = server.GetComponent<UI.ServerDropdown>();
			UnityEngine.RectTransform addButton = UI.CreateButtonFrom(editColorSchemeButton.gameObject, customServerEndPointText.transform, "AddServer", () =>
				editServerViewController.TryPresent(mainFlowCoordinator.childFlowCoordinator, false));
			addButton.localPosition = new UnityEngine.Vector3(-40, 39.5f, 0);
			addButton.Find("Icon").GetComponent<HMUI.ImageView>().sprite = UnityEngine.Resources.FindObjectsOfTypeAll<UnityEngine.Sprite>().First(sprite => sprite.name == "AddIcon");
			UnityEngine.RectTransform editButton = UI.CreateButtonFrom(editColorSchemeButton.gameObject, customServerEndPointText.transform, "EditServer", () =>
				editServerViewController.TryPresent(mainFlowCoordinator.childFlowCoordinator, true));
			editButton.localPosition = new UnityEngine.Vector3(52, 39.5f, 0);
			editServerButton = editButton.GetComponent<UnityEngine.UI.Button>();

			customServerHostName.didChangeEvent -= RefreshNetworkConfig;
			customServerHostName.didChangeEvent += RefreshNetworkConfig;
			if(string.IsNullOrEmpty(customServerHostName.value) || Config.Instance.Servers.TryGetValue(customServerHostName.value, out _))
				RefreshNetworkConfig();
			else
				UpdateNetworkConfig(customServerHostName.value);
		}

		UnityEngine.Transform MultiplayerSettingsPanel = UnityEngine.Resources.FindObjectsOfTypeAll<MultiplayerSettingsPanelController>()[0].transform;
		if(!haveMpEx) {
			UnityEngine.Transform hideLevels = UI.CreateToggle(MultiplayerSettingsPanel, "HideLevels", "BEATUP_HIDE_OTHER_LEVELS", new Property<bool>(Config.Instance, nameof(Config.HideOtherLevels)));
			hideLevels.gameObject.AddComponent<HMUI.Touchable>();
			LocalizedHoverHint hint = hideLevels.gameObject.AddComponent<LocalizedHoverHint>();
			hint.localizedComponent = hideLevels.gameObject.AddComponent<InstantHoverHint>();
			hint.Key = "BEATUP_MAY_IMPROVE_PERFORMANCE";
		}
		infoText = UnityEngine.Object.Instantiate(MultiplayerSettingsPanel.parent.Find("PlayerOptions/ViewPort/Content/SinglePlayerOnlyTitle").gameObject, MultiplayerSettingsPanel);
		infoText.name = "BeatUpClient_Info";
		Polyglot.LocalizedTextMeshProUGUI text = infoText.GetComponentInChildren<Polyglot.LocalizedTextMeshProUGUI>();
		text.localizedComponent.richText = true;
		text.Key = "BEATUP_INFO";

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

	readonly Hive.Versioning.Version pluginVersion;

	[IPA.Init]
	public BeatUpClient(IPA.Logging.Logger pluginLogger, IPA.Loader.PluginMetadata metadata, IPA.Config.Config conf) {
		Instance = this;
		Log = pluginLogger;
		pluginVersion = metadata.HVersion;
		Config.Instance = IPA.Config.Stores.GeneratedStore.Generated<Config>(conf);
		Log?.Debug("Logger initialized.");
	}
	[IPA.OnEnable]
	public void OnEnable() {
		if((uint)HarmonyLib.AccessTools.Field(typeof(NetworkConstants), nameof(NetworkConstants.kProtocolVersion)).GetValue(null) != 8u) {
			Log?.Critical("Unsupported game version!");
			return;
		}
		string localization = "Polyglot\t100\n" +
			"BEATUP_COUNTDOWN_DURATION\t\tCountdown Duration\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SKIP_RESULTS_PYRAMID\t\tSkip Results Pyramid\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_PER_PLAYER_DIFFICULTY\t\tPer-Player Difficulty\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_PER_PLAYER_MODIFIERS\t\tPer-Player Modifiers\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_ADD_SERVER\t\tAdd Server\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_EDIT_SERVER\t\tEdit Server\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_ENTER_HOSTNAME\t\tEnter Hostname\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_HIDE_OTHER_LEVELS\t\tHide notes from other players\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_MAY_IMPROVE_PERFORMANCE\t\tMay improve performance\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			$"BEATUP_INFO\t\tBeatUpClient {pluginVersion} <color=red>| BETA</color> is active.<br>If any issues arise, please contact rcelyte#5372 <b>immediately</b>.\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_VOTE_MODIFIERS\t\tSuggested Modifiers (Vote)\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n" +
			"BEATUP_SWITCH\t\tSwitch\t"+/*French*/"\t"+/*Spanish*/"\t"+/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"+/*Japanese*/"\t"+/*Simplified Chinese*/"\t\t"+/*Korean*/"\t\t\t\t\t\t\t\t\n";
		Polyglot.LocalizationImporter.Import(localization, Polyglot.GoogleDriveDownloadFormat.TSV);

		haveSiraUtil = (IPA.Loader.PluginManager.GetPluginFromId("SiraUtil") != null);
		haveSongCore = (IPA.Loader.PluginManager.GetPluginFromId("SongCore") != null);
		haveMpCore = (IPA.Loader.PluginManager.GetPluginFromId("MultiplayerCore") != null);
		haveMpEx = (IPA.Loader.PluginManager.GetPluginFromId("MultiplayerExtensions") != null);
		Log?.Debug($"haveSiraUtil={haveSiraUtil}");
		Log?.Debug($"haveSongCore={haveSongCore}");
		Log?.Debug($"haveMpCore={haveMpCore}");

		try {
			System.Collections.Generic.List<System.Type> sections = new System.Collections.Generic.List<System.Type>();
			sections.Add(typeof(BeatUpClient));
			if(haveSongCore)
				sections.Add(typeof(BeatUpClient_SongCore));
			if(haveMpCore)
				sections.Add(typeof(BeatUpClient_MpCore));
			Log?.Debug("Warming methods");
			foreach(System.Type type in sections)
				WarmMethods(type);
			Log?.Debug("Loading assets");
			UnityEngine.AssetBundle data = UnityEngine.AssetBundle.LoadFromStream(System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream("BeatUpClient.data"));
			defaultPackCover = data.LoadAllAssets<UnityEngine.Sprite>()[0];
			badges = data.LoadAllAssets<UnityEngine.GameObject>();
			Log?.Debug("Applying patches");
			if(haveMpCore)
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
	public static void GetPreviewInfo(CustomPreviewBeatmapLevel previewBeatmapLevel, ref System.Collections.Generic.IEnumerable<string?> requirements, ref System.Collections.Generic.IEnumerable<string?> suggestions) {
		string? levelHash = HashForLevelID(previewBeatmapLevel.levelID);
		if(NullableStringHelper.IsNullOrEmpty(levelHash))
			return;
		SongCore.Data.ExtraSongData? extraSongData = SongCore.Collections.RetrieveExtraSongData(levelHash);
		if(extraSongData == null)
			return;
		requirements = extraSongData._difficulties.SelectMany(diff => diff.additionalDifficultyData._requirements);
		suggestions = extraSongData._difficulties.SelectMany(diff => diff.additionalDifficultyData._suggestions);
	}
}

static class BeatUpClient_MpCore {
	#if MPCORE_SUPPORT
	static class DiJack {
		static readonly System.Collections.Generic.Dictionary<System.Type, System.Type?> InjectMap = new System.Collections.Generic.Dictionary<System.Type, System.Type?>();
		static void ConcreteBinderNonGeneric_To(Zenject.ConcreteBinderNonGeneric __instance, ref System.Collections.Generic.IEnumerable<System.Type> concreteTypes) {
			System.Type[] newTypes = concreteTypes.ToArray();
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

	public static class Reflection {
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

	public static async System.Threading.Tasks.Task<EntitlementsStatus> MpShareWrapper(System.Threading.Tasks.Task<EntitlementsStatus> status, string levelId, NetworkPlayerEntitlementChecker checker) {
		switch(await status) {
			case EntitlementsStatus.Ok: return await ShareTask(levelId);
			case EntitlementsStatus.NotOwned: return await checker._additionalContentModel.GetLevelEntitlementStatusAsync(levelId, System.Threading.CancellationToken.None) switch {
					AdditionalContentModel.EntitlementStatus.Owned => await ShareTask(levelId),
					AdditionalContentModel.EntitlementStatus.NotOwned => EntitlementsStatus.NotOwned,
					_ => EntitlementsStatus.Unknown,
			};
			default: return status.Result;
		};
	}

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpEntitlementChecker), "GetEntitlementStatus")]
	public static void MpEntitlementChecker_GetEntitlementStatus(MultiplayerCore.Objects.MpEntitlementChecker __instance, ref System.Threading.Tasks.Task<EntitlementsStatus> __result, string levelId) {
		Log?.Debug($"MpEntitlementChecker_GetEntitlementStatus(levelId=\"{levelId}\")");
		__result = MpShareWrapper(__result, levelId, __instance);
	}

	[Patch(PatchType.Prefix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.LoadLevel))]
	public static void MpLevelLoader_LoadLevel_pre(MultiplayerLevelLoader.MultiplayerBeatmapLoaderState ____loaderState, out bool __state) =>
		__state = (____loaderState == MultiplayerLevelLoader.MultiplayerBeatmapLoaderState.NotLoading);

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.LoadLevel))]
	public static void MpLevelLoader_LoadLevel_post(MultiplayerCore.Objects.MpLevelLoader __instance, bool __state, ILevelGameplaySetupData gameplaySetupData, float initialStartTime, System.Threading.CancellationTokenSource ____getBeatmapCancellationTokenSource, ref System.Threading.Tasks.Task<BeatmapLevelsModel.GetBeatmapLevelResult> ____getBeatmapLevelResultTask) =>
		MultiplayerLevelLoader_LoadLevel_post(__instance, __state, gameplaySetupData, initialStartTime, ____getBeatmapCancellationTokenSource, ref ____getBeatmapLevelResultTask);

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Objects.MpLevelLoader), nameof(MultiplayerCore.Objects.MpLevelLoader.Report))]
	public static void MpLevelLoader_Report(double value) =>
		Progress(new PacketHandler.LoadProgress(PacketHandler.LoadProgress.LoadState.Downloading, (ushort)(value * 65535)));

	[PatchOverload(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseMasterServer), new[] {typeof(DnsEndPoint), typeof(string), typeof(System.Nullable<int>)})]
	[PatchOverload(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseMasterServer), new[] {typeof(DnsEndPoint), typeof(string), typeof(System.Nullable<int>), typeof(string)})]
	public static void NetworkConfigPatcher_UseMasterServer(DnsEndPoint endPoint, string statusUrl) =>
		UpdateNetworkConfig(endPoint.ToString(), statusUrl);

	[Patch(PatchType.Postfix, typeof(MultiplayerCore.Patchers.NetworkConfigPatcher), nameof(MultiplayerCore.Patchers.NetworkConfigPatcher.UseOfficialServer))]
	public static void NetworkConfigPatcher_UseOfficialServer() =>
		UpdateNetworkConfig(string.Empty);

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
			IPreviewBeatmapLevel preview = _beatmapLevelProvider.GetBeatmapFromPacket(packet);
			PacketHandler.RecommendPreview current = playerData.previews[player.sortIndex];
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
