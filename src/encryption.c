#include "global.h"
#include "packets.h"
#include "encryption.h"
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <mbedtls/sha256.h>
#include <stdlib.h>

static uint32_t min32(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

static inline uint32_t PRF_Hash(uint8_t *key, size_t key_len, uint8_t *seed, uint32_t seed_cap, uint32_t length) {
	uint8_t hash[MBEDTLS_MD_MAX_SIZE];
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	mbedtls_md_hmac(md_info, key, key_len, seed, length, hash);
	uint32_t end = min32(length + mbedtls_md_get_size(md_info), seed_cap);
	memcpy(&seed[length], hash, end - length);
	return end;
}

static void PRF(uint8_t out[restrict static 510], uint8_t out_len, uint8_t *key, size_t key_len, const char baseSeed[restrict static 13], const struct Cookie32 random[static 2]) {
	uint8_t seed[13 + sizeof(struct Cookie32[2])];
	memcpy(seed, baseSeed, 13);
	memcpy(&seed[13], random, sizeof(struct Cookie32[2]));
	for(uint32_t length2 = 0; length2 < out_len;) {
		memcpy(&out[length2], seed, sizeof(seed));
		length2 = PRF_Hash(key, key_len, out, 510, length2);
	}
}

struct EncryptionState *EncryptionState_init(const mbedtls_mpi *secret, const struct Cookie32 random[static 2], bool client) {
	if(!mbedtls_md_info_from_type(MBEDTLS_MD_SHA256)) {
		uprintf("mbedtls_md_info_from_type(MBEDTLS_MD_SHA256) failed\n");
		return NULL;
	}
	uint8_t sourceArray[510], scratch[510];
	size_t secret_len = mbedtls_mpi_size(secret);
	if(secret_len > sizeof(sourceArray)) {
		uprintf("secret too big\n");
		return NULL;
	}
	int32_t res = mbedtls_mpi_write_binary(secret, sourceArray, secret_len);
	if(res) {
		uprintf("mbedtls_mpi_write_binary() failed: %s\n", mbedtls_high_level_strerr(res));
		return NULL;
	}
	struct EncryptionState *const state = calloc(1, sizeof(*state));
	if(state == NULL) {
		uprintf("alloc error\n");
		return NULL;
	}
	PRF(scratch, 48, sourceArray, secret_len, "master secret", random);
	PRF(sourceArray, 192, scratch, 48, "key expansion", random);
	memcpy(client ? state->receiveKey : state->sendKey, sourceArray, 32);
	memcpy(client ? state->sendKey : state->receiveKey, &sourceArray[32], 32);
	memcpy(client ? state->receiveMacKey : state->sendMacKey, &sourceArray[64], 64);
	memcpy(client ? state->sendMacKey : state->receiveMacKey, &sourceArray[128], 64);
	state->outboundSequence = ~0u;
	state->receiveWindowEnd = 0;
	state->receiveWindow = 0;
	mbedtls_aes_init(&state->aes);
	EncryptionState_ref(state);
	return state;
}

struct EncryptionState *EncryptionState_ref(struct EncryptionState *state) {
	if(state != NULL)
		++state->refCount;
	return state;
}
struct EncryptionState *EncryptionState_unref(struct EncryptionState *state) {
	if(state == NULL || --state->refCount)
		return state;
	mbedtls_aes_free(&state->aes);
	free(state);
	return NULL;
}

static bool InvalidSequenceNum(struct EncryptionState *state, uint32_t sequenceNum) {
	if(sequenceNum > state->receiveWindowEnd)
		return false; // The window will slide forward following successful decryption
	if(sequenceNum + 64 <= state->receiveWindowEnd) // This will kill the connection upon unsigned overflow (200+ days of activity), but the client does it so we do too.
		return true; // Too old
	return (state->receiveWindow >> (sequenceNum % 64)) & 1;
}

static bool PutSequenceNum(struct EncryptionState *state, uint32_t sequenceNum) {
	if(sequenceNum > state->receiveWindowEnd) { // move window
		if(sequenceNum - state->receiveWindowEnd < 64) {
			while(++state->receiveWindowEnd < sequenceNum)
				state->receiveWindow &= ~(1 << (state->receiveWindowEnd % 64));
		} else {
			state->receiveWindow = 0;
			state->receiveWindowEnd = sequenceNum;
		}
	} else if(sequenceNum + 64 <= state->receiveWindowEnd) {
		return true;
	} else if((state->receiveWindow >> (sequenceNum % 64)) & 1) {
		return true;
	}
	state->receiveWindow |= 1 << (sequenceNum % 64);
	return false;
}

[[maybe_unused]] static bool SafeValidateHMAC(const uint8_t key[restrict static 64], const uint8_t *restrict data, size_t data_len, uint32_t sequence, uint8_t hash[restrict static 32]) {
	uint8_t comp[data_len + 4];
	memcpy(comp, data, data_len);
	comp[data_len++] = sequence & 255;
	comp[data_len++] = sequence >> 8 & 255;
	comp[data_len++] = sequence >> 16 & 255;
	comp[data_len++] = sequence >> 24 & 255;

	uint8_t valid[MBEDTLS_MD_MAX_SIZE];
	if(mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), key, 64, comp, data_len, valid) != 0)
		return true;
	if(memcmp(hash, valid, 32)) {
		uprintf("!!! INTERNAL VALIDATION ERROR !!!\n");
		return true;
	}
	return false;
}

static bool FastHMAC(const uint8_t key[restrict static 64], const uint8_t *restrict data, size_t data_len, uint32_t sequence, uint8_t hash_out[restrict static 32]) {
	uint8_t sequenceLE[4] = {sequence & 255, sequence >> 8 & 255, sequence >> 16 & 255, sequence >> 24 & 255};

	bool err = false;
	mbedtls_sha256_context ctx;
	mbedtls_sha256_init(&ctx);

	uint8_t pad[64], temp[32];
	for(uint_fast8_t i = 0; i < sizeof(pad); ++i)
		pad[i] = 0x36 ^ key[i];
	err |= (mbedtls_sha256_starts(&ctx, false) != 0);
	err |= (mbedtls_sha256_update(&ctx, pad, sizeof(pad)) != 0);
	err |= (mbedtls_sha256_update(&ctx, data, data_len) != 0);
	err |= (mbedtls_sha256_update(&ctx, sequenceLE, sizeof(sequenceLE)) != 0);
	err |= (mbedtls_sha256_finish(&ctx, temp) != 0);

	for(uint_fast8_t i = 0; i < sizeof(pad); ++i)
		pad[i] ^= 0x6a;
	err |= (mbedtls_sha256_starts(&ctx, false) != 0);
	err |= (mbedtls_sha256_update(&ctx, pad, sizeof(pad)) != 0);
	err |= (mbedtls_sha256_update(&ctx, temp, sizeof(temp)) != 0);
	err |= (mbedtls_sha256_finish(&ctx, hash_out) != 0);
	mbedtls_sha256_free(&ctx);

	#ifdef DEBUG
	err |= SafeValidateHMAC(key, data, data_len, sequence, hash_out);
	#endif
	return err;
}

uint32_t EncryptionState_decrypt(struct EncryptionState *state, const uint8_t raw[static 1536], const uint8_t *raw_end, uint8_t out[restrict static 1536]) {
	struct PacketEncryptionLayer header;
	if(!pkt_read(&header, &raw, raw_end, PV_LEGACY_DEFAULT))
		return 0;
	uint32_t length = (uint32_t)(raw_end - raw);
	if(!header.encrypted) {
		memcpy(out, raw, length);
		return length;
	}
	if(state == NULL || header.encrypted != 1 || length == 0 || length % 16 || InvalidSequenceNum(state, header.sequenceId))
		return 0;
	mbedtls_aes_setkey_dec(&state->aes, state->receiveKey, sizeof(state->receiveKey) * 8);
	mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_DECRYPT, length, header.iv, raw, out);
	
	uint8_t pad = out[length - 1];
	if(pad + 11u > length)
		return 0;
	length -= pad + 11u;
	uint8_t mac[10], expected[32];
	memcpy(mac, &out[length], sizeof(mac));
	if(FastHMAC(state->receiveMacKey, out, length, header.sequenceId, expected) || memcmp(mac, expected, sizeof(mac))) {
		uprintf("Hash validation failed\n");
		return 0;
	}
	if(PutSequenceNum(state, header.sequenceId))
		return 0;
	return length;
}

uint32_t EncryptionState_encrypt(struct EncryptionState *state, mbedtls_ctr_drbg_context *ctr_drbg, const uint8_t *restrict buf, uint32_t buf_len, uint8_t out[static 1536]) {
	if(state != NULL) {
		struct PacketEncryptionLayer header = {
			.encrypted = true,
			.sequenceId = ++state->outboundSequence,
		};
		mbedtls_ctr_drbg_random(ctr_drbg, header.iv, sizeof(header.iv));
		uint32_t header_len = (uint32_t)pkt_write(&header, (uint8_t*[]){out}, &out[1536], PV_LEGACY_DEFAULT);
		uint8_t cap[16 + MBEDTLS_MD_MAX_SIZE], cap_len = buf_len & 15;
		uint32_t cut_len = buf_len - cap_len;
		memcpy(cap, &buf[cut_len], cap_len);
		if(FastHMAC(state->sendMacKey, buf, buf_len, header.sequenceId, &cap[cap_len])) {
			uprintf("FastHMAC() failed\n");
			return 0;
		}
		cap_len += 10;
		uint8_t pad = 16 - ((buf_len + 10) & 15);
		memset(&cap[cap_len], pad - 1, pad); cap_len += pad;
		mbedtls_aes_setkey_enc(&state->aes, state->sendKey, sizeof(state->sendKey) * 8);
		mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_ENCRYPT, cut_len, header.iv, buf, &out[header_len]);
		mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_ENCRYPT, cap_len, header.iv, cap, &out[header_len + cut_len]);
		return header_len + cut_len + cap_len;
	}
	uint32_t header_len = (uint32_t)pkt_write_c((uint8_t*[]){out}, &out[1536], PV_LEGACY_DEFAULT, PacketEncryptionLayer, {
		.encrypted = false,
	});
	memcpy(&out[header_len], buf, buf_len);
	return header_len + buf_len;
}
