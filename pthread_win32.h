#ifndef __PTHREAD_WIN32_H__
#define __PTHREAD_WIN32_H__

/* pthread emulation for windows */

#ifdef _WIN32

#ifndef STATIC /* by default do not export anything */
#define STATIC static
#endif

#include <windows.h>
#include <process.h>

typedef struct {
	HANDLE handle;
	unsigned thread_id;
	int detached;
} pthread_t;

typedef CRITICAL_SECTION   pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;

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

#endif /* _WIN32 */

#endif /* __PTHREAD_WIN32_H__ */
