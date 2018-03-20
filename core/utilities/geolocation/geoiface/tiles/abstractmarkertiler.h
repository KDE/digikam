/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : An abstract base class for tiling of markers
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef ABSTRACT_MARKER_TILER_H
#define ABSTRACT_MARKER_TILER_H

// Qt includes

#include <QBitArray>
#include <QObject>
#include <QPoint>

// Local includes

#include "tileindex.h"
#include "geoifacetypes.h"
#include "digikam_export.h"
#include "geogroupstate.h"

namespace Digikam
{

class DIGIKAM_EXPORT AbstractMarkerTiler : public QObject
{
    Q_OBJECT

public:

    enum TilerFlag
    {
        FlagNull    = 0,
        FlagMovable = 1
    };

    Q_DECLARE_FLAGS(TilerFlags, TilerFlag)

public:

    class ClickInfo
    {
    public:

        TileIndex::List   tileIndicesList;
        QVariant          representativeIndex;
        GeoGroupState        groupSelectionState;
        GeoMouseModes        currentMouseMode;
    };

public:

    class Tile
    {
    public:

        Tile();

        /**
         * NOTE: Tile is only deleted by AbstractMarkerTiler::tileDelete.
         * All subclasses of AbstractMarkerTiler have to reimplement tileDelete
         * to delete their Tile subclasses.
         * This was done in order not to have any virtual functions
         * in Tile and its subclasses in order to save memory, since there
         * can be a lot of tiles in a MarkerTiler.
         */
        ~Tile();

        Tile* getChild(const int linearIndex);

        void addChild(const int linearIndex, Tile* const tilePointer);

        /**
         * @brief Sets the pointer to a child tile to zero, but you have to delete the tile by yourself!
         */
        void clearChild(const int linearIndex);

        int indexOfChildTile(Tile* const tile);

        bool childrenEmpty() const;

        /**
         * @brief Take away the list of children, only to be used for deleting them.
         *
         * @todo Make this function protected.
         *
         */
        QVector<Tile*> takeChildren();

        static int maxChildCount();

    private:

        void prepareForChildren();

    private:

        QVector<Tile*> children;

    };

public:

    class NonEmptyIterator
    {
    public:

        NonEmptyIterator(AbstractMarkerTiler* const model, const int level);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const TileIndex& startIndex, const TileIndex& endIndex);
        NonEmptyIterator(AbstractMarkerTiler* const model, const int level, const GeoCoordinates::PairList& normalizedMapBounds);
        ~NonEmptyIterator();

        bool                 atEnd()        const;
        TileIndex            nextIndex();
        TileIndex            currentIndex() const;
        AbstractMarkerTiler* model()        const;

    private:

        bool initializeNextBounds();

    private:

        class Private;
        Private* const d;
    };

public:

    explicit AbstractMarkerTiler(QObject* const parent = 0);
    virtual ~AbstractMarkerTiler();

    void tileDeleteChildren(Tile* const tile);
    void tileDelete(Tile* const tile);
    void tileDeleteChild(Tile* const parentTile, Tile* const childTile, const int knownLinearIndex = -1);

    // these have to be implemented
    virtual TilerFlags tilerFlags() const;
    virtual Tile* tileNew();
    virtual void tileDeleteInternal(Tile* const tile);
    virtual void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level) = 0;
    virtual void regenerateTiles() = 0;
    virtual Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false) = 0;
    virtual int getTileMarkerCount(const TileIndex& tileIndex) = 0;
    virtual int getTileSelectedCount(const TileIndex& tileIndex) = 0;

    // these should be implemented for thumbnail handling
    virtual QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey) = 0;
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey) = 0;
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size) = 0;
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const = 0;
    virtual GeoGroupState getTileGroupState(const TileIndex& tileIndex) = 0;
    virtual GeoGroupState getGlobalGroupState() = 0;

    // these can be implemented if you want to react to actions in geolocation interface
    virtual void onIndicesClicked(const ClickInfo& clickInfo);
    virtual void onIndicesMoved(const TileIndex::List& tileIndicesList, const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex);

    virtual void setActive(const bool state) = 0;
    Tile* rootTile();
    bool indicesEqual(const QIntList& a, const QIntList& b, const int upToLevel) const;
    bool isDirty() const;
    void setDirty(const bool state = true);
    Tile* resetRootTile();

Q_SIGNALS:

    void signalTilesOrSelectionChanged();
    void signalThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);

protected:

    /**
     * @brief Only used to safely delete all tiles in the desctructor
     */
    void clear();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::AbstractMarkerTiler::TilerFlags)

#endif // ABSTRACT_MARKER_TILER_H
