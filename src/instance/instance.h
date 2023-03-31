#pragma once
#include "../net.h"
#include "../wire.h"

bool instance_init(const char *domainIPv4, const char *domain, const char *remoteMaster, struct WireContext *localMaster, const char *mapPoolFile, uint32_t count);
void instance_cleanup(void);
