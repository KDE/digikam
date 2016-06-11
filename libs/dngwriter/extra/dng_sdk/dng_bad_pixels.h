/*****************************************************************************/
// Copyright 2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_bad_pixels.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_bad_pixels__
#define __dng_bad_pixels__

/*****************************************************************************/

#include "dng_opcodes.h"

#include <vector>

/*****************************************************************************/

class dng_opcode_FixBadPixelsConstant: public dng_filter_opcode
	{

	private:

		uint32 fConstant;

		uint32 fBayerPhase;

	public:

		dng_opcode_FixBadPixelsConstant (uint32 constant,
										 uint32 bayerPhase);

		dng_opcode_FixBadPixelsConstant (dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual dng_point SrcRepeat ();

		virtual dng_rect SrcArea (const dng_rect &dstArea,
								  const dng_rect &imageBounds);

		virtual void Prepare (dng_negative &negative,
							  uint32 threadCount,
							  const dng_point &tileSize,
							  const dng_rect &imageBounds,
							  uint32 imagePlanes,
							  uint32 bufferPixelType,
							  dng_memory_allocator &allocator);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	protected:

		bool IsGreen (int32 row, int32 col) const
			{
			return ((row + col + fBayerPhase + (fBayerPhase >> 1)) & 1) == 0;
			}

	};

/*****************************************************************************/

class dng_bad_pixel_list
	{

	public:

		enum
			{
			kNoIndex = 0xFFFFFFFF
			};

	private:

		// List of bad single pixels.

		std::vector<dng_point> fBadPoints;

		// List of bad rectangles (usually single rows or columns).

		std::vector<dng_rect> fBadRects;

	public:

		dng_bad_pixel_list ();

		uint32 PointCount () const
			{
			return (uint32) fBadPoints.size ();
			}

		const dng_point & Point (uint32 index) const
			{
			return fBadPoints [index];
			}

		uint32 RectCount () const
			{
			return (uint32) fBadRects.size ();
			}

		const dng_rect & Rect (uint32 index) const
			{
			return fBadRects [index];
			}

		bool IsEmpty () const
			{
			return PointCount () == 0 &&
				   RectCount  () == 0;
			}

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		void AddPoint (const dng_point &pt);

		void AddRect (const dng_rect &r);

		void Sort ();

		bool IsPointIsolated (uint32 index,
							  uint32 radius) const;

		bool IsRectIsolated (uint32 index,
							 uint32 radius) const;

		bool IsPointValid (const dng_point &pt,
						   const dng_rect &imageBounds,
						   uint32 index = kNoIndex) const;

	};

/*****************************************************************************/

class dng_opcode_FixBadPixelsList: public dng_filter_opcode
	{

	protected:

		enum
			{
			kBadPointPadding = 2,
			kBadRectPadding  = 4
			};

	private:

		AutoPtr<dng_bad_pixel_list> fList;

		uint32 fBayerPhase;

	public:

		dng_opcode_FixBadPixelsList (AutoPtr<dng_bad_pixel_list> &list,
									 uint32 bayerPhase);

		dng_opcode_FixBadPixelsList (dng_stream &stream);

		virtual void PutData (dng_stream &stream) const;

		virtual dng_point SrcRepeat ();

		virtual dng_rect SrcArea (const dng_rect &dstArea,
								  const dng_rect &imageBounds);

		virtual void Prepare (dng_negative &negative,
							  uint32 threadCount,
							  const dng_point &tileSize,
							  const dng_rect &imageBounds,
							  uint32 imagePlanes,
							  uint32 bufferPixelType,
							  dng_memory_allocator &allocator);

		virtual void ProcessArea (dng_negative &negative,
								  uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer,
								  const dng_rect &dstArea,
								  const dng_rect &imageBounds);

	protected:

		bool IsGreen (int32 row, int32 col) const
			{
			return ((row + col + fBayerPhase + (fBayerPhase >> 1)) & 1) == 0;
			}

		virtual void FixIsolatedPixel (dng_pixel_buffer &buffer,
									   dng_point &badPoint);

		virtual void FixClusteredPixel (dng_pixel_buffer &buffer,
								        uint32 pointIndex,
										const dng_rect &imageBounds);

		virtual void FixSingleColumn (dng_pixel_buffer &buffer,
									  const dng_rect &badRect);

		virtual void FixSingleRow (dng_pixel_buffer &buffer,
								   const dng_rect &badRect);

		virtual void FixClusteredRect (dng_pixel_buffer &buffer,
									   const dng_rect &badRect,
									   const dng_rect &imageBounds);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
