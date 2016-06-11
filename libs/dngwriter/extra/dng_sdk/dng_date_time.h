/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_date_time.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Functions and classes for working with dates and times in DNG files.
 */

/*****************************************************************************/

#ifndef __dng_date_time__
#define __dng_date_time__

/*****************************************************************************/

#include "dng_classes.h"
#include "dng_string.h"
#include "dng_types.h"

/*****************************************************************************/

/// \brief Class for holding a date/time and converting to and from relevant
/// date/time formats

class dng_date_time
	{

	public:

		uint32 fYear;
		uint32 fMonth;
		uint32 fDay;
		uint32 fHour;
		uint32 fMinute;
		uint32 fSecond;

	public:

		/// Construct an invalid date/time

		dng_date_time ();

		/// Construct a date/time with specific values.
		/// \param year Year to use as actual integer value, such as 2006.
		/// \param month Month to use from 1 - 12, where 1 is January.
		/// \param day Day of month to use from 1 -31, where 1 is the first.
		/// \param hour Hour of day to use from 0 - 23, where 0 is midnight.
		/// \param minute Minute of hour to use from 0 - 59.
		/// \param second Second of minute to use from 0 - 59.

		dng_date_time (uint32 year,
					   uint32 month,
					   uint32 day,
					   uint32 hour,
					   uint32 minute,
					   uint32 second);

		/// Predicate to determine if a date is valid.
		/// \retval true if all fields are within range.

		bool IsValid () const;

		/// Predicate to determine if a date is invalid.
		/// \retval true if any field is out of range.

		bool NotValid () const
			{
			return !IsValid ();
			}

		/// Equal operator.

		bool operator== (const dng_date_time &dt) const
			{
			return fYear   == dt.fYear   &&
				   fMonth  == dt.fMonth  &&
				   fDay    == dt.fDay    &&
				   fHour   == dt.fHour   &&
				   fMinute == dt.fMinute &&
				   fSecond == dt.fSecond;
			}

		// Not-equal operator.

		bool operator!= (const dng_date_time &dt) const
			{
			return !(*this == dt);
			}

		/// Set date to an invalid value.

		void Clear ();

		/// Parse an EXIF format date string.
		/// \param s Input date string to parse.
		/// \retval true if date was parsed successfully and date is valid.

		bool Parse (const char *s);

	};

/*****************************************************************************/

/// \brief Class for holding a time zone.

class dng_time_zone
	{

	private:

		enum
			{

			kMaxOffsetHours = 15,
			kMinOffsetHours = -kMaxOffsetHours,

			kMaxOffsetMinutes = kMaxOffsetHours * 60,
			kMinOffsetMinutes = kMinOffsetHours * 60,

			kInvalidOffset = kMinOffsetMinutes - 1

			};

		// Offset from GMT in minutes.  Positive numbers are
		// ahead of GMT, negative number are behind GMT.

		int32 fOffsetMinutes;

	public:

		dng_time_zone ()
			:	fOffsetMinutes (kInvalidOffset)
			{
			}

		void Clear ()
			{
			fOffsetMinutes = kInvalidOffset;
			}

		void SetOffsetHours (int32 offset)
			{
			fOffsetMinutes = offset * 60;
			}

		void SetOffsetMinutes (int32 offset)
			{
			fOffsetMinutes = offset;
			}

		void SetOffsetSeconds (int32 offset)
			{
			fOffsetMinutes = (offset > 0) ? ((offset + 30) / 60)
										  : ((offset - 30) / 60);
			}

		bool IsValid () const
			{
			return fOffsetMinutes >= kMinOffsetMinutes &&
				   fOffsetMinutes <= kMaxOffsetMinutes;
			}

		bool NotValid () const
			{
			return !IsValid ();
			}

		int32 OffsetMinutes () const
			{
			return fOffsetMinutes;
			}

		bool IsExactHourOffset () const
			{
			return IsValid () && ((fOffsetMinutes % 60) == 0);
			}

		int32 ExactHourOffset () const
			{
			return fOffsetMinutes / 60;
			}

		dng_string Encode_ISO_8601 () const;

	};

/*****************************************************************************/

/// \brief Class for holding complete data/time/zone information.

class dng_date_time_info
	{

	private:

		// Is only the date valid and not the time?

		bool fDateOnly;

		// Date and time.

		dng_date_time fDateTime;

		// Subseconds string (stored in a separate tag in EXIF).

		dng_string fSubseconds;

		// Time zone, if known.

		dng_time_zone fTimeZone;

	public:

		dng_date_time_info ();

		bool IsValid () const;

		bool NotValid () const
			{
			return !IsValid ();
			}

		void Clear ()
			{
			*this = dng_date_time_info ();
			}

		const dng_date_time & DateTime () const
			{
			return fDateTime;
			}

		void SetDateTime (const dng_date_time &dt)
			{
			fDateOnly = false;
			fDateTime = dt;
			}

		const dng_string & Subseconds () const
			{
			return fSubseconds;
			}

		void SetSubseconds (const dng_string &s)
			{
			fSubseconds = s;
			}

		const dng_time_zone & TimeZone () const
			{
			return fTimeZone;
			}

		void SetZone (const dng_time_zone &zone)
			{
			fTimeZone = zone;
			}

		void Decode_ISO_8601 (const char *s);

		dng_string Encode_ISO_8601 () const;

		void Decode_IPTC_Date (const char *s);

		dng_string Encode_IPTC_Date () const;

		void Decode_IPTC_Time (const char *s);

		dng_string Encode_IPTC_Time () const;

	private:

		void SetDate (uint32 year,
					  uint32 month,
					  uint32 day);

		void SetTime (uint32 hour,
					  uint32 minute,
					  uint32 second);

	};

/*****************************************************************************/

/// Get the current date/time and timezone.
/// \param info Receives current data/time/zone.

void CurrentDateTimeAndZone (dng_date_time_info &info);

/*****************************************************************************/

/// Convert UNIX "seconds since Jan 1, 1970" time to a dng_date_time

void DecodeUnixTime (uint32 unixTime, dng_date_time &dt);

/*****************************************************************************/

/// Return timezone of current location at a given date.
/// \param dt Date at which to compute timezone difference. (For example, used
/// to determine Daylight Savings, etc.)
/// \retval Time zone for date/time dt.

dng_time_zone LocalTimeZone (const dng_date_time &dt);

/*****************************************************************************/

/// Tag to encode date represenation format

enum dng_date_time_format
	{
	dng_date_time_format_unknown            = 0, /// Date format not known
	dng_date_time_format_exif               = 1, /// EXIF date string
	dng_date_time_format_unix_little_endian = 2, /// 32-bit UNIX time as 4-byte little endian
	dng_date_time_format_unix_big_endian    = 3  /// 32-bit UNIX time as 4-byte big endian
	};

/*****************************************************************************/

/// \brief Store file offset from which date was read.
///
/// Used internally by Adobe to update date in original file.
/// \warning Use at your own risk.

class dng_date_time_storage_info
	{

	private:

		uint64 fOffset;

		dng_date_time_format fFormat;

	public:

		/// The default constructor initializes to an invalid state.

		dng_date_time_storage_info ();

		/// Construct with file offset and date format.

		dng_date_time_storage_info (uint64 offset,
									dng_date_time_format format);

		/// Predicate to determine if an offset is valid.
		/// \retval true if offset is valid.

		bool IsValid () const;

		// The accessors throw if the data is not valid.

		/// Getter for offset in file.
		/// \exception dng_exception with fErrorCode equal to dng_error_unknown
		/// if offset is not valid.

		uint64 Offset () const;

		/// Get for format date was originally stored in file. Throws a
		/// dng_error_unknown exception if offset is invalid.
		/// \exception dng_exception with fErrorCode equal to dng_error_unknown
		/// if offset is not valid.

		dng_date_time_format Format () const;

	};

/*****************************************************************************/

#endif

/*****************************************************************************/
