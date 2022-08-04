#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-types/shared/delegate.hpp"
#include "custom-types/shared/macros.hpp"
#include <System/Action.hpp>
#include <System/Action_1.hpp>
#include <System/Action_2.hpp>
#include <System/Action_3.hpp>
#include <System/Action_4.hpp>
#include <UnityEngine/Events/UnityAction.hpp>
#include <UnityEngine/Events/UnityAction_1.hpp>
#include <UnityEngine/Events/UnityAction_2.hpp>
#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

extern Logger *logger;

typedef bool (*PatchFunc)(); // TODO: think of a better name for this

template<PatchFunc *list> static bool InjectCallback(PatchFunc in) {
	static PatchFunc next = *list, body = in;
	*list = []() {
		return next() || body();
	};
	return false;
};

#define INJECT_CALLBACK(list, ...) static bool CONCAT(_void, __LINE__) = InjectCallback<&(list)>([]() __VA_ARGS__);

template<auto mPtr, typename funcType>
struct BSCHook;

template<auto mPtr, typename retval, typename ...Args>
struct BSCHook<mPtr, retval(Args...)> {
	using funcType = retval (*)(Args...);
	static_assert(std::is_same_v<funcType, typename ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!");
	static const MethodInfo *getInfo() {return ::il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get();}
	static funcType *trampoline() {return &base;}
	static inline funcType base = nullptr;
	static funcType hook() {return &::Hooking::HookCatchWrapper<&body, funcType>::wrapper;}
	static retval body(Args...);
};

#define BSC_MAKE_HOOK_INTERNAL(mPtr, mName, retval, ...) \
	struct CONCAT(Hook_, __LINE__) : BSCHook<mPtr, retval(__VA_ARGS__)> { \
		constexpr static const char *name() {return mName;} \
	}; \
	INJECT_CALLBACK(ApplyPatches, { \
		Hooking::InstallHook<CONCAT(Hook_, __LINE__)>(*logger); \
		return false; \
	}) \
	template<> retval ::BSCHook<mPtr, retval(__VA_ARGS__)>::body(__VA_ARGS__)

#define BSC_MAKE_HOOK_MATCH(mPtr, retval, ...) \
	BSC_MAKE_HOOK_INTERNAL(&mPtr, #mPtr, retval, __VA_ARGS__)
#define BSC_MAKE_HOOK_OVERLOAD(mClass, mPtr, retval, thisarg, ...) \
	BSC_MAKE_HOOK_INTERNAL(static_cast<retval (mClass::*)(__VA_ARGS__)>(&mClass::mPtr), #mClass "::" #mPtr, retval, thisarg, __VA_ARGS__)

#define BSC_ICALL(name, iname, retval, ...) \
	static retval (*name)(__VA_ARGS__) = NULL; \
	INJECT_CALLBACK(ApplyPatches, { \
		name = (retval (*)(__VA_ARGS__))il2cpp_functions::resolve_icall(iname); \
		if(!name) \
			logger->critical("Failed to resolve `" iname "`"); \
		return !name; \
	})

namespace BeatUpClient {
	extern PatchFunc ApplyPatches;
}

static inline auto MakeDelegate_Action(std::function<void()> fn) {
	return custom_types::MakeDelegate<System::Action*>(fn);
}
template<class Arg0> static inline auto MakeDelegate_Action(std::function<void(Arg0)> fn) {
	return custom_types::MakeDelegate<System::Action_1<Arg0>*>(fn);
}
template<class Arg0, class Arg1> static inline auto MakeDelegate_Action(std::function<void(Arg0, Arg1)> fn) {
	return custom_types::MakeDelegate<System::Action_2<Arg0, Arg1>*>(fn);
}
template<class Arg0, class Arg1, class Arg2> static inline auto MakeDelegate_Action(std::function<void(Arg0, Arg1, Arg2)> fn) {
	return custom_types::MakeDelegate<System::Action_3<Arg0, Arg1, Arg2>*>(fn);
}
template<class Arg0, class Arg1, class Arg2, class Arg3> static inline auto MakeDelegate_Action(std::function<void(Arg0, Arg1, Arg2, Arg3)> fn) {
	return custom_types::MakeDelegate<System::Action_4<Arg0, Arg1, Arg2, Arg3>*>(fn);
}
static inline auto MakeDelegate_Unity(std::function<void()> fn) {
	return custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(fn);
}
template<class Arg0> static inline auto MakeDelegate_Unity(std::function<void(Arg0)> fn) {
	return custom_types::MakeDelegate<UnityEngine::Events::UnityAction_1<Arg0>*>(fn);
}
template<class Arg0, class Arg1> static inline auto MakeDelegate_Unity(std::function<void(Arg0, Arg1)> fn) {
	return custom_types::MakeDelegate<UnityEngine::Events::UnityAction_2<Arg0, Arg1>*>(fn);
}

template<class L> static inline auto Delegate_Action(L &&ptr) {
	return MakeDelegate_Action(std::function(ptr));
}

template<class L> static inline auto Delegate_Unity(L &&ptr) {
	return MakeDelegate_Unity(std::function(ptr));
}
