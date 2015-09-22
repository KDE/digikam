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

#ifndef TEST_LOOKUP_ALTITUDE_GEONAMES_H
#define TEST_LOOKUP_ALTITUDE_GEONAMES_H

// Qt includes

#include <QtTest/QtTest>

class TestLookupAltitudeGeonames : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNoOp();
    void testSimpleLookup();
};

#endif /* TEST_LOOKUP_ALTITUDE_GEONAMES_H */
