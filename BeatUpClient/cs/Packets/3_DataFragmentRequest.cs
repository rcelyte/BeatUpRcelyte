static partial class BeatUpClient {
	internal struct DataFragmentRequest : IOneWaySerializable {
		public uint offset;
		public byte count;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)offset);
			writer.Put((byte)count);
		}
		public DataFragmentRequest(LiteNetLib.Utils.NetDataReader reader) {
			offset = reader.GetUInt();
			count = reader.GetByte();
		}
		public DataFragmentRequest(uint offset) =>
			(this.offset, this.count) = (offset, 1);
		public BeatUpPacket<DataFragmentRequest> Wrap() =>
			BeatUpPacket<DataFragmentRequest>.From(this);
	}
}
