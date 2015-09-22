/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-02-07
 * @brief  test for the simple datatypes and helper functions
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
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

#ifndef TEST_PRIMITIVES_H
#define TEST_PRIMITIVES_H

// Qt includes

#include <QtTest/QtTest>

class TestPrimitives : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNoOp();
    void testParseLatLonString();
    void testParseXYStringToPoint();
    void testParseBoundsString();
    void testNormalizeBounds_data();
    void testNormalizeBounds();
    void testGroupStateComputer();
};

#endif /* TEST_PRIMITIVES_H */
