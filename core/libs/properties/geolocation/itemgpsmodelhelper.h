/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Model for central Map view
 *
 * Copyright (C) 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_ITEM_GPS_MODEL_HELPER_H
#define DIGIKAM_ITEM_GPS_MODEL_HELPER_H

// Qt includes

#include <QObject>
#include <QStandardItemModel>
#include <QPixmap>

// Local includes

#include "geomodelhelper.h"
#include "thumbnailloadthread.h"
#include "gpsiteminfosorter.h"

namespace Digikam
{

const int RoleGPSItemInfo = Qt::UserRole + 1;

class ItemGPSModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit ItemGPSModelHelper(QStandardItemModel* const itemModel, QObject* const parent = 0);
    virtual ~ItemGPSModelHelper();

    virtual QAbstractItemModel* model()           const;
    virtual QItemSelectionModel* selectionModel() const;

    virtual bool itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const;

    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index, const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list, const int sortKey);

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_GPS_MODEL_HELPER_H
