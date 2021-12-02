#!/usr/bin/tcc -run
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const _Bool log = 1;

char *desc_buf;

const char *header_init =
	"#pragma once\n\n"
	"/* \n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" */\n\n"
	"#include \"enum.h\"\n"
	"#include <stdint.h>\n";

const char *code_init =
	"/* \n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" */\n\n"
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
char *read_word(char *s, char *out, uint32_t lim) {
	--lim;
	while(lim && (alpha(*s) || numeric(*s) || *s == '.'))
		--lim, *out++ = *s++;
	*out = 0;
	return s;
}
char *skip_line(char *s) {
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
char *skip_char(char *s, char c) {
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
void typename(char *in, char *out, uint32_t lim, const char *de) {
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
			} else {
				out += sprintf(out, "struct ");
				read_word(in, out, lim - 7);
			}
		}
		return;
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
		return;
	}
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
	} else {
		read_word(in, out, lim);
		return 0;
	}
	if(!*tin || *tin == '.')
		return type == 'c' || (type == 'u' && size == 8);
	read_word(in, out, lim);
	return 0;
}
char *parse_block(char *s, uint32_t indent);
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
char *parse_enum(char *s, uint32_t indent) {
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

	sprintf(&name[strlen(name)], suffix);
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
char *parse_struct_entries(char *s, uint32_t indent, uint32_t outdent, _Bool des, _Bool ser) {
	while(count_tabs(s) == indent) {
		s += indent;
		if(strncmp(s, "if(", 3) == 0) {
			s += 3;
			char *cond = s;
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
			s = parse_struct_entries(s, indent+1, outdent+1, des, ser);
			if(des)
				des_it += sprintf(des_it, "%.*s}\n", outdent, tabs);
			if(ser)
				ser_it += sprintf(ser_it, "%.*s}\n", outdent, tabs);
			if(log)
				log_it += sprintf(log_it, "%.*s}\n", outdent, tabs);
		} else {
			uint32_t count = 0;
			char type[1024], name[1024], length[1024] = {0};
			s = read_word(s, type, sizeof(type));
			if(*s == '[') {
				++s; // [
				if(alpha(*s)) {
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
			typename(type, stype, sizeof(stype), NULL);
			_Bool isU8Array = fnname(type, ftype, sizeof(ftype));
			if(count)
				struct_it += sprintf(struct_it, "\t%s %s[%u];\n", stype, name, count);
			else
				struct_it += sprintf(struct_it, "\t%s %s;\n", stype, name);
			if(des) {
				if(count && isU8Array) {
					des_it += sprintf(des_it, "%.*spkt_read%sArray(pkt, out.%s, %s%s);\n", outdent, tabs, ftype, name, alpha(*length) ? "out." : "", length);
				} else {
					if(count)
						des_it += sprintf(des_it, "%.*sif(%s%s <= %u)\n\t%.*sfor(uint32_t i = 0; i < %s%s; ++i)\n\t\t", outdent, tabs, alpha(*length) ? "out." : "", length, count, outdent, tabs, alpha(*length) ? "out." : "", length);
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
					log_it += sprintf(log_it, "%.*spkt_log%s(\"%s\", buf, it, in.%s%s);\n", outdent, tabs, ftype, name, name, count ? "[i]" : "");
				}
			}
		}
	}
	return s;
}
char *parse_struct(char *s, uint32_t indent) {
	_Bool des = (*s != 's') || log;
	_Bool ser = (*s != 'r');
	++indent, s += 2;
	{
		char name[1024];
		s = read_word(s, name, sizeof(name));
		if(*s == ' ') {
			char ev[1024];
			s = read_word(s+1, ev, sizeof(ev));
			enum_write(name, ev);
		} else {
			enum_write(name, NULL);
		}
		s = skip_char(s, '\n');

		struct_it += sprintf(struct_it, "struct %s {\n", name);
		if(des) {
			dec_it += sprintf(dec_it, "struct %s pkt_read%s(uint8_t **pkt);\n", name, name);
			des_it += sprintf(des_it, "struct %s pkt_read%s(uint8_t **pkt) {\n\tstruct %s out;\n", name, name, name);
		}
		if(ser) {
			dec_it += sprintf(dec_it, "void pkt_write%s(uint8_t **pkt, struct %s in);\n", name, name);
			ser_it += sprintf(ser_it, "void pkt_write%s(uint8_t **pkt, struct %s in) {\n", name, name);
		}
		if(log) {
			dec_it += sprintf(dec_it, "void pkt_log%s(const char *name, char *buf, char *it, struct %s in);\n", name, name);
			log_it += sprintf(log_it, "void pkt_log%s(const char *name, char *buf, char *it, struct %s in) {\n\tit += sprintf(it, \"%%s.\", name);\n", name, name, name);
		}
	}

	s = parse_struct_entries(s, indent, 1, des, ser);

	struct_it += sprintf(struct_it, "};\n");
	if(des)
		des_it += sprintf(des_it, "\treturn out;\n}\n");
	if(ser)
		ser_it += sprintf(ser_it, "}\n");
	if(log)
		log_it += sprintf(log_it, "}\n");
	return s;
}

char head_buf[524288], *head_it = head_buf;
char code_buf[524288], *code_it = code_buf;
char *parse_custom(char *s) {
	char **it = (*s == 'h') ? &head_it : &code_it;
	s += 5;
	while(*s == '\t') {
		++s;
		while(*s && *s != '\n')
			*(*it)++ = *s++;
		*(*it)++ = '\n';
		s = skip_char(s, '\n');
	}
}

char *parse_block(char *s, uint32_t indent) {
	if(strncmp(s, "d ", 2) == 0 || strncmp(s, "r ", 2) == 0 || strncmp(s, "s ", 2) == 0)
		return parse_struct(s, indent);
	if(strncmp(s, "head\n", 5) == 0 || strncmp(s, "code\n", 5) == 0)
		return parse_custom(s);
	if(alpha(*s))
		return parse_enum(s, indent);
	return skip_line(s);
}

int main(int argc, char const *argv[]) {
	FILE *in = tryopen("src/packets.txt", "r");
	fseek(in, 0, SEEK_END);
	char desc[ftell(in)+1];
	desc_buf = desc;
	char *end = &desc[sizeof(desc)-1];
	fseek(in, 0, SEEK_SET);
	if(fread(desc, 1, sizeof(desc)-1, in) != sizeof(desc)-1) {
		fprintf(stderr, "Failed to read src/packets.txt\n");
		return -1;
	}
	*end = 0;
	fclose(in);

	for(char *it = desc; *it;)
		it = parse_block(it, 0);

	FILE *header = tryopen("src/packets.h", "w");
	trywrite(header, "src/packets.h", header_init, &header_init[strlen(header_init)]);
	trywrite(header, "src/packets.h", enum_buf, enum_it);
	trywrite(header, "src/packets.h", head_buf, head_it);
	trywrite(header, "src/packets.h", struct_buf, struct_it);
	trywrite(header, "src/packets.h", dec_buf, dec_it);
	FILE *code = tryopen("src/packets.c", "w");
	trywrite(code, "src/packets.c", code_init, &code_init[strlen(code_init)]);
	trywrite(code, "src/packets.c", code_buf, code_it);
	trywrite(code, "src/packets.c", des_buf, des_it);
	trywrite(code, "src/packets.c", ser_buf, ser_it);
	trywrite(code, "src/packets.c", log_buf, log_it);
	fprintf(stderr, "done\n");
	return 0;
}
