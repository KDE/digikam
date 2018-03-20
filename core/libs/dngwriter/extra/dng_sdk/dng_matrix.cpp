/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_matrix.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_matrix.h"

#include "dng_exceptions.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_matrix::dng_matrix ()

	:	fRows (0)
	,	fCols (0)

	{

	}

/*****************************************************************************/

dng_matrix::dng_matrix (uint32 rows,
						uint32 cols)

	:	fRows (0)
	,	fCols (0)

	{

	if (rows < 1 || rows > kMaxColorPlanes ||
		cols < 1 || cols > kMaxColorPlanes)
		{

		ThrowProgramError ();

		}

	fRows = rows;
	fCols = cols;

	for (uint32 row = 0; row < fRows; row++)
		for (uint32 col = 0; col < fCols; col++)
			{

			fData [row] [col] = 0.0;

			}

	}

/*****************************************************************************/

dng_matrix::dng_matrix (const dng_matrix &m)

	:	fRows (m.fRows)
	,	fCols (m.fCols)

	{

	for (uint32 row = 0; row < fRows; row++)
		for (uint32 col = 0; col < fCols; col++)
			{

			fData [row] [col] = m.fData [row] [col];

			}

	}

/*****************************************************************************/

void dng_matrix::Clear ()
	{

	fRows = 0;
	fCols = 0;

	}

/*****************************************************************************/

void dng_matrix::SetIdentity (uint32 count)
	{

	*this = dng_matrix (count, count);

	for (uint32 j = 0; j < count; j++)
		{

		fData [j] [j] = 1.0;

		}

	}

/******************************************************************************/

bool dng_matrix::operator== (const dng_matrix &m) const
	{

	if (Rows () != m.Rows () ||
	    Cols () != m.Cols ())
		{

		return false;

		}

	for (uint32 j = 0; j < Rows (); j++)
		for (uint32 k = 0; k < Cols (); k++)
			{

			if (fData [j] [k] != m.fData [j] [k])
				{

				return false;

				}

			}

	return true;

	}

/******************************************************************************/

bool dng_matrix::IsDiagonal () const
	{

	if (IsEmpty ())
		{
		return false;
		}

	if (Rows () != Cols ())
		{
		return false;
		}

	for (uint32 j = 0; j < Rows (); j++)
		for (uint32 k = 0; k < Cols (); k++)
			{

			if (j != k)
				{

				if (fData [j] [k] != 0.0)
					{
					return false;
					}

				}

			}

	return true;

	}

/******************************************************************************/

real64 dng_matrix::MaxEntry () const
	{

	if (IsEmpty ())
		{

		return 0.0;

		}

	real64 m = fData [0] [0];

	for (uint32 j = 0; j < Rows (); j++)
		for (uint32 k = 0; k < Cols (); k++)
			{

			m = Max_real64 (m, fData [j] [k]);

			}

	return m;

	}

/******************************************************************************/

real64 dng_matrix::MinEntry () const
	{

	if (IsEmpty ())
		{

		return 0.0;

		}

	real64 m = fData [0] [0];

	for (uint32 j = 0; j < Rows (); j++)
		for (uint32 k = 0; k < Cols (); k++)
			{

			m = Min_real64 (m, fData [j] [k]);

			}

	return m;

	}

/*****************************************************************************/

void dng_matrix::Scale (real64 factor)
	{

	for (uint32 j = 0; j < Rows (); j++)
		for (uint32 k = 0; k < Cols (); k++)
			{

			fData [j] [k] *= factor;

			}

	}

/*****************************************************************************/

void dng_matrix::Round (real64 factor)
	{

	real64 invFactor = 1.0 / factor;

	for (uint32 j = 0; j < Rows (); j++)
		for (uint32 k = 0; k < Cols (); k++)
			{

			fData [j] [k] = Round_int32 (fData [j] [k] * factor) * invFactor;

			}

	}

/*****************************************************************************/

void dng_matrix::SafeRound (real64 factor)
	{

	real64 invFactor = 1.0 / factor;

	for (uint32 j = 0; j < Rows (); j++)
		{

		// Round each row to the specified accuracy, but make sure the
		// a rounding does not affect the total of the elements in a row
		// more than necessary.

		real64 error = 0.0;

		for (uint32 k = 0; k < Cols (); k++)
			{

			fData [j] [k] += error;

			real64 rounded = Round_int32 (fData [j] [k] * factor) * invFactor;

			error = fData [j] [k] - rounded;

			fData [j] [k] = rounded;

			}

		}

	}

/*****************************************************************************/

dng_matrix_3by3::dng_matrix_3by3 ()

	:	dng_matrix (3, 3)

	{
	}

/*****************************************************************************/

dng_matrix_3by3::dng_matrix_3by3 (const dng_matrix &m)

	:	dng_matrix (m)

	{

	if (Rows () != 3 ||
		Cols () != 3)
		{

		ThrowMatrixMath ();

		}

	}

/*****************************************************************************/

dng_matrix_3by3::dng_matrix_3by3 (real64 a00, real64 a01, real64 a02,
				        		  real64 a10, real64 a11, real64 a12,
				        		  real64 a20, real64 a21, real64 a22)


	:	dng_matrix (3, 3)

	{

	fData [0] [0] = a00;
	fData [0] [1] = a01;
	fData [0] [2] = a02;

	fData [1] [0] = a10;
	fData [1] [1] = a11;
	fData [1] [2] = a12;

	fData [2] [0] = a20;
	fData [2] [1] = a21;
	fData [2] [2] = a22;

	}

/*****************************************************************************/

dng_matrix_3by3::dng_matrix_3by3 (real64 a00, real64 a11, real64 a22)

	:	dng_matrix (3, 3)

	{

	fData [0] [0] = a00;
	fData [1] [1] = a11;
	fData [2] [2] = a22;

	}

/*****************************************************************************/

dng_matrix_4by3::dng_matrix_4by3 ()

	:	dng_matrix (4, 3)

	{
	}

/*****************************************************************************/

dng_matrix_4by3::dng_matrix_4by3 (const dng_matrix &m)

	:	dng_matrix (m)

	{

	if (Rows () != 4 ||
		Cols () != 3)
		{

		ThrowMatrixMath ();

		}

	}

/*****************************************************************************/

dng_matrix_4by3::dng_matrix_4by3 (real64 a00, real64 a01, real64 a02,
				       			  real64 a10, real64 a11, real64 a12,
				        		  real64 a20, real64 a21, real64 a22,
				        		  real64 a30, real64 a31, real64 a32)


	:	dng_matrix (4, 3)

	{

	fData [0] [0] = a00;
	fData [0] [1] = a01;
	fData [0] [2] = a02;

	fData [1] [0] = a10;
	fData [1] [1] = a11;
	fData [1] [2] = a12;

	fData [2] [0] = a20;
	fData [2] [1] = a21;
	fData [2] [2] = a22;

	fData [3] [0] = a30;
	fData [3] [1] = a31;
	fData [3] [2] = a32;

	}

/*****************************************************************************/

dng_vector::dng_vector ()

	:	fCount (0)

	{

	}

/*****************************************************************************/

dng_vector::dng_vector (uint32 count)

	:	fCount (0)

	{

	if (count < 1 || count > kMaxColorPlanes)
		{

		ThrowProgramError ();

		}

	fCount = count;

	for (uint32 index = 0; index < fCount; index++)
		{

		fData [index] = 0.0;

		}

	}

/*****************************************************************************/

dng_vector::dng_vector (const dng_vector &v)

	:	fCount (v.fCount)

	{

	for (uint32 index = 0; index < fCount; index++)
		{

		fData [index] = v.fData [index];

		}

	}

/*****************************************************************************/

void dng_vector::Clear ()
	{

	fCount = 0;

	}

/*****************************************************************************/

void dng_vector::SetIdentity (uint32 count)
	{

	*this = dng_vector (count);

	for (uint32 j = 0; j < count; j++)
		{

		fData [j] = 1.0;

		}

	}

/******************************************************************************/

bool dng_vector::operator== (const dng_vector &v) const
	{

	if (Count () != v.Count ())
		{

		return false;

		}

	for (uint32 j = 0; j < Count (); j++)
		{

		if (fData [j] != v.fData [j])
			{

			return false;

			}

		}

	return true;

	}

/******************************************************************************/

real64 dng_vector::MaxEntry () const
	{

	if (IsEmpty ())
		{

		return 0.0;

		}

	real64 m = fData [0];

	for (uint32 j = 0; j < Count (); j++)
		{

		m = Max_real64 (m, fData [j]);

		}

	return m;

	}

/******************************************************************************/

real64 dng_vector::MinEntry () const
	{

	if (IsEmpty ())
		{

		return 0.0;

		}

	real64 m = fData [0];

	for (uint32 j = 0; j < Count (); j++)
		{

		m = Min_real64 (m, fData [j]);

		}

	return m;

	}

/*****************************************************************************/

void dng_vector::Scale (real64 factor)
	{

	for (uint32 j = 0; j < Count (); j++)
		{

		fData [j] *= factor;

		}

	}

/*****************************************************************************/

void dng_vector::Round (real64 factor)
	{

	real64 invFactor = 1.0 / factor;

	for (uint32 j = 0; j < Count (); j++)
		{

		fData [j] = Round_int32 (fData [j] * factor) * invFactor;

		}

	}

/*****************************************************************************/

dng_matrix dng_vector::AsDiagonal () const
	{

	dng_matrix M (Count (), Count ());

	for (uint32 j = 0; j < Count (); j++)
		{

		M [j] [j] = fData [j];

		}

	return M;

	}

/*****************************************************************************/

dng_matrix dng_vector::AsColumn () const
	{

	dng_matrix M (Count (), 1);

	for (uint32 j = 0; j < Count (); j++)
		{

		M [j] [0] = fData [j];

		}

	return M;

	}

/******************************************************************************/

dng_vector_3::dng_vector_3 ()

	:	dng_vector (3)

	{
	}

/******************************************************************************/

dng_vector_3::dng_vector_3 (const dng_vector &v)

	:	dng_vector (v)

	{

	if (Count () != 3)
		{

		ThrowMatrixMath ();

		}

	}

/******************************************************************************/

dng_vector_3::dng_vector_3 (real64 a0,
							real64 a1,
						    real64 a2)

	:	dng_vector (3)

	{

	fData [0] = a0;
	fData [1] = a1;
	fData [2] = a2;

	}

/******************************************************************************/

dng_matrix operator* (const dng_matrix &A,
					  const dng_matrix &B)
	{

	if (A.Cols () != B.Rows ())
		{

		ThrowMatrixMath ();

		}

	dng_matrix C (A.Rows (), B.Cols ());

	for (uint32 j = 0; j < C.Rows (); j++)
		for (uint32 k = 0; k < C.Cols (); k++)
			{

			C [j] [k] = 0.0;

			for (uint32 m = 0; m < A.Cols (); m++)
				{

				real64 aa = A [j] [m];

				real64 bb = B [m] [k];

				C [j] [k] += aa * bb;

				}

			}

	return C;

	}

/******************************************************************************/

dng_vector operator* (const dng_matrix &A,
					  const dng_vector &B)
	{

	if (A.Cols () != B.Count ())
		{

		ThrowMatrixMath ();

		}

	dng_vector C (A.Rows ());

	for (uint32 j = 0; j < C.Count (); j++)
		{

		C [j] = 0.0;

		for (uint32 m = 0; m < A.Cols (); m++)
			{

			real64 aa = A [j] [m];

			real64 bb = B [m];

			C [j] += aa * bb;

			}

		}

	return C;

	}

/******************************************************************************/

dng_matrix operator* (real64 scale,
					  const dng_matrix &A)
	{

	dng_matrix B (A);

	B.Scale (scale);

	return B;

	}

/******************************************************************************/

dng_vector operator* (real64 scale,
					  const dng_vector &A)
	{

	dng_vector B (A);

	B.Scale (scale);

	return B;

	}

/******************************************************************************/

dng_matrix operator+ (const dng_matrix &A,
					  const dng_matrix &B)
	{

	if (A.Cols () != B.Cols () ||
		A.Rows () != B.Rows ())
		{

		ThrowMatrixMath ();

		}

	dng_matrix C (A);

	for (uint32 j = 0; j < C.Rows (); j++)
		for (uint32 k = 0; k < C.Cols (); k++)
			{

			C [j] [k] += B [j] [k];

			}

	return C;

	}

/******************************************************************************/

const real64 kNearZero = 1.0E-10;

/******************************************************************************/

// Work around bug #1294195, which may be a hardware problem on a specific machine.
// This pragma turns on "improved" floating-point consistency.
#ifdef _MSC_VER
#pragma optimize ("p", on)
#endif

static dng_matrix Invert3by3 (const dng_matrix &A)
	{

	real64 a00 = A [0] [0];
	real64 a01 = A [0] [1];
	real64 a02 = A [0] [2];
	real64 a10 = A [1] [0];
	real64 a11 = A [1] [1];
	real64 a12 = A [1] [2];
	real64 a20 = A [2] [0];
	real64 a21 = A [2] [1];
	real64 a22 = A [2] [2];

	real64 temp [3] [3];

	temp [0] [0] = a11 * a22 - a21 * a12;
	temp [0] [1] = a21 * a02 - a01 * a22;
	temp [0] [2] = a01 * a12 - a11 * a02;
	temp [1] [0] = a20 * a12 - a10 * a22;
	temp [1] [1] = a00 * a22 - a20 * a02;
	temp [1] [2] = a10 * a02 - a00 * a12;
	temp [2] [0] = a10 * a21 - a20 * a11;
	temp [2] [1] = a20 * a01 - a00 * a21;
	temp [2] [2] = a00 * a11 - a10 * a01;

	real64 det = (a00 * temp [0] [0] +
				  a01 * temp [1] [0] +
				  a02 * temp [2] [0]);

	if (Abs_real64 (det) < kNearZero)
		{

		ThrowMatrixMath ();

		}

	dng_matrix B (3, 3);

	for (uint32 j = 0; j < 3; j++)
		for (uint32 k = 0; k < 3; k++)
			{

			B [j] [k] = temp [j] [k] / det;

			}

	return B;

	}

// Reset floating-point optimization. See comment above.
#ifdef _MSC_VER
#pragma optimize ("p", off)
#endif

/******************************************************************************/

static dng_matrix InvertNbyN (const dng_matrix &A)
	{

	uint32 i;
	uint32 j;
	uint32 k;

	uint32 n = A.Rows ();

	real64 temp [kMaxColorPlanes] [kMaxColorPlanes * 2];

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			{

			temp [i] [j    ] = A [i] [j];

			temp [i] [j + n] = (i == j ? 1.0 : 0.0);

			}

	for (i = 0; i < n; i++)
		{

		real64 alpha = temp [i] [i];

		if (Abs_real64 (alpha) < kNearZero)
			{

			ThrowMatrixMath ();

			}

		for (j = 0; j < n * 2; j++)
			{

			temp [i] [j] /= alpha;

			}

		for (k = 0; k < n; k++)
			{

			if (i != k)
				{

				real64 beta = temp [k] [i];

				for (j = 0; j < n * 2; j++)
					{

					temp [k] [j] -= beta * temp [i] [j];

					}

				}

			}

		}

	dng_matrix B (n, n);

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			{

			B [i] [j] = temp [i] [j + n];

			}

	return B;

	}

/******************************************************************************/

dng_matrix Transpose (const dng_matrix &A)
	{

	dng_matrix B (A.Cols (), A.Rows ());

	for (uint32 j = 0; j < B.Rows (); j++)
		for (uint32 k = 0; k < B.Cols (); k++)
			{

			B [j] [k] = A [k] [j];

			}

	return B;

	}

/******************************************************************************/

dng_matrix Invert (const dng_matrix &A)
	{

	if (A.Rows () < 2 || A.Cols () < 2)
		{

		ThrowMatrixMath ();

		}

	if (A.Rows () == A.Cols ())
		{

		if (A.Rows () == 3)
			{

			return Invert3by3 (A);

			}

		return InvertNbyN (A);

		}

	else
		{

		// Compute the pseudo inverse.

		dng_matrix B = Transpose (A);

		return Invert (B * A) * B;

		}

	}

/******************************************************************************/

dng_matrix Invert (const dng_matrix &A,
				   const dng_matrix &hint)
	{

	if (A.Rows () == A   .Cols () ||
		A.Rows () != hint.Cols () ||
		A.Cols () != hint.Rows ())
		{

		return Invert (A);

		}

	else
		{

		// Use the specified hint matrix.

		return Invert (hint * A) * hint;

		}

	}

/*****************************************************************************/
