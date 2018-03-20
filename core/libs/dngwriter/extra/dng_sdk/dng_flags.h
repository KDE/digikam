/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_flags.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Conditional compilation flags for DNG SDK.
 *
 * All conditional compilation macros for the DNG SDK begin with a lowercase 'q'.
 */

/*****************************************************************************/

#ifndef __dng_flags__
#define __dng_flags__

/*****************************************************************************/

/// \def qMcOS 1 if compiling for Mac OS X
/// \def qWinOS 1 if compiling for Windows

// Make sure qMacOS and qWinOS are defined.

#if !defined(qMacOS) || !defined(qWinOS)
#include "RawEnvironment.h"
#endif

#if !defined(qMacOS) || !defined(qWinOS)
#error Unable to figure out platform
#endif

/*****************************************************************************/

/// \def qDNGDebug 1 if debug code is compiled in, 0 otherwise. Enables assertions and other
/// debug checks in exchange for slower processing.

// Figure out if debug build or not.

#ifndef qDNGDebug

#if defined(Debug)
#define qDNGDebug Debug

#elif defined(_DEBUG)
#define qDNGDebug _DEBUG

#else
#define qDNGDebug 0

#endif
#endif

/*****************************************************************************/

// Figure out byte order.

/// \def qDNGBigEndian 1 if this target platform is big endian (e.g. PowerPC Macintosh), else 0
/// \def qDNGLittleEndian 1 if this target platform is little endian (e.g. x86 processors), else 0

#ifndef qDNGBigEndian

#if defined(qDNGLittleEndian)
#define qDNGBigEndian !qDNGLittleEndian

#elif defined(__POWERPC__)
#define qDNGBigEndian 1

#elif defined(__s390__)
#define qDNGBigEndian 1

#elif defined(__sparc__)
#define qDNGBigEndian 1

#elif defined(__INTEL__)
#define qDNGBigEndian 0

#elif defined(_M_IX86)
#define qDNGBigEndian 0

#elif defined(_M_X64)
#define qDNGBigEndian 0

#elif defined(__LITTLE_ENDIAN__)
#define qDNGBigEndian 0

#elif defined(__BIG_ENDIAN__)
#define qDNGBigEndian 1

#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define qDNGBigEndian 1

#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define qDNGBigEndian 0

#else

#ifndef qXCodeRez
#error Unable to figure out byte order.
#endif

#endif
#endif

#ifndef qXCodeRez

#ifndef qDNGLittleEndian
#define qDNGLittleEndian !qDNGBigEndian
#endif

#endif

/*****************************************************************************/

/// \def qDNG64Bit 1 if this target platform uses 64-bit addresses, 0 otherwise

#ifndef qDNG64Bit

#if qMacOS

#ifdef __LP64__
#if    __LP64__
#define qDNG64Bit 1
#endif
#endif

#elif qWinOS

#ifdef WIN64
#if    WIN64
#define qDNG64Bit 1
#endif
#endif

#endif

#ifndef qDNG64Bit
#define qDNG64Bit 0
#endif

#endif

/*****************************************************************************/

/// \def qDNGThreadSafe 1 if target platform has thread support and threadsafe libraries, 0 otherwise

#ifndef qDNGThreadSafe
#define qDNGThreadSafe (qMacOS || qWinOS)
#endif

/*****************************************************************************/

/// \def qDNGValidateTarget 1 if dng_validate command line tool is being built, 0 otherwise

#ifndef qDNGValidateTarget
#define qDNGValidateTarget 0
#endif

/*****************************************************************************/

/// \def qDNGValidate 1 if DNG validation code is enabled, 0 otherwise.

#ifndef qDNGValidate
#define qDNGValidate qDNGValidateTarget
#endif

/*****************************************************************************/

/// \def qDNGPrintMessages 1 if dng_show_message should use fprintf to stderr.
/// 0 if it should use a platform specific interrupt mechanism.

#ifndef qDNGPrintMessages
#define qDNGPrintMessages qDNGValidate
#endif

/*****************************************************************************/

#endif

/*****************************************************************************/
