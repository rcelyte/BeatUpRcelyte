#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// TODO: also include Harmony when it isn't already available
__asm__(".section .rodata, \"a\"\n"
	"\"managed_bundle_\":\n"
	".incbin \".obj/BeatUpClientFreestanding.dll\"\n"
	"\"managed_bundle_end\":\n"
	".byte 0\n"
	".align 8\n"
	"\"managed_bundle_/size\":\n"
	".quad \"managed_bundle_end\" - \"managed_bundle_\"\n");
extern char BeatUpClientFreestanding_dll[] __asm__("managed_bundle_");
extern const uint64_t sizeof_BeatUpClientFreestanding_dll __asm__("managed_bundle_/size");

static void *(*mono_domain_get)() = NULL;
static void *(*mono_image_loaded)(const char *name) = NULL;
static void *(*mono_lookup_internal_call)(void *method) = NULL;
static void (*mono_add_internal_call)(const char *name, const void* method) = NULL;
static void *(*mono_image_open_from_data)(char *data, uint32_t data_len, int32_t need_copy, int32_t *status) = NULL;
static void *(*mono_assembly_load_from)(void *image, const char *fname, int32_t *status) = NULL;
static void *(*mono_method_desc_new)(const char *name, int32_t include_namespace) = NULL;
static void *(*mono_method_desc_search_in_image)(void *desc, void *image) = NULL;
static void (*mono_method_desc_free)(void *desc) = NULL;
static void *(*mono_runtime_invoke)(void *method, void *obj, void **params, void **exc) = NULL;
static void *(*mono_class_from_name)(void *image, const char* name_space, const char *name) = NULL;
static void *(*mono_class_get_field_from_name)(void *klass, const char *name) = NULL;
static void *(*mono_class_vtable)(void *domain, void *klass) = NULL;
static void (*mono_field_static_get_value)(void *vt, void *field, void *value) = NULL;
static void *(*mono_string_new_len)(void *domain, const char *text, unsigned int length) = NULL;
static void *(*mono_domain_assembly_open)(void *domain, const char *name) = NULL;
static void *(*mono_assembly_get_image)(void *assembly) = NULL;

static bool called = false;
static void *(*LoadSceneAsyncNameIndexInternal_Injected)(void *sceneName, int32_t sceneBuildIndex, void *parameters, bool mustCompleteNextFrame);

static void *res;
#define RETURN_UNLESS(ptr) \
	res = (ptr); \
	if(!res) \
		return false;

static void *domain = NULL;
static bool IPALoaded() {
	void *const image = RETURN_UNLESS(mono_image_loaded("IPA.Injector"));
	void *const class = RETURN_UNLESS(mono_class_from_name(image, "IPA.Injector", "Injector"));
	void *const field = RETURN_UNLESS(mono_class_get_field_from_name(class, "bootstrapped"));
	void *const vtable = RETURN_UNLESS(mono_class_vtable(domain, class));
	int32_t bootstrapped = false;
	mono_field_static_get_value(vtable, field, &bootstrapped);
	return bootstrapped;
}

enum UnityLogType {
	UnityLogType_Error = 0,
	UnityLogType_Warning = 2,
	UnityLogType_Log = 3,
	UnityLogType_Exception = 4,
};

typedef void (__stdcall *PFN_unityLog)(enum UnityLogType type, const char *message, const char *file, int line);
static PFN_unityLog unityLog = NULL;
static void LogUnity(const enum UnityLogType type, const unsigned line, const char function[], const char message[]) {
	char buffer[0x1000];
	const int buffer_len = snprintf(buffer, sizeof(buffer), "[BeatUpClient|Native] " __FILE__ ":%u(%s): %s\n", line, function, message);
	if(buffer_len < 0)
		return;
	if(unityLog != NULL) {
		unityLog(type, buffer, __FILE__, line);
	} else {
		fwrite(buffer, (unsigned)buffer_len, 1, stderr);
		fflush(stderr);
	}
}

static void *LateLoad(void *sceneName, int32_t sceneBuildIndex, void *parameters, bool mustCompleteNextFrame) {
	if(called)
		goto end;
	LogUnity(UnityLogType_Log, __LINE__, __FUNCTION__, "Loading");
	called = true;
	int32_t status = 1;
	void *const image = mono_image_open_from_data(BeatUpClientFreestanding_dll, sizeof_BeatUpClientFreestanding_dll, 0, &status);
	if(status || !image) {
		LogUnity(UnityLogType_Error, __LINE__, __FUNCTION__, "mono_image_open_from_data() failed");
		goto end;
	}
	void *const assembly = mono_assembly_load_from(image, "", &status);
	if(status || !assembly) {
		LogUnity(UnityLogType_Error, __LINE__, __FUNCTION__, "mono_assembly_load_from() failed");
		goto end;
	}
	void *const desc = mono_method_desc_new(IPALoaded() ? "BeatUpClient:NativeEnable_BSIPA" : "BeatUpClient:NativeEnable", 1);
	void *const run = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);
	if(!run) {
		LogUnity(UnityLogType_Error, __LINE__, __FUNCTION__, "mono_method_desc_search_in_image(\"BeatUpClient:NativeEnable\") failed");
		goto end;
	}
	void *version = mono_string_new_len(domain, MOD_VERSION, sizeof(MOD_VERSION) - sizeof(""));
	mono_runtime_invoke(run, NULL, &version, NULL);
	end: return LoadSceneAsyncNameIndexInternal_Injected(sceneName, sceneBuildIndex, parameters, mustCompleteNextFrame);
}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
__declspec(dllexport) void __stdcall UnityPluginLoad(PFN_unityLog *(__stdcall **const getInterface)(const unsigned long long (*guid)[2]));
[[gnu::visibility("default")]] void __stdcall UnityPluginLoad(PFN_unityLog *(__stdcall **const getInterface)(const unsigned long long (*guid)[2])) {
	unityLog = *(*getInterface)(&(unsigned long long[]){0x9E7507fA5B444D5DULL, 0x92FB979515EA83FCULL});

	LogUnity(UnityLogType_Log, __LINE__, __FUNCTION__, "Bootstrapping");

	const HINSTANCE libmono_dll = LoadLibraryA("mono-2.0-bdwgc.dll");
	if(libmono_dll == NULL) {
		LogUnity(UnityLogType_Error, __LINE__, __FUNCTION__, "Failed to load library mono-2.0-bdwgc.dll");
		MessageBoxA(NULL, "BeatUpClient Loader", "Failed to load library mono-2.0-bdwgc.dll", MB_OK);
		return;
	}
	#define LOAD_PROC(proc_) { \
		*(void**)&(proc_) = GetProcAddress(libmono_dll, #proc_); \
		if((proc_) == NULL) \
			goto fail0; \
	}
	LOAD_PROC(mono_domain_get)
	LOAD_PROC(mono_image_loaded)
	LOAD_PROC(mono_lookup_internal_call)
	LOAD_PROC(mono_add_internal_call)
	LOAD_PROC(mono_image_open_from_data)
	LOAD_PROC(mono_assembly_load_from)
	LOAD_PROC(mono_method_desc_new)
	LOAD_PROC(mono_method_desc_search_in_image)
	LOAD_PROC(mono_method_desc_free)
	LOAD_PROC(mono_runtime_invoke)
	LOAD_PROC(mono_class_from_name)
	LOAD_PROC(mono_class_get_field_from_name)
	LOAD_PROC(mono_class_vtable)
	LOAD_PROC(mono_field_static_get_value)
	LOAD_PROC(mono_string_new_len)
	LOAD_PROC(mono_domain_assembly_open)
	LOAD_PROC(mono_assembly_get_image)
	#undef LOAD_PROC
	if(false) fail0: {
		LogUnity(UnityLogType_Error, __LINE__, __FUNCTION__, "GetProcAddress() failed");
		MessageBoxA(NULL, "BeatUpClient Loader", "GetProcAddress() failed", MB_OK);
		return;
	}

	domain = mono_domain_get();
	void *const assembly = mono_domain_assembly_open(domain, "./Beat Saber_Data/Managed/UnityEngine.CoreModule.dll");
	void *const image = mono_assembly_get_image(assembly);
	// interop_debug_class(image, "UnityEngine.SceneManagement", "SceneManagerAPIInternal");
	void *const desc = mono_method_desc_new("UnityEngine.SceneManagement.SceneManagerAPIInternal:LoadSceneAsyncNameIndexInternal_Injected", 1);
	void *const method = mono_method_desc_search_in_image(desc, image);
	*(void**)&LoadSceneAsyncNameIndexInternal_Injected = mono_lookup_internal_call(method);
	void *(*const LateLoad_ptr)(void *sceneName, int32_t sceneBuildIndex, void *parameters, bool mustCompleteNextFrame) = LateLoad;
	mono_add_internal_call("UnityEngine.SceneManagement.SceneManagerAPIInternal::LoadSceneAsyncNameIndexInternal_Injected", *(const void**)&LateLoad_ptr);
	mono_method_desc_free(desc);
	return;
}

__declspec(dllexport) void __stdcall UnityPluginUnload();
[[gnu::visibility("default")]] void __stdcall UnityPluginUnload() {
	unityLog = NULL;
}
