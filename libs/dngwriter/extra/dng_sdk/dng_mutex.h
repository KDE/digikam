/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_mutex.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/******************************************************************************/

#ifndef __dng_mutex__
#define __dng_mutex__

/******************************************************************************/

#include "dng_flags.h"

/******************************************************************************/

#include "dng_types.h"

#if qDNGThreadSafe

#include "dng_pthread.h"

#endif

/******************************************************************************/

class dng_mutex
	{

	public:

		enum
			{
			kDNGMutexLevelLeaf = 0xffffffffu
			};

		dng_mutex (const char *mutexName,
				   uint32 mutexLevel = kDNGMutexLevelLeaf);

		virtual ~dng_mutex ();

		void Lock ();

		void Unlock ();

		const char *MutexName () const;

	protected:

		#if qDNGThreadSafe

		pthread_mutex_t fPthreadMutex;

		const uint32 fMutexLevel;

		uint32 fRecursiveLockCount;

		dng_mutex *fPrevHeldMutex;

		const char * const fMutexName;

		friend class dng_condition;

		#endif

	private:

		// Hidden copy constructor and assignment operator.

		dng_mutex (const dng_mutex &mutex);

		dng_mutex & operator= (const dng_mutex &mutex);

	};

/*****************************************************************************/

class dng_lock_mutex
	{

	private:

		dng_mutex *fMutex;

	public:

		dng_lock_mutex (dng_mutex *mutex);

		~dng_lock_mutex ();

	private:

		// Hidden copy constructor and assignment operator.

		dng_lock_mutex (const dng_lock_mutex &lock);

		dng_lock_mutex & operator= (const dng_lock_mutex &lock);

	};

/*****************************************************************************/

class dng_unlock_mutex
	{

	private:

		dng_mutex *fMutex;

	public:

		dng_unlock_mutex (dng_mutex *mutex);

		~dng_unlock_mutex ();

	private:

		// Hidden copy constructor and assignment operator.

		dng_unlock_mutex (const dng_unlock_mutex &unlock);

		dng_unlock_mutex & operator= (const dng_unlock_mutex &unlock);

	};

/*****************************************************************************/

#if qDNGThreadSafe

/*****************************************************************************/

class dng_condition
	{

	public:

		dng_condition ();

		~dng_condition ();

		bool Wait (dng_mutex &mutex, double timeoutSecs = -1.0);

		void Signal ();

		void Broadcast ();

	protected:

		pthread_cond_t fPthreadCondition;

	private:

		// Hidden copy constructor and assignment operator.

		dng_condition (const dng_condition &condition);

		dng_condition & operator= (const dng_condition &condition);

	};

/*****************************************************************************/

#endif // qDNGThreadSafe

/*****************************************************************************/

#endif

/*****************************************************************************/
