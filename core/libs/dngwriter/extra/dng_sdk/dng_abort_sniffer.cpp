/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_abort_sniffer.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_abort_sniffer.h"

#include "dng_mutex.h"

/*****************************************************************************/

#if qDNGThreadSafe

/*****************************************************************************/

class dng_priority_manager
	{

	private:

		dng_mutex fMutex;

		dng_condition fCondition;

		uint32 fCounter [dng_priority_count];

	public:

		dng_priority_manager ();

		void Increment (dng_priority priority);

		void Decrement (dng_priority priority);

		void Wait (dng_priority priority);

	private:

		dng_priority MinPriority ()
			{

			// Assumes mutex is locked.

			for (uint32 level = dng_priority_maximum;
				 level > dng_priority_minimum;
				 level--)
				{

				if (fCounter [level])
					{
					return (dng_priority) level;
					}

				}

			return dng_priority_minimum;

			}

	};

/*****************************************************************************/

dng_priority_manager::dng_priority_manager ()

	:	fMutex     ("dng_priority_manager::fMutex")
	,	fCondition ()

	{

	for (uint32 level = dng_priority_minimum;
		 level <= dng_priority_maximum;
		 level++)
		{

		fCounter [level] = 0;

		}

	}

/*****************************************************************************/

void dng_priority_manager::Increment (dng_priority priority)
	{

	dng_lock_mutex lock (&fMutex);

	fCounter [priority] += 1;

	}

/*****************************************************************************/

void dng_priority_manager::Decrement (dng_priority priority)
	{

	dng_lock_mutex lock (&fMutex);

	dng_priority oldMin = MinPriority ();

	fCounter [priority] -= 1;

	dng_priority newMin = MinPriority ();

	if (newMin < oldMin)
		{

		fCondition.Broadcast ();

		}

	}

/*****************************************************************************/

void dng_priority_manager::Wait (dng_priority priority)
	{

	if (priority < dng_priority_maximum)
		{

		dng_lock_mutex lock (&fMutex);

		while (priority < MinPriority ())
			{

			fCondition.Wait (fMutex);

			}

		}

	}

/*****************************************************************************/

static dng_priority_manager gPriorityManager;

/*****************************************************************************/

#endif

/*****************************************************************************/

dng_set_minimum_priority::dng_set_minimum_priority (dng_priority priority)

	:	fPriority (priority)

	{

	#if qDNGThreadSafe

	gPriorityManager.Increment (fPriority);

	#endif

	}

/*****************************************************************************/

dng_set_minimum_priority::~dng_set_minimum_priority ()
	{

	#if qDNGThreadSafe

	gPriorityManager.Decrement (fPriority);

	#endif

	}

/*****************************************************************************/

dng_abort_sniffer::dng_abort_sniffer ()

	:	fPriority (dng_priority_maximum)

	{

	}

/*****************************************************************************/

dng_abort_sniffer::~dng_abort_sniffer ()
	{

	}

/*****************************************************************************/

void dng_abort_sniffer::SniffForAbort (dng_abort_sniffer *sniffer)
	{

	if (sniffer)
		{

		#if qDNGThreadSafe

		gPriorityManager.Wait (sniffer->Priority ());

		#endif

		sniffer->Sniff ();

		}

	}

/*****************************************************************************/

void dng_abort_sniffer::StartTask (const char * /* name */,
								   real64 /* fract */)
	{

	}

/*****************************************************************************/

void dng_abort_sniffer::EndTask ()
	{

	}

/*****************************************************************************/

void dng_abort_sniffer::UpdateProgress (real64 /* fract */)
	{

	}

/*****************************************************************************/
