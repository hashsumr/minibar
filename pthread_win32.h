#ifndef __PTHREAD_WIN32_H__
#define __PTHREAD_WIN32_H__

/* pthread emulation for windows */

#ifdef _WIN32

#ifndef STATIC /* by default do not export anything */
#define STATIC static
#endif

#include <windows.h>
#include <process.h>

#define	PTHREAD_BARRIER_SERIAL_THREAD	1

typedef struct {
	HANDLE handle;
	unsigned thread_id;
	int detached;
} pthread_t;

#if _WIN32_WINNT >= 0x0600
#define PTHREAD_MUTEX_INITIALIZER SRWLOCK_INIT
typedef SRWLOCK pthread_mutex_t;
#else
typedef CRITICAL_SECTION   pthread_mutex_t;
#endif
typedef SRWLOCK pthread_mutex_t;
#define PTHREAD_COND_INITIALIZER {0}
typedef CONDITION_VARIABLE pthread_cond_t;
typedef SYNCHRONIZATION_BARRIER pthread_barrier_t;


/* thread */
STATIC int pthread_create(pthread_t *thread, void *attr, void *(*start_routine)(void *), void *arg);
STATIC int pthread_detach(pthread_t *thread);
STATIC int pthread_join(pthread_t thread, void **retval);

/* mutex */
STATIC int pthread_mutex_init(pthread_mutex_t *mutex, void *attr);
STATIC int pthread_mutex_destroy(pthread_mutex_t *mutex);
STATIC int pthread_mutex_lock(pthread_mutex_t *mutex);
STATIC int pthread_mutex_unlock(pthread_mutex_t *mutex);

/* condition variable */
STATIC int pthread_cond_init(pthread_cond_t *cond, void *attr);
STATIC int pthread_cond_destroy(pthread_cond_t *cond);
STATIC int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
STATIC int pthread_cond_broadcast(pthread_cond_t *cond);
STATIC int pthread_cond_signal(pthread_cond_t *cond);

/* barrier */
STATIC int pthread_barrier_init(pthread_barrier_t *barrier, void *attr, unsigned count);
STATIC int pthread_barrier_wait(pthread_barrier_t *barrier);
STATIC int pthread_barrier_destroy(pthread_barrier_t *barrier);

#endif /* _WIN32 */

#endif /* __PTHREAD_WIN32_H__ */
