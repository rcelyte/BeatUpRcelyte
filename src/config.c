#include "global.h"
#include "config.h"
#include "json.h"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WINDOWS
#define NEWLINE "\r\n"

static uint16_t GetCoreCount() {
	return 1; // TODO: Win32 stuff
}
#else
#include <sys/sysinfo.h>
#define NEWLINE "\n"

static uint16_t GetCoreCount() {
	return (uint16_t)get_nprocs();
}
#endif

static void config_read_cert(const char **it, jsonkey_t key, mbedtls_x509_crt *out) {
	uint32_t str_len = 0;
	const char *str = json_read_string(it, &str_len);

	int res = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
	if(str_len > 52 && strncmp(str, "-----BEGIN CERTIFICATE-----", 27) == 0) {
		res = mbedtls_x509_crt_parse(out, (const uint8_t*)str, str_len);
	} else if(str_len < CONFIG_STRING_LENGTH) {
		char filename[CONFIG_STRING_LENGTH];
		sprintf(filename, "%.*s", str_len, str);
		res = mbedtls_x509_crt_parse_file(out, filename);
	} else {
		json_error(it, "Error parsing config value \"%s\": file path too long\n", JSON_KEY_TOSTRING(key));
		return;
	}
	if(res)
		json_error(it, "Failed to load certificate: %s\n", mbedtls_high_level_strerr(res));
}

static void config_read_pk(const char **it, jsonkey_t key, mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_pk_context *out) {
	uint32_t str_len = 0;
	const char *str = json_read_string(it, &str_len);

	int res = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
	if(str_len > 52 && strncmp(str, "-----BEGIN ", 11) == 0) {
		res = mbedtls_pk_parse_key(out, (const uint8_t*)str, str_len, NULL, 0, mbedtls_ctr_drbg_random, ctr_drbg);
	} else if(str_len < CONFIG_STRING_LENGTH) {
		char filename[CONFIG_STRING_LENGTH];
		sprintf(filename, "%.*s", str_len, str);
		res = mbedtls_pk_parse_keyfile(out, filename, NULL, mbedtls_ctr_drbg_random, ctr_drbg);
	} else {
		json_error(it, "Error parsing config value \"%s\": file path too long\n", JSON_KEY_TOSTRING(key));
		return;
	}
	if(res)
		json_error(it, "Failed to load private key: %s\n", mbedtls_high_level_strerr(res));
}

static void config_read_url(const char **it, jsonkey_t key, char address_out[static CONFIG_STRING_LENGTH], char path_out[static CONFIG_STRING_LENGTH], uint16_t *port_out, bool *https_out) {
	uint32_t uri_len = 0;
	const char *uri = json_read_string(it, &uri_len);

	if(uri_len >= CONFIG_STRING_LENGTH) {
		json_error(it, "Error parsing config value \"%s\": URL too long\n", JSON_KEY_TOSTRING(key));
		return;
	}
	long portNum;
	if(uri_len > 8 && memcmp(uri, "https://", 8) == 0) {
		*https_out = true; portNum = 443;
		uri += 8; uri_len -= 8;
	} else if(uri_len > 7 && memcmp(uri, "http://", 7) == 0) {
		*https_out = false; portNum = 80;
		uri += 7; uri_len -= 7;
	} else {
		json_error(it, "Error parsing config value \"%s\": URL must begin with `http://` or `https://`\n", JSON_KEY_TOSTRING(key));
		return;
	}
	const char *path = memchr(uri, '/', uri_len);
	const char *port_end = path ? path : &uri[uri_len];
	const char *port = memchr(uri, ':', (uint32_t)(port_end - uri));
	uint32_t address_len = (uint32_t)((port ? port : port_end) - uri);
	memcpy(address_out, uri, address_len);
	address_out[address_len] = 0;
	if(port) {
		portNum = atol(&port[1]); // skip ':'
		if(portNum < 1 || portNum > 65535) {
			json_error(it, "Error parsing config value \"%s\": port number out of range\n", JSON_KEY_TOSTRING(key));
			return;
		}
	}
	*port_out = (uint16_t)portNum;
	uint32_t path_len = 0;
	if(path++) {
		path_len = (uint32_t)(&uri[uri_len] - path);
		memcpy(path_out, path, path_len);
		if(path_len && path_out[path_len - 1] != '/')
			path_out[path_len++] = '/';
	}
	path_out[path_len] = 0;
}

static void config_read_hex(const char **it, jsonkey_t key, uint8_t key_out[static 32], uint8_t *length_out) {
	static const uint8_t table[128] = {
		['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4, ['5'] = 5, ['6'] = 6, ['7'] = 7, ['8'] = 8, ['9'] = 9,
		['a'] = 10, ['b'] = 11, ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15, ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15,
	};
	uint32_t str_len = 0;
	const char *str = json_read_string(it, &str_len);
	if(str_len != 64) {
		json_error(it, "Error parsing config value \"%s\": bad key length\n", JSON_KEY_TOSTRING(key));
		return;
	}
	*length_out = 32;
	for(uint8_t *key_it = key_out; key_it < &key_out[32]; ++key_it, str += 2)
		*key_it = (uint8_t)(table[str[0] & 127] << 4 | table[str[1] & 127]);
}

static void config_read_string(const char **it, jsonkey_t key, char out[static CONFIG_STRING_LENGTH]) {
	uint32_t str_len = 0;
	const char *str = json_read_string(it, &str_len);
	if(str_len >= CONFIG_STRING_LENGTH) {
		json_error(it, "Error parsing config value \"%s\": name too long\n", JSON_KEY_TOSTRING(key));
		return;
	}
	snprintf(out, CONFIG_STRING_LENGTH, "%.*s", str_len, str);
}

static void config_read_uint16(const char **it, jsonkey_t key, uint16_t minValue, uint16_t maxValue, uint16_t *out) {
	int64_t value = json_read_int64(it);
	if(value < minValue || value > maxValue) {
		json_error(it, "Error parsing config value \"%s\": integer out of range\n", JSON_KEY_TOSTRING(key));
		return;
	}
	*out = (uint16_t)value;
}

bool config_load(struct Config *out, const char *path) {
	bool enableInstance = false, enableMaster = false, enableStatus = false, statusHTTPS = false;

	mbedtls_x509_crt_init(&out->certs[0]);
	mbedtls_x509_crt_init(&out->certs[1]);
	mbedtls_pk_init(&out->keys[0]);
	mbedtls_pk_init(&out->keys[1]);
	out->wireKey_len = 0;
	out->instanceCount = GetCoreCount();
	out->masterPort = 2328;
	out->statusPort = 0;
	*out->instanceAddress[0] = 0;
	*out->instanceAddress[1] = 0;
	*out->instanceParent = 0;
	*out->instanceMapPool = 0;
	*out->statusAddress = 0;
	*out->statusPath = 0;

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	if(mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const uint8_t*)"password", 8) != 0) {
		uprintf("mbedtls_ctr_drbg_seed() failed\n");
		goto fail;
	}

	char config_json[524288];
	FILE *f = fopen(path, "rb");
	if(f == NULL && errno == ENOENT) {
		FILE *def = fopen(path, "wb");
		if(!def) {
			uprintf("Failed to write default config to %s: %s\n", path, strerror(errno));
			goto fail;
		}
		char publicIP[] = "\"127.0.0.1\""; // TODO: resolve actual public IP
		uprintf("Writing default config to %s\n", path);
		fprintf(def,
			"{" NEWLINE
			"\t\"instance\": {" NEWLINE
			"\t\t\"address\": %s," NEWLINE
			"\t\t\"count\": 1" NEWLINE
			"\t}," NEWLINE
			"\t\"master\": {}," NEWLINE
			"\t\"status\": {" NEWLINE
			"\t\t\"url\": \"http://localhost\"" NEWLINE
			"\t}" NEWLINE
			"}" NEWLINE, publicIP);
		fclose(def);
		f = fopen(path, "rb");
	}
	if(f == NULL) {
		uprintf("Failed to open %s: %s\n", path, strerror(errno));
		goto fail;
	}
	fseek(f, 0, SEEK_END);
	size_t flen = (size_t)ftell(f);
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

	const char *it = config_json;
	JSON_ITER_OBJECT(&it) {
		case JSON_KEY('w','i','r','e','K','e','y'): config_read_hex(&it, key, out->wireKey, &out->wireKey_len); break; // TODO: multiple keys
		case JSON_KEY('i','n','s','t','a','n','c','e'): enableInstance = true; JSON_ITER_OBJECT(&it) {
			case JSON_KEY('a','d','d','r','e','s','s'): {
				if(*it != '[') {
					config_read_string(&it, key, out->instanceAddress[0]);
					break;
				}
				uint8_t i = 0;
				JSON_ITER_ARRAY(&it) {
					if(i < lengthof(out->instanceAddress))
						config_read_string(&it, key, out->instanceAddress[i++]);
					else
						json_skip_string(&it);
				}
				break;
			}
			case JSON_KEY('m','a','s','t','e','r'): config_read_string(&it, key, out->instanceParent); break;
			case JSON_KEY('m','a','p','P','o','o','l'): config_read_string(&it, key, out->instanceMapPool); break;
			case JSON_KEY('c','o','u','n','t'): config_read_uint16(&it, key, 0, 8192, &out->instanceCount); break;
			default: json_skip_any(&it);
		} break;
		case JSON_KEY('m','a','s','t','e','r'): enableMaster = true; JSON_ITER_OBJECT(&it) {
			case JSON_KEY('c','e','r','t'): config_read_cert(&it, key, &out->masterCert); break;
			case JSON_KEY('k','e','y'): config_read_pk(&it, key, &ctr_drbg, &out->masterKey); break;
			case JSON_KEY('p','o','r','t'): config_read_uint16(&it, key, 1, 65535, &out->masterPort); break;
			default: json_skip_any(&it);
		} break;
		case JSON_KEY('s','t','a','t','u','s'): enableStatus = true; JSON_ITER_OBJECT(&it) {
			case JSON_KEY('u','r','l'): config_read_url(&it, key, out->statusAddress, out->statusPath, &out->statusPort, &statusHTTPS); break;
			case JSON_KEY('c','e','r','t'): config_read_cert(&it, key, &out->statusCert); break;
			case JSON_KEY('k','e','y'): config_read_pk(&it, key, &ctr_drbg, &out->statusKey); break;
			default: json_skip_any(&it);
		} break;
		default: json_skip_any(&it);
	}
	if(json_is_error(it))
		goto fail;

	if(!enableInstance) {
		out->instanceCount = 0;
	} else if(*out->instanceParent && !out->wireKey_len) {
		uprintf("Missing required value \"wireKey\"\n");
		goto fail;
	}
	if(!*out->instanceAddress[1])
		memcpy(out->instanceAddress[1], out->instanceAddress[0], sizeof(out->instanceAddress[0]));

	if(enableMaster) {
		if(mbedtls_pk_get_type(&out->masterKey) == MBEDTLS_PK_NONE) {
			if(out->masterCert.version) {
				uprintf("Missing required value \"key\" for master\n");
				goto fail;
			}
			int res = mbedtls_pk_setup(&out->masterKey, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
			if(res) {
				uprintf("mbedtls_pk_setup() failed: %s\n", mbedtls_high_level_strerr(res));
				goto fail;
			}
			res = mbedtls_rsa_gen_key(mbedtls_pk_rsa(out->masterKey), mbedtls_ctr_drbg_random, &ctr_drbg, 2048, 65537);
			if(res) {
				uprintf("mbedtls_rsa_gen_key() failed: %s\n", mbedtls_high_level_strerr(res));
				goto fail;
			}
		}
		if(!out->masterCert.version) {
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
			mbedtls_x509write_crt_set_subject_key(&cert_ctx, &out->masterKey);
			mbedtls_x509write_crt_set_issuer_key(&cert_ctx, &out->masterKey);
			mbedtls_x509write_crt_set_md_alg(&cert_ctx, MBEDTLS_MD_SHA256);
			mbedtls_x509write_crt_set_basic_constraints(&cert_ctx, 0, -1);
			mbedtls_x509write_crt_set_subject_key_identifier(&cert_ctx);
			mbedtls_x509write_crt_set_authority_key_identifier(&cert_ctx);
			uint8_t cert[4096];
			int res = mbedtls_x509write_crt_der(&cert_ctx, cert, sizeof(cert), mbedtls_ctr_drbg_random, &ctr_drbg);
			mbedtls_x509write_crt_free(&cert_ctx);
			if(res < 0) {
				uprintf("mbedtls_x509write_crt_der() failed: %s\n", mbedtls_high_level_strerr(res));
				goto fail;
			}
			res = mbedtls_x509_crt_parse_der(&out->masterCert, &cert[sizeof(cert) - (uint32_t)res], (size_t)res);
			if(res < 0) {
				uprintf("mbedtls_x509_crt_parse_der() failed: %s\n", mbedtls_high_level_strerr(res));
				goto fail;
			}
		}
	} else {
		out->masterPort = 0;
	}

	if(statusHTTPS) {
		if(mbedtls_pk_get_type(&out->statusKey) == MBEDTLS_PK_NONE) {
			uprintf("Missing required value \"key\" for status\n");
			goto fail;
		}
		if(!out->statusCert.version) {
			uprintf("Missing required value \"cert\" for status\n");
			goto fail;
		}
	} else if(enableStatus) {
		if(!out->statusPort) {
			uprintf("Missing required value \"url\" for status\n");
			goto fail;
		}
		mbedtls_x509_crt_free(&out->statusCert);
		mbedtls_pk_free(&out->statusKey);
	}

	bool ret = false;
	if(false) {
		fail:
		config_free(out);
		ret = true;
	}
	mbedtls_entropy_free(&entropy);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	return ret;
}

void config_free(struct Config *cfg) {
	mbedtls_x509_crt_free(&cfg->certs[1]);
	mbedtls_x509_crt_free(&cfg->certs[0]);
	mbedtls_pk_free(&cfg->keys[1]);
	mbedtls_pk_free(&cfg->keys[0]);
}
