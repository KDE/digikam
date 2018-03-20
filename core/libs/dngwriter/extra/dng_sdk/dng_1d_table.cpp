/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_1d_table.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_1d_table.h"

#include "dng_1d_function.h"
#include "dng_memory.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_1d_table::dng_1d_table ()

	:	fBuffer ()
	,	fTable  (NULL)

	{

	}

/*****************************************************************************/

dng_1d_table::~dng_1d_table ()
	{

	}

/*****************************************************************************/

void dng_1d_table::SubDivide (const dng_1d_function &function,
							  uint32 lower,
							  uint32 upper,
							  real32 maxDelta)
	{

	uint32 range = upper - lower;

	bool subDivide = (range > (kTableSize >> 8));

	if (!subDivide)
		{

		real32 delta = Abs_real32 (fTable [upper] -
								   fTable [lower]);

		if (delta > maxDelta)
			{

			subDivide = true;

			}

		}

	if (subDivide)
		{

		uint32 middle = (lower + upper) >> 1;

		fTable [middle] = (real32) function.Evaluate (middle * (1.0 / (real64) kTableSize));

		if (range > 2)
			{

			SubDivide (function, lower, middle, maxDelta);

			SubDivide (function, middle, upper, maxDelta);

			}

		}

	else
		{

		real64 y0 = fTable [lower];
		real64 y1 = fTable [upper];

		real64 delta = (y1 - y0) / (real64) range;

		for (uint32 j = lower + 1; j < upper; j++)
			{

			y0 += delta;

			fTable [j] = (real32) y0;

			}

		}

	}

/*****************************************************************************/

void dng_1d_table::Initialize (dng_memory_allocator &allocator,
							   const dng_1d_function &function,
							   bool subSample)
	{

	fBuffer.Reset (allocator.Allocate ((kTableSize + 2) * sizeof (real32)));

	fTable = fBuffer->Buffer_real32 ();

	if (subSample)
		{

		fTable [0         ] = (real32) function.Evaluate (0.0);
		fTable [kTableSize] = (real32) function.Evaluate (1.0);

		real32 maxDelta = Max_real32 (Abs_real32 (fTable [kTableSize] -
												  fTable [0         ]), 1.0f) *
						  (1.0f / 256.0f);

		SubDivide (function,
				   0,
				   kTableSize,
				   maxDelta);

		}

	else
		{

		for (uint32 j = 0; j <= kTableSize; j++)
			{

			real64 x = j * (1.0 / (real64) kTableSize);

			real64 y = function.Evaluate (x);

			fTable [j] = (real32) y;

			}

		}

	fTable [kTableSize + 1] = fTable [kTableSize];

	}

/*****************************************************************************/

void dng_1d_table::Expand16 (uint16 *table16) const
	{

	real64 step = (real64) kTableSize / 65535.0;

	real64 y0 = fTable [0];
	real64 y1 = fTable [1];

	real64 base  = y0 * 65535.0 + 0.5;
	real64 slope = (y1 - y0) * 65535.0;

	uint32 index = 1;
	real64 fract = 0.0;

	for (uint32 j = 0; j < 0x10000; j++)
		{

		table16 [j] = (uint16) (base + slope * fract);

		fract += step;

		if (fract > 1.0)
			{

			index += 1;
			fract -= 1.0;

			y0 = y1;
			y1 = fTable [index];

			base  = y0 * 65535.0 + 0.5;
			slope = (y1 - y0) * 65535.0;

			}

		}

	}

/*****************************************************************************/
