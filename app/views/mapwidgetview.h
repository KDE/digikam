/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : central Map view
 *
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MAPWIDGETVIEW_H
#define MAPWIDGETVIEW_H

// Qt includes

#include <QWidget>
#include <QSortFilterProxyModel>

// libkgeomap includes

#include <libkgeomap/kgeomap_primitives.h>
#include <libkgeomap/modelhelper.h>

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"

// local includes

#include "statesavingobject.h"
#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"
#include "imagefiltermodel.h"
#include "camiteminfo.h"

namespace Digikam
{

class AlbumWidgetStack;
class ImageChangeset;

class MapWidgetView : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    enum Application
    {
        ApplicationDigikam  = 1,
        ApplicationImportUI = 2
    };

    MapWidgetView(QItemSelectionModel* const selectionModel,
                  KCategorizedSortFilterProxyModel* const imageFilterModel, QWidget* const parent, const Application application);
    ~MapWidgetView();

    void openAlbum(Album* const album);
    void setActive(const bool state);
    bool getActiveState() const;

    ImageInfo   currentImageInfo()   const;
    CamItemInfo currentCamItemInfo() const;

protected:

    void doLoadState();
    void doSaveState();

private:

    class Private;
    const QScopedPointer<Private> d;
};

// ------------------------------------------------------------------------------------------------------------

class MapViewModelHelper : public KGeoMap::ModelHelper
{
    Q_OBJECT

public:

    MapViewModelHelper(QItemSelectionModel* const selection, KCategorizedSortFilterProxyModel* const filterModel,
                       QObject* const parent, const MapWidgetView::Application application);
    virtual ~MapViewModelHelper();

    virtual QAbstractItemModel* model()                                                                const;
    virtual QItemSelectionModel* selectionModel()                                                      const;
    virtual bool itemCoordinates(const QModelIndex& index, KGeoMap::GeoCoordinates* const coordinates) const;

    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

    virtual void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices);

Q_SIGNALS:

    void signalFilteredImages(const QList<qlonglong>& idList);

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotThumbnailLoaded(const CamItemInfo& info);
    void slotImageChange(const ImageChangeset& changeset);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

#endif  // MAPWIDGETVIEW_H
