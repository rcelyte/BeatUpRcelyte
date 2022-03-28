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
	int32_t lastSentSequenceNum;
	_Bool hasReceivedSequenceNum;
	uint32_t lastReceivedSequenceNum;
	_Bool receivedSequenceNumBuffer[64];
	_Bool initialized;
};

_Bool EncryptionState_init(struct EncryptionState *state, const mbedtls_mpi *preMasterSecret, const uint8_t serverRandom[32], const uint8_t clientRandom[32], _Bool isClient);
void EncryptionState_free(struct EncryptionState *state);
_Bool EncryptionState_decrypt(struct EncryptionState *state, struct PacketEncryptionLayer header, uint8_t *data, uint32_t *length);
_Bool EncryptionState_encrypt(struct EncryptionState *state, struct PacketEncryptionLayer *header, mbedtls_ctr_drbg_context *ctr_drbg, const uint8_t *buf, uint32_t buf_len, uint8_t *out, uint32_t *out_len);
