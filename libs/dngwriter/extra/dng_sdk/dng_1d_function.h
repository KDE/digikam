/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_1d_function.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Classes for a 1D floating-point to floating-point function abstraction.
 */

/*****************************************************************************/

#ifndef __dng_1d_function__
#define __dng_1d_function__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"

/*****************************************************************************/

/// \brief A 1D floating-point function.
///
/// The domain (input) is always from 0.0 to 1.0, while the range (output) can be an arbitrary interval.

class dng_1d_function
	{

	public:

		virtual ~dng_1d_function ();

		/// Returns true if this function is the map x -> y such that x == y for all x . That is if Evaluate(x) == x for all x.

		virtual bool IsIdentity () const;

		/// Return the mapping for value x.
		/// This method must be implemented by a derived class of dng_1d_function and the derived class determines the
		/// lookup method and function used.
		/// \param x A value between 0.0 and 1.0 (inclusive).
		/// \retval Mapped value for x

		virtual real64 Evaluate (real64 x) const = 0;

		/// Return the reverse mapped value for y.
		/// This method can be implemented by derived classes. The default implementation uses Newton's method to solve
		/// for x such that Evaluate(x) == y.
		/// \param y A value to reverse map. Should be within the range of the function implemented by this dng_1d_function .
		/// \retval A value x such that Evaluate(x) == y (to very close approximation).

		virtual real64 EvaluateInverse (real64 y) const;

	};

/*****************************************************************************/

/// An identity (x -> y such that x == y for all x) mapping function.

class dng_1d_identity: public dng_1d_function
	{

	public:
		/// Always returns true for this class.

		virtual bool IsIdentity () const;

		/// Always returns x for this class.

		virtual real64 Evaluate (real64 x) const;

		/// Always returns y for this class.

		virtual real64 EvaluateInverse (real64 y) const;

		/// This class is a singleton, and is entirely threadsafe. Use this method to get an instance of the class.

		static const dng_1d_function & Get ();

	};

/*****************************************************************************/

/// A dng_1d_function that represents the composition (curry) of two other dng_1d_functions.

class dng_1d_concatenate: public dng_1d_function
	{

	protected:

		const dng_1d_function &fFunction1;

		const dng_1d_function &fFunction2;

	public:

		/// Create a dng_1d_function which computes y = function2.Evaluate(function1.Evaluate(x)).
		/// Compose function1 and function2 to compute y = function2.Evaluate(function1.Evaluate(x)). The range of function1.Evaluate must be a subset of 0.0 to 1.0 inclusive,
		/// otherwise the result of function1(x) will be pinned (clipped) to 0.0 if <0.0 and to 1.0 if > 1.0 .
		/// \param function1 Inner function of composition.
		/// \param function2 Outer function of composition.

		dng_1d_concatenate (const dng_1d_function &function1,
							const dng_1d_function &function2);

		/// Only true if both function1 and function2 have IsIdentity equal to true.

		virtual bool IsIdentity () const;

		/// Return the composed mapping for value x.
		/// \param x A value between 0.0 and 1.0 (inclusive).
		/// \retval function2.Evaluate(function1.Evaluate(x)).

		virtual real64 Evaluate (real64 x) const;

		/// Return the reverse mapped value for y.
		/// Be careful using this method with compositions where the inner function does not have a range 0.0 to 1.0 . (Or better yet, do not use such functions.)
		/// \param y A value to reverse map. Should be within the range of function2.Evaluate.
		/// \retval A value x such that function2.Evaluate(function1.Evaluate(x)) == y (to very close approximation).

		virtual real64 EvaluateInverse (real64 y) const;

	};

/*****************************************************************************/

/// A dng_1d_function that represents the inverse of another dng_1d_function.

class dng_1d_inverse: public dng_1d_function
	{

	protected:

		const dng_1d_function &fFunction;

	public:

		dng_1d_inverse (const dng_1d_function &f);

		virtual bool IsIdentity () const;

		virtual real64 Evaluate (real64 x) const;

		virtual real64 EvaluateInverse (real64 y) const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
