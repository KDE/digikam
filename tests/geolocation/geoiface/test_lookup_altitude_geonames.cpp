/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-05-07
 * @brief  Test for the geonames based altitude lookup class
 *
 * @author Copyright (C) 2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "test_lookup_altitude_geonames.h"

// local includes

#include "lookupaltitudegeonames.h"
#include "lookupfactory.h"
#include "geoiface_common.h"

using namespace GeoIface;

void TestLookupAltitudeGeonames::testNoOp()
{
}

void TestLookupAltitudeGeonames::testSimpleLookup()
{
    LookupAltitude* const myLookup = LookupFactory::getAltitudeLookup(QString::fromLatin1("geonames"), this);

    QSignalSpy spyRequestsReady(myLookup, SIGNAL(signalRequestsReady(QList<int>)));
    QSignalSpy spyLookupDone(myLookup, SIGNAL(signalDone()));

    LookupAltitude::Request::List requestsList;
    const int nRequests = 30;

    // add different requests
    for (qreal i = 0; i < nRequests; ++i)
    {
        LookupAltitude::Request myRequest;
        myRequest.coordinates = GeoCoordinates(52.0, 6.0+i);
        requestsList << myRequest;
    }

    // add those same requests again, expecting them to be merged into the existing requests:
    for (qreal i = 0; i < nRequests; ++i)
    {
        LookupAltitude::Request myRequest;
        myRequest.coordinates = GeoCoordinates(52.0, 6.0+i);
        requestsList << myRequest;
    }

    myLookup->addRequests(requestsList);
    myLookup->startLookup();

    // wait until the files are loaded:
    while (spyLookupDone.isEmpty())
    {
        QTest::qWait(100);
    }

    QCOMPARE(spyRequestsReady.count(), 2);
}

QTEST_GUILESS_MAIN(TestLookupAltitudeGeonames)
