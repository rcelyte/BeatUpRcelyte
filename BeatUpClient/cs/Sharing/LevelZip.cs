using static System.Linq.Enumerable;

static partial class BeatUpClient {
	class CallbackStream : System.IO.Stream {
		readonly System.IO.Stream stream;
		readonly System.Action<uint> onProgress;
		public override bool CanRead => stream.CanRead;
		public override bool CanSeek => stream.CanSeek;
		public override bool CanWrite => stream.CanWrite;
		public override long Length => stream.Length;
		public override long Position {
			get => stream.Position;
			set => stream.Position = value;
		}
		int UpdateProgress(int count) {
			onProgress((uint)count);
			return count;
		}
		public CallbackStream(System.IO.Stream stream, System.Action<uint> onProgress) =>
			(this.stream, this.onProgress) = (stream, onProgress);
		public override void Flush() => stream.Flush();
		public override long Seek(long offset, System.IO.SeekOrigin origin) => stream.Seek(offset, origin);
		public override void SetLength(long value) => stream.SetLength(value);
		public override int Read(byte[] buffer, int offset, int count) =>
			UpdateProgress(stream.Read(buffer, offset, count));
		public override void Write(byte[] buffer, int offset, int count) => stream.Write(buffer, offset, count);
		public override async System.Threading.Tasks.Task WriteAsync(byte[] buffer, int offset, int count, System.Threading.CancellationToken cancellationToken) {
			await stream.WriteAsync(buffer, offset, count, cancellationToken);
			UpdateProgress(count);
		}
	}

	static async System.Threading.Tasks.Task<(Hash256, System.ArraySegment<byte>)> ZipLevel(FileSystemBeatmapLevelData levelData, System.Threading.CancellationToken cancellationToken, System.Action<ushort>? onProgress = null) {
		string customLevelPath = System.IO.Path.GetDirectoryName(levelData.songAudioClipPath);
		System.Collections.Generic.List<System.IO.FileInfo> files = new(3 + levelData._difficultyBeatmaps.Count * 2);
		files.Add(new(System.IO.Path.Combine(customLevelPath, "Info.dat")));
		if(!string.IsNullOrEmpty(levelData._audioDataPath))
			files.Add(new(levelData._audioDataPath));
		foreach(FileDifficultyBeatmap diff in levelData._difficultyBeatmaps.Values) {
			if(!string.IsNullOrEmpty(diff.lightshowPath) && System.IO.File.Exists(diff.lightshowPath))
				files.Add(new(diff.lightshowPath));
			if(!string.IsNullOrEmpty(diff.beatmapPath) && System.IO.File.Exists(diff.beatmapPath))
				files.Add(new(diff.beatmapPath));
		}
		files.Add(new(levelData.songAudioClipPath));

		ulong totalLength = files.Aggregate(0LU, (total, file) => total + (ulong)file.Length);
		if(totalLength > MaxUnzippedSize) {
			Log.Warn("Level too large for sharing!");
			return (default, default);
		}
		ulong progressOffset = 0, progressLength = 53738;
		System.Diagnostics.Stopwatch rateLimit = System.Diagnostics.Stopwatch.StartNew();
		void UpdateProgress(ulong progress) {
			if(rateLimit.ElapsedMilliseconds < 28)
				return;
			rateLimit.Restart();
			ushort percent = (ushort)(progress * progressLength / totalLength + progressOffset);
			onProgress?.Invoke(percent);
		}
		using System.IO.MemoryStream memoryStream = new System.IO.MemoryStream();
		using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(memoryStream, System.IO.Compression.ZipArchiveMode.Create, true)) {
			ulong progress = 0;
			foreach(System.IO.FileInfo file in files) {
				Log.Debug($"    Compressing `{file.Name}`");
				using System.IO.Stream fileStream = file.OpenRead();
				using System.IO.Stream entry = archive.CreateEntry(file.Name).Open();
				await fileStream.CopyToAsync(new CallbackStream(entry, delta => UpdateProgress(progress += delta)), 81920, cancellationToken);
			}
		}
		(Hash256 hash, System.ArraySegment<byte> buffer) res = (default, default);
		if(!memoryStream.TryGetBuffer(out res.buffer))
			return res;
		totalLength = (ulong)res.buffer.Count;
		(progressOffset, progressLength) = (progressLength, 65535 - progressLength);
		res.hash = await Hash256.ComputeAsync(res.buffer, UpdateProgress);
		Log.Debug($"    Hashed {totalLength} bytes");
		return res;
	}
}
