using static System.Linq.Enumerable;

static partial class BeatUpClient {
	internal class HashedCustomBeatmapLevel : CustomBeatmapLevel {
		public string hash = string.Empty;
		public HashedCustomBeatmapLevel(CustomPreviewBeatmapLevel preview) : base(preview) {}
	}

	static EnvironmentInfoSO LoadEnvironmentInfo(string environmentName, EnvironmentInfoSO defaultInfo) =>
		Resolve<CustomLevelLoader>()!._environmentSceneInfoCollection.GetEnvironmentInfoBySerializedName(environmentName) ?? defaultInfo;
	static CustomPreviewBeatmapLevel LoadZippedPreviewBeatmapLevel(StandardLevelInfoSaveData info, System.IO.Compression.ZipArchive archive, string levelId, byte[]? fallbackCover) {
		EnvironmentInfoSO envInfo = LoadEnvironmentInfo(info.environmentName, Resolve<CustomLevelLoader>()!._defaultEnvironmentInfo);
		EnvironmentInfoSO envInfo360 = LoadEnvironmentInfo(info.allDirectionsEnvironmentName, Resolve<CustomLevelLoader>()!._defaultAllDirectionsEnvironmentInfo);
		PreviewDifficultyBeatmapSet[] sets = info.difficultyBeatmapSets.Select(difficultyBeatmapSet => {
			BeatmapCharacteristicSO characteristic = Resolve<BeatmapCharacteristicCollectionSO>()!.GetBeatmapCharacteristicBySerializedName(difficultyBeatmapSet.beatmapCharacteristicName);
			if(characteristic == null)
				return null;
			return new PreviewDifficultyBeatmapSet(characteristic, difficultyBeatmapSet.difficultyBeatmaps.Select(diff => {
				diff.difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
				return difficulty;
			}).ToArray());
		}).Where(preview => preview != null).Select(preview => preview!).ToArray();
		try {
			using System.IO.MemoryStream? stream = archive.GetEntry(info.coverImageFilename)?.UncompressedData;
			if(stream != null && stream.Length >= 1)
				fallbackCover = stream.GetBuffer();
		} catch(System.Exception) {}
		MemorySpriteLoader sprite = new MemorySpriteLoader(fallbackCover);
		return new CustomPreviewBeatmapLevel(defaultPackCover, info, string.Empty, sprite, levelId, info.songName, info.songSubName, info.songAuthorName, info.levelAuthorName, info.beatsPerMinute, info.songTimeOffset, info.shuffle, info.shufflePeriod, info.previewStartTime, info.previewDuration, envInfo, envInfo360, sets);
	}
	static async System.Threading.Tasks.Task<UnityEngine.AudioClip?> DecodeAudio(System.IO.Compression.ZipArchiveEntry song, UnityEngine.AudioType type) {
		byte[] songData = new byte[song.Length];
		song.Open().Read(songData, 0, songData.Length);
		System.Net.Sockets.TcpListener host = new System.Net.Sockets.TcpListener(new System.Net.IPEndPoint(System.Net.IPAddress.Any, 0));
		host.Start(1);
		using UnityEngine.Networking.UnityWebRequest www = UnityEngine.Networking.UnityWebRequestMultimedia.GetAudioClip("http://127.0.0.1:" + ((System.Net.IPEndPoint)host.LocalEndpoint).Port, type);
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
	static async System.Threading.Tasks.Task<CustomBeatmapLevel?> LoadZippedBeatmapLevelAsync(CustomPreviewBeatmapLevel preview, byte[] infoData, System.IO.Compression.ZipArchive archive, System.Threading.CancellationToken cancellationToken) {
		StandardLevelInfoSaveData info = preview.standardLevelInfoSaveData;
		System.IO.Compression.ZipArchiveEntry? song = archive.GetEntry(info.songFilename);
		if(song == null) {
			Log.Error($"UnpackLevel() failed: `{info.songFilename}` not found");
			return null;
		}
		UnityEngine.AudioClip? audioClip = await DecodeAudio(song, AudioTypeHelper.GetAudioTypeFromPath(info.songFilename));
		if(audioClip == null)
			return null;
		HashedCustomBeatmapLevel level = new HashedCustomBeatmapLevel(preview);
		using System.Security.Cryptography.SHA1 hash = System.Security.Cryptography.SHA1.Create();
		hash.HashCore(infoData, 0, infoData.Length);
		level.SetBeatmapLevelData(new BeatmapLevelData(audioClip, info.difficultyBeatmapSets.Select(setInfo => {
			BeatmapCharacteristicSO? characteristic = Resolve<BeatmapCharacteristicCollectionSO>()!.GetBeatmapCharacteristicBySerializedName(setInfo.beatmapCharacteristicName);
			CustomDifficultyBeatmapSet beatmapSet = new CustomDifficultyBeatmapSet(characteristic);
			beatmapSet.SetCustomDifficultyBeatmaps(setInfo.difficultyBeatmaps.Select(difficultyInfo => {
				string filename = difficultyInfo.beatmapFilename;
				System.IO.Compression.ZipArchiveEntry file = archive.GetEntry(filename) ?? throw new System.IO.FileNotFoundException("File not found in archive: " + filename);
				byte[] rawData = new byte[file.Length];
				file.Open().Read(rawData, 0, rawData.Length);
				hash.HashCore(rawData, 0, rawData.Length);
				BeatmapSaveDataVersion3.BeatmapSaveData saveData = BeatmapSaveDataVersion3.BeatmapSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(rawData, 0, rawData.Length));
				BeatmapDataBasicInfo basicInfo = BeatmapDataLoader.GetBeatmapDataBasicInfoFromSaveData(saveData);
				difficultyInfo.difficulty.BeatmapDifficultyFromSerializedName(out BeatmapDifficulty difficulty);
				return new CustomDifficultyBeatmap(level, beatmapSet, difficulty, difficultyInfo.difficultyRank, difficultyInfo.noteJumpMovementSpeed, difficultyInfo.noteJumpStartBeatOffset, info.beatsPerMinute, saveData, basicInfo);
			}).ToArray());
			return beatmapSet;
		}).ToArray()));
		level.hash = System.BitConverter.ToString(hash.HashFinal()).Replace("-", string.Empty);
		return level;
	}
	static async System.Threading.Tasks.Task<CustomBeatmapLevel?> UnzipLevel(string levelId, byte[] data, System.Threading.CancellationToken cancellationToken, byte[]? fallbackCover = null) {
		try {
			using System.IO.MemoryStream stream = new System.IO.MemoryStream(data);
			using System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(stream, System.IO.Compression.ZipArchiveMode.Read);
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
			infoFile.Open().Read(infoData, 0, infoData.Length);
			StandardLevelInfoSaveData info = StandardLevelInfoSaveData.DeserializeFromJSONString(System.Text.Encoding.UTF8.GetString(infoData, 0, infoData.Length));
			CustomPreviewBeatmapLevel preview = LoadZippedPreviewBeatmapLevel(info, archive, levelId, fallbackCover);
			return await LoadZippedBeatmapLevelAsync(preview, infoData, archive, cancellationToken);
		} catch(System.Exception ex) {
			Log.Critical($"Error in UnpackLevel(): {ex}");
			return null;
		}
	}
}
