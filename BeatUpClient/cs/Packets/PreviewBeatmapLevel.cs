static partial class BeatUpClient {
	static string? HashForLevelID([System.Diagnostics.CodeAnalysis.NotNullWhen(true)] string? levelId) {
		string[] parts = (levelId ?? string.Empty).Split(new[] {'_', ' '});
		if(parts.Length < 3 || parts[2].Length != 40)
			return null;
		return parts[2];
	}

	internal abstract class PreviewBeatmapLevel : BeatUpPacket, IPreviewBeatmapLevel {
		bool mpCore;
		public string? levelID {get;} = string.Empty;
		public string? songName {get;} = string.Empty;
		public string? songSubName {get;} = string.Empty;
		public string? songAuthorName {get;} = string.Empty;
		public string? levelAuthorName {get;} = string.Empty;
		public float beatsPerMinute {get;}
		public float songTimeOffset {get;}
		public float shuffle {get;}
		public float shufflePeriod {get;}
		public float previewStartTime {get;}
		public float previewDuration {get;}
		public float songDuration {get;}
		public System.Collections.Generic.IReadOnlyList<PreviewDifficultyBeatmapSet>? previewDifficultyBeatmapSets {get; protected set;} = null;
		public EnvironmentInfoSO? environmentInfo {get;} = null;
		public EnvironmentInfoSO? allDirectionsEnvironmentInfo {get;} = null;
		public readonly ByteArrayNetSerializable cover = new ByteArrayNetSerializable("cover", 0, 8192);
		UnityEngine.Sprite? localSprite = null;
		public async System.Threading.Tasks.Task<UnityEngine.Sprite> GetCoverImageAsync(System.Threading.CancellationToken cancellationToken) {
			Log.Debug("PreviewBeatmapLevel.GetCoverImageAsync()");
			localSprite ??= await new MemorySpriteLoader(cover.data).LoadSpriteAsync("", cancellationToken);
			localSprite ??= defaultPackCover;
			return localSprite;
		}
		public void SetCover(UnityEngine.Sprite? fullSprite) {
			localSprite = fullSprite;
			if(fullSprite == null) {
				cover.data = new byte[0];
				return;
			}
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
				if(data.Length > 8192)
					continue;
				cover.data = data;
				return;
			}
			cover.data = new byte[0];
		}
		public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((string?)(mpCore ? HashForLevelID(levelID) : levelID));
			writer.Put((string?)songName);
			writer.Put((string?)songSubName);
			writer.Put((string?)songAuthorName);
			writer.Put((string?)levelAuthorName);
			writer.Put((float)beatsPerMinute);
			if(mpCore) {
				writer.Put((float)songDuration);
				return;
			}
			writer.Put((float)songTimeOffset);
			writer.Put((float)shuffle);
			writer.Put((float)shufflePeriod);
			writer.Put((float)previewStartTime);
			writer.Put((float)previewDuration);
			writer.Put((float)songDuration);
			writer.Put((string?)environmentInfo?.serializedName);
			writer.Put((string?)allDirectionsEnvironmentInfo?.serializedName);
			writer.Put((byte)UpperBound((uint)(previewDifficultyBeatmapSets?.Count ?? 0), 8));
			foreach(PreviewDifficultyBeatmapSet previewDifficultyBeatmapSet in previewDifficultyBeatmapSets ?? new PreviewDifficultyBeatmapSet[0]) {
				writer.Put((string)previewDifficultyBeatmapSet.beatmapCharacteristic.serializedName);
				writer.Put((byte)UpperBound((uint)previewDifficultyBeatmapSet.beatmapDifficulties.Length, 5));
				foreach(BeatmapDifficulty difficulty in previewDifficultyBeatmapSet.beatmapDifficulties)
					writer.PutVarUInt((uint)difficulty);
			}
			cover.Serialize(writer);
		}
		PreviewBeatmapLevel(bool mpCore) =>
			(this.mpCore, cover.data) = (mpCore, new byte[0]);
		protected PreviewBeatmapLevel(LiteNetLib.Utils.NetDataReader reader, bool mpCore) : this(mpCore) {
			(levelID, songName, songSubName, songAuthorName, levelAuthorName) =
				(reader.GetString(), reader.GetString(), reader.GetString(), reader.GetString(), reader.GetString());
			beatsPerMinute = reader.GetFloat();
			if(mpCore) {
				songDuration = reader.GetFloat();
				levelID = $"custom_level_{levelID}";
				return;
			}
			(songTimeOffset, shuffle, shufflePeriod, previewStartTime, previewDuration, songDuration) =
				(reader.GetFloat(), reader.GetFloat(), reader.GetFloat(), reader.GetFloat(), reader.GetFloat(), reader.GetFloat());
			environmentInfo = Resolve<CustomLevelLoader>()!._environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(reader.GetString());
			allDirectionsEnvironmentInfo = Resolve<CustomLevelLoader>()!._environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(reader.GetString());
			uint count = UpperBound(reader.GetByte(), 8);
			previewDifficultyBeatmapSets = (count < 1) ? null : CreateArray(count, i => {
				// TODO: need a good solution for handling `lawless`, `lightshow`, etc when SongCore isn't available
				BeatmapCharacteristicSO? characteristic = SerializedCharacteristic(reader.GetString());
				BeatmapDifficulty[] difficulties = CreateArray(UpperBound(reader.GetByte(), 5), i => (BeatmapDifficulty)reader.GetVarUInt());
				return new PreviewDifficultyBeatmapSet(characteristic, difficulties);
			});
			cover.Deserialize(reader);
		}
		protected PreviewBeatmapLevel(IPreviewBeatmapLevel? prv, bool mpCore) : this(mpCore) {
			if(prv == null)
				return;
			Log.Debug($"PreviewBeatmapLevel(beatmapLevel={prv}, previewDifficultyBeatmapSets={previewDifficultyBeatmapSets}, mpCore={mpCore}) pre");
			(levelID, songName, songSubName, songAuthorName, levelAuthorName, beatsPerMinute, songTimeOffset, shuffle, shufflePeriod, previewStartTime, previewDuration, songDuration) =
				(prv.levelID, prv.songName, prv.songSubName, prv.songAuthorName, prv.levelAuthorName, prv.beatsPerMinute, prv.songTimeOffset, prv.shuffle, prv.shufflePeriod, prv.previewStartTime, prv.previewDuration, prv.songDuration);
			previewDifficultyBeatmapSets = prv.previewDifficultyBeatmapSets;
			cover.data = new byte[0];
			if(prv is PreviewBeatmapLevel preview) {
				cover.data = preview.cover.data;
			} else if(!mpCore) {
				System.Threading.Tasks.Task<UnityEngine.Sprite?> spriteTask = prv.GetCoverImageAsync(System.Threading.CancellationToken.None);
				if(spriteTask.IsCompleted) { // `Wait()`ing on an async method will deadlock
					SetCover(spriteTask.Result);
					Log.Debug($"    Cover size: {cover.data.Length} bytes");
				} else {
					Log.Debug($"    Cover not encoded; operation would block");
				}
			}
			environmentInfo = prv.environmentInfo;
			allDirectionsEnvironmentInfo = prv.allDirectionsEnvironmentInfo;
			Log.Debug($"PreviewBeatmapLevel(beatmapLevel={prv}, previewDifficultyBeatmapSets={previewDifficultyBeatmapSets}, mpCore={mpCore}) post");
		}
	}
}
