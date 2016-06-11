/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_file_stream.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Simple, portable, file read/write support.
 */

/*****************************************************************************/

#ifndef __dng_file_stream__
#define __dng_file_stream__

/*****************************************************************************/

#include "dng_stream.h"

/*****************************************************************************/

/// \brief A stream to/from a disk file. See dng_stream for read/write interface

class dng_file_stream: public dng_stream
	{

	private:

		FILE *fFile;

	public:

		/// Open a stream on a file.
		/// \param filename Pathname in platform synax.
		/// \param output Set to true if writing, false otherwise.
		/// \param bufferSize size of internal buffer to use. Defaults to 4k.

		dng_file_stream (const char *filename,
						 bool output = false,
						 uint32 bufferSize = kDefaultBufferSize);

		virtual ~dng_file_stream ();

	protected:

		virtual uint64 DoGetLength ();

		virtual void DoRead (void *data,
							 uint32 count,
							 uint64 offset);

		virtual void DoWrite (const void *data,
							  uint32 count,
							  uint64 offset);

	private:

		// Hidden copy constructor and assignment operator.

		dng_file_stream (const dng_file_stream &stream);

		dng_file_stream & operator= (const dng_file_stream &stream);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
