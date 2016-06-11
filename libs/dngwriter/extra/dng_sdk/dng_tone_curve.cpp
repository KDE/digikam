/*****************************************************************************/
// Copyright 2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_tone_curve.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_tone_curve.h"

#include "dng_assertions.h"
#include "dng_spline.h"
#include "dng_utils.h"

/******************************************************************************/

dng_tone_curve::dng_tone_curve ()

	:	fCoord ()

	{

	SetNull ();

	}

/******************************************************************************/

bool dng_tone_curve::operator== (const dng_tone_curve &curve) const
	{

	return fCoord == curve.fCoord;

	}

/******************************************************************************/

void dng_tone_curve::SetNull ()
	{

	fCoord.resize (2);

	fCoord [0].h = 0.0;
	fCoord [0].v = 0.0;

	fCoord [1].h = 1.0;
	fCoord [1].v = 1.0;

	}

/******************************************************************************/

bool dng_tone_curve::IsNull () const
	{

	dng_tone_curve temp;

	return (*this == temp);

	}

/******************************************************************************/

void dng_tone_curve::SetInvalid ()
	{

	fCoord.clear ();

	}

/******************************************************************************/

bool dng_tone_curve::IsValid () const
	{

	if (fCoord.size () < 2)
		{

		return false;

		}

	for (uint32 j = 0; j < fCoord.size (); j++)
		{

		if (fCoord [j] . h < 0.0 || fCoord [j] . h > 1.0 ||
			fCoord [j] . v < 0.0 || fCoord [j] . v > 1.0)
			{

			return false;

			}

		if (j > 0)
			{

			if (fCoord [j] . h <= fCoord [j - 1] . h)
				{

				return false;

				}

			}

		}

	return true;

	}

/******************************************************************************/

void dng_tone_curve::Solve (dng_spline_solver &solver) const
	{

	solver.Reset ();

	for (uint32 index = 0; index < fCoord.size (); index++)
		{

		solver.Add (fCoord [index].h,
					fCoord [index].v);

		}

	solver.Solve ();

	}

/*****************************************************************************/
