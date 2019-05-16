/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : Drag-and-drop handler for geolocation interface integration.
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_MAP_DRAG_DROP_HANDLER_H
#define DIGIKAM_MAP_DRAG_DROP_HANDLER_H

// Qt includes

#include <QAbstractItemModel>
#include <QMimeData>

// Local includes

#include "geodragdrophandler.h"
#include "digikam_export.h"

namespace Digikam
{

class GPSGeoIfaceModelHelper;

class DIGIKAM_EXPORT MapDragData : public QMimeData
{
    Q_OBJECT

public:

    explicit MapDragData()
      : QMimeData(),
        draggedIndices()
    {
    }

    QList<QPersistentModelIndex> draggedIndices;
};

class DIGIKAM_EXPORT MapDragDropHandler : public GeoDragDropHandler
{
    Q_OBJECT

public:

    explicit MapDragDropHandler(QAbstractItemModel* const /*pModel*/,
                                GPSGeoIfaceModelHelper* const parent);
    virtual ~MapDragDropHandler();

    virtual Qt::DropAction accepts(const QDropEvent* e) override;
    virtual bool dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates) override;
    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) override;

private:

    GPSGeoIfaceModelHelper* const gpsGeoIfaceModelHelper;
};

} // namespace Digikam

#endif // DIGIKAM_MAP_DRAG_DROP_HANDLER_H
