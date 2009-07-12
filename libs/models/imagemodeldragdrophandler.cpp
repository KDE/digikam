/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-26
 * Description : Qt Model for Images - drag and drop handling
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

#include "imagemodeldragdrophandler.h"

// Qt includes

#include <QMimeData>

// Local includes

#include "imagemodel.h"


namespace Digikam
{

ImageModelDragDropHandler::ImageModelDragDropHandler(ImageModel *model)
    : QObject(model), m_model(model)
{
}

ImageModel *ImageModelDragDropHandler::model() const
{
    return m_model;
}

bool ImageModelDragDropHandler::dropEvent(QAbstractItemView *, const QDropEvent *, const QModelIndex &)
{
    return false;
}

Qt::DropAction ImageModelDragDropHandler::accepts(const QDropEvent *, const QModelIndex &)
{
    return Qt::IgnoreAction;
}

QStringList ImageModelDragDropHandler::mimeTypes() const
{
    return QStringList();
}

QMimeData *ImageModelDragDropHandler::createMimeData(const QList<ImageInfo> &)
{
    return 0;
}

bool ImageModelDragDropHandler::acceptsMimeData(const QMimeData *mime)
{
    QStringList modelTypes = mimeTypes();
    for (int i = 0; i < modelTypes.count(); ++i)
    {
        if (mime->hasFormat(modelTypes.at(i))) //&& (e->dropAction() & model->supportedDropActions()))
            return true;
    }
    return false;
}

}

