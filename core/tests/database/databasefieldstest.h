/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-06-29
 * Description : Test the functions for dealing with DatabaseFields
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DATABASEFIELDSTEST_H
#define DATABASEFIELDSTEST_H

// Qt includes

#include <QtTest>

class DatabaseFieldsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testMinSizeType();
    void testIterators();
    void testMetaInfo();
    void testIteratorsSetOnly();
    void testSet();
    void testSetHashAddSets();
    void testHashRemoveAll();

    void initTestCase();
    void cleanupTestCase();
};

#endif // DATABASEFIELDSTEST_H
