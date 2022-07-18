static partial class BeatUpClient {
	internal enum LoadState : byte {
		None,
		Failed,
		Exporting,
		Downloading,
		Loading,
		Done,
	}

	internal struct LoadProgress : IOneWaySerializable {
		static uint CurrentSequence = 0;
		public uint sequence;
		public LoadState state;
		public ushort progress;
		public float normalized => progress / 65535f;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)sequence);
			writer.Put((byte)state);
			writer.Put((ushort)progress);
		}
		public LoadProgress(LiteNetLib.Utils.NetDataReader reader) {
			sequence = reader.GetUInt();
			state = (LoadState)UpperBound(reader.GetByte(), (uint)LoadState.Done);
			progress = reader.GetUShort();
		}
		public LoadProgress(LoadState state, ushort progress) =>
			(sequence, this.state, this.progress) = (0, state, progress);
		public LoadProgress(EntitlementsStatus status) {
			(sequence, progress, state) = (0, 0, status switch {
				EntitlementsStatus.NotOwned => LoadState.Failed,
				EntitlementsStatus.NotDownloaded => LoadState.Downloading,
				EntitlementsStatus.Ok => LoadState.Done,
				_ => LoadState.None,
			});
		}
		public BeatUpPacket<LoadProgress> Wrap() =>
			BeatUpPacket<LoadProgress>.From(new LoadProgress {sequence = ++CurrentSequence, state = state, progress = progress});
	}
}
