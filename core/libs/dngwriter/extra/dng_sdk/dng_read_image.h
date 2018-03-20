/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_read_image.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Support for DNG image reading.
 */

/*****************************************************************************/

#ifndef __dng_read_image__
#define __dng_read_image__

/*****************************************************************************/

#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_image.h"
#include "dng_memory.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_row_interleaved_image: public dng_image
	{

	private:

		dng_image &fImage;

		uint32 fFactor;

	public:

		dng_row_interleaved_image (dng_image &image,
								   uint32 factor);

		virtual void DoGet (dng_pixel_buffer &buffer) const;

		virtual void DoPut (const dng_pixel_buffer &buffer);

	private:

		int32 MapRow (int32 row) const;

	};

/*****************************************************************************/

/// \brief
///
///

class dng_read_image
	{

	protected:

		enum
			{

			// Target size for buffer used to copy data to the image.

			kImageBufferSize = 128 * 1024

			};

		AutoPtr<dng_memory_block> fCompressedBuffer;

		AutoPtr<dng_memory_block> fUncompressedBuffer;

		AutoPtr<dng_memory_block> fSubTileBlockBuffer;

	public:

		dng_read_image ();

		virtual ~dng_read_image ();

		///
		/// \param

		virtual bool CanRead (const dng_ifd &ifd);

		///
		/// \param host Host used for memory allocation, progress updating, and abort testing.
		/// \param ifd
		/// \param stream Stream to read image data from.
		/// \param image Result image to populate.

		virtual void Read (dng_host &host,
						   const dng_ifd &ifd,
						   dng_stream &stream,
						   dng_image &image);

	protected:

		virtual bool ReadUncompressed (dng_host &host,
									   const dng_ifd &ifd,
									   dng_stream &stream,
									   dng_image &image,
									   const dng_rect &tileArea,
									   uint32 plane,
									   uint32 planes);

		virtual bool ReadBaselineJPEG (dng_host &host,
									   const dng_ifd &ifd,
									   dng_stream &stream,
									   dng_image &image,
									   const dng_rect &tileArea,
									   uint32 plane,
									   uint32 planes,
									   uint32 tileByteCount);

		virtual bool ReadLosslessJPEG (dng_host &host,
									   const dng_ifd &ifd,
									   dng_stream &stream,
									   dng_image &image,
									   const dng_rect &tileArea,
									   uint32 plane,
									   uint32 planes,
									   uint32 tileByteCount);

		virtual bool CanReadTile (const dng_ifd &ifd);

		virtual bool NeedsCompressedBuffer (const dng_ifd &ifd);

		virtual void ReadTile (dng_host &host,
							   const dng_ifd &ifd,
							   dng_stream &stream,
							   dng_image &image,
							   const dng_rect &tileArea,
							   uint32 plane,
							   uint32 planes,
							   uint32 tileByteCount);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
