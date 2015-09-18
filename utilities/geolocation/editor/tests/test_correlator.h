/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-01-17
 * @brief  Test parsing gpx data.
 *
 * @author Copyright (C) 2010, 2014 by Michael G. Hansen
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

#ifndef TEST_CORRELATOR_H
#define TEST_CORRELATOR_H

// Qt includes

#include <QtTest/QtTest>

class TestGPXParsing : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNoOp();
    void testCorrelator1();
    void testInterpolation();

};

#endif /* TEST_CORRELATOR_H */
