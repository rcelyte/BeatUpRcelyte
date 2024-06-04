#!/bin/make
VERSION := 0.11.0

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
	curl https://globalcdn.nuget.org/packages/ilrepack.2.0.27.nupkg -o .obj/ILRepack.nupkg
	unzip -ojd .obj/ .obj/ILRepack.nupkg tools/ILRepack.exe
	rm .obj/ILRepack.nupkg
	echo "09acb91db953828f7f46ec79a51c2a65f73969e34f888ec95e0aa8029e66e2ac .obj/ILRepack.exe" | \
		sha256sum -c || (rm .obj/ILRepack.exe; false)
	mv .obj/ILRepack.exe $@

thirdparty/System.IO.Compression.dll:
	@echo "[curl $(notdir $@)]"
	@mkdir -p "$(@D)"
	curl https://beatmods.com/uploads/65779c825e029208799de4dd/universal/System.IO.Compression-4.6.57.zip -o .obj/System.IO.Compression.zip
	unzip -ojd .obj/ .obj/System.IO.Compression.zip Libs/System.IO.Compression.dll
	rm .obj/System.IO.Compression.zip
	echo "c457ab509c57506c08c52d23ff0581ba677f2af930efc882c8ce8b03e1eb46ec .obj/System.IO.Compression.dll" | \
		sha256sum -c || (rm .obj/System.IO.Compression.dll; false)
	mv .obj/System.IO.Compression.dll $@

BeatUpClient.dll: $(CSFILES) .obj/MakeThingsPublic.exe thirdparty/ILRepack.exe thirdparty/System.IO.Compression.dll makefile
	@echo "[csc $@]"
	@mkdir -p .obj/
	printf "{\n\
		\"\$$schema\": \"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\n\
		\"author\": \"rcelyte\",\n\
		\"description\": \"Tweaks and enhancements for enabling modded multiplayer\",\n\
		\"gameVersion\": \"1.37.0_9031708052\",\n\
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
		-res:.obj/manifest.json,.manifest.json -res:assets/cover.png,BeatUpClient.cover -res:assets/create.png,BeatUpClient.create -out:.obj/$@ \
		`find .obj/Refs/ -name "*.dll" | sed -e 's/^/-r:/'`
	mono thirdparty/ILRepack.exe /lib:.obj/Refs/Libs /lib:.obj/Refs/Managed /lib:.obj/Refs/Plugins /out:$@ .obj/$@ thirdparty/System.IO.Compression.dll

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
