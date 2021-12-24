#include <mbedtls/aes.h>
#include <mbedtls/bignum.h>
#include <mbedtls/md.h>

struct EncryptionState {
	mbedtls_aes_context aes;
	uint8_t sendKey[32];
	uint8_t receiveKey[32];
	uint8_t _sendMacKey[64];
	uint8_t _receiveMacKey[64];
	_Bool _hasReceivedSequenceNum;
	uint32_t _lastReceivedSequenceNum;
	_Bool _receivedSequenceNumBuffer[64];
};

_Bool EncryptionState_init(struct EncryptionState *state, const mbedtls_mpi *preMasterSecret, uint8_t serverRandom[32], uint8_t clientRandom[32], _Bool isClient);
_Bool EncryptionState_decrypt(struct EncryptionState *state, struct PacketEncryptionLayer header, uint8_t *data, uint32_t *length);
