#include <string.h>

static uint32_t status_resp(const char *source, char *buf, uint32_t buf_len) {
	if(buf_len < 5 || memcmp(buf, "GET /", 5))
		return 0;
	fprintf(stderr, "[%s] %.*s\n", source, (int32_t)((char*)memchr(&buf[5], ' ', buf_len) - buf), buf);
	if(buf_len > 5 && buf[5] == ' ') {
		return sprintf(buf,
			"HTTP/1.1 200 OK\r\n"
			"Connection: close\r\n"
			"Content-Length: 182\r\n"
			"X-Frame-Options: DENY\r\n"
			"X-Content-Type-Options: nosniff\r\n"
			"Content-Type: application/json; charset=utf-8\r\n"
			"X-DNS-Prefetch-Control: off\r\n"
			"X-Robots-Tag: noindex\r\n"
			"\r\n"
			"{\"minimumAppVersion\":\"1.16.4\",\"status\":0,\"maintenanceStartTime\":0,\"maintenanceEndTime\":0,\"userMessage\":{\"localizedMessages\":[{\"language\":\"en\",\"message\":\"Test message from server\"}]}}");
	} else if(buf_len > 16 && memcmp(&buf[5], "robots.txt ", 11) == 0) {
		return sprintf(buf,
			"HTTP/1.1 200 OK\r\n"
			"Connection: close\r\n"
			"Content-Length: 26\r\n"
			"X-Frame-Options: DENY\r\n"
			"X-Content-Type-Options: nosniff\r\n"
			"Content-Type: text/plain; charset=utf-8\r\n"
			"X-DNS-Prefetch-Control: off\r\n"
			"X-Robots-Tag: noindex\r\n"
			"\r\n"
			"User-agent: *\nDisallow: /\n");
	} else {
		return sprintf(buf,
			"HTTP/1.1 404 Not Found\r\n"
			"Connection: close\r\n"
			"Content-Length: 39\r\n"
			"X-Frame-Options: DENY\r\n"
			"X-Content-Type-Options: nosniff\r\n"
			"Content-Type: text/html; charset=utf-8\r\n"
			"X-DNS-Prefetch-Control: off\r\n"
			"X-Robots-Tag: noindex\r\n"
			"\r\n"
			"<html><body>404 not found</body></html>");
	}
}