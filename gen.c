#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#define DISABLE_LOG_PREFIX
#include "src/log.h"

static bool enableLog = false;

enum TType {
	TType_Enum_start,
	TType_Enum_end,
	TType_Struct_start,
	TType_Struct_end,
	TType_If_start,
	TType_If_end,
	TType_Field,
};

struct EnumToken {
	char type[64];
	char name[64];
	char switchField[64];
};

struct StructToken {
	char name[64];
	bool send, sendIntern;
	bool recv, recvIntern;
};

struct IfToken {
	char condition[204];
};

typedef uint8_t EnumFlags;
enum {
	EnumFlags_HasValue = 1,
	EnumFlags_IsDuplicate = 2,
};

struct FieldToken {
	char type[64];
	char name[64];
	char count[64];
	uint16_t maxCount;
	uint8_t bitWidth;
	EnumFlags enumFlags;
	int64_t enumValue;
};

struct Token {
	enum TType type;
	union {
		struct EnumToken enum_;
		struct StructToken struct_;
		struct IfToken if_;
		struct FieldToken field;
	};
} static tokens[81920], *tokens_end = tokens;

static const char *filename = NULL;
static noreturn void fail(const char *format, ...) {
	char msg[8192];
	snprintf(msg, sizeof(msg), "%s: %s\n", filename, format);
	va_list args;
	va_start(args, format);
	vuprintf(msg, args);
	va_end(args);
	exit(-1);
}

static const char *fileStart = NULL;
static noreturn void fail_at(const char *pos, const char *format, ...) {
	char msg[8192];
	uint32_t line = 1, column = 1;
	for(const char *it = fileStart; it < pos; ++it, ++column)
		if(*it == '\n')
			++line, column = 0;
	snprintf(msg, sizeof(msg), "%s:%u:%u: %s\n", filename, line, column, format);
	va_list args;
	va_start(args, format);
	vuprintf(msg, args);
	va_end(args);
	exit(-1);
}

static void skip_line(const char **it) {
	for(char prev = 0; **it && prev != '\n'; ++*it)
		prev = **it;
}

static void skip_char(const char **it, char ch) {
	if(*(*it)++ != ch) {
		if(ch == '\n')
			fail_at(*it, "expected newline");
		else
			fail_at(*it, "expected '%c'", ch);
	}
}

static bool skip_char_maybe(const char **it, char ch) {
	if(**it != ch)
		return false;
	++*it;
	return true;
}

static bool skip_indent_maybe(const char **it, uint32_t level) {
	const char *i = *it;
	while(level--)
		if(*i++ != '\t')
			return false;
	*it = i;
	return true;
}

static bool alpha(char ch) {
	char cap = ch & 223;
	return (cap >= 'A' && cap <= 'Z') || ch == '_';
}

static bool numeric(char ch) {
	return (ch >= '0' && ch <= '9') || ch == '+' || ch == '-';
}

static char read_char(const char **it) {
	char ch = *(*it)++;
	if(!ch)
		fail_at(*it - 1, "unexpected EOF");
	return ch;
}

static void read_name(const char **it, char *out, size_t out_len) {
	if(!alpha(**it))
		fail_at(*it, "expected name");
	while((**it >= '0' && **it <= '9') || alpha(**it)) {
		*out++ = *(*it)++;
		if(!(--out_len))
			fail_at(*it, "string too long");
	}
	*out = 0;
}

static void read_scope(const char **it, char *out, size_t out_len, char open, char close) {
	for(uint32_t level = 1; (level = level + (**it == open) - (**it == close));) {
		if((*out++ = read_char(it)) == '\n')
			fail_at(*it, "unexpected newline");
		if(!(--out_len))
			fail_at(*it, "string too long");
	}
	*out = 0, ++*it;
}

static int64_t read_number(const char **it) {
	char *end;
	int64_t num = strtoll(*it, &end, 0);
	if(end == *it)
		fail_at(*it, "expected number");
	if(errno == ERANGE)
		fail_at(*it, "failed to parse number");
	*it = end;
	return num;
}

static_assert(offsetof(struct Token, enum_.type) == offsetof(struct Token, field.type), "");
static_assert(offsetof(struct Token, enum_.name) == offsetof(struct Token, field.name), "");
[[nodiscard]] static const char *parse_struct_fields(const char *it, uint32_t indent, bool insideSwitch) {
	while(skip_indent_maybe(&it, indent)) {
		struct Token token = {
			.type = TType_Field,
			.field = {
				.count = {0},
				.maxCount = 1,
				.bitWidth = 0,
				.enumFlags = 0,
				.enumValue = 0,
			},
		};
		read_name(&it, token.field.type, sizeof(token.field.type));
		if(!insideSwitch && strcmp(token.field.type, "if") == 0) {
			token.type = TType_If_start;
			skip_char(&it, '(');
			read_scope(&it, token.if_.condition, sizeof(token.if_.condition), '(', ')');
			skip_char(&it, '\n');
			*tokens_end++ = token;
			it = parse_struct_fields(it, indent + 1, false);
			token.type = TType_If_end;
			*tokens_end++ = token;
			continue;
		}
		skip_char(&it, ' ');
		read_name(&it, token.field.name, sizeof(token.field.name));
		if(insideSwitch) {
			if(skip_char_maybe(&it, ' ')) {
				token.field.enumFlags = EnumFlags_HasValue;
				token.field.enumValue = read_number(&it);
			}
		} else if(skip_char_maybe(&it, '(')) {
			token.type = TType_Enum_start;
			read_scope(&it, token.enum_.switchField, sizeof(token.enum_.switchField), '(', ')');
			skip_char(&it, '\n');
			*tokens_end++ = token;
			it = parse_struct_fields(it, indent + 1, true);
			token.type = TType_Enum_end;
			*tokens_end++ = token;
			continue;
		} else if(skip_char_maybe(&it, '[')) {
			token.field.maxCount = read_number(&it);
			if(skip_char_maybe(&it, ','))
				read_scope(&it, token.field.count, sizeof(token.field.count), '[', ']');
			else
				skip_char(&it, ']');
		} else if(skip_char_maybe(&it, ':')) {
			token.field.bitWidth = read_number(&it);
		}
		skip_char(&it, '\n');
		*tokens_end++ = token;
	}
	return it;
}

[[nodiscard]] static const char *parse_struct(const char *it) {
	struct Token token;
	token.type = TType_Struct_start;
	token.struct_.sendIntern = token.struct_.send = (*it == 's' || *it == 'd');
	token.struct_.recvIntern = token.struct_.recv = (*it == 'r' || *it == 'd');
	++it, skip_char(&it, ' ');
	read_name(&it, token.struct_.name, sizeof(token.struct_.name));
	skip_char(&it, '\n');
	*tokens_end++ = token;
	it = parse_struct_fields(it, 1, false);
	token.type = TType_Struct_end;
	*tokens_end++ = token;
	return it;
}

[[nodiscard]] static const char *parse_enum(const char *it) {
	struct Token token = {
		.type = TType_Enum_start,
		.enum_ = {
			.switchField = {0},
		},
	};
	read_name(&it, token.enum_.type, sizeof(token.enum_.type));
	skip_char(&it, ' ');
	read_name(&it, token.enum_.name, sizeof(token.enum_.name));
	skip_char(&it, '\n');
	*tokens_end++ = token;
	while(skip_indent_maybe(&it, 1)) {
		struct Token fieldToken = {
			.type = TType_Field,
			.field = {
				.name = {0},
				.count = {0},
				.maxCount = 1,
				.bitWidth = 0,
				.enumValue = 0,
			},
		};
		read_name(&it, fieldToken.field.type, sizeof(fieldToken.field.type));
		fieldToken.field.enumFlags = skip_char_maybe(&it, ' ') ? EnumFlags_HasValue : 0;
		if(fieldToken.field.enumFlags & EnumFlags_HasValue)
			fieldToken.field.enumValue = read_number(&it);
		skip_char(&it, '\n');
		*tokens_end++ = fieldToken;
	}
	token.type = TType_Enum_end;
	*tokens_end++ = token;
	return it;
}

static void parse_file(const char *name);
static void parse(const char *it, const char *path, uint32_t path_len) {
	while(*it) {
		if((it[0] == 's' || it[0] == 'r' || it[0] == 'd' || it[0] == 'n') && it[1] == ' ') {
			it = parse_struct(it);
		} else if(alpha(*it)) {
			it = parse_enum(it);
		} else {
			const char *start = it;
			skip_line(&it);
			if(strncmp(start, "@include ", 9) == 0) {
				char name[8192];
				snprintf(name, sizeof(name), "%.*s%.*s", path_len, path, (uint32_t)(it - 10 - start), &start[9]);
				parse_file(name);
			}
		}
	}
}

static void parse_file(const char *name) {
	const char *oldFilename = filename;
	filename = name;
	FILE *infile = fopen(name, "rb");
	if(!infile)
		fail("File not found");
	fseek(infile, 0, SEEK_END);
	size_t infile_len = ftell(infile);
	char desc[infile_len+8];
	fileStart = desc;
	fseek(infile, 0, SEEK_SET);
	if(fread(desc, 1, infile_len, infile) != infile_len) {
		fclose(infile);
		fprintf(stderr, "Failed to read %s\n", name);
		exit(-1);
	}
	fclose(infile);
	for(uint8_t i = 0; i < 8; ++i)
		desc[infile_len + i] = 0;
	uint32_t path_len = 0;
	for(uint32_t i = 0; name[i];)
		if(name[i++] == '/')
			path_len = i;
	parse(desc, name, path_len);
	filename = oldFilename;
}

static void write_str(char **out, const char *str) {
	uint32_t len = strlen(str);
	memcpy(*out, str, len);
	*out += len;
}

static void write_fmt(char **out, const char *format, ...) {
	va_list args;
	va_start(args, format);
	*out += vsprintf(*out, format, args);
	va_end(args);
}
#define write_fmt_indent(out, level, format, ...) write_fmt(out, "%.*s" format, level, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t", __VA_ARGS__)
#define write_indent(out, level, str) write_fmt(out, "%.*s" str, level, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t")

#define TOKEN_LOOP(token) \
	for(struct Token *(token) = tokens; (token) < tokens_end; ++(token)) \
			switch((token)->type)
#define TOKEN_ITER(token) \
	for(; (token) < tokens_end; ++(token)) \
			switch((token)->type)

static void scan_duplicates(struct Token *token, const int64_t refValue) {
	int64_t enumValue = refValue;
	while((++token)->type == TType_Field) {
		enumValue = (token->field.enumFlags & EnumFlags_HasValue) ? token->field.enumValue : enumValue + 1;
		if(enumValue == refValue)
			token->field.enumFlags |= EnumFlags_IsDuplicate;
	}
}

struct {
	uint32_t count;
	char names[16384][64];
	char types[16384][64];
} static typedefs = {
	.count = 0,
};
static void resolve() {
	int64_t enumValue = 0;
	TOKEN_LOOP(token) {
		case TType_Enum_start: {
			enumValue = 0;
			memcpy(typedefs.types[typedefs.count], (token[1].type == TType_Enum_end) ? token->enum_.name : token->enum_.type, sizeof(*typedefs.types));
			memcpy(typedefs.names[typedefs.count], token->enum_.name, sizeof(*typedefs.names));
			++typedefs.count;
			break;
		}
		case TType_Field: {
			enumValue = (token->field.enumFlags & EnumFlags_HasValue) ? token->field.enumValue : enumValue + 1;
			scan_duplicates(token, enumValue);
			break;
		}
		default:;
	}
	const struct StructToken *st = NULL;
	for(struct Token *token = tokens_end - 1; token >= tokens; --token) {
		switch(token->type) {
			case TType_Struct_start: st = NULL; break;
			case TType_Struct_end: st = &token->struct_; break;
			case TType_Field: {
				if(!st)
					break;
				for(struct Token *t = token - 1; t >= tokens; --t) {
					if(t->type != TType_Struct_start && t->type != TType_Struct_end)
						continue;
					if(strcmp(t->struct_.name, token->field.type))
						continue;
					t->struct_.sendIntern |= st->sendIntern;
					t->struct_.recvIntern |= st->recvIntern;
					if(t->type == TType_Struct_start)
						break;
				}
				break;
			}
			default:;
		}
	}
}

static const char *StructType(const char *type) {
	static char buf[256] = "struct ";
	if(strcmp(type, "b") == 0) return "bool";
	if(strcmp(type, "i8") == 0) return "int8_t";
	if(strcmp(type, "u8") == 0) return "uint8_t";
	if(strcmp(type, "i16") == 0) return "int16_t";
	if(strcmp(type, "u16") == 0) return "uint16_t";
	if(strcmp(type, "i32") == 0 || strcmp(type, "vi32") == 0) return "int32_t";
	if(strcmp(type, "u32") == 0 || strcmp(type, "vu32") == 0) return "uint32_t";
	if(strcmp(type, "i64") == 0 || strcmp(type, "vi64") == 0) return "int64_t";
	if(strcmp(type, "u64") == 0 || strcmp(type, "vu64") == 0) return "uint64_t";
	if(strcmp(type, "f32") == 0) return "float";
	if(strcmp(type, "f64") == 0) return "double";
	for(uint32_t i = 0; i < typedefs.count; ++i)
		if(strcmp(type, typedefs.names[i]) == 0)
			return type;
	strncpy(&buf[7], type, sizeof(buf) - 7);
	return buf;
}

static const char *SerialType(const char *type) {
	for(uint32_t i = 0; i < typedefs.count; ++i)
		if(strcmp(type, typedefs.names[i]) == 0)
			return typedefs.types[i];
	return type;
}

static const char *sig_rdwr(const char *name, bool wr) {
	static char out[8192];
	snprintf(out, sizeof(out), "void _pkt_%s_%s(%sstruct %s *restrict data, %suint8_t **pkt, const uint8_t *end, struct PacketContext ctx)", name, wr ? "write" : "read", wr ? "const " : "", name, wr ? "" : "const ");
	return out;
}

static void gen_header_enum(char **out, struct Token *token) {
	uint32_t scope = 0;
	const char *enumName = token->enum_.name;
	write_fmt(out, "typedef %s %s;\n", StructType(token->enum_.type), enumName);
	if(token[1].type == TType_Enum_end)
		return;
	write_str(out, "enum {\n");
	TOKEN_ITER(token) {
		case TType_Enum_start: ++scope; break;
		case TType_Enum_end: {
			if(--scope)
				break;
			write_str(out, "};\n");
			return;
		}
		case TType_Field: {
			if(scope != 1)
				break;
			if(token->field.enumFlags & EnumFlags_HasValue) {
				if(token->field.enumValue > (~0u >> 1))
					write_fmt_indent(out, 1, "#define %s_%s %lldu\n", enumName, token->field.type, token->field.enumValue); // TODO: check subsequent values as well
				else
					write_fmt_indent(out, 1, "%s_%s = %lld,\n", enumName, token->field.type, token->field.enumValue);
			} else {
				write_fmt_indent(out, 1, "%s_%s,\n", enumName, token->field.type);
			}
			break;
		}
		default:;
	}
	fail("invalid token sequence");
}

static void gen_header_reflect(char **out, struct Token *token) {
	uint32_t scope = 0;
	const char *enumName = token->enum_.name;
	write_fmt(out, "[[maybe_unused]] static const char *_reflect_%s(%s value) {\n\tswitch(value) {\n", enumName, enumName);
	TOKEN_ITER(token) {
		case TType_Enum_start: ++scope; break;
		case TType_Enum_end: {
			if(--scope)
				break;
			write_indent(out, 2, "default: return \"???\";\n\t}\n}\n");
			return;
		}
		case TType_Field: {
			if(scope == 1 && (token->field.enumFlags & EnumFlags_IsDuplicate) == 0)
				write_fmt_indent(out, 2, "case %s_%s: return \"%s\";\n", enumName, token->field.type, token->field.type);
			break;
		}
		default:;
	}
	fail("invalid token sequence");
}

static void gen_header(char **out, const char *headerName) {
	write_fmt(out, "#pragma once\n#include <stddef.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include \"%s.h\"\n", headerName);
	TOKEN_LOOP(token) {
		case TType_Enum_start: {
			gen_header_enum(out, token);
			gen_header_reflect(out, token);
			break;
		}
		default:;
	}
	uint32_t indent = 0;
	TOKEN_LOOP(token) {
		case TType_Enum_start: if(*token->enum_.switchField) write_indent(out, indent++, "union {\n"); break;
		case TType_Enum_end: if(*token->enum_.switchField) write_indent(out, --indent, "};\n"); break;
		case TType_Struct_start: ++indent; write_fmt(out, "struct %s {\n%s", token->struct_.name, (token[1].type == TType_Struct_end) ? "\tuint8_t _empty;\n" : ""); break;
		case TType_Struct_end: --indent; write_fmt(out, "};\n"); break;
		case TType_Field:
		if(!indent)
			break;
		if(token->field.maxCount != 1)
			write_fmt_indent(out, indent, "%s %s[%hu];\n", StructType(token->field.type), token->field.name, token->field.maxCount);
		else
			write_fmt_indent(out, indent, "%s %s;\n", StructType(token->field.type), token->field.name);
		default:;
	}
	write_fmt(out,
		"static const struct PacketContext PV_LEGACY_DEFAULT = {\n"
		"\t.netVersion = 11,\n"
		"\t.protocolVersion = 6,\n"
		"\t.beatUpVersion = 0,\n"
		"\t.windowSize = 64,\n"
		"};\n");
	for(struct Token *token = tokens; token < tokens_end; ++token) {
		if(token->type != TType_Struct_start)
			continue;
		if(token->struct_.recv)
			write_fmt(out, "%s;\n", sig_rdwr(token->struct_.name, false));
		if(token->struct_.send)
			write_fmt(out, "%s;\n", sig_rdwr(token->struct_.name, true));
	}
	write_str(out,
		"#define reflect(type, value) _reflect_##type(value)\n"
		"typedef void (*PacketWriteFunc)(const void *restrict, uint8_t**, const uint8_t*, struct PacketContext);\n"
		"typedef void (*PacketReadFunc)(void *restrict, const uint8_t**, const uint8_t*, struct PacketContext);\n"
		"size_t _pkt_try_read(PacketReadFunc inner, void *restrict data, const uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);\n"
		"size_t _pkt_try_write(PacketWriteFunc inner, const void *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx);\n"
		"#define pkt_write_c(pkt, end, ctx, type, ...) _pkt_try_write((PacketWriteFunc)_pkt_##type##_write, &(struct type)__VA_ARGS__, pkt, end, ctx)\n"
		"#define pkt_read(data, ...) _pkt_try_read((PacketReadFunc)_Generic(*(data)");
	for(struct Token *token = tokens; token < tokens_end; ++token)
		if(token->type == TType_Struct_start && token->struct_.recv)
			write_fmt(out, ", struct %s: _pkt_%s_read", token->struct_.name, token->struct_.name);
	write_str(out,
		"), data, __VA_ARGS__)\n"
		"#define pkt_write(data, ...) _pkt_try_write((PacketWriteFunc)_Generic(*(data)");
	for(struct Token *token = tokens; token < tokens_end; ++token)
		if(token->type == TType_Struct_start && token->struct_.send)
			write_fmt(out, ", struct %s: _pkt_%s_write", token->struct_.name, token->struct_.name);
	write_str(out,
		"), data, __VA_ARGS__)\n"
		"size_t pkt_write_bytes(const uint8_t *restrict data, uint8_t **pkt, const uint8_t *end, struct PacketContext ctx, size_t count);\n");
}

static const char *scan_bitfield_type(const struct Token *it) {
	uint32_t bitWidth = 7;
	for(; it < tokens_end && it->type == TType_Field && it->field.bitWidth; ++it)
		bitWidth += it->field.bitWidth;
	switch(bitWidth / 8) {
		case 1: return "u8";
		case 2: return "u16";
		default: return "u32";
	}
}

static const char *fill_expr(const char *expr, const char *data, const char *ctx) {
	static char out[8192];
	char *out_end = out;
	size_t data_len = strlen(data), ctx_len = strlen(ctx), pad = data_len > ctx_len ? data_len : ctx_len;
	while(*expr == ' ')
		++expr;
	for(char prev = 0; *expr && out_end + pad - out < sizeof(out); prev = *expr++) {
		if(*expr == '.' && !alpha(prev))
			write_str(&out_end, data);
		else if(*expr == '$')
			write_str(&out_end, ctx);
		else
			*out_end++ = *expr;
	}
	*out_end = 0;
	return out;
}

static const char *FieldToken_get_count(const struct FieldToken *field, const char *structName) {
	static char out[8192];
	if(*field->count)
		snprintf(out, sizeof(out), "check_overflow(%s, %u, \"%s.%s\")", fill_expr(field->count, "data->", "ctx."), field->maxCount, structName, field->name);
	else
		snprintf(out, sizeof(out), "%u", field->maxCount);
	return out;
}

static void gen_source_rdwr(char **out, struct Token *token, bool wr, bool static_) {
	uint32_t scope = 0, indent = 1, bitName = ~0u, bitOffset = 0;
	const char *rdwr = wr ? "write" : "read", *structName = token->struct_.name, *switchName = NULL, *bitType = NULL;
	write_fmt(out, "%s%s {\n", static_ ? "static " : "", sig_rdwr(structName, wr));
	TOKEN_ITER(token) {
		case TType_Enum_start: switchName = token->enum_.name; write_fmt_indent(out, indent++, "switch(%s) {\n", fill_expr(token->enum_.switchField, "data->", "ctx.")); break;
		case TType_Enum_end: {
			switchName = NULL;
			write_fmt_indent(out, indent, "default: uprintf(\"Invalid value for enum `%s`\\n\"); longjmp(fail, 1);\n", token->enum_.name);
			write_indent(out, --indent, "}\n");
			break;
		}
		case TType_Struct_start: ++scope; break;
		case TType_Struct_end: {
			if(--scope)
				break;
			write_fmt(out, "}\n");
			return;
		}
		case TType_If_start: write_fmt_indent(out, indent++, "if(%s) {\n", fill_expr(token->if_.condition, "data->", "ctx.")); break;
		case TType_If_end: write_indent(out, --indent, "}\n"); break;
		case TType_Field: {
			if(scope != 1)
				break;
			if(token->field.bitWidth) {
				if(bitOffset == 0) {
					bitType = scan_bitfield_type(token);
					write_fmt_indent(out, indent, "%s bitfield%u%s;\n", StructType(bitType), ++bitName, wr ? " = 0" : "");
					if(!wr)
						write_fmt_indent(out, indent, "_pkt_%s_read(&bitfield%u, pkt, end, ctx);\n", bitType, bitName);
				}
				if(wr) {
					write_fmt_indent(out, indent, "bitfield%u |= (data->%s & %uu) << %u;\n", bitName, token->field.name, ~0llu >> (64 - token->field.bitWidth), bitOffset);
					if(token[1].type != TType_Field || token[1].field.bitWidth == 0)
						write_fmt_indent(out, indent, "_pkt_%s_write(&bitfield%u, pkt, end, ctx);\n", bitType, bitName);
				} else {
					write_fmt_indent(out, indent, "data->%s = bitfield%u >> %u & %u;\n", token->field.name, bitName, bitOffset, ~0llu >> (64 - token->field.bitWidth));
				}
				bitOffset += token->field.bitWidth;
			} else {
				bitOffset = 0;
				const char *stype = SerialType(token->field.type);
				if(switchName)
					write_fmt_indent(out, indent, "case %s_%s: ", switchName, token->field.type);
				if((strcmp(stype, "u8") == 0 || strcmp(stype, "i8") == 0) && token->field.maxCount != 1) {
					write_fmt_indent(out, switchName ? 0 : indent, "_pkt_raw_%s(data->%s, pkt, end, ctx, %s);%s", rdwr, token->field.name, FieldToken_get_count(&token->field, structName), switchName ? "" : "\n");
				} else {
					if(token->field.maxCount != 1)
						write_fmt_indent(out, switchName ? 0 : indent, "for(uint32_t i = 0, count = %s; i < count; ++i)%s", FieldToken_get_count(&token->field, structName), switchName ? " " : "\n\t");
					write_fmt_indent(out, switchName ? 0 : indent, "_pkt_%s_%s(&data->%s%s, pkt, end, ctx);%s", stype, rdwr, token->field.name, token->field.maxCount == 1 ? "" : "[i]", switchName ? "" : "\n");
				}
				if(switchName)
					write_str(out, " break;\n");
			}
			break;
		}
		default:;
	}
	fail("invalid token sequence");
}

static void gen_source(char **out, const char *headerName, const char *sourceName) {
	write_fmt(out, "#include \"%s\"\n", headerName);
	write_fmt(out, "#include \"%s.h\"\n", sourceName);
	TOKEN_LOOP(token) {
		case TType_Struct_start: {
			if(token->struct_.recvIntern)
				gen_source_rdwr(out, token, false, !token->struct_.recv);
			if(token->struct_.sendIntern)
				gen_source_rdwr(out, token, true, !token->struct_.send);
			break;
		}
		default:;
	}
}

int32_t main(int32_t argc, const char *argv[]) {
	if(argc == 5 && strcmp(argv[4], "-l") == 0) {
		enableLog = true;
		uprintf("Logging functions not yet implemented\n");
		return -1;
	} else if(argc != 4) {
		uprintf("Usage: %s <definition.txt> <output.h> <output.c> [-l]", argv[0]);
		return -1;
	}
	parse_file(argv[1]);
	parse("n PacketContext\n\tu8 netVersion\n\tu8 protocolVersion\n\tu8 beatUpVersion\n\tu32 windowSize\n", NULL, 0);
	resolve();
	const char *headerName = argv[2], *sourceName = argv[3];
	for(const char *it = headerName; *it;)
		if(*it++ == '/')
			headerName = it;
	for(const char *it = sourceName; *it;)
		if(*it++ == '/')
			sourceName = it;
	char output_h[524288], *output_h_end = output_h;
	gen_header(&output_h_end, headerName);
	char output_c[524288], *output_c_end = output_c;
	gen_source(&output_c_end, headerName, sourceName);

	filename = argv[2];
	FILE *outfile = fopen(argv[2], "wb");
	bool res = (fwrite(output_h, 1, output_h_end - output_h, outfile) != output_h_end - output_h);
	fclose(outfile);
	if(res)
		fail("Failed to write\n");

	filename = argv[3];
	outfile = fopen(argv[3], "wb");
	res = (fwrite(output_c, 1, output_c_end - output_c, outfile) != output_c_end - output_c);
	fclose(outfile);
	if(res)
		fail("Failed to write\n");
	return 0;
}
