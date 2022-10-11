#pragma once
#include "../net.h"

bool instance_init(const char *domainIPv4, const char *domain, const char *remoteMaster, struct NetContext *localMaster, const char *mapPoolFile, uint32_t count);
void instance_cleanup();
