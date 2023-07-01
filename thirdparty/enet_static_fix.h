#ifndef _WIN32
#include <netdb.h>
#define getaddrinfo getaddrinfo_stub_

// stop enet from linking `getaddrinfo()` to make glibc happy
static inline int getaddrinfo(const char*, const char*, const void*, void*) {
	return -1;
}
#endif
