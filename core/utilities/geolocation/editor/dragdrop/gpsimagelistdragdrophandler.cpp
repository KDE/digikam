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

#include "gpsimagelistdragdrophandler.h"

namespace Digikam
{

ImageListDragDropHandler::ImageListDragDropHandler(QObject* const parent)
    : QObject(parent)
{
}

ImageListDragDropHandler::~ImageListDragDropHandler()
{
}

// ------------------------------------------------------------------------------------------------

GPSImageListDragDropHandler::GPSImageListDragDropHandler(QObject* const parent)
    : ImageListDragDropHandler(parent)
{
}

GPSImageListDragDropHandler::~GPSImageListDragDropHandler()
{
}

QMimeData* GPSImageListDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& modelIndices)
{
    MapDragData* const mimeData = new MapDragData();
    mimeData->draggedIndices    = modelIndices;

    return mimeData;
}

} /* namespace Digikam */
