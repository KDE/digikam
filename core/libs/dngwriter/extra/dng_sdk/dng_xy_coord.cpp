/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_xy_coord.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_xy_coord.h"

#include "dng_matrix.h"
#include "dng_utils.h"

/******************************************************************************/

dng_xy_coord XYZtoXY (const dng_vector_3 &coord)
	{

	real64 X = coord [0];
	real64 Y = coord [1];
	real64 Z = coord [2];

	real64 total = X + Y + Z;

	if (total > 0.0)
		{

		return dng_xy_coord (X / total,
						     Y / total);

		}

	return D50_xy_coord ();

	}

/*****************************************************************************/

dng_vector_3 XYtoXYZ (const dng_xy_coord &coord)
	{

	dng_xy_coord temp = coord;

	// Restrict xy coord to someplace inside the range of real xy coordinates.
	// This prevents math from doing strange things when users specify
	// extreme temperature/tint coordinates.

	temp.x = Pin_real64 (0.000001, temp.x, 0.999999);
	temp.y = Pin_real64 (0.000001, temp.y, 0.999999);

	if (temp.x + temp.y > 0.999999)
		{
		real64 scale = 0.999999 / (temp.x + temp.y);
		temp.x *= scale;
		temp.y *= scale;
		}

	return dng_vector_3 (temp.x / temp.y,
					     1.0,
					     (1.0 - temp.x - temp.y) / temp.y);

	}

/*****************************************************************************/

dng_xy_coord PCStoXY ()
	{

	return D50_xy_coord ();

	}

/*****************************************************************************/

dng_vector_3 PCStoXYZ ()
	{

	return XYtoXYZ (PCStoXY ());

	}

/*****************************************************************************/
