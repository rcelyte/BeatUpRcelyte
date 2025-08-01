#!/bin/make
VERSION ::= 0.15.4

.SILENT:

sinclude ../makefile.user
sinclude makefile.user

CSFILES ::= $(wildcard cs/*.cs cs/*/*.cs)

default: BeatUpClient.dll

.obj/MakeThingsPublic.exe: MakeThingsPublic.cs makefile
	@echo "[csc $(notdir $@)]"
	@mkdir -p "$(@D)"
	csc -nologo -o+ -debug- -nullable+ -w:4 -warnaserror+ -langversion:9 "$<" -out:"$@" -r:$(BSINSTALL)/IPA/Libs/Mono.Cecil.dll
	MONO_PATH=$(BSINSTALL)/IPA/Libs mono --aot -O=all "$@"

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
	MONO_PATH=$(BSINSTALL)/IPA/Libs mono .obj/MakeThingsPublic.exe $(BSINSTALL)
	csc -nologo -t:library -nostdlib -o+ -debug- -nullable+ -unsafe+ -w:4 -nowarn:CS1702 -warnaserror+ -langversion:9 -define:MPCORE_SUPPORT $(CSFILES) \
		-res:.obj/manifest.json,.manifest.json -res:assets/cover.png,BeatUpClient.cover -res:assets/create.png,BeatUpClient.create -out:$@ \
		`find .obj/Refs/ -name "*.dll" | sed -e 's/^/-r:/'` -r:thirdparty/System.IO.Compression.dll

BeatUpClient.dll: thirdparty/ILRepack.exe .obj/BeatUpClient.dll thirdparty/System.IO.Compression.dll
	@echo "[ILRepack $@]"
	mono $^ -lib:.obj/Refs/IPA/Libs -lib:.obj/Refs/Managed -lib:.obj/Refs/Plugins -out:$@

.obj/BeatUpClientFreestanding.dll: thirdparty/ILRepack.exe .obj/BeatUpClient.dll thirdparty/System.IO.Compression.dll $(patsubst %,$(BSINSTALL)/IPA/Libs/%,0Harmony.dll Hive.Versioning.dll Mono.Cecil.dll MonoMod.RuntimeDetour.dll MonoMod.Utils.dll)
	@echo "[ILRepack $@]"
	mono $^ -lib:.obj/Refs/IPA/Libs -lib:.obj/Refs/Managed -lib:.obj/Refs/Plugins -out:$@

BeatUpClientNative.dll: cs/loader.c .obj/BeatUpClientFreestanding.dll
	@echo "[cc $@]"
	$(CC) -static -static-libgcc -shared -s -Wl,--gc-sections,--no-undefined -std=c99 "$<" -o "$@" -DMOD_VERSION=\"$(VERSION)\"

UnitySubsystemsManifest.json: makefile
	printf "{\n\
		\"name\": \"BeatUpClient\",\n\
		\"version\": \"$(VERSION)\",\n\
		\"libraryName\": \"BeatUpClientNative\",\n\
		\"displays\": [{\"id\": \"OpenXR Display\"}]\n\
	}" > "$@"

clean:
	@echo "[cleaning]"
	$(RM) -r .obj/ bin/ obj/ BeatUpClient.dll BeatUpClientNative.dll UnitySubsystemsManifest.json

.PHONY: default clean
