/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-07
 * Description : Test for the geonames based altitude lookup class
 *
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef TEST_LOOKUP_ALTITUDE_GEONAMES_H
#define TEST_LOOKUP_ALTITUDE_GEONAMES_H

// Qt includes

#include <QtTest>

class TestLookupAltitudeGeonames : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNoOp();
    void testSimpleLookup();
};

#endif /* TEST_LOOKUP_ALTITUDE_GEONAMES_H */
