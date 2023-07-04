#pragma once
#include "packets.h"
#include <mbedtls/bignum.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>
#include <mbedtls/timing.h>

enum EncryptMode {
	EncryptMode_None,
	EncryptMode_BGNet,
	EncryptMode_DTLS,
};

struct EncryptionState {
	uint32_t refCount;
	struct {
		bool initialized, resetNeeded;
		int32_t sendfd;
		mbedtls_ssl_context ssl;
		mbedtls_timing_delay_context timer;
	} dtls;
	struct {
		bool initialized;
		mbedtls_aes_context aes;
		uint8_t sendKey[32];
		uint8_t receiveKey[32];
		uint8_t sendMacKey[64];
		uint8_t receiveMacKey[64];
		uint32_t outboundSequence;
		uint32_t receiveWindowEnd;
		uint64_t receiveWindow;
	} bgnet;
};

struct Cookie32;
struct SS;
struct EncryptionState *EncryptionState_init(const mbedtls_ssl_config *config, int32_t sendfd);
struct EncryptionState *EncryptionState_ref(struct EncryptionState *state);
struct EncryptionState *EncryptionState_unref(struct EncryptionState *state);
bool EncryptionState_setKeys(struct EncryptionState *state, const mbedtls_mpi *secret, const struct Cookie32 random[static 2], bool client);
uint32_t EncryptionState_decrypt(struct EncryptionState *state, const struct SS *sendAddr, const uint8_t raw[static 1536], const uint8_t *raw_end, uint8_t out[restrict static 1536]);
uint32_t EncryptionState_encrypt(struct EncryptionState *state, const struct SS *sendAddr, mbedtls_ctr_drbg_context *ctr_drbg, enum EncryptMode mode, const uint8_t *restrict buf, uint32_t buf_len, uint8_t out[static 1536]);
