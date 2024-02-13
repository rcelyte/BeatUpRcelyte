using static System.Linq.Enumerable;

static partial class BeatUpClient {
	static class ShareTracker {
		public static event System.Action<ShareInfo, IConnectedPlayer, bool>? onProcess = null;

		public class DownloadPreview : BeatmapLevel {
			public System.Collections.Generic.List<Downloader> variants;
			public DownloadPreview(ShareInfo info, ConnectedPlayerManager.ConnectedPlayer connectedPlayer) :
					base(false, info.id.name, string.Empty, string.Empty, string.Empty, new string[0], new string[0], 0, -6f, 0, 0, 0, 0, PlayerSensitivityFlag.Safe, new StaticPreviewMediaData(defaultPackCover, null), null) {
				this.variants = new System.Collections.Generic.List<Downloader>() {new Downloader(info, connectedPlayer)};
			}
			public System.Threading.Tasks.Task<byte[]?> Fetch(System.Action<ushort>? progress = null) =>
				variants.FirstOrDefault()?.Fetch(progress, out System.Threading.CancellationTokenSource _) ??
					System.Threading.Tasks.Task.FromResult<byte[]?>(null);
		}

		static System.Collections.Generic.List<DownloadPreview> trackedLevels = new System.Collections.Generic.List<DownloadPreview>();
		static void FilterLevels(System.Predicate<Downloader> filter) {
			trackedLevels.RemoveAll(level => {
				level.variants.RemoveAll(filter);
				if(level.variants.Count == 0)
					Resolve<BeatmapLevelsModel>()!._loadedBeatmapLevels.Remove(level.levelID);
				return level.variants.Count == 0;
			});
		}
		static void OnDisconnect(IConnectedPlayer player) {
			if(player is ConnectedPlayerManager.ConnectedPlayer connectedPlayer)
				FilterLevels(dl => dl.Remove(connectedPlayer) == 0);
		}
		static bool ProcessShareInfo(ShareInfo info, IConnectedPlayer player) {
			ConnectedPlayerManager.ConnectedPlayer? connectedPlayer = player as ConnectedPlayerManager.ConnectedPlayer;
			if(connectedPlayer == null || info.blockSize != LocalBlockSize)
				return false;
			if(info.id.usage == ShareableType.None) {
				FilterLevels(dl => dl.Remove(connectedPlayer, info.offset) == 0);
				return false;
			}
			if(info.id.usage != ShareableType.BeatmapSet || info.id.mimeType != "application/json" || info.meta.byteLength < 1)
				return false;
			if(Resolve<BeatmapLevelsModel>()!._loadedBeatmapLevels.TryGetValue(info.id.name, out BeatmapLevel preview)) {
				DownloadPreview? share = preview as DownloadPreview;
				if(share == null)
					return false;
				Downloader? source = share.variants.FirstOrDefault(d => d.meta.Equals(info.meta));
				if(source == null)
					share.variants.Add(new Downloader(info, connectedPlayer));
				else
					source.Add(connectedPlayer, info.offset);
			} else {
				DownloadPreview share = new DownloadPreview(info, connectedPlayer);
				Resolve<BeatmapLevelsModel>()!._loadedBeatmapLevels[info.id.name] = share;
				trackedLevels.Add(share);
			}
			return true;
		}
		public static void OnShareInfo(ShareInfo info, IConnectedPlayer player) {
			bool processed = false;
			try {
				processed = ProcessShareInfo(info, player);
			} finally {
				onProcess?.Invoke(info, player, processed);
			}
		}
		// TODO: clear `levels` on disconnect
		[Init]
		public static void Init() {
			Net.onDisconnect += OnDisconnect;
		}
	}
}
