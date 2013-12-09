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

#include "tagdragdrop.moc"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>

// Local includes

#include "albummanager.h"
#include "ddragobjects.h"
#include "imageinfo.h"
#include "albumtreeview.h"
#include "metadatasynchronizer.h"
#include "albumdb.h"

namespace Digikam
{

TagDragDropHandler::TagDragDropHandler(TagModel* const model)
    : AlbumModelDragDropHandler(model)
{
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

        if(tagIDs.isEmpty())
            return false;

        KMenu popMenu(view);
        popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));

        QAction* const gotoAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice     = popMenu.exec(QCursor::pos());

        // First, collect the items with these tag
        // set up MetadataSynchronizer to write tags back to files,
        // which is started after tag are edited
        QList<qlonglong> itemIds;
        for(int index=0;index<tagIDs.count();++index)
        {
            itemIds += DatabaseAccess().db()->getItemIDsInTag(tagIDs.at(index));
        }
        MetadataSynchronizer* const tool =
            new MetadataSynchronizer(itemIds,
                                     MetadataSynchronizer::WriteFromDatabaseToFile);

        for(int index=0;index<tagIDs.count();++index)
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
                    KMessageBox::error(view, errMsg);
                }

                if (view && !view->isVisible())
                {
                    view->setVisible(true);
                }
            }
        }
        tool->start();
        return true;
    }
    else if (DItemDrag::canDecode(e->mimeData()))
    {
        KUrl::List       urls;
        KUrl::List       kioURLs;
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

        // First, collect the items with these tag
        // set up MetadataSynchronizer to write tags back to files,
        // which is started after tag are edited
        MetadataSynchronizer* const tool =
            new MetadataSynchronizer(imageIDs,
                                     MetadataSynchronizer::WriteFromDatabaseToFile);


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
                KMenu popMenu(view);
                popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
                QAction* setAction = 0;

                if (imageIDs.count() == 1)
                {
                    setAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
                }

                popMenu.addSeparator();
                popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

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

        for(int it = 0 ; it < selTags.count(); ++it)
        {
            TAlbum* const temp = dynamic_cast<TAlbum*>(selTags.at(it));

            if (temp)
            {
                tagIdList << temp->id();
                tagNames << temp->title();
            }
        }

        if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            assign = true;
        }
        else
        {
            KMenu popMenu(view);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Tags"));
            QAction* const assignAction = popMenu.addAction(SmallIcon("tag"),
                                                            i18n("Assign Tag(s) '%1' to Items", tagNames.join(", ")));
            popMenu.addSeparator();
            popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            QAction* const choice = popMenu.exec(QCursor::pos());
            assign                = (choice == assignAction);
        }

        if (assign)
        {
            emit assignTags(imageIDs, tagIdList);
        }

        tool->start();
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
            kDebug() << "List decode error" << droppedId.isEmpty();
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
            if (droppedAlbum->parent()->isRoot())
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
        kError() << "Cannot drag no tag";
        return 0;
    }

        QList<int> ids;

        foreach(Album* const album, albums)
        {
            ids << album->id();
        }

        return new DTagListDrag(ids);
}

} // namespace Digikam
