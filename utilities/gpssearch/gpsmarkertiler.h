/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : GPS search marker tiler
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010, 2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPSMARKERTILER_H
#define GPSMARKERTILER_H

// Qt includes

#include <QByteArray>
#include <QMetaType>
#include <QItemSelectionModel>

// KDE includes

#include <kio/global.h>
#include <kio/job.h>

// libkgeomap includes

#include <libkgeomap/abstractmarkertiler.h>
#include <libkgeomap/kgeomap_widget.h>

// local includes

#include "digikam_export.h"
#include "imageposition.h"
#include "databasechangesets.h"
#include "imagelister.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "databasewatch.h"
#include "databasefields.h"
#include "imagealbummodel.h"
#include "imagefiltermodel.h"

namespace Digikam
{

class GPSImageInfo;

class GPSMarkerTiler : public KGeoMap::AbstractMarkerTiler
{
    Q_OBJECT

public:

    class MyTile;

    explicit GPSMarkerTiler(QObject* const parent, ImageFilterModel* const imageFilterModel, QItemSelectionModel* const selectionModel);
    virtual ~GPSMarkerTiler();

    virtual Tile* tileNew();
    virtual void tileDelete(Tile* const tile);
    virtual void prepareTiles(const KGeoMap::GeoCoordinates& upperLeft, const KGeoMap::GeoCoordinates& lowerRight, int level);
    virtual void regenerateTiles();
    virtual KGeoMap::AbstractMarkerTiler::Tile* getTile(const KGeoMap::TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const KGeoMap::TileIndex& tileIndex);
    virtual int getTileSelectedCount(const KGeoMap::TileIndex& tileIndex);

    virtual QVariant getTileRepresentativeMarker(const KGeoMap::TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual KGeoMap::KGeoMapGroupState getTileGroupState(const KGeoMap::TileIndex& tileIndex);
    virtual KGeoMap::KGeoMapGroupState getGlobalGroupState();

    virtual void onIndicesClicked(const ClickInfo& clickInfo);

    virtual void setActive(const bool state);

    void setRegionSelection(const KGeoMap::GeoCoordinates::Pair& sel);
    void removeCurrentRegionSelection();
    void setPositiveFilterIsActive(const bool state);

Q_SIGNALS:

    void signalModelFilteredImages(const QList<qlonglong>& imagesId);

public Q_SLOTS:

    void slotNewModelData(const QList<ImageInfo>& infoList);

private Q_SLOTS:

    /// @todo Do we monitor all signals of the source models?
    void slotMapImagesJobResult(KJob* job);
    void slotMapImagesJobData(KIO::Job* job, const QByteArray& data);
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotImageChange(const ImageChangeset& changeset);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:

    QList<qlonglong> getTileMarkerIds(const KGeoMap::TileIndex& tileIndex);
    KGeoMap::KGeoMapGroupState getImageState(const qlonglong imageId);
    void removeMarkerFromTileAndChildren(const qlonglong imageId, const KGeoMap::TileIndex& markerTileIndex, MyTile* const startTile, const int startTileLevel, MyTile* const parentTile);
    void addMarkerToTileAndChildren(const qlonglong imageId, const KGeoMap::TileIndex& markerTileIndex, MyTile* const startTile, const int startTileLevel);

    class GPSMarkerTilerPrivate;
    GPSMarkerTilerPrivate* const d;
};

} // namespace Digikam

#endif //GPSMARKERTILER_H
