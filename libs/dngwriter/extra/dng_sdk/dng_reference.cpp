/*****************************************************************************/
// Copyright 2006-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_reference.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_reference.h"

#include "dng_1d_table.h"
#include "dng_hue_sat_map.h"
#include "dng_matrix.h"
#include "dng_resample.h"
#include "dng_utils.h"

/*****************************************************************************/

// This module contains routines that should be as fast as possible, even
// at the expense of slight code size increases.

#include "dng_fast_module.h"

/*****************************************************************************/

void RefZeroBytes (void *dPtr,
				   uint32 count)
	{

	memset (dPtr, 0, count);

	}

/*****************************************************************************/

void RefCopyBytes (const void *sPtr,
				   void *dPtr,
				   uint32 count)
	{

	memcpy (dPtr, sPtr, count);

	}

/*****************************************************************************/

void RefSwapBytes16 (uint16 *dPtr,
				     uint32 count)
	{

	for (uint32 j = 0; j < count; j++)
		{

		dPtr [j] = SwapBytes16 (dPtr [j]);

		}

	}

/*****************************************************************************/

void RefSwapBytes32 (uint32 *dPtr,
				     uint32 count)
	{

	for (uint32 j = 0; j < count; j++)
		{

		dPtr [j] = SwapBytes32 (dPtr [j]);

		}

	}

/*****************************************************************************/

void RefSetArea8 (uint8 *dPtr,
				  uint8 value,
				  uint32 rows,
				  uint32 cols,
				  uint32 planes,
				  int32 rowStep,
				  int32 colStep,
				  int32 planeStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		uint8 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			uint8 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = value;

				dPtr2 += planeStep;

				}

			dPtr1 += colStep;

			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefSetArea16 (uint16 *dPtr,
				   uint16 value,
				   uint32 rows,
				   uint32 cols,
				   uint32 planes,
				   int32 rowStep,
				   int32 colStep,
				   int32 planeStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		uint16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = value;

				dPtr2 += planeStep;

				}

			dPtr1 += colStep;

			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefSetArea32 (uint32 *dPtr,
				   uint32 value,
				   uint32 rows,
			       uint32 cols,
				   uint32 planes,
				   int32 rowStep,
				   int32 colStep,
				   int32 planeStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		uint32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			uint32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = value;

				dPtr2 += planeStep;

				}

			dPtr1 += colStep;

			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea8 (const uint8 *sPtr,
				   uint8 *dPtr,
				   uint32 rows,
				   uint32 cols,
				   uint32 planes,
				   int32 sRowStep,
				   int32 sColStep,
				   int32 sPlaneStep,
				   int32 dRowStep,
				   int32 dColStep,
				   int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8 *sPtr1 = sPtr;
		      uint8 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8 *sPtr2 = sPtr1;
			      uint8 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea16 (const uint16 *sPtr,
					uint16 *dPtr,
					uint32 rows,
					uint32 cols,
					uint32 planes,
					int32 sRowStep,
					int32 sColStep,
					int32 sPlaneStep,
					int32 dRowStep,
					int32 dColStep,
					int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint16 *sPtr1 = sPtr;
		      uint16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint16 *sPtr2 = sPtr1;
			      uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea32 (const uint32 *sPtr,
					uint32 *dPtr,
					uint32 rows,
					uint32 cols,
					uint32 planes,
					int32 sRowStep,
					int32 sColStep,
					int32 sPlaneStep,
					int32 dRowStep,
					int32 dColStep,
					int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint32 *sPtr1 = sPtr;
		      uint32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint32 *sPtr2 = sPtr1;
			      uint32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea8_16 (const uint8 *sPtr,
					  uint16 *dPtr,
					  uint32 rows,
					  uint32 cols,
					  uint32 planes,
					  int32 sRowStep,
					  int32 sColStep,
					  int32 sPlaneStep,
					  int32 dRowStep,
					  int32 dColStep,
					  int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8  *sPtr1 = sPtr;
		      uint16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8  *sPtr2 = sPtr1;
			      uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea8_S16 (const uint8 *sPtr,
					   int16 *dPtr,
					   uint32 rows,
					   uint32 cols,
					   uint32 planes,
					   int32 sRowStep,
					   int32 sColStep,
					   int32 sPlaneStep,
					   int32 dRowStep,
					   int32 dColStep,
					   int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8 *sPtr1 = sPtr;
		      int16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8 *sPtr2 = sPtr1;
			      int16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				int16 x = *sPtr;

				*dPtr2 = x ^ 0x8000;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea8_32 (const uint8 *sPtr,
					  uint32 *dPtr,
					  uint32 rows,
					  uint32 cols,
					  uint32 planes,
					  int32 sRowStep,
					  int32 sColStep,
					  int32 sPlaneStep,
					  int32 dRowStep,
					  int32 dColStep,
					  int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8  *sPtr1 = sPtr;
		      uint32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8  *sPtr2 = sPtr1;
			      uint32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea16_S16 (const uint16 *sPtr,
					    int16 *dPtr,
					    uint32 rows,
					    uint32 cols,
					    uint32 planes,
					    int32 sRowStep,
					    int32 sColStep,
					    int32 sPlaneStep,
					    int32 dRowStep,
					    int32 dColStep,
					    int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint16 *sPtr1 = sPtr;
		      int16  *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint16 *sPtr2 = sPtr1;
			      int16  *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2 ^ 0x8000;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea16_32 (const uint16 *sPtr,
					   uint32 *dPtr,
					   uint32 rows,
					   uint32 cols,
					   uint32 planes,
					   int32 sRowStep,
					   int32 sColStep,
					   int32 sPlaneStep,
					   int32 dRowStep,
					   int32 dColStep,
					   int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint16 *sPtr1 = sPtr;
		      uint32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint16 *sPtr2 = sPtr1;
			      uint32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea8_R32 (const uint8 *sPtr,
					   real32 *dPtr,
					   uint32 rows,
					   uint32 cols,
					   uint32 planes,
					   int32 sRowStep,
					   int32 sColStep,
					   int32 sPlaneStep,
					   int32 dRowStep,
					   int32 dColStep,
					   int32 dPlaneStep,
					   uint32 pixelRange)
	{

	real32 scale = 1.0f / (real32) pixelRange;

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8  *sPtr1 = sPtr;
		      real32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8  *sPtr2 = sPtr1;
			      real32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = scale * (real32) *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyArea16_R32 (const uint16 *sPtr,
					    real32 *dPtr,
					    uint32 rows,
					    uint32 cols,
					    uint32 planes,
					    int32 sRowStep,
					    int32 sColStep,
					    int32 sPlaneStep,
					    int32 dRowStep,
					    int32 dColStep,
					    int32 dPlaneStep,
						uint32 pixelRange)
	{

	real32 scale = 1.0f / (real32) pixelRange;

	for (uint32 row = 0; row < rows; row++)
		{

		const uint16 *sPtr1 = sPtr;
		      real32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint16 *sPtr2 = sPtr1;
			      real32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = scale * (real32) *sPtr2;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyAreaS16_R32 (const int16 *sPtr,
					     real32 *dPtr,
					     uint32 rows,
					     uint32 cols,
					     uint32 planes,
					     int32 sRowStep,
					     int32 sColStep,
					     int32 sPlaneStep,
					     int32 dRowStep,
					     int32 dColStep,
					     int32 dPlaneStep,
						 uint32 pixelRange)
	{

	real32 scale = 1.0f / (real32) pixelRange;

	for (uint32 row = 0; row < rows; row++)
		{

		const int16  *sPtr1 = sPtr;
		      real32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const int16  *sPtr2 = sPtr1;
			      real32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				int32 x = (*sPtr ^ 0x8000);

				*dPtr2 = scale * (real32) x;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyAreaR32_8 (const real32 *sPtr,
					   uint8 *dPtr,
					   uint32 rows,
					   uint32 cols,
					   uint32 planes,
					   int32 sRowStep,
					   int32 sColStep,
					   int32 sPlaneStep,
					   int32 dRowStep,
					   int32 dColStep,
					   int32 dPlaneStep,
					   uint32 pixelRange)
	{

	real32 scale = (real32) pixelRange;

	for (uint32 row = 0; row < rows; row++)
		{

		const real32 *sPtr1 = sPtr;
		      uint8  *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const real32 *sPtr2 = sPtr1;
			      uint8  *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = (uint8) (*sPtr2 * scale + 0.5f);

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyAreaR32_16 (const real32 *sPtr,
					    uint16 *dPtr,
					    uint32 rows,
					    uint32 cols,
					    uint32 planes,
					    int32 sRowStep,
					    int32 sColStep,
					    int32 sPlaneStep,
					    int32 dRowStep,
					    int32 dColStep,
					    int32 dPlaneStep,
						uint32 pixelRange)
	{

	real32 scale = (real32) pixelRange;

	for (uint32 row = 0; row < rows; row++)
		{

		const real32 *sPtr1 = sPtr;
		      uint16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const real32 *sPtr2 = sPtr1;
			      uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = (uint16) (*sPtr2 * scale + 0.5f);

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefCopyAreaR32_S16 (const real32 *sPtr,
					     int16 *dPtr,
					     uint32 rows,
					     uint32 cols,
					     uint32 planes,
					     int32 sRowStep,
					     int32 sColStep,
					     int32 sPlaneStep,
					     int32 dRowStep,
					     int32 dColStep,
					     int32 dPlaneStep,
						 uint32 pixelRange)
	{

	real32 scale = (real32) pixelRange;

	for (uint32 row = 0; row < rows; row++)
		{

		const real32 *sPtr1 = sPtr;
			  int16  *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const real32 *sPtr2 = sPtr1;
			      int16  *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				int32 x = (int32) (*sPtr2 * scale + 0.5f);

				*dPtr2 = (int16) (x ^ 0x8000);

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	}

/*****************************************************************************/

void RefRepeatArea8 (const uint8 *sPtr,
					 uint8 *dPtr,
					 uint32 rows,
					 uint32 cols,
					 uint32 planes,
					 int32 rowStep,
					 int32 colStep,
					 int32 planeStep,
					 uint32 repeatV,
					 uint32 repeatH,
					 uint32 phaseV,
					 uint32 phaseH)
	{

	const uint8 *sPtr0 = sPtr + phaseV * rowStep +
								phaseH * colStep;

	int32 backStepV = (repeatV - 1) * rowStep;
	int32 backStepH = (repeatH - 1) * colStep;

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8 *sPtr1 = sPtr0;
		      uint8 *dPtr1 = dPtr;

		uint32 colPhase = phaseH;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8 *sPtr2 = sPtr1;
			      uint8 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += planeStep;
				dPtr2 += planeStep;

				}

			if (++colPhase == repeatH)
				{
				colPhase = 0;
				sPtr1 -= backStepH;
				}
			else
				{
				sPtr1 += colStep;
				}

			dPtr1 += colStep;

			}

		if (++phaseV == repeatV)
			{
			phaseV = 0;
			sPtr0 -= backStepV;
			}
		else
			{
			sPtr0 += rowStep;
			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefRepeatArea16 (const uint16 *sPtr,
					  uint16 *dPtr,
					  uint32 rows,
					  uint32 cols,
					  uint32 planes,
					  int32 rowStep,
					  int32 colStep,
					  int32 planeStep,
					  uint32 repeatV,
					  uint32 repeatH,
					  uint32 phaseV,
					  uint32 phaseH)
	{

	const uint16 *sPtr0 = sPtr + phaseV * rowStep +
								 phaseH * colStep;

	int32 backStepV = (repeatV - 1) * rowStep;
	int32 backStepH = (repeatH - 1) * colStep;

	for (uint32 row = 0; row < rows; row++)
		{

		const uint16 *sPtr1 = sPtr0;
		      uint16 *dPtr1 = dPtr;

		uint32 colPhase = phaseH;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint16 *sPtr2 = sPtr1;
			      uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += planeStep;
				dPtr2 += planeStep;

				}

			if (++colPhase == repeatH)
				{
				colPhase = 0;
				sPtr1 -= backStepH;
				}
			else
				{
				sPtr1 += colStep;
				}

			dPtr1 += colStep;

			}

		if (++phaseV == repeatV)
			{
			phaseV = 0;
			sPtr0 -= backStepV;
			}
		else
			{
			sPtr0 += rowStep;
			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefRepeatArea32 (const uint32 *sPtr,
					  uint32 *dPtr,
					  uint32 rows,
					  uint32 cols,
					  uint32 planes,
					  int32 rowStep,
					  int32 colStep,
					  int32 planeStep,
					  uint32 repeatV,
					  uint32 repeatH,
					  uint32 phaseV,
					  uint32 phaseH)
	{

	const uint32 *sPtr0 = sPtr + phaseV * rowStep +
								 phaseH * colStep;

	int32 backStepV = (repeatV - 1) * rowStep;
	int32 backStepH = (repeatH - 1) * colStep;

	for (uint32 row = 0; row < rows; row++)
		{

		const uint32 *sPtr1 = sPtr0;
		      uint32 *dPtr1 = dPtr;

		uint32 colPhase = phaseH;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint32 *sPtr2 = sPtr1;
			      uint32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 = *sPtr2;

				sPtr2 += planeStep;
				dPtr2 += planeStep;

				}

			if (++colPhase == repeatH)
				{
				colPhase = 0;
				sPtr1 -= backStepH;
				}
			else
				{
				sPtr1 += colStep;
				}

			dPtr1 += colStep;

			}

		if (++phaseV == repeatV)
			{
			phaseV = 0;
			sPtr0 -= backStepV;
			}
		else
			{
			sPtr0 += rowStep;
			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefShiftRight16 (uint16 *dPtr,
					  uint32 rows,
					  uint32 cols,
					  uint32 planes,
					  int32 rowStep,
					  int32 colStep,
					  int32 planeStep,
					  uint32 shift)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		uint16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				*dPtr2 >>= shift;

				dPtr2 += planeStep;

				}

			dPtr1 += colStep;

			}

		dPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefBilinearRow16 (const uint16 *sPtr,
					   uint16 *dPtr,
					   uint32 cols,
					   uint32 patPhase,
					   uint32 patCount,
					   const uint32 * kernCounts,
					   const int32  * const * kernOffsets,
					   const uint16 * const * kernWeights,
					   uint32 sShift)
	{

	for (uint32 j = 0; j < cols; j++)
		{

		const uint16 *p = sPtr + (j >> sShift);

		uint32 count = kernCounts [patPhase];

		const int32  *offsets = kernOffsets [patPhase];
		const uint16 *weights = kernWeights [patPhase];

		if (++patPhase == patCount)
			{
			patPhase = 0;
			}

		uint32 total = 128;

		for (uint32 k = 0; k < count; k++)
			{

			int32  offset = offsets [k];
			uint32 weight = weights [k];

			uint32 pixel = p [offset];

			total += pixel * weight;

			}

		dPtr [j] = (uint16) (total >> 8);

		}

	}

/*****************************************************************************/

void RefBilinearRow32 (const real32 *sPtr,
					   real32 *dPtr,
					   uint32 cols,
					   uint32 patPhase,
					   uint32 patCount,
					   const uint32 * kernCounts,
					   const int32  * const * kernOffsets,
					   const real32 * const * kernWeights,
					   uint32 sShift)
	{

	for (uint32 j = 0; j < cols; j++)
		{

		const real32 *p = sPtr + (j >> sShift);

		uint32 count = kernCounts [patPhase];

		const int32  *offsets = kernOffsets [patPhase];
		const real32 *weights = kernWeights [patPhase];

		if (++patPhase == patCount)
			{
			patPhase = 0;
			}

		real32 total = 0.0f;

		for (uint32 k = 0; k < count; k++)
			{

			int32  offset = offsets [k];
			real32 weight = weights [k];

			real32 pixel = p [offset];

			total += pixel * weight;

			}

		dPtr [j] = total;

		}

	}

/*****************************************************************************/

void RefBaselineABCtoRGB (const real32 *sPtrA,
						  const real32 *sPtrB,
						  const real32 *sPtrC,
						  real32 *dPtrR,
						  real32 *dPtrG,
						  real32 *dPtrB,
						  uint32 count,
						  const dng_vector &cameraWhite,
						  const dng_matrix &cameraToRGB)
	{

	real32 clipA = (real32) cameraWhite [0];
	real32 clipB = (real32) cameraWhite [1];
	real32 clipC = (real32) cameraWhite [2];

	real32 m00 = (real32) cameraToRGB [0] [0];
	real32 m01 = (real32) cameraToRGB [0] [1];
	real32 m02 = (real32) cameraToRGB [0] [2];

	real32 m10 = (real32) cameraToRGB [1] [0];
	real32 m11 = (real32) cameraToRGB [1] [1];
	real32 m12 = (real32) cameraToRGB [1] [2];

	real32 m20 = (real32) cameraToRGB [2] [0];
	real32 m21 = (real32) cameraToRGB [2] [1];
	real32 m22 = (real32) cameraToRGB [2] [2];

	for (uint32 col = 0; col < count; col++)
		{

		real32 A = sPtrA [col];
		real32 B = sPtrB [col];
		real32 C = sPtrC [col];

		A = Min_real32 (A, clipA);
		B = Min_real32 (B, clipB);
		C = Min_real32 (C, clipC);

		real32 r = m00 * A + m01 * B + m02 * C;
		real32 g = m10 * A + m11 * B + m12 * C;
		real32 b = m20 * A + m21 * B + m22 * C;

		r = Pin_real32 (0.0f, r, 1.0f);
		g = Pin_real32 (0.0f, g, 1.0f);
		b = Pin_real32 (0.0f, b, 1.0f);

		dPtrR [col] = r;
		dPtrG [col] = g;
		dPtrB [col] = b;

		}

	}

/*****************************************************************************/

void RefBaselineABCDtoRGB (const real32 *sPtrA,
						   const real32 *sPtrB,
						   const real32 *sPtrC,
						   const real32 *sPtrD,
						   real32 *dPtrR,
						   real32 *dPtrG,
						   real32 *dPtrB,
						   uint32 count,
						   const dng_vector &cameraWhite,
						   const dng_matrix &cameraToRGB)
	{

	real32 clipA = (real32) cameraWhite [0];
	real32 clipB = (real32) cameraWhite [1];
	real32 clipC = (real32) cameraWhite [2];
	real32 clipD = (real32) cameraWhite [3];

	real32 m00 = (real32) cameraToRGB [0] [0];
	real32 m01 = (real32) cameraToRGB [0] [1];
	real32 m02 = (real32) cameraToRGB [0] [2];
	real32 m03 = (real32) cameraToRGB [0] [3];

	real32 m10 = (real32) cameraToRGB [1] [0];
	real32 m11 = (real32) cameraToRGB [1] [1];
	real32 m12 = (real32) cameraToRGB [1] [2];
	real32 m13 = (real32) cameraToRGB [1] [3];

	real32 m20 = (real32) cameraToRGB [2] [0];
	real32 m21 = (real32) cameraToRGB [2] [1];
	real32 m22 = (real32) cameraToRGB [2] [2];
	real32 m23 = (real32) cameraToRGB [2] [3];

	for (uint32 col = 0; col < count; col++)
		{

		real32 A = sPtrA [col];
		real32 B = sPtrB [col];
		real32 C = sPtrC [col];
		real32 D = sPtrD [col];

		A = Min_real32 (A, clipA);
		B = Min_real32 (B, clipB);
		C = Min_real32 (C, clipC);
		D = Min_real32 (D, clipD);

		real32 r = m00 * A + m01 * B + m02 * C + m03 * D;
		real32 g = m10 * A + m11 * B + m12 * C + m13 * D;
		real32 b = m20 * A + m21 * B + m22 * C + m23 * D;

		r = Pin_real32 (0.0f, r, 1.0f);
		g = Pin_real32 (0.0f, g, 1.0f);
		b = Pin_real32 (0.0f, b, 1.0f);

		dPtrR [col] = r;
		dPtrG [col] = g;
		dPtrB [col] = b;

		}

	}

/*****************************************************************************/

void RefBaselineHueSatMap (const real32 *sPtrR,
						   const real32 *sPtrG,
						   const real32 *sPtrB,
						   real32 *dPtrR,
						   real32 *dPtrG,
						   real32 *dPtrB,
						   uint32 count,
						   const dng_hue_sat_map &lut)
	{

	uint32 hueDivisions;
	uint32 satDivisions;
	uint32 valDivisions;

	lut.GetDivisions (hueDivisions,
					  satDivisions,
					  valDivisions);

	real32 hScale = (hueDivisions < 2) ? 0.0f : (hueDivisions * (1.0f / 6.0f));
	real32 sScale = (real32) (satDivisions - 1);
	real32 vScale = (real32) (valDivisions - 1);

	int32 maxHueIndex0 = hueDivisions - 1;
	int32 maxSatIndex0 = satDivisions - 2;
	int32 maxValIndex0 = valDivisions - 2;

	const dng_hue_sat_map::HSBModify *tableBase = lut.GetDeltas ();

	int32 hueStep = satDivisions;
	int32 valStep = hueDivisions * hueStep;

	#if 0	// Not required with "2.5D" table optimization.

	if (valDivisions < 2)
		{
		valStep      = 0;
		maxValIndex0 = 0;
		}

	#endif

	for (uint32 j = 0; j < count; j++)
		{

		real32 r = sPtrR [j];
		real32 g = sPtrG [j];
		real32 b = sPtrB [j];

		real32 h, s, v;

		DNG_RGBtoHSV (r, g, b, h, s, v);

		real32 hueShift;
		real32 satScale;
		real32 valScale;

		if (valDivisions < 2)		// Optimize most common case of "2.5D" table.
			{

			real32 hScaled = h * hScale;
			real32 sScaled = s * sScale;

			int32 hIndex0 = (int32) hScaled;
			int32 sIndex0 = (int32) sScaled;

			sIndex0 = Min_int32 (sIndex0, maxSatIndex0);

			int32 hIndex1 = hIndex0 + 1;

			if (hIndex0 >= maxHueIndex0)
				{
				hIndex0 = maxHueIndex0;
				hIndex1 = 0;
				}

			real32 hFract1 = hScaled - (real32) hIndex0;
			real32 sFract1 = sScaled - (real32) sIndex0;

			real32 hFract0 = 1.0f - hFract1;
			real32 sFract0 = 1.0f - sFract1;

			const dng_hue_sat_map::HSBModify *entry00 = tableBase + hIndex0 * hueStep +
																	sIndex0;

			const dng_hue_sat_map::HSBModify *entry01 = entry00 + (hIndex1 - hIndex0) * hueStep;

			real32 hueShift0 = hFract0 * entry00->fHueShift +
							   hFract1 * entry01->fHueShift;

			real32 satScale0 = hFract0 * entry00->fSatScale +
							   hFract1 * entry01->fSatScale;

			real32 valScale0 = hFract0 * entry00->fValScale +
							   hFract1 * entry01->fValScale;

			entry00++;
			entry01++;

			real32 hueShift1 = hFract0 * entry00->fHueShift +
							   hFract1 * entry01->fHueShift;

			real32 satScale1 = hFract0 * entry00->fSatScale +
							   hFract1 * entry01->fSatScale;

			real32 valScale1 = hFract0 * entry00->fValScale +
							   hFract1 * entry01->fValScale;

			hueShift = sFract0 * hueShift0 + sFract1 * hueShift1;
			satScale = sFract0 * satScale0 + sFract1 * satScale1;
			valScale = sFract0 * valScale0 + sFract1 * valScale1;

			}

		else
			{

			real32 hScaled = h * hScale;
			real32 sScaled = s * sScale;
			real32 vScaled = v * vScale;

			int32 hIndex0 = (int32) hScaled;
			int32 sIndex0 = (int32) sScaled;
			int32 vIndex0 = (int32) vScaled;

			sIndex0 = Min_int32 (sIndex0, maxSatIndex0);
			vIndex0 = Min_int32 (vIndex0, maxValIndex0);

			int32 hIndex1 = hIndex0 + 1;

			if (hIndex0 >= maxHueIndex0)
				{
				hIndex0 = maxHueIndex0;
				hIndex1 = 0;
				}

			real32 hFract1 = hScaled - (real32) hIndex0;
			real32 sFract1 = sScaled - (real32) sIndex0;
			real32 vFract1 = vScaled - (real32) vIndex0;

			real32 hFract0 = 1.0f - hFract1;
			real32 sFract0 = 1.0f - sFract1;
			real32 vFract0 = 1.0f - vFract1;

			const dng_hue_sat_map::HSBModify *entry00 = tableBase + vIndex0 * valStep +
																	hIndex0 * hueStep +
																	sIndex0;

			const dng_hue_sat_map::HSBModify *entry01 = entry00 + (hIndex1 - hIndex0) * hueStep;

			const dng_hue_sat_map::HSBModify *entry10 = entry00 + valStep;
			const dng_hue_sat_map::HSBModify *entry11 = entry01 + valStep;

			real32 hueShift0 = vFract0 * (hFract0 * entry00->fHueShift +
									      hFract1 * entry01->fHueShift) +
							   vFract1 * (hFract0 * entry10->fHueShift +
									      hFract1 * entry11->fHueShift);

			real32 satScale0 = vFract0 * (hFract0 * entry00->fSatScale +
									      hFract1 * entry01->fSatScale) +
							   vFract1 * (hFract0 * entry10->fSatScale +
									      hFract1 * entry11->fSatScale);

			real32 valScale0 = vFract0 * (hFract0 * entry00->fValScale +
									      hFract1 * entry01->fValScale) +
							   vFract1 * (hFract0 * entry10->fValScale +
									      hFract1 * entry11->fValScale);

			entry00++;
			entry01++;
			entry10++;
			entry11++;

			real32 hueShift1 = vFract0 * (hFract0 * entry00->fHueShift +
										  hFract1 * entry01->fHueShift) +
							   vFract1 * (hFract0 * entry10->fHueShift +
										  hFract1 * entry11->fHueShift);

			real32 satScale1 = vFract0 * (hFract0 * entry00->fSatScale +
										  hFract1 * entry01->fSatScale) +
							   vFract1 * (hFract0 * entry10->fSatScale +
										  hFract1 * entry11->fSatScale);

			real32 valScale1 = vFract0 * (hFract0 * entry00->fValScale +
										  hFract1 * entry01->fValScale) +
							   vFract1 * (hFract0 * entry10->fValScale +
										  hFract1 * entry11->fValScale);

			hueShift = sFract0 * hueShift0 + sFract1 * hueShift1;
			satScale = sFract0 * satScale0 + sFract1 * satScale1;
			valScale = sFract0 * valScale0 + sFract1 * valScale1;

			}

		hueShift *= (6.0f / 360.0f);	// Convert to internal hue range.

		h += hueShift;

		s = Min_real32 (s * satScale, 1.0f);
		v = Min_real32 (v * valScale, 1.0f);

		DNG_HSVtoRGB (h, s, v, r, g, b);

		dPtrR [j] = r;
		dPtrG [j] = g;
		dPtrB [j] = b;

		}

	}

/*****************************************************************************/

void RefBaselineRGBtoGray (const real32 *sPtrR,
						   const real32 *sPtrG,
						   const real32 *sPtrB,
						   real32 *dPtrG,
						   uint32 count,
						   const dng_matrix &matrix)
	{

	real32 m00 = (real32) matrix [0] [0];
	real32 m01 = (real32) matrix [0] [1];
	real32 m02 = (real32) matrix [0] [2];

	for (uint32 col = 0; col < count; col++)
		{

		real32 R = sPtrR [col];
		real32 G = sPtrG [col];
		real32 B = sPtrB [col];

		real32 g = m00 * R + m01 * G + m02 * B;

		g = Pin_real32 (0.0f, g, 1.0f);

		dPtrG [col] = g;

		}

	}

/*****************************************************************************/

void RefBaselineRGBtoRGB (const real32 *sPtrR,
						  const real32 *sPtrG,
						  const real32 *sPtrB,
						  real32 *dPtrR,
						  real32 *dPtrG,
						  real32 *dPtrB,
						  uint32 count,
						  const dng_matrix &matrix)
	{

	real32 m00 = (real32) matrix [0] [0];
	real32 m01 = (real32) matrix [0] [1];
	real32 m02 = (real32) matrix [0] [2];

	real32 m10 = (real32) matrix [1] [0];
	real32 m11 = (real32) matrix [1] [1];
	real32 m12 = (real32) matrix [1] [2];

	real32 m20 = (real32) matrix [2] [0];
	real32 m21 = (real32) matrix [2] [1];
	real32 m22 = (real32) matrix [2] [2];

	for (uint32 col = 0; col < count; col++)
		{

		real32 R = sPtrR [col];
		real32 G = sPtrG [col];
		real32 B = sPtrB [col];

		real32 r = m00 * R + m01 * G + m02 * B;
		real32 g = m10 * R + m11 * G + m12 * B;
		real32 b = m20 * R + m21 * G + m22 * B;

		r = Pin_real32 (0.0f, r, 1.0f);
		g = Pin_real32 (0.0f, g, 1.0f);
		b = Pin_real32 (0.0f, b, 1.0f);

		dPtrR [col] = r;
		dPtrG [col] = g;
		dPtrB [col] = b;

		}

	}

/*****************************************************************************/

void RefBaseline1DTable (const real32 *sPtr,
						 real32 *dPtr,
						 uint32 count,
						 const dng_1d_table &table)
	{

	for (uint32 col = 0; col < count; col++)
		{

		real32 x = sPtr [col];

		real32 y = table.Interpolate (x);

		dPtr [col] = y;

		}

	}

/*****************************************************************************/

void RefBaselineRGBTone (const real32 *sPtrR,
						 const real32 *sPtrG,
						 const real32 *sPtrB,
						 real32 *dPtrR,
						 real32 *dPtrG,
						 real32 *dPtrB,
						 uint32 count,
						 const dng_1d_table &table)
	{

	for (uint32 col = 0; col < count; col++)
		{

		real32 r = sPtrR [col];
		real32 g = sPtrG [col];
		real32 b = sPtrB [col];

		real32 rr;
		real32 gg;
		real32 bb;

		#define RGBTone(r, g, b, rr, gg, bb)\
			{\
			\
			DNG_ASSERT (r >= g && g >= b && r > b, "Logic Error RGBTone");\
			\
			rr = table.Interpolate (r);\
			bb = table.Interpolate (b);\
			\
			gg = bb + ((rr - bb) * (g - b) / (r - b));\
			\
			}

		if (r >= g)
			{

			if (g > b)
				{

				// Case 1: r >= g > b

				RGBTone (r, g, b, rr, gg, bb);

				}

			else if (b > r)
				{

				// Case 2: b > r >= g

				RGBTone (b, r, g, bb, rr, gg);

				}

			else if (b > g)
				{

				// Case 3: r >= b > g

				RGBTone (r, b, g, rr, bb, gg);

				}

			else
				{

				// Case 4: r >= g == b

				DNG_ASSERT (r >= g && g == b, "Logic Error 2");

				rr = table.Interpolate (r);
				gg = table.Interpolate (g);
				bb = gg;

				}

			}

		else
			{

			if (r >= b)
				{

				// Case 5: g > r >= b

				RGBTone (g, r, b, gg, rr, bb);

				}

			else if (b > g)
				{

				// Case 6: b > g > r

				RGBTone (b, g, r, bb, gg, rr);

				}

			else
				{

				// Case 7: g >= b > r

				RGBTone (g, b, r, gg, bb, rr);

				}

			}

		#undef RGBTone

		dPtrR [col] = rr;
		dPtrG [col] = gg;
		dPtrB [col] = bb;

		}

	}

/*****************************************************************************/

void RefResampleDown16 (const uint16 *sPtr,
						uint16 *dPtr,
						uint32 sCount,
						int32 sRowStep,
						const int16 *wPtr,
						uint32 wCount,
						uint32 pixelRange)
	{

	for (uint32 j = 0; j < sCount; j++)
		{

		int32 total = 8192;

		const uint16 *s = sPtr + j;

		for (uint32 k = 0; k < wCount; k++)
			{

			total += wPtr [k] * (int32) s [0];

			s += sRowStep;

			}

		dPtr [j] = (uint16) Pin_int32 (0,
									   total >> 14,
									   pixelRange);

		}

	}

/*****************************************************************************/

void RefResampleDown32 (const real32 *sPtr,
						real32 *dPtr,
						uint32 sCount,
						int32 sRowStep,
						const real32 *wPtr,
						uint32 wCount)
	{

	uint32 col;

	// Process first row.

	real32 w = wPtr [0];

	for (col = 0; col < sCount; col++)
		{

		dPtr [col] = w * sPtr [col];

		}

	sPtr += sRowStep;

	// Process middle rows.

	for (uint32 j = 1; j < wCount - 1; j++)
		{

		w = wPtr [j];

		for (col = 0; col < sCount; col++)
			{

			dPtr [col] += w * sPtr [col];

			}

		sPtr += sRowStep;

		}

	// Process last row.

	w = wPtr [wCount - 1];

	for (col = 0; col < sCount; col++)
		{

		dPtr [col] = Pin_real32 (0.0f,
							     dPtr [col] + w * sPtr [col],
							     1.0f);

		}

	}

/******************************************************************************/

void RefResampleAcross16 (const uint16 *sPtr,
						  uint16 *dPtr,
						  uint32 dCount,
						  const int32 *coord,
						  const int16 *wPtr,
						  uint32 wCount,
						  uint32 wStep,
						  uint32 pixelRange)
	{

	for (uint32 j = 0; j < dCount; j++)
		{

		int32 sCoord = coord [j];

		int32 sFract = sCoord &  kResampleSubsampleMask;
		int32 sPixel = sCoord >> kResampleSubsampleBits;

		const int16  *w = wPtr + sFract * wStep;
		const uint16 *s = sPtr + sPixel;

		int32 total = w [0] * (int32) s [0];

		for (uint32 k = 1; k < wCount; k++)
			{

			total += w [k] * (int32) s [k];

			}

		dPtr [j] = (uint16) Pin_int32 (0,
									   (total + 8192) >> 14,
									   pixelRange);

		}

	}

/******************************************************************************/

void RefResampleAcross32 (const real32 *sPtr,
						  real32 *dPtr,
						  uint32 dCount,
						  const int32 *coord,
						  const real32 *wPtr,
						  uint32 wCount,
						  uint32 wStep)
	{

	for (uint32 j = 0; j < dCount; j++)
		{

		int32 sCoord = coord [j];

		int32 sFract = sCoord &  kResampleSubsampleMask;
		int32 sPixel = sCoord >> kResampleSubsampleBits;

		const real32 *w = wPtr + sFract * wStep;
		const real32 *s = sPtr + sPixel;

		real32 total = w [0] * s [0];

		for (uint32 k = 1; k < wCount; k++)
			{

			total += w [k] * s [k];

			}

		dPtr [j] = Pin_real32 (0.0f, total, 1.0f);

		}

	}

/*****************************************************************************/

bool RefEqualBytes (const void *sPtr,
					const void *dPtr,
					uint32 count)
	{

	return memcmp (dPtr, sPtr, count) == 0;

	}

/*****************************************************************************/

bool RefEqualArea8 (const uint8 *sPtr,
				    const uint8 *dPtr,
				    uint32 rows,
				    uint32 cols,
				    uint32 planes,
				    int32 sRowStep,
				    int32 sColStep,
				    int32 sPlaneStep,
				    int32 dRowStep,
				    int32 dColStep,
				    int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint8 *sPtr1 = sPtr;
		const uint8 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint8 *sPtr2 = sPtr1;
			const uint8 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				if (*dPtr2 != *sPtr2)
					return false;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	return true;

	}

/*****************************************************************************/

bool RefEqualArea16 (const uint16 *sPtr,
					 const uint16 *dPtr,
					 uint32 rows,
					 uint32 cols,
					 uint32 planes,
					 int32 sRowStep,
					 int32 sColStep,
					 int32 sPlaneStep,
					 int32 dRowStep,
					 int32 dColStep,
					 int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint16 *sPtr1 = sPtr;
		const uint16 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint16 *sPtr2 = sPtr1;
			const uint16 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				if (*dPtr2 != *sPtr2)
					return false;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	return true;

	}

/*****************************************************************************/

bool RefEqualArea32 (const uint32 *sPtr,
					 const uint32 *dPtr,
					 uint32 rows,
					 uint32 cols,
					 uint32 planes,
					 int32 sRowStep,
					 int32 sColStep,
					 int32 sPlaneStep,
					 int32 dRowStep,
					 int32 dColStep,
					 int32 dPlaneStep)
	{

	for (uint32 row = 0; row < rows; row++)
		{

		const uint32 *sPtr1 = sPtr;
		const uint32 *dPtr1 = dPtr;

		for (uint32 col = 0; col < cols; col++)
			{

			const uint32 *sPtr2 = sPtr1;
			const uint32 *dPtr2 = dPtr1;

			for (uint32 plane = 0; plane < planes; plane++)
				{

				if (*dPtr2 != *sPtr2)
					return false;

				sPtr2 += sPlaneStep;
				dPtr2 += dPlaneStep;

				}

			sPtr1 += sColStep;
			dPtr1 += dColStep;

			}

		sPtr += sRowStep;
		dPtr += dRowStep;

		}

	return true;

	}

/*****************************************************************************/

void RefVignetteMask16 (uint16 *mPtr,
						uint32 rows,
						uint32 cols,
						int32 rowStep,
						int64 offsetH,
						int64 offsetV,
						int64 stepH,
						int64 stepV,
						uint32 tBits,
						const uint16 *table)
	{

	uint32 tShift = 32 - tBits;
	uint32 tRound = (1 << (tShift - 1));
	uint32 tLimit = 1 << tBits;

	for (uint32 row = 0; row < rows; row++)
		{

		int64 baseDelta = (offsetV + 32768) >> 16;

		baseDelta = baseDelta * baseDelta + tRound;

		int64 deltaH = offsetH + 32768;

		for (uint32 col = 0; col < cols; col++)
			{

			int64 temp = deltaH >> 16;

			int64 delta = baseDelta + (temp * temp);

			uint32 index = Min_uint32 ((uint32) (delta >> tShift), tLimit);

			mPtr [col] = table [index];

			deltaH += stepH;

			}

		offsetV += stepV;

		mPtr += rowStep;

		}

	}

/*****************************************************************************/

void RefVignette16 (int16 *sPtr,
					const uint16 *mPtr,
					uint32 rows,
					uint32 cols,
					uint32 planes,
					int32 sRowStep,
					int32 sPlaneStep,
					int32 mRowStep,
					uint32 mBits)
	{

	const uint32 mRound = 1 << (mBits - 1);

	switch (planes)
		{

		case 1:
			{

			for (uint32 row = 0; row < rows; row++)
				{

				for (uint32 col = 0; col < cols; col++)
					{

					uint32 s = sPtr [col] + 32768;

					uint32 m = mPtr [col];

					s = (s * m + mRound) >> mBits;

					s = Min_uint32 (s, 65535);

					sPtr [col] = (int16) (s - 32768);

					}

				sPtr += sRowStep;

				mPtr += mRowStep;

				}

			break;

			}

		case 3:
			{

			int16 *rPtr = sPtr;
			int16 *gPtr = rPtr + sPlaneStep;
			int16 *bPtr = gPtr + sPlaneStep;

			for (uint32 row = 0; row < rows; row++)
				{

				for (uint32 col = 0; col < cols; col++)
					{

					uint32 r = rPtr [col] + 32768;
					uint32 g = gPtr [col] + 32768;
					uint32 b = bPtr [col] + 32768;

					uint32 m = mPtr [col];

					r = (r * m + mRound) >> mBits;
					g = (g * m + mRound) >> mBits;
					b = (b * m + mRound) >> mBits;

					r = Min_uint32 (r, 65535);
					g = Min_uint32 (g, 65535);
					b = Min_uint32 (b, 65535);

					rPtr [col] = (int16) (r - 32768);
					gPtr [col] = (int16) (g - 32768);
					bPtr [col] = (int16) (b - 32768);

					}

				rPtr += sRowStep;
				gPtr += sRowStep;
				bPtr += sRowStep;

				mPtr += mRowStep;

				}

			break;

			}

		case 4:
			{

			int16 *aPtr = sPtr;
			int16 *bPtr = aPtr + sPlaneStep;
			int16 *cPtr = bPtr + sPlaneStep;
			int16 *dPtr = cPtr + sPlaneStep;

			for (uint32 row = 0; row < rows; row++)
				{

				for (uint32 col = 0; col < cols; col++)
					{

					uint32 a = aPtr [col] + 32768;
					uint32 b = bPtr [col] + 32768;
					uint32 c = cPtr [col] + 32768;
					uint32 d = dPtr [col] + 32768;

					uint32 m = mPtr [col];

					a = (a * m + mRound) >> mBits;
					b = (b * m + mRound) >> mBits;
					c = (c * m + mRound) >> mBits;
					d = (d * m + mRound) >> mBits;

					a = Min_uint32 (a, 65535);
					b = Min_uint32 (b, 65535);
					c = Min_uint32 (c, 65535);
					d = Min_uint32 (d, 65535);

					aPtr [col] = (int16) (a - 32768);
					bPtr [col] = (int16) (b - 32768);
					cPtr [col] = (int16) (c - 32768);
					dPtr [col] = (int16) (d - 32768);

					}

				aPtr += sRowStep;
				bPtr += sRowStep;
				cPtr += sRowStep;
				dPtr += sRowStep;

				mPtr += mRowStep;

				}

			break;

			}

		default:
			{

			for (uint32 plane = 0; plane < planes; plane++)
				{

				int16 *planePtr = sPtr;

				const uint16 *maskPtr = mPtr;

				for (uint32 row = 0; row < rows; row++)
					{

					for (uint32 col = 0; col < cols; col++)
						{

						uint32 s = planePtr [col] + 32768;

						uint32 m = maskPtr [col];

						s = (s * m + mRound) >> mBits;

						s = Min_uint32 (s, 65535);

						planePtr [col] = (int16) (s - 32768);

						}

					planePtr += sRowStep;

					maskPtr += mRowStep;

					}

				sPtr += sPlaneStep;

				}

			break;

			}

		}

	}

/******************************************************************************/

void RefMapArea16 (uint16 *dPtr,
				   uint32 count0,
				   uint32 count1,
				   uint32 count2,
				   int32 step0,
				   int32 step1,
				   int32 step2,
				   const uint16 *map)
	{

	if (step2 == 1 && count2 >= 32)
		{

		for (uint32 index0 = 0; index0 < count0; index0++)
			{

			uint16 *d1 = dPtr;

			for (uint32 index1 = 0; index1 < count1; index1++)
				{

				uint16 *d2 = d1;

				uint32 count = count2;

				// Get the data 32-bit aligned if it is not.

				if (!IsAligned32 (dPtr))
					{

					d2 [0] = map [d2 [0]];

					count--;

					d2++;

					}

				// Use 32-bit reads and writes for bulk processing.

				uint32 *dPtr32 = (uint32 *) d2;

				// Process in blocks of 16 pixels.

				uint32 blocks = count >> 4;

				count -= blocks << 4;
				d2    += blocks << 4;

				while (blocks--)
					{

					uint32 x0, x1, x2, x3, x4, x5, x6, x7;
					uint32 p0, p1, p2, p3, p4, p5, p6, p7;

					// Use 32 bit reads & writes, and pack and unpack the 16-bit values.
					// This results in slightly higher performance.

					// Note that this code runs on both little-endian and big-endian systems,
					// since the pixels are either never swapped or double swapped.

					x0 = dPtr32 [0];
					x1 = dPtr32 [1];
					x2 = dPtr32 [2];
					x3 = dPtr32 [3];

					p0 = map [x0 >> 16    ];
					p1 = map [x0 & 0x0FFFF];
					p2 = map [x1 >> 16    ];
					p3 = map [x1 & 0x0FFFF];
					p4 = map [x2 >> 16    ];
					p5 = map [x2 & 0x0FFFF];
					p6 = map [x3 >> 16    ];
					p7 = map [x3 & 0x0FFFF];

					x0 = (p0 << 16) | p1;
					x1 = (p2 << 16) | p3;
					x2 = (p4 << 16) | p5;
					x3 = (p6 << 16) | p7;

					x4 = dPtr32 [4];
					x5 = dPtr32 [5];
					x6 = dPtr32 [6];
					x7 = dPtr32 [7];

					dPtr32 [0] = x0;
					dPtr32 [1] = x1;
					dPtr32 [2] = x2;
					dPtr32 [3] = x3;

					p0 = map [x4 >> 16    ];
					p1 = map [x4 & 0x0FFFF];
					p2 = map [x5 >> 16    ];
					p3 = map [x5 & 0x0FFFF];
					p4 = map [x6 >> 16    ];
					p5 = map [x6 & 0x0FFFF];
					p6 = map [x7 >> 16    ];
					p7 = map [x7 & 0x0FFFF];

					x4 = (p0 << 16) | p1;
					x5 = (p2 << 16) | p3;
					x6 = (p4 << 16) | p5;
					x7 = (p6 << 16) | p7;

					dPtr32 [4] = x4;
					dPtr32 [5] = x5;
					dPtr32 [6] = x6;
					dPtr32 [7] = x7;

					dPtr32 += 8;

					}

				// Process remaining columns.

				for (uint32 j = 0; j < count; j++)
					{

					d2 [j] = map [d2 [j]];

					}

				d1 += step1;

				}

			dPtr += step0;

			}

		}

	else
		{

		for (uint32 index0 = 0; index0 < count0; index0++)
			{

			uint16 *d1 = dPtr;

			for (uint32 index1 = 0; index1 < count1; index1++)
				{

				uint16 *d2 = d1;

				for (uint32 index2 = 0; index2 < count2; index2++)
					{

					d2 [0] = map [d2 [0]];

					d2 += step2;

					}

				d1 += step1;

				}

			dPtr += step0;

			}

		}

	}

/*****************************************************************************/
