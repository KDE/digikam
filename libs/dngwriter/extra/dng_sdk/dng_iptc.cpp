/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_iptc.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_iptc.h"

#include "dng_assertions.h"
#include "dng_auto_ptr.h"
#include "dng_memory_stream.h"
#include "dng_stream.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_iptc::dng_iptc ()

	:	fForceUTF8 (false)

	,	fTitle ()

	,	fUrgency (-1)

	,	fCategory ()

	,	fSupplementalCategories ()

	,	fKeywords ()

	,	fInstructions ()

	,	fDateTimeCreated ()

	,	fAuthor          ()
	,	fAuthorsPosition ()

	,	fCity        ()
	,	fState       ()
	,	fCountry     ()
	,	fCountryCode ()

	,	fLocation ()

	,	fTransmissionReference ()

	,	fHeadline ()

	,	fCredit ()

	,	fSource ()

	,	fCopyrightNotice ()

	,	fDescription       ()
	,	fDescriptionWriter ()

	{

	}

/*****************************************************************************/

dng_iptc::~dng_iptc ()
	{

	}

/*****************************************************************************/

bool dng_iptc::IsEmpty () const
	{

	if (fTitle.NotEmpty ())
		{
		return false;
		}

	if (fUrgency >= 0)
		{
		return false;
		}

	if (fCategory.NotEmpty ())
		{
		return false;
		}

	if (fSupplementalCategories.Count () > 0)
		{
		return false;
		}

	if (fKeywords.Count () > 0)
		{
		return false;
		}

	if (fInstructions.NotEmpty ())
		{
		return false;
		}

	if (fDateTimeCreated.IsValid ())
		{
		return false;
		}

	if (fAuthor         .NotEmpty () ||
		fAuthorsPosition.NotEmpty ())
		{
		return false;
		}

	if (fCity   .NotEmpty () ||
		fState  .NotEmpty () ||
		fCountry.NotEmpty ())
		{
		return false;
		}

	if (fCountryCode.NotEmpty ())
		{
		return false;
		}

	if (fLocation.NotEmpty ())
		{
		return false;
		}

	if (fTransmissionReference.NotEmpty ())
		{
		return false;
		}

	if (fHeadline.NotEmpty ())
		{
		return false;
		}

	if (fCredit.NotEmpty ())
		{
		return false;
		}

	if (fSource.NotEmpty ())
		{
		return false;
		}

	if (fCopyrightNotice.NotEmpty ())
		{
		return false;
		}

	if (fDescription      .NotEmpty () ||
		fDescriptionWriter.NotEmpty ())
		{
		return false;
		}

	return true;

	}

/*****************************************************************************/

void dng_iptc::ParseString (dng_stream &stream,
						    dng_string &s,
						    CharSet charSet)
	{

	uint32 length = stream.Get_uint16 ();

	dng_memory_data buffer (length + 1);

	char *c = buffer.Buffer_char ();

	stream.Get (c, length);

	c [length] = 0;

	switch (charSet)
		{

		case kCharSetUTF8:
			{
			s.Set_UTF8 (c);
			break;
			}

		default:
		{
		    s.Set_SystemEncoding (c);
		    break;
		}

		}

	s.SetLineEndingsToNewLines ();

	s.StripLowASCII ();

	s.TrimTrailingBlanks ();

	}

/*****************************************************************************/

void dng_iptc::Parse (const void *blockData,
					  uint32 blockSize,
					  uint64 offsetInOriginalFile)
	{

	dng_stream stream (blockData,
					   blockSize,
					   offsetInOriginalFile);

	stream.SetBigEndian ();

	CharSet charSet = kCharSetUnknown;

	uint64 nextOffset = stream.Position ();

	while (nextOffset + 5 < stream.Length ())
		{

		stream.SetReadPosition (nextOffset);

		uint8 firstByte = stream.Get_uint8 ();

		if (firstByte != 0x1C) break;

		uint8  record   = stream.Get_uint8  ();
		uint8  dataSet  = stream.Get_uint8  ();
		uint32 dataSize = stream.Get_uint16 ();

		nextOffset = stream.Position () + dataSize;

		if (record == 1)
			{

			switch (dataSet)
				{

				case 90:
					{

					if (dataSize == 3)
						{

						uint32 byte1 = stream.Get_uint8 ();
						uint32 byte2 = stream.Get_uint8 ();
						uint32 byte3 = stream.Get_uint8 ();

						if (byte1 == 27 /* Escape */ &&
							byte2 == 0x25 &&
							byte3 == 0x47)
							{

							charSet = kCharSetUTF8;

							fForceUTF8 = true;

							}

						}

					break;

					}

				default:
					break;

				}

			}

		else if (record == 2)
			{

			stream.SetReadPosition (stream.Position () - 2);

			switch ((DataSet) dataSet)
				{

				case kObjectNameSet:
					{
					ParseString (stream, fTitle, charSet);
					break;
					}

				case kUrgencySet:
					{

					int32 size = stream.Get_uint16 ();

					if (size == 1)
						{

						char c = stream.Get_int8 ();

						if (c >= '0' && c <= '9')
							{
							fUrgency = c - '0';
							}

						}

					break;

					}

				case kCategorySet:
					{
					ParseString (stream, fCategory, charSet);
					break;
					}

				case kSupplementalCategoriesSet:
					{

					dng_string category;

					ParseString (stream, category, charSet);

					if (category.NotEmpty ())
						{
						fSupplementalCategories.Append (category);
						}

					break;

					}

				case kKeywordsSet:
					{

					dng_string keyword;

					ParseString (stream, keyword, charSet);

					if (keyword.NotEmpty ())
						{
						fKeywords.Append (keyword);
						}

					break;

					}

				case kSpecialInstructionsSet:
					{
					ParseString (stream, fInstructions, charSet);
					break;
					}

				case kDateCreatedSet:
					{

					uint32 length = stream.Get_uint16 ();

					if (length == 8)
						{

						char date [9];

						stream.Get (date, 8);

						date [8] = 0;

						fDateTimeCreated.Decode_IPTC_Date (date);

						}

					break;

					}

				case kTimeCreatedSet:
					{

					uint32 length = stream.Get_uint16 ();

					if (length == 11)
						{

						char time [12];

						stream.Get (time, 11);

						time [11] = 0;

						fDateTimeCreated.Decode_IPTC_Time (time);

						}

					break;

					}

				case kBylineSet:
					{
					ParseString (stream, fAuthor, charSet);
					break;
					}

				case kBylineTitleSet:
					{
					ParseString (stream, fAuthorsPosition, charSet);
					break;
					}

				case kCitySet:
					{
					ParseString (stream, fCity, charSet);
					break;
					}

				case kProvinceStateSet:
					{
					ParseString (stream, fState, charSet);
					break;
					}

				case kCountryNameSet:
					{
					ParseString (stream, fCountry, charSet);
					break;
					}

				case kCountryCodeSet:
					{
					ParseString (stream, fCountryCode, charSet);
					break;
					}

				case kSublocationSet:
					{
					ParseString (stream, fLocation, charSet);
					break;
					}

				case kOriginalTransmissionReferenceSet:
					{
					ParseString (stream, fTransmissionReference, charSet);
					break;
					}

				case kHeadlineSet:
					{
					ParseString (stream, fHeadline, charSet);
					break;
					}

				case kCreditSet:
					{
					ParseString (stream, fCredit, charSet);
					break;
					}

				case kSourceSet:
					{
					ParseString (stream, fSource, charSet);
					break;
					}

				case kCopyrightNoticeSet:
					{
					ParseString (stream, fCopyrightNotice, charSet);
					break;
					}

				case kCaptionSet:
					{
					ParseString (stream, fDescription, charSet);
					break;
					}

				case kCaptionWriterSet:
					{
					ParseString (stream, fDescriptionWriter, charSet);
					break;
					}

				// All other IPTC records are not part of the IPTC core
				// and/or are not kept in sync with XMP tags, so we ignore
				// them.

				default:
					break;

				}

			}

		}

	}

/*****************************************************************************/

void dng_iptc::SpoolString (dng_stream &stream,
							const dng_string &s,
							uint8 dataSet,
							uint32 maxChars,
							CharSet charSet)
	{

	if (s.IsEmpty ())
		{
		return;
		}

	stream.Put_uint16 (0x1C02);
	stream.Put_uint8  (dataSet);

	dng_string ss (s);

	ss.SetLineEndingsToReturns ();

	if (charSet == kCharSetUTF8)
		{

		// UTF-8 encoding.

		if (ss.Length () > maxChars)
			{
			ss.Truncate (maxChars);
			}

		uint32 len = ss.Length ();

		stream.Put_uint16 ((uint16) len);

		stream.Put (ss.Get (), len);

		}

	else
		{

		// System character set encoding.

		dng_memory_data buffer;

		uint32 len = ss.Get_SystemEncoding (buffer);

		if (len > maxChars)
			{

			uint32 lower = 0;
			uint32 upper = ss.Length () - 1;

			while (upper > lower)
				{

				uint32 middle = (upper + lower + 1) >> 1;

				dng_string sss (ss);

				sss.Truncate (middle);

				len = sss.Get_SystemEncoding (buffer);

				if (len <= maxChars)
					{

					lower = middle;

					}

				else
					{

					upper = middle - 1;

					}

				}

			ss.Truncate (lower);

			len = ss.Get_SystemEncoding (buffer);

			}

		stream.Put_uint16 ((uint16) len);

		stream.Put (buffer.Buffer_char (), len);

		}

	}

/*****************************************************************************/

bool dng_iptc::SafeForSystemEncoding (const dng_string &s)
	{

	return s.ValidSystemEncoding ();

	}

/*****************************************************************************/

bool dng_iptc::SafeForSystemEncoding (const dng_string_list &list)
	{

	for (uint32 j = 0; j < list.Count (); j++)
		{

		if (!SafeForSystemEncoding (list [j]))
			{

			return false;

			}

		}

	return true;

	}

/*****************************************************************************/

bool dng_iptc::SafeForSystemEncoding () const
	{

	if (!SafeForSystemEncoding (fTitle))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fCategory))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fSupplementalCategories))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fKeywords))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fInstructions))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fAuthor))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fAuthorsPosition))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fCity))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fState))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fCountry))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fCountryCode))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fLocation))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fTransmissionReference))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fHeadline))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fCredit))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fSource))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fCopyrightNotice))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fDescription))
		{
		return false;
		}

	if (!SafeForSystemEncoding (fDescriptionWriter))
		{
		return false;
		}

	return true;

	}

/*****************************************************************************/

dng_memory_block * dng_iptc::Spool (dng_memory_allocator &allocator,
									bool padForTIFF)
	{

	uint32 j;

	char s [64];

	dng_memory_stream stream (allocator, NULL, 2048);

	stream.SetBigEndian ();

	// Figure out character set to use.  Due to bugs in Photoshop CS2 and
	// before, we only write UTF-8 in cases where it is required to preserve
	// all the characters.  It is not ideal since it makes the files not
	// portable across systems with different encodings.  Perhaps we can
	// switch to using UTF-8 in all cases in the CS4 timeframe.

	CharSet charSet = SafeForSystemEncoding () ? kCharSetUnknown
											   : kCharSetUTF8;

	// Override to force UTF8

	if (fForceUTF8)
		{
		charSet = kCharSetUTF8;
		}

	// UTF-8 encoding marker.

	if (charSet == kCharSetUTF8)
		{

		stream.Put_uint16 (0x1C01);
		stream.Put_uint8  (90);
		stream.Put_uint16 (3);
		stream.Put_uint8  (27);
		stream.Put_uint8  (0x25);
		stream.Put_uint8  (0x47);

		}

	stream.Put_uint16 (0x1C02);
	stream.Put_uint8  (kRecordVersionSet);
	stream.Put_uint16 (2);
	stream.Put_uint16 (2);

	SpoolString (stream,
				 fTitle,
				 kObjectNameSet,
				 64,
				 charSet);

	if (fUrgency >= 0)
		{

		sprintf (s, "%1u", fUrgency);

		stream.Put_uint16 (0x1C02);
		stream.Put_uint8  (kUrgencySet);

		stream.Put_uint16 (1);

		stream.Put (s, 1);

		}

	SpoolString (stream,
				 fCategory,
				 kCategorySet,
				 3,
				 charSet);

	for (j = 0; j < fSupplementalCategories.Count (); j++)
		{

		SpoolString (stream,
				 	 fSupplementalCategories [j],
				     kSupplementalCategoriesSet,
				     32,
					 charSet);

		}

	for (j = 0; j < fKeywords.Count (); j++)
		{

		SpoolString (stream,
				 	 fKeywords [j],
				     kKeywordsSet,
				     64,
					 charSet);

		}

	SpoolString (stream,
				 fInstructions,
				 kSpecialInstructionsSet,
				 255,
				 charSet);

	if (fDateTimeCreated.IsValid ())
		{

		dng_string dateString = fDateTimeCreated.Encode_IPTC_Date ();

		if (dateString.NotEmpty ())
			{

			DNG_ASSERT (dateString.Length () == 8, "Wrong length IPTC date");

			stream.Put_uint16 (0x1C02);
			stream.Put_uint8  (kDateCreatedSet);

			stream.Put_uint16 (8);

			stream.Put (dateString.Get (), 8);

			}

		dng_string timeString = fDateTimeCreated.Encode_IPTC_Time ();

		if (timeString.NotEmpty ())
			{

			DNG_ASSERT (timeString.Length () == 11, "Wrong length IPTC time");

			stream.Put_uint16 (0x1C02);
			stream.Put_uint8  (kTimeCreatedSet);

			stream.Put_uint16 (11);

			stream.Put (timeString.Get (), 11);

			}

		}

	SpoolString (stream,
				 fAuthor,
				 kBylineSet,
				 32,
				 charSet);

	SpoolString (stream,
				 fAuthorsPosition,
				 kBylineTitleSet,
				 32,
				 charSet);

	SpoolString (stream,
				 fCity,
				 kCitySet,
				 32,
				 charSet);

	SpoolString (stream,
				 fLocation,
				 kSublocationSet,
				 32,
				 charSet);

	SpoolString (stream,
				 fState,
				 kProvinceStateSet,
				 32,
				 charSet);

	if (fCountryCode.Length () == 3)
		{

		SpoolString (stream,
					 fCountryCode,
					 kCountryCodeSet,
					 3,
					 charSet);

		}

	SpoolString (stream,
				 fCountry,
				 kCountryNameSet,
				 64,
				 charSet);

	SpoolString (stream,
				 fTransmissionReference,
				 kOriginalTransmissionReferenceSet,
				 32,
				 charSet);

	SpoolString (stream,
				 fHeadline,
				 kHeadlineSet,
				 255,
				 charSet);

	SpoolString (stream,
				 fCredit,
				 kCreditSet,
				 32,
				 charSet);

	SpoolString (stream,
				 fSource,
				 kSourceSet,
				 32,
				 charSet);

	SpoolString (stream,
				 fCopyrightNotice,
				 kCopyrightNoticeSet,
				 128,
				 charSet);

	SpoolString (stream,
				 fDescription,
				 kCaptionSet,
				 2000,
				 charSet);

	SpoolString (stream,
				 fDescriptionWriter,
				 kCaptionWriterSet,
				 32,
				 charSet);

	if (padForTIFF)
		{

		while (stream.Length () & 3)
			{
			stream.Put_uint8 (0);
			}

		}

	stream.Flush ();

	return stream.AsMemoryBlock (allocator);

	}

/*****************************************************************************/
