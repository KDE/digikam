/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_exceptions.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * C++ exception support for DNG SDK.
*/

/*****************************************************************************/

#ifndef __dng_exceptions__
#define __dng_exceptions__

/*****************************************************************************/

#include "dng_errors.h"
#include "dng_flags.h"

/*****************************************************************************/

/// Display a warning message. Note that this may just eat the message.

void ReportWarning (const char *message,
				    const char *sub_message = NULL);

/*****************************************************************************/

/// Display an error message. Note that this may just eat the message.

void ReportError (const char *message,
				  const char *sub_message = NULL);

/*****************************************************************************/

/// \brief All exceptions thrown by the DNG SDK use this exception class.

class dng_exception
	{

	private:

		dng_error_code fErrorCode;

	public:

		/// Construct an exception representing the given error code.
		/// \param code Error code this exception is for.

		dng_exception (dng_error_code code)

			: fErrorCode (code)

			{
			}

		virtual ~dng_exception ()
			{
			}

		/// Getter for error code of this exception
		/// \retval The error code of this exception.

		dng_error_code ErrorCode () const
			{
			return fErrorCode;
			}

	};

/******************************************************************************/

/// \brief Throw an exception based on an arbitrary error code.

void Throw_dng_error (dng_error_code err,
					  const char * message = NULL,
					  const char * sub_message = NULL,
					  bool silent = false);

/******************************************************************************/

/// \brief Convenience function to throw dng_exception with error code if
/// error_code is not dng_error_none .

inline void Fail_dng_error (dng_error_code err)
	{

	if (err != dng_error_none)
		{

		Throw_dng_error (err);

		}

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_unknown .

inline void ThrowProgramError (const char * sub_message = NULL)
	{

	Throw_dng_error (dng_error_unknown, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_not_yet_implemented .

inline void ThrowNotYetImplemented (const char * sub_message = NULL)
	{

	Throw_dng_error (dng_error_not_yet_implemented, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_silent .

inline void ThrowSilentError ()
	{

	Throw_dng_error (dng_error_silent);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_user_canceled .

inline void ThrowUserCanceled ()
	{

	Throw_dng_error (dng_error_user_canceled);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_host_insufficient .

inline void ThrowHostInsufficient (const char * sub_message = NULL)
	{

	Throw_dng_error (dng_error_host_insufficient, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_memory .

inline void ThrowMemoryFull (const char * sub_message = NULL)
	{

	Throw_dng_error (dng_error_memory, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_bad_format .

inline void ThrowBadFormat (const char * sub_message = NULL)
	{

	Throw_dng_error (dng_error_bad_format, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_matrix_math .

inline void ThrowMatrixMath (const char * sub_message = NULL)
	{

	Throw_dng_error (dng_error_matrix_math, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_open_file .

inline void ThrowOpenFile (const char * sub_message = NULL, bool silent = false)
	{

	Throw_dng_error (dng_error_open_file, NULL, sub_message, silent);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_read_file .

inline void ThrowReadFile (const char *sub_message = NULL)
	{

	Throw_dng_error (dng_error_read_file, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_write_file .

inline void ThrowWriteFile (const char *sub_message = NULL)
	{

	Throw_dng_error (dng_error_write_file, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_end_of_file .

inline void ThrowEndOfFile (const char *sub_message = NULL)
	{

	Throw_dng_error (dng_error_end_of_file, NULL, sub_message);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_file_is_damaged .

inline void ThrowFileIsDamaged ()
	{

	Throw_dng_error (dng_error_file_is_damaged);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_image_too_big_dng .

inline void ThrowImageTooBigDNG ()
	{

	Throw_dng_error (dng_error_image_too_big_dng);

	}

/*****************************************************************************/

/// \brief Convenience function to throw dng_exception with error code
/// dng_error_image_too_big_tiff .

inline void ThrowImageTooBigTIFF ()
	{

	Throw_dng_error (dng_error_image_too_big_tiff);

	}

/*****************************************************************************/

#endif

/*****************************************************************************/
