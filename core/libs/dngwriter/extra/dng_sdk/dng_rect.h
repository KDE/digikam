/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_rect.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_rect__
#define __dng_rect__

/*****************************************************************************/

#include "dng_types.h"
#include "dng_point.h"
#include "dng_utils.h"

/*****************************************************************************/

class dng_rect
	{

	public:

		int32 t;
		int32 l;
		int32 b;
		int32 r;

	public:

		dng_rect ()
			:	t (0)
			,	l (0)
			,	b (0)
			,	r (0)
			{
			}

		dng_rect (int32 tt, int32 ll, int32 bb, int32 rr)
			:	t (tt)
			,	l (ll)
			,	b (bb)
			,	r (rr)
			{
			}

		dng_rect (uint32 h, uint32 w)
			:	t (0)
			,	l (0)
			,	b (h)
			,	r (w)
			{
			}

		dng_rect (const dng_point &size)
			:	t (0)
			,	l (0)
			,	b (size.v)
			,	r (size.h)
			{
			}

		void Clear ()
			{
			*this = dng_rect ();
			}

		bool operator== (const dng_rect &rect) const;

		bool operator!= (const dng_rect &rect) const
			{
			return !(*this == rect);
			}

		bool IsZero () const;

		bool NotZero () const
			{
			return !IsZero ();
			}

		bool IsEmpty () const
			{
			return (t >= b) || (l >= r);
			}

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		uint32 W () const
			{
			return (r >= l ? (uint32) (r - l) : 0);
			}

		uint32 H () const
			{
			return (b >= t ? (uint32) (b - t) : 0);
			}

		dng_point TL () const
			{
			return dng_point (t, l);
			}

		dng_point TR () const
			{
			return dng_point (t, r);
			}

		dng_point BL () const
			{
			return dng_point (b, l);
			}

		dng_point BR () const
			{
			return dng_point (b, r);
			}

		dng_point Size () const
			{
			return dng_point (H (), W ());
			}

		real64 Diagonal () const
			{
			return hypot ((real64) W (),
						  (real64) H ());
			}

	};

/*****************************************************************************/

class dng_rect_real64
	{

	public:

		real64 t;
		real64 l;
		real64 b;
		real64 r;

	public:

		dng_rect_real64 ()
			:	t (0.0)
			,	l (0.0)
			,	b (0.0)
			,	r (0.0)
			{
			}

		dng_rect_real64 (real64 tt, real64 ll, real64 bb, real64 rr)
			:	t (tt)
			,	l (ll)
			,	b (bb)
			,	r (rr)
			{
			}

		dng_rect_real64 (real64 h, real64 w)
			:	t (0)
			,	l (0)
			,	b (h)
			,	r (w)
			{
			}

		dng_rect_real64 (const dng_point_real64 &size)
			:	t (0)
			,	l (0)
			,	b (size.v)
			,	r (size.h)
			{
			}

		dng_rect_real64 (const dng_rect &rect)
			:	t ((real64) rect.t)
			,	l ((real64) rect.l)
			,	b ((real64) rect.b)
			,	r ((real64) rect.r)
			{
			}

		void Clear ()
			{
			*this = dng_point_real64 ();
			}

		bool operator== (const dng_rect_real64 &rect) const;

		bool operator!= (const dng_rect_real64 &rect) const
			{
			return !(*this == rect);
			}

		bool IsZero () const;

		bool NotZero () const
			{
			return !IsZero ();
			}

		bool IsEmpty () const
			{
			return (t >= b) || (l >= r);
			}

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		real64 W () const
			{
			return Max_real64 (r - l, 0.0);
			}

		real64 H () const
			{
			return Max_real64 (b - t, 0.0);
			}

		dng_point_real64 TL () const
			{
			return dng_point_real64 (t, l);
			}

		dng_point_real64 TR () const
			{
			return dng_point_real64 (t, r);
			}

		dng_point_real64 BL () const
			{
			return dng_point_real64 (b, l);
			}

		dng_point_real64 BR () const
			{
			return dng_point_real64 (b, r);
			}

		dng_point_real64 Size () const
			{
			return dng_point_real64 (H (), W ());
			}

		dng_rect Round () const
			{
			return dng_rect (Round_int32 (t),
							 Round_int32 (l),
							 Round_int32 (b),
							 Round_int32 (r));
			}

		real64 Diagonal () const
			{
			return hypot (W (), H ());
			}

	};

/*****************************************************************************/

dng_rect operator& (const dng_rect &a,
					const dng_rect &b);

dng_rect operator| (const dng_rect &a,
					const dng_rect &b);

/*****************************************************************************/

dng_rect_real64 operator& (const dng_rect_real64 &a,
						   const dng_rect_real64 &b);

dng_rect_real64 operator| (const dng_rect_real64 &a,
						   const dng_rect_real64 &b);

/*****************************************************************************/

inline dng_rect operator+ (const dng_rect &a,
					       const dng_point &b)
	{

	return dng_rect (a.t + b.v,
					 a.l + b.h,
					 a.b + b.v,
					 a.r + b.h);

	}

/*****************************************************************************/

inline dng_rect_real64 operator+ (const dng_rect_real64 &a,
					       		  const dng_point_real64 &b)
	{

	return dng_rect_real64 (a.t + b.v,
					 		a.l + b.h,
					 		a.b + b.v,
					 		a.r + b.h);

	}

/*****************************************************************************/

inline dng_rect operator- (const dng_rect &a,
					       const dng_point &b)
	{

	return dng_rect (a.t - b.v,
					 a.l - b.h,
					 a.b - b.v,
					 a.r - b.h);

	}

/*****************************************************************************/

inline dng_rect_real64 operator- (const dng_rect_real64 &a,
					       		  const dng_point_real64 &b)
	{

	return dng_rect_real64 (a.t - b.v,
					 		a.l - b.h,
					 		a.b - b.v,
					 		a.r - b.h);

	}

/*****************************************************************************/

inline dng_rect Transpose (const dng_rect &a)
	{

	return dng_rect (a.l, a.t, a.r, a.b);

	}

/*****************************************************************************/

inline dng_rect_real64 Transpose (const dng_rect_real64 &a)
	{

	return dng_rect_real64 (a.l, a.t, a.r, a.b);

	}

/*****************************************************************************/

inline void HalfRect (dng_rect &rect)
	{

	rect.r = rect.l + (rect.W () >> 1);
	rect.b = rect.t + (rect.H () >> 1);

	}

/*****************************************************************************/

inline void DoubleRect (dng_rect &rect)
	{

	rect.r = rect.l + (rect.W () << 1);
	rect.b = rect.t + (rect.H () << 1);

	}

/*****************************************************************************/

inline void InnerPadRect (dng_rect &rect,
						  int32 pad)
	{

	rect.l += pad;
	rect.r -= pad;
	rect.t += pad;
	rect.b -= pad;

	}

/*****************************************************************************/

inline void OuterPadRect (dng_rect &rect,
						  int32 pad)
	{

	InnerPadRect (rect, -pad);

	}

/*****************************************************************************/

inline void InnerPadRectH (dng_rect &rect,
						   int32 pad)
	{

	rect.l += pad;
	rect.r -= pad;

	}

/*****************************************************************************/

inline void InnerPadRectV (dng_rect &rect,
						   int32 pad)
	{

	rect.t += pad;
	rect.b -= pad;

	}

/*****************************************************************************/

inline dng_rect MakeHalfRect (const dng_rect &rect)
	{

	dng_rect out = rect;

	HalfRect (out);

	return out;

	}

/*****************************************************************************/

inline dng_rect MakeDoubleRect (const dng_rect &rect)
	{

	dng_rect out = rect;

	DoubleRect (out);

	return out;

	}

/*****************************************************************************/

inline dng_rect MakeInnerPadRect (const dng_rect &rect,
								  int32 pad)
	{

	dng_rect out = rect;

	InnerPadRect (out, pad);

	return out;

	}

/*****************************************************************************/

#endif

/*****************************************************************************/
