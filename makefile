#!make
MAKEFLAGS += --no-print-directory -j$(command nproc 2>/dev/null || echo 2)
.SILENT:

OBJDIR := .obj/$(shell $(CC) -dumpmachine)
FILES := $(wildcard src/*.c src/*/*.c src/*/*/*.c)
OBJS := $(FILES:%=$(OBJDIR)/%.o) 
DEPS := $(OBJS:.o=.d)
EXT ?= $(shell uname -m)

LIBS := libmbedtls.a libmbedx509.a libmbedcrypto.a
OBJS += $(LIBS:%=$(OBJDIR)/%)

CFLAGS += -std=gnu11 -Imbedtls/include -Wall -Wno-unused-function -Werror
LDFLAGS += -Wl,--gc-sections,--fatal-warnings

server.$(EXT): $(OBJS)
	@echo "[linking $@]"
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

$(OBJDIR)/%.c.o: %.c mbedtls/.git makefile
	@echo "[compiling $(notdir $<)]"
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c "$<" -o "$@" -MMD -MP

$(OBJDIR)/libmbed%.a: mbedtls/.git
	@echo "[make $(notdir $@)]"
	mkdir -p "$@".build/
	cp -r mbedtls/3rdparty/ mbedtls/include/ mbedtls/library/ mbedtls/scripts/ "$@".build/
	make -C "$@".build/library CC=$(CC) AR=$(AR) $(notdir $@)
	mv "$@".build/library/$(notdir $@) "$@"
	rm -r "$@".build/

mbedtls/.git:
	git submodule update --init

$(OBJDIR)/libs.mk: libs.c makefile
	@mkdir -p "$(@D)"
	$(CC) -E libs.c -o "$@"

clean:
	@echo "[cleaning]"
	make -C mbedtls/library clean
	rm -rf .obj/
	rm -f server.$(EXT)

.PHONY: clean

include $(OBJDIR)/libs.mk
sinclude $(DEPS)
