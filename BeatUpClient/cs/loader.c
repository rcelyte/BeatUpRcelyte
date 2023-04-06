#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void *mono_domain_get();
void *mono_image_loaded(const char *name);
void *mono_lookup_internal_call(void *method);
void mono_add_internal_call(const char *name, const void* method);
void *mono_image_open_from_data(char *data, uint32_t data_len, int32_t need_copy, int32_t *status);
void *mono_assembly_load_from(void *image, const char *fname, int32_t *status);
void *mono_method_desc_new(const char *name, int32_t include_namespace);
void *mono_method_desc_search_in_image(void *desc, void *image);
void mono_method_desc_free(void *desc);
void *mono_runtime_invoke(void *method, void *obj, void **params, void **exc);
void *mono_class_from_name(void *image, const char* name_space, const char *name);
void *mono_class_get_field_from_name(void *klass, const char *name);
void *mono_class_vtable(void *domain, void *klass);
void mono_field_static_get_value(void *vt, void *field, void *value);
void *mono_string_new_len(void *domain, const char *text, unsigned int length);
void *mono_domain_assembly_open(void *domain, const char *name);
void *mono_assembly_get_image(void *assembly);

extern char BeatUpClient_dll[], BeatUpClient_dll_end[]; // TODO: also include Harmony when it isn't already available

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
	void *image = mono_image_open_from_data(BeatUpClient_dll, BeatUpClient_dll_end - BeatUpClient_dll, 0, &status);
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
	void *version = mono_string_new_len(domain, "0.5.1", sizeof("0.5.1") - 1);
	mono_runtime_invoke(run, NULL, &version, NULL);
	end:
	return LoadSceneAsyncNameIndexInternal_Injected(sceneName, sceneBuildIndex, parameters, mustCompleteNextFrame);
}

void __declspec(dllexport) __stdcall UnityPluginLoad(void *unityInterfaces) {
	fprintf(stderr, "[BeatUpClient|Native] Bootstrapping\n");
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
}
