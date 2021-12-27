#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static _Bool enableLog = 0;

char *desc_buf;

const char *warning =
	"/* \n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" */\n\n";

const char *header_loginit =
	"#define PACKET_LOGGING_FUNCS\n";

const char *header_init =
	"#pragma once\n\n"
	"#include \"enum.h\"\n"
	"#include <stdint.h>\n";

const char *code_loginit =
	"#include \"enum_reflection.h\"\n";

const char *code_init =
	"#include \"packets.h\"\n";

FILE *tryopen(const char *path, const char *mode) {
	FILE *f = fopen(path, mode);
	if(f)
		return f;
	fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
	exit(-1);
}
void trywrite(FILE *f, const char *name, const char *start, const char *end) {
	uint32_t len = end - start;
	if(fwrite(start, 1, len, f) == len)
		return;
	fprintf(stderr, "Failed to write to %s\n", name);
	exit(-1);
}
_Bool alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
_Bool numeric(char c) {
	return c >= '0' && c <= '9';
}
const char *read_word(const char *s, char *out, uint32_t lim) {
	--lim;
	while(lim && (alpha(*s) || numeric(*s) || *s == '.'))
		--lim, *out++ = *s++;
	*out = 0;
	return s;
}
const char *skip_line(const char *s) {
	while(*s && *s != '\n')
		++s;
	if(*s == '\n')
		++s;
	return s;
}
uint32_t count_tabs(const char *s) {
	for(uint32_t c = 0; 1; ++c, ++s)
		if(*s != '\t')
			return c;
}
const char *skip_char(const char *s, char c) {
	if(*s == c)
		return s+1;
	uint32_t line = 1, column = 1;
	for(char *it = desc_buf; it < s; ++it) {
		if(*it == '\n')
			++line, column = 1;
		else
			++column;
	}
	fprintf(stderr, "src/packets.txt:%u:%u: expected '%c'\n", line, column, c);
	exit(-1);
}
_Bool typename(char *in, char *out, uint32_t lim, const char *de) {
	char *tin = in;
	if(*tin == 'v')
		++tin;
	char type = *tin++;
	uint32_t size = atoll(tin);
	for(uint8_t i = 0; numeric(*tin) && i < 2; ++i)
		++tin;
	if(*tin) {
		if(de) {
			sprintf(out, "%s", de);
		} else {
			if(*tin == '.') {
				read_word(tin+1, out, lim);
				return type != 't';
			} else {
				out += sprintf(out, "struct ");
				read_word(in, out, lim - 7);
			}
		}
		return 0;
	}
	if(type == 'b') {
		sprintf(out, "_Bool");
	} else if(type == 'c') {
		sprintf(out, "char");
	} else if(type == 'i') {
		sprintf(out, "int%u_t", size);
	} else if(type == 'u') {
		sprintf(out, "uint%u_t", size);
	} else if(type == 'z') {
		sprintf(out, "void");
	} else {
		if(de) {
			sprintf(out, "%s", de);
		} else {
			out += sprintf(out, "struct ");
			read_word(in, out, lim - 7);
		}
	}
	return 0;
}
_Bool fnname(char *in, char *out, uint32_t lim) {
	char *tin = in;
	if(*tin == 'v')
		++tin;
	char type = *tin++;
	uint32_t size = atoll(tin);
	for(uint8_t i = 0; numeric(*tin) && i < 2; ++i)
		++tin;
	if(type == 'b') {
		sprintf(out, "Uint8");
	} else if(type == 'c') {
		sprintf(out, "Int8");
	} else if(type == 'i') {
		sprintf(out, "%sInt%u", (*in == 'v') ? "Var" : "", size);
	} else if(type == 'u') {
		sprintf(out, "%sUint%u", (*in == 'v') ? "Var" : "", size);
	} else if(type == 't' && *tin == '.') {
		read_word(tin+1, out, lim);
		return 0;
	} else {
		read_word(in, out, lim);
		return 0;
	}
	if(!*tin || *tin == '.')
		return type == 'c' || (type == 'u' && size == 8);
	read_word(in, out, lim);
	return 0;
}

const char *parse_block(const char *s, uint32_t indent);

char head_buf[524288], *head_it = head_buf;
char code_buf[524288], *code_it = code_buf;
const char *parse_custom(const char *s) {
	char **it = (*s == 'h') ? &head_it : &code_it;
	s += 5;
	while(*s == '\t') {
		++s;
		while(*s && *s != '\n')
			*(*it)++ = *s++;
		*(*it)++ = '\n';
		s = skip_char(s, '\n');
	}
	return s;
}

char enum_buf[524288], *enum_it = enum_buf;
char *currentEnum = NULL, **currentEnum_it = NULL;
void enum_write(const char *name, const char *ev) {
	if(!currentEnum)
		return;
	if(ev)
		*currentEnum_it += sprintf(*currentEnum_it, "\t%s_%s = %s,\n", currentEnum, name, ev);
	else
		*currentEnum_it += sprintf(*currentEnum_it, "\t%s_%s,\n", currentEnum, name);
}
const char *parse_enum(const char *s, uint32_t indent) {
	++indent;
	char buf[131072], *buf_it = buf;
	char type[1024], name[1024], suffix[1024];
	s = read_word(s, type, sizeof(type));
	typename(type, type, sizeof(type), "uint32_t");
	s = skip_char(s, ' ');
	s = read_word(s, name, sizeof(name) - 4);
	if(*s == ' ' && alpha(s[1]))
		s = read_word(s+1, suffix, sizeof(suffix));
	else
		*suffix = 0;
	if(*s == ' ') {
		char ev[1024];
		s = read_word(s+1, ev, sizeof(ev));
		enum_write(name, ev);
	} else {
		enum_write(name, NULL);
	}
	s = skip_char(s, '\n');

	sprintf(&name[strlen(name)], "%s", suffix);
	buf_it += sprintf(buf_it, "ENUM(%s, %s, {\n", type, name);
	if(count_tabs(s) == indent) {
		while(count_tabs(s) == indent) {
			s += indent;
			currentEnum = name, currentEnum_it = &buf_it;
			s = parse_block(s, indent);
		}
		buf_it += sprintf(buf_it, "})\n");
		currentEnum = NULL;
		memcpy(enum_it, buf, buf_it - buf);
		enum_it += buf_it - buf;
	}
	return s;
}
char log_buf[524288], *log_it = log_buf;
char dec_buf[524288], *dec_it = dec_buf;
char des_buf[524288], *des_it = des_buf;
char ser_buf[524288], *ser_it = ser_buf;
char struct_buf[524288], *struct_it = struct_buf;
static const char *tabs = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
const char *parse_struct_entries(const char *s, const char *structName, uint32_t indent, uint32_t outdent, _Bool des, _Bool ser, _Bool log) {
	while(count_tabs(s) == indent) {
		s += indent;
		if(strncmp(s, "if(", 3) == 0) {
			s += 3;
			const char *cond = s;
			while(*s && *s != ')' && *s != '\n')
				++s;
			uint32_t len = s - cond;
			s = skip_char(s, ')');
			s = skip_char(s, '\n');
			if(des)
				des_it += sprintf(des_it, "%.*sif(out.%.*s) {\n", outdent, tabs, len, cond);
			if(ser)
				ser_it += sprintf(ser_it, "%.*sif(in.%.*s) {\n", outdent, tabs, len, cond);
			if(log)
				log_it += sprintf(log_it, "%.*sif(in.%.*s) {\n", outdent, tabs, len, cond);
			s = parse_struct_entries(s, structName, indent+1, outdent+1, des, ser, log);
			if(des)
				des_it += sprintf(des_it, "%.*s}\n", outdent, tabs);
			if(ser)
				ser_it += sprintf(ser_it, "%.*s}\n", outdent, tabs);
			if(log)
				log_it += sprintf(log_it, "%.*s}\n", outdent, tabs);
		} else {
			uint32_t count = 0;
			char type[1024], name[1024], length[1024] = {0};
			_Bool rangecheck = 0;
			s = read_word(s, type, sizeof(type));
			if(*s == '[') {
				++s; // [
				if(alpha(*s)) {
					rangecheck = 1;
					s = read_word(s, length, sizeof(length));
					s = skip_char(s, ',');
				}
				count = atoll(s);
				while(numeric(*s))
					++s;
				if(!*length)
					sprintf(length, "%u", count);
				s = skip_char(s, ']');
			}
			s = skip_char(s, ' ');
			s = read_word(s, name, sizeof(name));
			s = skip_char(s, '\n');

			char stype[1024], ftype[1024];
			_Bool isEnum = typename(type, stype, sizeof(stype), NULL);
			_Bool isU8Array = fnname(type, ftype, sizeof(ftype));
			if(count)
				struct_it += sprintf(struct_it, "\t%s %s[%u];\n", stype, name, count);
			else
				struct_it += sprintf(struct_it, "\t%s %s;\n", stype, name);
			if(des) {
				if(rangecheck) {
					des_it += sprintf(des_it, "%.*sif(out.%s > %u) {\n", outdent, tabs, length, count);
					des_it += sprintf(des_it, "\t%.*sfprintf(stderr, \"Buffer overflow in read of %s.%s: %%u > %u\\n\", (uint32_t)out.%s);\n", outdent, tabs, structName, name, count, length);
					des_it += sprintf(des_it, "\t%.*sout.%s = 0;\n", outdent, tabs, length);
					des_it += sprintf(des_it, "%.*s}\n", outdent, tabs);
				}
				if(count && isU8Array) {
					des_it += sprintf(des_it, "%.*spkt_read%sArray(pkt, out.%s, %s%s);\n", outdent, tabs, ftype, name, alpha(*length) ? "out." : "", length);
				} else {
					if(count)
						des_it += sprintf(des_it, "%.*sfor(uint32_t i = 0; i < %s%s; ++i)\n\t", outdent, tabs, alpha(*length) ? "out." : "", length);
					des_it += sprintf(des_it, "%.*sout.%s%s = pkt_read%s(pkt);\n", outdent, tabs, name, count ? "[i]" : "", ftype);
				}
			}
			if(ser) {
				if(count && isU8Array) {
					ser_it += sprintf(ser_it, "%.*spkt_write%sArray(pkt, in.%s, %s%s);\n", outdent, tabs, ftype, name, alpha(*length) ? "in." : "", length);
				} else {
					if(count)
						ser_it += sprintf(ser_it, "%.*sfor(uint32_t i = 0; i < %s%s; ++i)\n\t", outdent, tabs, alpha(*length) ? "in." : "", length);
					ser_it += sprintf(ser_it, "%.*spkt_write%s(pkt, in.%s%s);\n", outdent, tabs, ftype, name, count ? "[i]" : "");
				}
			}
			if(log) {
				if(count && isU8Array) {
					log_it += sprintf(log_it, "%.*spkt_log%sArray(\"%s\", buf, it, in.%s, %s%s);\n", outdent, tabs, ftype, name, name, alpha(*length) ? "in." : "", length);
				} else {
					if(count)
						log_it += sprintf(log_it, "%.*sfor(uint32_t i = 0; i < %s%s; ++i)\n\t", outdent, tabs, alpha(*length) ? "in." : "", length);
					if(isEnum)
						log_it += sprintf(log_it, "%.*sfprintf(stderr, \"%%.*s%s=%%s\\n\", (uint32_t)(it - buf), buf, reflect(%s, in.%s%s));\n", outdent, tabs, name, stype, name, count ? "[i]" : "");
					else
						log_it += sprintf(log_it, "%.*spkt_log%s(\"%s\", buf, it, in.%s%s);\n", outdent, tabs, ftype, name, name, count ? "[i]" : "");
				}
			}
		}
	}
	return s;
}
const char *parse_struct(const char *s, uint32_t indent) {
	_Bool des = (*s != 's') || enableLog;
	_Bool ser = (*s != 'r');
	_Bool log = enableLog;
	++indent, s += 2;
	char name[1024];
	{
		s = read_word(s, name, sizeof(name));
		if(*s == ' ') {
			char ev[1024];
			s = read_word(s+1, ev, sizeof(ev));
			enum_write(name, ev);
		} else {
			enum_write(name, NULL);
		}
		s = skip_char(s, '\n');

		if(log) {
			char fn[1024];
			sprintf(fn, "void pkt_log%s", name);
			*code_it = 0;
			if(strstr(code_buf, fn))
				log = 0;
		}

		struct_it += sprintf(struct_it, "struct %s {\n", name);
		if(des) {
			dec_it += sprintf(dec_it, "struct %s pkt_read%s(const uint8_t **pkt);\n", name, name);
			des_it += sprintf(des_it, "struct %s pkt_read%s(const uint8_t **pkt) {\n\tstruct %s out;\n", name, name, name);
		}
		if(ser) {
			dec_it += sprintf(dec_it, "void pkt_write%s(uint8_t **pkt, struct %s in);\n", name, name);
			ser_it += sprintf(ser_it, "void pkt_write%s(uint8_t **pkt, struct %s in) {\n", name, name);
		}
		if(log) {
			dec_it += sprintf(dec_it, "void pkt_log%s(const char *name, char *buf, char *it, struct %s in);\n", name, name);
			log_it += sprintf(log_it, "void pkt_log%s(const char *name, char *buf, char *it, struct %s in) {\n\tit += sprintf(it, \"%%s.\", name);\n", name, name);
		}
	}

	const char *start = s;
	s = parse_struct_entries(s, name, indent, 1, des, ser, log);

	struct_it += sprintf(struct_it, "};\n");
	if(des) {
		if(s == start)
			des_it += sprintf(des_it, "\tout = (struct %s){};\n", name); // fixes -Wuninitialized in Clang
		des_it += sprintf(des_it, "\treturn out;\n}\n");
	}
	if(ser)
		ser_it += sprintf(ser_it, "}\n");
	if(log)
		log_it += sprintf(log_it, "}\n");
	return s;
}

const char *parse_block(const char *s, uint32_t indent) {
	if(strncmp(s, "d ", 2) == 0 || strncmp(s, "r ", 2) == 0 || strncmp(s, "s ", 2) == 0)
		return parse_struct(s, indent);
	if(strncmp(s, "head\n", 5) == 0 || strncmp(s, "code\n", 5) == 0)
		return parse_custom(s);
	if(alpha(*s))
		return parse_enum(s, indent);
	return skip_line(s);
}

int main(int argc, char const *argv[]) {
	if(argc == 4 && strcmp(argv[3], "-l") == 0) {
		enableLog = 1;
		--argc;
	}
	if(argc != 3)
		return 1;
	FILE *in = tryopen(argv[1], "r");
	fseek(in, 0, SEEK_END);
	size_t flen = ftell(in);
	char *desc = malloc(flen + 1);
	if(!desc) {
		fprintf(stderr, "alloc error\n");
		return 1;
	}
	desc_buf = desc;
	char *end = &desc[flen];
	fseek(in, 0, SEEK_SET);
	if(fread(desc, 1, flen, in) != flen) {
		fprintf(stderr, "Failed to read %s\n", argv[1]);
		return -1;
	}
	*end = 0;
	fclose(in);

	for(const char *it = desc; *it;)
		it = parse_block(it, 0);

	FILE *out = tryopen(argv[2], "w");
	if(argv[2][strlen(argv[2])-1] == 'h') {
		trywrite(out, argv[2], warning, &warning[strlen(warning)]);
		if(enableLog)
			trywrite(out, argv[2], header_loginit, &header_loginit[strlen(header_loginit)]);
		trywrite(out, argv[2], header_init, &header_init[strlen(header_init)]);
		trywrite(out, argv[2], enum_buf, enum_it);
		trywrite(out, argv[2], head_buf, head_it);
		trywrite(out, argv[2], struct_buf, struct_it);
		trywrite(out, argv[2], dec_buf, dec_it);
	} else {
		trywrite(out, argv[2], warning, &warning[strlen(warning)]);
		if(enableLog)
			trywrite(out, argv[2], code_loginit, &code_loginit[strlen(code_loginit)]);
		trywrite(out, argv[2], code_init, &code_init[strlen(code_init)]);
		trywrite(out, argv[2], code_buf, code_it);
		trywrite(out, argv[2], des_buf, des_it);
		trywrite(out, argv[2], ser_buf, ser_it);
		trywrite(out, argv[2], log_buf, log_it);
	}
	fclose(out);
	return 0;
}
