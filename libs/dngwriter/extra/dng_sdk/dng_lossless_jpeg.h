/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_lossless_jpeg.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Functions for encoding and decoding lossless JPEG format.
 */

/*****************************************************************************/

#ifndef __dng_lossless_jpeg__
#define __dng_lossless_jpeg__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_spooler
	{

	protected:

		virtual ~dng_spooler ()
			{
			}

	public:

		virtual void Spool (const void *data,
							uint32 count) = 0;

	};

/*****************************************************************************/

void DecodeLosslessJPEG (dng_stream &stream,
					     dng_spooler &spooler,
					     uint32 minDecodedSize,
					     uint32 maxDecodedSize,
						 bool bug16);

/*****************************************************************************/

void EncodeLosslessJPEG (const uint16 *srcData,
						 uint32 srcRows,
						 uint32 srcCols,
						 uint32 srcChannels,
						 uint32 srcBitDepth,
						 int32 srcRowStep,
						 int32 srcColStep,
						 dng_stream &stream);

/*****************************************************************************/

#endif

/*****************************************************************************/
