/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-18
 * @brief  Drag-and-drop handler for GeoIface
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#ifndef DRAGDROPHANDLER_H
#define DRAGDROPHANDLER_H

// local includes

#include "geocoordinates.h"
#include "geoiface_types.h"
#include "digikam_export.h"

class QDropEvent;

namespace GeoIface
{

class DIGIKAM_EXPORT DragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit DragDropHandler(QObject* const parent = 0);
    virtual ~DragDropHandler();

    virtual Qt::DropAction accepts(const QDropEvent* e) = 0;
    virtual bool dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates) = 0;
    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

} /* namespace GeoIface */

#endif /* DRAGDROPHANDLER_H */
