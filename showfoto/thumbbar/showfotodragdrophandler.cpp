/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-01-28
 * Description : drag and drop handling for Showfoto
 *
 * Copyright (C) 2014 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotodragdrophandler.moc"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmimetype.h>

// Local includes

#include "ddragobjects.h"
#include "showfotocategorizedview.h"
#include "showfotoiteminfo.h"

namespace ShowFoto
{

ShowfotoDragDropHandler::ShowfotoDragDropHandler(ShowfotoImageModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

QAction* ShowfotoDragDropHandler::addGroupAction(KMenu* const menu)
{
    return menu->addAction(SmallIcon("arrow-down-double"),
                           i18nc("@action:inmenu Group images with this image", "Group here"));
}

QAction* ShowfotoDragDropHandler::addCancelAction(KMenu* const menu)
{
    return menu->addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
}

bool ShowfotoDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    Q_UNUSED(abstractview);

    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    KUrl::List urls = e->mimeData()->urls();

    emit signalDroppedUrls(urls);

    return true;
}

Qt::DropAction ShowfotoDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (KUrl::List::canDecode(e->mimeData()))
    {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

QStringList ShowfotoDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << KUrl::List::mimeDataTypes();

    return mimeTypes;
}

QMimeData* ShowfotoDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ShowfotoItemInfo> infos = model()->showfotoItemInfos(indexes);
    QMimeData* const mimeData     = new QMimeData();

    KUrl::List       urls;

    foreach(const ShowfotoItemInfo& info, infos)
    {
        kDebug() << info.url.toLocalFile();
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

