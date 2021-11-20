.SILENT:

FILES := $(wildcard src/*.c src/*/*.c src/*/*/*.c)
OBJS := $(FILES:%=.obj/%.o)
DEPS := $(OBJS:.o=.d)
EXT := $(shell uname -m)

server.$(EXT): $(OBJS)
	@echo "[linking $@]"
	$(CC) $(OBJS) -Wl,--gc-sections,--fatal-warnings -pthread -lmbedcrypto -lmbedtls -lmbedx509 -o "$@"

.obj/%.c.o: %.c
	@echo "[compiling $(notdir $<)]"
	@mkdir -p "$(@D)"
	$(CC) -std=gnu11 -Wall -Wno-unused-function -Werror -c "$<" -o "$@" -MMD -MP

clean:
	@echo "[cleaning]"
	rm -r .obj/
	rm server.$(EXT)

.PHONY: clean

sinclude $(DEPS)
