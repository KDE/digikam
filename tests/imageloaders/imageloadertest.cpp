/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-25
 * Description : a test for the imageloader classes
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

#include "imageloadertest.h"
#include "imageloadertest.moc"

// Qt includes

#include <QImage>

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "pngloader.h"

using namespace Digikam;

QTEST_KDEMAIN(ImageLoaderTest, GUI)

void ImageLoaderTest::testLoadPNG()
{
    QString filename(KDESRCDIR"/test.png");
    QImage src(filename);
    QImage dst;
    bool success = true;

    DImg dimg;
    PNGLoader loader(&dimg);
    success = loader.load(filename, 0);
    QVERIFY(success);

    // compare basic values
    QCOMPARE((uint)src.width(),     dimg.width());
    QCOMPARE((uint)src.height(),    dimg.height());
    QCOMPARE(src.hasAlphaChannel(), dimg.hasAlpha());
    QCOMPARE(src.depth(),           dimg.bitsDepth());
}
