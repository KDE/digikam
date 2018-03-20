/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_string_list.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_string_list.h"

#include "dng_bottlenecks.h"
#include "dng_exceptions.h"
#include "dng_string.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_string_list::dng_string_list ()

	:	fCount     (0)
	,	fAllocated (0)
	,	fList      (NULL)

	{

	}

/*****************************************************************************/

dng_string_list::~dng_string_list ()
	{

	Clear ();

	}

/*****************************************************************************/

void dng_string_list::Allocate (uint32 minSize)
	{

	if (fAllocated < minSize)
		{

		uint32 newSize = Max_uint32 (minSize, fAllocated * 2);

		dng_string **list = (dng_string **)
							malloc (newSize * sizeof (dng_string *));

		if (!list)
			{

			ThrowMemoryFull ();

			}

		if (fCount)
			{

			DoCopyBytes (fList, list, fCount * sizeof (dng_string *));

			}

		if (fList)
			{

			free (fList);

			}

		fList = list;

		fAllocated = newSize;

		}

	}

/*****************************************************************************/

void dng_string_list::Insert (uint32 index,
							  const dng_string &s)
	{

	Allocate (fCount + 1);

	dng_string *ss = new dng_string (s);

	if (!ss)
		{

		ThrowMemoryFull ();

		}

	fCount++;

	for (uint32 j = fCount - 1; j > index; j--)
		{

		fList [j] = fList [j - 1];

		}

	fList [index] = ss;

	}

/*****************************************************************************/

bool dng_string_list::Contains (const dng_string &s) const
	{

	for (uint32 j = 0; j < fCount; j++)
		{

		if ((*this) [j] == s)
			{

			return true;

			}

		}

	return false;

	}

/*****************************************************************************/

void dng_string_list::Clear ()
	{

	if (fList)
		{

		for (uint32 index = 0; index < fCount; index++)
			{

			delete fList [index];

			}

		free (fList);

		fList = NULL;

		}

	fCount     = 0;
	fAllocated = 0;

	}

/*****************************************************************************/
