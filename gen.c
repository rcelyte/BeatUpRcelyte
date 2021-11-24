#!/usr/bin/tcc -run
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *desc_buf;

const char *header_init =
	"/* \n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" */\n\n"
	"#include \"enum.h\"\n"
	"#include <stdint.h>\n";

const char *header_base =
	"#define SERIALIZE_HEAD(pkt, msg, mtype) { \\\n"
	"\tstruct MessageHeader _message = (msg); \\\n"
	"\t_message.type = MessageType_##mtype; \\\n"
	"\tpkt_writeMessageHeader(pkt, _message); \\\n"
	"}\n"
	"#define SERIALIZE_BODY(pkt, stype, dtype, data) { \\\n"
	"\tfprintf(stderr, \"serialize \" #stype \"\\n\"); \\\n"
	"\tuint8_t *_end = *(pkt); \\\n"
	"\tpkt_write##dtype(&_end, data); \\\n"
	"\tstruct SerializeHeader _serial; \\\n"
	"\t_serial.length = _end + 1 - *(pkt); \\\n"
	"\t_serial.type = stype; \\\n"
	"\tpkt_writeSerializeHeader(pkt, _serial); \\\n"
	"\tpkt_write##dtype(pkt, data); \\\n"
	"}\n"
	"#define SERIALIZE(pkt, msg, mtype, stype, dtype, data) { \\\n"
	"\tfprintf(stderr, \"serialize \" #mtype \"Type_\" #stype \"\\n\"); \\\n"
	"\tuint8_t *_end = *(pkt); \\\n"
	"\tpkt_write##dtype(&_end, data); \\\n"
	"\tstruct MessageHeader _message = (msg); \\\n"
	"\t_message.type = MessageType_##mtype; \\\n"
	"\tstruct SerializeHeader _serial; \\\n"
	"\t_serial.length = _end + 1 - *(pkt); \\\n"
	"\t_serial.type = mtype##Type_##stype; \\\n"
	"\tpkt_writeMessageHeader(pkt, _message); \\\n"
	"\tpkt_writeSerializeHeader(pkt, _serial); \\\n"
	"\tpkt_write##dtype(pkt, data); \\\n"
	"}\n"
	"struct PacketEncryptionLayer {\n"
	"\t_Bool encrypted;\n"
	"\tuint32_t sequenceId;\n"
	"};\n"
	"struct NetPacketHeader {\n"
	"\tPacketType property;\n"
	"\tuint8_t connectionNumber;\n"
	"\t_Bool isFragmented;\n"
	"\tuint16_t sequence;\n"
	"\tuint8_t channelId;\n"
	"\tuint16_t fragmentId;\n"
	"\tuint16_t fragmentPart;\n"
	"\tuint16_t fragmentsTotal;\n"
	"};\n"
	"void pkt_writeUint8Array(uint8_t **pkt, uint8_t *in, uint32_t count);"
	"struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt);\n"
	"void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in);\n"
	"struct NetPacketHeader pkt_readNetPacketHeader(uint8_t **pkt);\n"
	"void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in);\n";

const char *code_init =
	"/* \n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" * AUTO GENERATED; DO NOT TOUCH\n"
	" */\n\n"
	"#include \"packets.h\"\n"
	"#include <stdio.h>\n"
	"#include <stdlib.h>\n"
	"#include <string.h>\n\n"
	"static uint8_t pkt_readUint8(uint8_t **pkt) {\n"
	"\tuint8_t v = (*pkt)[0];\n"
	"\t*pkt += sizeof(v);\n"
	"\treturn v;\n"
	"}\n"
	"static uint16_t pkt_readUint16(uint8_t **pkt) {\n"
	"\tuint16_t v = (*pkt)[0] | (*pkt)[1] << 8;\n"
	"\t*pkt += sizeof(v);\n"
	"\treturn v;\n"
	"}\n"
	"static uint32_t pkt_readUint32(uint8_t **pkt) {\n"
	"\tuint32_t v = (*pkt)[0] | (*pkt)[1] << 8 | (*pkt)[2] << 16 | (*pkt)[3] << 24;\n"
	"\t*pkt += sizeof(v);\n"
	"\treturn v;\n"
	"}\n"
	"static uint64_t pkt_readUint64(uint8_t **pkt) {\n"
	"\tuint64_t v = (uint64_t)(*pkt)[0] | (uint64_t)(*pkt)[1] << 8 | (uint64_t)(*pkt)[2] << 16 | (uint64_t)(*pkt)[3] << 24 | (uint64_t)(*pkt)[4] << 32 | (uint64_t)(*pkt)[5] << 40 | (uint64_t)(*pkt)[6] << 48 | (uint64_t)(*pkt)[7] << 56;\n"
	"\t*pkt += sizeof(v);\n"
	"\treturn v;\n"
	"}\n"
	/*"static uint64_t pkt_readVarUint64(uint8_t **pkt) {\n"
	"\tuint64_t byte, value = 0;\n"
	"\tuint32_t shift = 0;\n"
	"\tfor(; (byte = (uint64_t)pkt_readUint8(pkt)) & 128; shift += 7)\n"
	"\t\tvalue |= (byte & 127) << shift;\n"
	"\treturn value | byte << shift;\n"
	"}\n"*/
	"static uint64_t pkt_readVarUint64(uint8_t **pkt) {\n"
	"\tuint64_t byte, value = 0;\n"
	"\tuint8_t shift = 0;\n"
	"\tdo {\n"
	"\t\tbyte = pkt_readUint8(pkt);\n"
	"\t\tvalue |= (byte & 127) << shift;\n"
	"\t\tshift += 7;\n"
	"\t} while(byte & 128);\n"
	"\treturn value;\n"
	"}\n"
	"static int64_t pkt_readVarInt64(uint8_t **pkt) {\n"
	"\tint64_t varULong = (int64_t)pkt_readVarUint64(pkt);\n"
	"\tif((varULong & 1L) != 1L)\n"
	"\t\treturn varULong >> 1;\n"
	"\treturn -(varULong >> 1) + 1L;\n"
	"}\n"
	"static uint32_t pkt_readVarUint32(uint8_t **pkt) {\n"
	"\treturn (uint32_t)pkt_readVarUint64(pkt);\n"
	"}\n"
	"static int32_t pkt_readVarInt32(uint8_t **pkt) {\n"
	"\treturn (int32_t)pkt_readVarInt64(pkt);\n"
	"}\n"
	"#define pkt_readInt8Array(pkt, out, count) pkt_readUint8Array(pkt, (uint8_t*)out, count)\n"
	"static void pkt_readUint8Array(uint8_t **pkt, uint8_t *out, uint32_t count) {\n"
	"\tmemcpy(out, *pkt, count);\n"
	"\t*pkt += count;\n"
	"}\n"
	"static void pkt_writeUint8(uint8_t **pkt, uint8_t v) {\n"
	"\t(*pkt)[0] = v;\n"
	"\t*pkt += sizeof(v);\n"
	"}\n"
	"static void pkt_writeUint16(uint8_t **pkt, uint16_t v) {\n"
	"\t(*pkt)[0] = v & 255;\n"
	"\t(*pkt)[1] = v >> 8 & 255;\n"
	"\t*pkt += sizeof(v);\n"
	"}\n"
	"static void pkt_writeUint32(uint8_t **pkt, uint32_t v) {\n"
	"\t(*pkt)[0] = v & 255;\n"
	"\t(*pkt)[1] = v >> 8 & 255;\n"
	"\t(*pkt)[2] = v >> 16 & 255;\n"
	"\t(*pkt)[3] = v >> 24 & 255;\n"
	"\t*pkt += sizeof(v);\n"
	"}\n"
	"static void pkt_writeUint64(uint8_t **pkt, uint64_t v) {\n"
	"\t(*pkt)[0] = v & 255;\n"
	"\t(*pkt)[1] = v >> 8 & 255;\n"
	"\t(*pkt)[2] = v >> 16 & 255;\n"
	"\t(*pkt)[3] = v >> 24 & 255;\n"
	"\t(*pkt)[4] = v >> 32 & 255;\n"
	"\t(*pkt)[5] = v >> 40 & 255;\n"
	"\t(*pkt)[6] = v >> 48 & 255;\n"
	"\t(*pkt)[7] = v >> 56 & 255;\n"
	"\t*pkt += sizeof(v);\n"
	"}\n"
	"static void pkt_writeVarUint64(uint8_t **pkt, uint64_t v) {\n"
	"\tdo {\n"
	"\t\tuint8_t byte = v & 127;\n"
	"\t\tv >>= 7;\n"
	"\t\tif(v)\n"
	"\t\t\tbyte |= 128;\n"
	"\t\tpkt_writeUint8(pkt, byte);\n"
	"\t} while(v);\n"
	"}\n"
	"static void pkt_writeVarInt64(uint8_t **pkt, int64_t v) {\n"
	"\tif(v < 0)\n"
	"\t\treturn pkt_writeVarUint64(pkt, (-(v + 1L) << 1) + 1L);\n"
	"\tpkt_writeVarUint64(pkt, v << 1);\n"
	"}\n"
	"static void pkt_writeVarUint32(uint8_t **pkt, uint32_t v) {\n"
	"\tpkt_writeVarUint64(pkt, v);\n"
	"}\n"
	"static void pkt_writeVarInt32(uint8_t **pkt, int32_t v) {\n"
	"\tpkt_writeVarInt64(pkt, v);\n"
	"}\n"
	"#define pkt_writeInt8Array(pkt, out, count) pkt_writeUint8Array(pkt, (uint8_t*)out, count)\n"
	"void pkt_writeUint8Array(uint8_t **pkt, uint8_t *in, uint32_t count) {\n"
	"\tmemcpy(*pkt, in, count);\n"
	"\t*pkt += count;\n"
	"}\n"
	"struct PacketEncryptionLayer pkt_readPacketEncryptionLayer(uint8_t **pkt) {\n"
	"\tstruct PacketEncryptionLayer out;\n"
	"\tout.encrypted = pkt_readUint8(pkt);\n"
	"\tif(out.encrypted) {\n"
	"\t\tfprintf(stderr, \"READ ENCRYPTED PACKET\\n\");\n"
	"\t\tabort();\n"
	"\t}\n"
	"\treturn out;\n"
	"}\n"
	"void pkt_writePacketEncryptionLayer(uint8_t **pkt, struct PacketEncryptionLayer in) {\n"
	"\tpkt_writeUint8(pkt, in.encrypted);\n"
	"\tif(in.encrypted) {\n"
	"\t\tfprintf(stderr, \"WRITE ENCRYPTED PACKET\\n\");\n"
	"\t\tabort();\n"
	"\t}\n"
	"}\n"
	"static const uint8_t NetPacketHeaderSize[32] = {\n"
	"\t1, 4, 4, 3, 11, 14, 11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\n"
	"};\n"
	"struct NetPacketHeader pkt_readNetPacketHeader(uint8_t **pkt) {\n"
	"\tstruct NetPacketHeader out;\n"
	"\tuint8_t bits = pkt_readUint8(pkt);\n"
	"\tout.property = bits & 31;\n"
	"\tout.connectionNumber = bits >> 5 & 3;\n"
	"\tout.isFragmented = bits >> 7 & 1;\n"
	"\tout.sequence = (NetPacketHeaderSize[out.property] < 3) ? 0 : pkt_readUint16(pkt);\n"
	"\tout.channelId = (NetPacketHeaderSize[out.property] < 4) ? 0 : pkt_readUint8(pkt);\n"
	"\tout.fragmentId = (NetPacketHeaderSize[out.property] < 6) ? 0 : pkt_readUint16(pkt);\n"
	"\tout.fragmentPart = (NetPacketHeaderSize[out.property] < 8) ? 0 : pkt_readUint16(pkt);\n"
	"\tout.fragmentsTotal = (NetPacketHeaderSize[out.property] < 10) ? 0 : pkt_readUint16(pkt);\n"
	"\treturn out;\n"
	"}\n"
	"void pkt_writeNetPacketHeader(uint8_t **pkt, struct NetPacketHeader in) {\n"
	"\tpkt_writeUint8(pkt, in.property | in.connectionNumber << 5 | in.isFragmented << 7);\n"
	"\tif(NetPacketHeaderSize[in.property] >= 3) pkt_writeUint16(pkt, in.sequence);\n"
	"\tif(NetPacketHeaderSize[in.property] >= 4) pkt_writeUint8(pkt, in.channelId);\n"
	"\tif(NetPacketHeaderSize[in.property] >= 6) pkt_writeUint16(pkt, in.fragmentId);\n"
	"\tif(NetPacketHeaderSize[in.property] >= 8) pkt_writeUint16(pkt, in.fragmentPart);\n"
	"\tif(NetPacketHeaderSize[in.property] >= 10) pkt_writeUint16(pkt, in.fragmentsTotal);\n"
	"}\n";

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
	while(lim && (alpha(*s) || numeric(*s)))
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
			out += sprintf(out, "struct ");
			read_word(in, out, lim - 7);
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
	if(!*tin)
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
	char type[1024], name[1024];
	s = read_word(s, type, sizeof(type));
	typename(type, type, sizeof(type), "uint32_t");
	s = skip_char(s, ' ');
	s = read_word(s, name, sizeof(name) - 4);
	if(*s == ' ') {
		char ev[1024];
		s = read_word(s+1, ev, sizeof(ev));
		enum_write(name, ev);
	} else {
		enum_write(name, NULL);
	}
	s = skip_char(s, '\n');

	sprintf(&name[strlen(name)], "Type");
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
			s = parse_struct_entries(s, indent+1, outdent+1, des, ser);
			if(des)
				des_it += sprintf(des_it, "%.*s}\n", outdent, tabs);
			if(ser)
				ser_it += sprintf(ser_it, "%.*s}\n", outdent, tabs);
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
		}
	}
	return s;
}
char *parse_struct(char *s, uint32_t indent) {
	_Bool des = (*s != 's');
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
	}

	s = parse_struct_entries(s, indent, 1, des, ser);

	struct_it += sprintf(struct_it, "};\n");
	if(des)
		des_it += sprintf(des_it, "\treturn out;\n}\n");
	if(ser)
		ser_it += sprintf(ser_it, "}\n");
	return s;
}
char *parse_block(char *s, uint32_t indent) {
	if(strncmp(s, "d ", 2) == 0 || strncmp(s, "r ", 2) == 0 || strncmp(s, "s ", 2) == 0)
		return parse_struct(s, indent);
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
	trywrite(header, "src/packets.h", struct_buf, struct_it);
	trywrite(header, "src/packets.h", dec_buf, dec_it);
	trywrite(header, "src/packets.h", header_base, &header_base[strlen(header_base)]);
	FILE *code = tryopen("src/packets.c", "w");
	trywrite(code, "src/packets.c", code_init, &code_init[strlen(code_init)]);
	trywrite(code, "src/packets.c", des_buf, des_it);
	trywrite(code, "src/packets.c", ser_buf, ser_it);
	fprintf(stderr, "done\n");
	return 0;
}
