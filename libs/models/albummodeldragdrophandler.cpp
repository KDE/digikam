/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albummodeldragdrophandler.h"

// Local includes

#include "abstractalbummodel.h"

namespace Digikam
{

AlbumModelDragDropHandler::AlbumModelDragDropHandler(AbstractAlbumModel *model)
    : QObject(model), m_model(model)
{
}

AbstractAlbumModel *AlbumModelDragDropHandler::model() const
{
    return m_model;
}

bool AlbumModelDragDropHandler::dropEvent(QAbstractItemView *, QDropEvent *, const QModelIndex &)
{
    return false;
}

Qt::DropAction AlbumModelDragDropHandler::accepts(const QMimeData *, const QModelIndex &)
{
    return Qt::IgnoreAction;
}

QStringList AlbumModelDragDropHandler::mimeTypes() const
{
    return QStringList();
}

QMimeData *AlbumModelDragDropHandler::createMimeData(const QList<Album*> &)
{
    return 0;
}


}

