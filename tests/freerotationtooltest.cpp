/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the freerotationtool
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "freerotationtooltest.h"
#include "freerotationtooltest.moc"

// KDE includes

#include "qtest_kde.h"

// Local includes

#include "freerotationtool.h"

using namespace DigikamFreeRotationImagesPlugin;
using namespace Digikam;

QTEST_KDEMAIN(FreeRotationToolTest, GUI)

void FreeRotationToolTest::testCalculateAngle_data()
{
    QTest::addColumn<QPoint>("p1");
    QTest::addColumn<QPoint>("p2");
    QTest::addColumn<double>("result");

    QTest::newRow("empty")      << QPoint()      << QPoint()       << 0.0;
    QTest::newRow("p1=p2")      << QPoint(10,10) << QPoint(10,10)  << 0.0;
    QTest::newRow("p1.x=p2.x")  << QPoint(10,10) << QPoint(200,10) << 0.0;
    QTest::newRow("p1.y=p2.y")  << QPoint(10,10) << QPoint(10,200) << 90.0;
    QTest::newRow("45 degrees") << QPoint(10,10) << QPoint(20,20)  << -45.0;
    QTest::newRow("45 degrees") << QPoint(10,20) << QPoint(20,10)  << 45.0;
}

void FreeRotationToolTest::testCalculateAngle()
{
    QFETCH(QPoint, p1);
    QFETCH(QPoint, p2);
    QFETCH(double, result);

    double angle = FreeRotationTool::calculateAngle(p1, p2);
    QCOMPARE(angle, result);
}
