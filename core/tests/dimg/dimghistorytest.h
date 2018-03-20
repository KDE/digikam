/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
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

#ifndef DIMGEHISTORYTEST_H
#define DIMGEHISTORYTEST_H

// Qt includes

#include <QtTest>
#include <QEventLoop>

// Local includes

#include "editorcore.h"
#include "dimgabstracthistorytest.h"

class DImgHistoryTest : public DImgAbstractHistoryTest
{
    Q_OBJECT

public Q_SLOTS:

    void slotImageLoaded(const QString&, bool);
    void slotImageSaved(const QString&, bool);

private Q_SLOTS:

    void testXml();
    void testDImg();

    void initTestCase();
    void cleanupTestCase();
};

#endif // DIMGEHISTORYTEST_H
