#define BSC_ICALL(name, iname, retval, ...) \
	retval (*name)(__VA_ARGS__) = NULL; \
	INJECT_CALLBACK(BeatUpClient::ApplyPatches, { \
		name = (retval (*)(__VA_ARGS__))il2cpp_functions::resolve_icall(iname); \
		if(name) \
			BeatUpClient::logger->info("Resolved icall `" iname "`"); \
		else \
			BeatUpClient::logger->critical("Failed to resolve `" iname "`"); \
		return !name; \
	})

#include "global.hpp"

PatchFunc BeatUpClient::ApplyPatches = []() {
	logger->info("BeatUpClient::ApplyPatches()");
	return false;
};
