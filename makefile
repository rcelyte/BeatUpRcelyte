#!make
DESTDIR=/usr/local

MAKEFLAGS += --no-print-directory -j$(command nproc 2>/dev/null || echo 2)
.SILENT:

HOST := $(shell uname -m)
OBJDIR := .obj/$(shell $(CC) -dumpmachine)
FILES := $(wildcard src/*.c src/*/*.c)
OBJS := $(FILES:%=$(OBJDIR)/%.o) 
DEPS := $(OBJS:.o=.d)

LIBS := libmbedtls.a libmbedx509.a libmbedcrypto.a
OBJS += $(LIBS:%=$(OBJDIR)/%)

CFLAGS := -g -std=gnu11 -Imbedtls/include -Wall -Wno-unused-function -Werror -pedantic-errors -DFORCE_MASSIVE_LOBBIES -DPER_PLAYER_DIFFICULTY=1 -DPER_PLAYER_MODIFIERS=1
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

$(OBJDIR)/libmbed%.a: mbedtls/.git
	@echo "[make $(notdir $@)]"
	mkdir -p "$@.build/"
	cp -r mbedtls/3rdparty/ mbedtls/include/ mbedtls/library/ mbedtls/scripts/ "$@.build/"
	$(MAKE) -C "$@.build/library" CC=$(CC) AR=$(AR) $(notdir $@)
	mv "$@.build/library/$(notdir $@)" "$@"
	rm -r "$@.build/"

mbedtls/.git:
	git submodule update --init

$(OBJDIR)/libs.mk: libs.c makefile
	@mkdir -p "$(@D)"
	$(CC) -E libs.c -o "$@"

src/packets.h: src/packets.txt gen.c makefile
	$(MAKE) .obj/gen.$(HOST)
	@echo "[gen $(notdir $@)]"
	./.obj/gen.$(HOST) "$<" "$@" src/packets.c

.obj/gen.$(HOST): gen.c
	@echo "[cc $(notdir $@)]"
	cc -std=c99 -no-pie "$<" -o "$@"

CertBypass.dll: CertBypass.cs
	mkdir -p .obj/
	printf "{\"\$$schema\":\"https://raw.githubusercontent.com/bsmg/BSIPA-MetadataFileSchema/master/Schema.json\",\"author\":\"rcelyte\",\"description\":\"Bypasses authentication for custom servers\",\"gameVersion\":\"1.19.1\",\"dependsOn\":{\"BSIPA\":\"*\"},\"id\":\"CertBypass\",\"name\":\"CertBypass\",\"version\":\"1.0.0\",\"links\":{\"project-source\":\"https://github.com/rcelyte/BeatUpRcelyte\"}}" > .obj/manifest.json
	csc -nologo -t:library -o+ -debug- "$<" -res:.obj/manifest.json,CertBypass.manifest.json -out:"$@" -r:$(BSINSTALL)/Libs/0Harmony.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/IPA.Loader.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/BGNet.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/HMLib.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/Main.dll,$(BSINSTALL)/Beat\ Saber_Data/Managed/UnityEngine.CoreModule.dll

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
