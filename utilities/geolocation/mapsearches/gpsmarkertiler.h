/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : GPS search marker tiler
 *
 * Copyright (C) 2010      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef GPS_MARKER_TILER_H
#define GPS_MARKER_TILER_H

// Qt includes

#include <QByteArray>
#include <QMetaType>
#include <QItemSelectionModel>

// Local includes

#include "abstractmarkertiler.h"
#include "mapwidget.h"

// Local includes

#include "digikam_export.h"
#include "imageposition.h"
#include "coredbchangesets.h"
#include "imagelister.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"
#include "coredbwatch.h"
#include "coredbfields.h"
#include "imagealbummodel.h"
#include "imagefiltermodel.h"

namespace Digikam
{

class GPSImageInfo;

class GPSMarkerTiler : public AbstractMarkerTiler
{
    Q_OBJECT

public:

    class MyTile;

    explicit GPSMarkerTiler(QObject* const parent,
                            ImageFilterModel* const imageFilterModel,
                            QItemSelectionModel* const selectionModel);
    virtual ~GPSMarkerTiler();

    virtual Tile* tileNew();
    virtual void tileDelete(Tile* const tile);
    virtual void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level);
    virtual void regenerateTiles();
    virtual AbstractMarkerTiler::Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const TileIndex& tileIndex);
    virtual int getTileSelectedCount(const TileIndex& tileIndex);

    virtual QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual GeoGroupState getTileGroupState(const TileIndex& tileIndex);
    virtual GeoGroupState getGlobalGroupState();

    virtual void onIndicesClicked(const ClickInfo& clickInfo);

    virtual void setActive(const bool state);

    void setRegionSelection(const GeoCoordinates::Pair& sel);
    void removeCurrentRegionSelection();
    void setPositiveFilterIsActive(const bool state);

Q_SIGNALS:

    void signalModelFilteredImages(const QList<qlonglong>& imagesId);

public Q_SLOTS:

    void slotNewModelData(const QList<ImageInfo>& infoList);

private Q_SLOTS:

    /// @todo Do we monitor all signals of the source models?
    void slotMapImagesJobResult();
    void slotMapImagesJobData(const QList<ImageListerRecord>& records);
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotImageChange(const ImageChangeset& changeset);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:

    QList<qlonglong> getTileMarkerIds(const TileIndex& tileIndex);
    GeoGroupState getImageState(const qlonglong imageId);
    void removeMarkerFromTileAndChildren(const qlonglong imageId, const TileIndex& markerTileIndex, MyTile* const startTile, const int startTileLevel, MyTile* const parentTile);
    void addMarkerToTileAndChildren(const qlonglong imageId, const TileIndex& markerTileIndex, MyTile* const startTile, const int startTileLevel);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GPS_MARKER_TILER_H
