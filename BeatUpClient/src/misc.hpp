#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "custom-types/shared/macros.hpp"
#define lengthof(x) (sizeof(x)/sizeof(*(x)))
#define endof(x) (&(x)[lengthof(x)])

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

// #define BSC_INIT static void __attribute__((constructor)) CONCAT(_init_, __LINE__)()
#define BSC_INIT struct CONCAT(Init_, __LINE__) {CONCAT(Init_, __LINE__)();} static CONCAT(_init_, __LINE__); CONCAT(Init_, __LINE__)::CONCAT(Init_, __LINE__)()

#define BSC_ICALL(name, iname, retval, ...) \
static retval (*name)(__VA_ARGS__) = NULL; \
BSC_INIT { \
	static bool (*next)(Logger*) = ResolveICalls; \
	ResolveICalls = [](Logger *logger) { \
		bool res = next(logger); \
		name = (retval (*)(__VA_ARGS__))il2cpp_functions::resolve_icall(iname); \
		if(name) \
			return res; \
		logger->critical("Failed to resolve `" iname "`"); \
		return true; \
	}; \
}

#define BSC_MAKE_HOOK(mName, mInfo, retval, ...) \
struct CONCAT(Hook_, __LINE__) { \
	using funcType = retval (*)(__VA_ARGS__); \
	constexpr static const char* name() {return mName;} \
	static const MethodInfo* getInfo() mInfo \
	static funcType* trampoline() { return &base; } \
	static inline retval (*base)(__VA_ARGS__) = nullptr; \
	static funcType hook() { return &::Hooking::HookCatchWrapper<&body, funcType>::wrapper; } \
	static retval body(__VA_ARGS__); \
}; \
BSC_INIT { \
	static void (*next)(Logger*) = InstallHooks; \
	InstallHooks = [](Logger *logger) { \
		next(logger); \
		Hooking::InstallHook<CONCAT(Hook_, __LINE__)>(*logger); \
	}; \
} \
retval CONCAT(Hook_, __LINE__)::body(__VA_ARGS__)

#define BSC_MAKE_HOOK_MATCH(mPtr, ...) \
	BSC_MAKE_HOOK(#mPtr, { \
		static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(&mPtr)>::funcType>, "Hook method signature does not match!"); \
		return ::il2cpp_utils::il2cpp_type_check::MetadataGetter<&mPtr>::get(); \
	}, __VA_ARGS__)

#define BSC_MAKE_HOOK_FIND_CLASS_INSTANCE(namespace, class, name, ...) \
	BSC_MAKE_HOOK(class "." name, { \
		return ::il2cpp_utils::MethodTypeCheck<typename ::il2cpp_utils::InstanceMethodConverter<funcType>::fType>::find(namespace, class, name); \
	}, __VA_ARGS__)

#define DEFINE_CLASS_CODEGEN(namespaze, name, ...) DECLARE_CLASS_CODEGEN(namespaze, name, __VA_ARGS__) DEFINE_TYPE(namespaze, name)
#define DEFINE_CLASS_CODEGEN_INTERFACES(namespaze, name, ...) DECLARE_CLASS_CODEGEN_INTERFACES(namespaze, name, __VA_ARGS__) DEFINE_TYPE(namespaze, name)

#define DEFINE_CTOR(name, ...) \
public: \
void name __VA_ARGS__ \
template<::il2cpp_utils::CreationType creationType = ::il2cpp_utils::CreationType::Temporary, class... TArgs> \
static ___TargetType* New_ctor(TArgs&&... args) { \
    static_assert(::custom_types::Decomposer<decltype(&___TargetType::name)>::convertible<TArgs...>(), "Arguments provided to New_ctor must be convertible to the constructor!"); \
    ___TargetType* obj; \
    if constexpr (creationType == ::il2cpp_utils::CreationType::Temporary) { \
        obj = reinterpret_cast<___TargetType*>(::il2cpp_functions::object_new(___TypeRegistration::klass_ptr)); \
    } else { \
        obj = reinterpret_cast<___TargetType*>(::il2cpp_utils::createManual(___TypeRegistration::klass_ptr)); \
    } \
    obj->name(std::forward<TArgs>(args)...); \
    return obj; \
} \
___CREATE_INSTANCE_METHOD(name, ".ctor", METHOD_ATTRIBUTE_PUBLIC | METHOD_ATTRIBUTE_HIDE_BY_SIG | METHOD_ATTRIBUTE_SPECIAL_NAME | METHOD_ATTRIBUTE_RT_SPECIAL_NAME, nullptr)

#define DEFINE_OVERRIDE_METHOD(ret, name, overridingMethodInfo, ...) \
public: \
ret name __VA_ARGS__ \
___CREATE_INSTANCE_METHOD(name, #name, (overridingMethodInfo->flags & ~METHOD_ATTRIBUTE_ABSTRACT) | METHOD_ATTRIBUTE_PUBLIC | METHOD_ATTRIBUTE_HIDE_BY_SIG, overridingMethodInfo)

constexpr bool _string_cmp(const char *a, const char *b) {
	return std::string_view(a) == b;
}
#define SSTRINGIFY(...) #__VA_ARGS__
#define STRINGIFY(x) SSTRINGIFY(x)
#define STRING_ASSERT(c0, c1) static_assert(_string_cmp(c0, c1)) // Make sure to update macros whenever the original ones change
STRING_ASSERT(STRINGIFY(MAKE_HOOK_MATCH(nm,ptr,ret,)), "struct Hook_nm { using funcType = ret (*)(); static_assert(std::is_same_v<funcType, ::Hooking::InternalMethodCheck<decltype(ptr)>::funcType>, \"Hook method signature does not match!\"); constexpr static const char* name() { return \"nm\"; } static const MethodInfo* getInfo() { return ::il2cpp_utils::il2cpp_type_check::MetadataGetter<ptr>::get(); } static funcType* trampoline() { return &nm; } static inline ret (*nm)() = nullptr; static funcType hook() { return &::Hooking::HookCatchWrapper<&hook_nm, funcType>::wrapper; } static ret hook_nm(); }; ret Hook_nm::hook_nm()");
STRING_ASSERT(STRINGIFY(MAKE_HOOK_FIND_CLASS_INSTANCE(nm,ns,cl,mn,ret,)), "struct Hook_nm { using funcType = ret (*)(); constexpr static const char* name() { return \"nm\"; } static const MethodInfo* getInfo() { return ::il2cpp_utils::MethodTypeCheck<typename ::il2cpp_utils::InstanceMethodConverter<funcType>::fType>::find(ns, cl, mn); } static funcType* trampoline() { return &nm; } static inline ret (*nm)() = nullptr; static funcType hook() { return &::Hooking::HookCatchWrapper<&hook_nm, funcType>::wrapper; } static ret hook_nm(); }; ret Hook_nm::hook_nm()");
STRING_ASSERT(STRINGIFY(DECLARE_CTOR(name,)), "public: void name(); template<::il2cpp_utils::CreationType creationType = ::il2cpp_utils::CreationType::Temporary, class... TArgs> static ___TargetType* New_ctor(TArgs&&... args) { static_assert(::custom_types::Decomposer<decltype(&___TargetType::name)>::convertible<TArgs...>(), \"Arguments provided to New_ctor must be convertible to the constructor!\"); ___TargetType* obj; if constexpr (creationType == ::il2cpp_utils::CreationType::Temporary) { obj = reinterpret_cast<___TargetType*>(::il2cpp_functions::object_new(___TypeRegistration::klass_ptr)); } else { obj = reinterpret_cast<___TargetType*>(::il2cpp_utils::createManual(___TypeRegistration::klass_ptr)); } obj->name(std::forward<TArgs>(args)...); return obj; } private: template<typename T> struct ___MethodRegistrator_name; template<typename R, typename T, typename... TArgs> struct ___MethodRegistrator_name<R (T::*)(TArgs...)> : ::custom_types::MethodRegistrator { ___MethodRegistrator_name() { ___TargetType::___TypeRegistration::addMethod(this); } constexpr const char* name() const override { return \"name\"; } constexpr const char* csharpName() const override { return \".ctor\"; } int flags() const override { return 0x0006 | 0x0080 | 0x0800 | 0x1000; } const MethodInfo* virtualMethod() const override { return nullptr; } const Il2CppType* returnType() const override { il2cpp_functions::Init(); return ::il2cpp_functions::class_get_type(::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<R>::get()); } std::vector<ParameterInfo> params() const override { int32_t counter = 0; il2cpp_functions::Init(); return {(ParameterInfo{\"param\", counter++, static_cast<uint32_t>(-1), ::il2cpp_functions::class_get_type(::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<TArgs>::get())})...}; } uint8_t params_size() const override { return sizeof...(TArgs); } Il2CppMethodPointer methodPointer() const override { return reinterpret_cast<Il2CppMethodPointer>(&::custom_types::invoker_creator<decltype(&___TargetType::name)>::wrap<&___TargetType::name>); } InvokerMethod invoker() const override { return &::custom_types::invoker_creator<decltype(&___TargetType::name)>::invoke; } }; static inline ___MethodRegistrator_name<decltype(&___TargetType::name)> ___name_MethodRegistrator");
STRING_ASSERT(STRINGIFY(DEFINE_OVERRIDE_METHOD(ret, name, overridingMethodInfo,)), "public: ret name private: template<typename T> struct ___MethodRegistrator_name; template<typename R, typename T, typename... TArgs> struct ___MethodRegistrator_name<R (T::*)(TArgs...)> : ::custom_types::MethodRegistrator { ___MethodRegistrator_name() { ___TargetType::___TypeRegistration::addMethod(this); } constexpr const char* name() const override { return \"name\"; } constexpr const char* csharpName() const override { return \"name\"; } int flags() const override { return (overridingMethodInfo->flags & ~0x0400) | 0x0006 | 0x0080; } const MethodInfo* virtualMethod() const override { return overridingMethodInfo; } const Il2CppType* returnType() const override { il2cpp_functions::Init(); return ::il2cpp_functions::class_get_type(::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<R>::get()); } std::vector<ParameterInfo> params() const override { int32_t counter = 0; il2cpp_functions::Init(); return {(ParameterInfo{\"param\", counter++, static_cast<uint32_t>(-1), ::il2cpp_functions::class_get_type(::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<TArgs>::get())})...}; } uint8_t params_size() const override { return sizeof...(TArgs); } Il2CppMethodPointer methodPointer() const override { return reinterpret_cast<Il2CppMethodPointer>(&::custom_types::invoker_creator<decltype(&___TargetType::name)>::wrap<&___TargetType::name>); } InvokerMethod invoker() const override { return &::custom_types::invoker_creator<decltype(&___TargetType::name)>::invoke; } }; static inline ___MethodRegistrator_name<decltype(&___TargetType::name)> ___name_MethodRegistrator");

extern bool (*ResolveICalls)(Logger *logger);
extern void (*InstallHooks)(Logger *logger);
