#!/bin/make
VERSION := 0.5.3

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
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\"author\":\"rcelyte\",\"description\":\"Tweaks and enhancements for enabling modded multiplayer\",\"gameVersion\":\"1.20.0\",\"dependsOn\":{\"BSIPA\":\"*\"},\"conflictsWith\":{\"BeatTogether\":\"*\"},\"loadBefore\":[\"MultiplayerCore\"],\"id\":\"BeatUpClient\",\"name\":\"BeatUpClient\",\"version\":\"$(VERSION)\",\"links\":{\"project-source\":\"https://github.com/rcelyte/BeatUpRcelyte\"}}" > .obj/manifest.json
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe $(BSINSTALL)
	csc -nologo -t:library -nostdlib -o+ -debug- -nullable+ -unsafe+ -w:4 -warnaserror+ -langversion:9 -define:MPCORE_SUPPORT $(CSFILES) -res:.obj/manifest.json,.manifest.json -res:data.bundle,BeatUpClient.data -out:"$@" -r:.obj/Refs/Libs/0Harmony.dll,.obj/Refs/Libs/Hive.Versioning.dll,.obj/Refs/Libs/Mono.Cecil.dll,.obj/Refs/Libs/MonoMod.RuntimeDetour.dll,.obj/Refs/Libs/MonoMod.Utils.dll,.obj/Refs/Libs/Newtonsoft.Json.dll,.obj/Refs/Managed/BeatmapCore.dll,.obj/Refs/Managed/BGNet.dll,.obj/Refs/Managed/Colors.dll,.obj/Refs/Managed/GameplayCore.dll,.obj/Refs/Managed/HMLib.dll,.obj/Refs/Managed/HMUI.dll,.obj/Refs/Managed/IPA.Loader.dll,.obj/Refs/Managed/LiteNetLib.dll,.obj/Refs/Managed/Main.dll,.obj/Refs/Managed/mscorlib.dll,.obj/Refs/Managed/netstandard.dll,.obj/Refs/Managed/Polyglot.dll,.obj/Refs/Managed/System.IO.Compression.dll,.obj/Refs/Managed/UnityEngine.AssetBundleModule.dll,.obj/Refs/Managed/UnityEngine.AudioModule.dll,.obj/Refs/Managed/UnityEngine.CoreModule.dll,.obj/Refs/Managed/UnityEngine.ImageConversionModule.dll,.obj/Refs/Managed/UnityEngine.UI.dll,.obj/Refs/Managed/UnityEngine.UIModule.dll,.obj/Refs/Managed/UnityEngine.UnityWebRequestAudioModule.dll,.obj/Refs/Managed/UnityEngine.UnityWebRequestModule.dll,.obj/Refs/Managed/Unity.TextMeshPro.dll,.obj/Refs/Managed/VRUI.dll,.obj/Refs/Managed/Zenject.dll,.obj/Refs/Managed/Zenject-usage.dll,.obj/Refs/Plugins/MultiplayerCore.dll,.obj/Refs/Plugins/SiraUtil.dll,.obj/Refs/Plugins/SongCore.dll,.obj/Refs/Managed/Unity.ResourceManager.dll,.obj/Refs/Managed/System.Net.Http.dll

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
