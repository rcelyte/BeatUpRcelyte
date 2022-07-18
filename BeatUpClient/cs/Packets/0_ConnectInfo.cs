static partial class BeatUpClient {
	internal struct ConnectInfo : IOneWaySerializable {
		public uint protocolId;
		public ushort blockSize;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)protocolId);
			writer.Put((ushort)blockSize);
		}
		public ConnectInfo(ushort blockSize) =>
			(this.protocolId, this.blockSize) = (LocalProtocolId, blockSize);
		public ConnectInfo(LiteNetLib.Utils.NetDataReader reader) : this() {
			protocolId = reader.GetUInt();
			if(protocolId != LocalProtocolId)
				return;
			blockSize = reader.GetUShort();
		}
		public BeatUpPacket<ConnectInfo> Wrap() =>
			BeatUpPacket<ConnectInfo>.From(this);
	}
}
