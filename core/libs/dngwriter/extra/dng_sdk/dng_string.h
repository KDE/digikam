/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_string.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_string__
#define __dng_string__

/*****************************************************************************/

#include "dng_types.h"
#include "dng_memory.h"

/*****************************************************************************/

class dng_string
	{

	private:

		// Always stored internally as a UTF-8 encoded string.

		dng_memory_data fData;

	public:

		dng_string ();

		dng_string (const dng_string &s);

		dng_string & operator= (const dng_string &s);

		~dng_string ();

		const char * Get () const;

		bool IsASCII () const;

		void Set (const char *s);

		void Set_ASCII (const char *s);

		void Set_UTF8 (const char *s);

		uint32 Get_SystemEncoding (dng_memory_data &buffer) const;

		void Set_SystemEncoding (const char *s);

		bool ValidSystemEncoding () const;

		void Set_JIS_X208_1990 (const char *s);

		static uint32 DecodeUTF8 (const char *&s,
								  uint32 maxBytes = 6);

		uint32 Get_UTF16 (dng_memory_data &buffer) const;

		void Set_UTF16 (const uint16 *s);

		void Clear ();

		void Truncate (uint32 maxBytes);

		bool TrimTrailingBlanks ();

		bool TrimLeadingBlanks ();

		bool IsEmpty () const;

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		uint32 Length () const;

		bool operator== (const dng_string &s) const;

		bool operator!= (const dng_string &s) const
			{
			return !(*this == s);
			}

		// A utility for doing case insensitive comparisons on strings...

		static bool Matches (const char *t,
							 const char *s,
							 bool case_sensitive = false);

		// ...wrapped up for use with dng_string.

		bool Matches (const char *s,
					  bool case_sensitive = false) const;

		bool StartsWith (const char *s,
						 bool case_sensitive = false) const;

		bool EndsWith (const char *s,
					   bool case_sensitive = false) const;

		bool Contains (const char *s,
					   bool case_sensitive = false,
					   int32 *match_offset = NULL) const;

		bool Replace (const char *old_string,
					  const char *new_string,
					  bool case_sensitive = true);

		bool TrimLeading (const char *s,
						  bool case_sensitive = false);

		void Append (const char *s);

		void SetUppercase ();

		void SetLowercase ();

		void SetLineEndings (char ending);

		void SetLineEndingsToNewLines ()
			{
			SetLineEndings ('\n');
			}

		void SetLineEndingsToReturns ()
			{
			SetLineEndings ('\r');
			}

		void StripLowASCII ();

		void ForceASCII ();

		int32 Compare (const dng_string &s) const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
