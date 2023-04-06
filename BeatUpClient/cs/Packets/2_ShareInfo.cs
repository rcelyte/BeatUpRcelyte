static partial class BeatUpClient {
	internal enum ShareableType : ushort {
		None,
		Generic,
		BeatmapAudio, // Expect MIME "audio/ogg" or "audio/x-wav"
		BeatmapSet, // Expect MIME "application/json"
		Avatar, // Expect MIME "model/gltf-binary"
		// TODO: saber, bloq, etc
		// AssetBundle downloads are not secure, and should not be implemented
	}

	internal struct ShareMeta : System.IEquatable<ShareMeta> {
		public readonly ulong byteLength;
		public readonly Hash256 hash;
		public bool Equals(ShareMeta other) =>
			byteLength == other.byteLength && hash.Equals(other.hash);
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.PutVarULong(byteLength);
			writer.ResizeIfNeed(writer._position + Hash256.Size);
			hash.CopyTo(writer.Data, writer._position);
			writer._position += Hash256.Size;
		}
		public ShareMeta(LiteNetLib.Utils.NetDataReader reader) : this() {
			byteLength = reader.GetVarULong();
			hash = new(reader.RawData, reader.Position);
			reader.SkipBytes(Hash256.Size);
		}
		public ShareMeta(ulong byteLength, Hash256 hash = default) =>
			(this.byteLength, this.hash) = (byteLength, hash);
	}

	internal struct ShareId {
		public ShareableType usage;
		public string mimeType, name;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((ushort)usage);
			if(usage == ShareableType.None)
				return;
			writer.Put((string)mimeType);
			writer.Put((string)name);
		}
		public ShareId(LiteNetLib.Utils.NetDataReader reader) : this() {
			usage = (ShareableType)reader.GetUShort();
			(mimeType, name) = (usage != ShareableType.None) ? (reader.GetString(), reader.GetString()) : (string.Empty, string.Empty);
		}
	}

	internal struct ShareInfo : IOneWaySerializable {
		public uint offset;
		public ushort blockSize;
		public ShareMeta meta;
		public ShareId id;
		public void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((uint)offset);
			writer.Put((ushort)blockSize);
			meta.Serialize(writer);
			id.Serialize(writer);
		}
		public ShareInfo(LiteNetLib.Utils.NetDataReader reader) {
			offset = reader.GetUInt();
			blockSize = reader.GetUShort();
			meta = new ShareMeta(reader);
			id = new ShareId(reader);
		}
		public ShareInfo(uint offset, ShareMeta meta, ShareId id) =>
			(this.offset, blockSize, this.meta, this.id) = (offset, LocalBlockSize, meta, id);
		public BeatUpPacket<ShareInfo> Wrap() =>
			BeatUpPacket<ShareInfo>.From(this);
	}

	struct ShareData {
		public ShareInfo info;
		public System.ArraySegment<byte> data;
		public ShareData(ShareId id, Hash256 hash, System.ArraySegment<byte> data, uint offset) =>
			(this.info, this.data) = (new ShareInfo(offset, new ShareMeta((ulong)data.Count, hash), id), data);
		public static ShareData New() =>
			new ShareData(new ShareId {mimeType = string.Empty, name = string.Empty}, default, new System.ArraySegment<byte>(), 0);
	}
}
