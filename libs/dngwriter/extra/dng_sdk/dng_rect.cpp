/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_rect.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_rect.h"

#include "dng_utils.h"

/*****************************************************************************/

bool dng_rect::operator== (const dng_rect &rect) const
	{

	return (rect.t == t) &&
		   (rect.l == l) &&
		   (rect.b == b) &&
		   (rect.r == r);

	}

/*****************************************************************************/

bool dng_rect::IsZero () const
	{

	return (t == 0) && (l == 0) && (b == 0) && (r == 0);

	}

/*****************************************************************************/

bool dng_rect_real64::operator== (const dng_rect_real64 &rect) const
	{

	return (rect.t == t) &&
		   (rect.l == l) &&
		   (rect.b == b) &&
		   (rect.r == r);

	}

/*****************************************************************************/

bool dng_rect_real64::IsZero () const
	{

	return (t == 0.0) && (l == 0.0) && (b == 0.0) && (r == 0.0);

	}

/*****************************************************************************/

dng_rect operator& (const dng_rect &a,
					const dng_rect &b)
	{

	dng_rect c;

	c.t = Max_int32 (a.t, b.t);
	c.l = Max_int32 (a.l, b.l);

	c.b = Min_int32 (a.b, b.b);
	c.r = Min_int32 (a.r, b.r);

	if (c.IsEmpty ())
		{

		c = dng_rect ();

		}

	return c;

	}

/*****************************************************************************/

dng_rect operator| (const dng_rect &a,
					const dng_rect &b)
	{

	if (a.IsEmpty ())
		{
		return b;
		}

	if (b.IsEmpty ())
		{
		return a;
		}

	dng_rect c;

	c.t = Min_int32 (a.t, b.t);
	c.l = Min_int32 (a.l, b.l);

	c.b = Max_int32 (a.b, b.b);
	c.r = Max_int32 (a.r, b.r);

	return c;

	}

/*****************************************************************************/

dng_rect_real64 operator& (const dng_rect_real64 &a,
						   const dng_rect_real64 &b)
	{

	dng_rect_real64 c;

	c.t = Max_real64 (a.t, b.t);
	c.l = Max_real64 (a.l, b.l);

	c.b = Min_real64 (a.b, b.b);
	c.r = Min_real64 (a.r, b.r);

	if (c.IsEmpty ())
		{

		c = dng_rect_real64 ();

		}

	return c;

	}

/*****************************************************************************/

dng_rect_real64 operator| (const dng_rect_real64 &a,
						   const dng_rect_real64 &b)
	{

	if (a.IsEmpty ())
		{
		return b;
		}

	if (b.IsEmpty ())
		{
		return a;
		}

	dng_rect_real64 c;

	c.t = Min_real64 (a.t, b.t);
	c.l = Min_real64 (a.l, b.l);

	c.b = Max_real64 (a.b, b.b);
	c.r = Max_real64 (a.r, b.r);

	return c;

	}

/*****************************************************************************/
