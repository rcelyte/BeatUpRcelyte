#!/bin/make
VERSION := 0.4.1

.SILENT:

sinclude ../makefile.user
sinclude makefile.user

HOST := $(shell uname -m)
CC := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/clang --target=aarch64-linux-android26
CXX := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ --target=aarch64-linux-android26
CFLAGS := -g -std=c2x -fPIC -fvisibility=hidden -Wall -Werror -pedantic-errors
CXXFLAGS := -g -std=c++20 -fPIC -fvisibility=hidden -Wall -Wno-dollar-in-identifier-extension -Wno-zero-length-array -Wno-gnu-statement-expression -Wno-format-pedantic -Wno-vla-extension -Wno-unused-function -Werror -pedantic-errors -Iextern/includes/libil2cpp/il2cpp/libil2cpp -Iextern/includes/codegen/include -Iextern/includes -DVERSION=\"$(VERSION)\" -DDISABLE_POOLABLE -DDISABLE_DOWNLOADER -DDISABLE_SELECTORS -DDISABLE_LOCALIZATION -DDISABLE_360
LDFLAGS := -static-libstdc++ -shared -Wl,--no-undefined,--gc-sections,--fatal-warnings -Lextern/libs -lbeatsaber-hook_3_8_5 -lcodegen -lcustom-types
ifdef NDK
OBJDIR := .obj/$(shell $(CXX) -dumpmachine)
else
OBJDIR := .obj/unknown
ndk:
	$(error Android NDK path not set)
endif
FILES := $(wildcard src/*.c src/*.cpp)
OBJS := $(FILES:%=$(OBJDIR)/%.o)

CSFILES := $(wildcard cs/*.cs cs/*/*.cs)
CSREFS := Libs/0Harmony.dll Libs/Hive.Versioning.dll Libs/Newtonsoft.Json.dll Plugins/SongCore.dll Plugins/SiraUtil.dll Plugins/MultiplayerCore.dll Data/mscorlib.dll Data/IPA.Loader.dll Data/BGNet.dll Data/BeatmapCore.dll Data/Colors.dll Data/GameplayCore.dll Data/HMLib.dll Data/HMUI.dll Data/Main.dll Data/UnityEngine.CoreModule.dll Data/UnityEngine.AssetBundleModule.dll Data/UnityEngine.AudioModule.dll Data/UnityEngine.ImageConversionModule.dll Data/UnityEngine.UnityWebRequestModule.dll Data/UnityEngine.UnityWebRequestAudioModule.dll Data/UnityEngine.UIModule.dll Data/UnityEngine.UI.dll Data/Unity.TextMeshPro.dll Data/Polyglot.dll Data/System.IO.Compression.dll Data/LiteNetLib.dll Data/VRUI.dll Data/Zenject.dll Data/Zenject-usage.dll
CSREFS += Data/netstandard.dll # TODO: Required by `Hive.Versioning.VersionRange`; remove once beta version check is gone
PUBREFS := $(CSREFS:%=.obj/Refs/%)

libBeatUpClient.so: $(OBJS) | ndk
	@echo "[cxx $@]"
	$(CXX) $(LDFLAGS) $(OBJS) -o "$@"

$(OBJDIR)/%.c.o: %.c src/packets.h makefile | ndk
	@echo "[cc $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@" -MMD -MP

$(OBJDIR)/%.cpp.o: %.cpp src/packets.h extern makefile | ndk
	@echo "[cxx $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@" -MMD -MP

src/packets.c: src/packets.txt common/packets.txt ../.obj/gen.$(HOST) makefile
	@echo "[gen $(notdir $@)]"
	../.obj/gen.$(HOST) "$<" src/packets.h "$@"

../.obj/gen.$(HOST): ../gen.c
	$(MAKE) -C .. .obj/gen.$(HOST)

.obj/mod.json:
	@echo "[printf $(notdir $@)]"
	@mkdir -p "$(@D)"
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/Lauriethefish/QuestPatcher.QMod/main/QuestPatcher.QMod/Resources/qmod.schema.json\",\"_QPVersion\":\"0.1.1\",\"name\":\"BeatUpClient\",\"id\":\"BeatUpClient\",\"author\":\"rcelyte\",\"version\":\"$(VERSION)\",\"packageId\":\"com.beatgames.beatsaber\",\"packageVersion\":\"1.21.0\",\"description\":\"Tweaks and enhancements for enabling modded multiplayer\",\"coverImage\":\"cover.png\",\"dependencies\":[],\"modFiles\":[\"libBeatUpClient.so\"],\"libraryFiles\":[\"libbeatsaber-hook_3_8_5.so\"],\"fileCopies\":[],\"copyExtensions\":[]}" > .obj/mod.json

BeatUpClient.qmod: libBeatUpClient.so .obj/mod.json
	@echo "[zip $@]"
	zip -j BeatUpClient.qmod cover.png extern/libs/libbeatsaber-hook*.so libBeatUpClient.so .obj/mod.json

extern:
	@echo "[qpm restore]"
	qpm-rust restore

.obj/MakeThingsPublic.exe: MakeThingsPublic.cs makefile
	@echo "[csc $(notdir $@)]"
	@mkdir -p "$(@D)"
	csc -nologo -o+ -debug- -nullable+ -w:4 -warnaserror+ -langversion:8 "$<" -out:"$@" -r:$(BSINSTALL)/Libs/Mono.Cecil.dll
	MONO_PATH=$(BSINSTALL)/Libs mono --aot -O=all "$@"

.obj/Refs/Data/%.dll: $(BSINSTALL)/Beat\ Saber_Data/Managed/%.dll .obj/MakeThingsPublic.exe
	@echo "[MakeThingsPublic $(notdir $@)]"
	@mkdir -p "$(@D)"
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe "$<" "$@"
.obj/Refs/Plugins/%.dll: $(BSINSTALL)/Plugins/%.dll .obj/MakeThingsPublic.exe
	@echo "[MakeThingsPublic $(notdir $@)]"
	@mkdir -p "$(@D)"
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe "$<" "$@"
.obj/Refs/Libs/%.dll: $(BSINSTALL)/Libs/%.dll .obj/MakeThingsPublic.exe
	@echo "[MakeThingsPublic $(notdir $@)]"
	@mkdir -p "$(@D)"
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe "$<" "$@"

BeatUpClient.dll: $(CSFILES) $(PUBREFS) makefile
	@echo "[csc $@]"
	@mkdir -p .obj/
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\"author\":\"rcelyte\",\"description\":\"Tweaks and enhancements for enabling modded multiplayer\",\"gameVersion\":\"1.20.0\",\"dependsOn\":{\"BSIPA\":\"*\"},\"conflictsWith\":{\"BeatTogether\":\"*\"},\"loadBefore\":[\"MultiplayerCore\"],\"id\":\"BeatUpClient\",\"name\":\"BeatUpClient\",\"version\":\"$(VERSION)\",\"links\":{\"project-source\":\"https://github.com/rcelyte/BeatUpRcelyte\"}}" > .obj/manifest.json
	csc -nologo -t:library -nostdlib -o+ -debug- -nullable+ -unsafe+ -w:4 -warnaserror+ -langversion:8 -define:MPCORE_SUPPORT $(CSFILES) -res:.obj/manifest.json,.manifest.json -res:data.bundle,BeatUpClient.data -out:"$@" $(PUBREFS:%=-r:%)

clean:
	@echo "[cleaning]"
	rm -rf .folderNameWithoutSpaces/ .obj/ .tmp/ bin/ obj/ include/ shared/ BeatUpClient.dll BeatUpClient.qmod libBeatUpClient.so
	qpm-rust clear || true

.PHONY: clean ndk

sinclude $(FILES:%=$(OBJDIR)/%.d)