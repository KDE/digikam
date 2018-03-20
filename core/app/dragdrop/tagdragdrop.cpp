/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Qt Model for Tag - drag and drop handling
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "tagdragdrop.h"

// Qt includes

#include <QDropEvent>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "ddragobjects.h"
#include "imageinfo.h"
#include "albumtreeview.h"

namespace Digikam
{

TagDragDropHandler::TagDragDropHandler(TagModel* const model)
    : AlbumModelDragDropHandler(model)
{
}

TagModel* TagDragDropHandler::model() const
{
    return static_cast<TagModel*>(m_model);
}

bool TagDragDropHandler::dropEvent(QAbstractItemView* view, const QDropEvent* e,
                                   const QModelIndex& droppedOn)
{
    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    TAlbum* const destAlbum = model()->albumForIndex(droppedOn);

    if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return false;
        }

        if (tagIDs.isEmpty())
        {
            return false;
        }

        QMenu popMenu(view);
        QAction* const gotoAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice     = popMenu.exec(QCursor::pos());

        for (int index = 0 ; index < tagIDs.count() ; ++index)
        {
            TAlbum* const talbum = AlbumManager::instance()->findTAlbum(tagIDs.at(index));

            if (!talbum)
            {
                return false;
            }

            if (destAlbum && talbum == destAlbum)
            {
                return false;
            }

            if (choice == gotoAction)
            {
                TAlbum* newParentTag = 0;

                if (!destAlbum)
                {
                    // move dragItem to the root
                    newParentTag = AlbumManager::instance()->findTAlbum(0);
                }
                else
                {
                    // move dragItem as child of dropItem
                    newParentTag = destAlbum;
                }

                QString errMsg;

                if (!AlbumManager::instance()->moveTAlbum(talbum, newParentTag, errMsg))
                {
                    QMessageBox::critical(view, qApp->applicationName(), errMsg);
                }

                if (view && !view->isVisible())
                {
                    view->setVisible(true);
                }
            }
        }

        return true;
    }
    else if (DItemDrag::canDecode(e->mimeData()))
    {
        QList<QUrl>      urls;
        QList<QUrl>      kioURLs;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
        {
            return false;
        }

        if (imageIDs.size() == 1 && ImageInfo(imageIDs.first()).tagIds().contains(destAlbum->id()))
        {
            // Setting the dropped image as the album thumbnail
            // If the ctrl key is pressed, when dropping the image, the
            // thumbnail is set without a popup menu
            bool set = false;

            if (e->keyboardModifiers() == Qt::ControlModifier)
            {
                set = true;
            }
            else
            {
                QMenu popMenu(view);
                QAction* setAction = 0;

                if (imageIDs.count() == 1)
                {
                    setAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
                }

                popMenu.addSeparator();
                popMenu.addAction( QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel") );

                popMenu.setMouseTracking(true);
                QAction* const choice = popMenu.exec(QCursor::pos());
                set                   = (choice == setAction);
            }

            if (set)
            {
                QString errMsg;
                AlbumManager::instance()->updateTAlbumIcon(destAlbum, QString(), imageIDs.first(), errMsg);
            }

            return true;
        }

        // If a ctrl key is pressed while dropping the drag object,
        // the tag is assigned to the images without showing a
        // popup menu.
        bool assign = false;

        // Use selected tags instead of dropped on.
        QList<int> tagIdList;
        QStringList tagNames;

        QList<Album*> selTags = ((AbstractAlbumTreeView*)view)->selectedItems();

        for (int it = 0 ; it < selTags.count() ; ++it)
        {
            TAlbum* const temp = dynamic_cast<TAlbum*>(selTags.at(it));

            if (temp)
            {
                tagIdList << temp->id();
                tagNames << temp->title();
            }
        }

        // If nothing selected, use dropped on tag

        if (tagIdList.isEmpty())
        {
            tagIdList << destAlbum->id();
            tagNames << destAlbum->title();
        }

        if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            assign = true;
        }
        else
        {
            QMenu popMenu(view);
            QAction* const assignAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                            i18n("Assign Tag(s) '%1' to Items", tagNames.join(QLatin1String(", "))));
            popMenu.addSeparator();
            popMenu.addAction( QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            QAction* const choice = popMenu.exec(QCursor::pos());
            assign                = (choice == assignAction);
        }

        if (assign)
        {
            emit assignTags(imageIDs, tagIdList);
        }

        return true;
    }

    return false;
}

Qt::DropAction TagDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& dropIndex)
{
    TAlbum* const destAlbum = model()->albumForIndex(dropIndex);

    if (DTagListDrag::canDecode(e->mimeData()))
    {
        //int droppedId = 0;
        QList<int> droppedId;

        if (!DTagListDrag::decode(e->mimeData(), droppedId))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "List decode error" << droppedId.isEmpty();
            return Qt::IgnoreAction;
        }

        TAlbum* const droppedAlbum = AlbumManager::instance()->findTAlbum(droppedId.first());

        if (!droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Allow dragging on empty space to move the dragged album under the root albumForIndex
        // - unless the itemDrag is already at root level

        if (!destAlbum)
        {
            Album* const palbum = droppedAlbum->parent();

            if (!palbum)
            {
                 return Qt::IgnoreAction;
            }

            if (palbum->isRoot())
            {
                return Qt::IgnoreAction;
            }
            else
            {
                return Qt::MoveAction;
            }
        }

        // Dragging an item on itself makes no sense

        if (destAlbum == droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging a parent on its child makes no sense

        if (destAlbum && droppedAlbum && droppedAlbum->isAncestorOf(destAlbum))
        {
            return Qt::IgnoreAction;
        }

        return Qt::MoveAction;
    }
    else if (DItemDrag::canDecode(e->mimeData()) && destAlbum && destAlbum->parent())
    {
        // Only other possibility is image items being dropped
        // And allow this only if there is a Tag to be dropped
        // on and also the Tag is not root.
        return Qt::CopyAction;
    }

    return Qt::IgnoreAction;
}

QStringList TagDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DTagListDrag::mimeTypes()
              << DItemDrag::mimeTypes();

    return mimeTypes;
}

QMimeData* TagDragDropHandler::createMimeData(const QList<Album*>& albums)
{

    if (albums.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot drag no tag";
        return 0;
    }

    QList<int> ids;

    foreach (Album* const album, albums)
    {
        ids << album->id();
    }

    return new DTagListDrag(ids);
}

} // namespace Digikam
