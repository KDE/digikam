/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for geolocation interface used in the demo
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "mydragdrophandler.h"

// Qt includes

#include <QDropEvent>
#include <QDebug>

// Local includes

#include "mytreewidget.h"

MyDragDropHandler::MyDragDropHandler(QAbstractItemModel* const pModel, QObject* const parent)
    : GeoDragDropHandler(parent),
      model(pModel)
{
}

MyDragDropHandler::~MyDragDropHandler()
{
}

Qt::DropAction MyDragDropHandler::accepts(const QDropEvent* /*e*/)
{
    return Qt::CopyAction;
}

bool MyDragDropHandler::dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates)
{
    const MyDragData* const mimeData = qobject_cast<const MyDragData*>(e->mimeData());

    if (!mimeData)
        return false;

    qDebug() << mimeData->draggedIndices.count();

    for (int i = 0 ; i < mimeData->draggedIndices.count() ; ++i)
    {
        const QPersistentModelIndex itemIndex = mimeData->draggedIndices.at(i);

        if (!itemIndex.isValid())
            continue;

        model->setData(itemIndex, QVariant::fromValue(dropCoordinates), RoleCoordinates);
    }

    // TODO: tell the main window about this so it can start an altitude lookup

    return true;
}

QMimeData* MyDragDropHandler::createMimeData(const QList<QPersistentModelIndex>& /*modelIndices*/)
{
    return 0;
}
