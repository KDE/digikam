/*****************************************************************************/
// Copyright 2008-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_opcode_list.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_opcode_list.h"

#include "dng_globals.h"
#include "dng_host.h"
#include "dng_memory_stream.h"
#include "dng_negative.h"
#include "dng_tag_values.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_opcode_list::dng_opcode_list (uint32 stage)

	:	fList        ()
	,	fAlwaysApply (false)
	,	fStage		 (stage)

	{

	}

/******************************************************************************/

dng_opcode_list::~dng_opcode_list ()
	{

	Clear ();

	}

/******************************************************************************/

void dng_opcode_list::Clear ()
	{

	for (size_t index = 0; index < fList.size (); index++)
		{

		if (fList [index])
			{

			delete fList [index];

			fList [index] = NULL;

			}

		}

	fList.clear ();

	fAlwaysApply = false;

	}

/******************************************************************************/

uint32 dng_opcode_list::MinVersion (bool includeOptional) const
	{

	uint32 result = dngVersion_None;

	for (size_t index = 0; index < fList.size (); index++)
		{

		if (includeOptional || !fList [index]->Optional ())
			{

			result = Max_uint32 (result, fList [index]->MinVersion ());

			}

		}

	return result;

	}

/*****************************************************************************/

void dng_opcode_list::Apply (dng_host &host,
							 dng_negative &negative,
							 AutoPtr<dng_image> &image)
	{

	for (uint32 index = 0; index < Count (); index++)
		{

		dng_opcode &opcode (Entry (index));

		if (opcode.AboutToApply (host, negative))
			{

			opcode.Apply (host,
						  negative,
						  image);

			}

		}

	}

/*****************************************************************************/

void dng_opcode_list::Append (AutoPtr<dng_opcode> &opcode)
	{

	if (opcode->OpcodeID () == dngOpcode_Private)
		{
		SetAlwaysApply ();
		}

	opcode->SetStage (fStage);

	fList.push_back (NULL);

	fList [fList.size () - 1] = opcode.Release ();

	}

/*****************************************************************************/

dng_memory_block * dng_opcode_list::Spool (dng_host &host) const
	{

	if (IsEmpty ())
		{
		return NULL;
		}

	if (AlwaysApply ())
		{
		ThrowProgramError ();
		}

	dng_memory_stream stream (host.Allocator ());

	stream.SetBigEndian ();

	stream.Put_uint32 ((uint32) fList.size ());

	for (size_t index = 0; index < fList.size (); index++)
		{

		stream.Put_uint32 (fList [index]->OpcodeID   ());
		stream.Put_uint32 (fList [index]->MinVersion ());
		stream.Put_uint32 (fList [index]->Flags      ());

		fList [index]->PutData (stream);

		}

	return stream.AsMemoryBlock (host.Allocator ());

	}

/*****************************************************************************/

void dng_opcode_list::FingerprintToStream (dng_stream &stream) const
	{

	if (IsEmpty ())
		{
		return;
		}

	stream.Put_uint32 ((uint32) fList.size ());

	for (size_t index = 0; index < fList.size (); index++)
		{

		stream.Put_uint32 (fList [index]->OpcodeID   ());
		stream.Put_uint32 (fList [index]->MinVersion ());
		stream.Put_uint32 (fList [index]->Flags      ());

		if (fList [index]->OpcodeID () != dngOpcode_Private)
			{

			fList [index]->PutData (stream);

			}

		}

	}

/*****************************************************************************/

void dng_opcode_list::Parse (dng_host &host,
							 dng_stream &stream,
							 uint32 byteCount,
							 uint64 streamOffset)
	{

	Clear ();

	TempBigEndian tempBigEndian (stream);

	stream.SetReadPosition (streamOffset);

	uint32 count = stream.Get_uint32 ();

	#if qDNGValidate

	if (gVerbose)
		{

		if (count == 1)
			{
			printf ("1 opcode\n");
			}

		else
			{
			printf ("%u opcodes\n", count);
			}

		}

	#endif

	for (uint32 index = 0; index < count; index++)
		{

		uint32 opcodeID = stream.Get_uint32 ();

		AutoPtr<dng_opcode> opcode (host.Make_dng_opcode (opcodeID,
														  stream));

		Append (opcode);

		}

	if (stream.Position () != streamOffset + byteCount)
		{

		ThrowBadFormat ("Error parsing opcode list");

		}

	}

/*****************************************************************************/
