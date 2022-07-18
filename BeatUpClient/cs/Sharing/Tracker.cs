using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static class ShareTracker { // TODO: limit memory usage
		static System.Collections.Generic.Dictionary<string, System.Collections.Generic.List<Downloader>> levels = new System.Collections.Generic.Dictionary<string, System.Collections.Generic.List<Downloader>>();

		static void OnDisconnect(IConnectedPlayer player) {
			ConnectedPlayerManager.ConnectedPlayer? connectedPlayer = player as ConnectedPlayerManager.ConnectedPlayer;
			if(connectedPlayer == null)
				return;
			string[] levelsToRemove = levels.Where(level => {
				level.Value.RemoveAll(dl => dl.Remove(connectedPlayer) == 0);
				return level.Value.Count == 0;
			}).Select(level => level.Key).ToArray();
			foreach(string key in levelsToRemove)
				levels.Remove(key);
		}
		static void OnShareInfo(ShareInfo info, IConnectedPlayer player) {
			ConnectedPlayerManager.ConnectedPlayer? connectedPlayer = player as ConnectedPlayerManager.ConnectedPlayer;
			if(connectedPlayer == null || info.blockSize != LocalBlockSize)
				return;
			if(info.id.usage == ShareableType.None) {
				string[] levelsToRemove = levels.Where(level => {
					level.Value.RemoveAll(dl => dl.Remove(connectedPlayer, info.offset) == 0);
					return level.Value.Count == 0;
				}).Select(level => level.Key).ToArray();
				foreach(string key in levelsToRemove)
					levels.Remove(key);
				return;
			}
			if(info.id.usage == ShareableType.BeatmapSet && info.id.mimeType == "application/json" && info.meta.byteLength > 0) {
				if(!levels.TryGetValue(info.id.name, out System.Collections.Generic.List<Downloader> variants)) {
					variants = new System.Collections.Generic.List<Downloader>(1);
					levels.Add(info.id.name, variants);
				}
				Downloader? source = variants.FirstOrDefault(d => d.meta.Equals(info.meta));
				if(source == null) {
					source = new Downloader(info.meta);
					variants.Add(source);
				}
				source.Add(connectedPlayer, info.offset);
			}
		}
		public static bool CanFetch(string levelId) =>
			levels.ContainsKey(levelId);
		public static async System.Threading.Tasks.Task<(Hash256, byte[]?)> Fetch(string levelId, System.Action<ushort>? progress = null) {
			if(levels.TryGetValue(levelId, out System.Collections.Generic.List<Downloader> variants)) {
				Downloader source = variants[0];
				return (source.meta.hash, await source.Fetch(progress, out System.Threading.CancellationTokenSource _));
			}
			return (default, null);
		}
		// TODO: clear `levels` on disconnect
		[Init]
		public static void Init() {
			Net.onDisconnect += OnDisconnect;
			Net.onShareInfo += OnShareInfo;
		}
	}
}
