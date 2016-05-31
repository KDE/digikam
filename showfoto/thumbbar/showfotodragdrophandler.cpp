/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-01-28
 * Description : drag and drop handling for Showfoto
 *
 * Copyright (C) 2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfotodragdrophandler.h"

// Qt includes

#include <QDropEvent>
#include <QIcon>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "ddragobjects.h"
#include "showfotocategorizedview.h"
#include "showfotoiteminfo.h"

namespace ShowFoto
{

ShowfotoDragDropHandler::ShowfotoDragDropHandler(ShowfotoImageModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

QAction* ShowfotoDragDropHandler::addGroupAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("go-bottom")),
                           i18nc("@action:inmenu Group images with this image", "Group here"));
}

QAction* ShowfotoDragDropHandler::addCancelAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
}

bool ShowfotoDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    Q_UNUSED(abstractview);

    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    QList<QUrl> urls = e->mimeData()->urls();

    emit signalDroppedUrls(urls);

    return true;
}

Qt::DropAction ShowfotoDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (e->mimeData()->hasUrls())
    {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

QStringList ShowfotoDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* ShowfotoDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ShowfotoItemInfo> infos = model()->showfotoItemInfos(indexes);
    QMimeData* const mimeData     = new QMimeData();
    QList<QUrl> urls;

    foreach(const ShowfotoItemInfo& info, infos)
    {
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << info.url.toLocalFile();
        urls.append(info.url);
    }

    if (urls.isEmpty())
    {
        return 0;
    }

    mimeData->setUrls(urls);

    return mimeData;
}

ShowfotoImageModel* ShowfotoDragDropHandler::model() const
{
    return static_cast<ShowfotoImageModel*>(m_model);
}

} // namespace Digikam

