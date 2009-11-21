/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Qt Model for Tag - drag and drop handling
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "tagdragdrop.moc"

// Qt includes

#include <QDropEvent>

// KDE includes
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "ddragobjects.h"

namespace Digikam
{

TagDragDropHandler::TagDragDropHandler(TagModel *model)
                    : AlbumModelDragDropHandler(model)
{
}

bool TagDragDropHandler::dropEvent(QAbstractItemView *view, QDropEvent *e, const QModelIndex& droppedOn)
{
    if(accepts(e->mimeData(), droppedOn) == Qt::IgnoreAction)
        return false;

    TAlbum *destAlbum = model()->albumForIndex(droppedOn);

    if(DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;
        if (!DTagDrag::decode(e->mimeData(), tagID))
            return false;

        TAlbum* talbum = AlbumManager::instance()->findTAlbum(tagID);

        if(!talbum)
            return false;

        if (destAlbum && talbum == destAlbum)
            return false;

        KMenu popMenu(view);
        popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));

        QAction *gotoAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction *choice = popMenu.exec(QCursor::pos());

        if(choice == gotoAction)
        {
            TAlbum *newParentTag = 0;

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
                KMessageBox::error(view, errMsg);
            }

            if(view && !view->isVisible())
                view->setVisible(true);
        }

        return true;
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        TAlbum *srcAlbum;

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return false;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return false;

        // all the albumids will be the same
        int albumID = albumIDs.first();
        srcAlbum = AlbumManager::instance()->findTAlbum(albumID);
        if (!srcAlbum)
        {
            kWarning() << "Could not find source album of drag";
            return false;
        }

        if(srcAlbum == destAlbum)
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
                KMenu popMenu(view);
                popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
                QAction *setAction = 0;
                if (imageIDs.count() == 1)
                    setAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
                popMenu.addSeparator();
                popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

                popMenu.setMouseTracking(true);
                QAction *choice = popMenu.exec(QCursor::pos());
                set = (choice == setAction);
            }

            if(set)
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
        if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            assign = true;
        }
        else
        {
            KMenu popMenu(view);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
            QAction * assignAction = popMenu.addAction(SmallIcon("tag"),
                                             i18n("Assign Tag '%1' to Items", destAlbum->prettyUrl()));
            popMenu.addSeparator();
            popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            assign = (choice == assignAction);
        }

        if (assign)
        {
            emit assignTags(destAlbum->id(), imageIDs);
        }
    }
}

Qt::DropAction TagDragDropHandler::accepts(const QMimeData *data, const QModelIndex& dropIndex)
{

    TAlbum *destAlbum = model()->albumForIndex(dropIndex);

    int droppedId = 0;
    if (!DTagDrag::decode(data, droppedId))
        return Qt::IgnoreAction;

    TAlbum *droppedAlbum = AlbumManager::instance()->findTAlbum(droppedId);

    // TODO update, list supporting...
    if(DTagDrag::canDecode(data)/* || DTagListDrag::canDecode(data) */)
    {

        // Allow dragging on empty space when the itemDrag isn't already at root level
        if (!droppedAlbum && destAlbum->parent()->isRoot())
            return Qt::IgnoreAction;

        // Allow dragging at the root, to move the tag to the root
        if(!destAlbum)
            return Qt::MoveAction;

        // Dragging an item on itself makes no sense
        if(destAlbum == droppedAlbum)
            return Qt::IgnoreAction;

        // Dragging a parent on its child makes no sense
        if(destAlbum && droppedAlbum && destAlbum->isAncestorOf(droppedAlbum))
            return Qt::IgnoreAction;

        return Qt::MoveAction;
    }

    if (DItemDrag::canDecode(data) && destAlbum && destAlbum->parent())
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
    mimeTypes << DTagDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DItemDrag::mimeTypes();
    return mimeTypes;
}

QMimeData *TagDragDropHandler::createMimeData(const QList<Album*>& albums)
{

    if (albums.isEmpty())
    {
        kError() << "Cannot drag no tag";
        return 0;
    }

    if (albums.size() == 1)
    {
        return new DTagDrag(albums.first()->id());
    }
    else
    {
        // TODO update, supporting this sounds harder ;)
//        QList<int> ids;
//        foreach(Album *album, albums)
//        {
//            ids << album->id();
//        }
//        return new DTagListDrag(id);
    }
}

} // namespace Digikam
