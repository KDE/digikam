/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_area_task.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Class to handle partitioning a rectangular image processing operation taking into account multiple processing resources and memory constraints.
 */

/*****************************************************************************/

#ifndef __dng_area_task__
#define __dng_area_task__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_point.h"
#include "dng_types.h"

/*****************************************************************************/

/// \brief Abstract class for rectangular processing operations with support for partitioning across multiple processing resources and observing memory constraints.

class dng_area_task
	{

	protected:

		uint32 fMaxThreads;

		uint32 fMinTaskArea;

		dng_point fUnitCell;

		dng_point fMaxTileSize;

	public:

		dng_area_task ();

		virtual ~dng_area_task ();

		/// Getter for the maximum number of threads (resources) that can be used for processing
		///
		/// \retval Number of threads, minimum of 1, that can be used for this task.

		virtual uint32 MaxThreads () const
			{
			return fMaxThreads;
			}

		/// Getter for minimum area of a partitioned rectangle.
		/// Often it is not profitable to use more resources if it requires partitioning the input into chunks that are too small,
		/// as the overhead increases more than the speedup. This method can be ovreridden for a specific task to indicate the smallest
		/// area for partitioning. Default is 256x256 pixels.
		///
		/// \retval Minimum area for a partitoned tile in order to give performant operation. (Partitions can be smaller due to small inputs and edge cases.)

		virtual uint32 MinTaskArea () const
			{
			return fMinTaskArea;
			}

		/// Getter for dimensions of which partitioned tiles should be a multiple.
		/// Various methods of processing prefer certain alignments. The partitioning attempts to construct tiles such that the
		/// sizes are a multiple of the dimensions of this point.
		///
		/// \retval a point giving preferred alignment in x and y

		virtual dng_point UnitCell () const
			{
			return fUnitCell;
			}

		/// Getter for maximum size of a tile for processing.
		/// Often processing will need to allocate temporary buffers or use other resources that are either fixed or in limited supply.
		/// The maximum tile size forces further partitioning if the tile is bigger than this size.
		///
		/// \retval Maximum tile size allowed for this area task.

		virtual dng_point MaxTileSize () const
			{
			return fMaxTileSize;
			}

		/// Getter for RepeatingTile1.
		/// RepeatingTile1, RepeatingTile2, and RepeatingTile3 are used to establish a set of 0 to 3 tile patterns for which
		/// the resulting partitions that the final Process method is called on will not cross tile boundaries in any of the
		/// tile patterns. This can be used for a processing routine that needs to read from two tiles and write to a third
		/// such that all the tiles are aligned and sized in a certain way. A RepeatingTile value is valid if it is non-empty.
		/// Higher numbered RepeatingTile patterns are only used if all lower ones are non-empty. A RepeatingTile pattern must
		/// be a multiple of UnitCell in size for all constraints of the partitionerr to be met.

		virtual dng_rect RepeatingTile1 () const;

		/// Getter for RepeatingTile2.
		/// RepeatingTile1, RepeatingTile2, and RepeatingTile3 are used to establish a set of 0 to 3 tile patterns for which
		/// the resulting partitions that the final Process method is called on will not cross tile boundaries in any of the
		/// tile patterns. This can be used for a processing routine that needs to read from two tiles and write to a third
		/// such that all the tiles are aligned and sized in a certain way. A RepeatingTile value is valid if it is non-empty.
		/// Higher numbered RepeatingTile patterns are only used if all lower ones are non-empty. A RepeatingTile pattern must
		/// be a multiple of UnitCell in size for all constraints of the partitionerr to be met.

		virtual dng_rect RepeatingTile2 () const;

		/// Getter for RepeatingTile3.
		/// RepeatingTile1, RepeatingTile2, and RepeatingTile3 are used to establish a set of 0 to 3 tile patterns for which
		/// the resulting partitions that the final Process method is called on will not cross tile boundaries in any of the
		/// tile patterns. This can be used for a processing routine that needs to read from two tiles and write to a third
		/// such that all the tiles are aligned and sized in a certain way. A RepeatingTile value is valid if it is non-empty.
		/// Higher numbered RepeatingTile patterns are only used if all lower ones are non-empty. A RepeatingTile pattern must
		/// be a multiple of UnitCell in size for all constraints of the partitionerr to be met.

		virtual dng_rect RepeatingTile3 () const;

		/// Task startup method called before any processing is done on partitions.
		/// The Start method is called before any processing is done and can be overridden to allocate temporary buffers, etc.
		///
		/// \param threadCount Total number of threads that will be used for processing. Less than or equal to MaxThreads.
		/// \param tileSize Size of source tiles which will be processed. (Not all tiles will be this size due to edge conditions.)
		/// \param allocator dng_memory_allocator to use for allocating temporary buffers, etc.
		/// \param sniffer Sniffer to test for user cancellation and to set up progress.

		virtual void Start (uint32 threadCount,
							const dng_point &tileSize,
							dng_memory_allocator *allocator,
							dng_abort_sniffer *sniffer);

		/// Process one tile or fully partitioned area.
		/// This method is overridden by derived classes to implement the actual image processing. Note that the sniffer can be ignored if it is certain that a
		/// processing task will complete very quickly.
		/// This method should never be called directly but rather accessed via Process.
		/// There is no allocator parameter as all allocation should be done in Start.
		///
		/// \param threadIndex 0 to threadCount - 1 index indicating which thread this is. (Can be used to get a thread-specific buffer allocated in the Start method.)
		/// \param tile Area to process.
		/// \param sniffer dng_abort_sniffer to use to check for user cancellation and progress updates.

		virtual void Process (uint32 threadIndex,
							  const dng_rect &tile,
							  dng_abort_sniffer *sniffer) = 0;

		/// Task computation finalization and teardown method.
		/// Called after all resources have completed processing. Can be overridden to accumulate results and free resources allocated in Start.
		///
		/// \param threadCount Number of threads used for processing. Same as value passed to Start.

		virtual void Finish (uint32 threadCount);

		/// Find tile size taking into account repeating tiles, unit cell, and maximum tile size.
		/// \param area Computation area for which to find tile size.
		/// \retval Tile size as height and width in point.

		dng_point FindTileSize (const dng_rect &area) const;

		/// Handle one resource's worth of partitioned tiles.
		/// Called after thread partitioning has already been done. Area may be further subdivided to handle maximum tile size, etc.
		/// It will be rare to override this method.
		///
		/// \param threadIndex 0 to threadCount - 1 index indicating which thread this is.
		/// \param area Tile area partitioned to this resource.
		/// \param tileSize
		/// \param sniffer dng_abort_sniffer to use to check for user cancellation and progress updates.

		void ProcessOnThread (uint32 threadIndex,
							  const dng_rect &area,
							  const dng_point &tileSize,
							  dng_abort_sniffer *sniffer);

		/// Default resource partitioner that assumes a single resource to be used for processing.
		/// Implementations that are aware of multiple processing resources should override (replace) this method.
		/// This is usually done in dng_host::PerformAreaTask .
		/// \param task The task to perform.
		/// \param area The area on which mage processing should be performed.
		/// \param allocator dng_memory_allocator to use for allocating temporary buffers, etc.
		/// \param sniffer dng_abort_sniffer to use to check for user cancellation and progress updates.

		static void Perform (dng_area_task &task,
				  			 const dng_rect &area,
				  			 dng_memory_allocator *allocator,
				  			 dng_abort_sniffer *sniffer);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
