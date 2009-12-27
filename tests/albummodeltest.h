/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-11
 * Description : test cases for the various album models
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef ALBUMMODELTEST_H
#define ALBUMMODELTEST_H

// Qt includes

#include <QtCore/QObject>

class AlbumModelTest: public QObject
{
    Q_OBJECT
public:
    AlbumModelTest();
    virtual ~AlbumModelTest();

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testPAlbumModel();
    void testDAlbumModel();
    void testTAlbumModel();
    void testSAlbumModel();

private:

    QString dbPath;
    QString tempSuffix;

};

#endif /* ALBUMMODELTEST_H */
