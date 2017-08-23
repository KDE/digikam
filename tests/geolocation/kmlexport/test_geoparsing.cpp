/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-17
 * Description : test parsing gpx data
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "test_geoparsing.h"

// Qt includes

#include <QDateTime>
#include <QtTest>

// Local includes

#include "geodataparser_time.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(TestGPXParsing)

/**
 * @brief Test how well QDateTime deals with various string representations
 *
 * The behavior of QDateTime::fromString changed in some Qt version, so here
 * we can test what the current behavior is and quickly detect if Qt changes
 * again.
 */
void TestGPXParsing::testQDateTimeParsing()
{
    {
        // strings ending with a 'Z' are taken to be in UTC, regardless of milliseconds
        QDateTime time1 = QDateTime::fromString(QLatin1String("2009-03-11T13:39:55.622Z"), Qt::ISODate);
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QDateTime time2 = QDateTime::fromString(QLatin1String("2009-03-11T13:39:55Z"), Qt::ISODate);
        QCOMPARE(time2.timeSpec(), Qt::UTC);
    }

    {
        // eCoach in N900 records GPX files with this kind of date format:
        // 2010-01-14T09:26:02.287+02:00
        QDateTime time1 = QDateTime::fromString(QLatin1String("2010-01-14T09:26:02.287+02:00"), Qt::ISODate);

#if QT_VERSION>=0x040700
        // Qt >= 4.7: both date and time are parsed fine
        /// @todo What about the timezone?
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(9, 26, 2, 287));
#else
        // Qt < 4.7: the date is parsed fine, but the time fails:
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(0, 0, 0));
#endif

        // when we omit the time zone data, parsing succeeds
        // time is interpreted as local time
        QDateTime time2 = QDateTime::fromString(QLatin1String("2010-01-14T09:26:02.287")/*"+02:00"*/, Qt::ISODate);
        QCOMPARE(time2.date(), QDate(2010, 01, 14));
        QCOMPARE(time2.time(), QTime(9, 26, 2, 287));
        QCOMPARE(time2.timeSpec(), Qt::LocalTime);
    }
}

/**
 * @brief Test our custom parsing function
 */
void TestGPXParsing::testCustomParsing()
{
    {
        // this should work as usual:
        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2009-03-11T13:39:55.622Z"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2009, 03, 11));
        QCOMPARE(time1.time(), QTime(13, 39, 55, 622));
    }

    {
        // eCoach in N900: 2010-01-14T09:26:02.287+02:00
        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2010-01-14T09:26:02.287+02:00"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(7, 26, 02, 287));
    }

    {
        // test negative time zone offset: 2010-01-14T09:26:02.287+02:00
        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2010-01-14T09:26:02.287-02:00"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(11, 26, 02, 287));
    }

    {
        // test negative time zone offset with minutes: 2010-01-14T09:26:02.287+03:15
        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2010-01-14T09:26:02.287-03:15"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(12, 41, 02, 287));
    }
}
