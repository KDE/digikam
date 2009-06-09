/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : Jun 9, 2009
 * Description : a test for the ManualRenameInput widget
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

#include "manualrenameinputtest.h"
#include "manualrenameinputtest.moc"

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "manualrenameinput.h"

using namespace Digikam;

QTEST_KDEMAIN(ManualRenameInputTest, GUI)

void ManualRenameInputTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("#") << QString("#") << filename << cameraName << curdate << 1
                       << QString("1");

    QTest::newRow("##") << QString("##") << filename << cameraName << curdate << 2
                        << QString("02");

    QTest::newRow("###") << QString("###") << filename << cameraName << curdate << 4
                         << QString("004");

    QTest::newRow("### (40)") << QString("###") << filename << cameraName << curdate << 40
                              << QString("040");

    QTest::newRow("###_bla_##") << QString("###_bla_##") << filename << cameraName << curdate << 10
                                << QString("010_bla_10");

    QTest::newRow("####{2,3}") << QString("####{2,3}") << filename << cameraName << curdate << 1
                               << QString("0002");

    QTest::newRow("####{2,3}(10)") << QString("####{2,3}") << filename << cameraName << curdate << 10
                                   << QString("0029");

    QTest::newRow("####{2,3}_bla_## ###") << QString("####{2,3}_bla_## ###") << filename << cameraName << curdate << 1
                                          << QString("0002_bla_01 001");

}

void ManualRenameInputTest::testNumberToken()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}
