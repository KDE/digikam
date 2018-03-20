/*****************************************************************************/
// Copyright 2002-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_pthread.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

#include "dng_pthread.h"

/*****************************************************************************/

#if qDNGThreadSafe

/*****************************************************************************/

#include "dng_assertions.h"

/*****************************************************************************/

#if qWinOS

#pragma warning(disable : 4786)

// Nothing in this file requires Unicode,
// However, CreateSemaphore has a path parameter
// (which is NULL always in this code) and thus
// does not work on Win98 if UNICODE is defined.
// So we force it off here.

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <process.h>
#include <errno.h>
#include <memory>
#include <new>
#include <map>

#else

#include <sys/time.h>

#endif

/*****************************************************************************/

#if qWinOS

/*****************************************************************************/

namespace {
	struct waiter {
		struct waiter *prev;
		struct waiter *next;
		HANDLE semaphore;
		bool chosen_by_signal;
	};
}

/*****************************************************************************/

struct dng_pthread_mutex_impl
{
	CRITICAL_SECTION lock;

	dng_pthread_mutex_impl()  { ::InitializeCriticalSection(&lock); }
	~dng_pthread_mutex_impl() { ::DeleteCriticalSection(&lock); }
	void Lock()				   { ::EnterCriticalSection(&lock); }
	void Unlock()			   { ::LeaveCriticalSection(&lock); }
private:
	dng_pthread_mutex_impl &operator=(const dng_pthread_mutex_impl &) { }
	dng_pthread_mutex_impl(const dng_pthread_mutex_impl &) { }
};

/*****************************************************************************/

struct dng_pthread_cond_impl
{
	dng_pthread_mutex_impl lock;		// Mutual exclusion on next two variables
	waiter *head_waiter;			// List of threads waiting on this condition
	waiter *tail_waiter;			// Used to get FIFO, rather than LIFO, behavior for pthread_cond_signal
	unsigned int broadcast_generation;	// Used as sort of a separator on broadcasts
										// saves having to walk the waiters list setting
										// each one's "chosen_by_signal" flag while the condition is locked

	dng_pthread_cond_impl() : head_waiter(NULL), tail_waiter(NULL), broadcast_generation(0) { }
	~dng_pthread_cond_impl() { } ;

// Non copyable
private:
	dng_pthread_cond_impl &operator=(const dng_pthread_cond_impl &) { }
	dng_pthread_cond_impl(const dng_pthread_cond_impl &) { }

};

/*****************************************************************************/

namespace
{

	struct ScopedLock
	{
		dng_pthread_mutex_impl *mutex;

		ScopedLock(dng_pthread_mutex_impl *arg) : mutex(arg)
		{
			mutex->Lock();
		}
		ScopedLock(dng_pthread_mutex_impl &arg) : mutex(&arg)
		{
			mutex->Lock();
		}
		~ScopedLock()
		{
			mutex->Unlock();
		}
	private:
		ScopedLock &operator=(const ScopedLock &) { }
		ScopedLock(const ScopedLock &) { }
	};

	dng_pthread_mutex_impl validationLock;

	void ValidateMutex(dng_pthread_mutex_t *mutex)
	{
		if (*mutex != DNG_PTHREAD_MUTEX_INITIALIZER)
			return;

		ScopedLock lock(validationLock);

		if (*mutex == DNG_PTHREAD_MUTEX_INITIALIZER)
			dng_pthread_mutex_init(mutex, NULL);
	}

	void ValidateCond(dng_pthread_cond_t *cond)
	{
		if (*cond != DNG_PTHREAD_COND_INITIALIZER)
			return;

		ScopedLock lock(validationLock);

		if (*cond == DNG_PTHREAD_COND_INITIALIZER)
			dng_pthread_cond_init(cond, NULL);
	}

	DWORD thread_wait_sema_TLS_index;
	bool thread_wait_sema_inited = false;
	dng_pthread_once_t once_thread_TLS = DNG_PTHREAD_ONCE_INIT;

	void init_thread_TLS()
	{
		thread_wait_sema_TLS_index = ::TlsAlloc();
		thread_wait_sema_inited = true;
	}

	void finalize_thread_TLS()
	{
		if (thread_wait_sema_inited)
			{
			::TlsFree(thread_wait_sema_TLS_index);
			thread_wait_sema_inited = false;
			}
	}

	dng_pthread_mutex_impl primaryHandleMapLock;

	typedef std::map<DWORD, std::pair<HANDLE, void **> > ThreadMapType;

	// A map to make sure handles are freed and to allow returning a pointer sized result
	// even on 64-bit Windows.
	ThreadMapType primaryHandleMap;

	HANDLE GetThreadSemaphore()
	{
		dng_pthread_once(&once_thread_TLS, init_thread_TLS);

		HANDLE semaphore = ::TlsGetValue(thread_wait_sema_TLS_index);
		if (semaphore == NULL)
		{
			semaphore = ::CreateSemaphore(NULL, 0, 1, NULL);
			::TlsSetValue(thread_wait_sema_TLS_index, semaphore);
		}

		return semaphore;
	}

	void FreeThreadSemaphore()
	{
		if (thread_wait_sema_inited)
		{
			HANDLE semaphore = (HANDLE)::TlsGetValue(thread_wait_sema_TLS_index);

			if (semaphore != NULL)
			{
				::TlsSetValue(thread_wait_sema_TLS_index, NULL);
				::CloseHandle(semaphore);
			}
		}
	}

	struct trampoline_args
	{
		void *(*func)(void *);
		void *arg;
	};

	// This trampoline takes care of the return type being different
	// between pthreads thread funcs and Windows C lib thread funcs
	unsigned __stdcall trampoline(void *arg_arg)
	{
		trampoline_args *args_ptr = (trampoline_args *)arg_arg;
		trampoline_args args = *args_ptr;

		delete args_ptr;

		GetThreadSemaphore();

		void *result = args.func(args.arg);

		{
			ScopedLock lockMap(primaryHandleMapLock);

			ThreadMapType::iterator iter = primaryHandleMap.find(pthread_self());
			if (iter != primaryHandleMap.end())
				*iter->second.second = result;
		}

		FreeThreadSemaphore();

		return S_OK;
	}

}

/*****************************************************************************/

extern "C" {

/*****************************************************************************/

struct dng_pthread_attr_impl
	{
	size_t stacksize;
	};

/*****************************************************************************/

int dng_pthread_attr_init(pthread_attr_t *attr)
	{
	dng_pthread_attr_impl *newAttrs;

	newAttrs = new (std::nothrow) dng_pthread_attr_impl;
	if (newAttrs == NULL)
		return -1; // ENOMEM;

	newAttrs->stacksize = 0;

	*attr = newAttrs;

	return 0;
	}

/*****************************************************************************/

int dng_pthread_attr_destroy(pthread_attr_t *attr)
	{
	if (*attr == NULL)
		return -1; // EINVAL

	delete *attr;

	*attr = NULL;

	return 0;
	}

/*****************************************************************************/

int dng_pthread_attr_setstacksize(dng_pthread_attr_t *attr, size_t stacksize)
	{
	if (attr == NULL || (*attr) == NULL)
		return -1; // EINVAL

	(*attr)->stacksize = stacksize;

	return 0;
	}

/*****************************************************************************/

int dng_pthread_attr_getstacksize(const dng_pthread_attr_t *attr, size_t *stacksize)
	{
	if (attr == NULL || (*attr) == NULL || stacksize == NULL)
		return -1; // EINVAL

	*stacksize = (*attr)->stacksize;

	return 0;
	}

/*****************************************************************************/

int dng_pthread_create(dng_pthread_t *thread, const pthread_attr_t *attrs, void * (*func)(void *), void *arg)
{
	try
	{
		uintptr_t result;
		unsigned threadID;
		std::auto_ptr<trampoline_args> args(new (std::nothrow) trampoline_args);
		std::auto_ptr<void *> resultHolder(new (std::nothrow) (void *));

		if (args.get() == NULL || resultHolder.get () == NULL)
			return -1; // ENOMEM

		args->func = func;
		args->arg = arg;

		size_t stacksize = 0;

		if (attrs != NULL)
			dng_pthread_attr_getstacksize (attrs, &stacksize);

		result = _beginthreadex(NULL, (unsigned)stacksize, trampoline, args.get(), CREATE_SUSPENDED, &threadID);
		if (result == NULL)
			return -1; // ENOMEM
		args.release();

		{
			ScopedLock lockMap(primaryHandleMapLock);

			std::pair<DWORD, std::pair<HANDLE, void **> > newMapEntry(threadID,
																	 std::pair<HANDLE, void **>((HANDLE)result, resultHolder.get ()));
			std::pair<ThreadMapType::iterator, bool> insertion = primaryHandleMap.insert(newMapEntry);

			// If there is a handle open on the thread, its ID should not be reused so assert that an insertion was made.
			DNG_ASSERT(insertion.second, "pthread emulation logic error");
		}

		::ResumeThread((HANDLE)result);

		resultHolder.release ();

		*thread = (dng_pthread_t)threadID;
		return 0;
	}
	catch (const std::bad_alloc &)
	{
		return -1;
	}
}

/*****************************************************************************/

int dng_pthread_detach(dng_pthread_t thread)
{
	HANDLE primaryHandle;
	void **resultHolder = NULL;

	{
		ScopedLock lockMap(primaryHandleMapLock);

		ThreadMapType::iterator iter = primaryHandleMap.find(thread);
		if (iter == primaryHandleMap.end())
			return -1;

		primaryHandle = iter->second.first;

		// A join is waiting on the thread.
		if (primaryHandle == NULL)
			return -1;

		resultHolder = iter->second.second;

		primaryHandleMap.erase(iter);
	}

	delete resultHolder;

	if (!::CloseHandle(primaryHandle))
		return -1;

	return 0;
}

/*****************************************************************************/

int dng_pthread_join(dng_pthread_t thread, void **result)
{
	bool found = false;
	HANDLE primaryHandle = NULL;
	void **resultHolder = NULL;

	ThreadMapType::iterator iter;

	{
		ScopedLock lockMap(primaryHandleMapLock);

		iter = primaryHandleMap.find(thread);
		found = iter != primaryHandleMap.end();
		if (found)
		{
			primaryHandle = iter->second.first;
			resultHolder = iter->second.second;

			// Set HANDLE to NULL to force any later join or detach to fail.
			iter->second.first = NULL;
		}
	}

	// This case can happens when joining a thread not created with pthread_create,
	// which is a bad idea, but it gets mapped to doing the join, but always returns NULL.
	if (!found)
		primaryHandle = ::OpenThread(SYNCHRONIZE|THREAD_QUERY_INFORMATION, FALSE, thread);

	if (primaryHandle == NULL)
		return -1;

	DWORD err;
	if (::WaitForSingleObject(primaryHandle, INFINITE) != WAIT_OBJECT_0)
	{
		err = ::GetLastError();
		return -1;
	}

	{
		ScopedLock lockMap(primaryHandleMapLock);

		if (iter != primaryHandleMap.end())
			primaryHandleMap.erase(iter);
	}

	::CloseHandle(primaryHandle);
	if (result != NULL && resultHolder != NULL)
		*result = *resultHolder;

	delete resultHolder;

	return 0;
}

/*****************************************************************************/

dng_pthread_t dng_pthread_self()
{
	return (dng_pthread_t)::GetCurrentThreadId();
}

/*****************************************************************************/

void dng_pthread_exit(void *result)
{
	{
		ScopedLock lockMap(primaryHandleMapLock);

		ThreadMapType::iterator iter = primaryHandleMap.find(pthread_self());
		if (iter != primaryHandleMap.end())
			*iter->second.second = result;
	}

	FreeThreadSemaphore();

	_endthreadex(S_OK);
}

/*****************************************************************************/

int dng_pthread_mutex_init(dng_pthread_mutex_t *mutex, void * /* attrs */)
{
	dng_pthread_mutex_t result;
	try {
		result = new(dng_pthread_mutex_impl);
	} catch (const std::bad_alloc &)
	{
		return -1;
	}

	if (result == NULL)
		return -1;
	*mutex = result;
	return 0;
}

/*****************************************************************************/

int dng_pthread_mutex_destroy(dng_pthread_mutex_t *mutex)
{
	if (*mutex == DNG_PTHREAD_MUTEX_INITIALIZER)
	{
		*mutex = NULL;
		return 0;
	}

	delete *mutex;
	*mutex = NULL;
	return 0;
}

/*****************************************************************************/

int dng_pthread_cond_init(dng_pthread_cond_t *cond, void * /* attrs */)
{
	dng_pthread_cond_t result;
	try {
		result = new(dng_pthread_cond_impl);
	} catch (const std::bad_alloc &)
	{
		return -1;
	}

	if (result == NULL)
		return -1;
	*cond = result;
	return 0;
}

/*****************************************************************************/

int dng_pthread_cond_destroy(dng_pthread_cond_t *cond)
{
	if (*cond == DNG_PTHREAD_COND_INITIALIZER)
	{
		*cond = NULL;
		return 0;
	}

	delete *cond;
	*cond = NULL;
	return 0;
}

/*****************************************************************************/

int dng_pthread_mutexattr_init(dng_pthread_mutexattr_t* mutexattr)
{
	return 0;
}

/*****************************************************************************/

int dng_pthread_mutexattr_settype(dng_pthread_mutexattr_t* mutexattr, int type)
{
	return 0;
}

/*****************************************************************************/

int dng_pthread_mutex_lock(dng_pthread_mutex_t *mutex)
{
	ValidateMutex(mutex);
	(*mutex)->Lock();
	return 0;
}

/*****************************************************************************/

int dng_pthread_mutex_unlock(dng_pthread_mutex_t *mutex)
{
	ValidateMutex(mutex);
	(*mutex)->Unlock();
	return 0;
}

/*****************************************************************************/

static int cond_wait_internal(dng_pthread_cond_t *cond, dng_pthread_mutex_t *mutex, int timeout_milliseconds)
{
	dng_pthread_cond_impl &real_cond = **cond;
	dng_pthread_mutex_impl &real_mutex = **mutex;

	waiter this_wait;
	HANDLE semaphore = GetThreadSemaphore();
	int my_generation; // The broadcast generation this waiter is in

	{
		this_wait.next = NULL;
		this_wait.semaphore = semaphore;
		this_wait.chosen_by_signal = 0;

		ScopedLock lock1(real_cond.lock);

		// Add this waiter to the end of the list.
		this_wait.prev = real_cond.tail_waiter;
		if (real_cond.tail_waiter != NULL)
			real_cond.tail_waiter->next = &this_wait;
		real_cond.tail_waiter = &this_wait;

		// If the list was empty, set the head of the list to this waiter.
		if (real_cond.head_waiter == NULL)
			real_cond.head_waiter = &this_wait;

		// Note which broadcast generation this waiter belongs to.
		my_generation = real_cond.broadcast_generation;
	}

	real_mutex.Unlock();

	DWORD result = ::WaitForSingleObject(semaphore, timeout_milliseconds);

	if (result == WAIT_TIMEOUT)
	{
		// If the wait timed out, this thread is likely still on the waiters list
		// of the condition. However, there is a race in that the thread may have been
		// signaled or broadcast between when WaitForSingleObject decided
		// we had timed out and this code running.

		bool mustConsumeSemaphore = false;
		{
			ScopedLock lock2(real_cond.lock);

			bool chosen_by_signal = this_wait.chosen_by_signal;
			bool chosen_by_broadcast = my_generation != real_cond.broadcast_generation;

			if (chosen_by_signal || chosen_by_broadcast)
				mustConsumeSemaphore = true;
			else
			{
				// Still on waiters list. Remove this waiter from list.
				if (this_wait.next != NULL)
					this_wait.next->prev = this_wait.prev;
				else
					real_cond.tail_waiter = this_wait.prev;

				if (this_wait.prev != NULL)
					this_wait.prev->next = this_wait.next;
				else
					real_cond.head_waiter = this_wait.next;
			}
		}

		if (mustConsumeSemaphore)
		{
			::WaitForSingleObject(semaphore, INFINITE);
			result = WAIT_OBJECT_0;
		}
	}
	else
		DNG_ASSERT (result == WAIT_OBJECT_0, "pthread emulation logic error");

	// reacquire the mutex
	real_mutex.Lock();

	return (result == WAIT_TIMEOUT) ? DNG_ETIMEDOUT : 0;
}

/*****************************************************************************/

int dng_pthread_cond_wait(dng_pthread_cond_t *cond, dng_pthread_mutex_t *mutex)
{
	ValidateCond(cond);

	return cond_wait_internal(cond, mutex, INFINITE);
}

/*****************************************************************************/

int dng_pthread_cond_timedwait(dng_pthread_cond_t *cond, dng_pthread_mutex_t *mutex, struct dng_timespec *latest_time)
{
	ValidateCond(cond);

	struct dng_timespec sys_timespec;

	dng_pthread_now (&sys_timespec);

	__int64 sys_time  = (__int64)sys_timespec.tv_sec * 1000000000 + sys_timespec.tv_nsec;
	__int64 lock_time = (__int64)latest_time->tv_sec * 1000000000 + latest_time->tv_nsec;

	int wait_millisecs = (int)((lock_time - sys_time + 500000) / 1000000);

	if (wait_millisecs < 0)
		wait_millisecs = 0;

	return cond_wait_internal(cond, mutex, wait_millisecs);
}

/*****************************************************************************/

int dng_pthread_cond_signal(dng_pthread_cond_t *cond)
{
	ValidateCond(cond);

	waiter *first;
	dng_pthread_cond_impl &real_cond = **cond;

	{
		ScopedLock lock(real_cond.lock);

		first = real_cond.head_waiter;
		if (first != NULL)
		{
			if (first->next != NULL)
				first->next->prev = NULL;
			else
				real_cond.tail_waiter = NULL; // Or first->prev, which is always NULL in this case

			first->chosen_by_signal = true;

			real_cond.head_waiter = first->next;
		}
	}

	if (first != NULL)
		::ReleaseSemaphore(first->semaphore, 1, NULL);

	return 0;
}

/*****************************************************************************/

int dng_pthread_cond_broadcast(dng_pthread_cond_t *cond)
{
	ValidateCond(cond);

	waiter *first;
	dng_pthread_cond_impl &real_cond = **cond;

	{
		ScopedLock lock(real_cond.lock);

		first = real_cond.head_waiter;
		real_cond.head_waiter = NULL;
		real_cond.tail_waiter = NULL;

		real_cond.broadcast_generation++;
	}

	while (first != NULL)
	{
		waiter *next = first->next;
		::ReleaseSemaphore(first->semaphore, 1, NULL);
		first = next;
	}

	return 0;
}

/*****************************************************************************/

int dng_pthread_once(dng_pthread_once_t *once, void (*init_func)())
{
	if (once == NULL || init_func == NULL)
		return EINVAL;

	if (once->inited)
		return 0;

	if (::InterlockedIncrement(&once->semaphore) == 0)
	{
		init_func();
		once->inited = 1;
	}
	else
	{
		while (!once->inited)
			Sleep(0);
	}

	return 0;
}

/*****************************************************************************/

int dng_pthread_key_create(dng_pthread_key_t * key, void (*destructor) (void *))
{
	if (destructor != NULL)
		return -1;

	DWORD result = ::TlsAlloc();
	if (result == TLS_OUT_OF_INDEXES)
		return -1;
	*key = (unsigned long)result;
	return 0;
}

/*****************************************************************************/

int dng_pthread_key_delete(dng_pthread_key_t key)
{
	if (::TlsFree((DWORD)key))
		return 0;
	return -1;
}

/*****************************************************************************/

int dng_pthread_setspecific(dng_pthread_key_t key, const void *value)
{
	if (::TlsSetValue((DWORD)key, const_cast<void *>(value)))
		return 0;
	return -1;
}

/*****************************************************************************/

void *dng_pthread_getspecific(dng_pthread_key_t key)
{
	return ::TlsGetValue((DWORD)key);
}

/*****************************************************************************/

namespace {
	struct rw_waiter {
		struct rw_waiter *prev;
		struct rw_waiter *next;
		HANDLE semaphore;
		bool is_writer;
	};
}

struct dng_pthread_rwlock_impl
{
	dng_pthread_mutex_impl mutex;

	rw_waiter *head_waiter;
	rw_waiter *tail_waiter;

	unsigned long readers_active;
	unsigned long writers_waiting;
	bool writer_active;

	dng_pthread_cond_impl read_wait;
	dng_pthread_cond_impl write_wait;

	dng_pthread_rwlock_impl ()
		: mutex ()
		, head_waiter (NULL)
		, tail_waiter (NULL)
		, readers_active (0)
		, writers_waiting (0)
		, read_wait ()
		, write_wait ()
		, writer_active (false)
	{
	}

	~dng_pthread_rwlock_impl ()
	{
	}

	void WakeHeadWaiter ()
	{
		HANDLE semaphore = head_waiter->semaphore;

		head_waiter = head_waiter->next;
		if (head_waiter == NULL)
			tail_waiter = NULL;

		::ReleaseSemaphore(semaphore, 1, NULL);
	}

};

/*****************************************************************************/

int dng_pthread_rwlock_init(dng_pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attrs)
{
	dng_pthread_rwlock_impl *newRWLock;

	newRWLock = new (std::nothrow) dng_pthread_rwlock_impl;
	if (newRWLock == NULL)
		return -1; // ENOMEM;

	*rwlock = newRWLock;

	return 0;
}

/*****************************************************************************/

int dng_pthread_rwlock_destroy(dng_pthread_rwlock_t *rwlock)
{
	dng_pthread_rwlock_impl &real_rwlock = **rwlock;

	{
		ScopedLock lock (real_rwlock.mutex);

		if (real_rwlock.head_waiter != NULL ||
			real_rwlock.readers_active != 0 ||
			real_rwlock.writers_waiting != 0 ||
			real_rwlock.writer_active)
			return -1; // EBUSY
	}

	delete *rwlock;
	*rwlock = NULL;
	return 0;
}

/*****************************************************************************/

#define CHECK_RWLOCK_STATE(real_rwlock) \
	DNG_ASSERT (!real_rwlock.writer_active || real_rwlock.readers_active == 0, "dng_pthread_rwlock_t logic error")

/*****************************************************************************/

int dng_pthread_rwlock_rdlock(dng_pthread_rwlock_t *rwlock)
{
	dng_pthread_rwlock_impl &real_rwlock = **rwlock;

	struct rw_waiter this_wait;
	bool doWait = false;;
	int result = 0;
	HANDLE semaphore;

		{

		ScopedLock lock (real_rwlock.mutex);

		CHECK_RWLOCK_STATE (real_rwlock);

		if (real_rwlock.writers_waiting > 0 || real_rwlock.writer_active)
		{
			semaphore = GetThreadSemaphore();

			this_wait.next = NULL;
			this_wait.semaphore = semaphore;
			this_wait.is_writer = false;

			// Add this waiter to the end of the list.
			this_wait.prev = real_rwlock.tail_waiter;
			if (real_rwlock.tail_waiter != NULL)
				real_rwlock.tail_waiter->next = &this_wait;
			real_rwlock.tail_waiter = &this_wait;

			// If the list was empty, set the head of the list to this waiter.
			if (real_rwlock.head_waiter == NULL)
				real_rwlock.head_waiter = &this_wait;

			doWait = true;
		}
		else
			real_rwlock.readers_active++;
	}

	if (result == 0 && doWait)
		result = (WaitForSingleObject(semaphore, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;

	return result;
}

/*****************************************************************************/

int dng_pthread_rwlock_tryrdlock(dng_pthread_rwlock_t *rwlock)
{
	dng_pthread_rwlock_impl &real_rwlock = **rwlock;

	ScopedLock lock (real_rwlock.mutex);

	CHECK_RWLOCK_STATE (real_rwlock);

	if (real_rwlock.writers_waiting == 0 && !real_rwlock.writer_active)
	{
		real_rwlock.readers_active++;
		return 0;
	}

	return -1;
}

/*****************************************************************************/

int dng_pthread_rwlock_trywrlock(dng_pthread_rwlock_t *rwlock)
{
	dng_pthread_rwlock_impl &real_rwlock = **rwlock;

	ScopedLock lock (real_rwlock.mutex);

	CHECK_RWLOCK_STATE (real_rwlock);

	if (real_rwlock.readers_active == 0 &&
		real_rwlock.writers_waiting == 0 &&
		!real_rwlock.writer_active)
		{
		real_rwlock.writer_active = true;
		return 0;
		}

	return -1;
}

/*****************************************************************************/

int dng_pthread_rwlock_unlock(dng_pthread_rwlock_t *rwlock)
	{
	dng_pthread_rwlock_impl &real_rwlock = **rwlock;

	int result = 0;

	ScopedLock lock (real_rwlock.mutex);

	CHECK_RWLOCK_STATE (real_rwlock);

	if (real_rwlock.readers_active > 0)
		--real_rwlock.readers_active;
	else
		real_rwlock.writer_active = false;

	while (real_rwlock.head_waiter != NULL)
	{
		if (real_rwlock.head_waiter->is_writer)
		{
			if (real_rwlock.readers_active == 0)
			{
			    real_rwlock.writers_waiting--;
			    real_rwlock.writer_active = true;
			    real_rwlock.WakeHeadWaiter ();
			}

			break;
		}
		else
		{
			++real_rwlock.readers_active;
			real_rwlock.WakeHeadWaiter ();
		}
	}

	return result;
	}

/*****************************************************************************/

int dng_pthread_rwlock_wrlock(dng_pthread_rwlock_t *rwlock)
	{
	dng_pthread_rwlock_impl &real_rwlock = **rwlock;

	int result = 0;
	struct rw_waiter this_wait;
	HANDLE semaphore;
	bool doWait = false;

	{
		ScopedLock lock (real_rwlock.mutex);

		CHECK_RWLOCK_STATE (real_rwlock);

		if (real_rwlock.readers_active ||
			real_rwlock.writers_waiting ||
			real_rwlock.writer_active)
			{
			semaphore = GetThreadSemaphore();

			this_wait.next = NULL;
			this_wait.semaphore = semaphore;
			this_wait.is_writer = true;

			// Add this waiter to the end of the list.
			this_wait.prev = real_rwlock.tail_waiter;
			if (real_rwlock.tail_waiter != NULL)
				real_rwlock.tail_waiter->next = &this_wait;
			real_rwlock.tail_waiter = &this_wait;

			// If the list was empty, set the head of the list to this waiter.
			if (real_rwlock.head_waiter == NULL)
				real_rwlock.head_waiter = &this_wait;

			real_rwlock.writers_waiting++;

			doWait = true;
		}
		else
			real_rwlock.writer_active = true;
	}

	if (result == 0 && doWait)
		result = (WaitForSingleObject(semaphore, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;

	return result;
	}

/*****************************************************************************/

void dng_pthread_disassociate()
{
	FreeThreadSemaphore();
}

void dng_pthread_terminate()
	{
	finalize_thread_TLS();
	}

/*****************************************************************************/

}	// extern "C"

/*****************************************************************************/

#endif

/*****************************************************************************/

int dng_pthread_now (struct timespec *now)
	{

	if (now == NULL)
		return -1; // EINVAL

	#if qWinOS

	FILETIME ft;
	::GetSystemTimeAsFileTime(&ft);

	__int64 sys_time = ((__int64)ft.dwHighDateTime << 32) + ft.dwLowDateTime;

	#define SecsFrom1601To1970 11644473600LL

	sys_time -= SecsFrom1601To1970 * 10000000LL;

	sys_time *= 100;	// Convert from 100ns to 1ns units

	now->tv_sec  = (long)(sys_time / 1000000000);
	now->tv_nsec = (long)(sys_time % 1000000000);

	#else

	struct timeval tv;

	if (gettimeofday (&tv, NULL) != 0)
		return errno;

	now->tv_sec  = tv.tv_sec;
	now->tv_nsec = tv.tv_usec * 1000;

	#endif

	return 0;

	}

/*****************************************************************************/

#endif qDNGThreadSafe

/*****************************************************************************/
