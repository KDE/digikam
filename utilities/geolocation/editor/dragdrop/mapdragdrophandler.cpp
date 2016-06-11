/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-22
 * @brief  Drag-and-drop handler for GeoIface integration.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mapdragdrophandler.h"

// Qt includes

#include <QDropEvent>

// local includes

#include "gpsgeoifacemodelhelper.h"

namespace Digikam
{

MapDragDropHandler::MapDragDropHandler(QAbstractItemModel* const /*pModel*/, GPSGeoIfaceModelHelper* const parent)
    : DragDropHandler(parent),
      gpsGeoIfaceModelHelper(parent)
{
}

MapDragDropHandler::~MapDragDropHandler()
{
}

Qt::DropAction MapDragDropHandler::accepts(const QDropEvent* /*e*/)
{
    return Qt::CopyAction;
}

bool MapDragDropHandler::dropEvent(const QDropEvent* e, const GeoIface::GeoCoordinates& dropCoordinates)
{
    const MapDragData* const mimeData = qobject_cast<const MapDragData*>(e->mimeData());

    if (!mimeData)
        return false;

    QList<QPersistentModelIndex> droppedIndices;

    for (int i=0; i<mimeData->draggedIndices.count(); ++i)
    {
        // TODO: correctly handle items with multiple columns
        QModelIndex itemIndex = mimeData->draggedIndices.at(i);

        if (itemIndex.column()==0)
        {
            droppedIndices << itemIndex;
        }
    }

    gpsGeoIfaceModelHelper->onIndicesMoved(droppedIndices, dropCoordinates, QPersistentModelIndex());

    return true;
}

QMimeData* MapDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& modelIndices)
{
    Q_UNUSED(modelIndices);
    return 0;
}

} /* namespace Digikam */
