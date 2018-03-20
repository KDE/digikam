/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_xy_coord.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_xy_coord__
#define __dng_xy_coord__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_xy_coord
	{

	public:

		real64 x;
		real64 y;

	public:

		dng_xy_coord ()
			:	x (0.0)
			,	y (0.0)
			{
			}

		dng_xy_coord (real64 xx, real64 yy)
			:	x (xx)
			,	y (yy)
			{
			}

		void Clear ()
			{
			x = 0.0;
			y = 0.0;
			}

		bool IsValid () const
			{
			return x > 0.0 &&
				   y > 0.0;
			}

		bool NotValid () const
			{
			return !IsValid ();
			}

		bool operator== (const dng_xy_coord &coord) const
			{
			return coord.x == x &&
				   coord.y == y;
			}

		bool operator!= (const dng_xy_coord &coord) const
			{
			return !(*this == coord);
			}

	};

/*****************************************************************************/

inline dng_xy_coord operator+ (const dng_xy_coord &A,
							   const dng_xy_coord &B)
	{

	dng_xy_coord C;

	C.x = A.x + B.x;
	C.y = A.y + B.y;

	return C;

	}

/*****************************************************************************/

inline dng_xy_coord operator- (const dng_xy_coord &A,
							   const dng_xy_coord &B)
	{

	dng_xy_coord C;

	C.x = A.x - B.x;
	C.y = A.y - B.y;

	return C;

	}

/*****************************************************************************/

inline dng_xy_coord operator* (real64 scale,
							   const dng_xy_coord &A)
	{

	dng_xy_coord B;

	B.x = A.x * scale;
	B.y = A.y * scale;

	return B;

	}

/******************************************************************************/

inline real64 operator* (const dng_xy_coord &A,
						 const dng_xy_coord &B)
	{

	return A.x * B.x +
		   A.y * B.y;

	}

/*****************************************************************************/

// Standard xy coordinate constants.

inline dng_xy_coord StdA_xy_coord ()
	{
	return dng_xy_coord (0.4476, 0.4074);
	}

inline dng_xy_coord D50_xy_coord ()
	{
	return dng_xy_coord (0.3457, 0.3585);
	}

inline dng_xy_coord D55_xy_coord ()
	{
	return dng_xy_coord (0.3324, 0.3474);
	}

inline dng_xy_coord D65_xy_coord ()
	{
	return dng_xy_coord (0.3127, 0.3290);
	}

inline dng_xy_coord D75_xy_coord ()
	{
	return dng_xy_coord (0.2990, 0.3149);
	}

/*****************************************************************************/

// Convert between xy coordinates and XYZ coordinates.

dng_xy_coord XYZtoXY (const dng_vector_3 &coord);

dng_vector_3 XYtoXYZ (const dng_xy_coord &coord);

/*****************************************************************************/

// Returns the ICC XYZ profile connection space white point.

dng_xy_coord PCStoXY ();

dng_vector_3 PCStoXYZ ();

/*****************************************************************************/

#endif

/*****************************************************************************/
