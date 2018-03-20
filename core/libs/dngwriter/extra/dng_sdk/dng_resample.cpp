/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_resample.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_resample.h"

#include "dng_assertions.h"
#include "dng_bottlenecks.h"
#include "dng_filter_task.h"
#include "dng_host.h"
#include "dng_image.h"
#include "dng_memory.h"
#include "dng_pixel_buffer.h"
#include "dng_tag_types.h"
#include "dng_utils.h"

/******************************************************************************/

real64 dng_resample_bicubic::Extent () const
	{

	return 2.0;

	}

/******************************************************************************/

real64 dng_resample_bicubic::Evaluate (real64 x) const
	{

	const real64 A = -0.75;

	x = Abs_real64 (x);

    if (x >= 2.0)
        return 0.0;

    else if (x >= 1.0)
        return (((A * x - 5.0 * A) * x + 8.0 * A) * x - 4.0 * A);

    else
        return (((A + 2.0) * x - (A + 3.0)) * x * x + 1.0);

	}

/******************************************************************************/

const dng_resample_function & dng_resample_bicubic::Get ()
	{

	static dng_resample_bicubic static_dng_resample_bicubic;

	return static_dng_resample_bicubic;

	}

/*****************************************************************************/

dng_resample_coords::dng_resample_coords ()

	:	fOrigin (0)
	,	fCoords ()

	{

	}

/*****************************************************************************/

dng_resample_coords::~dng_resample_coords ()
	{

	}

/*****************************************************************************/

void dng_resample_coords::Initialize (int32 srcOrigin,
									  int32 dstOrigin,
									  uint32 srcCount,
									  uint32 dstCount,
									  dng_memory_allocator &allocator)
	{

	fOrigin = dstOrigin;

	uint32 dstEntries = RoundUp8 (dstCount);

	fCoords.Reset (allocator.Allocate (dstEntries * sizeof (int32)));

	int32 *coords = fCoords->Buffer_int32 ();

	real64 invScale = (real64) srcCount /
					  (real64) dstCount;

	for (uint32 j = 0; j < dstCount; j++)
		{

		real64 x = (real64) j + 0.5;

		real64 y = x * invScale - 0.5 + (real64) srcOrigin;

		coords [j] = Round_int32 (y * (real64) kResampleSubsampleCount);

		}

	// Pad out table by replicating last entry.

	for (uint32 k = dstCount; k < dstEntries; k++)
		{

		coords [k] = coords [dstCount - 1];

		}

	}

/*****************************************************************************/

dng_resample_weights::dng_resample_weights ()

	:	fRadius (0)

	,	fWeightStep (0)

	,	fWeights32 ()
	,	fWeights16 ()

	{

	}

/*****************************************************************************/

dng_resample_weights::~dng_resample_weights ()
	{

	}

/*****************************************************************************/

void dng_resample_weights::Initialize (real64 scale,
									   const dng_resample_function &kernel,
									   dng_memory_allocator &allocator)
	{

	uint32 j;

	// We only adjust the kernel size for scale factors less than 1.0.

	scale = Min_real64 (scale, 1.0);

	// Find radius of this kernel.

	fRadius = (uint32) (kernel.Extent () / scale + 0.9999);

	// Width is twice the radius.

	uint32 width = fRadius * 2;

	// Round to each set to weights to a multiple of 8 entries.

	fWeightStep = RoundUp8 (width);

	// Allocate and zero weight tables.

	fWeights32.Reset (allocator.Allocate (fWeightStep *
										  kResampleSubsampleCount *
										  sizeof (real32)));

	DoZeroBytes (fWeights32->Buffer		 (),
				 fWeights32->LogicalSize ());

	fWeights16.Reset (allocator.Allocate (fWeightStep *
										  kResampleSubsampleCount *
										  sizeof (int16)));

	DoZeroBytes (fWeights16->Buffer		 (),
				 fWeights16->LogicalSize ());

	// Compute kernel for each subsample values.

	for (uint32 sample = 0; sample < kResampleSubsampleCount; sample++)
		{

		real64 fract = sample * (1.0 / (real64) kResampleSubsampleCount);

		real32 *w32 = fWeights32->Buffer_real32 () + fWeightStep * sample;

		// Evaluate kernel function for 32 bit weights.

			{

			real64 t32 = 0.0;

			for (j = 0; j < width; j++)
				{

				int32 k = j - fRadius + 1;

				real64 x = (k - fract) * scale;

				w32 [j] = (real32) kernel.Evaluate (x);

				t32 += w32 [j];

				}

			// Scale 32 bit weights so total of weights is 1.0.

			real32 s32 = (real32) (1.0 / t32);

			for (j = 0; j < width; j++)
				{

				w32 [j] *= s32;

				}

			}

		// Round off 32 bit weights to 16 bit weights.

			{

			int16 *w16 = fWeights16->Buffer_int16 () + fWeightStep * sample;

			int32 t16 = 0;

			for (j = 0; j < width; j++)
				{

				w16 [j] = (int16) Round_int32 (w32 [j] * 16384.0);

				t16 += w16 [j];

				}

			// Adjust center entry for any round off error so total is
			// exactly 16384.

			w16 [fRadius - (fract >= 0.5 ? 0 : 1)] += (int16) (16384 - t16);

			}

		}

	}

/*****************************************************************************/

dng_resample_weights_2d::dng_resample_weights_2d ()

	:	fRadius (0)

	,	fRowStep (0)
	,	fColStep (0)

	,	fWeights32 ()
	,	fWeights16 ()

	{

	}

/*****************************************************************************/

dng_resample_weights_2d::~dng_resample_weights_2d ()
	{

	}

/*****************************************************************************/

void dng_resample_weights_2d::Initialize (const dng_resample_function &kernel,
										  dng_memory_allocator &allocator)
	{

	// Find radius of this kernel. Unlike with 1d resample weights (see
	// dng_resample_weights), we never scale up the kernel size.

	fRadius = (uint32) (kernel.Extent () + 0.9999);

	// Width is twice the radius.

	const uint32 width    = fRadius * 2;
	const uint32 widthSqr = width * width;

	const uint32 step = RoundUp8 (width * width);

	fRowStep = step * kResampleSubsampleCount2D;
	fColStep = step;

	// Allocate and zero weight tables.

	fWeights32.Reset (allocator.Allocate (step *
										  kResampleSubsampleCount2D *
										  kResampleSubsampleCount2D *
										  sizeof (real32)));

	DoZeroBytes (fWeights32->Buffer		 (),
				 fWeights32->LogicalSize ());

	fWeights16.Reset (allocator.Allocate (step *
										  kResampleSubsampleCount2D *
										  kResampleSubsampleCount2D *
										  sizeof (int16)));

	DoZeroBytes (fWeights16->Buffer		 (),
				 fWeights16->LogicalSize ());

	// Compute kernel for each subsample values.

	for (uint32 y = 0; y < kResampleSubsampleCount2D; y++)
		{

		real64 yFract = y * (1.0 / (real64) kResampleSubsampleCount2D);

		for (uint32 x = 0; x < kResampleSubsampleCount2D; x++)
			{

			real64 xFract = x * (1.0 / (real64) kResampleSubsampleCount2D);

			real32 *w32 = (real32 *) Weights32 (dng_point ((int32) y,
														   (int32) x));

			// Evaluate kernel function for 32 bit weights.

				{

				real64 t32 = 0.0;

				uint32 index = 0;

				for (uint32 i = 0; i < width; i++)
					{

					int32 yInt = ((int32) i) - fRadius + 1;
					real64 yPos = yInt - yFract;

					for (uint32 j = 0; j < width; j++)
						{

						int32 xInt = ((int32) j) - fRadius + 1;
						real64 xPos = xInt - xFract;

						#if 0

						// Radial.

						real64 dy2 = yPos * yPos;
						real64 dx2 = xPos * xPos;

						real64 r = sqrt (dx2 + dy2);

						w32 [index] = (real32) kernel.Evaluate (r);

						#else

						// Separable.

						w32 [index] = (real32) kernel.Evaluate (xPos) *
							          (real32) kernel.Evaluate (yPos);

						#endif

						t32 += w32 [index];

						index++;

						}

					}

				// Scale 32 bit weights so total of weights is 1.0.

				const real32 s32 = (real32) (1.0 / t32);

				for (uint32 i = 0; i < widthSqr; i++)
					{

					w32 [i] *= s32;

					}

				}

			// Round off 32 bit weights to 16 bit weights.

				{

				int16 *w16 = (int16 *) Weights16 (dng_point ((int32) y,
															 (int32) x));

				int32 t16 = 0;

				for (uint32 j = 0; j < widthSqr; j++)
					{

					w16 [j] = (int16) Round_int32 (w32 [j] * 16384.0);

					t16 += w16 [j];

					}

				// Adjust one of the center entries for any round off error so total
				// is exactly 16384.

				const uint32 xOffset      = fRadius - ((xFract >= 0.5) ? 0 : 1);
				const uint32 yOffset      = fRadius - ((yFract >= 0.5) ? 0 : 1);
				const uint32 centerOffset = width * yOffset + xOffset;

				w16 [centerOffset] += (int16) (16384 - t16);

				}

			}

		}

	}

/*****************************************************************************/

class dng_resample_task: public dng_filter_task
	{

	protected:

		dng_rect fSrcBounds;
		dng_rect fDstBounds;

		const dng_resample_function &fKernel;

		real64 fRowScale;
		real64 fColScale;

		dng_resample_coords fRowCoords;
		dng_resample_coords fColCoords;

		dng_resample_weights fWeightsV;
		dng_resample_weights fWeightsH;

		dng_point fSrcTileSize;

		AutoPtr<dng_memory_block> fTempBuffer [kMaxMPThreads];

	public:

		dng_resample_task (const dng_image &srcImage,
						   dng_image &dstImage,
						   const dng_rect &srcBounds,
						   const dng_rect &dstBounds,
						   const dng_resample_function &kernel);

		virtual dng_rect SrcArea (const dng_rect &dstArea);

		virtual dng_point SrcTileSize (const dng_point &dstTileSize);

		virtual void Start (uint32 threadCount,
							const dng_point &tileSize,
							dng_memory_allocator *allocator,
							dng_abort_sniffer *sniffer);

		virtual void ProcessArea (uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer);

	};

/*****************************************************************************/

dng_resample_task::dng_resample_task (const dng_image &srcImage,
						   			  dng_image &dstImage,
						   			  const dng_rect &srcBounds,
						   			  const dng_rect &dstBounds,
									  const dng_resample_function &kernel)

	:	dng_filter_task (srcImage,
						 dstImage)

	,	fSrcBounds (srcBounds)
	,	fDstBounds (dstBounds)

	,	fKernel (kernel)

	,	fRowScale (dstBounds.H () / (real64) srcBounds.H ())
	,	fColScale (dstBounds.W () / (real64) srcBounds.W ())

	,	fRowCoords ()
	,	fColCoords ()

	,	fWeightsV ()
	,	fWeightsH ()

	,	fSrcTileSize ()

	{

	if (srcImage.PixelSize  () <= 2 &&
		dstImage.PixelSize  () <= 2 &&
		srcImage.PixelRange () == dstImage.PixelRange ())
		{
		fSrcPixelType = ttShort;
		fDstPixelType = ttShort;
		}

	else
		{
		fSrcPixelType = ttFloat;
		fDstPixelType = ttFloat;
		}

	fUnitCell = dng_point (8, 8);

	fMaxTileSize.v = Pin_int32 (fUnitCell.v,
								Round_int32 (fMaxTileSize.v * fRowScale),
								fMaxTileSize.v);

	fMaxTileSize.h = Pin_int32 (fUnitCell.h,
								Round_int32 (fMaxTileSize.h * fColScale),
								fMaxTileSize.h);

	}

/*****************************************************************************/

dng_rect dng_resample_task::SrcArea (const dng_rect &dstArea)
	{

	int32 offsetV = fWeightsV.Offset ();
	int32 offsetH = fWeightsH.Offset ();

	uint32 widthV = fWeightsV.Width ();
	uint32 widthH = fWeightsH.Width ();

	dng_rect srcArea;

	srcArea.t = fRowCoords.Pixel (dstArea.t) + offsetV;
	srcArea.l = fColCoords.Pixel (dstArea.l) + offsetH;

	srcArea.b = fRowCoords.Pixel (dstArea.b - 1) + offsetV + widthV;
	srcArea.r = fColCoords.Pixel (dstArea.r - 1) + offsetH + widthH;

	return srcArea;

	}

/*****************************************************************************/

dng_point dng_resample_task::SrcTileSize (const dng_point & /* dstTileSize */)
	{

	return fSrcTileSize;

	}

/*****************************************************************************/

void dng_resample_task::Start (uint32 threadCount,
							   const dng_point &tileSize,
							   dng_memory_allocator *allocator,
							   dng_abort_sniffer *sniffer)
	{

	// Compute sub-pixel resolution coordinates in the source image for
	// each row and column of the destination area.

	fRowCoords.Initialize (fSrcBounds.t,
						   fDstBounds.t,
						   fSrcBounds.H (),
						   fDstBounds.H (),
						   *allocator);

	fColCoords.Initialize (fSrcBounds.l,
						   fDstBounds.l,
						   fSrcBounds.W (),
						   fDstBounds.W (),
						   *allocator);

	// Compute resampling kernels.

	fWeightsV.Initialize (fRowScale,
						  fKernel,
						  *allocator);

	fWeightsH.Initialize (fColScale,
						  fKernel,
						  *allocator);

	// Find upper bound on source tile.

	fSrcTileSize.v = Round_int32 (tileSize.v / fRowScale) + fWeightsV.Width () + 2;
	fSrcTileSize.h = Round_int32 (tileSize.h / fColScale) + fWeightsH.Width () + 2;

	// Allocate temp buffers.

	uint32 tempBufferSize = RoundUp8 (fSrcTileSize.h) * sizeof (real32);

	for (uint32 threadIndex = 0; threadIndex < threadCount; threadIndex++)
		{

		fTempBuffer [threadIndex] . Reset (allocator->Allocate (tempBufferSize));

		}

	// Allocate the pixel buffers.

	dng_filter_task::Start (threadCount,
							tileSize,
							allocator,
							sniffer);

	}

/*****************************************************************************/

void dng_resample_task::ProcessArea (uint32 threadIndex,
								     dng_pixel_buffer &srcBuffer,
								     dng_pixel_buffer &dstBuffer)
	{

	dng_rect srcArea = srcBuffer.fArea;
	dng_rect dstArea = dstBuffer.fArea;

	uint32 srcCols = srcArea.W ();
	uint32 dstCols = dstArea.W ();

	uint32 widthV = fWeightsV.Width ();
	uint32 widthH = fWeightsH.Width ();

	int32 offsetV = fWeightsV.Offset ();
	int32 offsetH = fWeightsH.Offset ();

	uint32 stepH = fWeightsH.Step ();

	const int32 *rowCoords = fRowCoords.Coords (0        );
	const int32 *colCoords = fColCoords.Coords (dstArea.l);

	if (fSrcPixelType == ttFloat)
		{

		const real32 *weightsH = fWeightsH.Weights32 (0);

		real32 *tPtr = fTempBuffer [threadIndex]->Buffer_real32 ();

		real32 *ttPtr = tPtr + offsetH - srcArea.l;

		for (int32 dstRow = dstArea.t; dstRow < dstArea.b; dstRow++)
			{

			int32 rowCoord = rowCoords [dstRow];

			int32 rowFract = rowCoord & kResampleSubsampleMask;

			const real32 *weightsV = fWeightsV.Weights32 (rowFract);

			int32 srcRow = (rowCoord >> kResampleSubsampleBits) + offsetV;

			for (uint32 plane = 0; plane < dstBuffer.fPlanes; plane++)
				{

				const real32 *sPtr = srcBuffer.ConstPixel_real32 (srcRow,
																  srcArea.l,
																  plane);

				DoResampleDown32 (sPtr,
								  tPtr,
								  srcCols,
								  srcBuffer.fRowStep,
								  weightsV,
								  widthV);

				real32 *dPtr = dstBuffer.DirtyPixel_real32 (dstRow,
															dstArea.l,
															plane);

				DoResampleAcross32 (ttPtr,
									dPtr,
									dstCols,
									colCoords,
									weightsH,
									widthH,
									stepH);

				}

			}

		}

	else
		{

		const int16 *weightsH = fWeightsH.Weights16 (0);

		uint16 *tPtr = fTempBuffer [threadIndex]->Buffer_uint16 ();

		uint16 *ttPtr = tPtr + offsetH - srcArea.l;

		uint32 pixelRange = fDstImage.PixelRange ();

		for (int32 dstRow = dstArea.t; dstRow < dstArea.b; dstRow++)
			{

			int32 rowCoord = rowCoords [dstRow];

			int32 rowFract = rowCoord & kResampleSubsampleMask;

			const int16 *weightsV = fWeightsV.Weights16 (rowFract);

			int32 srcRow = (rowCoord >> kResampleSubsampleBits) + offsetV;

			for (uint32 plane = 0; plane < dstBuffer.fPlanes; plane++)
				{

				const uint16 *sPtr = srcBuffer.ConstPixel_uint16 (srcRow,
																  srcArea.l,
																  plane);

				DoResampleDown16 (sPtr,
								  tPtr,
								  srcCols,
								  srcBuffer.fRowStep,
								  weightsV,
								  widthV,
								  pixelRange);

				uint16 *dPtr = dstBuffer.DirtyPixel_uint16 (dstRow,
															dstArea.l,
															plane);

				DoResampleAcross16 (ttPtr,
									dPtr,
									dstCols,
									colCoords,
									weightsH,
									widthH,
									stepH,
									pixelRange);

				}

			}

		}

	}

/*****************************************************************************/

void ResampleImage (dng_host &host,
					const dng_image &srcImage,
					dng_image &dstImage,
					const dng_rect &srcBounds,
					const dng_rect &dstBounds,
					const dng_resample_function &kernel)
	{

	dng_resample_task task (srcImage,
							dstImage,
							srcBounds,
							dstBounds,
							kernel);

	host.PerformAreaTask (task,
						  dstBounds);

	}

/*****************************************************************************/
