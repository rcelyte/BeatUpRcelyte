#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef WINDOWS
#define thread_return_t uint32_t
#else
#define thread_return_t uintptr_t
#endif
typedef thread_return_t (*ThreadRoutine)(void*);

#ifdef WINDOWS
#include <processthreadsapi.h>
#include <synchapi.h>
#define thread_t HANDLE

static inline bool _thread_create(thread_t *thread, ThreadRoutine func, void *userptr) {
	*thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, userptr, 0, NULL);
	return !thread;
}
static inline void thread_join(thread_t thread) {
	WaitForSingleObject(thread, 0xffffffff);
}
#else
#include <pthread.h>
#define thread_t pthread_t

static inline bool _thread_create(thread_t *thread, ThreadRoutine func, void *userptr) {
	return pthread_create(thread, NULL, (void*(*)(void*))func, userptr) != 0;
}
static inline void thread_join(thread_t thread) {
	pthread_join(thread, NULL);
}
#endif

#define thread_create(thread, func, userptr) _thread_create(thread, (ThreadRoutine)(func), userptr)
