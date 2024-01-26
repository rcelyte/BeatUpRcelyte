using static System.Linq.Enumerable;

static partial class BeatUpClient {
	internal struct SharedBeatmapLevelData : IBeatmapLevelData, IAssetSongAudioClipProvider {
		public struct DifficultyKey {
			public BeatmapCharacteristicSO characteristic;
			public BeatmapDifficulty difficulty;
		};
		public struct DifficultyData {
			public string lightshowFilename, beatmapFilename;
		};
		public string? audioData;
		public System.Collections.Generic.Dictionary<DifficultyKey, (string? lightshow, string? beatmap)> difficulties;
		UnityEngine.AudioClip songAudio;
		public string hash;
		public static async System.Threading.Tasks.Task<SharedBeatmapLevelData> From(System.IO.Compression.ZipArchive archive, byte[] infoData, string songFilename, string audioDataFilename, System.Collections.Generic.IEnumerable<System.Collections.Generic.KeyValuePair<DifficultyKey, DifficultyData>> difficultyFilenames) {
			using System.Security.Cryptography.SHA1 hash = System.Security.Cryptography.SHA1.Create();
			hash.HashCore(infoData, 0, infoData.Length);
			string? ReadFile(string filename) {
				if(string.IsNullOrEmpty(filename))
					return null;
				System.IO.Compression.ZipArchiveEntry file = archive.GetEntry(filename) ?? throw new System.IO.FileNotFoundException("File not found in archive: " + filename);
				byte[] rawData = new byte[file.Length];
				file.Open().Read(rawData, 0, rawData.Length);
				hash.HashCore(rawData, 0, rawData.Length);
				return System.Text.Encoding.UTF8.GetString(rawData, 0, rawData.Length);
			}
			System.IO.Compression.ZipArchiveEntry songFile = archive.GetEntry(songFilename) ?? throw new System.IO.FileNotFoundException("File not found in archive: " + songFilename);
			return new SharedBeatmapLevelData {
				audioData = ReadFile(audioDataFilename),
				difficulties = new(difficultyFilenames.Select(difficulty => System.Collections.Generic.KeyValuePair.Create(
					difficulty.Key, (ReadFile(difficulty.Value.lightshowFilename), ReadFile(difficulty.Value.beatmapFilename))))),
				songAudio = await DecodeAudio(songFile, AudioTypeHelper.GetAudioTypeFromPath(songFilename)),
				hash = System.BitConverter.ToString(hash.HashFinal()).Replace("-", string.Empty),
			};
		}
		public string? GetAudioDataString() => audioData;
		public string? GetBeatmapString(in BeatmapKey key) {
			difficulties.TryGetValue(new DifficultyKey {characteristic = key.beatmapCharacteristic, difficulty = key.difficulty}, out var difficulty);
			return difficulty.beatmap;
		}
		public string? GetLightshowString(in BeatmapKey key) {
			difficulties.TryGetValue(new DifficultyKey {characteristic = key.beatmapCharacteristic, difficulty = key.difficulty}, out var difficulty);
			return difficulty.lightshow;
		}
		System.Threading.Tasks.Task<string?> IBeatmapLevelData.GetAudioDataStringAsync() =>
			System.Threading.Tasks.Task.FromResult(GetAudioDataString());
		System.Threading.Tasks.Task<string?> IBeatmapLevelData.GetBeatmapStringAsync(in BeatmapKey key) =>
			System.Threading.Tasks.Task.FromResult(GetBeatmapString(key));
		System.Threading.Tasks.Task<string?> IBeatmapLevelData.GetLightshowStringAsync(in BeatmapKey key) =>
			System.Threading.Tasks.Task.FromResult(GetLightshowString(key));
		UnityEngine.AudioClip IAssetSongAudioClipProvider.songAudioClip => songAudio;
	}
	static async System.Threading.Tasks.Task<UnityEngine.AudioClip> DecodeAudio(System.IO.Compression.ZipArchiveEntry song, UnityEngine.AudioType type) {
		using System.IO.Stream stream = song.Open();
		byte[] songData = new byte[song.Length];
		stream.Read(songData, 0, songData.Length);
		System.Net.Sockets.TcpListener host = new System.Net.Sockets.TcpListener(new System.Net.IPEndPoint(System.Net.IPAddress.Any, 0));
		host.Start(1);
		using UnityEngine.Networking.UnityWebRequest www = UnityEngine.Networking.UnityWebRequestMultimedia.GetAudioClip("http://127.0.0.1:" + ((System.Net.IPEndPoint)host.LocalEndpoint).Port, type);
		((UnityEngine.Networking.DownloadHandlerAudioClip)www.downloadHandler).streamAudio = true;
		UnityEngine.AsyncOperation request = www.SendWebRequest();
		System.Net.Sockets.TcpClient client = host.AcceptTcpClient();
		System.Net.Sockets.NetworkStream netStream = client.GetStream();
		byte[] resp = System.Text.Encoding.ASCII.GetBytes($"HTTP/1.1 200 \r\naccept-ranges: bytes\r\ncontent-length: {songData.Length}\r\ncontent-type: audio/ogg\r\n\r\n");
		netStream.Write(resp, 0, resp.Length);
		netStream.Write(songData, 0, songData.Length);
		while(!request.isDone)
			await System.Threading.Tasks.Task.Delay(100);
		client.Close();
		host.Stop();
		if(www.error is string error)
			throw new System.Exception($"Audio load error: {error}");
		return UnityEngine.Networking.DownloadHandlerAudioClip.GetContent(www);
	}
	static System.Threading.Tasks.Task<SharedBeatmapLevelData> UnzipV3(string infoText, byte[] infoData, System.IO.Compression.ZipArchive archive) {
		StandardLevelInfoSaveData info = StandardLevelInfoSaveData.DeserializeFromJSONString(infoText);
		return SharedBeatmapLevelData.From(archive, infoData, info.songFilename, string.Empty, info.difficultyBeatmapSets
			.SelectMany((StandardLevelInfoSaveData.DifficultyBeatmapSet set) => {
				BeatmapCharacteristicSO? characteristic = Resolve<BeatmapCharacteristicCollection>()!
					.GetBeatmapCharacteristicBySerializedName(set.beatmapCharacteristicName);
				return set.difficultyBeatmaps.Select((StandardLevelInfoSaveData.DifficultyBeatmap beatmap) =>
					beatmap.difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty parsedDifficulty) ?
						System.Collections.Generic.KeyValuePair.Create(new SharedBeatmapLevelData.DifficultyKey {
							characteristic = characteristic,
							difficulty = parsedDifficulty,
						}, new SharedBeatmapLevelData.DifficultyData {
							lightshowFilename = string.Empty,
							beatmapFilename = beatmap.beatmapFilename,
						}) : default);
			})
			.Where(data => data.Value.beatmapFilename != null));
	}

	static System.Threading.Tasks.Task<SharedBeatmapLevelData> UnzipV4(string infoText, byte[] infoData, System.IO.Compression.ZipArchive archive) {
		BeatmapLevelSaveDataVersion4.BeatmapLevelSaveData info =
			UnityEngine.JsonUtility.FromJson<BeatmapLevelSaveDataVersion4.BeatmapLevelSaveData>(infoText);
		return SharedBeatmapLevelData.From(archive, infoData, info.audio.songFilename, info.audio.audioDataFilename, info.difficultyBeatmaps
			.Select((BeatmapLevelSaveDataVersion4.BeatmapLevelSaveData.DifficultyBeatmap beatmap) => {
				BeatmapCharacteristicSO? characteristic = Resolve<BeatmapCharacteristicCollection>()!
					.GetBeatmapCharacteristicBySerializedName(beatmap.characteristic);
				return beatmap.difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty parsedDifficulty) ?
					System.Collections.Generic.KeyValuePair.Create(new SharedBeatmapLevelData.DifficultyKey {
						characteristic = characteristic,
						difficulty = parsedDifficulty,
					}, new SharedBeatmapLevelData.DifficultyData {
						lightshowFilename = beatmap.lightshowDataFilename,
						beatmapFilename = beatmap.beatmapDataFilename,
					}) : default;
			})
			.Where(data => data.Value.beatmapFilename != null));
	}

	static async System.Threading.Tasks.Task<IBeatmapLevelData?> UnzipLevel(string levelId, byte[] data, System.Threading.CancellationToken cancellationToken, byte[]? fallbackCover = null) {
		try {
			using System.IO.MemoryStream stream = new(data);
			using System.IO.Compression.ZipArchive archive = new(stream, System.IO.Compression.ZipArchiveMode.Read);
			ulong totalLength = 0;
			foreach(System.IO.Compression.ZipArchiveEntry entry in archive.Entries) {
				totalLength += (ulong)entry.Length;
				if((ulong)entry.Length <= MaxUnzippedSize && totalLength <= MaxUnzippedSize)
					continue;
				Log.Error("UnpackLevel() failed: Unzipped file too large");
				return null;
			}
			System.IO.Compression.ZipArchiveEntry? infoFile = archive.GetEntry("Info.dat");
			if(infoFile == null) {
				Log.Error("UnpackLevel() failed: `Info.dat` not found");
				return null;
			}
			byte[] infoData = new byte[infoFile.Length];
			await infoFile.Open().ReadAsync(infoData, 0, infoData.Length);
			string infoText = System.Text.Encoding.UTF8.GetString(infoData, 0, infoData.Length);
			return (BeatmapSaveDataHelpers.GetVersion(infoText) < BeatmapSaveDataHelpers.version4) ?
				await UnzipV3(infoText, infoData, archive) :
				await UnzipV4(infoText, infoData, archive);
		} catch(System.Exception ex) {
			Log.Critical($"Error in UnpackLevel(): {ex}");
			return null;
		}
	}
}
