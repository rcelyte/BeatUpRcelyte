#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <stdbool.h>
#define CONFIG_STRING_LENGTH 4096

struct Config {
	union {
		mbedtls_pk_context keys[2];
		struct {
			mbedtls_pk_context masterKey, statusKey;
		};
	};
	union {
		mbedtls_x509_crt certs[2];
		struct {
			mbedtls_x509_crt masterCert, statusCert;
		};
	};
	uint8_t wireKey_len;
	uint8_t wireKey[32];
	uint16_t instanceCount, masterPort, statusPort;
	char instanceAddress[2][CONFIG_STRING_LENGTH];
	char instanceParent[CONFIG_STRING_LENGTH];
	char statusAddress[CONFIG_STRING_LENGTH];
	char statusPath[CONFIG_STRING_LENGTH];
};

bool config_load(struct Config *out, const char *path);
void config_free(struct Config *cfg);
// struct Config config_default();
