/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-22
 * @brief  Drag-and-drop handler for KGeoMap integration.
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

#ifndef MAPDRAGDROPHANDLER_H
#define MAPDRAGDROPHANDLER_H

// Qt includes

#include <QAbstractItemModel>
#include <QMimeData>

// Libkgeomap includes

#include <KGeoMap/DragDropHandler>

namespace Digikam
{

class GPSSyncKGeoMapModelHelper;

class MapDragData : public QMimeData
{
    Q_OBJECT

public:

    MapDragData()
      : QMimeData(),
        draggedIndices()
    {
    }

    QList<QPersistentModelIndex> draggedIndices;
};

class MapDragDropHandler : public KGeoMap::DragDropHandler
{
    Q_OBJECT

public:

    MapDragDropHandler(QAbstractItemModel* const /*pModel*/, GPSSyncKGeoMapModelHelper* const parent);
    virtual ~MapDragDropHandler();

    virtual Qt::DropAction accepts(const QDropEvent* e);
    virtual bool dropEvent(const QDropEvent* e, const KGeoMap::GeoCoordinates& dropCoordinates);
    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices);

private:

    GPSSyncKGeoMapModelHelper* const gpsSyncKGeoMapModelHelper;
};

} /* Digikam */

#endif /* MAPDRAGDROPHANDLER_H */
