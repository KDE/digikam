/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_fast_module.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Include file to set optimization to highest level for performance-critical routines.
 * Normal files should have otpimization set to normal level to save code size as there is less
 * cache pollution this way.
 */

/*****************************************************************************/

// Include this file in modules that contain routines that should be as fast
// as possible, even at the expense of slight code size increases.

/*****************************************************************************/

#ifdef _MSC_VER
#pragma optimize ("t", on)
#endif

/*****************************************************************************/
