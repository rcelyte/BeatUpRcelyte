#include "config.h"
#include "instance/instance.h"
#include "master/master.h"
#include "status/status.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static const char *config_path = "./beatupserver.json";
static bool headless = false;

static struct Config cfg;
int main(int argc, const char *argv[]) {
	// fprintf(stderr, "MAX CODE: %u\n", StringToServerCode("99999", 5));
	for(const char **arg = &argv[1]; arg < &argv[argc]; ++arg) {
		if(strcmp(*arg, "--daemon") == 0) {
			headless = 1;
		} else if(strcmp(*arg, "-4") == 0 || strcmp(*arg, "--ipv4") == 0) {
			net_useIPv4 = true;
		} else if(strcmp(*arg, "-c") == 0 || strcmp(*arg, "--config") == 0) {
			if(++arg < &argv[argc])
				config_path = *arg;
		}
	}
	if(headless == 0 && isatty(0) == 0) {
		fprintf(stderr, "Not running in an interactive terminal\n");
		return -1;
	}
	#ifdef WINDOWS
	if(headless) {
		fprintf(stderr, "Headless mode is only supported on POSIX systems\n");
		return -1;
	}
	#endif
	memset(&cfg, 0, sizeof(cfg));
	if(config_load(&cfg, config_path)) // TODO: live config reloading
		goto fail0;
	wire_set_key(cfg.wireKey, cfg.wireKey_len);
	if(cfg.statusPort) {
		status_internal_init();
		if(mbedtls_pk_get_type(&cfg.statusKey) != MBEDTLS_PK_NONE) {
			if(status_ssl_init(cfg.certs, cfg.keys, cfg.statusAddress, cfg.statusPath, cfg.statusPort))
				goto fail1;
		} else {
			if(status_init(cfg.statusPath, cfg.statusPort))
				goto fail1;
		}
	}
	struct NetContext *localMaster = NULL;
	if(cfg.masterPort) {
		localMaster = master_init(cfg.certs, cfg.keys, cfg.masterPort);
		if(!localMaster)
			goto fail3;
	}
	if(instance_init(cfg.instanceAddress[0], cfg.instanceAddress[1], cfg.instanceParent, localMaster, cfg.instanceMapPool, cfg.instanceCount))
		goto fail4;
	if(headless) {
		#ifndef WINDOWS
		sigset_t sigset;
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGINT);
		sigaddset(&sigset, SIGHUP);
		sigprocmask(SIG_BLOCK, &sigset, NULL);
		int sig;
		sigwait(&sigset, &sig);
		#endif
	} else {
		usleep(10000); // Race the logging on other threads
		fprintf(stderr, "Press [enter] to exit\n");
		getchar();
	}
	fail4:
	instance_cleanup();
	fail3:
	if(cfg.masterPort) // TODO: unconditional cleanup
		master_cleanup();
	fail1:
	if(cfg.statusPort) {
		if(mbedtls_pk_get_type(&cfg.statusKey) != MBEDTLS_PK_NONE)
			status_ssl_cleanup();
		else
			status_cleanup();
	}
	fail0:
	config_free(&cfg);
	return 0;
}
