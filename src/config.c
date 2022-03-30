#include "log.h"
#include "config.h"
#include "json.h"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <stdlib.h>
#include <errno.h>

#define lengthof(x) (sizeof(x)/sizeof(*(x)))

static _Bool load_cert(char *cert, uint32_t cert_len, mbedtls_x509_crt *out) {
	mbedtls_x509_crt_init(out);
	int32_t err;
	char buf[4096];
	if(cert_len > 52 && strncmp(cert, "-----BEGIN CERTIFICATE-----", 27) == 0) {
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

static _Bool load_key(char *key, uint32_t key_len, mbedtls_pk_context *out) {
	mbedtls_pk_init(out);
	int32_t err;
	char buf[4096];
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	if(key_len > 52 && strncmp(key, "-----BEGIN ", 11) == 0) {
		sprintf(buf, "inline certificate");
		err = mbedtls_pk_parse_key(out, (uint8_t*)key, key_len, NULL, 0, mbedtls_ctr_drbg_random, &ctr_drbg);
	} else {
		if(key_len >= sizeof(buf)) {
			uprintf("Failed to load %.*s: Path too long\n", key_len, key);
			return 1;
		}
		sprintf(buf, "%.*s", key_len, key);
		err = mbedtls_pk_parse_keyfile(out, buf, NULL, mbedtls_ctr_drbg_random, &ctr_drbg);
	}
	mbedtls_entropy_free(&entropy);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	if(err) {
		uprintf("Failed to load %s: %s\n", buf, mbedtls_high_level_strerr(err));
		return 1;
	}
	return 0;
}

_Bool config_load(struct Config *out, const char *path) {
	char *master_cert = NULL, *master_key = NULL, *status_cert = NULL, *status_key = NULL;
	uint32_t master_cert_len = 0, master_key_len = 0, status_cert_len = 0, status_key_len = 0;

	char config_json[524288];
	FILE *f = fopen(path, "r");
	if(f == NULL && errno == ENOENT) {
		FILE *def = fopen(path, "w");
		if(!def) {
			uprintf("Failed to write default config to %s: %s\n", path, strerror(errno));
			return 0;
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
		return 1;
	}
	fseek(f, 0, SEEK_END);
	size_t flen = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(flen >= sizeof(config_json)) {
		uprintf("Failed to read %s: File too large\n", path);
		fclose(f);
		return 1;
	}
	if(fread(config_json, 1, flen, f) != flen) {
		uprintf("Failed to read %s\n", path);
		return 1;
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
		return 1;
	}
	if(!*out->host_domainIPv4)
		sprintf(out->host_domainIPv4, "%s", out->host_domain);
	if(master_cert_len == 0) {
		uprintf("Missing required value \"HostCert\"\n");
		return 1;
	}
	if(master_key_len == 0) {
		uprintf("Missing required value \"HostKey\"\n");
		return 1;
	}
	if(!status_cert) {
		status_cert = master_cert;
		status_cert_len = master_cert_len;
	}
	if(!status_key) {
		status_key = master_key;
		status_key_len = master_key_len;
	}

	if(!load_cert(master_cert, master_cert_len, &out->certs[0])) {
		if(!load_cert(status_cert, status_cert_len, &out->certs[1])) {
			if(!load_key(master_key, master_key_len, &out->keys[0])) {
				if(!load_key(status_key, status_key_len, &out->keys[1])) {
					return 0;
				}
				mbedtls_pk_free(&out->keys[1]);
			}
			mbedtls_pk_free(&out->keys[0]);
		}
		mbedtls_x509_crt_free(&out->certs[1]);
	}
	mbedtls_x509_crt_free(&out->certs[0]);
	return 1;
}

void config_free(struct Config *cfg) {
	mbedtls_x509_crt_free(&cfg->certs[0]);
	mbedtls_x509_crt_free(&cfg->certs[1]);
	mbedtls_pk_free(&cfg->keys[0]);
	mbedtls_pk_free(&cfg->keys[1]);
}

/*struct Config config_default() { // TODO: use domain specific self-signed cert instead of generic one
	struct Config out;
	const char *cert_pem =
		"-----BEGIN CERTIFICATE-----\n"
		"MIIFYDCCA0igAwIBAgIJAOQjJ14if2NJMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV\n"
		"BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX\n"
		"aWRnaXRzIFB0eSBMdGQwHhcNMjExMTIwMDc1MDUzWhcNMjIxMTIwMDc1MDUzWjBF\n"
		"MQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50\n"
		"ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIIC\n"
		"CgKCAgEAzUmPLSJVUlcyVaIS2RljCmKmJrAp0U5yqxQPsMbFp0UTbAxqxN7mKNL9\n"
		"jKRHXs8/PssL4LI3GxFf9qp/jCNgmKl+KRzpNZVTgfuRtfr3xbKBLJ2cEoF7cM5s\n"
		"ebmbk9z90Tw8oyhtJYlK96dPPpeS2/vp/Qx7KjA6XvLTyXaisg3RjWBkk/SOp6EK\n"
		"lzHHYw3b8sxZWOcUn8r2bcWmXPlpHOZlzkGHf1ciDVSF/BNX5mQvIg0GoO6RpyA1\n"
		"J8CSOTVDkSkpvzUZB9MXN3+F8BvkoejNXeSyNMDTYl9pl9ivDdPqK24RJeo53EAg\n"
		"myLMoHBBtof2md3RfD6Chooudt9/lohUIiHhsv5WkHnCIiv+4TjsGRs9G9CLzw4q\n"
		"BUdpJLZjuVbgojkAeXC+MR8py7spihzRaoHTkhi7/wp6b+Iydl1KUmCMYV75IO+0\n"
		"gnOER4943i0UVLBjqkD94/HH3X6A00rsEm/R9psuom32ShHy+yHDI1C08K+5p3Rf\n"
		"0nAy0T34hR8c5vN0sQos6QOgHmNm+5rBfyjLlgJQ/KKU8jNR9xNh14CEy+heHWB/\n"
		"r+9wRX3YSz/mx30Q05C1iOGtUSn6QQn4MtDaQWG2VkMfmmju7PtptD2nZP3zKl2d\n"
		"M2eiymiIc3m26pMEGRYhYULNQ5sFYHmJYu6wVRwpYzqxHHac9mUCAwEAAaNTMFEw\n"
		"HQYDVR0OBBYEFKEdyCQi20ruEBOqh2tJtZK1GABUMB8GA1UdIwQYMBaAFKEdyCQi\n"
		"20ruEBOqh2tJtZK1GABUMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQAD\n"
		"ggIBABIDoYyNTe4+Ri+3tJGLd3MYgRl1PKwAI8Wh7Q4FbbRj/sC2eEV3CPKLVzIq\n"
		"dm7QAEwLpYIx/WH2n42mpdV08baXLG3y8mKWEhhh5UY7/8Gz1SDYghsxKJTmXoaA\n"
		"oK4DMsr1Vor5h9h2C2cofxdw7KSf0VjgJ5uyl046/kEyg3+O6OyfptrIYOeFg//m\n"
		"lA0x8waE2r889Eu37laWkwwOgbSlsaRMJfs/W88pbYmiSaGwfxWRFngYMustkvLi\n"
		"be7C9n/NayuOGrzuAPNcMKvrFyazn2QOVE07jICftMaKBThmSJ2V2R3OXY1Q1Q8k\n"
		"C1zuJi9Ot4xa7Xw/8KBkSR5rFiXVAhXGjmS/vf0BYd58s18lNVEMzV8WTqQG3r0T\n"
		"0NWXT/bkEYSwQuPe3A8etx4JViFO6gaHDmH82xZDahOGXgALSDcOnON+T+yDTXUV\n"
		"Ah4+bN2BBfDhGEbXM+nXnw/VDe9XedItoCv+qqfQwMg24WBrfWZVE7KcNI9wIyeC\n"
		"XlrKP/r9wVfytTS/2mYPPYOnZVnWj5gYGa0dkokYoSuYUwAD8ELFw80vnHw4YuXv\n"
		"moo40s/h4rr3lfoLAXUSzTyS1v2OIz2/D/FIkhHpSAgNCynNQ0PsHiXCbS4dk2YC\n"
		"dJco+wPzVhhc9jxACoLrEhRO4DWOvcKrpFErT4odVJCUC58i\n"
		"-----END CERTIFICATE-----\n";
	const char *key_pem =
		"-----BEGIN PRIVATE KEY-----\n"
		"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\n"
		"PNpM4ToKUvBvz1nhXu1qeT2gvDrVDd4V\n"
		"-----END PRIVATE KEY-----\n";
	mbedtls_x509_crt_init(&out.master_cert);
	mbedtls_x509_crt_parse(&out.master_cert, (const uint8_t*)cert_pem, 1923);
	mbedtls_pk_init(&out.master_key);
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t*)"plsnohax", 8);
	mbedtls_pk_parse_key(&out.master_key, (const uint8_t*)key_pem, 3272, NULL, 0, mbedtls_ctr_drbg_random, &ctr_drbg);
	out.status_cert = out.master_cert;
	out.status_key = out.master_key;
	mbedtls_entropy_free(&entropy);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	out.master_port = 2328;
	out.status_port = 443;
	return out;
}*/
