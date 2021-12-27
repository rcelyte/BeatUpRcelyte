#!make
DESTDIR=/usr/local

MAKEFLAGS += --no-print-directory -j$(command nproc 2>/dev/null || echo 2)
.SILENT:

HOST := $(shell uname -m)
OBJDIR := .obj/$(shell $(CC) -dumpmachine)
FILES := $(wildcard src/*.c)
OBJS := $(FILES:%=$(OBJDIR)/%.o) 
DEPS := $(OBJS:.o=.d)

LIBS := libmbedtls.a libmbedx509.a libmbedcrypto.a
OBJS += $(LIBS:%=$(OBJDIR)/%)

CFLAGS := -g -std=gnu11 -Imbedtls/include -Wall -Wno-unused-function -Werror
LDFLAGS := -O2 -no-pie -Wl,--gc-sections,--fatal-warnings

default: beatupserver

beatupserver: $(OBJS)
	@echo "[cc $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

beatupserver.%: $(OBJS)
	@echo "[cc $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

$(OBJDIR)/%.c.o: %.c $(OBJDIR)/libs.mk mbedtls/.git makefile
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

src/packets.c src/packets.h: src/packets.txt gen.c
	$(MAKE) .obj/gen.$(HOST)
	@echo "[gen $(notdir $@)]"
	./.obj/gen.$(HOST) "$<" "$@"

.obj/gen.$(HOST): gen.c
	@echo "[cc $(notdir $@)]"
	cc -std=c99 -no-pie "$<" -o "$@"

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
