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

// libkmap includes

#include <libkmap/html_widget.h>
#include <libkmap/kmap.h>
#include <libkmap/markermodel.h>

// local includes

#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"


namespace Digikam
{

class MapWidgetViewPriv;
class AlbumWidgetStack;

class MapViewModelHelperPrivate;

class MapViewModelHelper : public KMapIface::WMWModelHelper
{
    Q_OBJECT

public:

    MapViewModelHelper(ImageAlbumModel* const model, QItemSelectionModel* selection  , QObject* const parent = 0);
    virtual ~MapViewModelHelper();

    virtual QAbstractItemModel* model() const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index, KMapIface::WMWGeoCoordinate* const coordinates) const;

    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    MapViewModelHelperPrivate* const d;

};


class MapWidgetView : public QWidget
{

    Q_OBJECT

public: 

    MapWidgetView(ImageAlbumModel* model, QWidget* parent);
    ~MapWidgetView();

    void openAlbum(Album* album);

private:

    MapWidgetViewPriv* const d;    

};


} // namespace Digikam



#endif  // MAPWIDGETVIEW_H
