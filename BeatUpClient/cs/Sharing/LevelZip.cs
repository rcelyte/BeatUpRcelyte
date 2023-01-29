using static System.Linq.Enumerable;

static partial class BeatUpClient {
	class CallbackStream : System.IO.Stream {
		readonly System.IO.Stream stream;
		readonly System.Action<int> onProgress;
		public override bool CanRead => stream.CanRead;
		public override bool CanSeek => stream.CanSeek;
		public override bool CanWrite => stream.CanWrite;
		public override long Length => stream.Length;
		public override long Position {
			get => stream.Position;
			set => stream.Position = value;
		}
		int UpdateProgress(int count) {
			onProgress(count);
			return count;
		}
		public CallbackStream(System.IO.Stream stream, System.Action<int> onProgress) =>
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

	static async System.Threading.Tasks.Task<(Hash256, System.ArraySegment<byte>)> ZipLevel(CustomBeatmapLevel level, System.Threading.CancellationToken cancellationToken, System.Action<ushort>? onProgress = null) {
		(string path, string name)[] files = level.standardLevelInfoSaveData.difficultyBeatmapSets
			.SelectMany(set => set.difficultyBeatmaps)
			.Select(diff => (System.IO.Path.Combine(level.customLevelPath, diff.beatmapFilename), diff.beatmapFilename))
			.Prepend((level.songAudioClipPath, level.standardLevelInfoSaveData.songFilename))
			.Prepend((System.IO.Path.Combine(level.customLevelPath, "Info.dat"), "Info.dat"))
			.ToArray();
		ulong totalLength = files.Aggregate(0LU, (total, file) => total + (ulong)new System.IO.FileInfo(file.path).Length);
		if(totalLength > MaxUnzippedSize) {
			Log.Warn("Level too large for sharing!");
			return (default, default);
		}
		ulong progress = 0, progressLength = 53738, progressOffset = 0;
		System.Diagnostics.Stopwatch rateLimit = System.Diagnostics.Stopwatch.StartNew();
		System.Collections.IEnumerator SetProgressSafe(ushort percent) {
			yield return null;
			onProgress?.Invoke(percent);
		}
		void HandleProgress(int delta) {
			progress += (ulong)delta;
			if(rateLimit.ElapsedMilliseconds < 28)
				return;
			rateLimit.Restart();
			ushort percent = (ushort)(progress * progressLength / totalLength + progressOffset);
			PersistentSingleton<SharedCoroutineStarter>.instance.StartCoroutine(SetProgressSafe(percent));
		}
		using System.IO.MemoryStream memoryStream = new System.IO.MemoryStream();
		using(System.IO.Compression.ZipArchive archive = new System.IO.Compression.ZipArchive(memoryStream, System.IO.Compression.ZipArchiveMode.Create, true)) {
			foreach((string path, string name) file in files) {
				Log.Debug($"    Compressing `{file.path}`");
				using System.IO.Stream fileStream = System.IO.File.Open(file.path, System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.Read);
				using System.IO.Stream entry = archive.CreateEntry(file.name).Open();
				await fileStream.CopyToAsync(new CallbackStream(entry, HandleProgress), 81920, cancellationToken);
			}
		}
		(Hash256 hash, System.ArraySegment<byte> buffer) res = (new Hash256(), default);
		if(!memoryStream.TryGetBuffer(out res.buffer))
			return res;
		progress = 0;
		totalLength = (ulong)res.buffer.Count;
		progressOffset = progressLength;
		progressLength = 65535 - progressOffset;
		memoryStream.Seek(0, System.IO.SeekOrigin.Begin);
		res.hash = new(await System.Threading.Tasks.Task.Run(() => {
			using System.Security.Cryptography.SHA256 sha256 = System.Security.Cryptography.SHA256.Create();
			return sha256.ComputeHash(new CallbackStream(memoryStream, HandleProgress)); // TODO: need `cancellationToken` here
		}));
		Log.Debug($"    Hashed {totalLength} bytes");
		return res;
	}
}
