/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_string_list.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_string_list__
#define __dng_string_list__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_types.h"

/*****************************************************************************/

class dng_string_list
	{

	private:

		uint32 fCount;

		uint32 fAllocated;

		dng_string **fList;

	public:

		dng_string_list ();

		~dng_string_list ();

		uint32 Count () const
			{
			return fCount;
			}

		dng_string & operator[] (uint32 index)
			{
			return *(fList [index]);
			}

		const dng_string & operator[] (uint32 index) const
			{
			return *(fList [index]);
			}

		void Allocate (uint32 minSize);

		void Insert (uint32 index,
					 const dng_string &s);

		void Append (const dng_string &s)
			{
			Insert (Count (), s);
			}

		bool Contains (const dng_string &s) const;

		void Clear ();

	private:

		// Hidden copy constructor and assignment operator.

		dng_string_list (const dng_string_list &list);

		dng_string_list & operator= (const dng_string_list &list);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
