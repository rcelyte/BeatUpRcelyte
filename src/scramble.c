#include "global.h"
#include "scramble.h"
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <stdio.h>
#include <stdlib.h>

/*static ServerCode scramble_seed = 0;
static uint8_t scramble_order[25] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};*/

[[maybe_unused]] static void u8swap(uint8_t *a, uint8_t *b) {
	uint8_t c = *a;
	*a = *b, *b = c;
}

ServerCode StringToServerCode(const char *in, uint32_t len) {
	static const uint8_t readTable[128] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,27,28,29,30,31,32,33,34,35,36,1,1,1,1,1,1,1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	ServerCode out = 0;
	if(len <= 5)
		for(uint32_t i = 0, fac = 1; i < len; ++i, fac *= 36)
			out += readTable[in[i] & 127] * fac;
	return scramble_decode(out);
}

char *ServerCodeToString(char *out, ServerCode in) {
	char *s = out;
	for(in = scramble_encode(in); in; in /= 36)
		*s++ = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[--in % 36];
	*s = 0;
	return out;
}

void scramble_init() {}
ServerCode scramble_encode(ServerCode in) {return in;}
ServerCode scramble_decode(ServerCode in) {return in;}

/*void scramble_init() {
	uint64_t seed[2];
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	int32_t err = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t*)u8"M@$73RS€RV3R", 14);
	if(err)
		uprintf("mbedtls_ctr_drbg_seed() failed: %s\n", mbedtls_high_level_strerr(err));
	mbedtls_ctr_drbg_random(&ctr_drbg, (uint8_t*)seed, sizeof(seed));
	mbedtls_entropy_free(&entropy);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	scramble_seed = seed[0] & ((1 << 25) - 1);
	u8swap(&scramble_order[0], &scramble_order[(seed[0] >> 25) % 25]);
	for(uint8_t i = 1; i < 24; ++i) {
		u8swap(&scramble_order[i], &scramble_order[i + seed[1] % (25 - i)]);
		seed[1] /= (25 - i);
	}
}

ServerCode scramble_encode(ServerCode in) {
	if(in >= 1 << 25) {
		uprintf("SCRAMBLE FAILED\n");
		abort();
	}
	ServerCode out = scramble_seed;
	for(uint8_t i = 0; i < 25; ++i)
		out ^= (((in >> i) & 1) << scramble_order[i]);
	return out;
}

ServerCode scramble_decode(ServerCode in) {
	in = in & ((1 << 25) - 1);
	ServerCode out = 0;
	for(uint8_t i = 0; i < 25; ++i)
		out |= (((in >> scramble_order[i]) & 1) << i);
	return out ^ scramble_seed;
}*/
