/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_assertions.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Conditionally compiled assertion check support.
 */

/*****************************************************************************/

#ifndef __dng_assertions__
#define __dng_assertions__

/*****************************************************************************/

#include "dng_exceptions.h"
#include "dng_flags.h"

/*****************************************************************************/

#if qDNGDebug

/// Platform specific function to display an assert.
void dng_show_message (const char *s);

// Show a formatted error message.

void dng_show_message_f (const char *fmt, ...);

#endif

/*****************************************************************************/

#ifndef DNG_ASSERT

#if qDNGDebug

/// Conditionally compiled macro to check an assertion and display a message if
/// it fails and assertions are compiled in via qDNGDebug
/// \param x Predicate which must be true.
/// \param y String to display if x is not true.

#define DNG_ASSERT(x,y) { if (!(x)) dng_show_message (y); }

#else

/// Conditionally compiled macro to check an assertion and display a message if
/// it fails and assertions are compiled in via qDNGDebug
/// \param x Predicate which must be true.
/// \param y String to display if x is not true.

#define DNG_ASSERT(x,y)

#endif
#endif

/*****************************************************************************/

#ifndef DNG_REQUIRE

#if qDNGDebug

/// Conditionally compiled macro to check an assertion, display a message, and throw
/// an exception if it fails and assertions are compiled in via qDNGDebug
/// \param condition Predicate which must be true.
/// \param msg String to display if condition is not true.

#define DNG_REQUIRE(condition,msg)				\
	do											\
		{										\
												\
		if (!(condition))						\
			{									\
												\
			dng_show_message (msg);				\
												\
			ThrowProgramError (msg);			\
												\
			}									\
												\
		}										\
	while (0)

#else

/// Conditionally compiled macro to check an assertion, display a message, and throw
/// an exception if it fails and assertions are compiled in via qDNGDebug
/// \param condition Predicate which must be true.
/// \param msg String to display if condition is not true.

#define DNG_REQUIRE(condition,msg)				\
	do											\
		{										\
												\
		if (!(condition))						\
			{									\
												\
			ThrowProgramError (msg);			\
												\
			}									\
												\
		}										\
	while (0)

#endif
#endif

/*****************************************************************************/

#ifndef DNG_REPORT

/// Macro to display an informational message
/// \param x String to display.

#define DNG_REPORT(x) DNG_ASSERT (false, x)
#endif

/*****************************************************************************/

#endif

/*****************************************************************************/
