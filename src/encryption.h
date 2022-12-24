#include <mbedtls/aes.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/md.h>

struct EncryptionState {
	mbedtls_aes_context aes;
	uint8_t sendKey[32];
	uint8_t receiveKey[32];
	uint8_t sendMacKey[64];
	uint8_t receiveMacKey[64];
	uint32_t outboundSequence;
	uint32_t receiveWindowEnd;
	uint64_t receiveWindow;
	bool initialized;
};

struct Cookie32;
bool EncryptionState_init(struct EncryptionState *state, const mbedtls_mpi *preMasterSecret, const struct Cookie32 *serverRandom, const struct Cookie32 *clientRandom, bool isClient);
void EncryptionState_free(struct EncryptionState *state);
uint32_t EncryptionState_decrypt(struct EncryptionState *state, const uint8_t raw[static 1536], const uint8_t *raw_end, uint8_t out[restrict static 1536]);
uint32_t EncryptionState_encrypt(struct EncryptionState *state, mbedtls_ctr_drbg_context *ctr_drbg, const uint8_t *restrict buf, uint32_t buf_len, uint8_t out[static 1536]);
