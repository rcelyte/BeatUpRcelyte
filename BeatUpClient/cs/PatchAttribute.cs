static partial class BeatUpClient {
	public enum PatchType : byte {
		None,
		Prefix,
		Postfix,
		Transpiler,
		Finalizer,
	}

	public interface IPatch {
		System.Action Bind(System.Reflection.MethodInfo self);
	}

	[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
	public class Patch : System.Attribute, IPatch {
		[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
		public class Overload : Patch {
			public Overload(PatchType patchType, System.Type type, string fn, bool optional, params System.Type[] args) =>
				(this.patchType, this.optional, method) = (patchType, optional, fn == ".ctor" ? HarmonyLib.AccessTools.DeclaredConstructor(type, args) : HarmonyLib.AccessTools.DeclaredMethod(type, fn, args));
		}
		[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
		public class Generic : Patch {
			public Generic(PatchType patchType, System.Type type, string fn, params System.Type[] generics) =>
				(this.patchType, method) = (patchType, HarmonyLib.AccessTools.DeclaredMethod(type, fn, null, generics));
		}
		PatchType patchType = PatchType.None;
		bool optional = false;
		System.Reflection.MethodBase? method = null;
		Patch() {}
		public Patch(PatchType patchType, System.Type? type, string fn, bool optional = false) {
			if(type != null)
				(this.patchType, this.optional, method) = (patchType, optional, fn == ".ctor" ? (System.Reflection.MethodBase)type.GetConstructors()[0] : HarmonyLib.AccessTools.DeclaredMethod(type, fn));
		}
		public Patch(PatchType patchType, string assembly, string type, string fn, bool optional = false) :
			this(patchType, System.Reflection.Assembly.Load(assembly).GetType(type, false), fn, optional) {}
		public System.Action Bind(System.Reflection.MethodInfo self) {
			if(patchType == PatchType.None || (optional && method == null))
				return () => {};
			if(method == null)
				throw new System.ArgumentException($"Missing original method for `{self}`");
			return () => {
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
			};
		}
	}
}
