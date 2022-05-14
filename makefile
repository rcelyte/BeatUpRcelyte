#!/bin/make
DESTDIR := /usr/local
VERSION := 0.2.2

MAKEFLAGS += --no-print-directory -j$(command nproc 2>/dev/null || echo 2)
.SILENT:
.ONESHELL:

HOST := $(shell uname -m)
OBJDIR := .obj/$(shell $(CC) -dumpmachine)
FILES := $(wildcard src/*.c src/*/*.c)
HTMLS := $(wildcard src/*/*.html)
LIBS := libmbedtls.a libmbedx509.a libmbedcrypto.a
OBJS := $(FILES:%=$(OBJDIR)/%.o) $(HTMLS:%=$(OBJDIR)/%.s) $(LIBS:%=$(OBJDIR)/%)
DEPS := $(FILES:%=$(OBJDIR)/%.d)

CFLAGS := -g -std=gnu2x -Imbedtls/include -Wall -Werror -pedantic-errors -DFORCE_MASSIVE_LOBBIES
LDFLAGS := -O2 -Wl,--gc-sections,--fatal-warnings

CSREFS := ../Libs/0Harmony.dll ../Plugins/SongCore.dll ../Plugins/SiraUtil.dll ../Plugins/MultiplayerCore.dll Managed/IPA.Loader.dll Managed/BGNet.dll Managed/BeatmapCore.dll Managed/Colors.dll Managed/GameplayCore.dll Managed/HMLib.dll Managed/HMUI.dll Managed/Main.dll Managed/UnityEngine.CoreModule.dll Managed/UnityEngine.AssetBundleModule.dll Managed/UnityEngine.AudioModule.dll Managed/UnityEngine.ImageConversionModule.dll Managed/UnityEngine.UnityWebRequestModule.dll Managed/UnityEngine.UnityWebRequestAudioModule.dll Managed/UnityEngine.UIModule.dll Managed/UnityEngine.UI.dll Managed/Unity.TextMeshPro.dll Managed/Polyglot.dll Managed/System.IO.Compression.dll Managed/Newtonsoft.Json.dll Managed/LiteNetLib.dll Managed/VRUI.dll Managed/Zenject.dll Managed/Zenject-usage.dll
PUBREFS := $(CSREFS:%=.obj/Refs/Data/%)

sinclude makefile.user

ifdef DEBUG
CFLAGS += -DDEBUG
endif

default: beatupserver

beatupserver: $(OBJS)
	@echo "[cc $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

beatupserver.%: $(OBJS)
	@echo "[cc $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

$(OBJDIR)/%.c.o: %.c src/packets.h $(OBJDIR)/libs.mk mbedtls/.git makefile
	@echo "[cc $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@" -MMD -MP

$(OBJDIR)/%.html.s: %.html makefile
	@mkdir -p "$(@D)"
	tr -d '\r\n\t' < "$<" > "$(basename $@)"
	printf "\t.global $(basename $(notdir $<))_html\n$(basename $(notdir $<))_html:\n\t.incbin \"$(basename $@)\"\n\t.global $(basename $(notdir $<))_html_end\n$(basename $(notdir $<))_html_end:\n" > "$@"

$(OBJDIR)/libmbed%.a: mbedtls/.git
	@echo "[make $(notdir $@)]"
	mkdir -p "$@.build/"
	cp -r mbedtls/3rdparty/ mbedtls/include/ mbedtls/library/ mbedtls/scripts/ "$@.build/"
	$(MAKE) -C "$@.build/library" CC=$(CC) AR=$(AR) $(notdir $@)
	mv "$@.build/library/$(notdir $@)" "$@"
	rm -r "$@.build/"

mbedtls/.git:
	git submodule update --init

$(OBJDIR)/libs.mk: makefile
	@mkdir -p "$(@D)"
	@cat <<EOF | $(CC) -E - -o "$@"
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN32__)
	CFLAGS += -DWINDOWS
	LDFLAGS += -lws2_32 -lwinmm -lgdi32
	#else
	LDFLAGS += -pthread
	#if 0
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS := -no-pie -fsanitize=address \$$(LDFLAGS)
	#elif 1
	LDFLAGS := --static \$$(LDFLAGS)
	#endif
	#endif
	EOF

src/packets.h: src/packets.txt .obj/gen.$(HOST) makefile
	@echo "[gen $(notdir $@)]"
	./.obj/gen.$(HOST) "$<" "$@" src/packets.c

.obj/gen.$(HOST): gen.c
	@echo "[cc $(notdir $@)]"
	@mkdir -p "$(@D)"
	cc -std=c2x -no-pie "$<" -o "$@"

.obj/MakeThingsPublic.exe: MakeThingsPublic.cs makefile
	@echo "[csc $(notdir $@)]"
	@mkdir -p "$(@D)"
	csc -nologo -o+ -debug- -nullable+ -w:4 -warnaserror+ -langversion:8 "$<" -out:"$@" -r:$(BSINSTALL)/Libs/Mono.Cecil.dll
	MONO_PATH=$(BSINSTALL)/Libs mono --aot -O=all "$@"

.obj/Refs/Data/%.dll: $(BSINSTALL)/Beat\ Saber_Data/%.dll .obj/MakeThingsPublic.exe
	@echo "[MakeThingsPublic $(notdir $@)]"
	@mkdir -p "$(@D)"
	MONO_PATH=$(BSINSTALL)/Libs mono .obj/MakeThingsPublic.exe "$<" "$@"

BeatUpClient.dll: BeatUpClient.cs $(PUBREFS) makefile
	@echo "[csc $@]"
	@mkdir -p .obj/
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\"author\":\"rcelyte\",\"description\":\"Tweaks and enhancements for enabling modded multiplayer\",\"gameVersion\":\"1.20.0\",\"dependsOn\":{\"BSIPA\":\"*\"},\"conflictsWith\":{\"BeatTogether\":\"*\"},\"loadBefore\":[\"MultiplayerCore\"],\"id\":\"BeatUpClient\",\"name\":\"BeatUpClient\",\"version\":\"$(VERSION)\",\"links\":{\"project-source\":\"https://github.com/rcelyte/BeatUpRcelyte\"}}" > .obj/manifest.json
	csc -nologo -t:library -o+ -debug- -nullable+ -unsafe+ -w:4 -warnaserror+ -langversion:8 -define:MPCORE_SUPPORT "$<" -res:.obj/manifest.json,.manifest.json -res:data.bundle,BeatUpClient.data -out:"$@" $(PUBREFS:%=-r:%)

install: beatupserver
	@echo "[install $(notdir $<)]"
	install -D -m0755 "$<" $(DESTDIR)/bin/beatupserver

uninstall remove:
	rm -f $(DESTDIR)/bin/beatupserver

clean:
	@echo "[cleaning]"
	$(MAKE) -C mbedtls/library clean
	rm -rf .obj/
	rm -f beatupserver*

.PHONY: default install uninstall remove clean

include $(OBJDIR)/libs.mk
sinclude $(DEPS)
