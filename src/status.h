#include <string.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))

static uint32_t status_head(char *buf, const char *code, const char *mime, size_t len) {
	return sprintf(buf,
		"HTTP/1.1 %s\r\n"
		"Connection: close\r\n"
		"Content-Length: %zd\r\n"
		"X-Frame-Options: DENY\r\n"
		"X-Content-Type-Options: nosniff\r\n"
		"Content-Type: %s; charset=utf-8\r\n"
		"X-DNS-Prefetch-Control: off\r\n"
		"X-Robots-Tag: noindex\r\n"
		"\r\n", code, len, mime);
}
static uint32_t status_bin(char *buf, const char *code, const char *mime, const uint8_t *resp, uint32_t resp_len) {
	uint32_t head_len = status_head(buf, code, mime, resp_len);
	memcpy(&buf[head_len], resp, resp_len);
	return head_len + resp_len;
}
#define status_text(buf, code, mime, resp) status_bin(buf, code, mime, (const uint8_t*)resp, sizeof(resp)-sizeof(char))

struct TEMPList {
	ServerCode code;
	char levelName[256];
	uint32_t playerCount, playerCap;
	float playerNPS, levelNPS;
} static tempList[2] = {{
	.code = 40549761,
	.levelName = "In-Game Room Listing",
	.playerCount = 0,
	.playerCap = 0xffffffff,
	.playerNPS = 0,
	.levelNPS = 0,
}, {
	.code = 385792,
	.levelName = "rcelyte ''Serializers'' Summary & VIPs 02 [RCCD-0017] (C94) - 11. rcelyte — Exit This Thread's Stack Space (rcelyte's ''OVERFLOWING--200MB'' Generator)",
	.playerCount = 0,
	.playerCap = 10,
	.playerNPS = 14.52,
	.levelNPS = 16.33,
}};

_Static_assert(sizeof(char) == 1, "no.");
static size_t escape(char *out, size_t limit, const char *in, size_t in_len) {
	char *start = out;
	if(in_len)
		*out++ = ((*in & 192) == 128) ? *in & 127 : *in; // prevents buffer underrun in handling maliciously crafted strings
	for(size_t i = 1; i < in_len; ++i) {
		if(i + 5 + 3 > limit) {
			if((in[i] & 192) == 128) { // remove incomplete UTF-8 character
				--out;
				while((*out & 192) == 128)
					--out;
			}
			*out++ = 0xE2; *out++ = 0x80; *out++ = 0xA6;
			break;
		}
		switch(in[i]) {
			case '>': *out++ = '&'; *out++ = 'g'; *out++ = 't'; *out++ = ';'; break;
			case '<': *out++ = '&'; *out++ = 'l'; *out++ = 't'; *out++ = ';'; break;
			case '&': *out++ = '&'; *out++ = 'a'; *out++ = 'm'; *out++ = 'p'; *out++ = ';'; break;
			default: *out++ = in[i];
		}
	}
	return out - start;
}

static uint32_t status_web(char *buf, ServerCode code) {
	char page[65536];
	uint32_t len = sprintf(page,
		"<!DOCTYPE html>"
		"<html>"
		"<head>"
			"<meta charset=\"utf-8\">"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			"<title>Server Status</title>"
			"<link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">"
			"<style>"
				"@keyframes sl{from{background-position:0 0}to{background-position:1470px 504px}}"
				"@keyframes sc{from{transform:translate(0,-200%%)}to{transform:translate(-50%%,-200%%)}}"
				"@keyframes hint{13.33%%{max-width:6ch;padding:11pt 4pt 11pt 11pt}70%%{max-width:6ch;padding:11pt 4pt 11pt 11pt}to{}}"
				"body:before{content:\"\";position:fixed;width:150vmax;height:150vmax;top:50%%;left:50%%;z-index:-1;background:url(data:image/png;base64,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) 0 0 repeat;background-size:735px;-webkit-transform:translate(-50%%,-50%%) rotate(30deg);-moz-transform:translate(-50%%,-50%%) rotate(30deg);-ms-transform:translate(-50%%,-50%%) rotate(30deg);-o-transform:translate(-50%%,-50%%) rotate(30deg);transform:translate(-50%%,-50%%) rotate(30deg);animation:sl 120s linear infinite}"
				"#main th,#main td{background:#222;margin:4pt;padding:0;overflow:hidden}"
				"#main>thead th,#main a{padding:0 8pt}"
				"#main a{color:#fff;text-decoration:none;display:block}"
				".ln{height:29pt;overflow:hidden}"
				".ln>div{float:left;animation:sc 6s linear infinite}"
				".ln>div,.ln>span{white-space:nowrap}"
				".ln>span:before{content:\"\u00A0\";float:right}"
				"#h>span{float:left;text-align:right;font:14pt/18pt sans-serif;max-width:0;padding:11pt 0;overflow:hidden;transition:.2s ease-in;animation:hint 1.5s ease-in-out}"
				"@media (hover:hover) {#h:hover>span{max-width:6ch;padding:11pt 4pt 11pt 11pt}}"
				"@media (max-width:900px){#main th:nth-child(6){display:none}}");
	len += sprintf(&page[len],
				"@media (min-width:651px){#ph{width:8ch}}"
				"@media (max-width:650px){#main th:nth-child(2),#main td:nth-child(2){display:none}}"
				"@media (max-width:460px){#main th:nth-child(5){display:none}}"
			"</style>"
		"</head>"
		"<body style=\"background:#000;color:#fff;font:12pt/29pt sans-serif;margin:0\">"
			"<a href=\"/\" id=\"h\" style=\"color:#fff;text-decoration:none;white-space:nowrap\">"
				"%s", code ? "<span><svg viewBox=\"0 0 8 9\" height=\"9pt\"><path fill=\"#fff\" d=\"M6,0L2,4L6,8Z\"/></svg> <span style=\"font-style:italic\">back</span></span>" : "");
	len += sprintf(&page[len],
				"<img style=\"float:left;padding:7pt;height:26pt;\" src=\"data:image/png;base64,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"); len += sprintf(&page[len], "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"); len += sprintf(&page[len], "4N1KPNGOUh66C1MUY9XrwUlddjFGvxyLHHsao14P3nkt5pe/khFEefYj3MP5A3oFsT/nq2lWaPMrj913eePooj0eOgkOuHPJcUdbw1F8UN/6s1zuQDSmvoQn5ALKdbpl6698xc5SVPKQtbTx3lLUc3XLdMaKsZ+EUyRifIh9ANqBc91RNmldZ02oLUaxUZ6K8A1m38npzV2eKsq4VkKJYjKeOsrYjWyrG58iTKKsrX1GK+LReT1GwNa5yJUQ8b5R1LmUmQ3we5R3Iymo2b+c0S8HWu78iy2KnGZV1r4wj8KKKY8KCPd3yRzNGeb6R5jGf8uynEzPs+IyP4B+zKc84XZ9mK9hTXpM55lKec8GUNFfBnvQS+jGVMm6G8b/70z6ZPZMy7lD13wATP32fplGeeR2NY5aCPfWznJMoz71WSppDGY/e+++WZ19EY4qCPftyODMU7DQ9ci7Kh+M9DAHKzg53BLlQsHevB/GMyM675YRq7b5bTgiyf+UDyO675YkXz5hGOSHI/gv2gSC7Vz4QZFLBPpzt09TIDpWnfEC3sWDvnoz3AGVHytM9B3SzYO9udmb6ILvqlqd6coCnYB9OdgRBdqR8HAhyQ8E+POwEguxGOR0IcmvBPszvAIJ83Ui7cWMEmVLtdss/HkH2olwyRpB9KBeNEWQXY5cDxu6Vi8Yo1jV1b7f3kxFkJ8ow5q19uz1jFGsHygeM3StfGaNYm1dOMJZpvB3GUIYxlPt1xxh03WrCHcZQ1lCqYWxeGcb+la+JMebiiAuMoYwhF5TRHUMZxlMrJ3TH7pUx5PKvjFI9pGXRHUMZ3TGU0R1DGd3xVMrojv0rw9i/Mrpj98oJxu6VUar9Kx8wdq8MY/fK6I79K8PYvzJKtXvlBGP3yjD2r3ygO9arvHeLMYxtK6NU+1c+YOxeGcbuldEd+1eGsX9llGr/yjB2r5xgrG9jXkj8QHfsXTnB2L3ygVJtU/lAjKEMYxdjbJJMQqk2rnzc/HcYm1DeGYhRqpX3y6UUHoixG+XjRl8MYzPK3wsuGRil2pLyD68KXBgbVa7f0LLulRFj/8ow9q+MJvWvjAZ1r4xSrXWDMZRRqqEM44mUUar9K6MJ/SujAd2fSaFU+1dG27lXRoz9K6PZ3A+/EGP/yjB2X7JB7D/MaCr3YUaM3YcZxO7DjAbynmaE2HmcAewaWgXv/wCJo9HSIlxdqwAAAABJRU5ErkJggg==\">"
				"<h1 style=\"margin:0;padding:11pt;background:#151510;font:bold 18pt/18pt sans-serif\">BeatUpServer Instance</h1>"
			"</a>"
			"<div style=\"margin:4pt\">");
	if(code) {} else {
		len += sprintf(&page[len],
				"<table id=\"main\" style=\"width:100%%;table-layout:fixed;color:#fff;\">"
					"<thead>"
						"<tr>"
							"<th style=\"width:6ch\">Code</th><th>Last Played Level</th><th id=\"ph\">Players</th><th style=\"width:12ch\">Version</th><th style=\"width:15ch\">Player/Level NPS</th><th style=\"width:13.5pt\"></th>"
						"</tr>"
					"</thead>"
					"<tbody>");
		for(uint32_t i = 0; i < lengthof(tempList); ++i) {
			char scode[8], plim[16] = "∞", level[256];
			if(tempList[i].playerCap < 0xffffffff)
				sprintf(plim, "%u", tempList[i].playerCap);
			ServerCodeToString(scode, tempList[i].code);
			size_t level_len = escape(level, sizeof(level), tempList[i].levelName, strlen(tempList[i].levelName));
			len += sprintf(&page[len], "<tr>");
			len += sprintf(&page[len], "<th><a href=\"%s\"><code>%s</code></a></th>", scode, scode);
			len += sprintf(&page[len], "<td><a href=\"%s\"><div class=\"ln\"%s><span>%.*s</span><br><div>|%.*s||%.*s|</div></div></a></td>", scode, (tempList[i].code == 40549761) ? " style=\"color:#ff0;font-weight:bold\"" : "", level_len, level, level_len, level, level_len, level);
			len += sprintf(&page[len], "<th><a href=\"%s\">%u / %s</a></th>", scode, tempList[i].playerCount, plim);
			len += sprintf(&page[len], "<th><a href=\"%s\">Vanilla 1.19.1</a></th>", scode);
			len += sprintf(&page[len], "<th><a href=\"%s\">%.2f / %.2f</a></th>", scode, tempList[i].playerNPS, tempList[i].levelNPS);
			len += sprintf(&page[len], "<th><a href=\"%s\"></a></th>", scode);
			len += sprintf(&page[len], "</tr>");
		}
		len += sprintf(&page[len],
					"</tbody>"
				"</table>");
	}
	len += sprintf(&page[len],
			"</div>"
		"</body>"
		"</html>");
	return status_bin(buf, "200 OK", "text/html", (const uint8_t*)page, len);
}

static uint32_t status_resp(const char *source, const char *path, char *buf, uint32_t buf_len) {
	if(buf_len > 16 && memcmp(buf, "GET /robots.txt ", 16) == 0)
		return status_text(buf, "200 OK", "text/plain", "User-agent: *\nDisallow: /\n");
	_Bool isGame = 1;
	for(uint32_t i = 0, count = 0; i < buf_len; ++i) {
		if(buf[i] == '\n')
			++count;
		if(count > 3) {
			isGame = 0;
			break;
		}
	}
	if(isGame) {
		char rq[4096];
		uint32_t rq_len = sprintf(rq, "GET %s", path);
		if(buf_len < rq_len || memcmp(buf, rq, rq_len))
			return 0;
		fprintf(stderr, "[%s,game] %.*s\n", source, (int32_t)((char*)memchr(&buf[4], ' ', buf_len) - buf), buf);
		char *resp = buf;
		buf += rq_len, buf_len -= rq_len;
		if(buf_len && *buf == '/')
			++buf, --buf_len;
		if(buf_len && *buf == ' ')
			return status_text(resp, "200 OK", "application/json", "{\"minimumAppVersion\":\"1.16.4\",\"status\":0,\"maintenanceStartTime\":0,\"maintenanceEndTime\":0,\"userMessage\":{\"localizedMessages\":[{\"language\":\"en\",\"message\":\"Test message from server\"}]}}");
		else if(buf_len > 17 && memcmp(buf, "mp_override.json ", 17) == 0)
			return status_text(resp, "200 OK", "application/json", "{\"quickPlayAvailablePacksOverride\":{\"predefinedPackIds\":[{\"order\":0,\"packId\":\"ALL_LEVEL_PACKS\"},{\"order\":1,\"packId\":\"BUILT_IN_LEVEL_PACKS\"}],\"localizedCustomPacks\":[{\"serializedName\":\"customlevels\",\"order\":2,\"localizedNames\":[{\"language\":0,\"packName\":\"Custom\"}],\"packIds\":[\"custom_levelpack_CustomLevels\"]}]}}");
		return status_text(resp, "404 Not Found", "text/plain", "");
	} else {
		fprintf(stderr, "[%s,web] %.*s\n", source, (int32_t)((char*)memchr(&buf[4], ' ', buf_len) - buf), buf);
		if(buf_len > 17 && memcmp(buf, "GET /favicon.ico ", 17) == 0) {
			static const uint8_t favicon[] = {0,0,1,0,2,0,32,32,0,0,1,0,24,0,168,12,0,0,38,0,0,0,32,32,2,0,1,0,1,0,48,1,0,0,206,12,0,0,40,0,0,0,32,0,0,0,64,0,0,0,1,0,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,158,48,255,255,255,255,158,48,255,158,48,255,158,48,255,158,48,255,252,31,255,255,240,3,255,255,192,0,255,254,3,240,31,252,15,252,7,240,63,255,131,224,255,255,227,193,255,255,241,199,255,255,241,207,255,255,249,143,255,255,249,143,255,255,249,143,255,255,249,143,255,255,249,142,7,255,249,136,199,255,241,137,143,255,241,143,24,63,241,140,96,7,241,145,131,128,241,158,31,240,49,152,127,254,17,144,255,255,145,139,255,255,227,143,255,255,227,199,255,255,135,193,255,254,15,224,63,248,31,248,7,224,127,255,0,1,255,255,224,7,255,255,252,31,255,40,0,0,0,32,0,0,0,64,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
			return status_bin(buf, "200 OK", "image/x-icon", favicon, sizeof(favicon));
		} else if(buf_len > 10 && memcmp(buf, "GET /", 5) == 0) {
			uint32_t len = 0;
			for(; len < 5; ++len)
				if(buf[5 + len] == ' ')
					break;
			return status_web(buf, StringToServerCode(&buf[5], len));
		}
		return 0;
	}
}