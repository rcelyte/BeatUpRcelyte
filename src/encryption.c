#include "global.h"
#include "packets.h"
#include "encryption.h"
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <stdio.h>
#include <string.h>

static uint32_t min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

static inline void PRF_Hash(uint8_t *key, uint32_t key_len, uint8_t *seed, uint32_t seed_len, uint32_t *length) {
	uint8_t hash[MBEDTLS_MD_MAX_SIZE];
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	mbedtls_md_hmac(md_info, key, key_len, seed, *length, hash);
	uint32_t end = min(*length + mbedtls_md_get_size(md_info), seed_len);
	memcpy(&seed[*length], hash, end - *length);
	*length = end;
}

static void PRF(uint8_t *out, uint8_t *key, uint32_t key_len, const uint8_t *seed, uint8_t seed_len, uint8_t length) {
	uint32_t length2 = 0;
	uint8_t array[510];
	while(length2 < length) {
		memcpy(&array[length2], seed, seed_len);
		PRF_Hash(key, key_len, array, sizeof(array), &length2);
	}
	memcpy(out, array, length);
}

static uint8_t MakeSeed(uint8_t *out, const char *baseSeed, const uint8_t serverRandom[32], const uint8_t clientRandom[32]) {
	uint8_t len = strlen(baseSeed);
	memcpy(out, baseSeed, len);
	memcpy(&out[len], serverRandom, 32);
	memcpy(&out[len+32], clientRandom, 32);
	return len + 32 + 32;
}

bool EncryptionState_init(struct EncryptionState *state, const mbedtls_mpi *preMasterSecret, const uint8_t serverRandom[32], const uint8_t clientRandom[32], bool isClient) {
	uint8_t preMasterSecretBytes[mbedtls_mpi_size(preMasterSecret)];
	int32_t res = mbedtls_mpi_write_binary(preMasterSecret, preMasterSecretBytes, sizeof(preMasterSecretBytes));
	if(res) {
		uprintf("mbedtls_mpi_write_binary() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	uint8_t seed[80], sourceArray[192];
	PRF(sourceArray, preMasterSecretBytes, sizeof(preMasterSecretBytes), seed, MakeSeed(seed, "master secret", serverRandom, clientRandom), 48);
	PRF(sourceArray, sourceArray, 48, seed, MakeSeed(seed, "key expansion", serverRandom, clientRandom), 192);
	memcpy(isClient ? state->receiveKey : state->sendKey, sourceArray, 32);
	memcpy(isClient ? state->sendKey : state->receiveKey, &sourceArray[32], 32);
	memcpy(isClient ? state->receiveMacKey : state->sendMacKey, &sourceArray[64], 64);
	memcpy(isClient ? state->sendMacKey : state->receiveMacKey, &sourceArray[128], 64);
	state->outboundSequence = ~0u;
	state->receiveWindowEnd = 0;
	state->receiveWindow = 0;
	mbedtls_aes_init(&state->aes);
	state->initialized = true;
	return false;
}

void EncryptionState_free(struct EncryptionState *state) {
	if(!state->initialized)
		return;
	mbedtls_aes_free(&state->aes);
	state->initialized = false;
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

static inline void unchecked_u32_write(uint32_t data, uint8_t **pkt) {
	*(*pkt)++ = data & 255;
	*(*pkt)++ = data >> 8 & 255;
	*(*pkt)++ = data >> 16 & 255;
	*(*pkt)++ = data >> 24 & 255;
}

static bool AuthenticateMessage(struct EncryptionState *state, const uint8_t *data, uint8_t *data_end, uint8_t hash[10], uint32_t sequence) {
	unchecked_u32_write(sequence, &data_end);
	uint8_t expected[MBEDTLS_MD_MAX_SIZE];
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	mbedtls_md_hmac(md_info, state->receiveMacKey, sizeof(state->receiveMacKey), data, data_end - data, expected);
	if(memcmp(hash, expected, 10)) {
		uprintf("Packet hash mismatch\n");
		return true;
	}
	return false;
}

uint32_t EncryptionState_decrypt(struct EncryptionState *state, const uint8_t raw[static 1536], const uint8_t *raw_end, uint8_t out[restrict static 1536]) {
	struct PacketEncryptionLayer header;
	if(!pkt_read(&header, &raw, raw_end, PV_LEGACY_DEFAULT))
		return 0;
	size_t length = raw_end - raw;
	if(!header.encrypted) {
		memcpy(out, raw, length);
		return length;
	}
	if(!state->initialized || header.encrypted != 1 || length == 0 || length % 16 || InvalidSequenceNum(state, header.sequenceId))
		return 0;
	mbedtls_aes_setkey_dec(&state->aes, state->receiveKey, sizeof(state->receiveKey) * 8);
	mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_DECRYPT, length, header.iv, raw, out);
	
	uint8_t pad = out[length - 1];
	if(pad + 11u > length)
		return 0;
	length -= pad + 11u;
	uint8_t mac[10];
	memcpy(mac, &out[length], sizeof(mac));
	if(AuthenticateMessage(state, out, &out[length], mac, header.sequenceId))
		return 0;
	if(PutSequenceNum(state, header.sequenceId))
		return 0;
	return length;
}

static inline void unchecked_raw_write(const uint8_t *restrict data, uint8_t **pkt, size_t count) {
	memcpy(*pkt, data, count);
	*pkt += count;
}

static void SignMessage(struct EncryptionState *state, const uint8_t *data, uint8_t **data_end, uint32_t sequence) {
	uint8_t *sig_end = *data_end;
	unchecked_u32_write(sequence, &sig_end);
	uint8_t hash[MBEDTLS_MD_MAX_SIZE];
	mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), state->sendMacKey, sizeof(state->sendMacKey), data, sig_end - data, hash);
	unchecked_raw_write(hash, data_end, 10);
}

uint32_t EncryptionState_encrypt(struct EncryptionState *state, mbedtls_ctr_drbg_context *ctr_drbg, const uint8_t *restrict buf, uint32_t buf_len, uint8_t out[static 1536]) {
	const uint8_t *out_start = out, *out_end = &out[1536];
	if(state && state->initialized) {
		struct PacketEncryptionLayer header = {
			.encrypted = true,
			.sequenceId = ++state->outboundSequence,
		};
		mbedtls_ctr_drbg_random(ctr_drbg, header.iv, sizeof(header.iv));
		pkt_write(&header, &out, out_end, PV_LEGACY_DEFAULT);
		uint8_t unencrypted[1536], *unencrypted_end = unencrypted;
		unchecked_raw_write(buf, &unencrypted_end, buf_len);
		SignMessage(state, unencrypted, &unencrypted_end, header.sequenceId);
		uint8_t pad = 16 - ((unencrypted_end - unencrypted) & 15);
		memset(unencrypted_end, pad - 1, pad);
		size_t length = &unencrypted_end[pad] - unencrypted;
		mbedtls_aes_setkey_enc(&state->aes, state->sendKey, sizeof(state->sendKey) * 8);
		mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_ENCRYPT, length, header.iv, unencrypted, out);
		out += length;
	} else {
		pkt_write_c(&out, out_end, PV_LEGACY_DEFAULT, PacketEncryptionLayer, {
			.encrypted = false,
		});
		unchecked_raw_write(buf, &out, buf_len);
	}
	return out - out_start;
}
