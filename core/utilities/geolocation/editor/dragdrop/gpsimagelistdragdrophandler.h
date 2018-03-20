/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : Drag and drop handler for the image list
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GPSIMAGELISTDRAGDROPHANDLER_H
#define GPSIMAGELISTDRAGDROPHANDLER_H

// Qt includes

#include <QTreeView>

// Local includes

#include "mapdragdrophandler.h"

namespace Digikam
{

class ImageListDragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit ImageListDragDropHandler(QObject* const parent = 0);
    virtual ~ImageListDragDropHandler();

    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

// -------------------------------------------------------------------------------------------------

class GPSImageListDragDropHandler : public ImageListDragDropHandler
{
    Q_OBJECT

public:

    explicit GPSImageListDragDropHandler(QObject* const parent = 0);
    ~GPSImageListDragDropHandler();

    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices);
};

} // namespace Digikam

#endif /* GPSIMAGELISTDRAGDROPHANDLER_H */
