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

#include <QObject>
#include <QMap>
#include <QAbstractItemModel>

namespace Digikam
{
class PAlbum;
class TAlbum;
class AlbumModel;
}

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

private:

    void ensureItemCounts();

private:

    const QString        albumCategory;

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

    QMap<int, int>       palbumCountMap;

    /**
     * This model is used to ensure that adding and changing signals are emitted
     * correctly if the model is created beorre the scanning starts.
     */
    Digikam::AlbumModel* startModel;
    QList<int>           addedIds;
};

#endif /* ALBUMMODELTEST_H */
