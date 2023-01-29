static partial class BeatUpClient {
	internal static class Injected<T> where T : class {
		public static T? Instance = default(T);
		public static T? Resolve(Zenject.DiContainer container) =>
			(Instance = container.TryResolve<T>());
		public static T? Resolve<U>(Zenject.DiContainer container) where U : class =>
			(Instance = container.TryResolve<U>() as T);
	}
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	internal static T? Resolve<T>() where T : class =>
		Injected<T>.Instance;
}
