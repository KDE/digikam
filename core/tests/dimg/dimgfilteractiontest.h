/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#ifndef DIGIKAM_DIMG_FILTER_ACTION_TEST_H
#define DIGIKAM_DIMG_FILTER_ACTION_TEST_H

// Qt includes

#include <QtTest>
#include <QEventLoop>
#include <QDir>

// Local includes

#include "dimg.h"

using namespace Digikam;

class DImgFilterActionTest : public QObject
{
    Q_OBJECT

public:

    QDir    imageDir();
    QString originalImage();

    void showDiff(const DImg& orig, const DImg& ref, const DImg& result, const DImg& diff);

private Q_SLOTS:

    void testDRawDecoding();
    void testActions();

    void initTestCase();
    void cleanupTestCase();
};

#endif // DIGIKAM_DIMG_FILTER_ACTION_TEST_H
