#!make
DESTDIR := /usr/local
VERSION := 0.2.0

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

CFLAGS := -g -std=gnu11 -Imbedtls/include -Wall -Wno-unused-function -Werror -pedantic-errors -DFORCE_MASSIVE_LOBBIES
LDFLAGS := -O2 -Wl,--gc-sections,--fatal-warnings

sinclude makefile.user

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

src/packets.h: src/packets.txt gen.c makefile
	$(MAKE) .obj/gen.$(HOST)
	@echo "[gen $(notdir $@)]"
	./.obj/gen.$(HOST) "$<" "$@" src/packets.c

.obj/gen.$(HOST): gen.c
	@echo "[cc $(notdir $@)]"
	cc -std=c99 -no-pie "$<" -o "$@"

BeatUpClient.dll: BeatUpClient.cs makefile
	@echo "[csc $@]"
	mkdir -p .obj/
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\"author\":\"rcelyte\",\"description\":\"Tweaks and enhancements for enabling modded multiplayer\",\"gameVersion\":\"1.20.0\",\"dependsOn\":{\"BSIPA\":\"*\"},\"loadAfter\":[\"SongCore\",\"MultiplayerCore\"],\"id\":\"BeatUpClient\",\"name\":\"BeatUpClient\",\"version\":\"$(VERSION)\",\"links\":{\"project-source\":\"https://github.com/rcelyte/BeatUpRcelyte\"}}" > .obj/manifest.json
	csc -nologo -t:library -o+ -debug- -nullable+ -w:4 -warnaserror+ -langversion:8 "$<" -res:.obj/manifest.json,BeatUpClient.manifest.json -res:data.bundle,BeatUpClient.data -out:"$@" -r:$(BSINSTALL)/Libs/0Harmony.dll,$(BSINSTALL)/Plugins/SongCore.dll,$(BSINSTALL)/Plugins/SiraUtil.dll,$(BSINSTALL)/Plugins/MultiplayerCore.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/IPA.Loader.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/BGNet.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/BeatmapCore.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/GamePlayCore.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/HMLib.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/HMUI.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Main.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.CoreModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.AssetBundleModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.AudioModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.ImageConversionModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.UnityWebRequestModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.UnityWebRequestAudioModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.UIModule.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.UI.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Unity.TextMeshPro.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Polyglot.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/System.IO.Compression.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Newtonsoft.Json.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/LiteNetLib.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Zenject.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Zenject-usage.dll

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
