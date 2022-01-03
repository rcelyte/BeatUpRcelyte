#include "config.h"
#include "net.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

static const char *config_path = "./config.json";
static _Bool headless = 0;

int main(int argc, char const *argv[]) {
	for(const char **arg = &argv[1]; arg < &argv[argc]; ++arg) {
		if(strcmp(*arg, "--daemon") == 0) {
			headless = 1;
		} else if(strcmp(*arg, "-c") == 0 || strcmp(*arg, "--config") == 0) {
			if(++arg < &argv[argc])
				config_path = *arg;
		}
	}
	if(headless == 0 && isatty(0) == 0) {
		fprintf(stderr, "Not running in an interactive terminal\n");
		return -1;
	}
	struct Config cfg;
	if(config_load(&cfg, config_path))
		return -1;
	if(cfg.status_tls) {
		if(status_ssl_init(&cfg.status_cert, &cfg.status_key, cfg.status_path, cfg.status_port))
			return -1;
	} else {
		if(status_init(cfg.status_path, cfg.status_port))
			return -1;
	}
	if(instance_init(cfg.host_domain))
		return -1;
	if(master_init(&cfg.master_cert, &cfg.master_key, cfg.master_port))
		return -1;
	if(headless) {
		sigset_t sigset;
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGINT);
		sigaddset(&sigset, SIGHUP);
		sigprocmask(SIG_BLOCK, &sigset, NULL);
		int sig;
		sigwait(&sigset, &sig);
	} else {
		usleep(10000); // Fixes out of order logs
		fprintf(stderr, "Press [enter] to exit\n");
		getchar();
	}
	master_cleanup();
	instance_cleanup();
	if(cfg.status_tls)
		status_ssl_cleanup();
	else
		status_cleanup();
	config_free(&cfg);
	return 0;
}
