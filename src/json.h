#include <string.h>

static char _zero = 0;
static inline char *json_skip(char *s, char c) {
	if(*s++ != c)
		return &_zero;
	return s;
}
static inline char *json_whitespace(char *s) {
	while((*s >= '\t' && *s <= '\r') || *s == ' ')
		++s;
	return s;
}
[[maybe_unused]] static char *json_get_bool(char *s, bool *out) {
	if(strncmp(s, "true", 4) == 0) {
		*out = 1;
		return s+4;
	}
	if(strncmp(s, "false", 5) == 0) {
		*out = 0;
		return s+5;
	}
	return s;
}
static char *json_get_string(char *s, char **out, uint32_t *len) {
	s = json_skip(s, '"');
	*out = s;
	while(*s && *s != '"')
		if(*s++ == '\\')
			if(*s++ == 0)
				return NULL;
	*len = s - *out;
	return json_whitespace(json_skip(s, '"'));
}
static char *json_skip_string(char *s) {
	s = json_skip(s, '"');
	while(*s && *s != '"')
		if(*s++ == '\\')
			if(*s++ == 0)
				return NULL;
	return json_whitespace(json_skip(s, '"'));
}
static char *json_skip_object_fast(char *s) {
	s = json_skip(s, '{');
	uint32_t bc = 1;
	while(bc && *s) {
		bc = bc + (*s == '{') - (*s == '}');
		if(*s == '"')
			s = json_skip_string(s);
		else
			++s;
	}
	return s;
}
static char *json_skip_array_fast(char *s) {
	s = json_skip(s, '[');
	uint32_t bc = 1;
	while(bc && *s) {
		bc = bc + (*s == '[') - (*s == ']');
		if(*s == '"')
			s = json_skip_string(s);
		else
			++s;
	}
	return s;
}
static char *json_skip_value(char *s) {
	switch(*s) {
		case '"': s = json_skip_string(s); break;
		case '{': s = json_skip_object_fast(s); break;
		case '[': s = json_skip_array_fast(s); break;
		default: // ignore data; handles whitespace
			while(*s != ',' && *s != '}' && *s != ']' && *s) ++s;
			return s;
	}
	return json_whitespace(s);
}
static bool json_iter_object(char **s, char **key, uint32_t *key_len) {
	*s = json_whitespace(*s);
	if(**s)
		if(*(*s)++ == '}')
			return 0;
	*s = json_whitespace(*s);
	*s = json_get_string(*s, key, key_len);
	*s = json_whitespace(json_skip(*s, ':'));
	return **s != 0;
}
