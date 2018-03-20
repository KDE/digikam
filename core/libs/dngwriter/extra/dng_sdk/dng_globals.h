/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_globals.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Definitions of global variables controling DNG SDK behavior. Currenntly only used for validation control.
 */

/*****************************************************************************/

#ifndef __dng_globals__
#define __dng_globals__

/*****************************************************************************/

#include "dng_flags.h"
#include "dng_types.h"

/*****************************************************************************/

#if qDNGValidate

/// When validation (qValidate) is turned on, this globale enables verbose output about DNG tags and other properties.

extern bool gVerbose;

/// When validation (qValidate) is turned on, and verbose mode (gVerbose) is enabled, limits the number of lines of text that are dumped for each tag.

extern uint32 gDumpLineLimit;

#endif

/*****************************************************************************/

#endif

/*****************************************************************************/
