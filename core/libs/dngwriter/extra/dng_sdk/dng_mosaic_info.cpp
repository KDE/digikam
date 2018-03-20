/*****************************************************************************/
// Copyright 2006-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_mosaic_info.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_mosaic_info.h"

#include "dng_area_task.h"
#include "dng_assertions.h"
#include "dng_bottlenecks.h"
#include "dng_exceptions.h"
#include "dng_filter_task.h"
#include "dng_host.h"
#include "dng_ifd.h"
#include "dng_image.h"
#include "dng_info.h"
#include "dng_negative.h"
#include "dng_pixel_buffer.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_tile_iterator.h"
#include "dng_utils.h"

/*****************************************************************************/

// A interpolation kernel for a single pixel of a single plane.

class dng_bilinear_kernel
	{

	public:

		enum
			{
			kMaxCount = 8
			};

		uint32 fCount;

		dng_point fDelta [kMaxCount];

		real32 fWeight32 [kMaxCount];
		uint16 fWeight16 [kMaxCount];

		int32 fOffset [kMaxCount];

	public:

		dng_bilinear_kernel ()
			:	fCount (0)
			{
			}

		void Add (const dng_point &delta,
				  real32 weight);

		void Finalize (const dng_point &scale,
					   uint32 patRow,
					   uint32 patCol,
					   int32 rowStep,
					   int32 colStep);

	};

/*****************************************************************************/

void dng_bilinear_kernel::Add (const dng_point &delta,
				  			   real32 weight)
	{

	// Don't add zero weight elements.

	if (weight <= 0.0f)
		{
		return;
		}

	// If the delta already matches an existing element, just combine the
	// weights.

	for (uint32 j = 0; j < fCount; j++)
		{

		if (fDelta [j] == delta)
			{

			fWeight32 [j] += weight;

			return;

			}

		}

	// Add element to list.

	DNG_ASSERT (fCount < kMaxCount, "Too many kernel entries")

	fDelta    [fCount] = delta;
	fWeight32 [fCount] = weight;

	fCount++;

	}

/*****************************************************************************/

void dng_bilinear_kernel::Finalize (const dng_point &scale,
									uint32 patRow,
							   		uint32 patCol,
							   		int32 rowStep,
							   		int32 colStep)
	{

	uint32 j;

	// Adjust deltas to compensate for interpolation upscaling.

	for (j = 0; j < fCount; j++)
		{

		dng_point &delta = fDelta [j];

		if (scale.v == 2)
			{

			delta.v = (delta.v + (int32) (patRow & 1)) >> 1;

			}

		if (scale.h == 2)
			{

			delta.h = (delta.h + (int32) (patCol & 1)) >> 1;

			}

		}

	// Sort entries into row-column scan order.

	while (true)
		{

		bool didSwap = false;

		for (j = 1; j < fCount; j++)
			{

			dng_point &delta0 = fDelta [j - 1];
			dng_point &delta1 = fDelta [j    ];

			if (delta0.v > delta1.v ||
					(delta0.v == delta1.v &&
					 delta0.h >  delta1.h))
				{

				didSwap = true;

				dng_point tempDelta = delta0;

				delta0 = delta1;
				delta1 = tempDelta;

				real32 tempWeight = fWeight32 [j - 1];

				fWeight32 [j - 1] = fWeight32 [j];
				fWeight32 [j    ] = tempWeight;

				}

			}

		if (!didSwap)
			{
			break;
			}

		}

	// Calculate offsets.

	for (j = 0; j < fCount; j++)
		{

		fOffset [j] = rowStep * fDelta [j].v +
					  colStep * fDelta [j].h;

		}

	// Calculate 16-bit weights.

	uint16 total   = 0;
	uint32 biggest = 0;

	for (j = 0; j < fCount; j++)
		{

		// Round weights to 8 fractional bits.

		fWeight16 [j] = (uint16) Round_uint32 (fWeight32 [j] * 256.0);

		// Keep track of total of weights.

		total += fWeight16 [j];

		// Keep track of which weight is biggest.

		if (fWeight16 [biggest] < fWeight16 [j])
			{

			biggest = j;

			}

		}

	// Adjust largest entry so total of weights is exactly 256.

	fWeight16 [biggest] += (256 - total);

	// Recompute the floating point weights from the rounded integer weights
	// so results match more closely.

	for (j = 0; j < fCount; j++)
		{

		fWeight32 [j] = fWeight16 [j] * (1.0f / 256.0f);

		}

	}

/*****************************************************************************/

class dng_bilinear_pattern
	{

	public:

		enum
			{
			kMaxPattern = kMaxCFAPattern * 2
			};

		dng_point fScale;

		uint32 fPatRows;
		uint32 fPatCols;

		dng_bilinear_kernel fKernel [kMaxPattern]
					  		        [kMaxPattern];

		uint32 fCounts [kMaxPattern]
					   [kMaxPattern];

		int32 *fOffsets [kMaxPattern]
						[kMaxPattern];

		uint16 *fWeights16 [kMaxPattern]
						   [kMaxPattern];

		real32 *fWeights32 [kMaxPattern]
						   [kMaxPattern];

	public:

		dng_bilinear_pattern ()

			:	fScale ()
			,	fPatRows (0)
			,	fPatCols (0)

			{
			}

	private:

		uint32 DeltaRow (uint32 row, int32 delta)
			{
			return (row + fPatRows + delta) % fPatRows;
			}

		uint32 DeltaCol (uint32 col, int32 delta)
			{
			return (col + fPatCols + delta) % fPatCols;
			}

		real32 LinearWeight1 (int32 d1, int32 d2)
			{
			if (d1 == d2)
				return 1.0;
			else
				return d2 / (real32) (d2 - d1);
			}

		real32 LinearWeight2 (int32 d1, int32 d2)
			{
			if (d1 == d2)
				return 0.0;
			else
				return -d1 / (real32) (d2 - d1);
			}

	public:

		void Calculate (const dng_mosaic_info &info,
						uint32 dstPlane,
						int32 rowStep,
						int32 colStep);

	};

/*****************************************************************************/

void dng_bilinear_pattern::Calculate (const dng_mosaic_info &info,
						  			  uint32 dstPlane,
						  			  int32 rowStep,
						  			  int32 colStep)
	{

	uint32 j;
	uint32 k;
	uint32 patRow;
	uint32 patCol;

	// Find destination pattern size.

	fScale = info.FullScale ();

	fPatRows = info.fCFAPatternSize.v * fScale.v;
	fPatCols = info.fCFAPatternSize.h * fScale.h;

	// See if we need to scale up just while computing the kernels.

	dng_point tempScale (1, 1);

	if (info.fCFALayout >= 6)
		{

		tempScale = dng_point (2, 2);

		fPatRows *= tempScale.v;
		fPatCols *= tempScale.h;

		}

	// Find a boolean map for this plane color and layout.

	bool map [kMaxPattern]
			 [kMaxPattern];

	uint8 planeColor = info.fCFAPlaneColor [dstPlane];

	switch (info.fCFALayout)
		{

		case 1:		// Rectangular (or square) layout
			{

			for (j = 0; j < fPatRows; j++)
				{

				for (k = 0; k < fPatCols; k++)
					{

					map [j] [k] = (info.fCFAPattern [j] [k] == planeColor);

					}

				}

			break;

			}

		// Note that when the descriptions of the staggered patterns refer to even rows or
		// columns, this mean the second, fourth, etc. (i.e. using one-based numbering).
		// This needs to be clarified in the DNG specification.

		case 2:		// Staggered layout A: even (1-based) columns are offset down by 1/2 row
			{

			for (j = 0; j < fPatRows; j++)
				{

				for (k = 0; k < fPatCols; k++)
					{

					if ((j & 1) != (k & 1))
						{

						map [j] [k] = false;

						}

					else
						{

						map [j] [k] = (info.fCFAPattern [j >> 1] [k] == planeColor);

						}

					}

				}

			break;

			}

		case 3:		// Staggered layout B: even (1-based) columns are offset up by 1/2 row
			{

			for (j = 0; j < fPatRows; j++)
				{

				for (k = 0; k < fPatCols; k++)
					{

					if ((j & 1) == (k & 1))
						{

						map [j] [k] = false;

						}

					else
						{

						map [j] [k] = (info.fCFAPattern [j >> 1] [k] == planeColor);

						}

					}

				}

			break;

			}

		case 4:		// Staggered layout C: even (1-based) rows are offset right by 1/2 column
			{

			for (j = 0; j < fPatRows; j++)
				{

				for (k = 0; k < fPatCols; k++)
					{

					if ((j & 1) != (k & 1))
						{

						map [j] [k] = false;

						}

					else
						{

						map [j] [k] = (info.fCFAPattern [j] [k >> 1] == planeColor);

						}

					}

				}

			break;

			}

		case 5:		// Staggered layout D: even (1-based) rows are offset left by 1/2 column
			{

			for (j = 0; j < fPatRows; j++)
				{

				for (k = 0; k < fPatCols; k++)
					{

					if ((j & 1) == (k & 1))
						{

						map [j] [k] = false;

						}

					else
						{

						map [j] [k] = (info.fCFAPattern [j] [k >> 1] == planeColor);

						}

					}

				}

			break;

			}

		case 6:		// Staggered layout E: even rows are offset up by 1/2 row, even columns are offset left by 1/2 column
		case 7:		// Staggered layout F: even rows are offset up by 1/2 row, even columns are offset right by 1/2 column
		case 8:		// Staggered layout G: even rows are offset down by 1/2 row, even columns are offset left by 1/2 column
		case 9:		// Staggered layout H: even rows are offset down by 1/2 row, even columns are offset right by 1/2 column
			{

			uint32 eRow = (info.fCFALayout == 6 ||
						   info.fCFALayout == 7) ? 1 : 3;

			uint32 eCol = (info.fCFALayout == 6 ||
						   info.fCFALayout == 8) ? 1 : 3;

			for (j = 0; j < fPatRows; j++)
				{

				for (k = 0; k < fPatCols; k++)
					{

					uint32 jj = j & 3;
					uint32 kk = k & 3;

					if ((jj != 0 && jj != eRow) ||
						(kk != 0 && kk != eCol))
						{

						map [j] [k] = false;

						}

					else
						{

						map [j] [k] = (info.fCFAPattern [((j >> 1) & ~1) + Min_uint32 (jj, 1)]
														[((k >> 1) & ~1) + Min_uint32 (kk, 1)] == planeColor);

						}

					}

				}

			break;

			}

		default:
			ThrowProgramError ();
			break;
		}

	// Find projections of maps.

	bool mapH [kMaxPattern];
	bool mapV [kMaxPattern];

	for (j = 0; j < kMaxPattern; j++)
		{

		mapH [j] = false;
		mapV [j] = false;

		}

	for (j = 0; j < fPatRows; j++)
		{

		for (k = 0; k < fPatCols; k++)
			{

			if (map [j] [k])
				{

				mapV [j] = true;
				mapH [k] = true;

				}

			}

		}

	// Find kernel for each patten entry.

	for (patRow = 0; patRow < fPatRows; patRow += tempScale.v)
		{

		for (patCol = 0; patCol < fPatCols; patCol += tempScale.h)
			{

			dng_bilinear_kernel &kernel = fKernel [patRow] [patCol];

			// Special case no interpolation case.

			if (map [patRow] [patCol])
				{

				kernel.Add (dng_point (0, 0), 1.0f);

				continue;

				}

			// Special case common patterns in 3 by 3 neighborhood.

			uint32 n = DeltaRow (patRow, -1);
			uint32 s = DeltaRow (patRow,  1);
			uint32 w = DeltaCol (patCol, -1);
			uint32 e = DeltaCol (patCol,  1);

			bool mapNW = map [n] [w];
			bool mapN  = map [n] [patCol];
			bool mapNE = map [n] [e];

			bool mapW = map [patRow] [w];
			bool mapE = map [patRow] [e];

			bool mapSW = map [s] [w];
			bool mapS  = map [s] [patCol];
			bool mapSE = map [s] [e];

			// All sides.

			if (mapN && mapS && mapW && mapW)
				{

				kernel.Add (dng_point (-1,  0), 0.25f);
				kernel.Add (dng_point ( 0, -1), 0.25f);
				kernel.Add (dng_point ( 0,  1), 0.25f);
				kernel.Add (dng_point ( 1,  0), 0.25f);

				continue;

				}

			// N & S.

			if (mapN && mapS)
				{

				kernel.Add (dng_point (-1,  0), 0.5f);
				kernel.Add (dng_point ( 1,  0), 0.5f);

				continue;

				}

			// E & W.

			if (mapW && mapE)
				{

				kernel.Add (dng_point ( 0, -1), 0.5f);
				kernel.Add (dng_point ( 0,  1), 0.5f);

				continue;

				}

			// N & SW & SE.

			if (mapN && mapSW && mapSE)
				{

				kernel.Add (dng_point (-1,  0), 0.50f);
				kernel.Add (dng_point ( 1, -1), 0.25f);
				kernel.Add (dng_point ( 1,  1), 0.25f);

				continue;

				}

			// S & NW & NE.

			if (mapS && mapNW && mapNE)
				{

				kernel.Add (dng_point (-1, -1), 0.25f);
				kernel.Add (dng_point (-1,  1), 0.25f);
				kernel.Add (dng_point ( 1,  0), 0.50f);

				continue;

				}

			// W & NE & SE.

			if (mapW && mapNE && mapSE)
				{

				kernel.Add (dng_point (-1,  1), 0.25f);
				kernel.Add (dng_point ( 0, -1), 0.50f);
				kernel.Add (dng_point ( 1,  1), 0.25f);

				continue;

				}

			// E & NW & SW.

			if (mapE && mapNW && mapSW)
				{

				kernel.Add (dng_point (-1, -1), 0.25f);
				kernel.Add (dng_point ( 0,  1), 0.50f);
				kernel.Add (dng_point ( 1, -1), 0.25f);

				continue;

				}

			// Four corners.

			if (mapNW && mapNE && mapSE && mapSW)
				{

				kernel.Add (dng_point (-1, -1), 0.25f);
				kernel.Add (dng_point (-1,  1), 0.25f);
				kernel.Add (dng_point ( 1, -1), 0.25f);
				kernel.Add (dng_point ( 1,  1), 0.25f);

				continue;

				}

			// NW & SE

			if (mapNW && mapSE)
				{

				kernel.Add (dng_point (-1, -1), 0.50f);
				kernel.Add (dng_point ( 1,  1), 0.50f);

				continue;

				}

			// NE & SW

			if (mapNE && mapSW)
				{

				kernel.Add (dng_point (-1,  1), 0.50f);
				kernel.Add (dng_point ( 1, -1), 0.50f);

				continue;

				}

			// Else use double-bilinear kernel.

			int32 dv1 = 0;
			int32 dv2 = 0;

			while (!mapV [DeltaRow (patRow, dv1)])
				{
				dv1--;
				}

			while (!mapV [DeltaRow (patRow, dv2)])
				{
				dv2++;
				}

			real32 w1 = LinearWeight1 (dv1, dv2) * 0.5f;
			real32 w2 = LinearWeight2 (dv1, dv2) * 0.5f;

			int32 v1 = DeltaRow (patRow, dv1);
			int32 v2 = DeltaRow (patRow, dv2);

			int32 dh1 = 0;
			int32 dh2 = 0;

			while (!map [v1] [DeltaCol (patCol, dh1)])
				{
				dh1--;
				}

			while (!map [v1] [DeltaCol (patCol, dh2)])
				{
				dh2++;
				}

			kernel.Add (dng_point (dv1, dh1),
						LinearWeight1 (dh1, dh2) * w1);

			kernel.Add (dng_point (dv1, dh2),
						LinearWeight2 (dh1, dh2) * w1);

			dh1 = 0;
			dh2 = 0;

			while (!map [v2] [DeltaCol (patCol, dh1)])
				{
				dh1--;
				}

			while (!map [v2] [DeltaCol (patCol, dh2)])
				{
				dh2++;
				}

			kernel.Add (dng_point (dv2, dh1),
						LinearWeight1 (dh1, dh2) * w2);

			kernel.Add (dng_point (dv2, dh2),
						LinearWeight2 (dh1, dh2) * w2);

			dh1 = 0;
			dh2 = 0;

			while (!mapH [DeltaCol (patCol, dh1)])
				{
				dh1--;
				}

			while (!mapH [DeltaCol (patCol, dh2)])
				{
				dh2++;
				}

			w1 = LinearWeight1 (dh1, dh2) * 0.5f;
			w2 = LinearWeight2 (dh1, dh2) * 0.5f;

			int32 h1 = DeltaCol (patCol, dh1);
			int32 h2 = DeltaCol (patCol, dh2);

			dv1 = 0;
			dv2 = 0;

			while (!map [DeltaRow (patRow, dv1)] [h1])
				{
				dv1--;
				}

			while (!map [DeltaRow (patRow, dv2)] [h1])
				{
				dv2++;
				}

			kernel.Add (dng_point (dv1, dh1),
						LinearWeight1 (dv1, dv2) * w1);

			kernel.Add (dng_point (dv2, dh1),
						LinearWeight2 (dv1, dv2) * w1);

			dv1 = 0;
			dv2 = 0;

			while (!map [DeltaRow (patRow, dv1)] [h2])
				{
				dv1--;
				}

			while (!map [DeltaRow (patRow, dv2)] [h2])
				{
				dv2++;
				}

			kernel.Add (dng_point (dv1, dh2),
						LinearWeight1 (dv1, dv2) * w2);

			kernel.Add (dng_point (dv2, dh2),
						LinearWeight2 (dv1, dv2) * w2);

			}

		}

	// Deal with temp scale case.

	if (tempScale == dng_point (2, 2))
		{

		fPatRows /= tempScale.v;
		fPatCols /= tempScale.h;

		for (patRow = 0; patRow < fPatRows; patRow++)
			{

			for (patCol = 0; patCol < fPatCols; patCol++)
				{

				int32 patRow2 = patRow << 1;
				int32 patCol2 = patCol << 1;

				dng_bilinear_kernel &kernel = fKernel [patRow2] [patCol2];

				for (j = 0; j < kernel.fCount; j++)
					{

					int32 x = patRow2 + kernel.fDelta [j].v;

					if ((x & 3) != 0)
						{
						x = (x & ~3) + 2;
						}

					kernel.fDelta [j].v = ((x - patRow2) >> 1);

					x = patCol2 + kernel.fDelta [j].h;

					if ((x & 3) != 0)
						{
						x = (x & ~3) + 2;
						}

					kernel.fDelta [j].h = ((x - patCol2) >> 1);

					}

				kernel.Finalize (fScale,
								 patRow,
								 patCol,
								 rowStep,
								 colStep);

				fCounts    [patRow] [patCol] = kernel.fCount;
				fOffsets   [patRow] [patCol] = kernel.fOffset;
				fWeights16 [patRow] [patCol] = kernel.fWeight16;
				fWeights32 [patRow] [patCol] = kernel.fWeight32;

				}

			}

		}

	// Non-temp scale case.

	else
		{

		for (patRow = 0; patRow < fPatRows; patRow++)
			{

			for (patCol = 0; patCol < fPatCols; patCol++)
				{

				dng_bilinear_kernel &kernel = fKernel [patRow] [patCol];

				kernel.Finalize (fScale,
								 patRow,
								 patCol,
								 rowStep,
								 colStep);

				fCounts    [patRow] [patCol] = kernel.fCount;
				fOffsets   [patRow] [patCol] = kernel.fOffset;
				fWeights16 [patRow] [patCol] = kernel.fWeight16;
				fWeights32 [patRow] [patCol] = kernel.fWeight32;

				}

			}

		}

	}

/*****************************************************************************/

class dng_bilinear_interpolator
	{

	private:

		dng_bilinear_pattern fPattern [kMaxColorPlanes];

	public:

		dng_bilinear_interpolator (const dng_mosaic_info &info,
								   int32 rowStep,
								   int32 colStep);

		void Interpolate (dng_pixel_buffer &srcBuffer,
						  dng_pixel_buffer &dstBuffer);

	};

/*****************************************************************************/

dng_bilinear_interpolator::dng_bilinear_interpolator (const dng_mosaic_info &info,
													  int32 rowStep,
													  int32 colStep)
	{

	for (uint32 dstPlane = 0; dstPlane < info.fColorPlanes; dstPlane++)
		{

		fPattern [dstPlane] . Calculate (info,
										 dstPlane,
										 rowStep,
										 colStep);

		}

	}

/*****************************************************************************/

void dng_bilinear_interpolator::Interpolate (dng_pixel_buffer &srcBuffer,
						  					 dng_pixel_buffer &dstBuffer)
	{

	uint32 patCols = fPattern [0] . fPatCols;
	uint32 patRows = fPattern [0] . fPatRows;

	dng_point scale = fPattern [0] . fScale;

	uint32 sRowShift = scale.v - 1;
	uint32 sColShift = scale.h - 1;

	int32 dstCol = dstBuffer.fArea.l;

	int32 srcCol = dstCol >> sColShift;

	uint32 patPhase = dstCol % patCols;

	for (int32 dstRow = dstBuffer.fArea.t;
		 dstRow < dstBuffer.fArea.b;
		 dstRow++)
		{

		int32 srcRow = dstRow >> sRowShift;

		uint32 patRow = dstRow % patRows;

		for (uint32 dstPlane = 0;
			 dstPlane < dstBuffer.fPlanes;
			 dstPlane++)
			{

			const void *sPtr = srcBuffer.ConstPixel (srcRow,
													  srcCol,
													  srcBuffer.fPlane);

			void *dPtr = dstBuffer.DirtyPixel (dstRow,
										  	   dstCol,
										  	   dstPlane);

			if (dstBuffer.fPixelType == ttShort)
				{

				DoBilinearRow16 ((const uint16 *) sPtr,
					   			 (uint16 *) dPtr,
					   			 dstBuffer.fArea.W (),
					   			 patPhase,
					   			 patCols,
					   			 fPattern [dstPlane].fCounts    [patRow],
					   			 fPattern [dstPlane].fOffsets   [patRow],
					   			 fPattern [dstPlane].fWeights16 [patRow],
					   			 sColShift);

				}

			else
				{

				DoBilinearRow32 ((const real32 *) sPtr,
					   			 (real32 *) dPtr,
					   			 dstBuffer.fArea.W (),
					   			 patPhase,
					   			 patCols,
					   			 fPattern [dstPlane].fCounts    [patRow],
					   			 fPattern [dstPlane].fOffsets   [patRow],
					   			 fPattern [dstPlane].fWeights32 [patRow],
					   			 sColShift);

				}

			}

		}

	}

/*****************************************************************************/

class dng_fast_interpolator: public dng_filter_task
	{

	protected:

		const dng_mosaic_info &fInfo;

		dng_point fDownScale;

		uint32 fFilterColor [kMaxCFAPattern] [kMaxCFAPattern];

	public:

		dng_fast_interpolator (const dng_mosaic_info &info,
							   const dng_image &srcImage,
							   dng_image &dstImage,
							   const dng_point &downScale,
							   uint32 srcPlane);

		virtual dng_rect SrcArea (const dng_rect &dstArea);

		virtual void ProcessArea (uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer);

	};

/*****************************************************************************/

dng_fast_interpolator::dng_fast_interpolator (const dng_mosaic_info &info,
											  const dng_image &srcImage,
											  dng_image &dstImage,
											  const dng_point &downScale,
											  uint32 srcPlane)

	:	dng_filter_task (srcImage,
						 dstImage)

	,	fInfo       (info     )
	,	fDownScale  (downScale)

	{

	fSrcPlane  = srcPlane;
	fSrcPlanes = 1;

	fSrcPixelType = ttShort;
	fDstPixelType = ttShort;

	fSrcRepeat = fInfo.fCFAPatternSize;

	fUnitCell = fInfo.fCFAPatternSize;

	fMaxTileSize = dng_point (256 / fDownScale.v,
					  		  256 / fDownScale.h);

	fMaxTileSize.h = Max_int32 (fMaxTileSize.h, fUnitCell.h);
	fMaxTileSize.v = Max_int32 (fMaxTileSize.v, fUnitCell.v);

	// Find color map.

		{

		for (int32 r = 0; r < fInfo.fCFAPatternSize.v; r++)
			{

			for (int32 c = 0; c < fInfo.fCFAPatternSize.h; c++)
				{

				uint8 key = fInfo.fCFAPattern [r] [c];

				for (uint32 index = 0; index < fInfo.fColorPlanes; index++)
					{

					if (key == fInfo.fCFAPlaneColor [index])
						{

						fFilterColor [r] [c] = index;

						break;

						}

					}

				}

			}

		}

	}

/*****************************************************************************/

dng_rect dng_fast_interpolator::SrcArea (const dng_rect &dstArea)
	{

	return dng_rect (dstArea.t * fDownScale.v,
					 dstArea.l * fDownScale.h,
					 dstArea.b * fDownScale.v,
					 dstArea.r * fDownScale.h);

	}

/*****************************************************************************/

void dng_fast_interpolator::ProcessArea (uint32 /* threadIndex */,
								  	  	 dng_pixel_buffer &srcBuffer,
								      	 dng_pixel_buffer &dstBuffer)
	{

	dng_rect srcArea = srcBuffer.fArea;
	dng_rect dstArea = dstBuffer.fArea;

	// Downsample buffer.

	int32 srcRow = srcArea.t;

	uint32 srcRowPhase1 = 0;
	uint32 srcRowPhase2 = 0;

	uint32 patRows = fInfo.fCFAPatternSize.v;
	uint32 patCols = fInfo.fCFAPatternSize.h;

	uint32 cellRows = fDownScale.v;
	uint32 cellCols = fDownScale.h;

	uint32 plane;
	uint32 planes = fInfo.fColorPlanes;

	int32 dstPlaneStep = dstBuffer.fPlaneStep;

	uint32 total [kMaxColorPlanes];
	uint32 count [kMaxColorPlanes];

	for (plane = 0; plane < planes; plane++)
		{
		total [plane] = 0;
		count [plane] = 0;
		}

	for (int32 dstRow = dstArea.t; dstRow < dstArea.b; dstRow++)
		{

		const uint16 *sPtr = srcBuffer.ConstPixel_uint16 (srcRow,
														  srcArea.l,
														  fSrcPlane);

		uint16 *dPtr = dstBuffer.DirtyPixel_uint16 (dstRow,
													dstArea.l,
													0);

		uint32 srcColPhase1 = 0;
		uint32 srcColPhase2 = 0;

		for (int32 dstCol = dstArea.l; dstCol < dstArea.r; dstCol++)
			{

			const uint16 *ssPtr = sPtr;

			srcRowPhase2 = srcRowPhase1;

			for (uint32 cellRow = 0; cellRow < cellRows; cellRow++)
				{

				const uint32 *filterRow = fFilterColor [srcRowPhase2];

				if (++srcRowPhase2 == patRows)
					{
					srcRowPhase2 = 0;
					}

				srcColPhase2 = srcColPhase1;

				for (uint32 cellCol = 0; cellCol < cellCols; cellCol++)
					{

					uint32 color = filterRow [srcColPhase2];

					if (++srcColPhase2 == patCols)
						{
						srcColPhase2 = 0;
						}

					total [color] += (uint32) ssPtr [cellCol];
					count [color] ++;

					}

				ssPtr += srcBuffer.fRowStep;

				}

			for (plane = 0; plane < planes; plane++)
				{

				uint32 t = total [plane];
				uint32 c = count [plane];

				dPtr [plane * dstPlaneStep] = (uint16) ((t + (c >> 1)) / c);

				total [plane] = 0;
				count [plane] = 0;

				}

			srcColPhase1 = srcColPhase2;

			sPtr += cellCols;

			dPtr ++;

			}

		srcRowPhase1 = srcRowPhase2;

		srcRow += cellRows;

		}

	}

/*****************************************************************************/

dng_mosaic_info::dng_mosaic_info ()

	:	fCFAPatternSize  ()
	,	fColorPlanes     (0)
	,	fCFALayout		 (1)
	,	fBayerGreenSplit (0)
	,	fSrcSize		 ()
	,	fCroppedSize     ()
	,	fAspectRatio     (1.0)

	{

	}

/*****************************************************************************/

dng_mosaic_info::~dng_mosaic_info ()
	{

	}

/*****************************************************************************/

void dng_mosaic_info::Parse (dng_host & /* host */,
							 dng_stream & /* stream */,
							 dng_info &info)
	{

	// Find main image IFD.

	dng_ifd &rawIFD = *info.fIFD [info.fMainIndex].Get ();

	// This information only applies to CFA images.

	if (rawIFD.fPhotometricInterpretation != piCFA)
		{
		return;
		}

	// Copy CFA pattern.

	fCFAPatternSize.v = rawIFD.fCFARepeatPatternRows;
	fCFAPatternSize.h = rawIFD.fCFARepeatPatternCols;

	for (int32 j = 0; j < fCFAPatternSize.v; j++)
		{
		for (int32 k = 0; k < fCFAPatternSize.h; k++)
			{
			fCFAPattern [j] [k] = rawIFD.fCFAPattern [j] [k];
			}
		}

	// Copy CFA plane information.

	fColorPlanes = info.fShared->fCameraProfile.fColorPlanes;

	for (uint32 n = 0; n < fColorPlanes; n++)
		{
		fCFAPlaneColor [n] = rawIFD.fCFAPlaneColor [n];
		}

	// Copy CFA layout information.

	fCFALayout = rawIFD.fCFALayout;

	// Green split value for Bayer patterns.

	fBayerGreenSplit = rawIFD.fBayerGreenSplit;

	}

/*****************************************************************************/

void dng_mosaic_info::PostParse (dng_host & /* host */,
								 dng_negative &negative)
	{

	// Keep track of source image size.

	fSrcSize = negative.Stage2Image ()->Size ();

	// Default cropped size.

	fCroppedSize.v = Round_int32 (negative.DefaultCropSizeV ().As_real64 ());
	fCroppedSize.h = Round_int32 (negative.DefaultCropSizeH ().As_real64 ());

	// Pixel aspect ratio.

	fAspectRatio = negative.DefaultScaleH ().As_real64 () /
				   negative.DefaultScaleV ().As_real64 ();

	}

/*****************************************************************************/

bool dng_mosaic_info::SetFourColorBayer ()
	{

	if (fCFAPatternSize != dng_point (2, 2))
		{
		return false;
		}

	if (fColorPlanes != 3)
		{
		return false;
		}

	uint8 color0 = fCFAPlaneColor [0];
	uint8 color1 = fCFAPlaneColor [1];
	uint8 color2 = fCFAPlaneColor [2];

	// Look for color 1 repeated twice in a diagonal.

	if ((fCFAPattern [0] [0] == color1 && fCFAPattern [1] [1] == color1) ||
		(fCFAPattern [0] [1] == color1 && fCFAPattern [1] [0] == color1))
		{

		// OK, this looks like a Bayer pattern.

		// Find unused color code.

		uint8 color3 = 0;

		while (color3 == color0 ||
			   color3 == color1 ||
			   color3 == color2)
			{
			color3++;
			}

		// Switch the four color mosaic.

		fColorPlanes = 4;

		fCFAPlaneColor [3] = color3;

		// Replace the "green" in the "blue" rows with the new color.

		if (fCFAPattern [0] [0] == color0)
			{
			fCFAPattern [1] [0] = color3;
			}

		else if (fCFAPattern [0] [1] == color0)
			{
			fCFAPattern [1] [1] = color3;
			}

		else if (fCFAPattern [1] [0] == color0)
			{
			fCFAPattern [0] [0] = color3;
			}

		else
			{
			fCFAPattern [0] [1] = color3;
			}

		return true;

		}

	return false;

	}

/*****************************************************************************/

dng_point dng_mosaic_info::FullScale () const
	{

	switch (fCFALayout)
		{

		// Staggered layouts with offset columns double the row count
		// during interpolation.

		case 2:
		case 3:
			return dng_point (2, 1);

		// Staggered layouts with offset rows double the column count
		// during interpolation.

		case 4:
		case 5:
			return dng_point (1, 2);

		// Otherwise there is no size change during interpolation.

		default:
			break;

		}

	return dng_point (1, 1);

	}

/*****************************************************************************/

bool dng_mosaic_info::IsSafeDownScale (const dng_point &downScale) const
	{

	if (downScale.v >= fCFAPatternSize.v &&
		downScale.h >= fCFAPatternSize.h)
		{

		return true;

		}

	dng_point test;

	test.v = Min_int32 (downScale.v, fCFAPatternSize.v);
	test.h = Min_int32 (downScale.h, fCFAPatternSize.h);

	for (int32 phaseV = 0; phaseV <= fCFAPatternSize.v - test.v; phaseV++)
		{

		for (int32 phaseH = 0; phaseH <= fCFAPatternSize.h - test.h; phaseH++)
			{

			uint32 plane;

			bool contains [kMaxColorPlanes];

			for (plane = 0; plane < fColorPlanes; plane++)
				{

				contains [plane] = false;

				}

			for (int32 srcRow = 0; srcRow < test.v; srcRow++)
				{

				for (int32 srcCol = 0; srcCol < test.h; srcCol++)
					{

					uint8 srcKey = fCFAPattern [srcRow + phaseV]
											   [srcCol + phaseH];

					for (plane = 0; plane < fColorPlanes; plane++)
						{

						if (srcKey == fCFAPlaneColor [plane])
							{

							contains [plane] = true;

							}

						}


					}

				}

			for (plane = 0; plane < fColorPlanes; plane++)
				{

				if (!contains [plane])
					{

					return false;

					}

				}

			}

		}

	return true;

	}

/*****************************************************************************/

uint32 dng_mosaic_info::SizeForDownScale (const dng_point &downScale) const
	{

	uint32 sizeV = Max_uint32 (1, (fCroppedSize.v + (downScale.v >> 1)) / downScale.v);
	uint32 sizeH = Max_uint32 (1, (fCroppedSize.h + (downScale.h >> 1)) / downScale.h);

	return Max_int32 (sizeV, sizeH);

	}

/*****************************************************************************/

bool dng_mosaic_info::ValidSizeDownScale (const dng_point &downScale,
										  uint32 minSize) const
	{

	const int32 kMaxDownScale = 64;

	if (downScale.h > kMaxDownScale ||
		downScale.v > kMaxDownScale)
		{

		return false;

		}

	return SizeForDownScale (downScale) >= minSize;

	}

/*****************************************************************************/

dng_point dng_mosaic_info::DownScale (uint32 minSize,
									  uint32 prefSize,
									  real64 cropFactor) const
	{

	dng_point bestScale (1, 1);

	if (prefSize && IsColorFilterArray ())
		{

		// Adjust sizes for crop factor.

		minSize  = Round_uint32 (minSize  / cropFactor);
		prefSize = Round_uint32 (prefSize / cropFactor);

		prefSize = Max_uint32 (prefSize, minSize);

		// Start by assuming we need the full size image.

		int32 bestSize = SizeForDownScale (bestScale);

		// Find size of nearly square cell.

		dng_point squareCell (1, 1);

		if (fAspectRatio < 1.0 / 1.8)
			{

			squareCell.h = Min_int32 (4, Round_int32 (1.0 / fAspectRatio));

			}

		if (fAspectRatio > 1.8)
			{

			squareCell.v = Min_int32 (4, Round_int32 (fAspectRatio));

			}

		// Find minimum safe cell size.

		dng_point testScale = squareCell;

		while (!IsSafeDownScale (testScale))
			{

			testScale.v += squareCell.v;
			testScale.h += squareCell.h;

			}

		// See if this scale is usable.

		if (!ValidSizeDownScale (testScale, minSize))
			{

			// We cannot downsample at all...

			return bestScale;

			}

		// See if this is closer to the preferred size.

		int32 testSize = SizeForDownScale (testScale);

		if (Abs_int32 (testSize - (int32) prefSize) <=
		    Abs_int32 (bestSize - (int32) prefSize))
			{
			bestScale = testScale;
			bestSize  = testSize;
			}

		else
			{
			return bestScale;
			}

		// Now keep adding square cells as long as possible.

		while (true)
			{

			testScale.v += squareCell.v;
			testScale.h += squareCell.h;

			if (IsSafeDownScale (testScale))
				{

				if (!ValidSizeDownScale (testScale, minSize))
					{
					return bestScale;
					}

				// See if this is closer to the preferred size.

				testSize = SizeForDownScale (testScale);

				if (Abs_int32 (testSize - (int32) prefSize) <=
					Abs_int32 (bestSize - (int32) prefSize))
					{
					bestScale = testScale;
					bestSize  = testSize;
					}

				else
					{
					return bestScale;
					}

				}

			}

		}

	return bestScale;

	}

/*****************************************************************************/

dng_point dng_mosaic_info::DstSize (const dng_point &downScale) const
	{

	if (downScale == dng_point (1, 1))
		{

		dng_point scale = FullScale ();

		return dng_point (fSrcSize.v * scale.v,
						  fSrcSize.h * scale.h);

		}

	const int32 kMaxDownScale = 64;

	if (downScale.h > kMaxDownScale ||
		downScale.v > kMaxDownScale)
		{

		return dng_point (0, 0);

		}

	dng_point size;

	size.v = Max_int32 (1, (fSrcSize.v + (downScale.v >> 1)) / downScale.v);
	size.h = Max_int32 (1, (fSrcSize.h + (downScale.h >> 1)) / downScale.h);

	return size;

	}

/*****************************************************************************/

void dng_mosaic_info::InterpolateGeneric (dng_host &host,
										  dng_negative & /* negative */,
								   		  const dng_image &srcImage,
								   		  dng_image &dstImage,
								   		  uint32 srcPlane) const
	{

	// Find destination to source bit shifts.

	dng_point scale = FullScale ();

	uint32 srcShiftV = scale.v - 1;
	uint32 srcShiftH = scale.h - 1;

	// Find tile sizes.

	const uint32 kMaxDstTileRows = 128;
	const uint32 kMaxDstTileCols = 128;

	dng_point dstTileSize = dstImage.RepeatingTile ().Size ();

	dstTileSize.v = Min_int32 (dstTileSize.v, kMaxDstTileRows);
	dstTileSize.h = Min_int32 (dstTileSize.h, kMaxDstTileCols);

	dng_point srcTileSize = dstTileSize;

	srcTileSize.v >>= srcShiftV;
	srcTileSize.h >>= srcShiftH;

	srcTileSize.v += fCFAPatternSize.v * 2;
	srcTileSize.h += fCFAPatternSize.h * 2;

	// Allocate source buffer.

	dng_pixel_buffer srcBuffer;

	srcBuffer.fPlane = srcPlane;

	srcBuffer.fRowStep = srcTileSize.h;

	srcBuffer.fPixelType = srcImage.PixelType ();
	srcBuffer.fPixelSize = srcImage.PixelSize ();

	uint32 srcBufferSize = srcBuffer.fPixelSize *
						   srcBuffer.fRowStep *
						   srcTileSize.v;

	AutoPtr<dng_memory_block> srcData (host.Allocate (srcBufferSize));

	srcBuffer.fData = srcData->Buffer ();

	// Allocate destination buffer.

	dng_pixel_buffer dstBuffer;

	dstBuffer.fPlanes = fColorPlanes;

	dstBuffer.fRowStep   = dstTileSize.h * fColorPlanes;
	dstBuffer.fPlaneStep = dstTileSize.h;

	dstBuffer.fPixelType = dstImage.PixelType ();
	dstBuffer.fPixelSize = dstImage.PixelSize ();

	uint32 dstBufferSize = dstBuffer.fPixelSize *
						   dstBuffer.fRowStep *
						   dstTileSize.v;

	AutoPtr<dng_memory_block> dstData (host.Allocate (dstBufferSize));

	dstBuffer.fData = dstData->Buffer ();

	// Create interpolator.

	dng_bilinear_interpolator interpolator (*this,
											srcBuffer.fRowStep,
											srcBuffer.fColStep);

	// Iterate over destination tiles.

	dng_rect dstArea;

	dng_tile_iterator iter1 (dstImage, dstImage.Bounds ());

	while (iter1.GetOneTile (dstArea))
		{

		// Break into buffer sized tiles.

		dng_rect dstTile;

		dng_tile_iterator iter2 (dstTileSize, dstArea);

		while (iter2.GetOneTile (dstTile))
			{

			host.SniffForAbort ();

			// Setup buffers for this tile.

			dng_rect srcTile (dstTile);

			srcTile.t >>= srcShiftV;
			srcTile.b >>= srcShiftV;

			srcTile.l >>= srcShiftH;
			srcTile.r >>= srcShiftH;

			srcTile.t -= fCFAPatternSize.v;
			srcTile.b += fCFAPatternSize.v;

			srcTile.l -= fCFAPatternSize.h;
			srcTile.r += fCFAPatternSize.h;

			srcBuffer.fArea = srcTile;
			dstBuffer.fArea = dstTile;

			// Get source data.

			srcImage.Get (srcBuffer,
						  dng_image::edge_repeat,
						  fCFAPatternSize.v,
						  fCFAPatternSize.h);

			// Process data.

			interpolator.Interpolate (srcBuffer,
									  dstBuffer);

			// Save results.

			dstImage.Put (dstBuffer);

			}

		}

	}

/*****************************************************************************/

void dng_mosaic_info::InterpolateFast (dng_host &host,
									   dng_negative & /* negative */,
							  	   	   const dng_image &srcImage,
								   	   dng_image &dstImage,
								       const dng_point &downScale,
								       uint32 srcPlane) const
	{

	// Create fast interpolator task.

	dng_fast_interpolator interpolator (*this,
										srcImage,
										dstImage,
										downScale,
										srcPlane);

	// Find area to process.

	dng_rect bounds = dstImage.Bounds ();

	// Do the interpolation.

	host.PerformAreaTask (interpolator,
						  bounds);

	}

/*****************************************************************************/

void dng_mosaic_info::Interpolate (dng_host &host,
								   dng_negative &negative,
							  	   const dng_image &srcImage,
								   dng_image &dstImage,
								   const dng_point &downScale,
								   uint32 srcPlane) const
	{

	if (downScale == dng_point (1, 1))
		{

		InterpolateGeneric (host,
							negative,
							srcImage,
							dstImage,
							srcPlane);

		}

	else
		{

		InterpolateFast (host,
						 negative,
						 srcImage,
						 dstImage,
						 downScale,
						 srcPlane);

		}

	}

/*****************************************************************************/
