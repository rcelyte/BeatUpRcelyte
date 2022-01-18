#include "scramble.h"
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <stdio.h>
#include <stdlib.h>

/*static ServerCode scramble_seed = 0;
static uint8_t scramble_order[25] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};*/

static void u8swap(uint8_t *a, uint8_t *b) {
	uint8_t c = *a;
	*a = *b, *b = c;
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
		fprintf(stderr, "mbedtls_ctr_drbg_seed() failed: %s\n", mbedtls_high_level_strerr(err));
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
		fprintf(stderr, "SCRAMBLE FAILED\n");
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
