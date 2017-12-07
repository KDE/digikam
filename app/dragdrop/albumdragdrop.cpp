/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "albumdragdrop.h"

// Qt includes

#include <QDropEvent>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "importui.h"
#include "ddragobjects.h"
#include "dio.h"
#include "imageinfo.h"
#include "imageinfolist.h"

namespace Digikam
{

AlbumDragDropHandler::AlbumDragDropHandler(AlbumModel* const model)
    : AlbumModelDragDropHandler(model)
{
}

AlbumModel* AlbumDragDropHandler::model() const
{
    return static_cast<AlbumModel*>(m_model);
}

bool AlbumDragDropHandler::dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn)
{
    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    AlbumPointer<PAlbum> destAlbum = model()->albumForIndex(droppedOn);

    if (!destAlbum)
    {
        return false;
    }

    if (DAlbumDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        int         albumId = 0;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return false;
        }

        AlbumPointer<PAlbum> droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!droppedAlbum)
        {
            return false;
        }

        // TODO Copy?
        QMenu popMenu(view);
        QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice = popMenu.exec(QCursor::pos());

        if (!droppedAlbum || !destAlbum)
        {
            return false;
        }

        if (choice == moveAction)
        {
            DIO::move(droppedAlbum, destAlbum);
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

        // Check if items dropped come from outside current album.
        // This can be the case with recursive content album mode.
        ImageInfoList extImgInfList;

        for (QList<qlonglong>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (info.albumId() != destAlbum->id())
            {
                extImgInfList << info;
            }
        }

        if (extImgInfList.isEmpty())
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
                    setAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
                }

                popMenu.addSeparator();
                popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
                popMenu.setMouseTracking(true);
                QAction* const choice = popMenu.exec(QCursor::pos());
                set                   = (setAction == choice);
            }

            if (set && destAlbum)
            {
                QString errMsg;
                AlbumManager::instance()->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
            }

            return true;
        }

        // If shift key is pressed while dragging, move the drag object without
        // displaying popup menu -> move
        bool move         = false;
        bool copy         = false;
        bool setThumbnail = false;

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
            QMenu popMenu(view);
            QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
            QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
            QAction* thumbnailAction  = 0;

            if (imageIDs.count() == 1)
            {
                thumbnailAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
            }

            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* const choice = popMenu.exec(QCursor::pos());

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

        if (!destAlbum)
        {
            return false;
        }

        if (move)
        {
            DIO::move(extImgInfList, destAlbum);
        }
        else if (copy)
        {
            DIO::copy(extImgInfList, destAlbum);
        }
        else if (setThumbnail)
        {
            QString errMsg;
            AlbumManager::instance()->updatePAlbumIcon(destAlbum, extImgInfList.first().id(), errMsg);
        }

        return true;
    }
    // -- DnD from Camera GUI ----------------------------
    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        ImportUI* const ui = dynamic_cast<ImportUI*>(e->source());

        if (ui)
        {
            QMenu popMenu(view);
            QAction* const downAction    = popMenu.addAction(QIcon::fromTheme(QLatin1String("file-export")), i18n("Download From Camera"));
            QAction* const downDelAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("file-export")), i18n("Download && Delete From Camera"));
            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* const choice = popMenu.exec(QCursor::pos());

            if (choice && destAlbum)
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
    else if (e->mimeData()->hasUrls())
    {
        QList<QUrl> srcURLs = e->mimeData()->urls();
        bool move           = false;
        bool copy           = false;

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
            QMenu popMenu(view);
            QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
            QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* const choice     = popMenu.exec(QCursor::pos());

            if (choice == copyAction)
            {
                copy = true;
            }
            else if (choice == moveAction)
            {
                move = true;
            }
        }

        if (!destAlbum)
        {
            return false;
        }

        if (move)
        {
            DIO::move(srcURLs, destAlbum);
        }
        else if (copy)
        {
            DIO::copy(srcURLs, destAlbum);
        }

        return true;
    }

    return false;
}

Qt::DropAction AlbumDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& dropIndex)
{
    PAlbum* const destAlbum = model()->albumForIndex(dropIndex);

    if (!destAlbum)
    {
        return Qt::IgnoreAction;
    }

    // Dropping on root is not allowed and
    // Dropping on trash is not implemented yet
    if (destAlbum->isRoot() || destAlbum->isTrashAlbum())
    {
        return Qt::IgnoreAction;
    }

    if (DAlbumDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        int         albumId = 0;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return Qt::IgnoreAction;
        }

        PAlbum* const droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

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
    else if (DItemDrag::canDecode(e->mimeData())           ||
             DCameraItemListDrag::canDecode(e->mimeData()) ||
             e->mimeData()->hasUrls())
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList AlbumDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DAlbumDrag::mimeTypes()
              << DItemDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << QLatin1String("text/uri-list");

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
        qCWarning(DIGIKAM_GENERAL_LOG) << "Dragging multiple albums is not implemented";
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(albums.first());

    // Root and Trash Albums are not dragable
    if (!palbum || palbum->isRoot() || palbum->isTrashAlbum())
    {
        return 0;
    }

    return (new DAlbumDrag(albums.first()->databaseUrl(), albums.first()->id(), palbum->fileUrl()));
}

} // namespace Digikam
