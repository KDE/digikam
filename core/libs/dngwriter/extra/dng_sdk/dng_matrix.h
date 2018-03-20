/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_matrix.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Matrix and vector classes, including specialized 3x3 and 4x3 versions as
 * well as length 3 vectors.
 */

/*****************************************************************************/

#ifndef __dng_matrix__
#define __dng_matrix__

/*****************************************************************************/

#include "dng_sdk_limits.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_matrix
	{

	protected:

		uint32 fRows;
		uint32 fCols;

		real64 fData [kMaxColorPlanes] [kMaxColorPlanes];

	public:

		dng_matrix ();

		dng_matrix (uint32 rows,
					uint32 cols);

		dng_matrix (const dng_matrix &m);

		virtual ~dng_matrix ()
			{
			}

		void Clear ();

		void SetIdentity (uint32 count);

		uint32 Rows () const
			{
			return fRows;
			}

		uint32 Cols () const
			{
			return fCols;
			}

		real64 * operator [] (uint32 row)
			{
			return fData [row];
			}

		const real64 * operator [] (uint32 row) const
			{
			return fData [row];
			}

		bool operator== (const dng_matrix &m) const;

		bool operator!= (const dng_matrix &m) const
			{
			return !(*this == m);
			}

		bool IsEmpty () const
			{
			return fRows == 0 || fCols == 0;
			}

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		bool IsDiagonal () const;

		real64 MaxEntry () const;

		real64 MinEntry () const;

		void Scale (real64 factor);

		void Round (real64 factor);

		void SafeRound (real64 factor);

	};

/*****************************************************************************/

class dng_matrix_3by3: public dng_matrix
	{

	public:

		dng_matrix_3by3 ();

		dng_matrix_3by3 (const dng_matrix &m);

		dng_matrix_3by3 (real64 a00, real64 a01, real64 a02,
				         real64 a10, real64 a11, real64 a12,
				         real64 a20, real64 a21, real64 a22);

		dng_matrix_3by3 (real64 a00, real64 a11, real64 a22);

	};

/*****************************************************************************/

class dng_matrix_4by3: public dng_matrix
	{

	public:

		dng_matrix_4by3 ();

		dng_matrix_4by3 (const dng_matrix &m);

		dng_matrix_4by3 (real64 a00, real64 a01, real64 a02,
				         real64 a10, real64 a11, real64 a12,
				         real64 a20, real64 a21, real64 a22,
				         real64 a30, real64 a31, real64 a32);

	};

/*****************************************************************************/

class dng_vector
	{

	protected:

		uint32 fCount;

		real64 fData [kMaxColorPlanes];

	public:

		dng_vector ();

		dng_vector (uint32 count);

		dng_vector (const dng_vector &v);

		virtual ~dng_vector ()
			{
			}

		void Clear ();

		void SetIdentity (uint32 count);

		uint32 Count () const
			{
			return fCount;
			}

		real64 & operator [] (uint32 index)
			{
			return fData [index];
			}

		const real64 & operator [] (uint32 index) const
			{
			return fData [index];
			}

		bool operator== (const dng_vector &v) const;

		bool operator!= (const dng_vector &v) const
			{
			return !(*this == v);
			}

		bool IsEmpty () const
			{
			return fCount == 0;
			}

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		real64 MaxEntry () const;

		real64 MinEntry () const;

		void Scale (real64 factor);

		void Round (real64 factor);

		dng_matrix AsDiagonal () const;

		dng_matrix AsColumn () const;

	};

/*****************************************************************************/

class dng_vector_3: public dng_vector
	{

	public:

		dng_vector_3 ();

		dng_vector_3 (const dng_vector &v);

		dng_vector_3 (real64 a0,
					  real64 a1,
					  real64 a2);

	};

/*****************************************************************************/

dng_matrix operator* (const dng_matrix &A,
					  const dng_matrix &B);

dng_vector operator* (const dng_matrix &A,
					  const dng_vector &B);

dng_matrix operator* (real64 scale,
					  const dng_matrix &A);

dng_vector operator* (real64 scale,
					  const dng_vector &A);

/*****************************************************************************/

dng_matrix operator+ (const dng_matrix &A,
					  const dng_matrix &B);

/*****************************************************************************/

dng_matrix Transpose (const dng_matrix &A);

/*****************************************************************************/

dng_matrix Invert (const dng_matrix &A);

dng_matrix Invert (const dng_matrix &A,
				   const dng_matrix &hint);

/*****************************************************************************/

inline real64 MaxEntry (const dng_matrix &A)
	{
	return A.MaxEntry ();
	}

inline real64 MaxEntry (const dng_vector &A)
	{
	return A.MaxEntry ();
	}

/*****************************************************************************/

inline real64 MinEntry (const dng_matrix &A)
	{
	return A.MinEntry ();
	}

inline real64 MinEntry (const dng_vector &A)
	{
	return A.MinEntry ();
	}

/*****************************************************************************/

#endif

/*****************************************************************************/
