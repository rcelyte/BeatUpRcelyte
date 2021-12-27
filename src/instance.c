#define INSTANCE_BITS 14
#define START_PORT 14

#include "enum_reflection.h"
#include "instance.h"
#include "net.h"
#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
#include "debug.h"
#include <mbedtls/error.h>
#include <stdio.h>
#include <string.h>

struct Context {
	struct NetContext net;
};

#ifdef WINDOWS
static DWORD WINAPI
#else
static void*
#endif
instance_handler(struct Context *ctx) {
	fprintf(stderr, "Instance started\n");
	uint8_t buf[262144];
	memset(buf, 0, sizeof(buf));
	uint32_t len;
	struct MasterServerSession *session;
	PacketProperty property;
	uint8_t *pkt;
	while((len = net_recv(&ctx->net, buf, sizeof(buf), &session, &property, &pkt))) { // TODO: close instances with zero sessions using `onDisconnect`
		const uint8_t *data = pkt, *end = &pkt[len];
		fprintf(stderr, "property=%s\n", reflect(PacketProperty, property));
		struct MessageHeader message = pkt_readMessageHeader(&data);
		struct SerializeHeader serial = pkt_readSerializeHeader(&data);
		debug_logType(message, serial);
		if(data != end)
			fprintf(stderr, "BAD PACKET LENGTH (expected %u, got %zu)\n", len, data - pkt);
	}
	return 0;
}

#ifdef WINDOWS
static HANDLE instance_threads[1 << INSTANCE_BITS];
#else
static pthread_t instance_threads[1 << INSTANCE_BITS];
#endif
static struct Context ctx[1 << INSTANCE_BITS];
static uint32_t instance_count = 1; // ServerCode 0 ("") is not valid
static const char *instance_domain = NULL;
void instance_init(const char *domain) {
	instance_domain = domain;
	memset(instance_threads, 0, sizeof(instance_threads));
}

void instance_cleanup() {
	for(uint32_t i = 1; i < instance_count; ++i)
		instance_close((ServerCode){i});
}

_Bool instance_open(ServerCode *out, mbedtls_ctr_drbg_context *ctr_drbg) {
	if(instance_count >= (1 << INSTANCE_BITS)) {
		fprintf(stderr, "Instance limit reached\n");
		return 1;
	}
	if(net_init(&ctx[instance_count].net, 0, NULL)) {
		fprintf(stderr, "net_init() failed\n");
		return 1;
	}
	#ifdef WINDOWS
	instance_threads[instance_count] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)instance_handler, &ctx[instance_count], 0, NULL);
	#else
	if(pthread_create(&instance_threads[instance_count], NULL, (void*)&instance_handler, &ctx[instance_count]))
		instance_threads[instance_count] = 0;
	#endif
	if(!instance_threads[instance_count]) {
		fprintf(stderr, "Instance thread creation failed\n");
		return 1;
	}
	*out = instance_count++;
	return 0;
}

void instance_close(ServerCode code) {
	if(instance_threads[code]) {
		net_stop(&ctx[code].net);
		fprintf(stderr, "Stopping instance %u\n", code);
		#ifdef WINDOWS
		WaitForSingleObject(instance_threads[code], INFINITE);
		#else
		pthread_join(instance_threads[code], NULL);
		#endif
		instance_threads[code] = 0;
		net_cleanup(&ctx[code].net);
	}
}

_Bool instance_get_isopen(ServerCode code) {
	if(code >= instance_count)
		return 0;
	return instance_threads[code];
}

struct NetContext *instance_get_net(ServerCode code) {
	return &ctx[code].net;
}

struct IPEndPoint instance_get_address(ServerCode code) {
	fprintf(stderr, "TODO: ADD 'HostName' CONFIG VALUE\n");
	struct IPEndPoint out;
	out.address.length = sprintf(out.address.data, "%s", instance_domain);
	struct SS addr = {sizeof(struct sockaddr_storage)};
	getsockname(net_get_sockfd(&ctx[code].net), &addr.sa, &addr.len);
	switch(addr.ss.ss_family) {
		case AF_INET: {
			out.port = htons(addr.in.sin_port);
			break;
		}
		case AF_INET6: {
			out.port = htons(addr.in6.sin6_port);
			break;
		}
		default:
		out.port = 0;
	}
	return out;
}
