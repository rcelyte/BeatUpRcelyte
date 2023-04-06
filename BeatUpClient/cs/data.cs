static partial class BeatUpClient {
	[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Explicit, Size = 32)]
	internal struct Hash256 {
		public const int Size = 32;
		[System.Runtime.InteropServices.FieldOffset(0)] (ulong, ulong, ulong, ulong) raw;
		[System.Runtime.InteropServices.FieldOffset(0)] byte first;
		public bool Equals(Hash256 other) =>
			raw == other.raw;
		public Hash256(byte[] from, int offset = 0) : this() {
			if(offset > from.Length || offset + Size > from.Length)
				throw new System.ArgumentOutOfRangeException("offset");
			System.Runtime.CompilerServices.Unsafe.CopyBlock(ref first, ref from[offset], Size);
		}
		public static async System.Threading.Tasks.Task<Hash256> ComputeAsync(System.ArraySegment<byte> buffer, System.Action<ulong>? onProgress = null) {
			long progress = 0;
			void ProgressTick(float deltaTime) =>
				onProgress((ulong)System.Threading.Interlocked.Read(ref progress));
			try {
				if(onProgress != null)
					UnityEngine.ResourceManagement.Util.ComponentSingleton<MonoBehaviourCallbackHooks>.Instance.OnUpdateDelegate += ProgressTick;
				return await System.Threading.Tasks.Task.Run(() => { // TODO: `CancellationToken`?
					using System.Security.Cryptography.SHA256 sha256 = System.Security.Cryptography.SHA256.Create();
					// return new Hash256(sha256.ComputeHash(buffer.Array, buffer.Offset, buffer.Count));
					int offset = buffer.Offset, end = buffer.Offset + buffer.Count;
					while(end - offset > 65536) {
						sha256.HashCore(buffer.Array, offset, 65536);
						offset += 65536;
						System.Threading.Interlocked.Exchange(ref progress, offset);
					}
					sha256.HashCore(buffer.Array, offset, end - offset);
					Hash256 hash = new(sha256.HashFinal());
					System.Threading.Interlocked.Exchange(ref progress, end);
					sha256.Initialize();
					return hash;
				});
			} finally {
				if(onProgress != null)
					UnityEngine.ResourceManagement.Util.ComponentSingleton<MonoBehaviourCallbackHooks>.Instance.OnUpdateDelegate -= ProgressTick;
			}
		}
		public void CopyTo(byte[] to, int offset = 0) {
			if(offset > to.Length || offset + Size > to.Length)
				throw new System.ArgumentException("Destination is too short.");
			System.Runtime.CompilerServices.Unsafe.CopyBlock(ref to[offset], ref first, Size);
		}
	}

	internal interface IOneWaySerializable {
		void Serialize(LiteNetLib.Utils.NetDataWriter writer);
	}

	internal abstract class BeatUpPacket : LiteNetLib.Utils.INetSerializable {
		public abstract void Serialize(LiteNetLib.Utils.NetDataWriter writer);
		void LiteNetLib.Utils.INetSerializable.Deserialize(LiteNetLib.Utils.NetDataReader reader) {} // BeatUpPacket deserialization bypasses this method
	}

	internal class BeatUpPacket<T> : BeatUpPacket, IPoolablePacket where T : struct, IOneWaySerializable {
		static readonly Net.MessageType messageType = (Net.MessageType)System.Enum.Parse(typeof(Net.MessageType), typeof(T).Name);
		[System.ThreadStatic] static BeatUpPacket<T>? pool = null;
		T inner;
		public static BeatUpPacket<T> From(T data) {
			BeatUpPacket<T> packet = pool ?? new();
			pool = null;
			packet.inner = data;
			return packet;
		}
		public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((byte)messageType);
			inner.Serialize(writer);
		}
		void IPoolablePacket.Release() =>
			pool = this;
	}

	struct Property<T> : IValue<T> {
		System.Func<T> get;
		System.Action<T> set;
		public T value {get => get(); set => set(value);}
		public Property(object instance, string propName) {
			System.Reflection.PropertyInfo member = instance.GetType().GetProperty(propName);
			get = (System.Func<T>)System.Delegate.CreateDelegate(typeof(System.Func<T>), instance, member.GetGetMethod());
			set = (System.Action<T>)System.Delegate.CreateDelegate(typeof(System.Action<T>), instance, member.GetSetMethod());
		}
	}
}
