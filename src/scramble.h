#include "packets.h"

ServerCode StringToServerCode(const char *in, uint32_t len);
char *ServerCodeToString(char *out, ServerCode in);

void scramble_init();
ServerCode scramble_encode(ServerCode in);
ServerCode scramble_decode(ServerCode in);
