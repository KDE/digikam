/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-12-13
 * Description : test cases for tags tree manipulation in database
 *
 * Copyright (C) 2015-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

#ifndef DATABASETAGSTEST_H
#define DATABASETAGSTEST_H

// Qt includes

#include <QObject>
#include <QMap>
#include <QAbstractItemModel>

namespace Digikam
{
class PAlbum;
class TAlbum;
class AlbumModel;
}

class DatabaseTagsTest: public QObject
{

    Q_OBJECT

public:

    DatabaseTagsTest();
    virtual ~DatabaseTagsTest();

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();
/*
    void testPAlbumModel();
    void testDisablePAlbumCount();
    void testDAlbumModel();
    void testDAlbumCount();
    void testDAlbumContainsAlbums();
    void testDAlbumSorting();
    void testTAlbumModel();
    void testSAlbumModel();
    void testStartAlbumModel();

    void deletePAlbum(Digikam::PAlbum* album);

    void setLastPAlbumCountMap(const QMap<int, int> &map);

    // slots for ensuring signal order while scanning albums
    void slotStartModelRowsInserted(const QModelIndex& parent, int start, int end);
    void slotStartModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
*/
private:

//    void ensureItemCounts();

private:

    QString              dbPath;
    QString              tempSuffix;

    Digikam::PAlbum*     palbumRoot0;
    Digikam::PAlbum*     palbumRoot1;
    Digikam::PAlbum*     palbumRoot2;
    Digikam::PAlbum*     palbumChild0Root0;
    Digikam::PAlbum*     palbumChild1Root0;
    Digikam::PAlbum*     palbumChild2Root0;
    Digikam::PAlbum*     palbumChild0Root1;

    Digikam::TAlbum*     rootTag;
    Digikam::TAlbum*     talbumRoot0;
    Digikam::TAlbum*     talbumRoot1;
    Digikam::TAlbum*     talbumChild0Root0;
    Digikam::TAlbum*     talbumChild1Root0;
    Digikam::TAlbum*     talbumChild0Child1Root0;
    Digikam::TAlbum*     talbumChild0Root1;

    QList<int>           addedIds;
};

#endif /* DATABASETAGSTEST_H */
