/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-12
 * Description : Test for the GeoCoordinates class
 *
 * Copyright (C) 2011 by Michael G. Hansen <mike at mghansen dot de>
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

// Local includes

#include "test_geocoordinates.h"
#include "geocoordinates.h"

using namespace Digikam;

void TestGeoCoordinates::testNoOp()
{
}

void TestGeoCoordinates::testGeoCoordinates()
{
    GeoCoordinates coord1(52.0, 6.0);
    QVERIFY(coord1.hasCoordinates());
    QCOMPARE(coord1.geoUrl(), QLatin1String("geo:52,6"));

    GeoCoordinates coord2(52.0, 6.0);
    GeoCoordinates coord3(53.0, 6.0);
    QVERIFY(coord1==coord2);
    QVERIFY(!(coord1==coord3));

    GeoCoordinates coord4 = GeoCoordinates(52.0, 6.0);
    QVERIFY(coord1 == coord4);
}

/**
 * GeoCoordinates are declared as Q_MOVABLE_TYPE, here we test whether the class still
 * works with Qt's container classes.
 */
void TestGeoCoordinates::testMovable()
{
    GeoCoordinates::List startList;

    startList
        << GeoCoordinates()
        << GeoCoordinates(5.0, 10.0)
        << GeoCoordinates(5.0, 10.0, 15.0);

    GeoCoordinates::List copiedList = startList;

    // force a deep copy to occur
    copiedList << GeoCoordinates();

    QCOMPARE(copiedList.at(0), GeoCoordinates());
    QCOMPARE(copiedList.at(1), GeoCoordinates(5.0, 10.0));
    QCOMPARE(copiedList.at(2), GeoCoordinates(5.0, 10.0, 15.0));

    // optional code for benchmarks, but I could not detect any difference
    // with and without Q_MOVABLE_TYPE here, looks like QList does not gain
    // any speed from Q_MOVABLE_TYPE
//     QBENCHMARK
//     {
//         const int benchSize = 100;
//         GeoCoordinates::List benchList;
//         for (int i=0; i<benchSize; ++i)
//         {
//             for (int j=0; j<benchSize; ++j)
//             {
//                 benchList << GeoCoordinates(double(i)/50.0, double(j)/50.0);
//             }
//         }
//
// //         QBENCHMARK
//         {
//             for (int i=0; i<benchSize*10; ++i)
//             {
//                 GeoCoordinates::List benchListCopied = benchList;
//
//                 // force a deep copy to occur:
//                 benchListCopied[0] = GeoCoordinates();
//                 benchListCopied << GeoCoordinates();
//             }
//         }
//     }
}

QTEST_GUILESS_MAIN(TestGeoCoordinates)
