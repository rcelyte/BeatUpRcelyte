#!/bin/make
DESTDIR := /usr/local

MAKEFLAGS += --no-print-directory -j$(command nproc 2>/dev/null || echo 2)
.SILENT:
.ONESHELL:

HOST := $(shell uname -m)
NATIVE_CC ?= cc
OBJDIR := .obj/$(shell $(CC) -dumpmachine)
FILES := $(wildcard src/*.c src/*/*.c)
HTMLS := $(wildcard src/*/*.html)
LIBS := libmbedtls.a libmbedx509.a libmbedcrypto.a
OBJS := $(FILES:%=$(OBJDIR)/%.o) $(HTMLS:%=$(OBJDIR)/%.s) $(LIBS:%=$(OBJDIR)/%)
DEPS := $(FILES:%=$(OBJDIR)/%.d)

CFLAGS := -std=gnu2x -Imbedtls/include -Wall -Wextra -Werror -pedantic-errors -DMP_EXTENDED_ROUTING
LDFLAGS := -O2 -Wl,--gc-sections,--fatal-warnings -fno-pie -pthread

sinclude makefile.user

ifdef DEBUG
CFLAGS += -g -DDEBUG
endif

default: beatupserver

beatupserver: $(OBJS)
	@echo "[cc $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

beatupserver.%: $(OBJS)
	@echo "[cc $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

$(OBJDIR)/%.c.o: %.c src/packets.gen.h $(OBJDIR)/libs.mk mbedtls/.git makefile
	@echo "[cc $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@" -MMD -MP

$(OBJDIR)/%.html.s: %.html makefile # waiting for `#embed`...
	@mkdir -p "$(@D)"
	tr -d '\r\n\t' < "$<" > "$(basename $@)"
	printf "\t.global $(basename $(notdir $<))_html\n$(basename $(notdir $<))_html:\n\t.incbin \"$(basename $@)\"\n\t.global $(basename $(notdir $<))_html_end\n$(basename $(notdir $<))_html_end:\n.section \".note.GNU-stack\"\n" > "$@"

$(OBJDIR)/libmbed%.a: mbedtls/.git
	@echo "[make $(notdir $@)]"
	mkdir -p "$@.build/"
	cp -r mbedtls/3rdparty/ mbedtls/include/ mbedtls/library/ mbedtls/scripts/ "$@.build/"
	$(MAKE) -C "$@.build/library" CC=$(CC) AR=$(AR) PYTHON=true PERL=true $(notdir $@)
	mv "$@.build/library/$(notdir $@)" "$@"
	rm -r "$@.build/"

mbedtls/.git:
	git submodule update --init

$(OBJDIR)/libs.mk: makefile
	@mkdir -p "$(@D)"
	@cat <<EOF | $(CC) -E - -o "$@"
	#if 1
	LDFLAGS := --static \$$(LDFLAGS)
	#elif 0
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS := -fsanitize=address -static-libsan \$$(LDFLAGS)
	#endif
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN32__)
	CFLAGS += -DWINDOWS
	LDFLAGS += -lws2_32
	#endif
	EOF

src/packets.gen.h: src/packets.txt src/wire.txt .obj/gen.$(HOST) makefile
	@echo "[gen $(notdir $@)]"
	./.obj/gen.$(HOST) "$<" "$@" src/packets.gen.c.h

.obj/gen.$(HOST): gen.c
	@echo "[cc $(notdir $@)]"
	@mkdir -p "$(@D)"
	$(NATIVE_CC) -std=c2x -no-pie "$<" -o "$@"

bsipa BeatUpClient/BeatUpClient.dll:
	$(MAKE) -C BeatUpClient BeatUpClient.dll

bmbf BeatUpClient/BeatUpClient.qmod:
	$(MAKE) -C BeatUpClient BeatUpClient.qmod

install: beatupserver
	@echo "[install $(notdir $<)]"
	install -D -m0755 "$<" $(DESTDIR)/bin/beatupserver

uninstall remove:
	rm -f $(DESTDIR)/bin/beatupserver

clean:
	@echo "[cleaning]"
	$(MAKE) -C BeatUpClient clean
	$(MAKE) -C mbedtls/library clean
	rm -rf .obj/
	rm -f beatupserver

.PHONY: default bsipa bmbf install uninstall remove clean

include $(OBJDIR)/libs.mk
sinclude $(DEPS)
