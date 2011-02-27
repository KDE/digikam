/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "albumdragdrop.moc"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmenu.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "cameraui.h"
#include "ddragobjects.h"
#include "dio.h"
#include "imageinfo.h"
#include "imageinfolist.h"

namespace Digikam
{

AlbumDragDropHandler::AlbumDragDropHandler(AlbumModel* model)
    : AlbumModelDragDropHandler(model)
{
}

bool AlbumDragDropHandler::dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn)
{
    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    PAlbum* destAlbum = model()->albumForIndex(droppedOn);

    if (!destAlbum)
    {
        return false;
    }

    if (DAlbumDrag::canDecode(e->mimeData()))
    {
        KUrl::List urls;
        int albumId;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return false;
        }

        PAlbum* droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!droppedAlbum)
        {
            return false;
        }

        // TODO Copy?
        KMenu popMenu(view);
        popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
        QAction* moveAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* choice = popMenu.exec(QCursor::pos());

        if (choice == moveAction)
        {
            KIO::Job* job = DIO::move(droppedAlbum, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }

        return true;
    }
    else if (DItemDrag::canDecode(e->mimeData()))
    {

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
        {
            return false;
        }

        // Check if items dropped come from outside current album.
        // This can be the case with recursive content album mode.
        KUrl::List       extUrls;
        ImageInfoList    extImgInfList;
        QList<qlonglong> extImageIDs;

        for (QList<qlonglong>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (info.albumId() != destAlbum->id())
            {
                extUrls.append(info.databaseUrl());
                extImgInfList.append(info);
                extImageIDs << *it;
            }
        }

        if (extUrls.isEmpty())
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
                popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
                QAction* setAction = 0;

                if (imageIDs.count() == 1)
                {
                    setAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
                }

                popMenu.addSeparator();
                popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
                popMenu.setMouseTracking(true);
                QAction* choice = popMenu.exec(QCursor::pos());
                set = (setAction == choice);
            }

            if (set)
            {
                QString errMsg;
                AlbumManager::instance()->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
            }

            return true;
        }

        // If shift key is pressed while dragging, move the drag object without
        // displaying popup menu -> move
        bool move = false, copy = false, setThumbnail = false;

        if (e->keyboardModifiers() == Qt::ShiftModifier)
        {
            move = true;
        }
        // If ctrl key is pressed while dragging, copy the drag object without
        // displaying popup menu -> copy
        else if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            copy = true;
        }
        else
        {
            KMenu popMenu(view);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction* moveAction      = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
            QAction* copyAction      = popMenu.addAction(SmallIcon("edit-copy"), i18n("&Copy Here"));
            QAction* thumbnailAction = 0;

            if (imageIDs.count() == 1)
            {
                thumbnailAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
            }

            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* choice = popMenu.exec(QCursor::pos());

            if (choice)
            {
                if (choice == moveAction)
                {
                    move = true;
                }
                else if (choice == copyAction)
                {
                    copy = true;
                }
                else if (choice == thumbnailAction)
                {
                    setThumbnail = true;
                }
            }
        }

        if (move)
        {
            KIO::Job* job = DIO::move(extUrls, extImageIDs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }
        else if (copy)
        {
            KIO::Job* job = DIO::copy(extUrls, extImageIDs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }
        else if (setThumbnail)
        {
            QString errMsg;
            AlbumManager::instance()->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
        }

        return true;
    }
    // -- DnD from Camera GUI ----------------------------
    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        CameraUI* ui = dynamic_cast<CameraUI*>(e->source());

        if (ui)
        {
            KMenu popMenu(view);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction* downAction    = popMenu.addAction(SmallIcon("file-export"),
                                     i18n("Download From Camera"));
            QAction* downDelAction = popMenu.addAction(SmallIcon("file-export"),
                                     i18n("Download && Delete From Camera"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* choice = popMenu.exec(QCursor::pos());

            if (choice)
            {
                if (choice == downAction)
                {
                    ui->slotDownload(true, false, destAlbum);
                }
                else if (choice == downDelAction)
                {
                    ui->slotDownload(true, true, destAlbum);
                }
            }
        }
    }
    // -- DnD from an external source ---------------------
    else if (KUrl::List::canDecode(e->mimeData()))
    {
        KUrl destURL(destAlbum->databaseUrl());

        KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());

        bool move = false, copy = false;

        // If shift key is pressed while dropping, move the drag object without
        // displaying popup menu -> move
        if (e->keyboardModifiers() == Qt::ShiftModifier)
        {
            move = true;
        }
        // If ctrl key is pressed while dropping, copy the drag object without
        // displaying popup menu -> copy
        else if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            copy = true;
        }
        else
        {
            KMenu popMenu(view);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction* moveAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
            QAction* copyAction = popMenu.addAction(SmallIcon("edit-copy"), i18n("&Copy Here"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* choice = popMenu.exec(QCursor::pos());

            if (choice == copyAction)
            {
                copy = true;
            }
            else if (choice == moveAction)
            {
                move = true;
            }
        }

        if (move)
        {
            KIO::Job* job = DIO::move(srcURLs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }
        else if (copy)
        {
            KIO::Job* job = DIO::copy(srcURLs, destAlbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }

        return true;
    }

    return false;
}

Qt::DropAction AlbumDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& dropIndex)
{
    PAlbum* destAlbum = model()->albumForIndex(dropIndex);

    if (DAlbumDrag::canDecode(e->mimeData()))
    {
        // do not allow to drop on root
        if (dropIndex == model()->rootAlbumIndex())
        {
            return Qt::IgnoreAction;
        }

        if (!destAlbum)
        {
            return Qt::IgnoreAction;
        }

        KUrl::List urls;
        int albumId;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return Qt::IgnoreAction;
        }

        PAlbum* droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging an item on itself makes no sense
        if (droppedAlbum == destAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging a parent on its child makes no sense
        if (droppedAlbum->isAncestorOf(destAlbum))
        {
            return Qt::IgnoreAction;
        }

        return Qt::MoveAction;
    }
    else if (DItemDrag::canDecode(e->mimeData()) ||
             DCameraItemListDrag::canDecode(e->mimeData()) ||
             KUrl::List::canDecode(e->mimeData()))
    {
        // Do not allow drop images on album root
        if (destAlbum && !destAlbum->isRoot())
        {
            return Qt::MoveAction;
        }
    }

    return Qt::IgnoreAction;
}

QStringList AlbumDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << DAlbumDrag::mimeTypes()
              << DItemDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << KUrl::List::mimeDataTypes();
    return mimeTypes;
}

QMimeData* AlbumDragDropHandler::createMimeData(const QList<Album*>& albums)
{
    if (albums.isEmpty())
    {
        return 0;
    }

    if (albums.size() > 1)
    {
        kWarning() << "Dragging multiple albums is not implemented";
    }

    PAlbum* palbum = dynamic_cast<PAlbum*>(albums.first());
    return new DAlbumDrag(albums.first()->databaseUrl(), albums.first()->id(),
                          palbum ? palbum->fileUrl() : KUrl());
}

} // namespace Digikam
