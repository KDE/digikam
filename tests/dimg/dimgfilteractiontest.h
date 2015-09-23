/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-13
 * Description : a test for applying FilterActions
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIMGFILTERACTIONTEST_H
#define DIMGFILTERACTIONTEST_H

// Qt includes

#include <QtTest/QtTest>
#include <QEventLoop>
#include <QtCore/QDir>

// Local includes

namespace Digikam
{
class DImg;
}

class DImgFilterActionTest : public QObject
{
    Q_OBJECT

public:

    QDir    imageDir();
    QString originalImage();

    void showDiff(const Digikam::DImg& orig, const Digikam::DImg& ref, const Digikam::DImg& result, const Digikam::DImg& diff);

private Q_SLOTS:

    void testDRawDecoding();
    void testActions();

    void initTestCase();
    void cleanupTestCase();
};

#endif // DIMGFILTERACTIONTEST_H
