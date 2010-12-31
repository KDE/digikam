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
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

// libkmap includes

#include <libkmap/abstractmarkertiler.h>
#include <libkmap/kmap_widget.h>

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

class GPSMarkerTiler : public KMap::AbstractMarkerTiler
{
    Q_OBJECT

public:

    enum SortOptions
    {
        SortYoungestFirst = 0,
        SortOldestFirst   = 1,
        SortRating        = 2
    };

    class MyTile;

    class GPSImageInfo;

    GPSMarkerTiler(QObject* const parent = 0, ImageFilterModel* const imageFilterModel = 0, QItemSelectionModel* const selectionModel = 0);
    virtual ~GPSMarkerTiler();

    virtual Tile* tileNew();
    virtual void tileDelete(Tile* const tile);
    virtual void prepareTiles(const KMap::GeoCoordinates& upperLeft,const KMap::GeoCoordinates& lowerRight, int level);
    virtual void regenerateTiles();
    virtual KMap::AbstractMarkerTiler::Tile* getTile(const KMap::TileIndex& tileIndex, const bool stopIfEmpty = false);
    virtual int getTileMarkerCount(const KMap::TileIndex& tileIndex);
    virtual int getTileSelectedCount(const KMap::TileIndex& tileIndex);

    virtual QVariant getTileRepresentativeMarker(const KMap::TileIndex& tileIndex, const int sortKey);
    virtual QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey);
    virtual QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size);
    virtual bool indicesEqual(const QVariant& a, const QVariant& b) const;
    virtual KMap::KMapGroupState getTileGroupState(const KMap::TileIndex& tileIndex);
    virtual KMap::KMapGroupState getGlobalGroupState();

    virtual void onIndicesClicked(const KMap::TileIndex::List& tileIndicesList, const KMap::KMapGroupState& groupSelectionState, KMap::MouseMode currentMouseMode);

    virtual void setActive(const bool state);

    GPSImageInfo gpsData(const qlonglong id, const KMap::GeoCoordinates& coordinates, const int rating, const QDateTime& creationDate);

    void setRegionSelection(const KMap::GeoCoordinates::Pair& sel);
    void removeCurrentRegionSelection();
    void setPositiveFilterIsActive(const bool state);

Q_SIGNALS:

    void signalModelFilteredImages(const QList<qlonglong>& imagesId);
    void signalClearImages();

public Q_SLOTS:

    void slotNewModelData(const QList<ImageInfo>& infos);

private Q_SLOTS:

    /// @todo Do we monitor all signals of the source models?
    void slotMapImagesJobResult(KJob* job);
    void slotMapImagesJobData(KIO::Job* job, const QByteArray& data);
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotImageChange(const ImageChangeset& changeset);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:

    QList<qlonglong> getTileMarkerIds(const KMap::TileIndex& tileIndex);
    KMap::KMapGroupState getImageState(const qlonglong imageId);

    class GPSMarkerTilerPrivate;
    GPSMarkerTilerPrivate* const d;
};

} // namespace Digikam

#endif //GPSMARKERTILER_H
