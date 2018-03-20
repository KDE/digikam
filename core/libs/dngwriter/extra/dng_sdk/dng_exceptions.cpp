/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_exceptions.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_exceptions.h"

#include "dng_flags.h"
#include "dng_globals.h"

/*****************************************************************************/

#ifndef qDNGReportErrors
#define qDNGReportErrors ((qDNGDebug && qMacOS) || qDNGValidate)
#endif

/*****************************************************************************/

void ReportWarning (const char *message,
				    const char *sub_message)
	{

	#if qDNGReportErrors

	fprintf (stderr, "*** Warning: %s", message);

	if (sub_message)
		{

		fprintf (stderr, " (%s)", sub_message);

		}

	fprintf (stderr, " ***\n");

	#else

	(void) message;
	(void) sub_message;

	#endif

	}

/*****************************************************************************/

void ReportError (const char *message,
				  const char *sub_message)
	{

	#if qDNGReportErrors

	fprintf (stderr, "*** Error: %s", message);

	if (sub_message)
		{

		fprintf (stderr, " (%s)", sub_message);

		}

	fprintf (stderr, " ***\n");

	// When Xcode won't let us set a break point, comment out the line below to
	// force the app to crash at this point (assuming the message is in protected
	// memory).

	// * const_cast< char *> (message) = 'X';

	#else

	(void) message;
	(void) sub_message;

	#endif

	}

/*****************************************************************************/

void Throw_dng_error (dng_error_code err,
					  const char *message,
					  const char *sub_message,
					  bool silent)
	{

	#if qDNGReportErrors

		{

		if (!message)
			{

			switch (err)
				{

				case dng_error_none:
				case dng_error_silent:
				case dng_error_user_canceled:
					{
					break;
					}

				case dng_error_not_yet_implemented:
					{
					message = "Not yet implemented";
					break;
					}

				case dng_error_host_insufficient:
					{
					message = "Host insufficient";
					break;
					}

				case dng_error_memory:
					{
					message = "Unable to allocate memory";
					break;
					}

				case dng_error_bad_format:
					{
					message = "File format is invalid";
					break;
					}

				case dng_error_matrix_math:
					{
					message = "Matrix math error";
					break;
					}

				case dng_error_open_file:
					{
					message = "Unable to open file";
					break;
					}

				case dng_error_read_file:
					{
					message = "File read error";
					break;
					}

				case dng_error_write_file:
					{
					message = "File write error";
					break;
					}

				case dng_error_end_of_file:
					{
					message = "Unexpected end-of-file";
					break;
					}

				case dng_error_file_is_damaged:
					{
					message = "File is damaged";
					break;
					}

				case dng_error_image_too_big_dng:
					{
					message = "Image is too big to save as DNG";
					break;
					}

				case dng_error_image_too_big_tiff:
					{
					message = "Image is too big to save as TIFF";
					break;
					}

				default:
					{
					message = "Unknown error";
					break;
					}

				}

			}

		if (message && !silent)
			{
			ReportError (message, sub_message);
			}

		}

	#else

	message;
	sub_message;
	silent;

	#endif

	throw dng_exception (err);

	}

/*****************************************************************************/
