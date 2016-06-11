/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_info.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_info.h"

#include "dng_camera_profile.h"
#include "dng_exceptions.h"
#include "dng_globals.h"
#include "dng_host.h"
#include "dng_tag_codes.h"
#include "dng_parse_utils.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_info::dng_info ()

	:	fTIFFBlockOffset         (0)
	,	fTIFFBlockOriginalOffset (kDNGStreamInvalidOffset)
	,	fBigEndian				 (false)
	,	fMagic					 (0)
	,	fExif					 ()
	,	fShared					 ()
	,	fMainIndex				 (-1)
	,	fIFDCount				 (0)
	,	fChainedIFDCount		 (0)
	,	fMakerNoteNextIFD		 (0)

	{

	}

/*****************************************************************************/

dng_info::~dng_info ()
	{

	}

/*****************************************************************************/

void dng_info::ValidateMagic ()
	{

	switch (fMagic)
		{

		case magicTIFF:
		case magicExtendedProfile:
		case magicPanasonic:
		case magicOlympusA:
		case magicOlympusB:
			{

			return;

			}

		default:
			{

			#if qDNGValidate

			ReportError ("Invalid TIFF magic number");

			#endif

			ThrowBadFormat ();
			break;
			}

		}

	}

/*****************************************************************************/

void dng_info::ParseTag (dng_host &host,
						 dng_stream &stream,
						 dng_exif *exif,
						 dng_shared *shared,
						 dng_ifd *ifd,
						 uint32 parentCode,
						 uint32 tagCode,
						 uint32 tagType,
						 uint32 tagCount,
						 uint64 tagOffset,
						 int64 offsetDelta)
	{

	bool isSubIFD = parentCode >= tcFirstSubIFD &&
					parentCode <= tcLastSubIFD;

	bool isMainIFD = (parentCode == 0 || isSubIFD) &&
					 ifd &&
					 ifd->fUsesNewSubFileType &&
			 		 ifd->fNewSubFileType == sfMainImage;

	// Panasonic RAW format stores private tags using tag codes < 254 in
	// IFD 0.  Redirect the parsing of these tags into a logical
	// "PanasonicRAW" IFD.

	// Panasonic is starting to use some higher numbers also (280..283).

	if (fMagic == 85 && parentCode == 0 && (tagCode < tcNewSubFileType ||
											(tagCode >= 280 && tagCode <= 283)))
		{

		parentCode = tcPanasonicRAW;

		ifd = NULL;

		}

	stream.SetReadPosition (tagOffset);

	if (ifd && ifd->ParseTag (stream,
						 	  parentCode,
						 	  tagCode,
						 	  tagType,
						 	  tagCount,
						 	  tagOffset))
		{

		return;

		}

	stream.SetReadPosition (tagOffset);

	if (exif && shared && exif->ParseTag (stream,
										  *shared,
										  parentCode,
										  isMainIFD,
										  tagCode,
										  tagType,
										  tagCount,
										  tagOffset))
		{

		return;

		}

	stream.SetReadPosition (tagOffset);

	if (shared && exif && shared->ParseTag (stream,
											*exif,
								    		parentCode,
								    		isMainIFD,
								    		tagCode,
								    		tagType,
								    		tagCount,
								    		tagOffset,
								    		offsetDelta))
		{

		return;

		}

	if (parentCode == tcOlympusMakerNote &&
		tagType == ttUndefined &&
		tagCount >= 14)
		{

		uint32 olympusMakerParent = 0;

		switch (tagCode)
			{

			case 8208:
				olympusMakerParent = tcOlympusMakerNote8208;
				break;

			case 8224:
				olympusMakerParent = tcOlympusMakerNote8224;
				break;

			case 8240:
				olympusMakerParent = tcOlympusMakerNote8240;
				break;

			case 8256:
				olympusMakerParent = tcOlympusMakerNote8256;
				break;

			case 8272:
				olympusMakerParent = tcOlympusMakerNote8272;
				break;

			case 12288:
				olympusMakerParent = tcOlympusMakerNote12288;
				break;

			default:
				break;

			}

		if (olympusMakerParent)
			{

			// Olympus made a mistake in some camera models in computing
			// the size of these sub-tags, so we fudge the count.

			if (ParseMakerNoteIFD (host,
								   stream,
							       stream.Length () - tagOffset,
				   	  		       tagOffset,
				   	  		       offsetDelta,
				   	  		       tagOffset,
				   	  		       stream.Length (),
				   	  		       olympusMakerParent))
				{

				return;

				}

			}

		}

	if (parentCode == tcRicohMakerNote &&
		tagCode == 0x2001 &&
		tagType == ttUndefined &&
		tagCount > 22)
		{

		char header [20];

		stream.SetReadPosition (tagOffset);

		stream.Get (header, sizeof (header));

		if (memcmp (header, "[Ricoh Camera Info]", 19) == 0)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   tagCount - 20,
				   	  		   tagOffset + 20,
				   	  		   offsetDelta,
				   	  		   tagOffset + 20,
				   	  		   tagOffset + tagCount,
				   	  		   tcRicohMakerNoteCameraInfo);

			return;

			}

		}

	#if qDNGValidate

		{

		stream.SetReadPosition (tagOffset);

		if (gVerbose)
			{

			printf ("*");

			DumpTagValues (stream,
						   LookupTagType (tagType),
						   parentCode,
						   tagCode,
						   tagType,
						   tagCount);

			}

		// If type is ASCII, then parse anyway so we report any ASCII
		// NULL termination or character set errors.

		else if (tagType == ttAscii)
			{

			dng_string s;

			ParseStringTag (stream,
							parentCode,
							tagCode,
							tagCount,
							s,
							false);

			}

		}

	#endif

	}

/*****************************************************************************/

bool dng_info::ValidateIFD (dng_stream &stream,
						    uint64 ifdOffset,
						    int64 offsetDelta)
	{

	// Make sure we have a count.

	if (ifdOffset + 2 > stream.Length ())
		{
		return false;
		}

	// Get entry count.

	stream.SetReadPosition (ifdOffset);

	uint32 ifdEntries = stream.Get_uint16 ();

	if (ifdEntries < 1)
		{
		return false;
		}

	// Make sure we have room for all entries and next IFD link.

	if (ifdOffset + 2 + ifdEntries * 12 + 4 > stream.Length ())
		{
		return false;
		}

	// Check each entry.

	for (uint32 tag_index = 0; tag_index < ifdEntries; tag_index++)
		{

		stream.SetReadPosition (ifdOffset + 2 + tag_index * 12);

		stream.Skip (2);		// Ignore tag code.

		uint32 tagType  = stream.Get_uint16 ();
		uint32 tagCount = stream.Get_uint32 ();

		uint32 tag_type_size = TagTypeSize (tagType);

		if (tag_type_size == 0)
			{
			return false;
			}

		uint32 tag_data_size = tagCount * tag_type_size;

		if (tag_data_size > 4)
			{

			uint64 tagOffset = stream.Get_uint32 ();

			tagOffset += offsetDelta;

			if (tagOffset + tag_data_size > stream.Length ())
				{
				return false;
				}

			}

		}

	return true;

	}

/*****************************************************************************/

void dng_info::ParseIFD (dng_host &host,
						 dng_stream &stream,
						 dng_exif *exif,
						 dng_shared *shared,
						 dng_ifd *ifd,
						 uint64 ifdOffset,
						 int64 offsetDelta,
						 uint32 parentCode)
	{

	#if qDNGValidate

	bool isMakerNote = (parentCode >= tcFirstMakerNoteIFD &&
						parentCode <= tcLastMakerNoteIFD);

	#endif

	stream.SetReadPosition (ifdOffset);

	if (ifd)
		{
		ifd->fThisIFD = ifdOffset;
		}

	uint32 ifdEntries = stream.Get_uint16 ();

	#if qDNGValidate

	if (gVerbose)
		{

		printf ("%s: Offset = %u, Entries = %u\n\n",
				LookupParentCode (parentCode),
			    (unsigned) ifdOffset,
			    (unsigned) ifdEntries);

		}

	if ((ifdOffset & 1) && !isMakerNote)
		{

		char message [256];

		sprintf (message,
				 "%s has odd offset (%u)",
				 LookupParentCode (parentCode),
				 (unsigned) ifdOffset);

		ReportWarning (message);

		}

	#endif

	uint32 prev_tag_code = 0;

	for (uint32 tag_index = 0; tag_index < ifdEntries; tag_index++)
		{

		stream.SetReadPosition (ifdOffset + 2 + tag_index * 12);

		uint32 tagCode  = stream.Get_uint16 ();
		uint32 tagType  = stream.Get_uint16 ();

		// Minolta 7D files have a bug in the EXIF block where the count
		// is wrong, and we run off into next IFD link.  So if abort parsing
		// if we get a zero code/type combinations.

		if (tagCode == 0 && tagType == 0)
			{

			#if qDNGValidate

			char message [256];

			sprintf (message,
					 "%s had zero/zero tag code/type entry",
					 LookupParentCode (parentCode));

			ReportWarning (message);

			#endif

			return;

			}

		uint32 tagCount = stream.Get_uint32 ();

		#if qDNGValidate

			{

			if (tag_index > 0 && tagCode <= prev_tag_code && !isMakerNote)
				{

				char message [256];

				sprintf (message,
						 "%s tags are not sorted in ascending numerical order",
						 LookupParentCode (parentCode));

				ReportWarning (message);

				}

			}

		#endif

		prev_tag_code = tagCode;

		uint32 tag_type_size = TagTypeSize (tagType);

		if (tag_type_size == 0)
			{

			#if qDNGValidate

				{

				char message [256];

				sprintf (message,
						 "%s %s has unknown type (%u)",
						 LookupParentCode (parentCode),
						 LookupTagCode (parentCode, tagCode),
						 (unsigned) tagType);

				ReportWarning (message);

				}

			#endif

			continue;

			}

		uint64 tagOffset = ifdOffset + 2 + tag_index * 12 + 8;

		if (tagCount * tag_type_size > 4)
			{

			tagOffset = stream.Get_uint32 ();

			#if qDNGValidate

				{

				if (!(ifdOffset & 1) &&
				     (tagOffset & 1) &&
				    !isMakerNote     &&
				    parentCode != tcKodakDCRPrivateIFD &&
					parentCode != tcKodakKDCPrivateIFD)
					{

					char message [256];

					sprintf (message,
							 "%s %s has odd data offset (%u)",
						 	 LookupParentCode (parentCode),
						 	 LookupTagCode (parentCode, tagCode),
							 (unsigned) tagOffset);

					ReportWarning (message);

					}

				}

			#endif

			tagOffset += offsetDelta;

			stream.SetReadPosition (tagOffset);

			}

		ParseTag (host,
				  stream,
			      exif,
				  shared,
				  ifd,
				  parentCode,
				  tagCode,
				  tagType,
				  tagCount,
				  tagOffset,
				  offsetDelta);

		}

	stream.SetReadPosition (ifdOffset + 2 + ifdEntries * 12);

	uint32 nextIFD = stream.Get_uint32 ();

	#if qDNGValidate

	if (gVerbose)
		{
		printf ("NextIFD = %u\n", (unsigned) nextIFD);
		}

	#endif

	if (ifd)
		{
		ifd->fNextIFD = nextIFD;
		}

	#if qDNGValidate

	if (nextIFD)
		{

		if (parentCode != 0 &&
				(parentCode < tcFirstChainedIFD ||
				 parentCode > tcLastChainedIFD  ))
			{

			char message [256];

			sprintf (message,
					 "%s has an unexpected non-zero NextIFD (%u)",
				 	 LookupParentCode (parentCode),
				 	 (unsigned) nextIFD);

			ReportWarning (message);

			}

		}

	if (gVerbose)
		{
		printf ("\n");
		}

	#endif

	}

/*****************************************************************************/

bool dng_info::ParseMakerNoteIFD (dng_host &host,
								  dng_stream &stream,
								  uint64 ifdSize,
						 		  uint64 ifdOffset,
								  int64 offsetDelta,
								  uint64 minOffset,
								  uint64 maxOffset,
						 		  uint32 parentCode)
	{

	uint32 tagIndex;
	uint32 tagCode;
	uint32 tagType;
	uint32 tagCount;

	// Assume there is no next IFD pointer.

	fMakerNoteNextIFD = 0;

	// If size is too small to hold a single entry IFD, abort.

	if (ifdSize < 14)
		{
		return false;
		}

	// Get entry count.

	stream.SetReadPosition (ifdOffset);

	uint32 ifdEntries = stream.Get_uint16 ();

	// Make the entry count if reasonable for the MakerNote size.

	if (ifdEntries < 1 || 2 + ifdEntries * 12 > ifdSize)
		{
		return false;
		}

	// Scan IFD to verify all the tag types are all valid.

	for (tagIndex = 0; tagIndex < ifdEntries; tagIndex++)
		{

		stream.SetReadPosition (ifdOffset + 2 + tagIndex * 12 + 2);

		tagType = stream.Get_uint16 ();

		// Kludge: Some Canon MakerNotes contain tagType = 0 tags, so we
		// need to ignore them.  This was a "firmware 1.0.4" Canon 40D raw file.

		if (parentCode == tcCanonMakerNote && tagType == 0)
			{
			continue;
			}

		if (TagTypeSize (tagType) == 0)
			{
			return false;
			}

		}

	// OK, the IFD looks reasonable enough to parse.

	#if qDNGValidate

	if (gVerbose)
		{

		printf ("%s: Offset = %u, Entries = %u\n\n",
				LookupParentCode (parentCode),
			    (unsigned) ifdOffset,
			    (unsigned) ifdEntries);

		}

	#endif

	for (tagIndex = 0; tagIndex < ifdEntries; tagIndex++)
		{

		stream.SetReadPosition (ifdOffset + 2 + tagIndex * 12);

		tagCode  = stream.Get_uint16 ();
		tagType  = stream.Get_uint16 ();
		tagCount = stream.Get_uint32 ();

		if (tagType == 0)
			{
			continue;
			}

		uint32 tagSize = tagCount * TagTypeSize (tagType);

		uint64 tagOffset = ifdOffset + 2 + tagIndex * 12 + 8;

		if (tagSize > 4)
			{

			tagOffset = stream.Get_uint32 () + offsetDelta;

			if (tagOffset           < minOffset ||
				tagOffset + tagSize > maxOffset)
				{

				// Tag data is outside the valid offset range,
				// so ignore this tag.

				continue;

				}

			stream.SetReadPosition (tagOffset);

			}

		// Olympus switched to using IFDs in version 3 makernotes.

		if (parentCode == tcOlympusMakerNote &&
			tagType == ttIFD &&
			tagCount == 1)
			{

			uint32 olympusMakerParent = 0;

			switch (tagCode)
				{

				case 8208:
					olympusMakerParent = tcOlympusMakerNote8208;
					break;

				case 8224:
					olympusMakerParent = tcOlympusMakerNote8224;
					break;

				case 8240:
					olympusMakerParent = tcOlympusMakerNote8240;
					break;

				case 8256:
					olympusMakerParent = tcOlympusMakerNote8256;
					break;

				case 8272:
					olympusMakerParent = tcOlympusMakerNote8272;
					break;

				case 12288:
					olympusMakerParent = tcOlympusMakerNote12288;
					break;

				default:
					break;

				}

			if (olympusMakerParent)
				{

				stream.SetReadPosition (tagOffset);

				uint64 subMakerNoteOffset = stream.Get_uint32 () + offsetDelta;

				if (subMakerNoteOffset >= minOffset &&
					subMakerNoteOffset <  maxOffset)
					{

					if (ParseMakerNoteIFD (host,
										   stream,
										   maxOffset - subMakerNoteOffset,
										   subMakerNoteOffset,
										   offsetDelta,
										   minOffset,
										   maxOffset,
										   olympusMakerParent))
						{

						continue;

						}

					}

				}

			stream.SetReadPosition (tagOffset);

			}

		ParseTag (host,
				  stream,
				  fExif.Get (),
				  fShared.Get (),
				  NULL,
				  parentCode,
				  tagCode,
				  tagType,
				  tagCount,
				  tagOffset,
				  offsetDelta);

		}

	// Grab next IFD pointer, for possible use.

	if (ifdSize >= 2 + ifdEntries * 12 + 4)
		{

		stream.SetReadPosition (ifdOffset + 2 + ifdEntries * 12);

		fMakerNoteNextIFD = stream.Get_uint32 ();

		}

	#if qDNGValidate

	if (gVerbose)
		{
		printf ("\n");
		}

	#endif

	return true;

	}

/*****************************************************************************/

void dng_info::ParseMakerNote (dng_host &host,
							   dng_stream &stream,
							   uint32 makerNoteCount,
							   uint64 makerNoteOffset,
							   int64 offsetDelta,
							   uint64 minOffset,
							   uint64 maxOffset)
	{

	uint8 firstBytes [16];

	memset (firstBytes, 0, sizeof (firstBytes));

	stream.SetReadPosition (makerNoteOffset);

	stream.Get (firstBytes, (uint32) Min_uint64 (sizeof (firstBytes),
												 makerNoteCount));

	// Epson MakerNote with header.

	if (memcmp (firstBytes, "EPSON\000\001\000", 8) == 0)
		{

		if (makerNoteCount > 8)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 8,
				   	  		   makerNoteOffset + 8,
				   	  		   offsetDelta,
				   	  		   minOffset,
				   	  		   maxOffset,
				   	  		   tcEpsonMakerNote);

			}

		return;

		}

	// Fujifilm MakerNote.

	if (memcmp (firstBytes, "FUJIFILM", 8) == 0)
		{

		stream.SetReadPosition (makerNoteOffset + 8);

		TempLittleEndian tempEndian (stream);

		uint32 ifd_offset = stream.Get_uint32 ();

		if (ifd_offset >= 12 && ifd_offset < makerNoteCount)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - ifd_offset,
							   makerNoteOffset + ifd_offset,
							   makerNoteOffset,
							   minOffset,
							   maxOffset,
							   tcFujiMakerNote);

			}

		return;

		}

	// Leica MakerNote.

	if (memcmp (firstBytes, "LEICA\000\000\000", 8) == 0)
		{

		if (makerNoteCount > 8)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 8,
							   makerNoteOffset + 8,
							   makerNoteOffset,
							   minOffset,
							   maxOffset,
							   tcLeicaMakerNote);

			}

		return;

		}

	// Nikon version 2 MakerNote with header.

	if (memcmp (firstBytes, "Nikon\000\002", 7) == 0)
		{

		stream.SetReadPosition (makerNoteOffset + 10);

		bool bigEndian = false;

		uint16 endianMark = stream.Get_uint16 ();

		if (endianMark == byteOrderMM)
			{
			bigEndian = true;
			}

		else if (endianMark != byteOrderII)
			{
			return;
			}

		TempBigEndian temp_endian (stream, bigEndian);

		uint16 magic = stream.Get_uint16 ();

		if (magic != 42)
			{
			return;
			}

		uint32 ifd_offset = stream.Get_uint32 ();

		if (ifd_offset >= 8 && ifd_offset < makerNoteCount - 10)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 10 - ifd_offset,
							   makerNoteOffset + 10 + ifd_offset,
							   makerNoteOffset + 10,
							   minOffset,
							   maxOffset,
							   tcNikonMakerNote);

			}

		return;

		}

	// Newer version of Olympus MakerNote with byte order mark.

	if (memcmp (firstBytes, "OLYMPUS\000", 8) == 0)
		{

		stream.SetReadPosition (makerNoteOffset + 8);

		bool bigEndian = false;

		uint16 endianMark = stream.Get_uint16 ();

		if (endianMark == byteOrderMM)
			{
			bigEndian = true;
			}

		else if (endianMark != byteOrderII)
			{
			return;
			}

		TempBigEndian temp_endian (stream, bigEndian);

		uint16 version = stream.Get_uint16 ();

		if (version != 3)
			{
			return;
			}

		if (makerNoteCount > 12)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 12,
				   	  		   makerNoteOffset + 12,
				   	  		   makerNoteOffset,
				   	  		   minOffset,
				   	  		   maxOffset,
				   	  		   tcOlympusMakerNote);

			}

		return;

		}

	// Olympus MakerNote with header.

	if (memcmp (firstBytes, "OLYMP", 5) == 0)
		{

		if (makerNoteCount > 8)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 8,
				   	  		   makerNoteOffset + 8,
				   	  		   offsetDelta,
				   	  		   minOffset,
				   	  		   maxOffset,
				   	  		   tcOlympusMakerNote);

			}

		return;

		}

	// Panasonic MakerNote.

	if (memcmp (firstBytes, "Panasonic\000\000\000", 12) == 0)
		{

		if (makerNoteCount > 12)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 12,
							   makerNoteOffset + 12,
							   offsetDelta,
							   minOffset,
							   maxOffset,
							   tcPanasonicMakerNote);

			}

		return;

		}

	// Pentax MakerNote.

	if (memcmp (firstBytes, "AOC", 4) == 0)
		{

		if (makerNoteCount > 6)
			{

			stream.SetReadPosition (makerNoteOffset + 4);

			bool bigEndian = stream.BigEndian ();

			uint16 endianMark = stream.Get_uint16 ();

			if (endianMark == byteOrderMM)
				{
				bigEndian = true;
				}

			else if (endianMark == byteOrderII)
				{
				bigEndian = false;
				}

			TempBigEndian temp_endian (stream, bigEndian);

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 6,
							   makerNoteOffset + 6,
							   offsetDelta,
							   minOffset,
							   maxOffset,
							   tcPentaxMakerNote);

			}

		return;

		}

	// Ricoh MakerNote.

	if (memcmp (firstBytes, "RICOH", 5) == 0 ||
		memcmp (firstBytes, "Ricoh", 5) == 0)
		{

		if (makerNoteCount > 8)
			{

			TempBigEndian tempEndian (stream);

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount - 8,
				   	  		   makerNoteOffset + 8,
				   	  		   offsetDelta,
				   	  		   minOffset,
				   	  		   maxOffset,
				   	  		   tcRicohMakerNote);

			}

		return;

		}

	// Nikon MakerNote without header.

	if (fExif->fMake.StartsWith ("NIKON"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
			   	  		   makerNoteOffset,
			   	  		   offsetDelta,
			   	  		   minOffset,
			   	  		   maxOffset,
			   	  		   tcNikonMakerNote);

		return;

		}

	// Canon MakerNote.

	if (fExif->fMake.StartsWith ("CANON"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
			   	  		   makerNoteOffset,
			   	  		   offsetDelta,
			   	  		   minOffset,
			   	  		   maxOffset,
			   	  		   tcCanonMakerNote);

		return;

		}

	// Minolta MakerNote.

	if (fExif->fMake.StartsWith ("MINOLTA"       ) ||
		fExif->fMake.StartsWith ("KONICA MINOLTA"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
						   makerNoteOffset,
						   offsetDelta,
						   minOffset,
						   maxOffset,
						   tcMinoltaMakerNote);

		return;

		}

	// Sony MakerNote.

	if (fExif->fMake.StartsWith ("SONY"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
						   makerNoteOffset,
						   offsetDelta,
						   minOffset,
						   maxOffset,
						   tcSonyMakerNote);

		return;

		}

	// Kodak MakerNote.

	if (fExif->fMake.StartsWith ("EASTMAN KODAK"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
			   	  		   makerNoteOffset,
			   	  		   offsetDelta,
			   	  		   minOffset,
			   	  		   maxOffset,
			   	  		   tcKodakMakerNote);

		return;

		}

	// Mamiya MakerNote.

	if (fExif->fMake.StartsWith ("Mamiya"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
			   	  		   makerNoteOffset,
			   	  		   offsetDelta,
			   	  		   minOffset,
			   	  		   maxOffset,
			   	  		   tcMamiyaMakerNote);

		// Mamiya uses a MakerNote chain.

		while (fMakerNoteNextIFD)
			{

			ParseMakerNoteIFD (host,
							   stream,
							   makerNoteCount,
							   offsetDelta + fMakerNoteNextIFD,
							   offsetDelta,
							   minOffset,
							   maxOffset,
							   tcMamiyaMakerNote);

			}

		return;

		}

	// Nikon MakerNote without header.

	if (fExif->fMake.StartsWith ("Hasselblad"))
		{

		ParseMakerNoteIFD (host,
						   stream,
						   makerNoteCount,
			   	  		   makerNoteOffset,
			   	  		   offsetDelta,
			   	  		   minOffset,
			   	  		   maxOffset,
			   	  		   tcHasselbladMakerNote);

		return;

		}

	}

/*****************************************************************************/

void dng_info::ParseSonyPrivateData (dng_host & /* host */,
									 dng_stream & /* stream */,
									 uint32 /* count */,
									 uint64 /* oldOffset */,
									 uint64 /* newOffset */)
	{

	// Sony private data is encrypted, sorry.

	}

/*****************************************************************************/

void dng_info::ParseDNGPrivateData (dng_host &host,
									dng_stream &stream)
	{

	if (fShared->fDNGPrivateDataCount < 2)
		{
		return;
		}

	// DNG private data should always start with a null-terminated
	// company name, to define the format of the private data.

	dng_string privateName;

		{

		char buffer [64];

		stream.SetReadPosition (fShared->fDNGPrivateDataOffset);

		uint32 readLength = Min_uint32 (fShared->fDNGPrivateDataCount,
										sizeof (buffer) - 1);

		stream.Get (buffer, readLength);

		buffer [readLength] = 0;

		privateName.Set (buffer);

		}

	// Pentax is storing their MakerNote in the DNGPrivateData data.

	if (privateName.StartsWith ("PENTAX" ) ||
		privateName.StartsWith ("SAMSUNG"))
		{

		#if qDNGValidate

		if (gVerbose)
			{
			printf ("Parsing Pentax/Samsung DNGPrivateData\n\n");
			}

		#endif

		stream.SetReadPosition (fShared->fDNGPrivateDataOffset + 8);

		bool bigEndian = stream.BigEndian ();

		uint16 endianMark = stream.Get_uint16 ();

		if (endianMark == byteOrderMM)
			{
			bigEndian = true;
			}

		else if (endianMark == byteOrderII)
			{
			bigEndian = false;
			}

		TempBigEndian temp_endian (stream, bigEndian);

		ParseMakerNoteIFD (host,
						   stream,
						   fShared->fDNGPrivateDataCount - 10,
						   fShared->fDNGPrivateDataOffset + 10,
						   fShared->fDNGPrivateDataOffset,
						   fShared->fDNGPrivateDataOffset,
						   fShared->fDNGPrivateDataOffset + fShared->fDNGPrivateDataCount,
						   tcPentaxMakerNote);

		return;

		}

	// Stop parsing if this is not an Adobe format block.

	if (!privateName.Matches ("Adobe"))
		{
		return;
		}

	TempBigEndian temp_order (stream);

	uint32 section_offset = 6;

	while (section_offset + 8 < fShared->fDNGPrivateDataCount)
		{

		stream.SetReadPosition (fShared->fDNGPrivateDataOffset + section_offset);

		uint32 section_key   = stream.Get_uint32 ();
		uint32 section_count = stream.Get_uint32 ();

		if (section_key == DNG_CHAR4 ('M','a','k','N') && section_count > 6)
			{

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("Found MakerNote inside DNGPrivateData\n\n");
				}

			#endif

			uint16 order_mark = stream.Get_uint16 ();
			uint64 old_offset = stream.Get_uint32 ();

			uint32 tempSize = section_count - 6;

			AutoPtr<dng_memory_block> tempBlock (host.Allocate (tempSize));

			uint64 positionInOriginalFile = stream.PositionInOriginalFile();

			stream.Get (tempBlock->Buffer (), tempSize);

			dng_stream tempStream (tempBlock->Buffer (),
								   tempSize,
								   positionInOriginalFile);

			tempStream.SetBigEndian (order_mark == byteOrderMM);

			ParseMakerNote (host,
							tempStream,
							tempSize,
							0,
							0 - old_offset,
							0,
							tempSize);

			}

		else if (section_key == DNG_CHAR4 ('S','R','2',' ') && section_count > 6)
			{

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("Found Sony private data inside DNGPrivateData\n\n");
				}

			#endif

			uint16 order_mark = stream.Get_uint16 ();
			uint64 old_offset = stream.Get_uint32 ();

			uint64 new_offset = fShared->fDNGPrivateDataOffset + section_offset + 14;

			TempBigEndian sr2_order (stream, order_mark == byteOrderMM);

			ParseSonyPrivateData (host,
							  	  stream,
								  section_count - 6,
								  old_offset,
								  new_offset);

			}

		else if (section_key == DNG_CHAR4 ('R','A','F',' ') && section_count > 4)
			{

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("Found Fuji RAF tags inside DNGPrivateData\n\n");
				}

			#endif

			uint16 order_mark = stream.Get_uint16 ();

			uint32 tagCount = stream.Get_uint32 ();

			uint64 tagOffset = stream.Position ();

			if (tagCount)
				{

				TempBigEndian raf_order (stream, order_mark == byteOrderMM);

				ParseTag (host,
						  stream,
						  fExif.Get (),
						  fShared.Get (),
						  NULL,
						  tcFujiRAF,
						  tcFujiHeader,
						  ttUndefined,
						  tagCount,
						  tagOffset,
						  0);

				stream.SetReadPosition (tagOffset + tagCount);

				}

			tagCount = stream.Get_uint32 ();

			tagOffset = stream.Position ();

			if (tagCount)
				{

				TempBigEndian raf_order (stream, order_mark == byteOrderMM);

				ParseTag (host,
						  stream,
						  fExif.Get (),
						  fShared.Get (),
						  NULL,
						  tcFujiRAF,
						  tcFujiRawInfo1,
						  ttUndefined,
						  tagCount,
						  tagOffset,
						  0);

				stream.SetReadPosition (tagOffset + tagCount);

				}

			tagCount = stream.Get_uint32 ();

			tagOffset = stream.Position ();

			if (tagCount)
				{

				TempBigEndian raf_order (stream, order_mark == byteOrderMM);

				ParseTag (host,
						  stream,
						  fExif.Get (),
						  fShared.Get (),
						  NULL,
						  tcFujiRAF,
						  tcFujiRawInfo2,
						  ttUndefined,
						  tagCount,
						  tagOffset,
						  0);

				stream.SetReadPosition (tagOffset + tagCount);

				}

			}

		else if (section_key == DNG_CHAR4 ('C','n','t','x') && section_count > 4)
			{

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("Found Contax Raw header inside DNGPrivateData\n\n");
				}

			#endif

			uint16 order_mark = stream.Get_uint16 ();

			uint32 tagCount  = stream.Get_uint32 ();

			uint64 tagOffset = stream.Position ();

			if (tagCount)
				{

				TempBigEndian contax_order (stream, order_mark == byteOrderMM);

				ParseTag (host,
						  stream,
						  fExif.Get (),
						  fShared.Get (),
						  NULL,
						  tcContaxRAW,
						  tcContaxHeader,
						  ttUndefined,
						  tagCount,
						  tagOffset,
						  0);

				}

			}

		else if (section_key == DNG_CHAR4 ('C','R','W',' ') && section_count > 4)
			{

			#if qDNGValidate

			if (gVerbose)
				{
				printf ("Found Canon CRW tags inside DNGPrivateData\n\n");
				}

			#endif

			uint16 order_mark = stream.Get_uint16 ();
			uint32 entries    = stream.Get_uint16 ();

			uint64 crwTagStart = stream.Position ();

			for (uint32 parsePass = 1; parsePass <= 2; parsePass++)
				{

				stream.SetReadPosition (crwTagStart);

				for (uint32 index = 0; index < entries; index++)
					{

					uint32 tagCode = stream.Get_uint16 ();

					uint32 tagCount = stream.Get_uint32 ();

					uint64 tagOffset = stream.Position ();

					// We need to grab the model id tag first, and then all the
					// other tags.

					if ((parsePass == 1) == (tagCode == 0x5834))
						{

						TempBigEndian tag_order (stream, order_mark == byteOrderMM);

						ParseTag (host,
								  stream,
								  fExif.Get (),
								  fShared.Get (),
								  NULL,
								  tcCanonCRW,
								  tagCode,
								  ttUndefined,
								  tagCount,
								  tagOffset,
								  0);

						}

					stream.SetReadPosition (tagOffset + tagCount);

					}

				}

			}

		else if (section_count > 4)
			{

			uint32 parentCode = 0;

			bool code32  = false;
			bool hasType = true;

			switch (section_key)
				{

				case DNG_CHAR4 ('M','R','W',' '):
					{
					parentCode = tcMinoltaMRW;
					code32     = true;
					hasType    = false;
					break;
					}

				case DNG_CHAR4 ('P','a','n','o'):
					{
					parentCode = tcPanasonicRAW;
					break;
					}

				case DNG_CHAR4 ('L','e','a','f'):
					{
					parentCode = tcLeafMOS;
					break;
					}

				case DNG_CHAR4 ('K','o','d','a'):
					{
					parentCode = tcKodakDCRPrivateIFD;
					break;
					}

				case DNG_CHAR4 ('K','D','C',' '):
					{
					parentCode = tcKodakKDCPrivateIFD;
					break;
					}

				default:
					break;

				}

			if (parentCode)
				{

				#if qDNGValidate

				if (gVerbose)
					{
					printf ("Found %s tags inside DNGPrivateData\n\n",
							LookupParentCode (parentCode));
					}

				#endif

				uint16 order_mark = stream.Get_uint16 ();
				uint32 entries    = stream.Get_uint16 ();

				for (uint32 index = 0; index < entries; index++)
					{

					uint32 tagCode = code32 ? stream.Get_uint32 ()
											: stream.Get_uint16 ();

					uint32 tagType  = hasType ? stream.Get_uint16 ()
											  : ttUndefined;

					uint32 tagCount = stream.Get_uint32 ();

					uint32 tagSize = tagCount * TagTypeSize (tagType);

					uint64 tagOffset = stream.Position ();

					TempBigEndian tag_order (stream, order_mark == byteOrderMM);

					ParseTag (host,
							  stream,
							  fExif.Get (),
							  fShared.Get (),
							  NULL,
							  parentCode,
							  tagCode,
							  tagType,
							  tagCount,
							  tagOffset,
							  0);

					stream.SetReadPosition (tagOffset + tagSize);

					}

				}

			}

		section_offset += 8 + section_count;

		if (section_offset & 1)
			{
			section_offset++;
			}

		}

	}

/*****************************************************************************/

void dng_info::Parse (dng_host &host,
					  dng_stream &stream)
	{

	fTIFFBlockOffset = stream.Position ();

	fTIFFBlockOriginalOffset = stream.PositionInOriginalFile ();

	// Check byte order indicator.

	uint16 byteOrder = stream.Get_uint16 ();

	if (byteOrder == byteOrderII)
		{

		fBigEndian = false;

		#if qDNGValidate

		if (gVerbose)
			{
			printf ("\nUses little-endian byte order\n");
			}

		#endif

		stream.SetLittleEndian ();

		}

	else if (byteOrder == byteOrderMM)
		{

		fBigEndian = true;

		#if qDNGValidate

		if (gVerbose)
			{
			printf ("\nUses big-endian byte order\n");
			}

		#endif

		stream.SetBigEndian ();

		}

	else
		{

		#if qDNGValidate

		ReportError ("Unknown byte order");

		#endif

		ThrowBadFormat ();

		}

	// Check "magic number" indicator.

	fMagic = stream.Get_uint16 ();

	#if qDNGValidate

	if (gVerbose)
		{
		printf ("Magic number = %u\n\n", (unsigned) fMagic);
		}

	#endif

	ValidateMagic ();

	// Parse IFD 0.

	uint64 next_offset = stream.Get_uint32 ();

	fExif.Reset (host.Make_dng_exif ());

	fShared.Reset (host.Make_dng_shared ());

	fIFD [0].Reset (host.Make_dng_ifd ());

	ParseIFD (host,
			  stream,
			  fExif.Get (),
			  fShared.Get (),
			  fIFD [0].Get (),
			  fTIFFBlockOffset + next_offset,
			  fTIFFBlockOffset,
			  0);

	next_offset = fIFD [0]->fNextIFD;

	fIFDCount = 1;

	// Parse chained IFDs.

	while (next_offset)
		{

		if (next_offset >= stream.Length ())
			{

			#if qDNGValidate

				{

				ReportWarning ("Chained IFD offset past end of stream");

				}

			#endif

			break;

			}

		// Some TIFF file writers forget about the next IFD offset, so
		// validate the IFD at that offset before parsing it.

		if (!ValidateIFD (stream,
						  fTIFFBlockOffset + next_offset,
						  fTIFFBlockOffset))
			{

			#if qDNGValidate

				{

				ReportWarning ("Chained IFD is not valid");

				}

			#endif

			break;

			}

		if (fChainedIFDCount == kMaxChainedIFDs)
			{

			#if qDNGValidate

				{

				ReportWarning ("Chained IFD count exceeds DNG SDK parsing limit");

				}

			#endif

			break;

			}

		fChainedIFD [fChainedIFDCount].Reset (host.Make_dng_ifd ());

		ParseIFD (host,
				  stream,
				  NULL,
				  NULL,
				  fChainedIFD [fChainedIFDCount].Get (),
				  fTIFFBlockOffset + next_offset,
				  fTIFFBlockOffset,
				  tcFirstChainedIFD + fChainedIFDCount);

		next_offset = fChainedIFD [fChainedIFDCount]->fNextIFD;

		fChainedIFDCount++;

		}

	// Parse SubIFDs.

	uint32 searchedIFDs = 0;

	bool tooManySubIFDs = false;

	while (searchedIFDs < fIFDCount && !tooManySubIFDs)
		{

		uint32 searchLimit = fIFDCount;

		for (uint32 searchIndex = searchedIFDs;
			 searchIndex < searchLimit && !tooManySubIFDs;
			 searchIndex++)
			{

			for (uint32 subIndex = 0;
			     subIndex < fIFD [searchIndex]->fSubIFDsCount;
			     subIndex++)
				{

				if (fIFDCount == kMaxSubIFDs + 1)
					{

					tooManySubIFDs = true;

					break;

					}

				stream.SetReadPosition (fIFD [searchIndex]->fSubIFDsOffset +
							 			subIndex * 4);

				uint32 sub_ifd_offset = stream.Get_uint32 ();

				fIFD [fIFDCount].Reset (host.Make_dng_ifd ());

				ParseIFD (host,
						  stream,
						  fExif.Get (),
						  fShared.Get (),
						  fIFD [fIFDCount].Get (),
						  fTIFFBlockOffset + sub_ifd_offset,
						  fTIFFBlockOffset,
						  tcFirstSubIFD + fIFDCount - 1);

				fIFDCount++;

				}

			searchedIFDs = searchLimit;

			}

		}

	#if qDNGValidate

		{

		if (tooManySubIFDs)
			{

			ReportWarning ("SubIFD count exceeds DNG SDK parsing limit");

			}

		}

	#endif

	// Parse EXIF IFD.

	if (fShared->fExifIFD)
		{

		ParseIFD (host,
				  stream,
				  fExif.Get (),
				  fShared.Get (),
				  NULL,
				  fTIFFBlockOffset + fShared->fExifIFD,
				  fTIFFBlockOffset,
				  tcExifIFD);

		}

	// Parse GPS IFD.

	if (fShared->fGPSInfo)
		{

		ParseIFD (host,
				  stream,
				  fExif.Get (),
				  fShared.Get (),
				  NULL,
				  fTIFFBlockOffset + fShared->fGPSInfo,
				  fTIFFBlockOffset,
				  tcGPSInfo);

		}

	// Parse Interoperability IFD.

	if (fShared->fInteroperabilityIFD)
		{

		// Some Kodak KDC files have bogus Interoperability IFDs, so
		// validate the IFD before trying to parse it.

		if (ValidateIFD (stream,
						 fTIFFBlockOffset + fShared->fInteroperabilityIFD,
						 fTIFFBlockOffset))
			{

			ParseIFD (host,
					  stream,
					  fExif.Get (),
					  fShared.Get (),
					  NULL,
					  fTIFFBlockOffset + fShared->fInteroperabilityIFD,
					  fTIFFBlockOffset,
					  tcInteroperabilityIFD);

			}

		#if qDNGValidate

		else
			{

			ReportWarning ("The Interoperability IFD is not a valid IFD");

			}

		#endif

		}

	// Parse Kodak DCR Private IFD.

	if (fShared->fKodakDCRPrivateIFD)
		{

		ParseIFD (host,
				  stream,
				  fExif.Get (),
				  fShared.Get (),
				  NULL,
				  fTIFFBlockOffset + fShared->fKodakDCRPrivateIFD,
				  fTIFFBlockOffset,
				  tcKodakDCRPrivateIFD);

		}

	// Parse Kodak KDC Private IFD.

	if (fShared->fKodakKDCPrivateIFD)
		{

		ParseIFD (host,
				  stream,
				  fExif.Get (),
				  fShared.Get (),
				  NULL,
				  fTIFFBlockOffset + fShared->fKodakKDCPrivateIFD,
				  fTIFFBlockOffset,
				  tcKodakKDCPrivateIFD);

		}

	// Parse MakerNote tag.

	if (fShared->fMakerNoteCount)
		{

		ParseMakerNote (host,
						stream,
						(uint32) (fTIFFBlockOffset + fShared->fMakerNoteCount),
						fShared->fMakerNoteOffset,
						fTIFFBlockOffset,
						0,
						stream.Length ());

		}

	// Parse DNGPrivateData tag.

	if (fShared->fDNGPrivateDataCount &&
		fShared->fDNGVersion)
		{

		ParseDNGPrivateData (host, stream);

		}

	#if qDNGValidate

	// If we are running dng_validate on stand-alone camera profile file,
	// complete the validation of the profile.

	if (fMagic == magicExtendedProfile)
		{

		dng_camera_profile_info &profileInfo = fShared->fCameraProfile;

		dng_camera_profile profile;

		profile.Parse (stream, profileInfo);

		if (profileInfo.fColorPlanes < 3 || !profile.IsValid (profileInfo.fColorPlanes))
			{

			ReportError ("Invalid camera profile file");

			}

		}

	#endif

	}

/*****************************************************************************/

void dng_info::PostParse (dng_host &host)
	{

	uint32 index;

	fExif->PostParse (host, *fShared.Get ());

	fShared->PostParse (host, *fExif.Get ());

	for (index = 0; index < fIFDCount; index++)
		{

		fIFD [index]->PostParse ();

		}

	for (index = 0; index < fChainedIFDCount; index++)
		{

		fChainedIFD [index]->PostParse ();

		}

	if (fShared->fDNGVersion != 0)
		{

		// Find main IFD.

		fMainIndex = -1;

		for (index = 0; index < fIFDCount; index++)
			{

			if (fIFD [index]->fUsesNewSubFileType &&
				fIFD [index]->fNewSubFileType == sfMainImage)
				{

				if (fMainIndex == -1)
					{

					fMainIndex = index;

					}

				#if qDNGValidate

				else
					{

					ReportError ("Multiple IFDs marked as main image");

					}

				#endif

				}

			else if (fIFD [index]->fNewSubFileType == sfPreviewImage ||
					 fIFD [index]->fNewSubFileType == sfAltPreviewImage)
				{

				// Fill in default color space for DNG previews if not included.

				if (fIFD [index]->fPreviewInfo.fColorSpace == previewColorSpace_MaxEnum)
					{

					if (fIFD [index]->fSamplesPerPixel == 1)
						{

						fIFD [index]->fPreviewInfo.fColorSpace = previewColorSpace_GrayGamma22;

						}

					else
						{

						fIFD [index]->fPreviewInfo.fColorSpace = previewColorSpace_sRGB;

						}

					}

				}

			}

		// Deal with lossless JPEG bug in early DNG versions.

		if (fShared->fDNGVersion < dngVersion_1_1_0_0)
			{

			if (fMainIndex != -1)
				{

				fIFD [fMainIndex]->fLosslessJPEGBug16 = true;

				}

			}

		// Warn about Chained IFDs.

		#if qDNGValidate

		if (fChainedIFDCount > 0)
			{

			ReportWarning ("This file has Chained IFDs, which will be ignored by DNG readers");

			}

		#endif

		}

	}

/*****************************************************************************/

bool dng_info::IsValidDNG ()
	{

	// Check shared info.

	if (!fShared->IsValidDNG ())
		{

		return false;

		}

	// Check TIFF magic number.

	if (fMagic != 42)
		{

		#if qDNGValidate

		ReportError ("Invalid TIFF magic number");

		#endif

		return false;

		}

	// Make sure we have a main image IFD.

	if (fMainIndex == -1)
		{

		#if qDNGValidate

		ReportError ("Unable to find main image IFD");

		#endif

		return false;

		}

	// Make sure is each IFD is valid.

	for (uint32 index = 0; index < fIFDCount; index++)
		{

		uint32 parentCode = (index == 0 ? 0 : tcFirstSubIFD + index - 1);

		if (!fIFD [index]->IsValidDNG (*fShared.Get (),
								       parentCode))
			{

			// Only errors in the main IFD are fatal to parsing.

			if (index == (uint32) fMainIndex)
				{

				return false;

				}

			}

		}

	return true;

	}

/*****************************************************************************/
