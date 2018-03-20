/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_filter_task.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_filter_task.h"

#include "dng_bottlenecks.h"
#include "dng_exceptions.h"
#include "dng_image.h"
#include "dng_memory.h"
#include "dng_tag_types.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_filter_task::dng_filter_task (const dng_image &srcImage,
						 		  dng_image &dstImage)

	:	fSrcImage     (srcImage)
	,	fDstImage     (dstImage)

	,	fSrcPlane     (0                    )
	,	fSrcPlanes    (srcImage.Planes    ())
	,	fSrcPixelType (srcImage.PixelType ())

	,	fDstPlane     (0                    )
	,	fDstPlanes    (dstImage.Planes    ())
	,	fDstPixelType (dstImage.PixelType ())

	,	fSrcRepeat    (1, 1)

	{

	}

/*****************************************************************************/

dng_filter_task::~dng_filter_task ()
	{

	}

/*****************************************************************************/

void dng_filter_task::Start (uint32 threadCount,
							 const dng_point &tileSize,
							 dng_memory_allocator *allocator,
							 dng_abort_sniffer * /* sniffer */)
	{

	dng_point srcTileSize = SrcTileSize (tileSize);

	uint32 srcPixelSize = TagTypeSize (fSrcPixelType);

	uint32 srcBufferSize = srcTileSize.v *
						   RoundUpForPixelSize (srcTileSize.h, srcPixelSize) *
						   srcPixelSize *
						   fSrcPlanes;

	uint32 dstPixelSize = TagTypeSize (fDstPixelType);

	uint32 dstBufferSize = tileSize.v *
						   RoundUpForPixelSize (tileSize.h, dstPixelSize) *
						   dstPixelSize *
						   fDstPlanes;

	for (uint32 threadIndex = 0; threadIndex < threadCount; threadIndex++)
		{

		fSrcBuffer [threadIndex] . Reset (allocator->Allocate (srcBufferSize));

		fDstBuffer [threadIndex] . Reset (allocator->Allocate (dstBufferSize));

		// Zero buffers so any pad bytes have defined values.

		DoZeroBytes (fSrcBuffer [threadIndex]->Buffer      (),
					 fSrcBuffer [threadIndex]->LogicalSize ());

		DoZeroBytes (fDstBuffer [threadIndex]->Buffer      (),
					 fDstBuffer [threadIndex]->LogicalSize ());

		}

	}

/*****************************************************************************/

void dng_filter_task::Process (uint32 threadIndex,
							   const dng_rect &area,
							   dng_abort_sniffer * /* sniffer */)
	{

	// Find source area for this destination area.

	dng_rect srcArea = SrcArea (area);

	// Setup srcBuffer.

	dng_pixel_buffer srcBuffer;

	srcBuffer.fArea = srcArea;

	srcBuffer.fPlane  = fSrcPlane;
	srcBuffer.fPlanes = fSrcPlanes;

	srcBuffer.fPixelType  = fSrcPixelType;
	srcBuffer.fPixelSize  = TagTypeSize (fSrcPixelType);

	srcBuffer.fPlaneStep = RoundUpForPixelSize (srcArea.W (),
											    srcBuffer.fPixelSize);

	srcBuffer.fRowStep = srcBuffer.fPlaneStep *
						 srcBuffer.fPlanes;

	srcBuffer.fData = fSrcBuffer [threadIndex]->Buffer ();

	// Setup dstBuffer.

	dng_pixel_buffer dstBuffer;

	dstBuffer.fArea = area;

	dstBuffer.fPlane  = fDstPlane;
	dstBuffer.fPlanes = fDstPlanes;

	dstBuffer.fPixelType  = fDstPixelType;
	dstBuffer.fPixelSize  = TagTypeSize (fDstPixelType);

	dstBuffer.fPlaneStep = RoundUpForPixelSize (area.W (),
												dstBuffer.fPixelSize);

	dstBuffer.fRowStep = dstBuffer.fPlaneStep *
						 dstBuffer.fPlanes;

	dstBuffer.fData = fDstBuffer [threadIndex]->Buffer ();

	// Get source pixels.

	fSrcImage.Get (srcBuffer,
				   dng_image::edge_repeat,
				   fSrcRepeat.v,
				   fSrcRepeat.h);

	// Process area.

	ProcessArea (threadIndex,
				 srcBuffer,
				 dstBuffer);

	// Save result pixels.

	fDstImage.Put (dstBuffer);

	}

/*****************************************************************************/
