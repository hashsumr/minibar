#ifndef __PTHREAD_WIN32_C__
#define __PTHREAD_WIN32_C__

/* pthread emulation for windows */

#ifdef _WIN32

#include "pthread_win32.h"
#include <stdlib.h>

typedef struct {
	void *(*start_routine)(void *);
	void *arg;
} thread_start_t;

static unsigned __stdcall
thread_entry(void *arg) {
	thread_start_t *ts = (thread_start_t *)arg;
	ts->start_routine(ts->arg);
	free(ts);
	return 0;
}

/* thread */

STATIC int
pthread_create(pthread_t *thread, void *attr,
	void *(*start_routine)(void *), void *arg) {
	(void) attr;
	thread_start_t *ts = malloc(sizeof(*ts));
	if (!ts) return -1;

	ts->start_routine = start_routine;
	ts->arg = arg;

	thread->detached = 0;
	thread->handle = (HANDLE) _beginthreadex(
		NULL, 0, thread_entry, ts, 0, &thread->thread_id);

	if (!thread->handle) {
		free(ts);
		return -1;
	}
	return 0;
}

STATIC int
pthread_detach(pthread_t *thread) {
	thread->detached = 1;
	CloseHandle(thread->handle);
	thread->handle = NULL;
	return 0;
}

STATIC int
pthread_join(pthread_t thread, void **retval) {
	(void) retval;
	WaitForSingleObject(thread.handle, INFINITE);
	CloseHandle(thread.handle);
	return 0;
}

/* mutex */

STATIC int
pthread_mutex_init(pthread_mutex_t *mutex, void *attr) {
	(void)attr;
	InitializeCriticalSection(mutex);
	return 0;
}

STATIC int
pthread_mutex_destroy(pthread_mutex_t *mutex) {
	DeleteCriticalSection(mutex);
	return 0;
}

STATIC int
pthread_mutex_lock(pthread_mutex_t *mutex) {
	EnterCriticalSection(mutex);
	return 0;
}

STATIC int
pthread_mutex_unlock(pthread_mutex_t *mutex) {
	LeaveCriticalSection(mutex);
	return 0;
}

/* condition variable */

STATIC int
pthread_cond_init(pthread_cond_t *cond, void *attr) {
	(void)attr;
	InitializeConditionVariable(cond);
	return 0;
}

STATIC int
pthread_cond_destroy(pthread_cond_t *cond) {
	(void)cond;
	return 0; /* no-op on Windows */
}

STATIC int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	SleepConditionVariableCS(cond, mutex, INFINITE);
	return 0;
}

STATIC int
pthread_cond_broadcast(pthread_cond_t *cond) {
	WakeAllConditionVariable(cond);
	return 0;
}

STATIC int
pthread_cond_signal(pthread_cond_t *cond) {
	WakeConditionVariable(cond);
	return 0;
}

#endif	/* _WIN32 */

#endif	/* __PTHREAD_WIN32_C__ */
