#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>

struct Config {
	mbedtls_pk_context keys[2];
	mbedtls_x509_crt certs[2];
	uint16_t master_port, status_port;
	char host_domain[4096], host_domainIPv4[4096], status_domain[4096], status_path[4096];
	_Bool status_tls;
};

_Bool config_load(struct Config *out, const char *path);
void config_free(struct Config *cfg);
// struct Config config_default();
