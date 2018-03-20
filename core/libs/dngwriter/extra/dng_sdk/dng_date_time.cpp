/*****************************************************************************/
// Copyright 2006-2008 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_date_time.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_date_time.h"

#include "dng_exceptions.h"
#include "dng_mutex.h"
#include "dng_stream.h"
#include "dng_string.h"
#include "dng_utils.h"

#include <time.h>

#if qMacOS
#include <CoreServices/CoreServices.h>
#endif

#if qWinOS
#include <windows.h>
#if defined(__GNUC__)
#include <winbase.h>

/*
 * Win32 kernel time functions from Wine API.
 *
 * Copyright 1995 Martin von Loewis and Cameron Heide
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define FILETIME2LL( pft, ll) ll = (((LONGLONG)((pft)->dwHighDateTime))<<32) + (pft)-> dwLowDateTime;
#define LL2FILETIME( ll, pft ) (pft)->dwLowDateTime = (UINT)(ll); (pft)->dwHighDateTime = (UINT)((ll) >> 32);

const int MonthLengths[2][12] =
{
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
        { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
inline int IsLeapYear(int Year)
{
        return Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0) ? 1 : 0;
};
BOOL TIME_GetTimezoneBias(const TIME_ZONE_INFORMATION *pTZinfo,
                          FILETIME *lpFileTime, BOOL islocal, LONG *pBias);
BOOL TIME_CompTimeZoneID(const TIME_ZONE_INFORMATION *pTZinfo,
                         FILETIME *lpFileTime, BOOL islocal);
int TIME_DayLightCompareDate(const SYSTEMTIME *date,
                             const SYSTEMTIME *compareDate);
BOOL TzSpecificLocalTimeToSystemTime(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
                                     LPSYSTEMTIME lpLocalTime,
                                     LPSYSTEMTIME lpUniversalTime);
#endif /* defined(__GNUC__) */
#endif

/******************************************************************************/

dng_date_time::dng_date_time ()

	:	fYear   (0)
	,	fMonth  (0)
	,	fDay    (0)
	,	fHour   (0)
	,	fMinute (0)
	,	fSecond (0)

	{

	}

/******************************************************************************/

dng_date_time::dng_date_time (uint32 year,
					  		  uint32 month,
					  		  uint32 day,
					  		  uint32 hour,
					  		  uint32 minute,
					  		  uint32 second)

	:	fYear   (year)
	,	fMonth  (month)
	,	fDay    (day)
	,	fHour   (hour)
	,	fMinute (minute)
	,	fSecond (second)

	{

	}

/******************************************************************************/

bool dng_date_time::IsValid () const
	{

	return fYear   >= 1 && fYear   <= 9999 &&
		   fMonth  >= 1 && fMonth  <= 12   &&
		   fDay    >= 1 && fDay    <= 31   &&
		   fHour   >= 0 && fHour   <= 23   &&
		   fMinute >= 0 && fMinute <= 59   &&
		   fSecond >= 0 && fSecond <= 59;

	}

/*****************************************************************************/

void dng_date_time::Clear ()
	{

	*this = dng_date_time ();

	}

/*****************************************************************************/

static uint32 DateTimeParseU32 (const char *&s)
	{

	uint32 x = 0;

	while (*s == ' ' || *s == ':')
		s++;

	while (*s >= '0' && *s <= '9')
		{
		x = x * 10 + (uint32) (*(s++) - '0');
		}

	return x;

	}

/*****************************************************************************/

bool dng_date_time::Parse (const char *s)
	{

	fYear   = DateTimeParseU32 (s);
	fMonth  = DateTimeParseU32 (s);
	fDay    = DateTimeParseU32 (s);
	fHour   = DateTimeParseU32 (s);
	fMinute = DateTimeParseU32 (s);
	fSecond = DateTimeParseU32 (s);

	return IsValid ();

	}

/*****************************************************************************/

dng_string dng_time_zone::Encode_ISO_8601 () const
	{

	dng_string result;

	if (IsValid ())
		{

		if (OffsetMinutes () == 0)
			{

			result.Set ("Z");

			}

		else
			{

			char s [64];

			int offset = OffsetMinutes ();

			if (offset > 0)
				{

				sprintf (s, "+%02d:%02d", offset / 60, offset % 60);

				}

			else
				{

				offset = -offset;

				sprintf (s, "-%02d:%02d", offset / 60, offset % 60);

				}

			result.Set (s);

			}

		}

	return result;

	}

/*****************************************************************************/

dng_date_time_info::dng_date_time_info ()

	:	fDateOnly   (true)
	,	fDateTime   ()
	,	fSubseconds ()
	,	fTimeZone   ()

	{

	}

/*****************************************************************************/

bool dng_date_time_info::IsValid () const
	{

	return fDateTime.IsValid ();

	}

/*****************************************************************************/

void dng_date_time_info::SetDate (uint32 year,
								  uint32 month,
								  uint32 day)
	{

	fDateTime.fYear  = year;
	fDateTime.fMonth = month;
	fDateTime.fDay   = day;

	}

/*****************************************************************************/

void dng_date_time_info::SetTime (uint32 hour,
								  uint32 minute,
								  uint32 second)
	{

	fDateOnly = false;

	fDateTime.fHour   = hour;
	fDateTime.fMinute = minute;
	fDateTime.fSecond = second;

	}

/*****************************************************************************/

void dng_date_time_info::Decode_ISO_8601 (const char *s)
	{

	Clear ();

	uint32 len = (uint32) strlen (s);

	if (!len)
		{
		return;
		}

	unsigned year  = 0;
	unsigned month = 0;
	unsigned day   = 0;

	if (sscanf (s,
				"%u-%u-%u",
				&year,
				&month,
				&day) != 3)
		{
		return;
		}

	SetDate ((uint32) year,
			 (uint32) month,
			 (uint32) day);

	if (fDateTime.NotValid ())
		{
		Clear ();
		return;
		}

	for (uint32 j = 0; j < len; j++)
		{

		if (s [j] == 'T')
			{

			unsigned hour   = 0;
			unsigned minute = 0;
			unsigned second = 0;

			if (sscanf (s + j + 1,
						"%u:%u:%u",
						&hour,
						&minute,
						&second) == 3)
				{

				SetTime ((uint32) hour,
						 (uint32) minute,
						 (uint32) second);

				if (fDateTime.NotValid ())
					{
					Clear ();
					return;
					}

				for (uint32 k = j + 1; k < len; k++)
					{

					if (s [k] == '.')
						{

						while (++k < len && s [k] >= '0' && s [k] <= '9')
							{

							char ss [2];

							ss [0] = s [k];
							ss [1] = 0;

							fSubseconds.Append (ss);

							}

						break;

						}

					}

				for (uint32 k = j + 1; k < len; k++)
					{

					if (s [k] == 'Z')
						{

						fTimeZone.SetOffsetMinutes (0);

						break;

						}

					if (s [k] == '+' || s [k] == '-')
						{

						int32 sign = (s [k] == '-' ? -1 : 1);

						unsigned tzhour = 0;
						unsigned tzmin  = 0;

						if (sscanf (s + k + 1,
									"%u:%u",
									&tzhour,
									&tzmin) > 0)
							{

							fTimeZone.SetOffsetMinutes (sign * (tzhour * 60 + tzmin));

							}

						break;

						}

					}

				}

			break;

			}

		}

	}

/*****************************************************************************/

dng_string dng_date_time_info::Encode_ISO_8601 () const
	{

	dng_string result;

	if (IsValid ())
		{

		char s [256];

		sprintf (s,
				 "%04u-%02u-%02u",
				 (unsigned) fDateTime.fYear,
				 (unsigned) fDateTime.fMonth,
				 (unsigned) fDateTime.fDay);

		result.Set (s);

		if (!fDateOnly)
			{

			sprintf (s,
					 "T%02u:%02u:%02u",
					 (unsigned) fDateTime.fHour,
					 (unsigned) fDateTime.fMinute,
					 (unsigned) fDateTime.fSecond);

			result.Append (s);

			if (fSubseconds.NotEmpty ())
				{

				bool subsecondsValid = true;

				uint32 len = fSubseconds.Length ();

				for (uint32 index = 0; index < len; index++)
					{

					if (fSubseconds.Get () [index] < '0' ||
						fSubseconds.Get () [index] > '9')
						{
						subsecondsValid = false;
						break;
						}

					}

				if (subsecondsValid)
					{
					result.Append (".");
					result.Append (fSubseconds.Get ());
					}

				}

			// Kludge: Early versions of the XMP toolkit assume Zulu time
			// if the time zone is missing.  It is safer for fill in the
			// local time zone.

			dng_time_zone tempZone = fTimeZone;

			if (tempZone.NotValid ())
				{
				tempZone = LocalTimeZone (fDateTime);
				}

			result.Append (tempZone.Encode_ISO_8601 ().Get ());

			}

		}

	return result;

	}

/*****************************************************************************/

void dng_date_time_info::Decode_IPTC_Date (const char *s)
	{

	if (strlen (s) == 8)
		{

		unsigned year   = 0;
		unsigned month  = 0;
		unsigned day    = 0;

		if (sscanf (s,
					"%4u%2u%2u",
					&year,
					&month,
					&day) == 3)
			{

			SetDate ((uint32) year,
					 (uint32) month,
					 (uint32) day);

			}

		}

	}

/*****************************************************************************/

dng_string dng_date_time_info::Encode_IPTC_Date () const
	{

	dng_string result;

	if (IsValid ())
		{

		char s [64];

		sprintf (s,
			     "%04u%02u%02u",
			     fDateTime.fYear,
			     fDateTime.fMonth,
			     fDateTime.fDay);

		result.Set (s);

		}

	return result;

	}

/*****************************************************************************/

void dng_date_time_info::Decode_IPTC_Time (const char *s)
	{

	if (strlen (s) == 11)
		{

		char time [12];

		memcpy (time, s, sizeof (time));

		if (time [6] == '+' ||
			time [6] == '-')
			{

			int tzsign = (time [6] == '-') ? -1 : 1;

			time [6] = 0;

			unsigned hour   = 0;
			unsigned minute = 0;
			unsigned second = 0;
			unsigned tzhour = 0;
			unsigned tzmin  = 0;

			if (sscanf (time,
						"%2u%2u%2u",
						&hour,
						&minute,
						&second) == 3 &&
				sscanf (time + 7,
						"%2u%2u",
						&tzhour,
						&tzmin) == 2)
				{

				dng_time_zone zone;

				zone.SetOffsetMinutes (tzsign * (tzhour * 60 + tzmin));

				if (zone.IsValid ())
					{

					SetTime ((uint32) hour,
							 (uint32) minute,
							 (uint32) second);

					SetZone (zone);

					}

				}

			}

		}

	}

/*****************************************************************************/

dng_string dng_date_time_info::Encode_IPTC_Time () const
	{

	dng_string result;

	if (IsValid () && !fDateOnly && fTimeZone.IsValid ())
		{

		char s [64];

		sprintf (s,
				 "%02u%02u%02u%c%02u%02u",
				 fDateTime.fHour,
				 fDateTime.fMinute,
				 fDateTime.fSecond,
				 fTimeZone.OffsetMinutes () >= 0 ? '+' : '-',
				 Abs_int32 (fTimeZone.OffsetMinutes ()) / 60,
				 Abs_int32 (fTimeZone.OffsetMinutes ()) % 60);

		result.Set (s);

		}

	return result;

	}

/*****************************************************************************/

static dng_mutex gDateTimeMutex ("gDateTimeMutex");

/*****************************************************************************/

void CurrentDateTimeAndZone (dng_date_time_info &info)
	{

	time_t sec;

	time (&sec);

	tm t;
	tm zt;

		{

		dng_lock_mutex lock (&gDateTimeMutex);

		t  = *localtime (&sec);
		zt = *gmtime    (&sec);

		}

	dng_date_time dt;

	dt.fYear   = t.tm_year + 1900;
	dt.fMonth  = t.tm_mon + 1;
	dt.fDay    = t.tm_mday;
	dt.fHour   = t.tm_hour;
	dt.fMinute = t.tm_min;
	dt.fSecond = t.tm_sec;

	info.SetDateTime (dt);

	int tzHour = t.tm_hour - zt.tm_hour;
	int tzMin  = t.tm_min  - zt.tm_min;

	bool zonePositive = (t.tm_year >  zt.tm_year) ||
						(t.tm_year == zt.tm_year && t.tm_yday >  zt.tm_yday) ||
						(t.tm_year == zt.tm_year && t.tm_yday == zt.tm_yday && tzHour > 0) ||
						(t.tm_year == zt.tm_year && t.tm_yday == zt.tm_yday && tzHour == 0 && tzMin >= 0);

	tzMin += tzHour * 60;

	if (zonePositive)
		{

		while (tzMin < 0)
			tzMin += 24 * 60;

		}

	else
		{

		while (tzMin > 0)
			tzMin -= 24 * 60;

		}

	dng_time_zone zone;

	zone.SetOffsetMinutes (tzMin);

	info.SetZone (zone);

	}

/*****************************************************************************/

void DecodeUnixTime (uint32 unixTime, dng_date_time &dt)
	{

	time_t sec = (time_t) unixTime;

	tm t;

		{

		dng_lock_mutex lock (&gDateTimeMutex);

		#if qMacOS && !defined(__MACH__)

		// Macintosh CFM stores time in local time zone.

		tm *tp = localtime (&sec);

		#else

		// Macintosh Mach-O and Windows stores time in Zulu time.

		tm *tp = gmtime (&sec);

		#endif

		if (!tp)
			{
			dt.Clear ();
			return;
			}

		t = *tp;

		}

	dt.fYear   = t.tm_year + 1900;
	dt.fMonth  = t.tm_mon + 1;
	dt.fDay    = t.tm_mday;
	dt.fHour   = t.tm_hour;
	dt.fMinute = t.tm_min;
	dt.fSecond = t.tm_sec;

	}

/*****************************************************************************/

dng_time_zone LocalTimeZone (const dng_date_time &dt)
	{

	dng_time_zone result;

	if (dt.IsValid ())
		{

		#if qMacOS

		CFTimeZoneRef zoneRef = CFTimeZoneCopyDefault ();

		if (zoneRef)
			{

			CFGregorianDate gregDate;

			gregDate.year   = dt.fYear;
			gregDate.month  = dt.fMonth;
			gregDate.day    = dt.fDay;
			gregDate.hour   = dt.fHour;
			gregDate.minute = dt.fMinute;
			gregDate.second = dt.fSecond;

			CFAbsoluteTime absTime = CFGregorianDateGetAbsoluteTime (gregDate, zoneRef);

			CFTimeInterval secondsDelta = CFTimeZoneGetSecondsFromGMT (zoneRef, absTime);

			CFRelease (zoneRef);

			result.SetOffsetSeconds (secondsDelta);

			if (result.IsValid ())
				{
				return result;
				}

			}

		#endif

		#if qWinOS

		if (GetTimeZoneInformation          != NULL &&
			SystemTimeToTzSpecificLocalTime != NULL &&
		    SystemTimeToFileTime            != NULL)
			{

			TIME_ZONE_INFORMATION tzInfo;

			DWORD x = GetTimeZoneInformation (&tzInfo);

			SYSTEMTIME localST;

			memset (&localST, 0, sizeof (localST));

			localST.wYear   = (WORD) dt.fYear;
			localST.wMonth  = (WORD) dt.fMonth;
			localST.wDay    = (WORD) dt.fDay;
			localST.wHour   = (WORD) dt.fHour;
			localST.wMinute = (WORD) dt.fMinute;
			localST.wSecond = (WORD) dt.fSecond;

			SYSTEMTIME utcST;

			if (TzSpecificLocalTimeToSystemTime (&tzInfo, &localST, &utcST))
				{

				FILETIME localFT;
				FILETIME utcFT;

				(void) SystemTimeToFileTime (&localST, &localFT);
				(void) SystemTimeToFileTime (&utcST  , &utcFT  );

				uint64 time1 = (((uint64) localFT.dwHighDateTime) << 32) + localFT.dwLowDateTime;
				uint64 time2 = (((uint64) utcFT  .dwHighDateTime) << 32) + utcFT  .dwLowDateTime;

				// FILETIMEs are in units to 100 ns.  Convert to seconds.

				int64 time1Sec = time1 / 10000000;
				int64 time2Sec = time2 / 10000000;

				int32 delta = (int32) (time1Sec - time2Sec);

				result.SetOffsetSeconds (delta);

				if (result.IsValid ())
					{
					return result;
					}

				}

			}

		#endif

		}

	// Figure out local time zone.

	dng_date_time_info current_info;

	CurrentDateTimeAndZone (current_info);

	result = current_info.TimeZone ();

	return result;

	}

/*****************************************************************************/

dng_date_time_storage_info::dng_date_time_storage_info ()

	:	fOffset	(kDNGStreamInvalidOffset     )
	,	fFormat	(dng_date_time_format_unknown)

	{

	}

/*****************************************************************************/

dng_date_time_storage_info::dng_date_time_storage_info (uint64 offset,
														dng_date_time_format format)

	:	fOffset	(offset)
	,	fFormat	(format)

	{

	}

/*****************************************************************************/

bool dng_date_time_storage_info::IsValid () const
	{

	return fOffset != kDNGStreamInvalidOffset;

	}

/*****************************************************************************/

uint64 dng_date_time_storage_info::Offset () const
	{

	if (!IsValid ())
		ThrowProgramError ();

	return fOffset;

	}

/*****************************************************************************/

dng_date_time_format dng_date_time_storage_info::Format () const
	{

	if (!IsValid ())
		ThrowProgramError ();

	return fFormat;

	}

/*****************************************************************************
 * Win32 kernel time functions from Wine API used to implement
 * TzSpecificLocalTimeToSystemTime() from win32 which is unavialable with MinGW.
 */

#if qWinOS && defined(__GNUC__)

/***********************************************************************
 *  TIME_GetTimezoneBias
 *
 *  Calculates the local time bias for a given time zone.
 *
 * PARAMS
 *  pTZinfo    [in]  The time zone data.
 *  lpFileTime [in]  The system or local time.
 *  islocal    [in]  It is local time.
 *  pBias      [out] The calculated bias in minutes.
 *
 * RETURNS
 *  TRUE when the time zone bias was calculated.
 */
BOOL TIME_GetTimezoneBias(const TIME_ZONE_INFORMATION *pTZinfo,
                          FILETIME *lpFileTime, BOOL islocal, LONG *pBias)
 {
    LONG bias = pTZinfo->Bias;
    DWORD tzid = TIME_CompTimeZoneID( pTZinfo, lpFileTime, islocal);

    if( tzid == TIME_ZONE_ID_INVALID)
        return FALSE;
    if (tzid == TIME_ZONE_ID_DAYLIGHT)
        bias += pTZinfo->DaylightBias;
    else if (tzid == TIME_ZONE_ID_STANDARD)
        bias += pTZinfo->StandardBias;
    *pBias = bias;
    return TRUE;
}

/***********************************************************************
 *  TIME_CompTimeZoneID
 *
 *  Computes the local time bias for a given time and time zone.
 *
 *  PARAMS
 *      pTZinfo     [in] The time zone data.
 *      lpFileTime  [in] The system or local time.
 *      islocal     [in] it is local time.
 *
 *  RETURNS
 *      TIME_ZONE_ID_INVALID    An error occurred
 *      TIME_ZONE_ID_UNKNOWN    There are no transition time known
 *      TIME_ZONE_ID_STANDARD   Current time is standard time
 *      TIME_ZONE_ID_DAYLIGHT   Current time is dayligh saving time
 */
BOOL TIME_CompTimeZoneID(const TIME_ZONE_INFORMATION *pTZinfo,
                         FILETIME *lpFileTime, BOOL islocal)
{
    int ret;
    BOOL beforeStandardDate, afterDaylightDate;
    DWORD retval = TIME_ZONE_ID_INVALID;
    LONGLONG llTime = 0; /* initialized to prevent gcc complaining */
    SYSTEMTIME SysTime;
    FILETIME ftTemp;

    if (pTZinfo->DaylightDate.wMonth != 0)
    {
        if (pTZinfo->StandardDate.wMonth == 0 ||
            pTZinfo->StandardDate.wDay<1 ||
            pTZinfo->StandardDate.wDay>5 ||
            pTZinfo->DaylightDate.wDay<1 ||
            pTZinfo->DaylightDate.wDay>5)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return TIME_ZONE_ID_INVALID;
        }

        if (!islocal) {
            FILETIME2LL( lpFileTime, llTime );
            llTime -= ( pTZinfo->Bias + pTZinfo->DaylightBias )
                * (LONGLONG)600000000;
            LL2FILETIME( llTime, &ftTemp)
            lpFileTime = &ftTemp;
        }

        FileTimeToSystemTime(lpFileTime, &SysTime);

         /* check for daylight saving */
        ret = TIME_DayLightCompareDate( &SysTime, &pTZinfo->StandardDate);
        if (ret == -2)
          return TIME_ZONE_ID_INVALID;

        beforeStandardDate = ret < 0;

        if (!islocal) {
            llTime -= ( pTZinfo->StandardBias - pTZinfo->DaylightBias )
                * (LONGLONG)600000000;
            LL2FILETIME( llTime, &ftTemp)
            FileTimeToSystemTime(lpFileTime, &SysTime);
        }

        ret = TIME_DayLightCompareDate( &SysTime, &pTZinfo->DaylightDate);
        if (ret == -2)
          return TIME_ZONE_ID_INVALID;

        afterDaylightDate = ret >= 0;

        retval = TIME_ZONE_ID_STANDARD;
        if( pTZinfo->DaylightDate.wMonth <  pTZinfo->StandardDate.wMonth ) {
            /* Northern hemisphere */
            if( beforeStandardDate && afterDaylightDate )
                retval = TIME_ZONE_ID_DAYLIGHT;
        } else    /* Down south */
            if( beforeStandardDate || afterDaylightDate )
            retval = TIME_ZONE_ID_DAYLIGHT;
    } else
        /* No transition date */
        retval = TIME_ZONE_ID_UNKNOWN;

    return retval;
}

/***********************************************************************
 *  TIME_DayLightCompareDate
 *
 * Compares two dates without looking at the year.
 *
 * PARAMS
 *   date        [in] The local time to compare.
 *   compareDate [in] The daylight saving begin or end date.
 *
 * RETURNS
 *
 *  -1 if date < compareDate
 *   0 if date == compareDate
 *   1 if date > compareDate
 *  -2 if an error occurs
 */
int TIME_DayLightCompareDate(const SYSTEMTIME *date,
                             const SYSTEMTIME *compareDate)
{
    int limit_day, dayinsecs;

    if (date->wMonth < compareDate->wMonth)
        return -1; /* We are in a month before the date limit. */

    if (date->wMonth > compareDate->wMonth)
        return 1; /* We are in a month after the date limit. */

    if (compareDate->wDayOfWeek <= 6)
    {
        WORD First;
        /* compareDate->wDay is interpreted as number of the week in the month
         * 5 means: the last week in the month */
        int weekofmonth = compareDate->wDay;
          /* calculate the day of the first DayOfWeek in the month */
        First = ( 6 + compareDate->wDayOfWeek - date->wDayOfWeek + date->wDay
               ) % 7 + 1;
        limit_day = First + 7 * (weekofmonth - 1);
        /* check needed for the 5th weekday of the month */
        if(limit_day > MonthLengths[date->wMonth==2 && IsLeapYear(date->wYear)]
                [date->wMonth - 1])
            limit_day -= 7;
    }
    else
    {
       limit_day = compareDate->wDay;
    }

    /* convert to seconds */
    limit_day = ((limit_day * 24  + compareDate->wHour) * 60 +
            compareDate->wMinute ) * 60;
    dayinsecs = ((date->wDay * 24  + date->wHour) * 60 +
            date->wMinute ) * 60 + date->wSecond;
    /* and compare */
    return dayinsecs < limit_day ? -1 :
           dayinsecs > limit_day ? 1 :
           0;   /* date is equal to the date limit. */
}

/***********************************************************************
 *              TzSpecificLocalTimeToSystemTime
 *
 *  Converts a local time to a time in utc.
 *
 * PARAMS
 *  lpTimeZoneInformation [in]  The desired time zone.
 *  lpLocalTime           [in]  The local time.
 *  lpUniversalTime       [out] The calculated utc time.
 *
 * RETURNS
 *  Success: TRUE. lpUniversalTime contains the converted time.
 *  Failure: FALSE.
 */
BOOL TzSpecificLocalTimeToSystemTime(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
                                     LPSYSTEMTIME lpLocalTime,
                                     LPSYSTEMTIME lpUniversalTime)
{
    FILETIME ft;
    LONG lBias;
    LONGLONG t;
    TIME_ZONE_INFORMATION tzinfo;

    if (lpTimeZoneInformation != NULL)
    {
        memcpy(&tzinfo, lpTimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));
    }
    else
    {
        if (GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_INVALID)
            return FALSE;
    }

    if (!SystemTimeToFileTime(lpLocalTime, &ft))
        return FALSE;
    FILETIME2LL( &ft, t)
    if (!TIME_GetTimezoneBias(&tzinfo, &ft, TRUE, &lBias))
        return FALSE;
    /* convert minutes to 100-nanoseconds-ticks */
    t += (LONGLONG)lBias * 600000000;
    LL2FILETIME( t, &ft)
    return FileTimeToSystemTime(&ft, lpUniversalTime);
}

#endif /* qWinOS && defined(__GNUC__) */
