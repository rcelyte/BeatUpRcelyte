#include "global.h"
#include "net.h"
#include "instance/eenet.h"
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <mbedtls/sha256.h>
#include <stdlib.h>
#include <errno.h>

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

struct EncryptionState *EncryptionState_init(const mbedtls_ssl_config *config, int32_t sendfd) {
	if(!mbedtls_md_info_from_type(MBEDTLS_MD_SHA256)) {
		uprintf("mbedtls_md_info_from_type(MBEDTLS_MD_SHA256) failed\n");
		return NULL;
	}
	struct EncryptionState *const state = calloc(1, sizeof(*state));
	if(state == NULL) {
		uprintf("alloc error\n");
		return NULL;
	}
	EncryptionState_ref(state);
	mbedtls_ssl_init(&state->dtls.ssl);
	mbedtls_aes_init(&state->bgnet.aes);
	state->tlsResetNeeded = true;
	state->dtls.sendfd = sendfd;

	if(mbedtls_ssl_setup(&state->dtls.ssl, config)) {
		EncryptionState_unref(state);
		return NULL;
	}
	mbedtls_ssl_set_mtu(&state->dtls.ssl, 1200); // TODO: actual MTU
	mbedtls_ssl_set_timer_cb(&state->dtls.ssl, &state->dtls.timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
	state->initialized = true;
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
	mbedtls_aes_free(&state->bgnet.aes);
	mbedtls_ssl_free(&state->dtls.ssl);
	free(state);
	return NULL;
}

bool EncryptionState_setKeys(struct EncryptionState *state, const mbedtls_mpi *secret, const struct Cookie32 random[static 2], bool client) {
	state->initialized = false;
	uint8_t sourceArray[510], scratch[510];
	size_t secret_len = mbedtls_mpi_size(secret);
	if(secret_len > sizeof(sourceArray)) {
		uprintf("secret too big\n");
		return true;
	}
	int32_t res = mbedtls_mpi_write_binary(secret, sourceArray, secret_len);
	if(res) {
		uprintf("mbedtls_mpi_write_binary() failed: %s\n", mbedtls_high_level_strerr(res));
		return true;
	}
	PRF(scratch, 48, sourceArray, secret_len, "master secret", random);
	PRF(sourceArray, 192, scratch, 48, "key expansion", random);
	memcpy(client ? state->bgnet.receiveKey : state->bgnet.sendKey, sourceArray, 32);
	memcpy(client ? state->bgnet.sendKey : state->bgnet.receiveKey, &sourceArray[32], 32);
	memcpy(client ? state->bgnet.receiveMacKey : state->bgnet.sendMacKey, &sourceArray[64], 64);
	memcpy(client ? state->bgnet.sendMacKey : state->bgnet.receiveMacKey, &sourceArray[128], 64);
	state->bgnet.outboundSequence = ~0u;
	state->bgnet.receiveWindowEnd = 0;
	state->bgnet.receiveWindow = 0;
	state->initialized = true;
	return false;
}

struct DtlsBio {
	int32_t sendfd;
	struct SS sendAddr;
	size_t recvBuf_len;
	const uint8_t *recvBuf;
};

static int DtlsBio_send(const struct DtlsBio *bio, const uint8_t *data, const size_t data_len) {
	const ssize_t ret = sendto(bio->sendfd, (const char*)data, data_len, 0, &bio->sendAddr.sa, bio->sendAddr.len);
	if(ret >= 0)
		return (int)ret;
	if(errno == EPIPE || errno == ECONNRESET)
		return -0x0050; // MBEDTLS_ERR_NET_CONN_RESET
	if(errno == EINTR)
		return MBEDTLS_ERR_SSL_WANT_WRITE;
	return -1;
}

static int DtlsBio_recv(struct DtlsBio *this, uint8_t *data, const size_t data_cap) {
	if(this->recvBuf_len == 0)
		return MBEDTLS_ERR_SSL_WANT_READ;
	const size_t data_len = (data_cap < this->recvBuf_len) ? data_cap : this->recvBuf_len;
	memcpy(data, this->recvBuf, data_len);
	this->recvBuf_len = 0;
	return (int)data_len;
}

static bool InvalidSequenceNum(struct EncryptionState *state, uint32_t sequenceNum) {
	if(sequenceNum > state->bgnet.receiveWindowEnd)
		return false; // The window will slide forward following successful decryption
	if(sequenceNum + 64 <= state->bgnet.receiveWindowEnd) // This will kill the connection upon unsigned overflow (200+ days of activity), but the client does it so we do too.
		return true; // Too old
	return (state->bgnet.receiveWindow >> (sequenceNum % 64)) & 1;
}

static bool PutSequenceNum(struct EncryptionState *state, uint32_t sequenceNum) {
	if(sequenceNum > state->bgnet.receiveWindowEnd) { // move window
		if(sequenceNum - state->bgnet.receiveWindowEnd < 64) {
			while(++state->bgnet.receiveWindowEnd < sequenceNum)
				state->bgnet.receiveWindow &= ~(1 << (state->bgnet.receiveWindowEnd % 64));
		} else {
			state->bgnet.receiveWindow = 0;
			state->bgnet.receiveWindowEnd = sequenceNum;
		}
	} else if(sequenceNum + 64 <= state->bgnet.receiveWindowEnd) {
		return true;
	} else if((state->bgnet.receiveWindow >> (sequenceNum % 64)) & 1) {
		return true;
	}
	state->bgnet.receiveWindow |= 1 << (sequenceNum % 64);
	return false;
}

#ifdef DEBUG
static bool SafeValidateHMAC(const uint8_t key[restrict static 64], const uint8_t *restrict data, size_t data_len, uint32_t sequence, uint8_t hash[restrict static 32]) {
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
#endif

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

uint32_t EncryptionState_decrypt(struct EncryptionState *state, const struct SS *sendAddr, const uint8_t raw[static 1536], const uint8_t *raw_end, uint8_t out[restrict static 1536]) {
	if(raw_end == raw)
		return 0;
	if(state->forceENet || *raw >= MBEDTLS_SSL_MSG_CHANGE_CIPHER_SPEC) {
		if(!state->initialized)
			return 0;
		state->encrypt |= (*raw == MBEDTLS_SSL_MSG_HANDSHAKE);
		if(!state->encrypt) {
			state->forceENet |= (*raw == EENET_CONNECT_BYTE);
			uint32_t length = (uint32_t)(raw_end - raw);
			memcpy(out, raw, length);
			return length;
		}
		if(state->tlsResetNeeded) {
			mbedtls_ssl_session_reset(&state->dtls.ssl);
			switch(sendAddr->ss.ss_family) {
				case AF_INET: mbedtls_ssl_set_client_transport_id(&state->dtls.ssl, (const uint8_t*)&sendAddr->in.sin_addr.s_addr, sizeof(sendAddr->in.sin_addr.s_addr)); break;
				case AF_INET6: mbedtls_ssl_set_client_transport_id(&state->dtls.ssl, sendAddr->in6.sin6_addr.s6_addr, sizeof(sendAddr->in6.sin6_addr.s6_addr)); break;
				default:;
			}
		}
		mbedtls_ssl_set_bio(&state->dtls.ssl, &(struct DtlsBio){
			.sendfd = state->dtls.sendfd,
			.sendAddr = *sendAddr,
			.recvBuf_len = (size_t)(raw_end - raw),
			.recvBuf = raw,
		}, (mbedtls_ssl_send_t*)DtlsBio_send, (mbedtls_ssl_recv_t*)DtlsBio_recv, NULL);
		const int res = mbedtls_ssl_read(&state->dtls.ssl, out, 1536);
		mbedtls_ssl_set_bio(&state->dtls.ssl, NULL, NULL, NULL, NULL);
		state->tlsResetNeeded = (res == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED);
		return (res < 0) ? 0 : (uint32_t)res;
	}
	struct PacketEncryptionLayer header;
	if(!pkt_read(&header, &raw, raw_end, (struct PacketContext){0}))
		return 0;
	uint32_t length = (uint32_t)(raw_end - raw);
	if(!header.encrypted) { // TODO: latch encryption (reject unencrypted packets following encrypted ones)
		memcpy(out, raw, length);
		return length;
	}
	if(!state->initialized || header.encrypted != 1 || length == 0 || length % 16 || InvalidSequenceNum(state, header.sequenceId))
		return 0;
	mbedtls_aes_setkey_dec(&state->bgnet.aes, state->bgnet.receiveKey, sizeof(state->bgnet.receiveKey) * 8);
	mbedtls_aes_crypt_cbc(&state->bgnet.aes, MBEDTLS_AES_DECRYPT, length, header.iv, raw, out);
	
	uint8_t pad = out[length - 1];
	if(pad + 11u > length)
		return 0;
	length -= pad + 11u;
	uint8_t mac[10], expected[32];
	memcpy(mac, &out[length], sizeof(mac));
	if(FastHMAC(state->bgnet.receiveMacKey, out, length, header.sequenceId, expected) || memcmp(mac, expected, sizeof(mac))) {
		uprintf("Hash validation failed\n");
		return 0;
	}
	if(PutSequenceNum(state, header.sequenceId))
		return 0;
	state->encrypt = true;
	return length;
}

uint32_t EncryptionState_encrypt(struct EncryptionState *state, const struct SS *sendAddr, mbedtls_ctr_drbg_context *ctr_drbg, enum EncryptMode mode, const uint8_t *restrict buf, uint32_t buf_len, uint8_t out[static 1536]) {
	switch(mode) {
		case EncryptMode_None: {
			uint32_t header_len = (uint32_t)pkt_write_c((uint8_t*[]){out}, &out[1536], (struct PacketContext){0}, PacketEncryptionLayer, {
				.encrypted = false,
			});
			memcpy(&out[header_len], buf, buf_len);
			return header_len + buf_len;
		}
		case EncryptMode_BGNet: {
			if(!state->initialized)
				break;
			struct PacketEncryptionLayer header = {
				.encrypted = true,
				.sequenceId = ++state->bgnet.outboundSequence,
			};
			mbedtls_ctr_drbg_random(ctr_drbg, header.iv, sizeof(header.iv));
			uint32_t header_len = (uint32_t)pkt_write(&header, (uint8_t*[]){out}, &out[1536], (struct PacketContext){0});
			uint8_t cap[16 + MBEDTLS_MD_MAX_SIZE], cap_len = buf_len & 15;
			uint32_t cut_len = buf_len - cap_len;
			memcpy(cap, &buf[cut_len], cap_len);
			if(FastHMAC(state->bgnet.sendMacKey, buf, buf_len, header.sequenceId, &cap[cap_len])) {
				uprintf("FastHMAC() failed\n");
				break;
			}
			cap_len += 10;
			uint8_t pad = 16 - ((buf_len + 10) & 15);
			memset(&cap[cap_len], pad - 1, pad); cap_len += pad;
			mbedtls_aes_setkey_enc(&state->bgnet.aes, state->bgnet.sendKey, sizeof(state->bgnet.sendKey) * 8);
			mbedtls_aes_crypt_cbc(&state->bgnet.aes, MBEDTLS_AES_ENCRYPT, cut_len, header.iv, buf, &out[header_len]);
			mbedtls_aes_crypt_cbc(&state->bgnet.aes, MBEDTLS_AES_ENCRYPT, cap_len, header.iv, cap, &out[header_len + cut_len]);
			return header_len + cut_len + cap_len;
		}
		case EncryptMode_DTLS: {
			if(!state->initialized)
				break;
			if(!state->encrypt) {
				memcpy(out, buf, buf_len);
				return buf_len;
			}
			mbedtls_ssl_set_bio(&state->dtls.ssl, &(struct DtlsBio){
				.sendfd = state->dtls.sendfd,
				.sendAddr = *sendAddr,
			}, (mbedtls_ssl_send_t*)DtlsBio_send, NULL, NULL);
			mbedtls_ssl_write(&state->dtls.ssl, buf, buf_len);
			mbedtls_ssl_set_bio(&state->dtls.ssl, NULL, NULL, NULL, NULL);
			return 0;
		}
	}
	return 0;
}
