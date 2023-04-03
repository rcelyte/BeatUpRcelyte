#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WINDOWS
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

struct JsonIterator {
	const char *head, *const end;
	bool fault, _pad0[sizeof(void*) - 1];
};

static void json_error(struct JsonIterator *const iter, const char *const format, ...) {
	iter->head = iter->end;
	if(iter->fault)
		return;
	iter->fault = true;
	va_list args;
	va_start(args, format);
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	vfprintf(stderr, format, args); // TODO: configurable logging
	#pragma GCC diagnostic pop
	va_end(args);
}

static bool json_skip_char(struct JsonIterator *const iter, char c) {
	if(iter->head >= iter->end || *iter->head++ != c)
		json_error(iter, "JSON parse error: expected '%c'\n", c); // TODO: actually useful syntax error messages
	return iter->head >= iter->end;
}

static bool json_skip_char_maybe(struct JsonIterator *const iter, char c) {
	bool match = (iter->head < iter->end && *iter->head == c);
	iter->head += match;
	return match;
}

static void json_skip_whitespace(struct JsonIterator *const iter) {
	while(iter->head < iter->end && ((*iter->head >= '\t' && *iter->head <= '\r') || *iter->head == ' '))
		++iter->head;
}

static void json_skip_string(struct JsonIterator *const iter) {
	json_skip_char(iter, '"');
	while(iter->head < iter->end && (*iter->head != '"' || *(iter->head - 1) == '\\'))
		++iter->head;
	json_skip_char(iter, '"');
}

static void json_skip_number(struct JsonIterator *const iter) {
	while(iter->head < iter->end && (*iter->head == '+' || (*iter->head >= '-' && *iter->head <= '9' && *iter->head != '/') || (*iter->head & 223) == 'E'))
		++iter->head;
}

static void json_skip_object(struct JsonIterator *const iter) {
	json_skip_char(iter, '{');
	for(uint32_t bc = 1; bc && iter->head < iter->end;) {
		bc = bc + (*iter->head == '{') - (*iter->head == '}');
		if(*iter->head == '"')
			json_skip_string(iter);
		else
			++iter->head;
	}
}

static void json_skip_array(struct JsonIterator *const iter) {
	json_skip_char(iter, '[');
	for(uint32_t bc = 1; bc && iter->head < iter->end;) {
		bc = bc + (*iter->head == '[') - (*iter->head == ']');
		if(*iter->head == '"')
			json_skip_string(iter);
		else
			++iter->head;
	}
}

static bool json_read_bool(struct JsonIterator *const iter) {
	if(json_skip_char_maybe(iter, 't')) {
		json_skip_char(iter, 'r');
		json_skip_char(iter, 'u');
		json_skip_char(iter, 'e');
		return true;
	}
	json_skip_char(iter, 'f');
	json_skip_char(iter, 'a');
	json_skip_char(iter, 'l');
	json_skip_char(iter, 's');
	json_skip_char(iter, 'e');
	return false;
}

static void json_skip_any(struct JsonIterator *const iter) {
	switch(iter->head < iter->end ? *iter->head : 0) {
		case '"': json_skip_string(iter); break;
		case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': json_skip_number(iter); break;
		case '{': json_skip_object(iter); break;
		case '[': json_skip_array(iter); break;
		case 't': case 'f': json_read_bool(iter); break;
		case 'n': {
			json_skip_char(iter, 'n');
			json_skip_char(iter, 'u');
			json_skip_char(iter, 'l');
			json_skip_char(iter, 'l');
			break;
		}
		default: json_error(iter, "JSON parse error: expected value\n");
	}
}

static struct String json_read_string(struct JsonIterator *const iter) {
	if(!json_skip_char_maybe(iter, '"')) {
		json_skip_any(iter);
		return (struct String){.isNull = true};
	}
	for(const char *start = iter->head; iter->head < iter->end; ++iter->head) {
		if(*iter->head != '"' || *(iter->head - 1) == '\\')
			continue;
		struct String out = {
			.length = iter->head++ - start,
		};
		if(out.length > sizeof(out.data))
			out.length = sizeof(out.data);
		memcpy(out.data, start, out.length);
		return out;
	}
	json_error(iter, "JSON parse error: unexpected end of content\n");
	return (struct String){.isNull = true};
}

static struct String json_read_object_key(struct JsonIterator *const iter) {
	struct String key = json_read_string(iter);
	json_skip_whitespace(iter);
	json_skip_char(iter, ':');
	json_skip_whitespace(iter); // `value` start
	return key;
}

static struct String json_iter_object_start(struct JsonIterator *const iter) {
	json_skip_char(iter, '{');
	json_skip_whitespace(iter);
	if(json_skip_char_maybe(iter, '}'))
		return (struct String){.isNull = true};
	return json_read_object_key(iter);
}

static struct String json_iter_object_next(struct JsonIterator *const iter) {
	json_skip_whitespace(iter); // `value` end
	if(json_skip_char_maybe(iter, '}'))
		return (struct String){.isNull = true};
	json_skip_char(iter, ',');
	json_skip_whitespace(iter);
	return json_read_object_key(iter);
}

#define JSON_ITER_OBJECT(iter, key) \
	for(struct String key = json_iter_object_start(iter); !key.isNull; key = json_iter_object_next(iter))

static uint64_t json_read_uint64(struct JsonIterator *const iter) {
	uint64_t res = 0;
	const char *const start = iter->head;
	while(iter->head < iter->end && *iter->head >= '0' && *iter->head <= '9') // TODO: range check
		res = res * 10 + (uint8_t)*iter->head++ - '0';
	if(iter->head == start)
		json_error(iter, "JSON parse error: expected integer\n");
	return res;
}
