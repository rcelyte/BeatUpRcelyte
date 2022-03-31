#include "log.h"
#include "packets.h"
#include "encryption.h"
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <stdio.h>
#include <string.h>

static uint32_t min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

static void PRF_Hash(uint8_t *key, uint32_t key_len, uint8_t *seed, uint32_t seed_len, uint32_t *length) {
	uint8_t hash[MBEDTLS_MD_MAX_SIZE];
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	mbedtls_md_hmac(md_info, key, key_len, seed, *length, hash);
	uint32_t end = min(*length + mbedtls_md_get_size(md_info), seed_len);
	memcpy(&seed[*length], hash, end - *length);
	*length = end;
}

static void PRF(uint8_t *out, uint8_t *key, uint32_t key_len, const uint8_t *seed, uint32_t seed_len, uint32_t length) {
	uint32_t length2 = 0;
	uint8_t array[length + seed_len];
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

_Bool EncryptionState_init(struct EncryptionState *state, const mbedtls_mpi *preMasterSecret, const uint8_t serverRandom[32], const uint8_t clientRandom[32], _Bool isClient) {
	uint8_t preMasterSecretBytes[mbedtls_mpi_size(preMasterSecret)];
	int32_t err = mbedtls_mpi_write_binary(preMasterSecret, preMasterSecretBytes, sizeof(preMasterSecretBytes));
	if(err) {
		uprintf("mbedtls_mpi_write_binary() failed: %s\n", mbedtls_high_level_strerr(err));
		return 1;
	}
	uint8_t seed[80], sourceArray[192];
	PRF(sourceArray, preMasterSecretBytes, sizeof(preMasterSecretBytes), seed, MakeSeed(seed, "master secret", serverRandom, clientRandom), 48);
	PRF(sourceArray, sourceArray, 48, seed, MakeSeed(seed, "key expansion", serverRandom, clientRandom), 192);
	memcpy(isClient ? state->receiveKey : state->sendKey, sourceArray, 32);
	memcpy(isClient ? state->sendKey : state->receiveKey, &sourceArray[32], 32);
	memcpy(isClient ? state->receiveMacKey : state->sendMacKey, &sourceArray[64], 64);
	memcpy(isClient ? state->sendMacKey : state->receiveMacKey, &sourceArray[128], 64);
	state->lastSentSequenceNum = -1;
	state->hasReceivedSequenceNum = 0;
	memset(state->receivedSequenceNumBuffer, 0, sizeof(state->receivedSequenceNumBuffer));
	mbedtls_aes_init(&state->aes);
	state->initialized = 1;
	return 0;
}

void EncryptionState_free(struct EncryptionState *state) {
	if(state->initialized) {
		mbedtls_aes_free(&state->aes);
		state->initialized = 0;
	}
}

static _Bool IsInvalidSequenceNum(struct EncryptionState *state, uint32_t sequenceNum) {
	if(!state->hasReceivedSequenceNum)
		return 0;
	if(sequenceNum > state->lastReceivedSequenceNum)
		return 0;
	if(sequenceNum + 64 <= state->lastReceivedSequenceNum)
		return 1;
	return state->receivedSequenceNumBuffer[sequenceNum % 64];
}

static _Bool PutSequenceNum(struct EncryptionState *state, uint32_t sequenceNum) {
	if(!state->hasReceivedSequenceNum) {
		state->hasReceivedSequenceNum = 1;
		state->lastReceivedSequenceNum = sequenceNum;
	} else if(sequenceNum > state->lastReceivedSequenceNum) {
		uint32_t delta = sequenceNum - state->lastReceivedSequenceNum;
		if(delta >= 64)
			memset(state->receivedSequenceNumBuffer, 0, sizeof(state->receivedSequenceNumBuffer));
		else
			for(uint32_t i = 1; i < delta; i++)
				state->receivedSequenceNumBuffer[(state->lastReceivedSequenceNum + i) % 64] = 0;
		state->lastReceivedSequenceNum = sequenceNum;
	} else {
		if(sequenceNum + 64 <= state->lastReceivedSequenceNum)
			return 1;
		if(state->receivedSequenceNumBuffer[sequenceNum % 64u])
			return 1;
	}
	state->receivedSequenceNumBuffer[sequenceNum % 64] = 1;
	return 0;
}

static _Bool ValidateReceivedMac(struct EncryptionState *state, uint8_t *data, uint32_t length, uint8_t mac[10]) {
	uint8_t hash[MBEDTLS_MD_MAX_SIZE];
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	mbedtls_md_hmac(md_info, state->receiveMacKey, sizeof(state->receiveMacKey), data, length, hash);
	if(memcmp(mac, hash, 10)) {
		uprintf("Packet hash mismatch: expected %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx, got %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n", hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7], mac[8], mac[9]);
		return 1;
	}
	return 0;
}

_Bool EncryptionState_decrypt(struct EncryptionState *state, struct PacketEncryptionLayer header, uint8_t *data, uint32_t *length) {
	if(!state->initialized)
		return 1;
	if(*length == 0 || *length % 16)
		return 1;
	if(IsInvalidSequenceNum(state, header.sequenceId))
		return 1;
	uint8_t decrypted[*length];
	mbedtls_aes_setkey_dec(&state->aes, state->receiveKey, sizeof(state->receiveKey) * 8);
	mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_DECRYPT, *length, header.iv, data, decrypted);
	
	uint8_t pad = decrypted[*length - 1];
	if(pad + 10 + 1 > *length)
		return 1;
	*length -= pad + 10 + 1;
	uint8_t ref[10];
	memcpy(ref, &decrypted[*length], sizeof(ref));
	decrypted[*length] = header.sequenceId;
	decrypted[*length+1] = header.sequenceId >> 8;
	decrypted[*length+2] = header.sequenceId >> 16;
	decrypted[*length+3] = header.sequenceId >> 24;
	if(ValidateReceivedMac(state, decrypted, *length + 4, ref))
		return 1;
	if(PutSequenceNum(state, header.sequenceId))
		return 1;
	memcpy(data, decrypted, *length);
	return 0;
}

static uint32_t GetNextSentSequenceNum(struct EncryptionState *state) {
	return (uint32_t)++state->lastSentSequenceNum;
}

static void ComputeSendMac(struct EncryptionState *state, const uint8_t *data, uint32_t length, uint8_t *out) {
	uint8_t hash[MBEDTLS_MD_MAX_SIZE];
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	mbedtls_md_hmac(md_info, state->sendMacKey, sizeof(state->sendMacKey), data, length, hash);
	memcpy(out, hash, 10);
}

_Bool EncryptionState_encrypt(struct EncryptionState *state, struct PacketEncryptionLayer *header, mbedtls_ctr_drbg_context *ctr_drbg, const uint8_t *buf, uint32_t buf_len, uint8_t *out, uint32_t *out_len) {
	uint32_t length = buf_len + 10;
	uint8_t pad = 16 - (length & 15);
	header->encrypted = 1;
	header->sequenceId = GetNextSentSequenceNum(state);
	mbedtls_ctr_drbg_random(ctr_drbg, header->iv, sizeof(header->iv));
	uint8_t unencrypted[length + pad];
	{
		memcpy(unencrypted, buf, buf_len);
		unencrypted[buf_len] = header->sequenceId;
		unencrypted[buf_len+1] = header->sequenceId >> 8;
		unencrypted[buf_len+2] = header->sequenceId >> 16;
		unencrypted[buf_len+3] = header->sequenceId >> 24;
		ComputeSendMac(state, unencrypted, buf_len + 4, &unencrypted[buf_len]);
		memset(&unencrypted[length], pad - 1, pad);
	}
	*out_len = length + pad;
	uint8_t iv[sizeof(header->iv)];
	memcpy(iv, header->iv, sizeof(header->iv));
	mbedtls_aes_setkey_enc(&state->aes, state->sendKey, sizeof(state->sendKey) * 8);
	mbedtls_aes_crypt_cbc(&state->aes, MBEDTLS_AES_ENCRYPT, *out_len, iv, unencrypted, out);
	return 0;
}
