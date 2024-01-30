#pragma once
#define lengthof(x) (sizeof(x) / sizeof((x)[0]))
#define endof(x) (&(x)[lengthof(x)])
