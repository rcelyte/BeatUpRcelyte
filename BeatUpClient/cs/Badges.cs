static partial class BeatUpClient {
	static class Badges {
		static async void OnShareInfo(ShareInfo info, IConnectedPlayer player) {
			if(!player.isConnectionOwner || info.id.usage != ShareableType.Generic || info.id.mimeType != "application/x-unityfs")
				return;
			ConnectedPlayerManager.ConnectedPlayer? connectedPlayer = player as ConnectedPlayerManager.ConnectedPlayer;
			if(connectedPlayer == null)
				return;
			string[] desc = info.id.name.Split(new[] {':'}, 3);
			if(desc[0] != "BeatUpClient" || desc[1] != "badge")
				return;
			Downloader source = new Downloader(info.meta);
			source.Add(connectedPlayer, info.offset);
			byte[]? data = await source.Fetch(null, out System.Threading.CancellationTokenSource _);
			if(data == null)
				return;
			// TODO: verify signature
			// TODO: load asset
			// TODO: cache?
			// TODO: add to avatar
		}
		[Init]
		public static void Init() =>
			Net.onShareInfo += OnShareInfo;
	}
}
