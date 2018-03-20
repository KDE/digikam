/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_filter_task.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Specialization of dng_area_task for processing an area from one dng_image to an area of another.
 */

/*****************************************************************************/

#ifndef __dng_filter_task__
#define __dng_filter_task__

/*****************************************************************************/

#include "dng_area_task.h"
#include "dng_auto_ptr.h"
#include "dng_point.h"
#include "dng_rect.h"
#include "dng_sdk_limits.h"

/*****************************************************************************/

/// \brief Represents a task which filters an area of a source dng_image to an area of a destination dng_image.

class dng_filter_task: public dng_area_task
	{

	protected:

		const dng_image &fSrcImage;

		dng_image &fDstImage;

		uint32 fSrcPlane;
		uint32 fSrcPlanes;
		uint32 fSrcPixelType;

		uint32 fDstPlane;
		uint32 fDstPlanes;
		uint32 fDstPixelType;

		dng_point fSrcRepeat;

		AutoPtr<dng_memory_block> fSrcBuffer [kMaxMPThreads];
		AutoPtr<dng_memory_block> fDstBuffer [kMaxMPThreads];

	public:

		/// Construct a filter task given a source and destination images.
		/// \param srcImage Image from which source pixels are read.
		/// \param dstImage Image to which result pixels are written.

		dng_filter_task (const dng_image &srcImage,
						 dng_image &dstImage);

		virtual ~dng_filter_task ();

		/// Compute the source area needed for a given destination area.
		/// Default implementation assumes destination area is equal to source area for all cases.
		/// \param dstArea Area to for which pixels will be computed.
		/// \retval The source area needed as input to calculate the requested destination area.

		virtual dng_rect SrcArea (const dng_rect &dstArea)
			{
			return dstArea;
			}

		/// Given a destination tile size, calculate input tile size.
		/// Simlar to SrcArea, and should seldom be overridden.
		/// \param dstTileSize The destination tile size that is targeted for output.
		/// \retval The source tile size needed to compute a tile of the destination size.

		virtual dng_point SrcTileSize (const dng_point &dstTileSize)
			{
			return SrcArea (dng_rect (dstTileSize)).Size ();
			}

		/// Implements filtering operation from one buffer to another.
		/// Source and destination pixels are set up in member fields of this class.
		/// Ideally, no allocation should be done in this routine.
		///
		/// \param threadIndex The thread on which this routine is being called, between 0 and threadCount - 1 for the threadCount passed to Start method.
		/// \param srcBuffer Input area and source pixels.
		/// \param dstBuffer Output area and destination pixels.

		virtual void ProcessArea (uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer) = 0;

		/// Called prior to processing on specific threads. Can be used to allocate per-thread memory buffers, etc.
		/// \param threadCount Total number of threads that will be used for processing. Less than or equal to MaxThreads of dng_area_task.
		/// \param tileSize Size of source tiles which will be processed. (Not all tiles will be this size due to edge conditions.)
		/// \param allocator dng_memory_allocator to use for allocating temporary buffers, etc.
		/// \param sniffer Sniffer to test for user cancellation and to set up progress.

		virtual void Start (uint32 threadCount,
							const dng_point &tileSize,
							dng_memory_allocator *allocator,
							dng_abort_sniffer *sniffer);

		/// Process one tile or partitioned area.
		/// Should not be overridden. Instead, override ProcessArea, which is where to implement filter processing for a specific type of dng_filter_task.
		/// There is no allocator parameter as all allocation should be done in Start.
		///
		/// \param threadIndex 0 to threadCount - 1 index indicating which thread this is. (Can be used to get a thread-specific buffer allocated in the Start method.)
		/// \param area Size of tiles to be used for sizing buffers, etc. (Edges of processing can be smaller.)
		/// \param sniffer dng_abort_sniffer to use to check for user cancellation and progress updates.

		virtual void Process (uint32 threadIndex,
							  const dng_rect &area,
							  dng_abort_sniffer *sniffer);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
