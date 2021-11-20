#ifdef reflect
#define ENUM(t, n, ...) typedef t n; enum __VA_ARGS__; static const char *const reflect_##n = #__VA_ARGS__;
#else
#define ENUM(t, n, ...) typedef t n; enum __VA_ARGS__;
#endif
