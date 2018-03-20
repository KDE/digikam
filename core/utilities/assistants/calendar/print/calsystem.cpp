/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-11-03
 * Description : calendar system.
 *
 * Copyright (C) 2014      by John Layt <john at layt dot net>
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "calsystem.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class CalSystemPrivate : public QSharedData
{
public:

    explicit CalSystemPrivate(CalSystem::CalendarSystem calendar);

    CalSystem::CalendarSystem calendarSystem()                         const;
    qint64 epoch()                                                     const;
    qint64 earliestValidDate()                                         const;
    int    earliestValidYear()                                         const;
    qint64 latestValidDate()                                           const;
    int    latestValidYear()                                           const;
    int    yearOffset()                                                const;
    int    maxMonthsInYear()                                           const;
    int    monthsInYear(int year)                                      const;
    int    maxDaysInYear()                                             const;
    int    daysInYear(int year)                                        const;
    int    maxDaysInMonth()                                            const;
    int    daysInMonth(int year, int month)                            const;
    bool   hasYearZero()                                               const;
    bool   hasLeapMonths()                                             const;

    int    quarter(int month)                                          const;
    bool   isLeapYear(int year)                                        const;
    void   julianDayToDate(qint64 jd, int* year, int* month, int* day) const;
    qint64 julianDayFromDate(int yr, int mth, int dy)                  const;

    bool   isValidYear(int year)                                       const;
    bool   isValidMonth(int year, int month)                           const;
    int    addYears(int y1, int years)                                 const;
    int    diffYears(int y1, int y2)                                   const;

public:

    CalSystem::CalendarSystem m_calendarSystem;
};

static const char julianMonths[] =
{
    0,
    31,
    28,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31
};

CalSystemPrivate::CalSystemPrivate(CalSystem::CalendarSystem calendar)
    : QSharedData(),
      m_calendarSystem(calendar)
{
}

CalSystem::CalendarSystem CalSystemPrivate::calendarSystem() const
{
    if (m_calendarSystem == CalSystem::DefaultCalendar)
    {
        return CalSystem::GregorianCalendar;
    }
    else
    {
        return m_calendarSystem;
    }
}

qint64 CalSystemPrivate::epoch() const
{
    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
            return 1721426;  //  0001-01-01 Gregorian

        case CalSystem::CopticCalendar:
            return 1825030;  //  0001-01-01 ==  0284-08-29 Gregorian

        case CalSystem::EthiopicCalendar:
            return 1724221;  //  0001-01-01 ==  0008-08-29 Gregorian

        case CalSystem::EthiopicAmeteAlemCalendar:
            return -284655;  //  0001-01-01 == -5492-08-29 Gregorian

        case CalSystem::IndianNationalCalendar:
            return 1749994;  //  0000-01-01 == 0078-03-22 Gregorian

        case CalSystem::IslamicCivilCalendar:
            return 1948440;  //  0001-01-01 == 0622-07-19 Gregorian

        case CalSystem::ISO8601Calendar:
            return 1721060;  //  0000-01-01 Gregorian

        case CalSystem::JapaneseCalendar:
            return 1721426;  //  0001-01-01 Gregorian

        case CalSystem::JulianCalendar:
            return 1721424;  //  0001-01-01 ==  Gregorian

        case CalSystem::ROCCalendar:
            return 2419403;  //  0001-01-01 ==  1912-01-01 Gregorian

        case CalSystem::ThaiCalendar:
            return 1522734;  //  0000-01-01 == -0544-01-01 Gregorian

        default:
            return 0;
    }
}

qint64 CalSystemPrivate::earliestValidDate() const
{
    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
            return -31738;   // -4800-01-01 Gregorian

        case CalSystem::CopticCalendar:
            return 1825030;  //  0001-01-01 == 0284-08-29 Gregorian

        case CalSystem::EthiopicCalendar:
            return 1724221;  //  0001-01-01 == 0008-08-29 Gregorian

        case CalSystem::EthiopicAmeteAlemCalendar:
            return -284655;  //  0001-01-01 == -5492-08-29 Gregorian

        case CalSystem::IndianNationalCalendar:
            return 1749994;  //  0000-01-01 == 0078-03-22 Gregorian

        case CalSystem::IslamicCivilCalendar:
            return 1948440;  //  0001-01-01 == 0622-07-19 Gregorian

        case CalSystem::ISO8601Calendar:
            return 1721060;  //  0000-01-01 Gregorian

        case CalSystem::JapaneseCalendar:
            return -31738;   // -4800-01-01 Gregorian

        case CalSystem::JulianCalendar:
            return -31776;   // -4800-01-01 Julian

        case CalSystem::ROCCalendar:
            return 2419403;  //  0001-01-01 == 1912-01-01 Gregorian

        case CalSystem::ThaiCalendar:
            return 1522734;  //  0000-01-01 == -0544-01-01 Gregorian

        default:
            return 0;
    }
}

int CalSystemPrivate::earliestValidYear() const
{
    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
        case CalSystem::JapaneseCalendar:
        case CalSystem::JulianCalendar:
            return -4800;

        case CalSystem::IndianNationalCalendar:
        case CalSystem::ISO8601Calendar:
        case CalSystem::ThaiCalendar:
            return 0;

        default:
            return 1;
    }
}

qint64 CalSystemPrivate::latestValidDate() const
{
    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
            return 5373484;  //  9999-12-31 Gregorian

        case CalSystem::CopticCalendar:
            return 5477164;  //  9999-13-05 == 10283-11-12 Gregorian

        case CalSystem::EthiopicCalendar:
            return 5376721;  //  9999-13-05 == 10008-11-10 Gregorian

        case CalSystem::EthiopicAmeteAlemCalendar:
            return 3367114;  //  9999-13-05 ==  4506-09-29 Gregorian

        case CalSystem::IndianNationalCalendar:
            return 5402054;  //  9999-12-30 == 10078-03-21 Gregorian

        case CalSystem::IslamicCivilCalendar:
            return 5491751;  //  9999-12-29 == 10323-10-21 Gregorian

        case CalSystem::ISO8601Calendar:
            return 5373484;  //  9999-12-31 Gregorian

        case CalSystem::JapaneseCalendar:
            return 5373484;  //  9999-12-31 Gregorian

        case CalSystem::JulianCalendar:
            return 5373557;  //  9999-12-31 == 10000-03-13 Gregorian

        case CalSystem::ROCCalendar:
            return 6071462;  //  9999-12-31 == 11910-12-31 Gregorian

        case CalSystem::ThaiCalendar:
            return 5175158;  //  9999-12-31 ==  9456-12-31 Gregorian

        default:
            return 0;
    }
}

int CalSystemPrivate::latestValidYear() const
{
    switch (calendarSystem())
    {
        default:
            return 9999;
    }
}

int CalSystemPrivate::yearOffset() const
{
    switch (calendarSystem())
    {
        case CalSystem::ROCCalendar:
            return 1911;  // 0001-01-01 == 1912-01-01 Gregorian

        case CalSystem::ThaiCalendar:
            return -543;  // 0000-01-01 == -544-01-01 Gregorian

        default:
            return 0;
    }
}

int CalSystemPrivate::maxMonthsInYear() const
{
    switch (calendarSystem())
    {
        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
            return 13;

        default:
            return 12;
    }
}

int CalSystemPrivate::monthsInYear(int year) const
{
    year = year + yearOffset();

    switch (calendarSystem())
    {
        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
            return 13;

        default:
            return 12;
    }
}

int CalSystemPrivate::maxDaysInYear() const
{
    switch (calendarSystem())
    {
        case CalSystem::IslamicCivilCalendar:
            return 355;

        default:
            return 366;
    }
}

int CalSystemPrivate::daysInYear(int year) const
{
    switch (calendarSystem())
    {
        case CalSystem::IslamicCivilCalendar:
            return isLeapYear(year) ? 355 : 354;

        default:
            return isLeapYear(year) ? 366 : 365;
    }
}

int CalSystemPrivate::maxDaysInMonth() const
{
    switch (calendarSystem())
    {
        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
        case CalSystem::IslamicCivilCalendar:
            return 30;

        default:
            return 31;
    }
}

int CalSystemPrivate::daysInMonth(int year, int month) const
{
    if (month < 1 || month > monthsInYear(year))
    {
        return 0;
    }

    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
        case CalSystem::ISO8601Calendar:
        case CalSystem::JapaneseCalendar:
        case CalSystem::ROCCalendar:
        case CalSystem::ThaiCalendar:
        case CalSystem::JulianCalendar:
        {
            if (month == 2 && isLeapYear(year))
            {
                return 29;
            }
            else
            {
                return julianMonths[month];
            }
        }

        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
        {
            if (month == 13)
            {
                return isLeapYear(year) ? 6 : 5;
            }
            else
            {
                return 30;
            }
        }

        case CalSystem::IndianNationalCalendar:
        {
            if (month >= 7)
            {
                return 30;
            }
            else if (month >= 2)
            {
                return 31;
            }
            else if (isLeapYear(year))
            {
                return 31;
            }
            else
            {
                return 30;
            }
        }

        case CalSystem::IslamicCivilCalendar:
        {
            if (month == 12 && isLeapYear(year))
            {
                return 30;
            }
            else if (month % 2 == 0)
            {
                return 29;
            }
            else
            {
                return 30;
            }
        }

        default:
            return 0;
    }
}

bool CalSystemPrivate::hasYearZero() const
{
    switch (calendarSystem())
    {
        case CalSystem::IndianNationalCalendar:
        case CalSystem::ISO8601Calendar:
        case CalSystem::ThaiCalendar:
            return true;

        default:
            return false;
    }
}

bool CalSystemPrivate::hasLeapMonths() const
{
    switch (calendarSystem())
    {
        default:
            return false;
    }
}

int CalSystemPrivate::quarter(int month) const
{
    switch (calendarSystem())
    {
        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
            if (month == 13)
            { // Consider the short epagomenal month as part of the 4th quarter
                return 4;
            }

            [[fallthrough]];
        default:
            return (((month - 1) / 3) + 1);
    }
}

bool CalSystemPrivate::isLeapYear(int year) const
{
    year = year + yearOffset();

    // Uses same rule as Gregorian and in same years as Gregorian to keep in sync
    // Can't use yearOffset() as this offset only applies for isLeapYear()
    if (calendarSystem() == CalSystem::IndianNationalCalendar)
    {
        year = year + 78;
    }

    if (year < 1 && !hasYearZero())
    {
        ++year;
    }

    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
        case CalSystem::IndianNationalCalendar:
        case CalSystem::ISO8601Calendar:
        case CalSystem::JapaneseCalendar:
        case CalSystem::ROCCalendar:
        case CalSystem::ThaiCalendar:
            return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);

        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
            return (year % 4 == 3);

        case CalSystem::JulianCalendar:
            return (year % 4 == 0);

        case CalSystem::IslamicCivilCalendar:
            return ((((11 * year) + 14) % 30) < 11);

        default:
            return false;
    }
}

void CalSystemPrivate::julianDayToDate(qint64 jd, int* year, int* month, int* day) const
{
    int yy=0, mm=0, dd=0;

    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
        case CalSystem::ISO8601Calendar:
        case CalSystem::JapaneseCalendar:
        case CalSystem::ROCCalendar:
        case CalSystem::ThaiCalendar:
        {
            // Formula from The Calendar FAQ by Claus Tondering
            // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
            qint64 a = jd + 32044;
            qint64 b = ((4 * a) + 3) / 146097;
            qint64 c = a - ((146097 * b) / 4);
            qint64 d = ((4 * c) + 3) / 1461;
            qint64 e = c - ((1461 * d) / 4);
            qint64 m = ((5 * e) + 2) / 153;
            dd       = e - (((153 * m) + 2) / 5) + 1;
            mm       = m + 3 - (12 * (m / 10));
            yy       = (100 * b) + d - 4800 + (m / 10);
            break;
        }

        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
        {
            // Formula derived from first principles by John Layt
            qint64 s   = jd - (epoch() - 365);
            qint64 l   = s / 1461;
            yy         = (l * 4) + qMin((qint64)3, (s % 1461) / 365);
            qint64 diy = s - (yy * 365) + (yy / 4);
            mm         = (diy / 30) + 1;
            dd         = (diy % 30) + 1;
            break;
        }

        case CalSystem::IndianNationalCalendar:
        {
            // Formula from the "Explanatory Supplement to the Astronomical Almanac"
            // Revised Edition 2006 section 12.94 pp 605-606, US Naval Observatory
            // Originally from the "Report of the Calendar Reform Committee" 1955, Indian Government
            qint64 l = jd + 68518;
            qint64 n = (4 * l) / 146097;
            l        = l - (146097 * n + 3) / 4;
            qint64 i = (4000 * (l + 1)) / 1461001;
            l        = l - (1461 * i) / 4 + 1;
            qint64 j = ((l - 1) / 31) * (1 - l / 185) + (l / 185) * ((l - 156) / 30 + 5) - l / 366;
            dd       = l - 31 * j + ((j + 2) / 8) * (j - 5);
            l        = j / 11;
            mm       = j + 2 - 12 * l;
            yy       = 100 * (n - 49) + l + i - 78;
            break;
        }

        case CalSystem::IslamicCivilCalendar:
        {
            // Formula from the "Explanatory Supplement to the Astronomical Almanac"
            // Revised Edition 2006 section ??? pp ???, US Naval Observatory
            // Derived from Fliegel & Van Flandern 1968
            qint64 l = jd - epoch() + 10632;
            qint64 n = (l - 1) / 10631;
            l        = l - 10631 * n + 354;
            int j    = ((10985 - l) / 5316) * ((50 * l) / 17719) + (l / 5670) * ((43 * l) / 15238);
            l        = l - ((30 - j) / 15) * ((17719 * j) / 50) - (j / 16) * ((15238 * j) / 43) + 29;
            yy       = (30 * n) + j - 30;
            mm       = (24 * l) / 709;
            dd       = l - ((709 * mm) / 24);
            break;
        }

        case CalSystem::JulianCalendar:
        {
            // Formula from The Calendar FAQ by Claus Tondering
            // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
            qint64 b = 0;
            qint64 c = jd + 32082;
            qint64 d = ((4 * c) + 3) / 1461;
            qint64 e = c - ((1461 * d) / 4);
            qint64 m = ((5 * e) + 2) / 153;
            dd       = e - (((153 * m) + 2) / 5) + 1;
            mm       = m + 3 - (12 * (m / 10));
            yy       = (100 * b) + d - 4800 + (m / 10);
            break;
        }

        default:
            break;
    }

    if (!hasYearZero() && yy < 1)
    {
        yy -= 1;
    }

    yy = yy - yearOffset();

    if (year)
    {
        *year = yy;
    }
    if (month)
    {
        *month = mm;
    }
    if (day)
    {
        *day = dd;
    }
}

qint64 CalSystemPrivate::julianDayFromDate(int yr, int mth, int dy) const
{
    qint64 jd    = 0;
    qint64 year  = yr + yearOffset();
    qint64 month = mth;
    qint64 day   = dy;

    if (year < 1 && !hasYearZero())
    {
        year = year + 1;
    }

    switch (calendarSystem())
    {
        case CalSystem::GregorianCalendar:
        case CalSystem::ISO8601Calendar:
        case CalSystem::JapaneseCalendar:
        case CalSystem::ROCCalendar:
        case CalSystem::ThaiCalendar:
        {
            // Formula from The Calendar FAQ by Claus Tondering
            // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
            int a = (14 - month) / 12;
            year  = year + 4800 - a;
            int m = month + (12 * a) - 3;
            jd    = day
                   + (((153 * m) + 2) / 5)
                   + (365 * year)
                   + (year / 4)
                   - (year / 100)
                   + (year / 400)
                   - 32045;
            break;
        }

        case CalSystem::CopticCalendar:
        case CalSystem::EthiopicCalendar:
        case CalSystem::EthiopicAmeteAlemCalendar:
        {
            // Formula derived from first principles by John Layt
            jd = epoch() - 1
                + ((year - 1) * 365)
                + (year / 4)
                + ((month - 1) * 30)
                + day;
            break;
        }

        case CalSystem::IndianNationalCalendar:
        {
            // Formula from the "Explanatory Supplement to the Astronomical Almanac"
            // Revised Edition 2006 section 12.94 pp 605-606, US Naval Observatory
            // Originally from the "Report of the Calendar Reform Committee" 1955, Indian Government
            jd = 365 * year
                + (year + 78 - 1 / month) / 4
                + 31 * month
                - (month + 9) / 11
                - (month / 7) * (month - 7)
                - (3 * ((year  + 78 - 1 / month) / 100 + 1)) / 4
                + day
                + 1749579;
            break;
        }

        case CalSystem::IslamicCivilCalendar:
        {
            // Formula from the "Explanatory Supplement to the Astronomical Almanac"
            // Revised Edition 2006 section ??? pp ???, US Naval Observatory
            // Derived from Fliegel & Van Flandern 1968
            jd = (3 + (11 * year)) / 30
                + 354 * year
                + 30 * month
                - (month - 1) / 2
                + day
                + epoch()
                - 385;
            break;
        }

        case CalSystem::JulianCalendar:
        {
            // Formula from The Calendar FAQ by Claus Tondering
            // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
            int a = (14 - month) / 12;
            year  = year + 4800 - a;
            int m = month + (12 * a) - 3;
            jd    = day
                   + (((153 * m) + 2) / 5)
                   + (365 * year)
                   + (year / 4)
                   - 32083;
            break;
        }

        default:
            break;
    }

    return jd;
}

// Some private utility rules

bool CalSystemPrivate::isValidYear(int year) const
{
    return year >= earliestValidYear() &&
           year <= latestValidYear()   &&
           (year == 0 ? hasYearZero() : true);
}

bool CalSystemPrivate::isValidMonth(int year, int month) const
{
    return isValidYear(year) &&
           month >= 1        &&
           month <= monthsInYear(year);
}

int CalSystemPrivate::addYears(int y1, int years) const
{
    int y2 = y1 + years;

    if (!hasYearZero()) 
    {
        if (y1 > 0 && y2 <= 0)
        {
            --y2;
        }
        else if (y1 < 0 && y2 >= 0) 
        {
            ++y2;
        }
    }

    return y2;
}

int CalSystemPrivate::diffYears(int y1, int y2) const
{
    int dy = y2 - y1;

    if (!hasYearZero())
    {
        if (y2 > 0 && y1 < 0)
        {
            dy -= 1;
        }
        else if (y2 < 0 && y1 > 0)
        {
            dy += 1;
        }
    }

    return dy;
}

// ---------------------------------------------------------------

CalSystem::CalSystem(CalSystem::CalendarSystem calendar)
    : d(new CalSystemPrivate(calendar))
{
}

CalSystem::~CalSystem()
{
}

CalSystem &CalSystem::operator=(const CalSystem &other)
{
    d = other.d;
    return *this;
}

CalSystem::CalendarSystem CalSystem::calendarSystem() const
{
    return d->calendarSystem();
}

QDate CalSystem::epoch() const
{
    return QDate::fromJulianDay(d->epoch());
}

QDate CalSystem::earliestValidDate() const
{
    return QDate::fromJulianDay(d->earliestValidDate());
}

QDate CalSystem::latestValidDate() const
{
    return QDate::fromJulianDay(d->latestValidDate());
}

int CalSystem::maximumMonthsInYear() const
{
    return d->maxMonthsInYear();
}

int CalSystem::maximumDaysInYear() const
{
    return d->maxDaysInYear();
}

int CalSystem::maximumDaysInMonth() const
{
    return d->maxDaysInMonth();
}

bool CalSystem::isValid(const QDate& date) const
{
    return date.isValid()              &&
           date >= earliestValidDate() &&
           date <= latestValidDate();
}

bool CalSystem::isValid(int year, int month, int day) const
{
    return d->isValidMonth(year, month) &&
           day >= 1                     &&
           day <= d->daysInMonth(year, month);
}

bool CalSystem::isValid(int year, int dayOfYear) const
{
    return d->isValidYear(year) &&
           dayOfYear > 0        &&
           dayOfYear <= d->daysInYear(year);
}

QDate CalSystem::date(int year, int month, int day) const
{
    if (isValid(year, month, day))
    {
        return QDate::fromJulianDay(d->julianDayFromDate(year, month, day));
    }
    else
    {
        return QDate();
    }
}

QDate CalSystem::date(int year, int dayOfYear) const
{
    if (isValid(year, dayOfYear))
    {
        return QDate::fromJulianDay(d->julianDayFromDate(year, 1, 1) + dayOfYear - 1);
    }
    else
    {
        return QDate();
    }
}

void CalSystem::getDate(const QDate& date, int* year, int* month, int* day) const
{
    int yy = 0;
    int mm = 0;
    int dd = 0;

    if (isValid(date))
    {
        d->julianDayToDate(date.toJulianDay(), &yy, &mm, &dd);
    }

    if (year)
    {
        *year = yy;
    }
    if (month)
    {
        *month = mm;
    }
    if (day)
    {
        *day = dd;
    }
}

int CalSystem::year(const QDate& date) const
{
    int y = 0;

    if (isValid(date))
    {
        d->julianDayToDate(date.toJulianDay(), &y, 0, 0);
    }

    return y;
}

int CalSystem::month(const QDate& date) const
{
    int m = 0;

    if (isValid(date))
    {
        d->julianDayToDate(date.toJulianDay(), 0, &m, 0);
    }

    return m;
}

int CalSystem::day(const QDate& date) const
{
    int dd = 0;

    if (isValid(date))
    {
        d->julianDayToDate(date.toJulianDay(), 0, 0, &dd);
    }

    return dd;
}

int CalSystem::quarter(const QDate& date) const
{
    if (isValid(date))
    {
        int month;
        d->julianDayToDate(date.toJulianDay(), 0, &month, 0);

        return d->quarter(month);
    }
    else
    {
        return 0;
    }
}

int CalSystem::quarter(int year, int month, int day) const
{
    if (isValid(year, month, day))
    {
        return d->quarter(month);
    }
    else
    {
        return 0;
    }
}

int CalSystem::dayOfYear(const QDate& date) const
{
    if (isValid(date))
    {
        return date.toJulianDay() - firstDayOfYear(date).toJulianDay() + 1;
    }
    else
    {
        return 0;
    }
}

int CalSystem::dayOfYear(int year, int month, int day) const
{
    return dayOfYear(date(year, month, day));
}

int CalSystem::dayOfWeek(const QDate& date) const
{
    //jd 0 = Monday = weekday 1.  We've never skipped weekdays.

    if (isValid(date))
    {
        if (date.toJulianDay() >= 0)
        {
            return (date.toJulianDay() % daysInWeek()) + 1;
        }
        else
        {
            return ((date.toJulianDay() + 1) % daysInWeek()) + daysInWeek();
        }
    }
    else
    {
        return 0;
    }
}

int CalSystem::dayOfWeek(int year, int month, int day) const
{
    return dayOfWeek(date(year, month, day));
}

// TODO: These are ISO weeks, may need to localise
int CalSystem::weekNumber(const QDate& date, int* yearNum) const
{
    if (isValid(date))
    {
        int year, month, day;
        d->julianDayToDate(date.toJulianDay(), &year, &month, &day);
        return weekNumber(year, month, day, yearNum);
    }
    else
    {
        return 0;
    }
}

// TODO: These are ISO weeks, may need to localise
int CalSystem::weekNumber(int year, int month, int day, int* yearNum) const
{
    if (!isValid(year, month, day))
    {
        if (yearNum)
        {
            *yearNum = 0;
        }

        return 0;
    }

    int yday = dayOfYear(year, month, day) - 1;
    int wday = dayOfWeek(year, month, day);

    if (wday == 7)
    {
        wday = 0;
    }

    int w;

    for (;;)
    {
        int len = d->daysInYear(year);

        // What yday (-3 ... 3) does the ISO year begin on?
        int bot = ((yday + 11 - wday) % 7) - 3;

        // What yday does the NEXT ISO year begin on?
        int top = bot - (len % 7);

        if (top < -3)
        {
            top += 7;
        }

        top += len;

        if (yday >= top)
        {
            ++year;
            w = 1;
            break;
        }

        if (yday >= bot)
        {
            w = 1 + ((yday - bot) / 7);
            break;
        }

        --year;
        yday += d->daysInYear(year);
    }

    if (yearNum)
    {
        *yearNum = year;
    }

    return w;
}

int CalSystem::monthsInYear(const QDate& date) const
{
    if (isValid(date))
    {
        return d->monthsInYear(year(date));
    }
    else
    {
        return 0;
    }
}

int CalSystem::monthsInYear(int year) const
{
    if (d->isValidYear(year))
    {
        return d->monthsInYear(year);
    }
    else
    {
        return 0;
    }
}

int CalSystem::weeksInYear(const QDate& date) const
{
    if (isValid(date))
    {
        return weeksInYear(year(date));
    }
    else
    {
        return 0;
    }
}

// TODO: This is ISO weeks, may need to localise
int CalSystem::weeksInYear(int year) const
{
    if (d->isValidYear(year))
    {
        int weekYear = year;
        int lastWeek = weekNumber(lastDayOfYear(year), &weekYear);

        if (lastWeek < 1 || weekYear != year)
        {
            lastWeek = weekNumber(addDays(lastDayOfYear(year), -7), &weekYear);
        }

        return lastWeek;
    }
    else
    {
        return 0;
    }
}

int CalSystem::daysInYear(const QDate& date) const
{
    if (isValid(date))
    {
        return d->daysInYear(year(date));
    }
    else
    {
        return 0;
    }
}

int CalSystem::daysInYear(int year) const
{
    if (d->isValidYear(year))
    {
        return d->daysInYear(year);
    }
    else
    {
        return 0;
    }
}

int CalSystem::daysInMonth(const QDate& date) const
{
    if (isValid(date))
    {
        int year, month;
        d->julianDayToDate(date.toJulianDay(), &year, &month, 0);
        return d->daysInMonth(year, month);
    }
    else
    {
        return 0;
    }
}

int CalSystem::daysInMonth(int year, int month) const
{
    if (d->isValidMonth(year, month))
    {
        return d->daysInMonth(year, month);
    }
    else
    {
        return 0;
    }
}

int CalSystem::daysInWeek() const
{
    return 7;
}

bool CalSystem::isLeapYear(const QDate& date) const
{
    if (isValid(date))
    {
        return d->isLeapYear(year(date));
    }
    else
    {
        return false;
    }
}

bool CalSystem::isLeapYear(int year) const
{
    if (d->isValidYear(year))
    {
        return d->isLeapYear(year);
    }
    else
    {
        return false;
    }
}

QDate CalSystem::addYears(const QDate& dt, int years) const
{
    if (isValid(dt))
    {
        int year, month, day;
        d->julianDayToDate(dt.toJulianDay(), &year, &month, &day);
        year  = d->addYears(year, years);
        month = qMin(month, d->monthsInYear(year));
        return date(year, month, qMin(day, d->daysInMonth(year, month)));
    }
    else
    {
        return QDate();
    }
}

QDate CalSystem::addMonths(const QDate& dt, int months) const
{
    if (isValid(dt))
    {
        int year, month, day;
        d->julianDayToDate(dt.toJulianDay(), &year, &month, &day);

        while (months != 0)
        {
            if (months < 0)
            {
                if (month + months >= 1)
                {
                    month += months;
                    months = 0;
                }
                else if (months < 0)
                {
                    year    = d->addYears(year, -1);
                    months += d->monthsInYear(year);
                }
            }
            else
            {
                int miy = d->monthsInYear(year);

                if (month + months <= miy)
                {
                    month += months;
                    months = 0;
                }
                else
                {
                    year    = d->addYears(year, 1);
                    months -= miy;
                }
            }
        }

        return date(year, month, qMin(day, d->daysInMonth(year, month)));
    }
    else
    {
        return QDate();
    }
}

QDate CalSystem::addDays(const QDate& date, int days) const
{
    return date.addDays(days);
}

//Caters for Leap Months, but possibly not for Hebrew
int CalSystem::yearsDifference(const QDate& fromDate, const QDate& toDate) const
{
    if (!isValid(fromDate) || !isValid(toDate) || toDate == fromDate)
    {
        return 0;
    }

    if (toDate < fromDate)
    {
        return - yearsDifference(toDate, fromDate);
    }

    int y1, m1, d1, y2, m2, d2;
    d->julianDayToDate(fromDate.toJulianDay(), &y1, &m1, &d1);
    d->julianDayToDate(toDate.toJulianDay(),   &y2, &m2, &d2);

    if (y2 == y1)
    {
        return 0;
    }

    if (m2 > m1)
    {
        return d->diffYears(y1, y2);
    }

    if (m2 < m1)
    {
        return d->diffYears(y1, y2) - 1;
    }

    // m2 == m1
    // Allow for last day of month to last day of month and leap days
    // e.g. 2000-02-29 to 2001-02-28 is 1 year not 0 years
    if ((d2 >= d1) || (d1 == d->daysInMonth(y1, m1) && d2 == d->daysInMonth(y2, m2)))
    {
        return d->diffYears(y1, y2);
    }
    else
    {
        return d->diffYears(y1, y2) - 1;
    }
}

// Caters for Leap Months, but possibly not for Hebrew
int CalSystem::monthsDifference(const QDate& fromDate, const QDate& toDate) const
{
    if (!isValid(fromDate) || !isValid(toDate) || toDate == fromDate)
    {
        return 0;
    }

    if (toDate < fromDate)
    {
        return - monthsDifference(toDate, fromDate);
    }

    int y1, m1, d1, y2, m2, d2, my;
    d->julianDayToDate(fromDate.toJulianDay(), &y1, &m1, &d1);
    d->julianDayToDate(toDate.toJulianDay(),   &y2, &m2, &d2);

    // Calculate number of months in full years preceding y2
    if (y2 == y1)
    {
        my = 0;
    }
    else if (d->hasLeapMonths())
    {
        my = 0;

        for (int y = y1; y < y2; y = d->addYears(y, 1))
        {
            my = my + monthsInYear(y);
        }
    }
    else
    {
        my = d->diffYears(y1, y2) * monthsInYear(y2);
    }

    // Allow for last day of month to last day of month and leap days
    // e.g. 2010-03-31 to 2010-04-30 is 1 month not 0 months
    // also 2000-02-29 to 2001-02-28 is 12 months not 11 months
    if ((d2 >= d1) || (d1 == d->daysInMonth(y1, m1) && d2 == d->daysInMonth(y2, m2)))
    {
        return my + m2 - m1;
    }
    else
    {
        return my + m2 - m1 - 1;
    }
}

qint64 CalSystem::daysDifference(const QDate& fromDate, const QDate& toDate) const
{
    if (isValid(fromDate) && isValid(toDate))
    {
        return toDate.toJulianDay() - fromDate.toJulianDay();
    }
    else
    {
        return 0;
    }
}

//Caters for Leap Months, but possibly not for Hebrew
bool CalSystem::dateDifference(const QDate& fromDate, const QDate& toDate,
                               int* years, int* months, int* days, int* direction) const
{
    int dy  = 0;
    int dm  = 0;
    int dd  = 0;
    int dir = 1;

    if (isValid(fromDate) && isValid(toDate) && fromDate != toDate)
    {
        if (toDate < fromDate)
        {
            dateDifference(toDate, fromDate, &dy, &dm, &dd, 0);
            dir = -1;
        }
        else
        {
            int y1, m1, d1, y2, m2, d2;
            d->julianDayToDate(fromDate.toJulianDay(), &y1, &m1, &d1);
            d->julianDayToDate(toDate.toJulianDay(),   &y2, &m2, &d2);

            dy = yearsDifference(fromDate, toDate);

            // Calculate months and days difference
            int miy0 = d->monthsInYear(d->addYears(y2, -1));

            if (miy0 == 0)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot compute date difference";
                return false;
            }

            if (d2 >= d1)
            {
                dm = (miy0 + m2 - m1) % miy0;
                dd = d2 - d1;
            }
            else
            {
                // d2 < d1
                // Allow for last day of month to last day of month and leap days
                // e.g. 2010-03-31 to 2010-04-30 is 1 month
                //      2000-02-29 to 2001-02-28 is 1 year
                //      2000-02-29 to 2001-03-01 is 1 year 1 day
                int dim0 = daysInMonth(addMonths(toDate, -1));
                int dim1 = d->daysInMonth(y1, m1);

                if (dim0 == 0 || dim1 == 0)
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot compute date difference";
                    return false;
                }

                if (d1 == dim1 && d2 == d->daysInMonth(y2, m2))
                {
                    dm = (miy0 + m2 - m1) % miy0;
                    dd = 0;
                }
                else if (month(addMonths(toDate, -1)) == m1 && dim0 < dim1)
                {
                    // Special case where fromDate = leap day and toDate in month following but non-leap year
                    // e.g. 2000-02-29 to 2001-03-01 needs to use 29 to calculate day number not 28
                    dm = (miy0 + m2 - m1 - 1) % miy0;
                    dd = (dim1 + d2 - d1) % dim1;
                }
                else
                {
                    dm = (miy0 + m2 - m1 - 1) % miy0;
                    dd = (dim0 + d2 - d1) % dim0;
                }
            }
        }
    }

    if (years)
    {
        *years = dy;
    }

    if (months)
    {
        *months = dm;
    }

    if (days)
    {
        *days = dd;
    }

    if (direction)
    {
        *direction = dir;
    }

    return true;
}

QDate CalSystem::firstDayOfYear(const QDate& dt) const
{
    if (isValid(dt))
    {
        return date(year(dt), 1, 1);
    }
    else
    {
        return QDate();
    }
}

QDate CalSystem::firstDayOfYear(int year) const
{
    return date(year, 1, 1);
}

QDate CalSystem::lastDayOfYear(const QDate& dt) const
{
    if (isValid(dt))
    {
        int y = year(dt);
        return date(y, d->daysInYear(y));
    }
    else
    {
        return QDate();
    }
}

QDate CalSystem::lastDayOfYear(int year) const
{
    if (d->isValidYear(year))
    {
        return date(year, d->daysInYear(year));
    }
    else
    {
        return QDate();
    }
}

QDate CalSystem::firstDayOfMonth(const QDate& dt) const
{
    int year, month;
    getDate(dt, &year, &month, 0);

    return date(year, month, 1);
}

QDate CalSystem::firstDayOfMonth(int year, int month) const
{
    return date(year, month, 1);
}

QDate CalSystem::lastDayOfMonth(const QDate& dt) const
{
    int year, month;
    getDate(dt, &year, &month, 0);

    return date(year, month, daysInMonth(year, month));
}

QDate CalSystem::lastDayOfMonth(int year, int month) const
{
    return date(year, month, daysInMonth(year, month));
}

} // Namespace Digikam
