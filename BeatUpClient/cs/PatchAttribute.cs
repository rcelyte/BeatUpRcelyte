static partial class BeatUpClient {
	public enum PatchType : byte {
		Prefix,
		Postfix,
		Transpiler,
	}

	[System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = true)]
	public class Patch : System.Attribute {
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
		internal void Apply(System.Reflection.MethodInfo self) {
			if(method == null)
				throw new System.ArgumentException($"Missing original method for `{self}`");
			// Log.Debug($"Patching {method.DeclaringType.FullName} : {method}");
			HarmonyLib.HarmonyMethod hm = new HarmonyLib.HarmonyMethod(self);
			var processor = harmony.CreateProcessor(method);
			switch(patchType) {
				case PatchType.Prefix: processor.AddPrefix(hm); break;
				case PatchType.Postfix: processor.AddPostfix(hm); break;
				case PatchType.Transpiler: processor.AddTranspiler(hm); break;
			}
			processor.Patch();
		}
		/*public static void ApplyAllFor(System.Type type) {
			foreach(System.Reflection.MethodInfo method in type.GetMethods())
				foreach(System.Attribute attrib in method.GetCustomAttributes(false))
					if(attrib is Patch patch)
						patch.Apply(method);
		}*/
	}
}
