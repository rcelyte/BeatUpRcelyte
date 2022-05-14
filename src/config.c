#include "log.h"
#include "config.h"
#include "json.h"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <stdlib.h>
#include <errno.h>

static bool load_cert(const char *cert, uint32_t cert_len, mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_pk_context *key, mbedtls_x509_crt *out) {
	int32_t err = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
	char buf[4096];
	if(cert_len == 0) {
		mbedtls_x509write_cert cert_ctx;
		mbedtls_x509write_crt_init(&cert_ctx);
		mbedtls_mpi serial;
		mbedtls_mpi_init(&serial);
		mbedtls_mpi_read_string(&serial, 10, "1");
		mbedtls_x509write_crt_set_serial(&cert_ctx, &serial);
		mbedtls_mpi_free(&serial);
		mbedtls_x509write_crt_set_validity(&cert_ctx, "20010101000000", "20301231235959");
		mbedtls_x509write_crt_set_issuer_name(&cert_ctx, "CN=CA,O=mbed TLS,C=UK");
		mbedtls_x509write_crt_set_subject_name(&cert_ctx, "CN=CA,O=mbed TLS,C=UK");
		mbedtls_x509write_crt_set_subject_key(&cert_ctx, key);
		mbedtls_x509write_crt_set_issuer_key(&cert_ctx, key);
		mbedtls_x509write_crt_set_md_alg(&cert_ctx, MBEDTLS_MD_SHA256);
		mbedtls_x509write_crt_set_basic_constraints(&cert_ctx, 0, -1);
		mbedtls_x509write_crt_set_subject_key_identifier(&cert_ctx);
		mbedtls_x509write_crt_set_authority_key_identifier(&cert_ctx);
		int32_t len = mbedtls_x509write_crt_der(&cert_ctx, (uint8_t*)buf, sizeof(buf), mbedtls_ctr_drbg_random, ctr_drbg);
		mbedtls_x509write_crt_free(&cert_ctx);
		err = mbedtls_x509_crt_parse_der(out, (uint8_t*)&buf[sizeof(buf) - len], len);
		sprintf(buf, "generated certificate");
	} else if(cert_len > 52 && strncmp(cert, "-----BEGIN CERTIFICATE-----", 27) == 0) {
		sprintf(buf, "inline certificate");
		err = mbedtls_x509_crt_parse(out, (uint8_t*)cert, cert_len);
	} else {
		if(cert_len >= sizeof(buf)) {
			uprintf("Failed to load %.*s: Path too long\n", cert_len, cert);
			return 1;
		}
		sprintf(buf, "%.*s", cert_len, cert);
		err = mbedtls_x509_crt_parse_file(out, buf);
	}
	if(err) {
		uprintf("Failed to load %s: %s\n", buf, mbedtls_high_level_strerr(err));
		return 1;
	}
	return 0;
}

static bool load_key(const char *key, uint32_t key_len, mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_pk_context *out) {
	bool res = 0;
	int32_t err = 0;
	char name[4096];
	if(key_len == 0) {
		mbedtls_pk_setup(out, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
		mbedtls_rsa_context *rsa = mbedtls_pk_rsa(*out);
		res = mbedtls_rsa_gen_key(rsa, mbedtls_ctr_drbg_random, ctr_drbg, 2048, 65537) != 0;
	} else if(key_len > 52 && strncmp(key, "-----BEGIN ", 11) == 0) {
		sprintf(name, "inline certificate");
		err = mbedtls_pk_parse_key(out, (uint8_t*)key, key_len, NULL, 0, mbedtls_ctr_drbg_random, ctr_drbg);
	} else if(key_len < sizeof(name)) {
		sprintf(name, "%.*s", key_len, key);
		err = mbedtls_pk_parse_keyfile(out, name, NULL, mbedtls_ctr_drbg_random, ctr_drbg);
	} else {
		uprintf("Failed to load %.*s: Path too long\n", key_len, key);
		res = 1;
	}
	if(err)
		uprintf("Failed to load %s: %s\n", name, mbedtls_high_level_strerr(err));
	return res || err != 0;
}

bool config_load(struct Config *out, const char *path) {
	bool res = 1;
	char *master_cert = NULL, *master_key = NULL, *status_cert = NULL, *status_key = NULL;
	uint32_t master_cert_len = 0, master_key_len = 0, status_cert_len = 0, status_key_len = 0;

	mbedtls_x509_crt_init(&out->certs[0]);
	mbedtls_x509_crt_init(&out->certs[1]);
	mbedtls_pk_init(&out->keys[0]);
	mbedtls_pk_init(&out->keys[1]);

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	if(mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t*)"password", 8) != 0)
		goto fail;

	char config_json[524288];
	FILE *f = fopen(path, "r");
	if(f == NULL && errno == ENOENT) {
		FILE *def = fopen(path, "w");
		if(!def) {
			uprintf("Failed to write default config to %s: %s\n", path, strerror(errno));
			goto fail;
		}
		uprintf("Writing default config to %s\n", path);
		#ifdef WINDOWS
		fprintf(def, "{\r\n\t\"HostName\": \"\",\r\n\t\"HostCert\": \"cert.pem\",\r\n\t\"HostKey\": \"key.pem\",\r\n\t\"StatusUri\": \"http://localhost/status\"\r\n}\r\n");
		#else
		fprintf(def, "{\n\t\"HostName\": \"\",\n\t\"HostCert\": \"cert.pem\",\n\t\"HostKey\": \"key.pem\",\n\t\"StatusUri\": \"http://localhost/status\"\n}\n");
		#endif
		fclose(def);
		f = fopen(path, "r");
	}
	if(!f) {
		uprintf("Failed to open %s: %s\n", path, strerror(errno));
		goto fail;
	}
	fseek(f, 0, SEEK_END);
	size_t flen = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(flen >= sizeof(config_json)) {
		uprintf("Failed to read %s: File too large\n", path);
		fclose(f);
		goto fail;
	}
	if(fread(config_json, 1, flen, f) != flen) {
		uprintf("Failed to read %s\n", path);
		fclose(f);
		goto fail;
	}
	config_json[flen] = 0;
	fclose(f);

	out->master_port = 2328;
	out->status_port = 80;
	*out->host_domain = 0;
	*out->host_domainIPv4 = 0;
	*out->status_domain = 0;
	sprintf(out->status_path, "/");
	out->status_tls = 0;

	char *key, *it = config_json;
	uint32_t key_len;
	while(json_iter_object(&it, &key, &key_len)) {
		/*if(key_len == 6 && memcmp(key, "master", 6) == 0)
			it = json_config_host(it, &master);
		else if(key_len == 6 && memcmp(key, "status", 6) == 0)
			it = json_config_host(it, &status);
		else
			it = json_skip_value(it);*/
		#define IFEQ(str) if(key_len == sizeof(str) - 1 && memcmp(key, str, sizeof(str) - 1) == 0)
		IFEQ("HostName") {
			char *domain;
			uint32_t domain_len = 0;
			it = json_get_string(it, &domain, &domain_len);
			if(domain_len >= lengthof(out->host_domain)) {
				uprintf("Error parsing config value \"HostName\": name too long\n");
				continue;
			}
			sprintf(out->host_domain, "%.*s", domain_len, domain);
		} else IFEQ("HostName_IPv4") {
			char *domain;
			uint32_t domain_len = 0;
			it = json_get_string(it, &domain, &domain_len);
			if(domain_len >= lengthof(out->host_domainIPv4)) {
				uprintf("Error parsing config value \"HostName\": name too long\n");
				continue;
			}
			sprintf(out->host_domainIPv4, "%.*s", domain_len, domain);
		} else IFEQ("HostCert") {
			it = json_get_string(it, &master_cert, &master_cert_len);
		} else IFEQ("HostKey") {
			it = json_get_string(it, &master_key, &master_key_len);
		} else IFEQ("Port") {
			out->master_port = atoi(it);
			it = json_skip_value(it);
		} else IFEQ("StatusUri") {
			char *uri;
			uint32_t uri_len;
			it = json_get_string(it, &uri, &uri_len);
			if(uri_len >= lengthof(out->status_domain)) {
				uprintf("Error parsing config value \"StatusUri\": URI too long\n");
				continue;
			}
			if(uri_len > 8 && memcmp(uri, "https://", 8) == 0) {
				out->status_tls = 1, out->status_port = 443;
				uri += 8, uri_len -= 8;
			} else if(uri_len > 7 && memcmp(uri, "http://", 7) == 0) {
				out->status_tls = 0, out->status_port = 80;
				uri += 7, uri_len -= 7;
			} else {
				uprintf("Error parsing config value \"StatusUri\": URI must begin with http:// or https://\n");
				continue;
			}
			uint32_t sub_len;
			for(sub_len = 0; sub_len < uri_len; ++sub_len)
				if(uri[sub_len] == ':' || uri[sub_len] == '/')
					break;
			sprintf(out->status_domain, "%.*s", sub_len, uri);
			uri += sub_len, uri_len -= sub_len;
			if(uri_len == 0)
				continue;
			if(*uri == ':') {
				out->status_port = atoi(&uri[1]);
				for(; uri_len; ++uri, --uri_len)
					if(*uri == '/')
						break;
				if(uri_len == 0)
					continue;
			}
			sprintf(out->status_path, "%.*s", uri_len, uri);
			if(out->status_path[uri_len-1] == '/')
				out->status_path[uri_len-1] = 0;
		} else IFEQ("StatusCert") {
			it = json_get_string(it, &status_cert, &status_cert_len);
		} else IFEQ("StatusKey") {
			it = json_get_string(it, &status_key, &status_key_len);
		} else {
			it = json_skip_value(it);
		}
		#undef IFEQ
	}

	if(!*out->host_domain) {
		uprintf("Missing required value \"HostName\"\n");
		goto fail;
	}
	if(!*out->host_domainIPv4)
		sprintf(out->host_domainIPv4, "%s", out->host_domain);
	if(master_cert_len == 0) {
		uprintf("Using self-signined host certificate\n");
		master_key_len = 0;
	} else if(master_key_len == 0) {
		uprintf("Missing required value \"HostKey\"\n");
		goto fail;
	}
	if(status_cert_len == 0) {
		status_cert = master_cert;
		status_cert_len = master_cert_len;
		status_key = master_key;
		status_key_len = master_key_len;
	} else if(status_key_len == 0) {
		uprintf("Missing required value \"StatusKey\"\n");
		goto fail;
	}

	if(load_key(master_key, master_key_len, &ctr_drbg, &out->keys[0]))
		goto fail;
	if(load_key(status_key, status_key_len, &ctr_drbg, &out->keys[1]))
		goto fail;
	if(load_cert(master_cert, master_cert_len, &ctr_drbg, &out->keys[0], &out->certs[0]))
		goto fail;
	if(load_cert(status_cert, status_cert_len, &ctr_drbg, &out->keys[1], &out->certs[1]))
		goto fail;
	res = 0;
	fail:
	if(res)
		config_free(out);
	mbedtls_entropy_free(&entropy);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	return res;
}

void config_free(struct Config *cfg) {
	mbedtls_x509_crt_free(&cfg->certs[1]);
	mbedtls_x509_crt_free(&cfg->certs[0]);
	mbedtls_pk_free(&cfg->keys[1]);
	mbedtls_pk_free(&cfg->keys[0]);
}
