#if MPCORE_SUPPORT
using static System.Linq.Enumerable;

static partial class BeatUpClient_MpCore {
	static class DiJack {
		static readonly System.Collections.Generic.Dictionary<System.Type, System.Type> InjectMap = new System.Collections.Generic.Dictionary<System.Type, System.Type>();
		static void ConcreteBinderNonGeneric_To(Zenject.ConcreteBinderNonGeneric __instance, ref System.Collections.Generic.IEnumerable<System.Type> concreteTypes) {
			System.Type[] newTypes = concreteTypes.ToArray();
			uint i = 0;
			foreach(System.Type type in concreteTypes) {
				if(InjectMap.TryGetValue(type, out System.Type? inject)) {
					BeatUpClient.Log.Debug($"Replacing {type} with {inject}");
					newTypes[i] = inject;
					__instance.BindInfo.ContractTypes.Add(inject);
				}
				++i;
			}
			concreteTypes = newTypes;
		}

		public static void Register(System.Type type) =>
			InjectMap.Add(type.BaseType, type);

		public static void UnregisterAll() =>
			InjectMap.Clear();

		public static void Patch() {
			System.Reflection.MethodInfo original = typeof(Zenject.ConcreteBinderNonGeneric).GetMethod(nameof(Zenject.ConcreteBinderNonGeneric.To), new[] {typeof(System.Collections.Generic.IEnumerable<System.Type>)});
			System.Reflection.MethodInfo prefix = typeof(DiJack).GetMethod(nameof(ConcreteBinderNonGeneric_To), System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic);
			BeatUpClient.harmony.Patch(original, prefix: new HarmonyLib.HarmonyMethod(prefix));
		}
	}
}
#endif
