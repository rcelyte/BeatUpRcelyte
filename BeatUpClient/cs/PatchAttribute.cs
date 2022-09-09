static partial class BeatUpClient {
	public enum PatchType : byte {
		Prefix,
		Postfix,
		Transpiler,
		Finalizer,
	}

	public struct BoundPatch { // TODO: move detour logic into here once Harmony is gone
		/*System.Reflection.MethodBase original;
		System.Reflection.MethodInfo self;*/
		public System.Action Apply;
		public BoundPatch(System.Action Apply) =>
			this.Apply = Apply;
	}

	public interface IPatch {
		BoundPatch Bind(System.Reflection.MethodInfo self);
	}

	[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
	public class Patch : System.Attribute, IPatch {
		[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
		public class Overload : Patch {
			public Overload(PatchType patchType, System.Type type, string fn, params System.Type[] args) =>
				(this.patchType, method) = (patchType, HarmonyLib.AccessTools.DeclaredMethod(type, fn, args));
		}
		[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
		public class Generic : Patch {
			public Generic(PatchType patchType, System.Type type, string fn, params System.Type[] generics) =>
				(this.patchType, method) = (patchType, HarmonyLib.AccessTools.DeclaredMethod(type, fn, null, generics));
		}
		System.Reflection.MethodBase? method;
		PatchType patchType;
		Patch() {}
		public Patch(PatchType patchType, System.Type type, string fn) =>
			(this.patchType, method) = (patchType, fn == ".ctor" ? (System.Reflection.MethodBase)type.GetConstructors()[0] : HarmonyLib.AccessTools.DeclaredMethod(type, fn));
		public BoundPatch Bind(System.Reflection.MethodInfo self) {
			if(method == null)
				throw new System.ArgumentException($"Missing original method for `{self}`");
			return new BoundPatch(() => {
				// Log.Debug($"Patching {method.DeclaringType.FullName} : {method}");
				HarmonyLib.HarmonyMethod hm = new HarmonyLib.HarmonyMethod(self);
				var processor = harmony.CreateProcessor(method);
				switch(patchType) {
					case PatchType.Prefix: processor.AddPrefix(hm); break;
					case PatchType.Postfix: processor.AddPostfix(hm); break;
					case PatchType.Transpiler: processor.AddTranspiler(hm); break;
					case PatchType.Finalizer: processor.AddFinalizer(hm); break;
				}
				processor.Patch();
			});
		}
	}
}
