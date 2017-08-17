/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-06
 * Description : test for the filesaveoptionsbox
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "filesaveoptionsboxtest.h"

// Qt includes

#include <QTest>

// Local inclues

#include "filesaveoptionsbox.h"
#include "dimg.h"

using namespace Digikam;

QTEST_MAIN(FileSaveOptionsBoxTest)

void FileSaveOptionsBoxTest::testDiscoverFormat_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("format");

    QTest::newRow("jpg") << "test.jpg" << (int) DImg::JPEG;
    QTest::newRow("jpeg") << "test.jpeg" << (int) DImg::JPEG;
    QTest::newRow("JPG") << "test.JPG" << (int) DImg::JPEG;
    QTest::newRow("jpg") << "jpg" << (int) DImg::JPEG;
    QTest::newRow("jpeg") << "jpeg" << (int) DImg::JPEG;

    QTest::newRow("bla.tiff.jpeg") << "bla.tiff.jpeg" << (int) DImg::JPEG;
    QTest::newRow("bla.jpg.tiff") << "bla.jpg.tiff" << (int) DImg::TIFF;

#ifdef HAVE_JASPER
    QTest::newRow("bla.png.jpeg.pgx") << "bla.png.jpeg.pgx" << (int) DImg::JP2K;
#endif // HAVE_JASPER

    QTest::newRow("pgf") << "PGF" << (int) DImg::PGF;

    QTest::newRow("unknwon") << "i.dont.know" << (int) DImg::NONE; // krazy:exclude=spelling
}

void FileSaveOptionsBoxTest::testDiscoverFormat()
{
    QFETCH(QString, filename);
    QFETCH(int, format);

    FileSaveOptionsBox box;
    QCOMPARE((int) box.discoverFormat(filename), format);
}

void FileSaveOptionsBoxTest::testDiscoverFormatDefault()
{
    FileSaveOptionsBox box;
    QCOMPARE(box.discoverFormat(QLatin1String("unknown")), DImg::NONE);
    QCOMPARE(box.discoverFormat(QLatin1String("unknown"), DImg::PGF), DImg::PGF);
}
