#pragma once
#include <Zenject/DiContainer.hpp>

namespace BeatUpClient {
	template<class T> struct Injected {
		static inline T *Instance = NULL;
		template<class U> static inline T *Resolve(Zenject::DiContainer *container) {
			Instance = il2cpp_utils::try_cast<T>(container->TryResolve(csTypeOf(U*))).value_or(nullptr);
			return Instance;
		}
	};
	template<class T> static inline T *Resolve() {
		return Injected<T>::Instance;
	}
}
