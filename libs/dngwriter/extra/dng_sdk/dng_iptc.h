/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_iptc.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Support for IPTC metadata within DNG files.
 */

/*****************************************************************************/

#ifndef __dng_iptc__
#define __dng_iptc__

/*****************************************************************************/

#include "dng_date_time.h"
#include "dng_string.h"
#include "dng_string_list.h"

/*****************************************************************************/

/// \brief Class for reading and holding IPTC metadata associated with a DNG file.
///
/// See the \ref spec_iptc "IPTC specification"
/// for information on member fields of this class.

class dng_iptc
	{

	public:

		bool fForceUTF8;

		dng_string fTitle;

		int32 fUrgency;

		dng_string fCategory;

		dng_string_list fSupplementalCategories;

		dng_string_list fKeywords;

		dng_string fInstructions;

		dng_date_time_info fDateTimeCreated;

		dng_string fAuthor;
		dng_string fAuthorsPosition;

		dng_string fCity;
		dng_string fState;
		dng_string fCountry;
		dng_string fCountryCode;

		dng_string fLocation;

		dng_string fTransmissionReference;

		dng_string fHeadline;

		dng_string fCredit;

		dng_string fSource;

		dng_string fCopyrightNotice;

		dng_string fDescription;
		dng_string fDescriptionWriter;

	protected:

		enum DataSet
			{
			kRecordVersionSet					= 0,
			kObjectNameSet						= 5,
			kUrgencySet							= 10,
			kCategorySet						= 15,
			kSupplementalCategoriesSet			= 20,
			kKeywordsSet						= 25,
			kSpecialInstructionsSet				= 40,
			kDateCreatedSet						= 55,
			kTimeCreatedSet						= 60,
			kBylineSet							= 80,
			kBylineTitleSet						= 85,
			kCitySet							= 90,
			kSublocationSet						= 92,
			kProvinceStateSet					= 95,
			kCountryCodeSet						= 100,
			kCountryNameSet						= 101,
			kOriginalTransmissionReferenceSet	= 103,
			kHeadlineSet						= 105,
			kCreditSet							= 110,
			kSourceSet							= 115,
			kCopyrightNoticeSet					= 116,
			kCaptionSet							= 120,
			kCaptionWriterSet					= 122
			};

		enum CharSet
			{
			kCharSetUnknown						= 0,
			kCharSetUTF8						= 1
			};

	public:

		dng_iptc ();

		virtual ~dng_iptc ();

		/// Test if IPTC metadata exists.
		/// \retval true if no IPTC metadata exists for this DNG.

		bool IsEmpty () const;

		/// Test if IPTC metadata exists.
		/// \retval true if IPTC metadata exists for this DNG.

		bool NotEmpty () const
			{
			return !IsEmpty ();
			}

		/// Parse a complete block of IPTC data.
		/// \param blockData The block of IPTC data.
		/// \param blockSize Size in bytes of data block.
		/// \param offsetInOriginalFile Used to enable certain file patching operations such as updating date/time in place.

		void Parse (const void *blockData,
					uint32 blockSize,
					uint64 offsetInOriginalFile);

		/// Serialize IPTC data to a memory block.
		/// \param allocator Memory allocator used to acquire memory block.
		/// \param padForTIFF Forces length of block to be a multiple of four bytes in accordance with TIFF standard.
		/// \retval Memory block

		dng_memory_block * Spool (dng_memory_allocator &allocator,
								  bool padForTIFF);

	protected:

		void ParseString (dng_stream &stream,
						  dng_string &s,
						  CharSet charSet);

		void SpoolString (dng_stream &stream,
						  const dng_string &s,
						  uint8 dataSet,
						  uint32 maxChars,
						  CharSet charSet);

		static bool SafeForSystemEncoding (const dng_string &s);

		static bool SafeForSystemEncoding (const dng_string_list &list);

		bool SafeForSystemEncoding () const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
