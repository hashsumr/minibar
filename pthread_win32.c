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
#if _WIN32_WINNT >= 0x0600
	InitializeSRWLock(mutex);
#else
	InitializeCriticalSection(mutex);
#endif
	return 0;
}

STATIC int
pthread_mutex_destroy(pthread_mutex_t *mutex) {
#if _WIN32_WINNT >= 0x0600
	/* do nothing */
#else
	DeleteCriticalSection(mutex);
#endif
	return 0;
}

STATIC int
pthread_mutex_lock(pthread_mutex_t *mutex) {
#if _WIN32_WINNT >= 0x0600
	AcquireSRWLockExclusive(mutex);
#else
	EnterCriticalSection(mutex);
#endif
	return 0;
}

STATIC int
pthread_mutex_unlock(pthread_mutex_t *mutex) {
#if _WIN32_WINNT >= 0x0600
	ReleaseSRWLockExclusive(mutex);
#else
	LeaveCriticalSection(mutex);
#endif
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
#if _WIN32_WINNT >= 0x0600
	SleepConditionVariableSRW(cond, mutex, INFINITE, 0);
#else
	SleepConditionVariableCS(cond, mutex, INFINITE);
#endif
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

/* barrier */
STATIC int
pthread_barrier_init(pthread_barrier_t *barrier, void *attr, unsigned count) {
	(void)attr;

	if (count == 0)
		return -1;

	if (!InitializeSynchronizationBarrier(
		barrier, count, -1)) {
		return -1;
	}

	return 0;
}

STATIC int
pthread_barrier_wait(pthread_barrier_t *barrier) {
	BOOL last = EnterSynchronizationBarrier(
		barrier,
		SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);

	/*
	 * POSIX requires exactly ONE thread to return
	 * PTHREAD_BARRIER_SERIAL_THREAD
	 */
	if (last)
		return PTHREAD_BARRIER_SERIAL_THREAD;

	return 0;
}

STATIC int
pthread_barrier_destroy(pthread_barrier_t *barrier) {
	DeleteSynchronizationBarrier(barrier);
	return 0;
}

#endif	/* _WIN32 */

#endif	/* __PTHREAD_WIN32_C__ */
