static partial class BeatUpClient {
	static class ShareProvider { // TODO: Simplify protocol & remove block size negotiation
		static ShareData share = ShareData.New();
		public static ShareData GetCurrent() => share;
		public static ShareInfo Set(string levelId, Hash256 hash, System.ArraySegment<byte> data) {
			share.info.id.usage = ShareableType.None;
			Log.Debug($"UNLIST `{share.info.id.name}`");
			Net.Send(share.info.Wrap());
			share = new ShareData(new ShareId {usage = ShareableType.BeatmapSet, mimeType = "application/json", name = levelId}, hash, data, 0);
			Log.Debug($"SHARE `{levelId}`");
			return share.info;
		}
		public static void OnDataFragmentRequest(DataFragmentRequest request, IConnectedPlayer player) {
			for(; request.count != 0; ++request.offset, --request.count) {
				System.ArraySegment<byte> fragment = new System.ArraySegment<byte>();
				uint rel = request.offset - share.info.offset;
				ulong byteOffset = (ulong)rel * LocalBlockSize; // `ulong` is required to represent underflow values
				if(byteOffset < (ulong)share.data.Count)
					fragment = new System.ArraySegment<byte>(share.data.Array, share.data.Offset + (int)byteOffset, System.Math.Min(LocalBlockSize, share.data.Count - (int)byteOffset));
				Net.SendUnreliableToPlayer(new DataFragment(request, fragment).Wrap(), player);
			}
		} // TODO: clear share on disconnect
	}
}
