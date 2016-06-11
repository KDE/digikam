/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_1d_function.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_1d_function.h"

#include "dng_utils.h"

/*****************************************************************************/

dng_1d_function::~dng_1d_function ()
	{

	}

/*****************************************************************************/

bool dng_1d_function::IsIdentity () const
	{

	return false;

	}

/*****************************************************************************/

real64 dng_1d_function::EvaluateInverse (real64 y) const
	{

	const uint32 kMaxIterations = 30;
	const real64 kNearZero      = 1.0e-10;

	real64 x0 = 0.0;
	real64 y0 = Evaluate (x0);

	real64 x1 = 1.0;
	real64 y1 = Evaluate (x1);

	for (uint32 iteration = 0; iteration < kMaxIterations; iteration++)
		{

		if (Abs_real64 (y1 - y0) < kNearZero)
			{
			break;
			}

		real64 x2 = Pin_real64 (0.0,
								x1 + (y - y1) * (x1 - x0) / (y1 - y0),
								1.0);

		real64 y2 = Evaluate (x2);

		x0 = x1;
		y0 = y1;

		x1 = x2;
		y1 = y2;

		}

	return x1;

	}

/*****************************************************************************/

bool dng_1d_identity::IsIdentity () const
	{

	return true;

	}

/*****************************************************************************/

real64 dng_1d_identity::Evaluate (real64 x) const
	{

	return x;

	}

/*****************************************************************************/

real64 dng_1d_identity::EvaluateInverse (real64 x) const
	{

	return x;

	}

/*****************************************************************************/

const dng_1d_function & dng_1d_identity::Get ()
	{

	static dng_1d_identity static_function;

	return static_function;

	}

/*****************************************************************************/

dng_1d_concatenate::dng_1d_concatenate (const dng_1d_function &function1,
										const dng_1d_function &function2)

	:	fFunction1 (function1)
	,	fFunction2 (function2)

	{

	}

/*****************************************************************************/

bool dng_1d_concatenate::IsIdentity () const
	{

	return fFunction1.IsIdentity () &&
		   fFunction2.IsIdentity ();

	}

/*****************************************************************************/

real64 dng_1d_concatenate::Evaluate (real64 x) const
	{

	real64 y = Pin_real64 (0.0, fFunction1.Evaluate (x), 1.0);

	return fFunction2.Evaluate (y);

	}

/*****************************************************************************/

real64 dng_1d_concatenate::EvaluateInverse (real64 x) const
	{

	real64 y = fFunction2.EvaluateInverse (x);

	return fFunction1.EvaluateInverse (y);

	}

/*****************************************************************************/

dng_1d_inverse::dng_1d_inverse (const dng_1d_function &f)

	:	fFunction (f)

	{

	}

/*****************************************************************************/

bool dng_1d_inverse::IsIdentity () const
	{

	return fFunction.IsIdentity ();

	}

/*****************************************************************************/

real64 dng_1d_inverse::Evaluate (real64 x) const
	{

	return fFunction.EvaluateInverse (x);

	}

/*****************************************************************************/

real64 dng_1d_inverse::EvaluateInverse (real64 y) const
	{

	return fFunction.Evaluate (y);

	}

/*****************************************************************************/
