/*****************************************************************************/
// Copyright 2008-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_opcode_list.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#ifndef __dng_opcode_list__
#define __dng_opcode_list__

/*****************************************************************************/

#include "dng_auto_ptr.h"
#include "dng_classes.h"
#include "dng_opcodes.h"

#include <vector>

/*****************************************************************************/

class dng_opcode_list
	{

	private:

		std::vector<dng_opcode *> fList;

		bool fAlwaysApply;

		uint32 fStage;

	public:

		dng_opcode_list (uint32 stage);

		~dng_opcode_list ();

		bool IsEmpty () const
			{
			return fList.size () == 0;
			}

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		bool AlwaysApply () const
			{
			return fAlwaysApply && NotEmpty ();
			}

		void SetAlwaysApply ()
			{
			fAlwaysApply = true;
			}

		uint32 Count () const
			{
			return (uint32) fList.size ();
			}

		dng_opcode & Entry (uint32 index)
			{
			return *fList [index];
			}

		const dng_opcode & Entry (uint32 index) const
			{
			return *fList [index];
			}

		void Clear ();

		uint32 MinVersion (bool includeOptional) const;

		void Apply (dng_host &host,
					dng_negative &negative,
					AutoPtr<dng_image> &image);

		void Append (AutoPtr<dng_opcode> &opcode);

		dng_memory_block * Spool (dng_host &host) const;

		void FingerprintToStream (dng_stream &stream) const;

		void Parse (dng_host &host,
					dng_stream &stream,
					uint32 byteCount,
					uint64 streamOffset);

	private:

		// Hidden copy constructor and assignment operator.

		dng_opcode_list (const dng_opcode_list &list);

		dng_opcode_list & operator= (const dng_opcode_list &list);

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
