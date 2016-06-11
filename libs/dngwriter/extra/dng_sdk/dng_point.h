/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_point.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_point__
#define __dng_point__

/*****************************************************************************/

#include "dng_types.h"
#include "dng_utils.h"

/*****************************************************************************/

class dng_point
	{

	public:

		int32 v;
		int32 h;

	public:

		dng_point ()
			:	v (0)
			,	h (0)
			{
			}

		dng_point (int32 vv, int32 hh)
			:	v (vv)
			,	h (hh)
			{
			}

		bool operator== (const dng_point &pt) const
			{
			return (v == pt.v) &&
				   (h == pt.h);
			}

		bool operator!= (const dng_point &pt) const
			{
			return !(*this == pt);
			}

	};

/*****************************************************************************/

class dng_point_real64
	{

	public:

		real64 v;
		real64 h;

	public:

		dng_point_real64 ()
			:	v (0.0)
			,	h (0.0)
			{
			}

		dng_point_real64 (real64 vv, real64 hh)
			:	v (vv)
			,	h (hh)
			{
			}

		dng_point_real64 (const dng_point &pt)
			:	v ((real64) pt.v)
			,	h ((real64) pt.h)
			{
			}

		bool operator== (const dng_point_real64 &pt) const
			{
			return (v == pt.v) &&
				   (h == pt.h);
			}

		bool operator!= (const dng_point_real64 &pt) const
			{
			return !(*this == pt);
			}

		dng_point Round () const
			{
			return dng_point (Round_int32 (v),
							  Round_int32 (h));
			}

	};

/*****************************************************************************/

inline dng_point operator+ (const dng_point &a,
				  			const dng_point &b)


	{

	return dng_point (a.v + b.v,
					  a.h + b.h);

	}

/*****************************************************************************/

inline dng_point_real64 operator+ (const dng_point_real64 &a,
				  				   const dng_point_real64 &b)


	{

	return dng_point_real64 (a.v + b.v,
					  		 a.h + b.h);

	}

/*****************************************************************************/

inline dng_point operator- (const dng_point &a,
				  			const dng_point &b)


	{

	return dng_point (a.v - b.v,
					  a.h - b.h);

	}

/*****************************************************************************/

inline dng_point_real64 operator- (const dng_point_real64 &a,
				  				   const dng_point_real64 &b)


	{

	return dng_point_real64 (a.v - b.v,
					         a.h - b.h);

	}

/*****************************************************************************/

inline real64 DistanceSquared (const dng_point_real64 &a,
							   const dng_point_real64 &b)


	{

	dng_point_real64 diff = a - b;

	return (diff.v * diff.v) + (diff.h * diff.h);

	}

/*****************************************************************************/

inline dng_point Transpose (const dng_point &a)
	{

	return dng_point (a.h, a.v);

	}

/*****************************************************************************/

inline dng_point_real64 Transpose (const dng_point_real64 &a)
	{

	return dng_point_real64 (a.h, a.v);

	}

/*****************************************************************************/

#endif

/*****************************************************************************/
