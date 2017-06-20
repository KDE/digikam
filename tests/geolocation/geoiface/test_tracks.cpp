/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-17
 * Description : Test parsing gpx data.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QDateTime>
#include <QtTest>
#include <QDebug>

// Local includes

#include "test_tracks.h"
#include "trackmanager.h"
#include "trackreader.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(TestTracks)

/**
 * @brief Return the path of the directory containing the test data
 */
QString GetTestDataDirectory()
{
    return QString(QFINDTESTDATA("data/"));
}

/**
 * @brief Dummy test that does nothing
 */
void TestTracks::testNoOp()
{
}

/**
 * @brief Test how well QDateTime deals with various string representations
 *
 * The behavior of QDateTime::fromString changed in some Qt version, so here
 * we can test what the current behavior is and quickly detect if Qt changes
 * again.
 */
void TestTracks::testQDateTimeParsing()
{
    {
        // strings ending with a 'Z' are taken to be in UTC, regardless of milliseconds
        QDateTime time1 = QDateTime::fromString(QString::fromLatin1("2009-03-11T13:39:55.622Z"), Qt::ISODate);
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QDateTime time2 = QDateTime::fromString(QString::fromLatin1("2009-03-11T13:39:55Z"), Qt::ISODate);
        QCOMPARE(time2.timeSpec(), Qt::UTC);
    }

    {
        // eCoach in N900 records GPX files with this kind of date format:
        // 2010-01-14T09:26:02.287+02:00
        QDateTime time1 = QDateTime::fromString(QString::fromLatin1("2010-01-14T09:26:02.287+02:00"), Qt::ISODate);

        /// @todo What about the timezone?
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(9, 26, 2, 287));

        // when we omit the time zone data, parsing succeeds
        // time is interpreted as local time
        QDateTime time2 = QDateTime::fromString(QString::fromLatin1("2010-01-14T09:26:02.287")/*"+02:00"*/, Qt::ISODate);
        QCOMPARE(time2.date(), QDate(2010, 01, 14));
        QCOMPARE(time2.time(), QTime(9, 26, 2, 287));
        QCOMPARE(time2.timeSpec(), Qt::LocalTime);
    }
}

/**
 * @brief Test our custom parsing function
 */
void TestTracks::testCustomDateTimeParsing()
{
    {
        // this should work as usual:
        const QDateTime time1 = TrackReader::ParseTime(QString::fromLatin1("2009-03-11T13:39:55.622Z"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2009, 03, 11));
        QCOMPARE(time1.time(), QTime(13, 39, 55, 622));
    }

    {
        // eCoach in N900: 2010-01-14T09:26:02.287+02:00
        const QDateTime time1 = TrackReader::ParseTime(QString::fromLatin1("2010-01-14T09:26:02.287+02:00"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(7, 26, 02, 287));
    }

    {
        // test negative time zone offset: 2010-01-14T09:26:02.287+02:00
        const QDateTime time1 = TrackReader::ParseTime(QString::fromLatin1("2010-01-14T09:26:02.287-02:00"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(11, 26, 02, 287));
    }

    {
        // test negative time zone offset with minutes: 2010-01-14T09:26:02.287+03:15
        const QDateTime time1 = TrackReader::ParseTime(QString::fromLatin1("2010-01-14T09:26:02.287-03:15"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(12, 41, 02, 287));
    }
}

/**
 * @brief Test loading of gpx files using TrackManager (threaded)
 */
void TestTracks::testFileLoading()
{
    QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("gpxfile-1.gpx"));

    QList<QUrl> fileList;
    fileList << testDataDir;

    TrackManager myParser;

    QSignalSpy spyTrackFiles(&myParser, SIGNAL(signalTracksChanged(const QList<TrackManager::TrackChanges>)));
    QSignalSpy spyAllDone(&myParser, SIGNAL(signalAllTrackFilesReady()));

    myParser.loadTrackFiles(fileList);

    // wait until the files are loaded:
    while (spyAllDone.isEmpty())
    {
        QTest::qWait(100);
    }

    QCOMPARE(spyAllDone.count(), 1);
    QCOMPARE(spyTrackFiles.count(), 1);

    const TrackManager::Track& file1 = myParser.getTrack(0);
    QVERIFY(!file1.points.isEmpty());
}

/**
 * @brief Test loading of a GPX file directly
 */
void TestTracks::testSaxLoader()
{
    QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("gpxfile-1.gpx"));
    TrackReader::TrackReadResult fileData = TrackReader::loadTrackFile(testDataDir);
    QVERIFY(fileData.isValid);
    QVERIFY(fileData.loadError.isEmpty());

    // verify that the points are sorted by date:
    for (int i = 1; i<fileData.track.points.count(); ++i)
    {
        QVERIFY(TrackManager::TrackPoint::EarlierThan(fileData.track.points.at(i-1), fileData.track.points.at(i)));
    }
}

/**
 * @brief Test loading of invalid GPX files
 */
void TestTracks::testSaxLoaderError()
{
    {
        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("gpx-invalid-empty.gpx"));
        TrackReader::TrackReadResult fileData = TrackReader::loadTrackFile(testDataDir);
        QVERIFY(!fileData.isValid);
        QVERIFY(!fileData.loadError.isEmpty());
        qDebug() << fileData.loadError;
    }

    {
        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("gpx-invalid-xml-error.gpx"));
        TrackReader::TrackReadResult fileData = TrackReader::loadTrackFile(testDataDir);
        QVERIFY(!fileData.isValid);
        QVERIFY(!fileData.loadError.isEmpty());
        qDebug() << fileData.loadError;
    }

    {
        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1Char('/') + QLatin1String("gpx-invalid-no-points.gpx"));
        TrackReader::TrackReadResult fileData = TrackReader::loadTrackFile(testDataDir);
        QVERIFY(!fileData.isValid);
        QVERIFY(!fileData.loadError.isEmpty());
        qDebug() << fileData.loadError;
    }
}
