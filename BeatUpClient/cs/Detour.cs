using static System.Linq.Enumerable;

static partial class BeatUpClient {
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base() => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T>(T obj) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2>(T1 arg1, T2 arg2) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2, T3>(T1 arg1, T2 arg2, T3 arg3) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2, T3, T4>(T1 arg1, T2 arg2, T3 arg3, T4 arg4) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2, T3, T4, T5>(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2, T3, T4, T5, T6>(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2, T3, T4, T5, T6, T7>(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
	public static object Base<T1, T2, T3, T4, T5, T6, T7, T8>(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8) => null!;
	[System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
	public unsafe static System.IntPtr PassRef<T>(ref T value) => new(System.Runtime.CompilerServices.Unsafe.AsPointer(ref value));
	[System.AttributeUsage(System.AttributeTargets.Method)]
	public class DetourAttribute : System.Attribute, IPatch { // TODO: make sure all of this works as expected
		System.Type type;
		string? methodName;
		public DetourAttribute(System.Type type, string? methodName = null) =>
			(this.type, this.methodName) = (type, methodName);
		static System.Type[] ParameterTypes(System.Reflection.ParameterInfo[] parameters, int start) {
			System.Type[] types = new System.Type[System.Math.Max(parameters.Length - start, 0)];
			for(int i = start; i < parameters.Length; ++i)
				types[i - start] = parameters[i].ParameterType;
			return types;
		}
		static readonly System.Reflection.BindingFlags declaredFlags = System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.GetField | System.Reflection.BindingFlags.SetField | System.Reflection.BindingFlags.GetProperty | System.Reflection.BindingFlags.SetProperty | System.Reflection.BindingFlags.DeclaredOnly;
		public System.Action Bind(System.Reflection.MethodInfo self) {
			System.Reflection.ParameterInfo[] parameters = self.GetParameters();
			System.Reflection.MethodBase? original = null;
			if(methodName == null) {
				if(parameters.Length >= 1 && parameters[0].ParameterType == type)
					original = type.GetConstructor(declaredFlags | System.Reflection.BindingFlags.Instance, null, ParameterTypes(parameters, 1), new System.Reflection.ParameterModifier[0]);
			} else {
				original = type.GetMethod(methodName, declaredFlags | System.Reflection.BindingFlags.Static, null, ParameterTypes(parameters, 0), new System.Reflection.ParameterModifier[0]);
				if(parameters.Length >= 1 && parameters[0].ParameterType == type)
					original ??= type.GetMethod(methodName, declaredFlags | System.Reflection.BindingFlags.Instance, null, ParameterTypes(parameters, 1), new System.Reflection.ParameterModifier[0]);
			}
			if(original == null)
				throw new System.ArgumentException($"Missing original method for `{self}`");
			MonoMod.RuntimeDetour.IDetour hook = new MonoMod.RuntimeDetour.Detour(original, self, new MonoMod.RuntimeDetour.DetourConfig {ManualApply = true});
			onUnpatch += hook.Dispose;
			System.Reflection.MethodBase trampoline = hook.GenerateTrampoline();
			MonoMod.RuntimeDetour.IDetour ilhook = new MonoMod.RuntimeDetour.ILHook(self, il => {
				bool fix = false;
				Mono.Cecil.Cil.Code fixOp = ((original as System.Reflection.MethodInfo)?.ReturnType != typeof(void)) ? Mono.Cecil.Cil.Code.Unbox_Any : Mono.Cecil.Cil.Code.Pop;
				foreach(Mono.Cecil.Cil.Instruction ins in il.Body.Instructions) {
					if(ins.OpCode.Code == Mono.Cecil.Cil.Code.Call && ins.Operand is Mono.Cecil.MethodReference method && method.DeclaringType?.FullName == "BeatUpClient" && method.Name == "Base") {
						ins.Operand = il.Import(trampoline);
						fix = true;
					} else if(fix) {
						if(ins.OpCode.Code == fixOp)
							ins.OpCode = Mono.Cecil.Cil.OpCodes.Nop;
						fix = false;
					}
				}
			}, new MonoMod.RuntimeDetour.ILHookConfig {ManualApply = true});
			onUnpatch += ilhook.Dispose;
			return () => {
				ilhook.Apply();
				hook.Apply();
			};
		}
		static System.Action? onUnpatch = null;
		public static void UnpatchAll() {
			onUnpatch?.Invoke();
			onUnpatch = null;
		}
	}
}
