#include "config.h"
#include "net.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static const char *const config_path = "config.json";

int main(int argc, char const *argv[]) {
	if(!isatty(0)) {
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
	if(master_init(&cfg.master_cert, &cfg.master_key, cfg.master_port))
		return -1;
	usleep(10000); // Fixes out of order logs
	fprintf(stderr, "Press [enter] to exit\n");
	getchar();
	master_cleanup();
	if(cfg.status_tls)
		status_ssl_cleanup();
	else
		status_cleanup();
	config_free(&cfg);
	return 0;
}
