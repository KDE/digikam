/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-28
 * @brief  Test loading and saving of data in KipiImageItem.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
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

#ifndef TEST_KIPIIMAGEITEM_H
#define TEST_KIPIIMAGEITEM_H

// Qt includes

#include <QtTest/QtTest>

// KDE includes

// local includes

class TestKipiImageItem : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void testNoOp();
    void testBasicLoading();

};

#endif /* TEST_KIPIIMAGEITEM_H */

