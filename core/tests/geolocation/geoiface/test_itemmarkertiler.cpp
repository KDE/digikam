/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : test for the model holding markers
 *
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "test_itemmarkertiler.h"

// Qt includes

#include <QStandardItemModel>
#include <QDebug>

// Local includes

#include "geoifacecommon.h"

using namespace Digikam;

const int CoordinatesRole = Qt::UserRole + 0;

MarkerModelHelper::MarkerModelHelper(QAbstractItemModel* const itemModel,
                                     QItemSelectionModel* const itemSelectionModel)
    : GeoModelHelper(itemModel),
      m_itemModel(itemModel),
      m_itemSelectionModel(itemSelectionModel)
{
    connect(itemModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));
}

MarkerModelHelper::~MarkerModelHelper()
{
}

void MarkerModelHelper::slotDataChanged(const QModelIndex& topLeft,
                                        const QModelIndex& bottomRight)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    emit(signalModelChangedDrastically());
}

QAbstractItemModel* MarkerModelHelper::model() const
{
    return m_itemModel;
}

QItemSelectionModel* MarkerModelHelper::selectionModel() const
{
    return m_itemSelectionModel;
}

bool MarkerModelHelper::itemCoordinates(const QModelIndex& index,
                                        GeoCoordinates* const coordinates) const
{
    if (!index.data(CoordinatesRole).canConvert<GeoCoordinates>())
        return false;

    if (coordinates)
        *coordinates = index.data(CoordinatesRole).value<GeoCoordinates>();

    return true;
}

const GeoCoordinates coord_1_2     = GeoCoordinates::fromGeoUrl(QLatin1String("geo:1,2"));
const GeoCoordinates coord_50_60   = GeoCoordinates::fromGeoUrl(QLatin1String("geo:50,60"));
const GeoCoordinates coord_m50_m60 = GeoCoordinates::fromGeoUrl(QLatin1String("geo:-50,-60"));

QStandardItem* MakeItemAt(const GeoCoordinates& coordinates)
{
    QStandardItem* const newItem = new QStandardItem(coordinates.geoUrl());
    newItem->setData(QVariant::fromValue(coordinates), CoordinatesRole);

    return newItem;
}

/**
 * @brief Helper function: count the number of markers found by an iterator
 */
int CountMarkersInIterator(ItemMarkerTiler::NonEmptyIterator* const it)
{
    int markerCount = 0;

    while (!it->atEnd())
    {
        const TileIndex currentIndex = it->currentIndex();
        markerCount                 += it->model()->getTileMarkerCount(currentIndex);
        it->nextIndex();
//         qDebug()<<currentIndex;
    }

    return markerCount;
}

void TestItemMarkerTiler::testNoOp()
{
}

void TestItemMarkerTiler::testIndices()
{
    const int maxLevel = TileIndex::MaxLevel;

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex = TileIndex::fromCoordinates(coord_1_2, l);
        QVERIFY(tileIndex.level() == l);
    }
}

void TestItemMarkerTiler::testAddMarkers1()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));

    const int maxLevel = TileIndex::MaxLevel;

    // there should be no tiles in the model yet:
    for (int l = 0; l <= maxLevel; ++l)
    {
        QVERIFY(mm.getTile(TileIndex::fromCoordinates(coord_50_60, l), true) == 0);
    }

    itemModel->appendRow(MakeItemAt(coord_50_60));

    // now there should be tiles with one marker:
    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->childrenEmpty());
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    itemModel->appendRow(MakeItemAt(coord_50_60));

    // now there should be tiles with two markers:
    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
    }
}

void TestItemMarkerTiler::testRemoveMarkers2()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));

    const int maxLevel = TileIndex::MaxLevel;

    itemModel->appendRow(MakeItemAt(coord_50_60));
    QStandardItem* const item2 = MakeItemAt(coord_50_60);
    itemModel->appendRow(item2);

    // now there should be tiles with two markers:
    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
    }

    // remove one item:
    qDeleteAll(itemModel->takeRow(itemModel->indexFromItem(item2).row()));

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
    }
}

void TestItemMarkerTiler::testMoveMarkers1()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel  = TileIndex::MaxLevel;
    const int fillLevel = maxLevel - 2;

    // add a marker to the model and create tiles up to a certain level:
    QStandardItem* const item1     = MakeItemAt(coord_1_2);
    itemModel->appendRow(item1);
    const QModelIndex markerIndex1 = itemModel->indexFromItem(item1);

    GEOIFACE_ASSERT(markerIndex1.isValid());

    for (int l = 1; l <= fillLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_1_2, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->childrenEmpty());
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
    }

    // now move marker 1:
    itemModel->setData(markerIndex1, QVariant::fromValue(coord_50_60), CoordinatesRole);

    for (int l = 0; l <= fillLevel; ++l)
    {
        // make sure the marker can not be found at the old position any more
        TileIndex tileIndex = TileIndex::fromCoordinates(coord_1_2, l);
        QVERIFY(mm.getTile(tileIndex, true) == 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 0);
        QVERIFY(mm.getTile(tileIndex, true) == 0);

        // find it at the new position:
        tileIndex                           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->childrenEmpty());
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

//     mm.clear();
}

void TestItemMarkerTiler::testMoveMarkers2()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel = TileIndex::MaxLevel;

    const int fillLevel = maxLevel - 2;

    // add markers to the model and create tiles up to a certain level:
    QStandardItem* const item1     = MakeItemAt(coord_1_2);
    itemModel->appendRow(item1);
    const QModelIndex markerIndex1 = itemModel->indexFromItem(item1);
    QStandardItem* const item2     = MakeItemAt(coord_1_2);
    itemModel->appendRow(item2);
//    const QModelIndex markerIndex2 = itemModel->indexFromItem(item2);

    for (int l = 1; l <= fillLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_1_2, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->childrenEmpty());
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 2);
    }

    QStandardItem* const item3     = MakeItemAt(coord_50_60);
    itemModel->appendRow(item3);
//    const QModelIndex markerIndex3 = itemModel->indexFromItem(item3);

    for (int l = 1; l <= fillLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(myTile->childrenEmpty());
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
    }

    // now move marker 1:
    itemModel->setData(markerIndex1, QVariant::fromValue(coord_50_60), CoordinatesRole);

    // make sure the item model was also updated:
    QVERIFY(item1->data(CoordinatesRole).value<GeoCoordinates>() == coord_50_60);

    for (int l = 0; l <= fillLevel; ++l)
    {
        // make sure there is only one marker left at the old position:
        TileIndex tileIndex = TileIndex::fromCoordinates(coord_1_2, l);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);

        // find it at the new position:
        tileIndex                           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);

        if (l > fillLevel)
        {
            QVERIFY(myTile->childrenEmpty());
        }

        QVERIFY(mm.getTileMarkerCount(tileIndex) == 2);
    }

//     mm.clear();
}

void TestItemMarkerTiler::testIteratorWholeWorldNoBackingModel()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel = TileIndex::MaxLevel;

    for (int l = 0; l <= maxLevel; ++l)
    {
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 0 );
    }
}

void TestItemMarkerTiler::testIteratorWholeWorld()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel = TileIndex::MaxLevel;

    for (int l = 0; l <= maxLevel; ++l)
    {
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 0 );
    }

    itemModel->appendRow(MakeItemAt(coord_1_2));
    itemModel->appendRow(MakeItemAt(coord_50_60));

    for (int l = 0; l <= maxLevel; ++l)
    {
        // iterate over the whole world:
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 2 );
    }
}

void TestItemMarkerTiler::testIteratorPartial1()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel = TileIndex::MaxLevel;

    itemModel->appendRow(MakeItemAt(coord_1_2));
    itemModel->appendRow(MakeItemAt(coord_50_60));

    for (int l = 0; l <= maxLevel; ++l)
    {
        {
            // iterate over a part which should be empty:
            GeoCoordinates::PairList boundsList;
            boundsList << GeoCoordinates::makePair(-10.0, -10.0, -5.0, -5.0);
            ItemMarkerTiler::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 0 );
        }

        {
            // iterate over a part which should contain one marker:
            GeoCoordinates::PairList boundsList;
            boundsList << GeoCoordinates::makePair(-10.0, -10.0, 5.0, 5.0);
            ItemMarkerTiler::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 1 );

            // iterate over a part which should contain one marker:
            GeoCoordinates::PairList boundsList1;
            boundsList1 << GeoCoordinates::makePair(1.0, 2.0, 5.0, 5.0);
            ItemMarkerTiler::NonEmptyIterator it1(&mm, l, boundsList1);
            QVERIFY( CountMarkersInIterator(&it1) == 1 );

            GeoCoordinates::PairList boundsList2;
            boundsList2 << GeoCoordinates::makePair(-1.0, -2.0, 1.0, 2.0);
            ItemMarkerTiler::NonEmptyIterator it2(&mm, l, boundsList2);
            QVERIFY( CountMarkersInIterator(&it2) == 1 );
        }

        {
            // iterate over a part which should contain two markers:
            GeoCoordinates::PairList boundsList;
            boundsList << GeoCoordinates::makePair(0.0, 0.0, 60.0, 60.0);
            ItemMarkerTiler::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 2 );
        }

        {
            // iterate over two parts which should contain two markers:
            GeoCoordinates::PairList boundsList;
            boundsList << GeoCoordinates::makePair(0.0, 0.0, 5.0, 5.0);
            boundsList << GeoCoordinates::makePair(49.0, 59.0, 51.0, 61.0);
            ItemMarkerTiler::NonEmptyIterator it(&mm, l, boundsList);
            QVERIFY( CountMarkersInIterator(&it) == 2 );
        }
    }

    const GeoCoordinates coord_2_2 = GeoCoordinates(2.0, 2.0);

    itemModel->appendRow(MakeItemAt(coord_2_2));
    {
        // at level 1, the iterator should find only one marker:
        GeoCoordinates::PairList boundsList;
        boundsList << GeoCoordinates::makePair(0.0, 0.0, 1.0, 2.0);
        ItemMarkerTiler::NonEmptyIterator it(&mm, 1, boundsList);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestItemMarkerTiler::testIteratorPartial2()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel = TileIndex::MaxLevel;

    GeoCoordinates::PairList boundsList;
    boundsList << GeoCoordinates::makePair(0.55, 1.55, 0.56, 1.56);

    const GeoCoordinates coordInBounds1    = GeoCoordinates(0.556, 1.556);
    const GeoCoordinates coordOutOfBounds1 = GeoCoordinates(0.5, 1.5);
    const GeoCoordinates coordOutOfBounds2 = GeoCoordinates(0.5, 1.6);
    const GeoCoordinates coordOutOfBounds3 = GeoCoordinates(0.6, 1.5);
    const GeoCoordinates coordOutOfBounds4 = GeoCoordinates(0.6, 1.6);
    itemModel->appendRow(MakeItemAt(coordInBounds1));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds1));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds2));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds3));
    itemModel->appendRow(MakeItemAt(coordOutOfBounds4));

    for (int l = 3; l <= maxLevel; ++l)
    {
        ItemMarkerTiler::NonEmptyIterator it(&mm, l, boundsList);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestItemMarkerTiler::testRemoveMarkers1()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
    const int maxLevel = TileIndex::MaxLevel;

    for (int l = 0; l <= maxLevel; ++l)
    {
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QVERIFY(CountMarkersInIterator(&it) == 0 );
    }

    QStandardItem* const item1 = MakeItemAt(coord_1_2);
    itemModel->appendRow(item1);
    itemModel->appendRow(MakeItemAt(coord_50_60));

    for (int l = 0; l <= maxLevel; ++l)
    {
        // iterate over the whole world:
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QCOMPARE(CountMarkersInIterator(&it), 2);
    }

    // first make sure that comparison of indices still works
    const QPersistentModelIndex index1 = itemModel->indexFromItem(item1);
    const QPersistentModelIndex index2 = itemModel->indexFromItem(item1);
    QCOMPARE(index1, index2);

    // now remove items:
    QCOMPARE(itemModel->takeRow(itemModel->indexFromItem(item1).row()).count(), 1);
    delete item1;
    QCOMPARE(itemModel->rowCount(), 1);

    for (int l = 0; l <= maxLevel; ++l)
    {
        // iterate over the whole world:
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QCOMPARE(CountMarkersInIterator(&it), 1);
    }
}

/**
 * @brief Make sure that items which are in the model before it is given to the tiled model are found by the tile model
 */
void TestItemMarkerTiler::testPreExistingMarkers()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    itemModel->appendRow(MakeItemAt(coord_50_60));
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));

    const int maxLevel = TileIndex::MaxLevel;

    for (int l = 0; l <= maxLevel; ++l)
    {
        // iterate over the whole world:
        ItemMarkerTiler::NonEmptyIterator it(&mm, l);
        QVERIFY( CountMarkersInIterator(&it) == 1 );
    }
}

void TestItemMarkerTiler::testSelectionState1()
{
    QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
    QItemSelectionModel* const selectionModel = new QItemSelectionModel(itemModel.data());
    ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), selectionModel));

    const int maxLevel         = TileIndex::MaxLevel;

    QStandardItem* const item1 = MakeItemAt(coord_50_60);
    item1->setSelectable(true);
    itemModel->appendRow(item1);
    QModelIndex item1Index     = itemModel->indexFromItem(item1);

    // verify the selection state of the tiles:
    // make sure we do not create tiles all the way down,
    // because we want to test whether the state is okay in newly created tiles
    const int preMaxLevel = maxLevel -2;

    for (int l = 0; l <= preMaxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QVERIFY(mm.getTileMarkerCount(tileIndex) == 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedNone);
    }

    selectionModel->select(item1Index, QItemSelectionModel::Select);

    // verify the selection state of the tiles:
    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
        QVERIFY(mm.getTileSelectedCount(tileIndex)==1);
    }

    // add an unselected item and make sure the tilecount is still correct
    QStandardItem* const item2             = MakeItemAt(coord_50_60);
    item2->setSelectable(true);
    itemModel->appendRow(item2);
    const QPersistentModelIndex item2Index = itemModel->indexFromItem(item2);

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedSome);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
    }

    selectionModel->select(item2Index, QItemSelectionModel::Select);

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 2);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    // now remove the selected item:
    QCOMPARE(itemModel->takeRow(item2Index.row()).count(), 1);
    delete item2;

    // verify the selection state of the tiles:
    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    // add a selected item and then move it:
    QStandardItem* const item3             = MakeItemAt(coord_1_2);
    item3->setSelectable(true);
    itemModel->appendRow(item3);
    const QPersistentModelIndex item3Index = itemModel->indexFromItem(item3);
    selectionModel->select(item3Index, QItemSelectionModel::Select);

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_1_2, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    itemModel->setData(item3Index, QVariant::fromValue(coord_50_60), CoordinatesRole);

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 2);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 2);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    itemModel->setData(item3Index, QVariant::fromValue(coord_m50_m60), CoordinatesRole);

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_50_60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    for (int l = 0; l <= maxLevel; ++l)
    {
        const TileIndex tileIndex           = TileIndex::fromCoordinates(coord_m50_m60, l);
        ItemMarkerTiler::Tile* const myTile = mm.getTile(tileIndex, true);
        QVERIFY(myTile != 0);
        QCOMPARE(mm.getTileMarkerCount(tileIndex), 1);
        QCOMPARE(mm.getTileSelectedCount(tileIndex), 1);
        QVERIFY(mm.getTileGroupState(tileIndex)==SelectedAll);
    }

    // TODO: set a model with selected items, make sure the selections are read out
    //       this is currently implemented by simply setting the tiles as dirty
}

void TestItemMarkerTiler::benchmarkIteratorWholeWorld()
{
    return;
#if 0
//     QBENCHMARK
    {
        QScopedPointer<QStandardItemModel> itemModel(new QStandardItemModel());
        ItemMarkerTiler mm(new MarkerModelHelper(itemModel.data(), 0));
        const int maxLevel = TileIndex::MaxLevel;

        {
            int l = maxLevel-1;
            ItemMarkerTiler::NonEmptyIterator it(&mm, l);
            QVERIFY( CountMarkersInIterator(&it) == 0 );
        }

        itemModel->appendRow(MakeItemAt(coord_1_2));
        itemModel->appendRow(MakeItemAt(coord_50_60));

        for (qreal x = -50; x < 50; x+=1.0)
        {
            for (qreal y = -50; y < 50; y+=1.0)
            {
                itemModel->appendRow(MakeItemAt(GeoCoordinates(x,y)));
            }
        }

//         QBENCHMARK
        {
            const int l=maxLevel;
//             for (int l = 0; l <= maxLevel; ++l)
            {
                // iterate over the whole world:
                ItemMarkerTiler::NonEmptyIterator it(&mm, l);
                CountMarkersInIterator(&it);
            }
        }
    }
#endif
}

QTEST_GUILESS_MAIN(TestItemMarkerTiler)
