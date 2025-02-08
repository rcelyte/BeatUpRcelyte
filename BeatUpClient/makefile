#!/bin/make
VERSION := 0.15.2

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

thirdparty/ILRepack.exe:
	@echo "[curl $(notdir $@)]"
	@mkdir -p "$(@D)"
	curl -o .obj/ILRepack.nupkg https://globalcdn.nuget.org/packages/ilrepack.2.0.27.nupkg
	unzip -ojd .obj/ .obj/ILRepack.nupkg tools/ILRepack.exe
	rm .obj/ILRepack.nupkg
	echo "09acb91db953828f7f46ec79a51c2a65f73969e34f888ec95e0aa8029e66e2ac .obj/ILRepack.exe" | \
		sha256sum -c || (rm .obj/ILRepack.exe; false)
	mv .obj/ILRepack.exe $@

thirdparty/System.IO.Compression.dll:
	@echo "[curl $(notdir $@)]"
	@mkdir -p "$(@D)"
	curl -o .obj/System.IO.Compression.zip https://beatmods.com/cdn/mod/a4e9e26f61967e56168e08eecb01ab88.zip # 4.6.57
	unzip -ojd .obj/ .obj/System.IO.Compression.zip Libs/System.IO.Compression.dll
	rm .obj/System.IO.Compression.zip
	echo "c457ab509c57506c08c52d23ff0581ba677f2af930efc882c8ce8b03e1eb46ec .obj/System.IO.Compression.dll" | sha256sum -c || (rm .obj/System.IO.Compression.dll; false)
	mv .obj/System.IO.Compression.dll $@

thirdparty/MultiplayerCore.dll:
	@echo "[curl $(notdir $@)]"
	@mkdir -p "$(@D)"
	curl -Lo .obj/MultiplayerCore.zip https://github.com/Goobwabber/MultiplayerCore/releases/download/v1.5.3/MultiplayerCore-1.5.3-bs1.38.0-206ebf3.zip
	unzip -ojd .obj/ .obj/MultiplayerCore.zip Plugins/MultiplayerCore.dll
	rm .obj/MultiplayerCore.zip
	echo "a6423140df3fab109cfe34e79116c192b0861f5ea47d98951de946635372f2bb .obj/MultiplayerCore.dll" | sha256sum -c || (rm .obj/MultiplayerCore.dll; false)
	mv .obj/MultiplayerCore.dll $@

thirdparty/SiraUtil.dll:
	@echo "[curl $(notdir $@)]"
	@mkdir -p "$(@D)"
	curl -o .obj/SiraUtil.zip https://beatmods.com/cdn/mod/36eda431ac40738390362de70fe4e6c7.zip # 3.1.12
	unzip -ojd .obj/ .obj/SiraUtil.zip Plugins/SiraUtil.dll
	rm .obj/SiraUtil.zip
	echo "a159854628d95ec767bffaded4accd8ba537ab753ddfd8a31c8caacc3387dcd0 .obj/SiraUtil.dll" | sha256sum -c || (rm .obj/SiraUtil.dll; false)
	mv .obj/SiraUtil.dll $@

thirdparty/SongCore.dll:
	@echo "[curl $(notdir $@)]"
	@mkdir -p "$(@D)"
	curl -o .obj/SongCore.zip https://beatmods.com/cdn/mod/d6d96830b7755e1a1f44d7faf328ddc5.zip # 3.14.15
	unzip -ojd .obj/ .obj/SongCore.zip Plugins/SongCore.dll
	rm .obj/SongCore.zip
	echo "cf6fdc49573e0079cbcdc04ef87044ec315cb2524852d84cd855ad2d4d8d514e .obj/SongCore.dll" | sha256sum -c || (rm .obj/SongCore.dll; false)
	mv .obj/SongCore.dll $@

.obj/BeatUpClient.dll: $(CSFILES) .obj/MakeThingsPublic.exe thirdparty/System.IO.Compression.dll thirdparty/MultiplayerCore.dll thirdparty/SiraUtil.dll thirdparty/SongCore.dll makefile
	@echo "[csc $(notdir $@)]"
	@mkdir -p .obj/
	printf "{\n\
		\"\$$schema\": \"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\n\
		\"author\": \"rcelyte\",\n\
		\"description\": \"Tweaks and enhancements for enabling modded multiplayer\",\n\
		\"gameVersion\": \"1.38.0_11020369061\",\n\
		\"dependsOn\": {\"BSIPA\": \"^4.3.0\"},\n\
		\"conflictsWith\": {\"BeatTogether\": \"*\"},\n\
		\"loadBefore\": [\"MultiplayerCore\"],\n\
		\"id\": \"BeatUpClient\",\n\
		\"name\": \"BeatUpClient\",\n\
		\"version\": \"$(VERSION)\",\n\
		\"links\": {\"project-source\": \"https://github.com/rcelyte/BeatUpRcelyte/BeatUpClient\"}\n\
	}" > .obj/manifest.json
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe $(BSINSTALL)
	csc -nologo -t:library -nostdlib -o+ -debug- -nullable+ -unsafe+ -w:4 -nowarn:CS1702 -warnaserror+ -langversion:9 -define:MPCORE_SUPPORT $(CSFILES) \
		-res:.obj/manifest.json,.manifest.json -res:assets/cover.png,BeatUpClient.cover -res:assets/create.png,BeatUpClient.create -out:$@ \
		`find .obj/Refs/ -name "*.dll" | sed -e 's/^/-r:/'` -r:thirdparty/System.IO.Compression.dll

BeatUpClient.dll: thirdparty/ILRepack.exe .obj/BeatUpClient.dll thirdparty/System.IO.Compression.dll
	@echo "[ILRepack $@]"
	mono $^ -lib:.obj/Refs/Libs -lib:.obj/Refs/Managed -lib:.obj/Refs/Plugins -out:$@

.obj/BeatUpClientFreestanding.dll: thirdparty/ILRepack.exe .obj/BeatUpClient.dll thirdparty/System.IO.Compression.dll $(BSINSTALL)/Libs/Mono.Cecil.dll $(BSINSTALL)/Libs/MonoMod.Utils.dll $(BSINSTALL)/Libs/MonoMod.RuntimeDetour.dll $(BSINSTALL)/Libs/0Harmony.dll
	@echo "[ILRepack $@]"
	mono $^ -lib:.obj/Refs/Libs -lib:.obj/Refs/Managed -lib:.obj/Refs/Plugins -out:$@

loader.dll: cs/loader.c .obj/BeatUpClientFreestanding.dll
	@echo "[cc $@]"
	$(CC) -static -static-libgcc -shared -s -Wl,--gc-sections,--no-undefined "$<" -o "$@" -DMOD_VERSION=\"$(VERSION)\"

clean:
	@echo "[cleaning]"
	rm -rf .obj/ bin/ obj/ include/ shared/ BeatUpClient.dll BeatUpClient.qmod libBeatUpClient.so
	qpm-rust clear || true

.PHONY: clean ndk

sinclude $(FILES:%=$(OBJDIR)/%.d)
