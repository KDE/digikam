/*****************************************************************************/
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_pthread.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_pthread__
#define __dng_pthread__

/*****************************************************************************/

#include "dng_flags.h"

/*****************************************************************************/

#if qDNGThreadSafe

/*****************************************************************************/

#if !qWinOS

/*****************************************************************************/

/* Try generic POSIX compile */

#include <errno.h>
#include <pthread.h>

#define dng_pthread_disassociate()
#define dng_pthread_terminate()

/*****************************************************************************/

#else

/*****************************************************************************/

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/

#define DNG_ETIMEDOUT       60              /* Operation timed out */

struct dng_timespec {
	long tv_sec;
	long tv_nsec;
};


typedef unsigned long dng_pthread_t;

typedef struct dng_pthread_mutex_impl *dng_pthread_mutex_t;
typedef struct dng_pthread_cond_impl *dng_pthread_cond_t;
typedef unsigned long dng_pthread_key_t;


#define DNG_PTHREAD_MUTEX_INITIALIZER ((struct dng_pthread_mutex_impl *)-1)
#define DNG_PTHREAD_COND_INITIALIZER ((struct dng_pthread_cond_impl *)-1)

struct _dng_pthread_once_t {
	int inited;
	long semaphore;
};

typedef struct _dng_pthread_once_t dng_pthread_once_t;
#define DNG_PTHREAD_ONCE_INIT { 0, -1 }

#define dng_pthread_equal(t1, t2) ((t1) == (t2))

typedef struct dng_pthread_attr_impl *dng_pthread_attr_t;

int dng_pthread_attr_init(dng_pthread_attr_t *attr);
int dng_pthread_attr_destroy(dng_pthread_attr_t *attr);

int dng_pthread_attr_setstacksize(dng_pthread_attr_t *attr, size_t stacksize);
int dng_pthread_attr_getstacksize(const dng_pthread_attr_t *attr, size_t *stacksize);

int dng_pthread_create(dng_pthread_t *thread, const dng_pthread_attr_t * /* attrs */, void * (*func)(void *), void *arg);
int dng_pthread_detach(dng_pthread_t thread);
int dng_pthread_join(dng_pthread_t thread, void **result);
dng_pthread_t dng_pthread_self();
void dng_pthread_exit(void *result);

#define DNG_PTHREAD_MUTEX_RECURSIVE 0
typedef unsigned long dng_pthread_mutexattr_t;

int dng_pthread_mutexattr_init(dng_pthread_mutexattr_t *mutexattr);
int dng_pthread_mutexattr_settype(dng_pthread_mutexattr_t *mutexattr, int /*the options*/);

int dng_pthread_mutex_init(dng_pthread_mutex_t *mutex, void * /* attrs */);
int dng_pthread_mutex_destroy(dng_pthread_mutex_t *mutex);
int dng_pthread_mutex_lock(dng_pthread_mutex_t *mutex);
int dng_pthread_mutex_unlock(dng_pthread_mutex_t *mutex);

int dng_pthread_cond_init(dng_pthread_cond_t *cond, void * /* attrs */);
int dng_pthread_cond_destroy(dng_pthread_cond_t *cond);
int dng_pthread_cond_wait(dng_pthread_cond_t *cond, dng_pthread_mutex_t *mutex);
int dng_pthread_cond_timedwait(dng_pthread_cond_t *cond, dng_pthread_mutex_t *mutex, struct dng_timespec *latest_time);
int dng_pthread_cond_signal(dng_pthread_cond_t *cond);
int dng_pthread_cond_broadcast(dng_pthread_cond_t *cond);

int dng_pthread_once(dng_pthread_once_t *once, void (*init_func)());

int dng_pthread_key_create(dng_pthread_key_t * key, void (*destructor) (void *));
int dng_pthread_key_delete(dng_pthread_key_t key);
int dng_pthread_setspecific(dng_pthread_key_t key, const void *value);
void *dng_pthread_getspecific(dng_pthread_key_t key);

typedef struct dng_pthread_rwlock_impl *dng_pthread_rwlock_t;
typedef void *pthread_rwlockattr_t;

int dng_pthread_rwlock_destroy(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_init(dng_pthread_rwlock_t * rwlock, const pthread_rwlockattr_t * attrs);
int dng_pthread_rwlock_rdlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_tryrdlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_trywrlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_unlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_wrlock(dng_pthread_rwlock_t * rwlock);

typedef struct dng_pthread_rwlock_impl *dng_pthread_rwlock_t;
typedef void *pthread_rwlockattr_t;

int dng_pthread_rwlock_destroy(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_init(dng_pthread_rwlock_t * rwlock, const pthread_rwlockattr_t * attrs);
int dng_pthread_rwlock_rdlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_tryrdlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_trywrlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_unlock(dng_pthread_rwlock_t * rwlock);
int dng_pthread_rwlock_wrlock(dng_pthread_rwlock_t * rwlock);

// dng_pthread may maintain per-thread global state. This routine frees that global state.
// there is no need to call this for threads created by dng_pthread and one can call
// dng_pthread routines of a thread after dng_pthread_disassociate as the global state will
// be recreated as necessary. However dng_pthread_disassociate will need to be called again
// and there is a slight performance cost. Do not call this routine while holding a mutex, etc.
void dng_pthread_disassociate();

void dng_pthread_terminate();

/*****************************************************************************/

// Map symbols back to plain pthread names. This whole mechanism is so the DNG pthreads library
// symbols do not collide with another pthread emulation library
// that may be in use in the same linked entity. However if that is the case, it would be far better
// to have the DNG code use the same pthread library as the rest of the code.

#define pthread_t dng_pthread_t
#define pthread_mutex_t dng_pthread_mutex_t
#define pthread_cond_t dng_pthread_cond_t
#define pthread_once_t dng_pthread_once_t
#define pthread_key_t dng_pthread_key_t

#undef PTHREAD_MUTEX_INITIALIZER
#define PTHREAD_MUTEX_INITIALIZER DNG_PTHREAD_MUTEX_INITIALIZER
#undef PTHREAD_COND_INITIALIZER
#define PTHREAD_COND_INITIALIZER DNG_PTHREAD_COND_INITIALIZER

#undef PTHREAD_ONCE_INIT
#define PTHREAD_ONCE_INIT DNG_PTHREAD_ONCE_INIT

#define timespec dng_timespec

/* If it is defined on Windows, it probably has the wrong value... */
#if defined(WIN32) || !defined(ETIMEDOUT)
#define ETIMEDOUT DNG_ETIMEDOUT
#endif

#define pthread_equal dng_pthread_equal

#define pthread_attr_t dng_pthread_attr_t

#define pthread_attr_init dng_pthread_attr_init
#define pthread_attr_destroy dng_pthread_attr_destroy

#define pthread_attr_setstacksize dng_pthread_attr_setstacksize
#define pthread_attr_getstacksize dng_pthread_attr_getstacksize

#define pthread_create dng_pthread_create
#define pthread_detach dng_pthread_detach
#define pthread_join dng_pthread_join
#define pthread_self dng_pthread_self
#define pthread_exit dng_pthread_exit

#define pthread_mutex_init dng_pthread_mutex_init
#define pthread_mutex_destroy dng_pthread_mutex_destroy
#define pthread_mutex_lock dng_pthread_mutex_lock
#define pthread_mutex_unlock dng_pthread_mutex_unlock

#define pthread_cond_init dng_pthread_cond_init
#define pthread_cond_destroy dng_pthread_cond_destroy
#define pthread_cond_wait dng_pthread_cond_wait
#define pthread_cond_timedwait dng_pthread_cond_timedwait
#define pthread_cond_signal dng_pthread_cond_signal
#define pthread_cond_broadcast dng_pthread_cond_broadcast

#define pthread_once dng_pthread_once

#define pthread_key_create dng_pthread_key_create
#define pthread_key_delete dng_pthread_key_delete
#define pthread_setspecific dng_pthread_setspecific
#define pthread_getspecific dng_pthread_getspecific

#define pthread_rwlock_t dng_pthread_rwlock_t

#define pthread_rwlock_destroy dng_pthread_rwlock_destroy
#define pthread_rwlock_init dng_pthread_rwlock_init
#define pthread_rwlock_rdlock dng_pthread_rwlock_rdlock
#define pthread_rwlock_tryrdlock dng_pthread_rwlock_tryrdlock
#define pthread_rwlock_trywrlock dng_pthread_rwlock_trywrlock
#define pthread_rwlock_unlock dng_pthread_rwlock_unlock
#define pthread_rwlock_wrlock dng_pthread_rwlock_wrlock

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

/*****************************************************************************/

#endif

/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

int dng_pthread_now (struct timespec *now);

#ifdef __cplusplus
}
#endif

/*****************************************************************************/

#endif // qDNGThreadSafe

/*****************************************************************************/

#endif

/*****************************************************************************/
