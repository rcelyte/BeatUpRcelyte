#pragma once
#include "log.h"
#include <stdint.h>
#define lengthof(x) (sizeof(x)/sizeof(*(x)))
#define endof(x) (&(x)[lengthof(x)])
