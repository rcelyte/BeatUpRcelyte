#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN32__)
CFLAGS += -DWINDOWS
LDFLAGS += -lws2_32 -lwinmm -lgdi32
#else
LDFLAGS += -pthread
#endif
