static partial class BeatUpClient {
	internal struct DataFragment : IOneWaySerializable {
		public readonly uint offset;
		public readonly System.ArraySegment<byte> data;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)offset);
			writer.Put(data.Array, data.Offset, data.Count);
		}
		public DataFragment(LiteNetLib.Utils.NetDataReader reader, int length) {
			offset = reader.GetUInt();
			length -= sizeof(uint);
			data = new System.ArraySegment<byte>(reader.RawData, reader.Position, length);
			reader.SkipBytes(length);
		}
		public DataFragment(DataFragmentRequest request, System.ArraySegment<byte> data) =>
			(offset, this.data) = (request.offset, data);
		public BeatUpPacket<DataFragment> Wrap() =>
			BeatUpPacket<DataFragment>.From(this);
	}
}
