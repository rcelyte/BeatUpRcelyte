static partial class BeatUpClient {
	public delegate T CreateArrayCallback<T>(uint i);
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	public static T[] CreateArray<T>(uint count, CreateArrayCallback<T> cb) {
		T[] arr = new T[count];
		for(uint i = 0; i < count; ++i)
			arr[(int)i] = cb(i);
		return arr;
	}

	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	static uint UpperBound(uint value, uint limit) {
		if(value > limit)
			throw new System.ArgumentOutOfRangeException();
		return value;
	}

	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	static ulong RoundUpDivide(ulong n, uint d) =>
		(n + d - 1) / d;

	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	public static bool Or(this bool res, System.Action func) {
		if(!res)
			func();
		return res;
	}

	internal static BeatmapCharacteristicSO SerializedCharacteristic(string? name) =>
		Resolve<BeatmapCharacteristicCollection>()!.GetBeatmapCharacteristicBySerializedName(name) ??
		Resolve<BeatmapCharacteristicCollection>()!.GetBeatmapCharacteristicBySerializedName("Standard");
}
