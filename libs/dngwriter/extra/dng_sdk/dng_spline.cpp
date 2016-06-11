/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_spline.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_spline.h"

#include "dng_assertions.h"
#include "dng_exceptions.h"

/******************************************************************************/

dng_spline_solver::dng_spline_solver ()

	:	X ()
	,	Y ()
	,	S ()

	{

	}

/******************************************************************************/

dng_spline_solver::~dng_spline_solver ()
	{

	}

/******************************************************************************/

void dng_spline_solver::Reset ()
	{

	X.clear ();
	Y.clear ();

	S.clear ();

	}

/******************************************************************************/

void dng_spline_solver::Add (real64 x, real64 y)
	{

	X.push_back (x);
	Y.push_back (y);

	}

/******************************************************************************/

void dng_spline_solver::Solve ()
	{

	// This code computes the unique curve such that:
	//		It is C0, C1, and C2 continuous
	//		The second derivative is zero at the end points

	int32 count = (int32) X.size ();

	DNG_ASSERT (count >= 2, "Too few points");

	int32 start = 0;
	int32 end   = count;

	real64 A =  X [start+1] - X [start];
	real64 B = (Y [start+1] - Y [start]) / A;

	S.resize (count);

	S [start] = B;

	int32 j;

	// Slopes here are a weighted average of the slopes
	// to each of the adjcent control points.

	for (j = start + 2; j < end; ++j)
		{

		real64 C = X [j] - X [j-1];
		real64 D = (Y [j] - Y [j-1]) / C;

		S [j-1] = (B * C + D * A) / (A + C);

		A = C;
		B = D;

		}

	S [end-1] = 2.0 * B - S [end-2];
	S [start] = 2.0 * S [start] - S [start+1];

	if ((end - start) > 2)
		{

		std::vector<real64> E;
		std::vector<real64> F;
		std::vector<real64> G;

		E.resize (count);
		F.resize (count);
		G.resize (count);

		F [start] = 0.5;
		E [end-1] = 0.5;
		G [start] = 0.75 * (S [start] + S [start+1]);
		G [end-1] = 0.75 * (S [end-2] + S [end-1]);

		for (j = start+1; j < end - 1; ++j)
			{

			A = (X [j+1] - X [j-1]) * 2.0;

			E [j] = (X [j+1] - X [j]) / A;
			F [j] = (X [j] - X [j-1]) / A;
			G [j] = 1.5 * S [j];

			}

		for (j = start+1; j < end; ++j)
			{

			A = 1.0 - F [j-1] * E [j];

			if (j != end-1) F [j] /= A;

			G [j] = (G [j] - G [j-1] * E [j]) / A;

			}

		for (j = end - 2; j >= start; --j)
			G [j] = G [j] - F [j] * G [j+1];

		for (j = start; j < end; ++j)
			S [j] = G [j];

		}

	}

/******************************************************************************/

bool dng_spline_solver::IsIdentity () const
	{

	int32 count = (int32) X.size ();

	if (count != 2)
		return false;

	if (X [0] != 0.0 || X [1] != 1.0 ||
		Y [0] != 0.0 || Y [1] != 1.0)
		return false;

	return true;

	}

/******************************************************************************/

real64 dng_spline_solver::Evaluate (real64 x) const
	{

	int32 count = (int32) X.size ();

	// Check for off each end of point list.

	if (x <= X [0])
		return Y [0];

	if (x >= X [count-1])
		return Y [count-1];

	// Binary search for the index.

	int32 lower = 1;
	int32 upper = count - 1;

	while (upper > lower)
		{

		int32 mid = (lower + upper) >> 1;

		if (x == X [mid])
			{
			return Y [mid];
			}

		if (x > X [mid])
			lower = mid + 1;
		else
			upper = mid;

		}

	DNG_ASSERT (upper == lower, "Binary search error in point list");

	int32 j = lower;

	// X [j - 1] < x <= X [j]
	// A is the distance between the X [j] and X [j - 1]
	// B and C describe the fractional distance to either side. B + C = 1.

	// We compute a cubic spline between the two points with slopes
	// S[j-1] and S[j] at either end. Specifically, we compute the 1-D Bezier
	// with control values:
	//
	//		Y[j-1], Y[j-1] + S[j-1]*A, Y[j]-S[j]*A, Y[j]

	return EvaluateSplineSegment (x,
								  X [j - 1],
								  Y [j - 1],
								  S [j - 1],
								  X [j    ],
								  Y [j    ],
								  S [j    ]);

	}

/*****************************************************************************/
