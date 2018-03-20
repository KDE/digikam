/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for geolocation interface
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GEO_DRAG_DROP_HANDLER_H
#define GEO_DRAG_DROP_HANDLER_H

// Local includes

#include "geocoordinates.h"
#include "geoifacetypes.h"
#include "digikam_export.h"

class QDropEvent;

namespace Digikam
{

class DIGIKAM_EXPORT GeoDragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit GeoDragDropHandler(QObject* const parent = 0);
    virtual ~GeoDragDropHandler();

    virtual Qt::DropAction accepts(const QDropEvent* e) = 0;
    virtual bool dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates) = 0;
    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

} // namespace Digikam

#endif // GEO_DRAG_DROP_HANDLER_H
