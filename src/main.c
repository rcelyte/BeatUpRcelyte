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
	// fprintf(stderr, "MAX CODE: %u\n", StringToServerCode("ZZZZZ", 5));
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
	wire_init(cfg.wireKey, cfg.wireKey_len);
	struct WireContext *localMaster = NULL;
	if(cfg.masterPort) {
		localMaster = master_init(&cfg.masterCert, &cfg.masterKey, cfg.masterPort);
		if(localMaster == NULL)
			goto fail1;
	}
	if(cfg.statusPort) {
		status_internal_init();
		if(status_ssl_init(cfg.statusPath, cfg.statusPort, cfg.certs, cfg.keys, cfg.statusAddress, "", localMaster)) // TODO: remote master config
			goto fail2;
	}
	if(instance_init(cfg.instanceAddress[0], cfg.instanceAddress[1], &cfg.masterCert, &cfg.masterKey, cfg.instanceParent, localMaster, cfg.instanceMapPool, cfg.instanceCount))
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
	fail4: instance_cleanup();
	fail2: status_ssl_cleanup();
	fail1:
	if(cfg.masterPort) // TODO: unconditional cleanup
		master_cleanup();
	wire_cleanup();
	fail0: config_free(&cfg);
	return 0;
}
