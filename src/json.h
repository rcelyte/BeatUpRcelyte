#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WINDOWS
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

typedef uint64_t jsonkey_t; // an 8 byte or shorter string
#define JSON_KEY_V(c0, c1, c2, c3, c4, c5, c6, c7, ...) ((uint64_t)(c0) | (uint64_t)(c1) << 8 | (uint64_t)(c2) << 16 | (uint64_t)(c3) << 24 | (uint64_t)(c4) << 32 | (uint64_t)(c5) << 40 | (uint64_t)(c6) << 48 | (uint64_t)(c7) << 56)
#define JSON_KEY(...) JSON_KEY_V(__VA_ARGS__,0,0,0,0,0,0,0,0,)

#define JSON_KEY_TOSTRING(key) (_json_key_tostring((uint8_t[]){0,0,0,0,0,0,0,0,0}, (key)))
static inline const char *_json_key_tostring(uint8_t buf[restrict static 8], jsonkey_t key) {
	if(ntohl(1) == 1)
		key = __builtin_bswap64(key);
	memcpy(buf, &key, sizeof(key));
	return (char*)buf;
}

static const char _json_zero[] = {0,0,0,0,0,0,0,0};
[[maybe_unused]] static bool json_is_error(const char *it) {
	return (uintptr_t)(it - _json_zero) < lengthof(_json_zero);
}

static void json_error(const char **it, const char *format, ...) {
	*it = _json_zero;
	va_list args;
	va_start(args, format);
	vuprintf(format, args);
	va_end(args);
}

static bool json_skip_char(const char **it, char c) {
	if(*(*it)++ != c)
		json_error(it, "JSON parse error: expected '%c'\n", c); // TODO: actually useful syntax error messages
	return **it == 0;
}

static bool json_skip_char_maybe(const char **it, char c) {
	bool match = (**it == c);
	*it += match;
	return match;
}

static void json_skip_whitespace(const char **it) {
	while((**it >= '\t' && **it <= '\r') || **it == ' ')
		++(*it);
}

static bool json_read_bool(const char **it) {
	if(json_skip_char_maybe(it, 't')) {
		json_skip_char(it, 'r');
		json_skip_char(it, 'u');
		json_skip_char(it, 'e');
		return true;
	}
	json_skip_char(it, 'f');
	json_skip_char(it, 'a');
	json_skip_char(it, 'l');
	json_skip_char(it, 's');
	json_skip_char(it, 'e');
	return false;
}

static const char *json_read_string(const char **it, uint32_t *length_out) { // TODO: decode escaped characters
	*length_out = 0;
	if(json_skip_char(it, '"'))
		return NULL;
	const char *out = *it;
	for(; **it && **it != '"'; ++(*length_out)) {
		if(*(*it)++ == '\\') {
			if(*(*it)++ == 0) {
				json_error(it, "JSON parse error: unexpected end of content\n");
				break;
			}
		}
	}
	json_skip_char(it, '"');
	return out;
}

static void json_skip_string(const char **it) {
	json_skip_char(it, '"');
	while(**it && **it != '"')
		if(*(*it)++ == '\\')
			if(*(*it)++ == 0)
				json_error(it, "JSON parse error: unexpected end of content\n");
	json_skip_char(it, '"');
}

static void json_skip_number(const char **it) {
	while(**it == '+' || (**it >= '-' && **it <= '9' && **it != '/') || (**it & 223) == 'E')
		++(*it);
}

static void json_skip_object(const char **it) {
	json_skip_char(it, '{');
	uint32_t bc = 1;
	while(bc && **it) {
		bc = bc + (**it == '{') - (**it == '}');
		if(**it == '"')
			json_skip_string(it);
		else
			++(*it);
	}
}

static void json_skip_array(const char **it) {
	json_skip_char(it, '[');
	uint32_t bc = 1;
	while(bc && **it) {
		bc = bc + (**it == '[') - (**it == ']');
		if(**it == '"')
			json_skip_string(it);
		else
			++(*it);
	}
}

[[maybe_unused]] static void json_skip_any(const char **it) {
	switch(**it) {
		case '"': json_skip_string(it); break;
		case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': json_skip_number(it); break;
		case '{': json_skip_object(it); break;
		case '[': json_skip_array(it); break;
		case 't': case 'f': json_read_bool(it); break;
		case 'n': {
			json_skip_char(it, 'n');
			json_skip_char(it, 'u');
			json_skip_char(it, 'l');
			json_skip_char(it, 'l');
			break;
		}
		default: json_error(it, "JSON parse error: expected value\n"); return;
	}
}

static jsonkey_t json_read_key(const char **it) {
	uint32_t str_len = 0;
	const char *str = json_read_string(it, &str_len);
	if(!str)
		return 0;
	json_skip_whitespace(it);
	json_skip_char(it, ':');
	json_skip_whitespace(it); // `value` start
	if(str_len > sizeof(jsonkey_t))
		return ~0llu;
	char ex[8] = {0,0,0,0,0,0,0,0};
	memcpy(ex, str, str_len);
	jsonkey_t key = JSON_KEY(ex[0], ex[1], ex[2], ex[3], ex[4], ex[5], ex[6], ex[7]);
	return key ? key : ~0llu;
}

[[maybe_unused]] static jsonkey_t json_iter_object_start(const char **it) {
	json_skip_char(it, '{');
	json_skip_whitespace(it);
	if(json_skip_char_maybe(it, '}'))
		return 0;
	return json_read_key(it);
}

[[maybe_unused]] static jsonkey_t json_iter_object_next(const char **it) {
	json_skip_whitespace(it); // `value` end
	if(json_skip_char_maybe(it, '}'))
		return 0;
	json_skip_char(it, ',');
	json_skip_whitespace(it);
	return json_read_key(it);
}

[[maybe_unused]] static bool json_iter_array_start(const char **it) {
	json_skip_char(it, '[');
	json_skip_whitespace(it); // `value` start
	return !json_skip_char_maybe(it, ']');
}
[[maybe_unused]] static bool json_iter_array_next(const char **it) {
	json_skip_whitespace(it); // `value` end
	if(json_skip_char_maybe(it, ']'))
		return false;
	bool error = json_skip_char(it, ',');
	json_skip_whitespace(it); // `value` start
	return !error;
}

#define JSON_ITER_OBJECT(it) \
	for(jsonkey_t key = json_iter_object_start(it); key; key = json_iter_object_next(it)) \
		switch(key)

#define JSON_ITER_ARRAY(it) \
	for(bool cont = json_iter_array_start(it); cont; cont = json_iter_array_next(it))

[[maybe_unused]] static int64_t json_read_int64(const char **it) {
	char *end = NULL; // strtoll expects a non-const pointer
	int64_t v = strtoll(*it, &end, 10);
	*it = end;
	return v;
}
