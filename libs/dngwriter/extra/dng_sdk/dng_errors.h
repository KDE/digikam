/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_errors.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Error code values.
 */

/*****************************************************************************/

#ifndef __dng_errors__
#define __dng_errors__

/*****************************************************************************/

#include "dng_types.h"

/*****************************************************************************/

/// Type for all errors used in DNG SDK. Generally held inside a dng_exception.

typedef int32 dng_error_code;

enum
	{
	dng_error_none					= 0,		//< No error. Success.
	dng_error_unknown       		= 100000,	//< Logic or program error or other unclassifiable error.
	dng_error_not_yet_implemented,				//< Functionality requested is not yet implemented.
	dng_error_silent,							//< An error which should not be signalled to user.
	dng_error_user_canceled,					//< Processing stopped by user (or host application) request
	dng_error_host_insufficient,				//< Necessary host functionality is not present.
	dng_error_memory,							//< Out of memory.
	dng_error_bad_format,						//< File format is not valid.
	dng_error_matrix_math,						//< Matrix has wrong shape, is badly conditioned, or similar problem.
	dng_error_open_file,						//< Could not open file.
	dng_error_read_file,						//< Error reading file.
	dng_error_write_file,						//< Error writing file.
	dng_error_end_of_file,						//< Unexpected end of file.
	dng_error_file_is_damaged,					//< File is damaged in some way.
	dng_error_image_too_big_dng,				//< Image is too big to save as DNG.
	dng_error_image_too_big_tiff				//< Image is too big to save as TIFF.
	};

/*****************************************************************************/

#endif

/*****************************************************************************/
