/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_types.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_types__
#define __dng_types__

/*****************************************************************************/

#include "dng_flags.h"

/*****************************************************************************/

// Standard integer types.

#ifdef _MSC_VER
#include <stddef.h>
#else
#include <stdint.h>
#endif

/*****************************************************************************/

#ifdef qDNGUseStdInt

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#else

typedef signed char		 int8;
typedef signed short	 int16;
#if __LP64__
typedef signed int		 int32;
#else
typedef signed long		 int32;
#endif
typedef signed long long int64;

typedef unsigned char      uint8;
typedef unsigned short	   uint16;
#if __LP64__
typedef unsigned int	   uint32;
#else
typedef unsigned long	   uint32;
#endif
typedef unsigned long long uint64;

#endif

typedef uintptr_t uintptr;

/*****************************************************************************/

typedef float  real32;
typedef double real64;

/*****************************************************************************/

/// \def Build a Macintosh style four-character constant in a compiler safe way.

#define DNG_CHAR4(a,b,c,d)	((((uint32) a) << 24) |\
							 (((uint32) b) << 16) |\
							 (((uint32) c) <<  8) |\
							 (((uint32) d)      ))

/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/*****************************************************************************/

// Visual Studio now prefers _hypot to hypot

#ifdef _MSC_VER

#ifdef hypot
#undef hypot
#endif

#define hypot _hypot

#endif

/*****************************************************************************/

#endif

/*****************************************************************************/
