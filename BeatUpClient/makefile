#!/bin/make
VERSION := 0.7.0

.SILENT:

sinclude ../makefile.user
sinclude makefile.user

HOST := $(shell uname -m)
CC := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/clang --target=aarch64-linux-android26
CXX := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ --target=aarch64-linux-android26
CFLAGS := -g -std=c2x -fPIC -fvisibility=hidden -Wall -Wextra -Werror -pedantic-errors
CXXFLAGS := -g -std=c++20 -fPIC -fvisibility=hidden -Wall -Wextra -Wno-dollar-in-identifier-extension -Wno-zero-length-array -Wno-gnu-statement-expression -Wno-format-pedantic -Wno-vla-extension -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Werror -pedantic-errors -Iextern/includes/libil2cpp/il2cpp/libil2cpp -Iextern/includes/codegen/include -Iextern/includes -DVERSION=\"$(VERSION)\" -DDISABLE_POOLABLE -DDISABLE_DOWNLOADER -DDISABLE_SELECTORS -DDISABLE_LOCALIZATION -DDISABLE_360
LDFLAGS := -static-libstdc++ -shared -Wl,--no-undefined,--gc-sections,--fatal-warnings -Lextern/libs -lbeatsaber-hook_3_13_0 -lcodegen -lcustom-types
ifdef NDK
OBJDIR := .obj/$(shell $(CXX) -dumpmachine)
else
OBJDIR := .obj/unknown
ndk:
	$(error Android NDK path not set)
endif
FILES := $(wildcard src/*.c src/*.cpp src/*/*.cpp)
OBJS := $(FILES:%=$(OBJDIR)/%.o)

CSFILES := $(wildcard cs/*.cs cs/*/*.cs)

libBeatUpClient.so: $(OBJS) | ndk
	@echo "[cxx $@]"
	$(CXX) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.c.o: %.c src/packets.gen.h makefile | ndk
	@echo "[cc $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@" -MMD -MP

$(OBJDIR)/%.cpp.o: %.cpp src/packets.gen.h extern makefile | ndk
	@echo "[cxx $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@" -MMD -MP

src/packets.gen.h: src/packets.txt ../.obj/gen.$(HOST) makefile
	@echo "[gen $(notdir $@)]"
	../.obj/gen.$(HOST) "$<" "$@" src/packets.gen.c.h

../.obj/gen.$(HOST): ../gen.c
	$(MAKE) -C .. .obj/gen.$(HOST)

.obj/mod.json:
	@echo "[printf $(notdir $@)]"
	@mkdir -p "$(@D)"
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/Lauriethefish/QuestPatcher.QMod/main/QuestPatcher.QMod/Resources/qmod.schema.json\",\"_QPVersion\":\"0.1.1\",\"name\":\"BeatUpClient\",\"id\":\"BeatUpClient\",\"author\":\"rcelyte\",\"version\":\"$(VERSION)\",\"packageId\":\"com.beatgames.beatsaber\",\"packageVersion\":\"1.24.0\",\"description\":\"Tweaks and enhancements for enabling modded multiplayer\",\"coverImage\":\"cover.png\",\"dependencies\":[{\"version\":\"^0.15\",\"id\":\"custom-types\",\"downloadIfMissing\":\"https://github.com/sc2ad/Il2CppQuestTypePatching/releases/download/v0.15.19/CustomTypes.qmod\"},{\"version\":\"^0.25.0\",\"id\":\"codegen\",\"downloadIfMissing\":\"https://github.com/sc2ad/BeatSaber-Quest-Codegen/releases/download/v0.25.0/Codegen.qmod\"}],\"modFiles\":[\"libBeatUpClient.so\"],\"libraryFiles\":[\"libbeatsaber-hook_3_13_0.so\"],\"fileCopies\":[],\"copyExtensions\":[]}" > .obj/mod.json

BeatUpClient.qmod: libBeatUpClient.so .obj/mod.json
	@echo "[zip $@]"
	zip -j BeatUpClient.qmod cover.png extern/libs/libbeatsaber-hook*.so libBeatUpClient.so .obj/mod.json

extern:
	@echo "[qpm restore]"
	qpm-rust restore

.obj/MakeThingsPublic.exe: MakeThingsPublic.cs makefile
	@echo "[csc $(notdir $@)]"
	@mkdir -p "$(@D)"
	csc -nologo -o+ -debug- -nullable+ -w:4 -warnaserror+ -langversion:9 "$<" -out:"$@" -r:$(BSINSTALL)/Libs/Mono.Cecil.dll
	MONO_PATH=$(BSINSTALL)/Libs mono --aot -O=all "$@"

BeatUpClient.dll: $(CSFILES) .obj/MakeThingsPublic.exe makefile
	@echo "[csc $@]"
	@mkdir -p .obj/
	printf "{\n\
		\"\$$schema\": \"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\n\
		\"author\": \"rcelyte\",\n\
		\"description\": \"Tweaks and enhancements for enabling modded multiplayer\",\n\
		\"gameVersion\": \"1.34.2_7115288407\",\n\
		\"dependsOn\": {\"BSIPA\": \"^4.3.0\"},\n\
		\"conflictsWith\": {\"BeatTogether\": \"*\"},\n\
		\"loadBefore\": [\"MultiplayerCore\"],\n\
		\"id\": \"BeatUpClient\",\n\
		\"name\": \"BeatUpClient\",\n\
		\"version\": \"$(VERSION)\",\n\
		\"links\": {\"project-source\": \"https://github.com/rcelyte/BeatUpRcelyte/BeatUpClient\"}\n\
	}" > .obj/manifest.json
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe $(BSINSTALL)
	csc -nologo -t:library -nostdlib -o+ -debug- -nullable+ -unsafe+ -w:4 -warnaserror+ -langversion:9 $(CSFILES) \
		-res:.obj/manifest.json,.manifest.json -res:data.bundle,BeatUpClient.data -out:"$@" \
		-r:.obj/Refs/Libs/0Harmony.dll \
		-r:.obj/Refs/Libs/Hive.Versioning.dll \
		-r:.obj/Refs/Libs/Mono.Cecil.dll \
		-r:.obj/Refs/Libs/MonoMod.RuntimeDetour.dll \
		-r:.obj/Refs/Libs/MonoMod.Utils.dll \
		-r:.obj/Refs/Libs/Newtonsoft.Json.dll \
		-r:.obj/Refs/Managed/BeatmapCore.dll \
		-r:.obj/Refs/Managed/BGNet.dll \
		-r:.obj/Refs/Managed/Colors.dll \
		-r:.obj/Refs/Managed/GameplayCore.dll \
		-r:.obj/Refs/Managed/HMLib.dll \
		-r:.obj/Refs/Managed/HMUI.dll \
		-r:.obj/Refs/Managed/IPA.Loader.dll \
		-r:.obj/Refs/Managed/LiteNetLib.dll \
		-r:.obj/Refs/Managed/Main.dll \
		-r:.obj/Refs/Managed/mscorlib.dll \
		-r:.obj/Refs/Managed/netstandard.dll \
		-r:.obj/Refs/Managed/Polyglot.dll \
		-r:.obj/Refs/Libs/System.IO.Compression.dll \
		-r:.obj/Refs/Managed/UnityEngine.AssetBundleModule.dll \
		-r:.obj/Refs/Managed/UnityEngine.AudioModule.dll \
		-r:.obj/Refs/Managed/UnityEngine.CoreModule.dll \
		-r:.obj/Refs/Managed/UnityEngine.ImageConversionModule.dll \
		-r:.obj/Refs/Managed/UnityEngine.UI.dll \
		-r:.obj/Refs/Managed/UnityEngine.UIModule.dll \
		-r:.obj/Refs/Managed/UnityEngine.UnityWebRequestAudioModule.dll \
		-r:.obj/Refs/Managed/UnityEngine.UnityWebRequestModule.dll \
		-r:.obj/Refs/Managed/Unity.TextMeshPro.dll \
		-r:.obj/Refs/Managed/VRUI.dll \
		-r:.obj/Refs/Managed/Zenject.dll \
		-r:.obj/Refs/Managed/Zenject-usage.dll \
		-r:.obj/Refs/Plugins/MultiplayerCore.dll \
		-r:.obj/Refs/Plugins/SiraUtil.dll \
		-r:.obj/Refs/Plugins/SongCore.dll \
		-r:.obj/Refs/Managed/Unity.ResourceManager.dll \
		-r:.obj/Refs/Managed/System.Net.Http.dll \
		-r:.obj/Refs/Managed/Ignorance.dll \
		-r:.obj/Refs/Managed/BGLib.AppFlow.dll \
		-r:.obj/Refs/Managed/BGLib.DotnetExtension.dll \
		-r:.obj/Refs/Managed/BGLib.UnityExtension.dll \
		-r:.obj/Refs/Managed/Networking.dll \
		-r:.obj/Refs/Managed/Menu.CommonLib.dll

AudioPluginMsHRTF.dll: cs/loader.c BeatUpClient.dll
	@echo "[cc $@]"
	@mkdir -p .obj/
	printf "\t.global BeatUpClient_dll\nBeatUpClient_dll:\n\t.incbin \"BeatUpClient.dll\"\n\t.global BeatUpClient_dll_end\nBeatUpClient_dll_end:\n" > .obj/BeatUpClient.dll.S
	$(CC) -mwindows -m64 -static -static-libgcc -shared -s -Wl,--gc-sections,--no-undefined -L$(BSINSTALL)/MonoBleedingEdge/EmbedRuntime -l:mono-2.0-bdwgc.dll "$<" .obj/BeatUpClient.dll.S -o "$@" -DMOD_VERSION=\"$(VERSION)\"

clean:
	@echo "[cleaning]"
	rm -rf .obj/ bin/ obj/ include/ shared/ BeatUpClient.dll BeatUpClient.qmod libBeatUpClient.so
	qpm-rust clear || true

.PHONY: clean ndk

sinclude $(FILES:%=$(OBJDIR)/%.d)
