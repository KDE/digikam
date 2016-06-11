/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_rational.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_rational.h"

#include "dng_utils.h"

/*****************************************************************************/

real64 dng_srational::As_real64 () const
	{

	if (d)
		return (real64) n / (real64) d;

	else
		return 0.0;

	}

/*****************************************************************************/

void dng_srational::Set_real64 (real64 x, int32 dd)
	{

	if (x == 0.0)
		{

		*this = dng_srational (0, 1);

		}

	if (dd == 0)
		{

		real64 y = Abs_real64 (x);

		if (y >= 32768.0)
			{
			dd = 1;
			}

		else if (y >= 1.0)
			{
			dd = 32768;
			}

		else
			{
			dd = 32768 * 32768;
			}

		}

	*this = dng_srational (Round_int32 (x * dd), dd);

	}

/*****************************************************************************/

void dng_srational::ReduceByFactor (int32 factor)
	{

	while (n % factor == 0 &&
		   d % factor == 0 &&
		   d >= factor)
		{
		n /= factor;
		d /= factor;
		}

	}

/*****************************************************************************/

real64 dng_urational::As_real64 () const
	{

	if (d)
		return (real64) n / (real64) d;

	else
		return 0.0;

	}

/*****************************************************************************/

void dng_urational::Set_real64 (real64 x, uint32 dd)
	{

	if (x <= 0.0)
		{

		*this = dng_urational (0, 1);

		}

	if (dd == 0)
		{

		if (x >= 32768.0)
			{
			dd = 1;
			}

		else if (x >= 1.0)
			{
			dd = 32768;
			}

		else
			{
			dd = 32768 * 32768;
			}

		}

	*this = dng_urational (Round_uint32 (x * dd), dd);

	}

/*****************************************************************************/

void dng_urational::ReduceByFactor (uint32 factor)
	{

	while (n % factor == 0 &&
		   d % factor == 0 &&
		   d >= factor)
		{
		n /= factor;
		d /= factor;
		}

	}

/*****************************************************************************/
