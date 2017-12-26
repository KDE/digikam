/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-17
 * Description : A marker tiler operating on item models
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemmarkertiler.h"

// local includes

#include "geomodelhelper.h"
#include "digikam_debug.h"
#include "geoifacecommon.h"

namespace Digikam
{

class ItemMarkerTiler::MyTile : public Tile
{
public:

    MyTile()
        : Tile(),
          markerIndices(),
          selectedCount(0)
    {
    }

    /**
     * Note: MyTile is only deleted by ItemMarkerTiler::tileDelete.
     * All subclasses of AbstractMarkerTiler have to reimplement tileDelete
     * to delete their Tile subclasses.
     * This was done in order not to have any virtual functions
     * in Tile and its subclasses in order to save memory, since there
     * can be a lot of tiles in a MarkerTiler.
     */
    virtual ~MyTile()
    {
    }

    void removeMarkerIndexOrInvalidIndex(const QModelIndex& indexToRemove);

public:

    QList<QPersistentModelIndex> markerIndices;
    int                          selectedCount;
};

void ItemMarkerTiler::MyTile::removeMarkerIndexOrInvalidIndex(const QModelIndex& indexToRemove)
{
    int i=0;

    while (i < markerIndices.count())
    {
        const QPersistentModelIndex& currentIndex = markerIndices.at(i);

        // NOTE: this function is usually called after the model has sent
        //       an aboutToRemove-signal. It is possible that the persistent
        //       marker index became invalid before the caller received the signal.
        //       we remove any invalid indices as we find them.
        if ( !currentIndex.isValid() )
        {
            markerIndices.takeAt(i);
            continue;
        }

        if ( currentIndex == indexToRemove )
        {
            markerIndices.takeAt(i);
            return;
        }

        ++i;
    }
}

// -------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemMarkerTiler::Private
{
public:

    Private()
      : modelHelper(0),
        selectionModel(0),
        markerModel(0),
        activeState(false)
    {
    }

    GeoModelHelper*         modelHelper;
    QItemSelectionModel* selectionModel;
    QAbstractItemModel*  markerModel;
    bool                 activeState;
};

ItemMarkerTiler::ItemMarkerTiler(GeoModelHelper* const modelHelper, QObject* const parent)
    : AbstractMarkerTiler(parent),
      d(new Private())
{
    resetRootTile();
    setMarkerGeoModelHelper(modelHelper);
}

ItemMarkerTiler::~ItemMarkerTiler()
{
    // WARNING: we have to call clear! By the time AbstractMarkerTiler calls clear,
    // this object does not exist any more, and thus the tiles are not correctly destroyed!
    clear();

    delete d;
}

void ItemMarkerTiler::setMarkerGeoModelHelper(GeoModelHelper* const modelHelper)
{
    d->modelHelper    = modelHelper;
    d->markerModel    = modelHelper->model();
    d->selectionModel = modelHelper->selectionModel();

    if (d->markerModel != 0)
    {
        // TODO: disconnect the old model if there was one
        connect(d->markerModel, &QAbstractItemModel::rowsInserted,
                this, &ItemMarkerTiler::slotSourceModelRowsInserted);

        connect(d->markerModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                this, &ItemMarkerTiler::slotSourceModelRowsAboutToBeRemoved);

        // TODO: this signal now has to be monitored in the model helper
//         connect(d->markerModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
//                 this, SLOT(slotSourceModelDataChanged(QModelIndex,QModelIndex)));

        connect(d->modelHelper, &GeoModelHelper::signalModelChangedDrastically,
                this, &ItemMarkerTiler::slotSourceModelReset);

        connect(d->markerModel, &QAbstractItemModel::modelReset, this,
                &ItemMarkerTiler::slotSourceModelReset);

        connect(d->markerModel, &QAbstractItemModel::layoutChanged, this,
                &ItemMarkerTiler::slotSourceModelLayoutChanged);

        connect(d->modelHelper, &GeoModelHelper::signalThumbnailAvailableForIndex,
                this, &ItemMarkerTiler::slotThumbnailAvailableForIndex);

        if (d->selectionModel)
        {
            connect(d->selectionModel, &QItemSelectionModel::selectionChanged,
                    this, &ItemMarkerTiler::slotSelectionChanged);
        }
    }

    setDirty();
}

QVariant ItemMarkerTiler::getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey)
{
    const QList<QPersistentModelIndex> modelIndices = getTileMarkerIndices(tileIndex);

    if (modelIndices.isEmpty())
        return QVariant();

    return QVariant::fromValue(d->modelHelper->bestRepresentativeIndexFromList(modelIndices, sortKey));
}

QPixmap ItemMarkerTiler::pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)
{
    return d->modelHelper->pixmapFromRepresentativeIndex(index.value<QPersistentModelIndex>(), size);
}

QVariant ItemMarkerTiler::bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)
{
    QList<QPersistentModelIndex> indexList;

    for (int i = 0; i < indices.count(); ++i)
    {
        indexList << indices.at(i).value<QPersistentModelIndex>();
    }

    return QVariant::fromValue(d->modelHelper->bestRepresentativeIndexFromList(indexList, sortKey));
}

void ItemMarkerTiler::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
//     qCDebug(DIGIKAM_GEOIFACE_LOG)<<selected<<deselected;
    if (isDirty())
    {
        return;
    }
//     d->isDirty=true;
//     emit(signalTilesOrSelectionChanged());
//     return;

    for (int i = 0; i < selected.count(); ++i)
    {
        const QItemSelectionRange selectionRange = selected.at(i);

        for (int row = selectionRange.top(); row <= selectionRange.bottom(); ++row)
        {
            // get the coordinates of the item
            GeoCoordinates coordinates;

            if (!d->modelHelper->itemCoordinates(d->markerModel->index(row, 0, selectionRange.parent()), &coordinates))
                continue;

            for (int l = 0; l <= TileIndex::MaxLevel; ++l)
            {
                const TileIndex tileIndex = TileIndex::fromCoordinates(coordinates, l);
                MyTile* const myTile      = static_cast<MyTile*>(getTile(tileIndex, true));

                if (!myTile)
                    break;

                myTile->selectedCount++;
//              qCDebug(DIGIKAM_GEOIFACE_LOG) << l << tileIndex << myTile->selectedCount;
                GEOIFACE_ASSERT(myTile->selectedCount <= myTile->markerIndices.count());

                if (myTile->childrenEmpty())
                    break;
            }
        }
    }

    for (int i = 0; i < deselected.count(); ++i)
    {
        const QItemSelectionRange selectionRange = deselected.at(i);

        for (int row = selectionRange.top(); row <= selectionRange.bottom(); ++row)
        {
            // get the coordinates of the item
            GeoCoordinates coordinates;

            if (!d->modelHelper->itemCoordinates(d->markerModel->index(row, 0, selectionRange.parent()), &coordinates))
                continue;

            for (int l = 0; l <= TileIndex::MaxLevel; ++l)
            {
                const TileIndex tileIndex = TileIndex::fromCoordinates(coordinates, l);
                MyTile* const myTile      = static_cast<MyTile*>(getTile(tileIndex, true));

                if (!myTile)
                    break;

                myTile->selectedCount--;
                GEOIFACE_ASSERT(myTile->selectedCount >= 0);

                if (myTile->childrenEmpty())
                    break;
            }
        }
    }

    emit(signalTilesOrSelectionChanged());
}

void ItemMarkerTiler::slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << topLeft << bottomRight;
    setDirty();

    if (d->activeState)
        emit signalTilesOrSelectionChanged();

    // TODO: if only a few items were changed, try to see whether they are still in the right tiles
}

void ItemMarkerTiler::slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end)
{
    if (isDirty())
    {
        // rows will be added once the tiles are regenerated
        return;
    }

    // sort the new items into our tiles:
    for (int i = start; i <= end; ++i)
    {
        addMarkerIndexToGrid(QPersistentModelIndex(d->markerModel->index(i, 0, parentIndex)));
    }

    emit(signalTilesOrSelectionChanged());
}

void ItemMarkerTiler::slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end)
{
    // TODO: emit(signalTilesOrSelectionChanged()); in rowsWereRemoved
#if QT_VERSION < 0x040600
    // removeMarkerIndexFromGrid does not work in Qt 4.5 because the model has already deleted all
    // the data of the item, but we need the items coordinates to work efficiently
    setDirty();
    return;
#else
    if (isDirty())
    {
        return;
    }

    // remove the items from their tiles:
    for (int i = start; i <= end; ++i)
    {
        const QModelIndex itemIndex = d->markerModel->index(start, 0, parentIndex);

        // remove the marker from the grid, but leave the selection count alone because the
        // selection model will send a signal about the deselection of the marker
        removeMarkerIndexFromGrid(itemIndex, true);
    }
#endif
}

void ItemMarkerTiler::slotThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap)
{
    emit(signalThumbnailAvailableForIndex(QVariant::fromValue(index), pixmap));
}

void ItemMarkerTiler::slotSourceModelReset()
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "----";
    setDirty();
}

/**
 * @brief Remove a marker from the grid
 * @param ignoreSelection Do not remove the marker from the count of selected items.
 *                        This is only used by slotSourceModelRowsAboutToBeRemoved internally,
 *                        because the selection model sends us an extra signal about the deselection.
 */
void ItemMarkerTiler::removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection)
{
    if (isDirty())
    {
        // if the model is dirty, there is no need to remove the marker
        // because the tiles will be regenerated on the next call
        // that requests data
        return;
    }

    GEOIFACE_ASSERT(markerIndex.isValid());

    bool markerIsSelected = false;

    if (d->selectionModel)
    {
        markerIsSelected = d->selectionModel->isSelected(markerIndex);
    }

    // remove the marker from the grid:
    GeoCoordinates markerCoordinates;

    if (!d->modelHelper->itemCoordinates(markerIndex, &markerCoordinates))
        return;

    const TileIndex tileIndex = TileIndex::fromCoordinates(markerCoordinates, TileIndex::MaxLevel);
    QList<MyTile*> tiles;

    // here l functions as the number of indices that we actually use, therefore we have to go one more up
    // in this case, l==0 returns the root tile
    for (int l = 0; l <= TileIndex::MaxLevel+1; ++l)
    {
        MyTile* const currentTile = static_cast<MyTile*>(getTile(tileIndex.mid(0, l), true));

        if (!currentTile)
            break;

        tiles << currentTile;
        currentTile->removeMarkerIndexOrInvalidIndex(markerIndex);

        if (markerIsSelected&&!ignoreSelection)
        {
            currentTile->selectedCount--;
            GEOIFACE_ASSERT(currentTile->selectedCount >= 0);
        }
    }

    // delete the tiles which are now empty!
    for (int l = tiles.count()-1; l > 0; --l)
    {
        MyTile* const currentTile = tiles.at(l);

        if (!currentTile->markerIndices.isEmpty())
            break;

        MyTile* const parentTile = tiles.at(l-1);
        tileDeleteChild(parentTile, currentTile);
    }
}

int ItemMarkerTiler::getTileMarkerCount(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    GEOIFACE_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return 0;
    }

    return myTile->markerIndices.count();
}

int ItemMarkerTiler::getTileSelectedCount(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    GEOIFACE_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return 0;
    }

    return myTile->selectedCount;
}

GeoGroupState ItemMarkerTiler::getTileGroupState(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    GEOIFACE_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return SelectedNone;
    }

    const int selectedCount = myTile->selectedCount;

    if (selectedCount == 0)
    {
        return SelectedNone;
    }
    else if (selectedCount == myTile->markerIndices.count())
    {
        return SelectedAll;
    }

    return SelectedSome;
}

AbstractMarkerTiler::Tile* ItemMarkerTiler::getTile(const TileIndex& tileIndex, const bool stopIfEmpty)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    GEOIFACE_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    MyTile* tile = static_cast<MyTile*>(rootTile());

    for (int level = 0; level < tileIndex.indexCount(); ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);
        MyTile* childTile      = 0;

        if (tile->childrenEmpty())
        {
            // if there are any markers in the tile,
            // we have to sort them into the child tiles:
            if (!tile->markerIndices.isEmpty())
            {
                for (int i = 0; i < tile->markerIndices.count(); ++i)
                {
                    const QPersistentModelIndex currentMarkerIndex = tile->markerIndices.at(i);
                    GEOIFACE_ASSERT(currentMarkerIndex.isValid());

                    // get the tile index for this marker:
                    GeoCoordinates currentMarkerCoordinates;

                    if (!d->modelHelper->itemCoordinates(currentMarkerIndex, &currentMarkerCoordinates))
                        continue;

                    const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentMarkerCoordinates, level);
                    const int newTileIndex          = markerTileIndex.toIntList().last();

                    MyTile* newTile = static_cast<MyTile*>(tile->getChild(newTileIndex));

                    if (newTile == 0)
                    {
                        newTile = static_cast<MyTile*>(tileNew());
                        tile->addChild(newTileIndex, newTile);
                    }

                    newTile->markerIndices<<currentMarkerIndex;

                    if (d->selectionModel)
                    {
                        if (d->selectionModel->isSelected(currentMarkerIndex))
                        {
                            newTile->selectedCount++;
                        }
                    }
                }
            }
        }

        childTile = static_cast<MyTile*>(tile->getChild(currentIndex));

        if (childTile == 0)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop
                return 0;
            }

            childTile = static_cast<MyTile*>(tileNew());
            tile->addChild(currentIndex, childTile);
        }

        tile = childTile;
    }

    return tile;
}

QList<QPersistentModelIndex> ItemMarkerTiler::getTileMarkerIndices(const TileIndex& tileIndex)
{
    if (isDirty())
    {
        regenerateTiles();
    }

    GEOIFACE_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return QList<QPersistentModelIndex>();
    }

    return myTile->markerIndices;
}

void ItemMarkerTiler::addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex)
{
    if (isDirty())
    {
        // the model is dirty, so let regenerateTiles do the rest
        regenerateTiles();
        return;
    }

    GeoCoordinates markerCoordinates;

    if (!d->modelHelper->itemCoordinates(markerIndex, &markerCoordinates))
        return;

    TileIndex tileIndex = TileIndex::fromCoordinates(markerCoordinates, TileIndex::MaxLevel);
    GEOIFACE_ASSERT(tileIndex.level() == TileIndex::MaxLevel);

    bool markerIsSelected = false;

    if (d->selectionModel)
    {
        markerIsSelected = d->selectionModel->isSelected(markerIndex);
    }

    // add the marker to all existing tiles:
    MyTile* currentTile = static_cast<MyTile*>(rootTile());

    for (int l = 0; l <= TileIndex::MaxLevel; ++l)
    {
        currentTile->markerIndices<<markerIndex;

        if (markerIsSelected)
        {
            currentTile->selectedCount++;
        }

        // does the tile have any children?
        if (currentTile->childrenEmpty())
            break;

        // the tile has children. make sure the tile for our marker exists:
        const int nextIndex = tileIndex.linearIndex(l);
        MyTile* nextTile    = static_cast<MyTile*>(currentTile->getChild(nextIndex));

        if (nextTile == 0)
        {
            // we have to create the tile:
            nextTile = static_cast<MyTile*>(tileNew());
            currentTile->addChild(nextIndex, nextTile);
        }

        // if this is the last loop iteration, populate the next tile now:
        if (l == TileIndex::MaxLevel)
        {
            nextTile->markerIndices<<markerIndex;

            if (markerIsSelected)
            {
                nextTile->selectedCount++;
            }
        }

        currentTile = nextTile;
    }
}

void ItemMarkerTiler::prepareTiles(const GeoCoordinates& /*upperLeft*/, const GeoCoordinates&, int /*level*/)
{
}

void ItemMarkerTiler::regenerateTiles()
{
    resetRootTile();
    setDirty(false);

    if (!d->markerModel)
        return;

    // read out all existing markers into tiles:
    for (int row = 0; row < d->markerModel->rowCount(); ++row)
    {
        const QModelIndex modelIndex = d->markerModel->index(row, 0);
        addMarkerIndexToGrid(QPersistentModelIndex(modelIndex));
    }
}

bool ItemMarkerTiler::indicesEqual(const QVariant& a, const QVariant& b) const
{
    return a.value<QPersistentModelIndex>()==b.value<QPersistentModelIndex>();
}

void ItemMarkerTiler::onIndicesClicked(const ClickInfo& clickInfo)
{
    QList<QPersistentModelIndex> clickedMarkers;

    for (int i = 0; i < clickInfo.tileIndicesList.count(); ++i)
    {
        const TileIndex tileIndex = clickInfo.tileIndicesList.at(i);

        clickedMarkers << getTileMarkerIndices(tileIndex);
    }

    const QPersistentModelIndex representativeModelIndex = clickInfo.representativeIndex.value<QPersistentModelIndex>();

    if (clickInfo.currentMouseMode == MouseModeSelectThumbnail && d->selectionModel)
    {
        const bool doSelect = (clickInfo.groupSelectionState & SelectedMask) != SelectedAll;

        const QItemSelectionModel::SelectionFlags selectionFlags = (doSelect ? QItemSelectionModel::Select
                                                                             : QItemSelectionModel::Deselect) |
                                                                   QItemSelectionModel::Rows;

        for (int i = 0; i < clickedMarkers.count(); ++i)
        {
            if (d->selectionModel->isSelected(clickedMarkers.at(i)) != doSelect)
            {
                d->selectionModel->select(clickedMarkers.at(i), selectionFlags);
            }
        }

        if (representativeModelIndex.isValid())
        {
            d->selectionModel->setCurrentIndex(representativeModelIndex, selectionFlags);
        }

        /**
         * @todo When do we report the clicks to the modelHelper?
         *       Or do we only report selection changes to the selection model?
         */
        //d->modelHelper->onIndicesClicked(clickedMarkers);
    }
    else if (clickInfo.currentMouseMode == MouseModeFilter)
    {
        /// @todo Also forward the representative index and the mouse mode in this call
        d->modelHelper->onIndicesClicked(clickedMarkers);
    }
}

void ItemMarkerTiler::onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                     const QPersistentModelIndex& targetSnapIndex)
{
    QList<QPersistentModelIndex> movedMarkers;

    if (tileIndicesList.isEmpty())
    {
        // complicated case: all selected markers were moved
        QModelIndexList selectedIndices = d->selectionModel->selectedIndexes();

        for (int i = 0; i < selectedIndices.count(); ++i)
        {
            // TODO: correctly handle items with multiple columns
            QModelIndex movedMarker = selectedIndices.at(i);

            if (movedMarker.column() == 0)
            {
                movedMarkers << movedMarker;
            }
        }
    }
    else
    {
        // only the tiles in tileIndicesList were moved
        for (int i = 0; i < tileIndicesList.count(); ++i)
        {
            const TileIndex tileIndex = tileIndicesList.at(i);

            movedMarkers << getTileMarkerIndices(tileIndex);
        }
    }

    d->modelHelper->onIndicesMoved(movedMarkers, targetCoordinates, targetSnapIndex);
}

void ItemMarkerTiler::slotSourceModelLayoutChanged()
{
    setDirty();
}

void ItemMarkerTiler::setActive(const bool state)
{
    d->activeState = state;
}

AbstractMarkerTiler::Tile* ItemMarkerTiler::tileNew()
{
    return new MyTile();
}

void ItemMarkerTiler::tileDeleteInternal(AbstractMarkerTiler::Tile* const tile)
{
    delete static_cast<MyTile*>(tile);
}

AbstractMarkerTiler::TilerFlags ItemMarkerTiler::tilerFlags() const
{
    TilerFlags resultFlags = FlagNull;

    if (d->modelHelper->modelFlags().testFlag(GeoModelHelper::FlagMovable))
    {
        resultFlags |= FlagMovable;
    }

    return resultFlags;
}

GeoGroupState ItemMarkerTiler::getGlobalGroupState()
{
    if (d->selectionModel)
    {
        if (d->selectionModel->hasSelection())
        {
            return SelectedMask;
        }
    }

    return SelectedNone;
}

} // namespace Digikam
