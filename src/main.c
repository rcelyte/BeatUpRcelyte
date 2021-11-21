#define SELF_SIGNED

#include "master.h"
#include "net.h"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef SELF_SIGNED
#include "json.h"

struct Host {
	char *domain, *cert, *key;
	uint32_t domain_len, cert_len, key_len;
	uint16_t port;
	_Bool renew;
	
};

static char *json_config_host(char *it, struct Host *res) {
	char *key;
	uint32_t key_len;
	while(json_iter_object(&it, &key, &key_len)) {
		if(key_len == 6 && memcmp(key, "domain", 6) == 0) {
			it = json_get_string(it, &res->domain, &res->domain_len);
			uint32_t ps = 0;
			for(uint32_t i = 0; i < res->domain_len; ++i) {
				if(res->domain[i] == ':')
					ps = i;
				else if(res->domain[i] < '0' || res->domain[i] > '9')
					ps = 0;
			}
			if(ps) {
				res->domain_len = ps;
				res->port = atoi(&res->domain[ps+1]);
			}
			// fprintf(stderr, "domain: %.*s\n", res->domain_len, res->domain);
		} else if(key_len == 5 && memcmp(key, "renew", 5) == 0) {
			it = json_get_bool(it, &res->renew);
			// fprintf(stderr, "renew: %hhu\n", res->renew);
		} else if(key_len == 4 && memcmp(key, "cert", 4) == 0) {
			it = json_get_string(it, &res->cert, &res->cert_len);
			// fprintf(stderr, "cert: %.*s\n", res->cert_len, res->cert);
		} else if(key_len == 3 && memcmp(key, "key", 3) == 0) {
			it = json_get_string(it, &res->key, &res->key_len);
			// fprintf(stderr, "key: %.*s\n", res->key_len, res->key);
		} else {
			it = json_skip_value(it);
		}
	}
	return it;
}

static _Bool load_cert(struct Host *in, mbedtls_x509_crt *out) {
	mbedtls_x509_crt_init(out);
	int32_t ret;
	char buf[4096];
	if(in->cert_len > 52 && strncmp(in->cert, "-----BEGIN CERTIFICATE-----", 27) == 0) {
		sprintf(buf, "inline certificate");
		ret = mbedtls_x509_crt_parse(out, (uint8_t*)in->cert, in->cert_len);
	} else if(in->renew) {
		fprintf(stderr, "AUTO RENEW NOT YET SUPPORTED\n");
		return 1;
	} else {
		if(in->cert_len >= sizeof(buf))
			ret = 1;
		else {
			sprintf(buf, "%.*s", in->cert_len, in->cert);
			ret = mbedtls_x509_crt_parse_file(out, buf);
		}
	}
	if(ret) {
		fprintf(stderr, "failed to load %s\n", buf);
		return 1;
	}
	return 0;
}

static _Bool load_key(struct Host *in, mbedtls_pk_context *out) {
	mbedtls_pk_init(out);
	int32_t ret;
	char buf[4096];
	if(in->key_len > 52 && strncmp(in->key, "-----BEGIN ", 11) == 0) {
		sprintf(buf, "inline certificate");
		ret = mbedtls_pk_parse_key(out, (uint8_t*)in->key, in->key_len, NULL, 0);
	} else {
		if(in->key_len >= sizeof(buf))
			ret = 1;
		else {
			sprintf(buf, "%.*s", in->key_len, in->key);
			ret = mbedtls_pk_parse_keyfile(out, buf, NULL);
		}
	}
	if(ret) {
		fprintf(stderr, "failed to load %s\n", buf);
		return 1;
	}
	return 0;
}
#endif

int main(int argc, char const *argv[]) {
	mbedtls_x509_crt host_cert, status_cert;
	mbedtls_pk_context host_key, status_key;
	#ifdef SELF_SIGNED
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
	mbedtls_x509_crt_init(&host_cert);
	mbedtls_x509_crt_parse(&host_cert, (const uint8_t*)cert_pem, 1923);
	mbedtls_pk_init(&host_key);
	#if MBEDTLS_VERSION_NUMBER < 0x03000000
	mbedtls_pk_parse_key(&host_key, (const uint8_t*)key_pem, 3272, NULL, 0);
	#else
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t*)"plsnohax", 8);
	mbedtls_pk_parse_key(&host_key, (const uint8_t*)key_pem, 3272, NULL, 0, mbedtls_ctr_drbg_random, &ctr_drbg);
	#endif
	status_cert = host_cert;
	status_key = host_key;
	#else
	{
		struct Host host = {NULL, NULL, NULL, 0, 0, 0, 2328, 0}, status = {NULL, NULL, NULL, 0, 0, 0, 443, 0};
		char *config;
		{
			FILE *f = fopen("config.json", "r");
			if(!f) {
				if(errno == ENOENT) {
					f = fopen("config.json", "w");
					if(f) {
						fprintf(f, "{\r\n\t\"host\": {\r\n\t\t\"domain\": \"example.com\",\r\n\t\t\"renew\": false,\r\n\t\t\"cert\": \"cert.pem\",\r\n\t\t\"key\": \"key.pem\"\r\n\t},\r\n\t\"status\": {\r\n\t\t\"domain\": \"status.example.com\",\r\n\t\t\"renew\": false,\r\n\t\t\"cert\": \"cert.pem\",\r\n\t\t\"key\": \"key.pem\"\r\n\t}\r\n}\r\n");
						fclose(f);
						fprintf(stderr, "Writing example config.json\n");
						return 0;
					}
				}
				fprintf(stderr, "Failed to open config.json: %s\n", strerror(errno));
				return -1;
			}
			fseek(f, 0, SEEK_END);
			size_t flen = ftell(f);
			config = malloc(flen);
			fseek(f, 0, SEEK_SET);
			if(fread(config, 1, flen, f) != flen) {
				fprintf(stderr, "Failed to read config.json\n");
				return -1;
			}
			fclose(f);
			char *key, *it = config;
			uint32_t key_len;
			while(json_iter_object(&it, &key, &key_len)) {
				if(key_len == 4 && memcmp(key, "host", 4) == 0)
					it = json_config_host(it, &host);
				else if(key_len == 6 && memcmp(key, "status", 6) == 0)
					it = json_config_host(it, &status);
				else
					it = json_skip_value(it);
			}
		}
		if(!host.domain) {
			fprintf(stderr, "Missing domain name\n");
			return -1;
		}
		if(!((host.cert_len && host.key_len) || host.renew)) {
			fprintf(stderr, host.cert_len ? "Missing SSL key\n" : "Missing SSL certificate\n");
			return -1;
		}
		if(!status.domain) {
			status.domain = host.domain;
			status.domain_len = host.domain_len;
		}
		if(host.renew && status.renew && host.domain_len == status.domain_len && memcmp(host.domain, status.domain, host.domain_len) == 0)
			status.renew = 0; // Don't double renew
		if(!status.cert) {
			status.cert = host.cert;
			status.cert_len = host.cert_len;
		}
		if(!status.key) {
			status.key = host.key;
			status.key_len = host.key_len;
		}

		if(load_cert(&host, &host_cert))
			return -1;
		if(load_cert(&status, &status_cert))
			return -1;
		if(load_key(&host, &host_key))
			return -1;
		if(load_key(&status, &status_key))
			return -1;
		free(config);
	}
	#endif

	if(net_init(status_cert, status_key))
		return -1;
	_Bool ret = master_run(&host_cert);
	net_cleanup();
	return ret ? -1 : 0;
}
