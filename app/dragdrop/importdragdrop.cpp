/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-09-07
 * Description : Qt Model for ImportUI - drag and drop handling
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importdragdrop.moc"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>

// Local includes

#include "importiconview.h"
#include "importui.h"
#include "ddragobjects.h"
#include "importcategorizedview.h"
#include "camiteminfo.h"
#include "albummanager.h"
#include "digikamapp.h"
#include "digikamview.h"

namespace Digikam
{

ImportDragDropHandler::ImportDragDropHandler(ImportImageModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

QAction* ImportDragDropHandler::addGroupAction(KMenu* const menu)
{
    return menu->addAction(SmallIcon("arrow-down-double"),
                           i18nc("@action:inmenu Group images with this image", "Group here"));
}

QAction* ImportDragDropHandler::addCancelAction(KMenu* const menu)
{
    return menu->addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
}

ImportDragDropHandler::DropAction ImportDragDropHandler::copyOrMove(const QDropEvent* e, QWidget* const view,
                                                                    bool allowMove, bool askForGrouping)
{
    if (e->keyboardModifiers() & Qt::ControlModifier)
    {
        return CopyAction;
    }
    else if (e->keyboardModifiers() & Qt::ShiftModifier)
    {
        return MoveAction;
    }

    if (!allowMove && !askForGrouping)
    {
        switch (e->proposedAction())
        {
            case Qt::CopyAction:
                return CopyAction;
            case Qt::MoveAction:
                return MoveAction;
            default:
                return NoAction;
        }
    }

    KMenu popMenu(view);

    QAction* moveAction = 0;

    if (allowMove)
    {
        moveAction = popMenu.addAction( SmallIcon("go-jump"), i18n("&Move Here"));
    }

    QAction* const copyAction = popMenu.addAction( SmallIcon("edit-copy"), i18n("&Copy Here"));
    popMenu.addSeparator();

    QAction* groupAction = 0;

    if (askForGrouping)
    {
        groupAction = addGroupAction(&popMenu);
        popMenu.addSeparator();
    }

    addCancelAction(&popMenu);

    popMenu.setMouseTracking(true);
    QAction* const choice = popMenu.exec(QCursor::pos());

    if (moveAction && choice == moveAction)
    {
        return MoveAction;
    }
    else if (choice == copyAction)
    {
        return CopyAction;
    }
    else if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }

    return NoAction;
}

/*
static DropAction tagAction(const QDropEvent*, QWidget* view, bool askForGrouping)
{
}

static DropAction groupAction(const QDropEvent*, QWidget* view)
{
}
*/

bool ImportDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    ImportCategorizedView* const view = static_cast<ImportCategorizedView*>(abstractview);

    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        KUrl::List lst = DigikamApp::instance()->view()->selectedUrls();

        KMenu popMenu(view);
        popMenu.addTitle(SmallIcon("digikam"), i18n("Exporting"));
        QAction* const upAction = popMenu.addAction(SmallIcon("media-flash-smart-media"),
                                                    i18n("Upload to Camera"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice = popMenu.exec(view->mapToGlobal(e->pos()));

        if (choice)
        {
            if (choice == upAction)
            {
                ImportUI::instance()->slotUploadItems(lst);
            }
        }

        return true;
    }
/*
    TODO: Implement tag dropping in import tool.
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
    }
*/
    return false;
}

Qt::DropAction ImportDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (DItemDrag::canDecode(e->mimeData()) || KUrl::List::canDecode(e->mimeData()))
    {
        if (e->keyboardModifiers() & Qt::ControlModifier)
        {
            return Qt::CopyAction;
        }
        else if (e->keyboardModifiers() & Qt::ShiftModifier)
        {
            return Qt::MoveAction;
        }

        return Qt::MoveAction;
    }

    if (DTagListDrag::canDecode(e->mimeData())            ||
        DCameraItemListDrag::canDecode(e->mimeData()) ||
        DCameraDragObject::canDecode(e->mimeData()))
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList ImportDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << DItemDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << DCameraDragObject::mimeTypes()
              << KUrl::List::mimeDataTypes();

    return mimeTypes;
}

QMimeData* ImportDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<CamItemInfo> infos = model()->camItemInfos(indexes);

    QStringList lst;

    foreach(CamItemInfo info, infos)
    {
        lst.append(info.folder + info.name);
    }

    if (lst.isEmpty())
    {
        return 0;
    }

    return (new DCameraItemListDrag(lst));
}

ImportImageModel* ImportDragDropHandler::model() const
{
    return static_cast<ImportImageModel*>(m_model);
}

} // namespace Digikam
