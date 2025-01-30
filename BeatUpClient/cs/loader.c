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
	void *image = RETURN_UNLESS(mono_image_loaded("IPA.Injector"));
	void *class = RETURN_UNLESS(mono_class_from_name(image, "IPA.Injector", "Injector"));
	void *field = RETURN_UNLESS(mono_class_get_field_from_name(class, "bootstrapped"));
	void *vtable = RETURN_UNLESS(mono_class_vtable(domain, class));
	int32_t bootstrapped = false;
	mono_field_static_get_value(vtable, field, &bootstrapped);
	return bootstrapped;
}

static void *LateLoad(void *sceneName, int32_t sceneBuildIndex, void *parameters, bool mustCompleteNextFrame) {
	if(called)
		goto end;
	fprintf(stderr, "[BeatUpClient|Native] Loading\n");
	called = true;
	int32_t status = 1;
	void *image = mono_image_open_from_data(BeatUpClientFreestanding_dll, sizeof_BeatUpClientFreestanding_dll, 0, &status);
	if(status || !image) {
		fprintf(stderr, "[BeatUpClient|Native] mono_image_open_from_data() failed\n");
		goto end;
	}
	void *assembly = mono_assembly_load_from(image, "", &status);
	if(status || !assembly) {
		fprintf(stderr, "[BeatUpClient|Native] mono_assembly_load_from() failed\n");
		goto end;
	}
	void *desc = mono_method_desc_new(IPALoaded() ? "BeatUpClient:NativeEnable_BSIPA" : "BeatUpClient:NativeEnable", 1);
	void *run = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);
	if(!run) {
		fprintf(stderr, "[BeatUpClient|Native] mono_method_desc_search_in_image(\"BeatUpClient:NativeEnable\") failed\n");
		goto end;
	}
	void *version = mono_string_new_len(domain, MOD_VERSION, sizeof(MOD_VERSION) - sizeof(""));
	mono_runtime_invoke(run, NULL, &version, NULL);
	end:
	return LoadSceneAsyncNameIndexInternal_Injected(sceneName, sceneBuildIndex, parameters, mustCompleteNextFrame);
}

#include <windows.h>
bool __declspec(dllexport) __stdcall LoadOVRPlugin() {
	fprintf(stderr, "[BeatUpClient|Native] Bootstrapping\n");

	const HINSTANCE libmono_dll = LoadLibraryA("mono-2.0-bdwgc.dll");
	if(libmono_dll == NULL) {
		fprintf(stderr, "[BeatUpClient|Native] Failed to load library mono-2.0-bdwgc.dll\n");
		MessageBoxA(NULL, "BeatUpClient Loader", "Failed to load library mono-2.0-bdwgc.dll", MB_OK);
		return true;
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
		fprintf(stderr, "[BeatUpClient|Native] GetProcAddress() failed\n");
		MessageBoxA(NULL, "BeatUpClient Loader", "GetProcAddress() failed", MB_OK);
		return true;
	}

	domain = mono_domain_get();
	void *assembly = mono_domain_assembly_open(domain, "./Beat Saber_Data/Managed/UnityEngine.CoreModule.dll");
	void *image = mono_assembly_get_image(assembly);
	// interop_debug_class(image, "UnityEngine.SceneManagement", "SceneManagerAPIInternal");
	void *desc = mono_method_desc_new("UnityEngine.SceneManagement.SceneManagerAPIInternal:LoadSceneAsyncNameIndexInternal_Injected", 1);
	void *method = mono_method_desc_search_in_image(desc, image);
	*(void**)&LoadSceneAsyncNameIndexInternal_Injected = mono_lookup_internal_call(method);
	void *(*LateLoad_ptr)(void *sceneName, int32_t sceneBuildIndex, void *parameters, bool mustCompleteNextFrame) = LateLoad;
	mono_add_internal_call("UnityEngine.SceneManagement.SceneManagerAPIInternal::LoadSceneAsyncNameIndexInternal_Injected", *(const void**)&LateLoad_ptr);
	mono_method_desc_free(desc);
	return true;
}
