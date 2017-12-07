/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-25
 * Description : A class to hold undo/redo commands.
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

#include "gpsundocommand.h"

// Local includes

#include "gpsimagemodel.h"

namespace Digikam
{

GPSUndoCommand::GPSUndoCommand(QUndoCommand* const parent)
    : QUndoCommand(parent)
{
}

void GPSUndoCommand::changeItemData(const bool redoIt)
{
    if (undoList.isEmpty())
        return;

    // get a pointer to the GPSImageModel:
    // TODO: why is the model returned as const?
    GPSImageModel* const imageModel = const_cast<GPSImageModel*>(dynamic_cast<const GPSImageModel*>(undoList.first().modelIndex.model()));

    if (!imageModel)
        return;

    for (int i=0; i<undoList.count(); ++i)
    {
        const UndoInfo& info      = undoList.at(i);
        GPSImageItem* const item = imageModel->itemFromIndex(info.modelIndex);

        // TODO: correctly handle the dirty flags
        // TODO: find a way to regenerate tag tree
        GPSDataContainer newData  = redoIt ? info.dataAfter : info.dataBefore;
        item->restoreGPSData(newData);
        QList<QList<TagData> > newRGTagList = redoIt ? info.newTagList : info.oldTagList;
        item->restoreRGTagList(newRGTagList);
    }
}

void GPSUndoCommand::redo()
{
    changeItemData(true);
}

void GPSUndoCommand::undo()
{
    changeItemData(false);
}

void GPSUndoCommand::addUndoInfo(const UndoInfo& info)
{
    undoList << info;
}

void GPSUndoCommand::UndoInfo::readOldDataFromItem(const GPSImageItem* const imageItem)
{
    this->dataBefore = imageItem->gpsData();
    this->oldTagList = imageItem->getTagList();
}

void GPSUndoCommand::UndoInfo::readNewDataFromItem(const GPSImageItem* const imageItem)
{
    this->dataAfter = imageItem->gpsData();
    this->newTagList = imageItem->getTagList();
}

} // namespace Digikam
