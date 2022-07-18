static partial class BeatUpClient {
	static class ShareCache { // TODO: This system needs a LOT more work before release
		static ShareData share = ShareData.New(); // TODO: multiple shares + cache
		public enum Status {
			None,
			Shared,
			Cached,
		}
		public static Status CheckAvailability(string levelId) {
			if(levelId == share.info.id.name)
				return Status.Cached;
			if(ShareTracker.CanFetch(levelId))
				return Status.Shared;
			return Status.None;
		}
		// static void Discard(); // TODO: limit cache size
		public static bool TryGet(string levelId, out ShareData cached) {
			if(levelId == share.info.id.name) {
				cached = share;
				return true;
			}
			cached = new ShareData();
			return false;
		}
		public static ShareInfo Add(string levelId, Hash256 hash, System.ArraySegment<byte> data) {
			if(levelId != share.info.id.name) {
				share.info.id.usage = ShareableType.None;
				Log.Debug($"UNLIST `{share.info.id.name}`");
				Net.Send(share.info.Wrap());
				share = new ShareData(new ShareId {usage = ShareableType.BeatmapSet, mimeType = "application/json", name = levelId}, hash, data, 0);
			}
			Log.Debug($"SHARE `{levelId}`");
			return share.info;
		}
		public static async System.Threading.Tasks.Task<System.ArraySegment<byte>> Fetch(string levelId, System.Action<ushort>? progress = null) {
			if(TryGet(levelId, out ShareData cached))
				return cached.data;
			System.ArraySegment<byte> segment = new System.ArraySegment<byte>();
			(Hash256 hash, byte[]? data) = await ShareTracker.Fetch(levelId, progress);
			if(data?.Length > 0)
				segment = new System.ArraySegment<byte>(data);
			return segment;
		}
		static void OnDataFragmentRequest(DataFragmentRequest request, IConnectedPlayer player) {
			for(; request.count != 0; ++request.offset, --request.count) {
				System.ArraySegment<byte> fragment = new System.ArraySegment<byte>();
				uint rel = request.offset - share.info.offset;
				ulong byteOffset = (ulong)rel * LocalBlockSize; // `ulong` is required to represent underflow values
				if(byteOffset < (ulong)share.data.Count)
					fragment = new System.ArraySegment<byte>(share.data.Array, share.data.Offset + (int)byteOffset, System.Math.Min(LocalBlockSize, share.data.Count - (int)byteOffset));
				Net.SendUnreliableToPlayer(new DataFragment(request, fragment).Wrap(), player);
			}
		}
		// TODO: clear share on disconnect
		[Init]
		public static void Init() =>
			Net.onDataFragmentRequest += OnDataFragmentRequest;
	}
}
