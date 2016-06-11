/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_spline.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_spline__
#define __dng_spline__

/*****************************************************************************/

#include "dng_1d_function.h"

#include <vector>

/*****************************************************************************/

inline real64 EvaluateSplineSegment (real64 x,
								     real64 x0,
								     real64 y0,
								     real64 s0,
								     real64 x1,
								     real64 y1,
								     real64 s1)
	{

	real64 A = x1 - x0;

	real64 B = (x - x0) / A;

	real64 C = (x1 - x) / A;

	real64 D = ((y0 * (2.0 - C + B) + (s0 * A * B)) * (C * C)) +
			   ((y1 * (2.0 - B + C) - (s1 * A * C)) * (B * B));

	return D;

	}

/*****************************************************************************/

class dng_spline_solver: public dng_1d_function
	{

	protected:

		std::vector<real64> X;
		std::vector<real64> Y;

		std::vector<real64> S;

	public:

		dng_spline_solver ();

		virtual ~dng_spline_solver ();

		void Reset ();

		void Add (real64 x, real64 y);

		void Solve ();

		virtual bool IsIdentity () const;

		virtual real64 Evaluate (real64 x) const;

	private:

		// Hidden copy constructor and assignment operator.

		dng_spline_solver (const dng_spline_solver &solver);

		dng_spline_solver & operator= (const dng_spline_solver &solver);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
