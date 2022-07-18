static partial class BeatUpClient {
	[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential, Size = 32)]
	internal struct Hash256 : System.IEquatable<Hash256> {
		public const int Size = 32;
		ulong _0, _1, _2, _3;
		public bool Equals(Hash256 other) =>
			(_0, _1, _2, _3) == (other._0, other._1, other._2, other._3);
		public unsafe System.Span<byte> Span() {
			fixed(Hash256 *ptr = &this) {
				return System.SpanExtensions.AsBytes(new System.Span<Hash256>(ptr, 1));
			}
		}
		public Hash256(System.ReadOnlySpan<byte> from) : this() =>
			from.CopyTo(Span());
	}

	internal interface IOneWaySerializable {
		void Serialize(LiteNetLib.Utils.NetDataWriter writer);
	}

	internal abstract class BeatUpPacket : LiteNetLib.Utils.INetSerializable {
		public abstract void Serialize(LiteNetLib.Utils.NetDataWriter writer);
		void LiteNetLib.Utils.INetSerializable.Deserialize(LiteNetLib.Utils.NetDataReader reader) {} // BeatUpPacket deserialization bypasses this method
	}

	internal class BeatUpPacket<T> : BeatUpPacket where T : struct, IOneWaySerializable {
		static readonly Net.MessageType messageType = (Net.MessageType)System.Enum.Parse(typeof(Net.MessageType), typeof(T).Name);
		[System.ThreadStatic]
		static BeatUpPacket<T> pooled = new BeatUpPacket<T>();
		[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
		public static T Value() => pooled.value;
		[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
		public static BeatUpPacket<T> From(T data) {
			pooled.value = data;
			return pooled;
		}
		T value;
		public override void Serialize(LiteNetLib.Utils.NetDataWriter writer) {
			writer.Put((byte)messageType);
			value.Serialize(writer);
		}
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
