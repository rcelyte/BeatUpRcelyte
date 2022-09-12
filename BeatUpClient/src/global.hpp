#pragma once
#include "setup.hpp"
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
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Transform.hpp>
#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

typedef bool (*PatchFunc)(); // TODO: think of a better name for this

#define INJECT_CALLBACK(list, ...) \
	static void __attribute__((constructor)) CONCAT(_inject_ctor, __LINE__)() { \
		static PatchFunc next = NULL; \
		next = (list); \
		(list) = []() -> bool { \
			if(next()) \
				return true; \
			__VA_ARGS__; \
			return false; \
		}; \
	}

template<auto mPtr, const char *mName, typename funcType>
struct BSCHook;

template<auto mPtr, const char *mName, typename retval, typename ...Args>
struct BSCHook<mPtr, mName, retval(Args...)> {
	using funcType = retval (*)(Args...);
	static_assert(std::is_same_v<funcType, typename ::Hooking::InternalMethodCheck<decltype(mPtr)>::funcType>, "Hook method signature does not match!");
	static const MethodInfo *getInfo() {return ::il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get();}
	static funcType *trampoline() {return &base;}
	static inline funcType base = nullptr;
	static funcType hook() {return &::Hooking::HookCatchWrapper<&body, funcType>::wrapper;}
	static retval body(Args...);
	constexpr static const char *name() {return mName;}
};

#define BSC_MAKE_HOOK_INTERNAL(mPtr, mName, retval, ...) \
	static const char CONCAT(_name, __LINE__)[] = mName; \
	INJECT_CALLBACK(ApplyPatches, { \
		Hooking::InstallHook<BSCHook<mPtr, CONCAT(_name, __LINE__), retval(__VA_ARGS__)>>(*BeatUpClient::logger); \
		return false; \
	}) \
	template<> retval ::BSCHook<mPtr, CONCAT(_name, __LINE__), retval(__VA_ARGS__)>::body(__VA_ARGS__)

#define BSC_MAKE_HOOK_MATCH(mPtr, retval, ...) \
	BSC_MAKE_HOOK_INTERNAL(&mPtr, #mPtr, retval, __VA_ARGS__)
#define BSC_MAKE_HOOK_OVERLOAD(mClass, mPtr, retval, thisarg, ...) \
	BSC_MAKE_HOOK_INTERNAL(static_cast<retval (mClass::*)(__VA_ARGS__)>(&mClass::mPtr), #mClass "::" #mPtr, retval, thisarg, __VA_ARGS__)

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

#ifndef BSC_ICALL
#define BSC_ICALL(name, iname, retval, ...) extern retval (*name)(__VA_ARGS__);
#endif

BSC_ICALL(GameObject_SetActive, "UnityEngine.GameObject::SetActive", void, UnityEngine::GameObject *self, bool value)
BSC_ICALL(Object_Destroy, "UnityEngine.Object::Destroy", void, UnityEngine::Object *obj, float t)
BSC_ICALL(Object_SetName, "UnityEngine.Object::SetName", void, UnityEngine::Object *obj, Il2CppString *name)

BSC_ICALL(Object_Internal_CloneSingleWithParent, "UnityEngine.Object::Internal_CloneSingleWithParent", UnityEngine::Object*, UnityEngine::Object *data, UnityEngine::Transform *parent, bool worldPositionStays)
static inline UnityEngine::GameObject *Instantiate(UnityEngine::GameObject *object, UnityEngine::Transform *parent) {
	return il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingleWithParent(object, parent, false));
}

BSC_ICALL(GameObject_GetComponent, "UnityEngine.GameObject::GetComponent", UnityEngine::Component*, UnityEngine::GameObject *self, System::Type *type)
template<typename T> static inline T *GetComponent(UnityEngine::GameObject *gameObject) {
	return il2cpp_utils::cast<T>(GameObject_GetComponent(gameObject, csTypeOf(T*)));
}

BSC_ICALL(Resources_FindObjectsOfTypeAll, "UnityEngine.Resources::FindObjectsOfTypeAll", Array<UnityEngine::Object*>*, System::Type *type);
template<typename T> static inline ArrayW<T*> FindResourcesOfType() {
	return Resources_FindObjectsOfTypeAll(csTypeOf(T*));
}

template<typename T> static inline std::optional<T*> FindResourceOfType() {
	ArrayW<T*> array = FindResourcesOfType<T>();
	return (array.Length() < 1) ? std::nullopt : std::optional<T*>(array[0]);
}

#undef BSC_ICALL
