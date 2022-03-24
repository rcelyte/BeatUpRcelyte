#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

_Bool enableLog = 0;

_Bool alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

_Bool numeric(char c) {
	return c >= '0' && c <= '9';
}

const char *desc = NULL, *descName = NULL, *source_buf = NULL;
void fail(const char *pos, const char *format, ...) {
	va_list args0, args1;
	va_start(args0, format);
	va_copy(args1, args0);
	char buf[vsnprintf(NULL, 0, format, args0) + 1];
	vsnprintf(buf, sizeof(buf), format, args1);
	va_end(args0);
	va_end(args1);

	uint32_t line = 1, column = 1;
	for(const char *it = desc; it < pos; ++it) {
		if(*it == '\n')
			++line, column = 1;
		else
			++column;
	}
	fprintf(stderr, "%s:%u:%u: %s\n", descName, line, column, buf);
	exit(-1);
}

void skip_line(const char **s) {
	while(**s && **s != '\n')
		++(*s);
	if(**s != '\n')
		fail(*s, "expected newline");
	++(*s);
}

void skip_char(const char **s, char c) {
	if(**s != c) {
		if(c == '\n')
			fail(*s, "expected newline");
		else if(c == '\t')
			fail(*s, "expected indent");
		else
			fail(*s, "expected '%c'", c);
	}
	++(*s);
}

_Bool skip_char_maybe(const char **s, char c) {
	if(**s != c)
		return 0;
	++(*s);
	return 1;
}

const char *skip_number(const char **s) {
	const char *start = *s;
	if(**s == '-')
		++(*s);
	if(!numeric(**s))
		fail(*s, "expected number");
	while(numeric(**s))
		++(*s);
	return start;
}

_Bool skip_tabs_maybe(const char **s, uint32_t count) {
	for(uint32_t i = 0; i < count; ++i)
		if((*s)[i] != '\t')
			return 0;
	*s += count;
	return 1;
}

_Bool skip_string_maybe(const char **s, const char *str) {
	size_t len = strlen(str);
	if(strncmp(*s, str, len))
		return 0;
	*s += len;
	return 1;
}

_Bool read_word(const char **in, char *out) {
	*out = 0;
	if(!alpha(**in))
		return 1;
	while(alpha(**in) || numeric(**in) || **in == '.')
		*out++ = *(*in)++;
	*out = 0;
	return 0;
}

void read_expr(const char **s, char *out) {
	while(alpha(**s) || numeric(**s) || **s == '.' || **s == '+' || **s == '-' || **s == '*' || **s == '/')
		*out++ = *(*s)++;
	*out = 0;
}

enum TypeType {
	TYPE_SERIAL,
	TYPE_DATA_ENUM,
	TYPE_DATA,
};
typedef uint8_t fieldwidth_t;
#define EPHEMERAL_BIT 128
fieldwidth_t read_type(const char **in, char *out, enum TypeType mode) {
	const char *start = *in;
	_Bool var = (**in == 'v');
	if(var)
		++(*in);
	char type = *(*in)++;
	const char *size = *in;
	if(type) {
		for(uint8_t i = 0; i < 2; ++i)
			if(numeric(**in))
				++(*in);
		if(numeric(**in) || alpha(**in) || (type & ~127))
			type = 0;
	}
	switch(type | ((mode == TYPE_SERIAL) ? 128 : 0)) { // double switch!
		case 'b': sprintf(out, "_Bool"); return 0;
		case 'c': sprintf(out, "char"); return 0;
		case 'i': sprintf(out, "int%u_t", atoi(size)); return 0;
		case 'u': sprintf(out, "uint%u_t", atoi(size)); return 0;
		case 'f': sprintf(out, (atoi(size) == 32) ? "float" : "double"); return 0;
		case 'z': sprintf(out, "void"); return 0;
		case 'b' | 128: sprintf(out, "Uint8"); return 8;
		case 'c' | 128: sprintf(out, "Int8"); return 8;
		case 'i' | 128: sprintf(out, var ? "VarInt%u" : "Int%u", atoi(size)); return atoi(size);
		case 'u' | 128: sprintf(out, var ? "VarUint%u" : "Uint%u", atoi(size)); return atoi(size);
		case 'f' | 128: sprintf(out, "Float%u", atoi(size)); return atoi(size);
		case 'z' | 128: *out = 0; return 0;
		default:;
	}
	*in = start;
	if(mode == TYPE_DATA)
		out += sprintf(out, "struct ");
	while(numeric(**in) || alpha(**in))
		*out++ = *(*in)++;
	*out = 0;
	return 0;
}

uint8_t read_type_double(const char **in, char *serial, char *data, char *log) {
	const char *start = *in;
	fieldwidth_t width = read_type(in, serial, TYPE_SERIAL);
	if(skip_char_maybe(in, '.')) {
		const char *start2 = *in;
		read_type(in, data, TYPE_DATA_ENUM);
		if(strcmp(data, "e")) {
			read_type(&start2, log, TYPE_SERIAL);
			return width;
		}
		width |= 128;
	}
	const char *start2 = start;
	read_type(&start, data, TYPE_DATA);
	read_type(&start2, log, TYPE_SERIAL);
	return width;
}

void write_fmt(char **out, const char *format, ...) {
	va_list args;
	va_start(args, format);
	*out += vsprintf(*out, format, args);
	va_end(args);
}

void write_warning(char **out) {
	write_fmt(out,
		"/* \n"
		" * AUTO GENERATED; DO NOT TOUCH\n"
		" * AUTO GENERATED; DO NOT TOUCH\n"
		" * AUTO GENERATED; DO NOT TOUCH\n"
		" */\n\n");
}

char **tabs(char **s, uint32_t count) {
	for(uint32_t i = 0; i < count; ++i)
		*(*s)++ = '\t';
	return s;
}

struct HeaderData {
	char *enums;
	char *structs;
	char *funcs;
};

struct EnumEntry {
	char name[128];
	const char *value;
} parse_value(struct HeaderData *header, char **source, const char **in, uint32_t indent);

const void *memmem(const uint8_t *ptr, const uint8_t *sub, size_t count, size_t subcount) {
	for(; count >= subcount; ++ptr, --count) {
		for(const uint8_t *a = ptr, *b = sub; b < &sub[subcount]; ++a, ++b)
			if(*a != *b)
				goto next;
		return ptr;
		next:
	}
	return NULL;
}

void write_ifsub(char **out, const char *str, uint32_t len, const char *sub) {
	write_fmt(out, "if(");
	for(--str; len; --len) {
		if(str[1] == '.' && (str[0] == '(' || str[0] == ' '))
			write_fmt(out, "%s", sub);
		*(*out)++ = *++str;
	}
	write_fmt(out, ") {\n");
}

void parse_struct_entries(const char **in, const char *structName, uint32_t indent, uint32_t outdent, const char *parent, char *def_start, char **def, char **des, char **ser, char **log) {
	uint32_t offset = 0;
	while(skip_tabs_maybe(in, indent)) {
		if(skip_string_maybe(in, "if(")) {
			if(parent)
				fail(*in, "bitfields cannot contain conditionals");
			const char *start = *in;
			for(uint32_t pc = 1; **in && pc && **in != '\n'; ++(*in)) {
				pc += (**in == '(');
				pc -= (**in == ')');
			}
			uint32_t len = *in - start - 1;
			skip_char(in, '\n');

			if(*des)
				write_ifsub(tabs(des, outdent), start, len, "out");
			if(*ser)
				write_ifsub(tabs(ser, outdent), start, len, "in");
			if(*log)
				write_ifsub(tabs(log, outdent), start, len, "in");
			parse_struct_entries(in, structName, indent + 1, outdent + 1, parent, def_start, def, des, ser, log);
			if(*des)
				*des += sprintf(*tabs(des, outdent), "}\n");
			if(*ser)
				*ser += sprintf(*tabs(ser, outdent), "}\n");
			if(*log)
				*log += sprintf(*tabs(log, outdent), "}\n");
		} else {
			char serialType[128], dataType[128], logType[128], name[128], length[128] = "out.";
			uint32_t count = 0;
			_Bool rangecheck = 0, lengthField = 0;
			fieldwidth_t width = read_type_double(in, serialType, dataType, logType);
			if(!skip_char_maybe(in, '\n')) {
				if(skip_char_maybe(in, '[')) {
					count = atoll(skip_number(in));
					if(skip_char_maybe(in, ',')) {
						lengthField = skip_char_maybe(in, '.');
						read_expr(in, &length[lengthField ? 4 : 0]), rangecheck = 1;
					} else {
						sprintf(length, "%u", count);
					}
					skip_char(in, ']');
				}
				skip_char(in, ' ');
				if(read_word(in, name))
					fail(*in, "expected member name");
				skip_char(in, '\n');

				if((width & EPHEMERAL_BIT) == 0) {
					char *tempDef = *def, *cmp = &(*def)[strlen(dataType)+1];
					if(count)
						tempDef += sprintf(tempDef, "\t%s %s[%u];\n", dataType, name, count);
					else
						tempDef += sprintf(tempDef, "\t%s %s;\n", dataType, name);
					if(memmem(def_start, cmp, *def - def_start, tempDef - cmp) == NULL)
						*def = tempDef;
				}

				if(*des) {
					if(rangecheck) {
						char length_name[1024];
						read_word((const char*[]){length}, length_name);
						*des += sprintf(*tabs(des, outdent), "if(%s > %u) {\n", length, count);
						*des += sprintf(*tabs(des, outdent + 1), "uprintf(\"Buffer overflow in read of %s.%s: %%u > %u\\n\", (uint32_t)%s)", structName, name, count, length);
						if(lengthField)
							*des += sprintf(*des, ", %s = 0", length_name);
						*des += sprintf(*des, ", *pkt = _trap;\n");
						*des += sprintf(*tabs(des, outdent++), "} else {\n");
					}
					if(count && width == 8) {
						*des += sprintf(*tabs(des, outdent), "pkt_read%sArray(pkt, out.%s, %s);\n", serialType, name, length);
					} else {
						if(width & EPHEMERAL_BIT) {
							*des += sprintf(*tabs(des, outdent), "%s %s", dataType, name);
						} else {
							if(count) {
								*des += sprintf(*tabs(des, outdent), "for(uint32_t i = 0; i < %s; ++i)\n", length);
								*des += sprintf(*tabs(des, outdent + 1), "out.%s[i]", name);
							} else {
								*des += sprintf(*tabs(des, outdent), "out.%s", name);
							}
						}
						if(parent)
							*des += sprintf(*des, " = (%s >> %u) & %u;\n", parent, offset, (1 << width) - 1);
						else
							*des += sprintf(*des, " = pkt_read%s(ctx, pkt);\n", serialType);
					}
					if(rangecheck)
						*des += sprintf(*tabs(des, --outdent), "}\n");
				}

				char *length_in = length;
				if(rangecheck && lengthField) {
					length_in = &length[1];
					memcpy(length_in, "in.", 3);
				}
				if(*log && (width & EPHEMERAL_BIT) == 0) {
					if(count && width == 8) {
						*log += sprintf(*tabs(log, outdent), "pkt_log%sArray(ctx, \"%s\", buf, it, in.%s, %s);\n", logType, name, name, length_in);
					} else {
						if(count)
							*log += sprintf(*tabs(log, outdent), "for(uint32_t i = 0; i < %s; ++i)\n\t", length_in);
						*log += sprintf(*tabs(log, outdent), "pkt_log%s(ctx, \"%s%s\", buf, it, in.%s%s);\n", logType, name, count ? "[]" : "", name, count ? "[i]" : "");
					}
				}

				if(*ser && (width & EPHEMERAL_BIT))
					*ser += sprintf(*tabs(ser, outdent), "%s %s = 0;\n", dataType, name);

				parse_struct_entries(in, structName, indent + 1, outdent, name, def_start, def, des, ser, log);

				if(*ser) {
					if(parent) {
						*ser += sprintf(*tabs(ser, outdent), "%s |= (in.%s << %u);\n", parent, name, offset);
					} else {
						if(count && width == 8) {
							*ser += sprintf(*tabs(ser, outdent), "pkt_write%sArray(ctx, pkt, %s%s, %s);\n", serialType, (width & EPHEMERAL_BIT) ? "" : "in.", name, length_in);
						} else {
							if(count)
								*ser += sprintf(*tabs(ser, outdent), "for(uint32_t i = 0; i < %s; ++i)\n\t", length_in);
							*ser += sprintf(*tabs(ser, outdent), "pkt_write%s(ctx, pkt, %s%s%s);\n", serialType, (width & EPHEMERAL_BIT) ? "" : "in.", name, count ? "[i]" : "");
						}
					}
				}
			}
			offset += width & ~EPHEMERAL_BIT;
		}
	}
	(void)offset;
}

struct EnumEntry parse_struct(struct HeaderData *header, char **source, const char **in, uint32_t indent) {
	struct EnumEntry self = {"", NULL};
	char des[131072] = {0}, *des_end = (**in == 'r' || **in == 'd' || enableLog) ? des : NULL;
	char ser[131072] = {0}, *ser_end = (**in == 's' || **in == 'd') ? ser : NULL;
	char log[131072] = {0}, *log_end = NULL;
	*in += 2;
	if(read_word(in, self.name))
		fail(*in, "expected struct name");
	if(skip_char_maybe(in, ' '))
		self.value = skip_number(in);
	skip_char(in, '\n');

	if(des_end) {
		char fn[1024];
		sprintf(fn, "%s pkt_read%s(", self.name, self.name);
		if(strstr(source_buf, fn))
			des_end = NULL;
	}
	if(enableLog) {
		char fn[1024];
		sprintf(fn, "void pkt_log%s(", self.name);
		if(strstr(source_buf, fn) == 0)
			log_end = log;
	}

	if(!(des_end || ser_end || log_end))
		return self;

	char def[131072], *def_end = def;
	def_end += sprintf(def_end, "struct %s {\n", self.name);
	parse_struct_entries(in, self.name, indent + 1, 1, NULL, def, &def_end, &des_end, &ser_end, &log_end);
	header->structs += sprintf(header->structs, "%s};\n", def);

	if(des_end) {
		header->funcs += sprintf(header->funcs, "struct %s pkt_read%s(struct PacketContext ctx, const uint8_t **pkt);\n", self.name, self.name);
		if(des_end == des)
			*source += sprintf(*source, "struct %s pkt_read%s(struct PacketContext ctx, const uint8_t **pkt) {\n\treturn (struct %s){};\n}\n", self.name, self.name, self.name);
		else
			*source += sprintf(*source, "struct %s pkt_read%s(struct PacketContext ctx, const uint8_t **pkt) {\n\tstruct %s out;\n%s\treturn out;\n}\n", self.name, self.name, self.name, des);
	}
	if(ser_end) {
		header->funcs += sprintf(header->funcs, "void pkt_write%s(struct PacketContext ctx, uint8_t **pkt, struct %s in);\n", self.name, self.name);
		*source += sprintf(*source, "void pkt_write%s(struct PacketContext ctx, uint8_t **pkt, struct %s in) {\n%s}\n", self.name, self.name, ser);
	}
	if(log_end) {
		header->funcs += sprintf(header->funcs, "void pkt_log%s(struct PacketContext ctx, const char *name, char *buf, char *it, struct %s in);\n", self.name, self.name);
		*source += sprintf(*source, "void pkt_log%s(struct PacketContext ctx, const char *name, char *buf, char *it, struct %s in) {\n\tit += sprintf(it, \"%%s.\", name);\n%s}\n", self.name, self.name, log);
	}
	return self;
}

struct EnumEntry parse_enum(struct HeaderData *header, char **source, const char **in, uint32_t indent) {
	struct EnumEntry self = {"", NULL};
	char type[128], suffix[128] = {0};
	read_type(in, type, TYPE_DATA_ENUM);
	skip_char(in, ' ');
	if(read_word(in, self.name))
		fail(*in, "expected enum name");
	if(skip_char_maybe(in, ' ')) {
		if(read_word(in, suffix))
			fail(*in, "expected enum suffix");
	}
	if(skip_char_maybe(in, ' '))
		self.value = skip_number(in);
	skip_char(in, '\n');

	char buf[131072], *buf_end = buf;
	buf_end += sprintf(buf_end, "ENUM(%s, %s%s, {\n", type, self.name, suffix);
	char *start = buf_end;
	while(skip_tabs_maybe(in, indent + 1)) {
		struct EnumEntry e = parse_value(header, source, in, indent + 1);
		if(*e.name) {
			if(e.value)
				buf_end += sprintf(buf_end, "\t%s%s_%s = %lld,\n", self.name, suffix, e.name, atoll(e.value));
			else
				buf_end += sprintf(buf_end, "\t%s%s_%s,\n", self.name, suffix, e.name);
		}
	}
	if(start == buf_end)
		header->enums += sprintf(header->enums, "typedef %s %s%s;\n", type, self.name, suffix);
	else
		header->enums += sprintf(header->enums, "%s})\n", buf);
	if(enableLog) {
		char fn[1024];
		sprintf(fn, "void pkt_log%s%s(", self.name, suffix);
		if(strstr(source_buf, fn) == 0) {
			*source += sprintf(*source, "void pkt_log%s%s(struct PacketContext ctx, const char *name, char *buf, char *it, %s%s in) {", self.name, suffix, self.name, suffix);
			if(start != buf_end)
				*source += sprintf(*source, "\n\tuprintf(\"%%.*s%%s=%%u (%%s)\\n\", (uint32_t)(it - buf), buf, name, in, reflect(%s%s, in));\n", self.name, suffix);
			*source += sprintf(*source, "}\n");
		}
	}
	return self;
}

void parse_extra(char **out, const char **in, uint32_t indent) {
	while(skip_tabs_maybe(in, indent)) {
		const char *start = *in;
		skip_line(in);
		*out += sprintf(*out, "%.*s", (uint32_t)(*in - start), start);
	}
}
void parse_source_extra(char **source, const char **in, uint32_t indent) {}

struct EnumEntry parse_value(struct HeaderData *header, char **source, const char **in, uint32_t indent) {
	struct EnumEntry e = {"", NULL};
	if((**in == 'r' || **in == 's' || **in == 'd' || **in == 'n') && (*in)[1] == ' ') {
		return parse_struct(header, source, in, indent);
	} else if(strncmp(*in, "u8 ", 3) == 0 || strncmp(*in, "u16 ", 4) == 0 || strncmp(*in, "u32 ", 4) == 0 || strncmp(*in, "i32 ", 4) == 0) {
		return parse_enum(header, source, in, indent);
	} else if(skip_string_maybe(in, "z ")) {
		if(read_word(in, e.name))
			fail(*in, "expected name");
		if(skip_char_maybe(in, ' '))
			e.value = skip_number(in);
		skip_char(in, '\n');
	} else if(skip_string_maybe(in, "enum\n")) {
		parse_extra(&header->enums, in, indent + 1);
		return parse_value(header, source, in, indent);
	} else if(skip_string_maybe(in, "head\n")) {
		parse_extra(&header->funcs, in, indent + 1);
		return parse_value(header, source, in, indent);
	} else if(skip_string_maybe(in, "code\n")) {
		parse_extra(source, in, indent + 1);
		return parse_value(header, source, in, indent);
	} else {
		skip_line(in);
	}
	return e;
}

int main(int argc, char const *argv[]) {
	if(argc == 5 && strcmp(argv[4], "-l") == 0) {
		enableLog = 1;
	} else if(argc != 4) {
		fprintf(stderr, "Usage: %s <definition.txt> <output.h> <output.c> [-l]", argv[0]);
		return 1;
	}
	FILE *infile = fopen(argv[1], "rb");
	fseek(infile, 0, SEEK_END);
	size_t infile_len = ftell(infile);
	char in[infile_len+1];
	const char *in_end = in;
	fseek(infile, 0, SEEK_SET);
	if(fread(in, 1, infile_len, infile) != infile_len) {
		fclose(infile);
		fprintf(stderr, "Failed to read %s\n", argv[1]);
		return -1;
	}
	fclose(infile);
	desc = in, descName = argv[1];
	in[infile_len] = 0;

	char header_enums[524288];
	char header_structs[524288];
	char header_funcs[196608];
	struct HeaderData header_end = {
		.enums = header_enums,
		.structs = header_structs,
		.funcs = header_funcs,
	};
	write_fmt(&header_end.enums, "#ifndef PACKETS_H\n#define PACKETS_H\n\n");
	write_warning(&header_end.enums);
	write_fmt(&header_end.enums, "#include \"log.h\"\n");
	write_fmt(&header_end.enums, "#include \"enum.h\"\n");
	write_fmt(&header_end.enums, "#include <stdint.h>\n\n");
	if(enableLog)
		write_fmt(&header_end.enums, "#define PACKET_LOGGING_FUNCS\n\n");
	write_fmt(&header_end.enums,
		"struct PacketContext {\n"
		"\tuint8_t netVersion;\n"
		"\tuint8_t protocolVersion;\n"
		"\tuint8_t windowSize;\n"
		"\tuint8_t beatUpVersion;\n"
		"};\n"
		"#define PV_LEGACY_DEFAULT (struct PacketContext){11, 6, 64, 0}\n\n");

	char source[524288], *source_end = source;
	source_buf = source;
	write_warning(&source_end);
	write_fmt(&source_end, "#include \"enum_reflection.h\"\n");
	write_fmt(&source_end, "#include \"packets.h\"\n");
	write_fmt(&source_end, "static const uint8_t _trap[128] = {~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,};\n");

	while(*in_end)
		parse_value(&header_end, &source_end, &in_end, 0);

	write_fmt(&header_end.funcs, "#endif // PACKETS_H\n");

	FILE *outfile = fopen(argv[2], "wb");
	if(fwrite(header_enums, 1, header_end.enums - header_enums, outfile) != header_end.enums - header_enums ||
	   fwrite(header_structs, 1, header_end.structs - header_structs, outfile) != header_end.structs - header_structs ||
	   fwrite(header_funcs, 1, header_end.funcs - header_funcs, outfile) != header_end.funcs - header_funcs) {
		fclose(outfile);
		fprintf(stderr, "Failed to write to %s\n", argv[2]);
		return -1;
	}
	fclose(outfile);

	outfile = fopen(argv[3], "wb");
	if(fwrite(source, 1, source_end - source, outfile) != source_end - source) {
		fclose(outfile);
		fprintf(stderr, "Failed to write to %s\n", argv[3]);
		return -1;
	}
	fclose(outfile);
	return 0;
}
