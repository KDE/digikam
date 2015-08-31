/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#include "imagedragdrop.moc"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmenu.h>

// Local includes

#include "albummanager.h"
#include "importui.h"
#include "importiconview.h"
#include "ddragobjects.h"
#include "dio.h"
#include "imagecategorizedview.h"
#include "imagethumbnailbar.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "tableview_treeview.h"

namespace Digikam
{

enum DropAction
{
    NoAction,
    CopyAction,
    MoveAction,
    GroupAction,
    AssignTagAction
};

static QAction* addGroupAction(KMenu* const menu)
{
    return menu->addAction( SmallIcon("arrow-down-double"),
                            i18nc("@action:inmenu Group images with this image", "Group here"));
}

static QAction* addCancelAction(KMenu* const menu)
{
    return menu->addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );
}

static DropAction copyOrMove(const QDropEvent* const e, QWidget* const view, bool allowMove = true, bool askForGrouping = false)
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

static DropAction tagAction(const QDropEvent* const, QWidget* const view, bool askForGrouping)
{
    KMenu popMenu(view);
    QAction* const tagAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tag to Dropped Items"));
    QAction* groupAction     = 0;

    if (askForGrouping)
    {
        popMenu.addSeparator();
        groupAction = addGroupAction(&popMenu);
    }

    popMenu.addSeparator();
    addCancelAction(&popMenu);

    popMenu.setMouseTracking(true);
    QAction* const choice = popMenu.exec(QCursor::pos());

    if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }
    else if (tagAction && choice == tagAction)
    {
        return AssignTagAction;
    }

    return NoAction;
}

static DropAction groupAction(const QDropEvent* const, QWidget* const view)
{
    KMenu popMenu(view);
    QAction* const groupAction = addGroupAction(&popMenu);
    popMenu.addSeparator();
    addCancelAction(&popMenu);

    QAction* const choice      = popMenu.exec(QCursor::pos());

    if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }

    return NoAction;
}

// -------------------------------------------------------------------------------------

ImageDragDropHandler::ImageDragDropHandler(ImageModel* const model)
    : AbstractItemDragDropHandler(model),
      m_readOnly(false)
{
}

ImageAlbumModel* ImageDragDropHandler::albumModel() const
{
    return qobject_cast<ImageAlbumModel*>(model());
}

void ImageDragDropHandler::setReadOnlyDrop(bool readOnly)
{
    m_readOnly = readOnly;
}

bool ImageDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    Album* album = 0;

    // Note that the drop event does not have to be in an ImageCategorizedView.
    // It can also be a TableViewTreeView.
    ImageCategorizedView* const view = qobject_cast<ImageCategorizedView*>(abstractview);

    if (view)
    {
        album = view->albumAt(e->pos());
    }
    else
    {
        TableViewTreeView* const tableViewTreeView = qobject_cast<TableViewTreeView*>(abstractview);

        if (tableViewTreeView)
        {
            album = tableViewTreeView->albumAt(e->pos());
        }
    }

    // unless we are readonly anyway, we always want an album
    if (!m_readOnly && (!album || album->isRoot()) )
    {
        return false;
    }

    PAlbum* palbum = 0;
    TAlbum* talbum = 0;

    if (album)
    {
        Album* currentAlbum = 0;

        if(albumModel() && !(albumModel()->currentAlbums().isEmpty()))
            currentAlbum = albumModel()->currentAlbums().first();

        if (album->type() == Album::PHYSICAL)
        {
            palbum = static_cast<PAlbum*>(album);
        }
        else if (currentAlbum && currentAlbum->type() == Album::PHYSICAL)
        {
            palbum = static_cast<PAlbum*>(currentAlbum);
        }

        if (album->type() == Album::TAG)
        {
            talbum = static_cast<TAlbum*>(album);
        }
        else if (currentAlbum && currentAlbum->type() == Album::TAG)
        {
            talbum = static_cast<TAlbum*>(currentAlbum);
        }
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        // Drag & drop inside of digiKam
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

        DropAction action = NoAction;

        ImageInfo droppedOnInfo;

        if (droppedOn.isValid())
        {
            ImageThumbnailBar* const thumbBar = qobject_cast<ImageThumbnailBar*>(abstractview);

            if (thumbBar)
            {
                droppedOnInfo = model()->imageInfo(thumbBar->imageFilterModel()->mapToSourceImageModel(droppedOn));
            }
            else
            {
                droppedOnInfo = model()->imageInfo(droppedOn);
            }
        }

        if (m_readOnly)
        {
            emit imageInfosDropped(ImageInfoList(imageIDs));
            return true;
        }
        else if (palbum)
        {
            // Check if items dropped come from outside current album.
            QList<ImageInfo> extImages, intImages;

            if (imageIDs.isEmpty())
            {
                return false;
            }

            for (QList<qlonglong>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
            {
                ImageInfo info(*it);

                if (info.albumId() != album->id())
                {
                    extImages << info;
                }
                else
                {
                    intImages << info;
                }
            }

            bool onlyExternal = (intImages.isEmpty() && !extImages.isEmpty());
            bool onlyInternal = (!intImages.isEmpty() && extImages.isEmpty());
            bool mixed        = (!intImages.isEmpty() && !extImages.isEmpty());

            // Check for drop of image on itself
            if (intImages.size() == 1 && intImages.first() == droppedOnInfo)
            {
                return false;
            }

            if (droppedOn.isValid() && onlyInternal)
            {
                action = groupAction(e, view);
            }
            else
            {
                // Determine action. Show Menu only if there are any album-external items.
                // Ask for grouping if dropped-on is valid (gives LinkAction)
                action = copyOrMove(e, view, mixed || onlyExternal, !droppedOnInfo.isNull());
            }

            if (onlyExternal)
            {
                // Only external items: copy or move as requested

                if (action == MoveAction)
                {
                    DIO::move(extImages, palbum);
                    return true;
                }
                else if (action == CopyAction)
                {
                    DIO::copy(extImages, palbum);
                    return true;
                }
            }
            else if (onlyInternal)
            {
                // Only items from the current album:
                // Move is a no-op. Do not show menu to ask for copy or move.
                // If the user indicates a copy operation (holding Ctrl), copy.

                if (action == CopyAction)
                {
                    DIO::copy(intImages, palbum);
                    return true;
                }
                else if (action == MoveAction)
                {
                    return false;
                }
            }
            else if (mixed)
            {
                // Mixed items.
                // For move operations, ignore items from current album.
                // If user requests copy, copy.

                if (action == MoveAction)
                {
                    DIO::move(extImages, palbum);
                    return true;
                }
                else if (action == CopyAction)
                {
                    DIO::copy(extImages+intImages, palbum);
                    return true;
                }
            }
        }
        else if (talbum)
        {
            action = tagAction(e, view, droppedOn.isValid());

            if (action == AssignTagAction)
            {
                emit assignTags(ImageInfoList(imageIDs), QList<int>() << talbum->id());
                return true;
            }
        }
        else
        {
            if (droppedOn.isValid())
            {
                // Ask if the user wants to group
                action = groupAction(e, view);
            }
        }

        if (action == GroupAction)
        {
            if (droppedOnInfo.isNull())
            {
                return false;
            }

            emit addToGroup(droppedOnInfo, ImageInfoList(imageIDs));
            return true;
        }

        return false;
    }
    else if (KUrl::List::canDecode(e->mimeData()))
    {
        if (!palbum && !m_readOnly)
        {
            return false;
        }

        // Drag & drop outside of digiKam

        KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());

        if (m_readOnly)
        {
            emit urlsDropped(srcURLs);
            return true;
        }

        DropAction action = copyOrMove(e, view);

        if (action == MoveAction)
        {
            DIO::move(srcURLs, palbum);
        }
        else if (action == CopyAction)
        {
            DIO::copy(srcURLs, palbum);
        }

        return true;
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;
        bool isDecoded = DTagListDrag::decode(e->mimeData(), tagIDs);

        if(!isDecoded)
        {
            kDebug() << "Error: Deconding failed!";
            return false;
        }

        KMenu popMenu(view);

        QList<ImageInfo> selectedInfos  = view->selectedImageInfosCurrentFirst();
        QAction* assignToSelectedAction = 0;

        if (selectedInfos.count() > 1)
        {
            assignToSelectedAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &Selected Items"));
        }

        QAction* assignToThisAction = 0;

        if (droppedOn.isValid())
        {
            assignToThisAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &This Item"));
        }

        QAction* const assignToAllAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &All Items"));

        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("&Cancel"));

        popMenu.setMouseTracking(true);
        QAction* const choice = popMenu.exec(view->mapToGlobal(e->pos()));

        if (choice)
        {
            if (choice == assignToSelectedAction)    // Selected Items
            {
                emit assignTags(selectedInfos, tagIDs);
            }
            else if (choice == assignToAllAction)    // All Items
            {
                emit assignTags(view->imageInfos(), tagIDs);
            }
            else if (choice == assignToThisAction)    // Dropped item only.
            {
                emit assignTags(QList<ImageInfo>() << model()->imageInfo(droppedOn), tagIDs);
            }
        }

        return true;
    }
    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        if (!palbum)
        {
            return false;
        }

        ImportIconView* const iconView = dynamic_cast<ImportIconView*>(e->source());

        if (!iconView)
        {
            return false;
        }

        KMenu popMenu(view);
        popMenu.addTitle(SmallIcon("digikam"), i18n("Importing"));
        QAction* const downAction    = popMenu.addAction(SmallIcon("get-hot-new-stuff"),
                                                         i18n("Download From Camera"));
        QAction* const downDelAction = popMenu.addAction(SmallIcon("get-hot-new-stuff"),
                                                         i18n("Download && Delete From Camera"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice        = popMenu.exec(view->mapToGlobal(e->pos()));

        if (choice)
        {
            if (choice == downAction)
            {
                ImportUI::instance()->slotDownload(true, false, palbum);
            }
            else if (choice == downDelAction)
            {
                ImportUI::instance()->slotDownload(true, true, palbum);
            }
        }

        return true;
    }

    return false;
}

Qt::DropAction ImageDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (albumModel() && albumModel()->currentAlbums().isEmpty())
    {
        return Qt::IgnoreAction;
    }

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

    if (DTagListDrag::canDecode(e->mimeData())        ||
        DCameraItemListDrag::canDecode(e->mimeData()) ||
        DCameraDragObject::canDecode(e->mimeData()))
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList ImageDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DItemDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << DCameraDragObject::mimeTypes()
              << KUrl::List::mimeDataTypes();

    return mimeTypes;
}

QMimeData* ImageDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ImageInfo> infos = model()->imageInfos(indexes);

    KUrl::List       urls;
    KUrl::List       kioURLs;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;

    foreach(const ImageInfo& info, infos)
    {
        urls.append(info.fileUrl());
        kioURLs.append(info.databaseUrl());
        albumIDs.append(info.albumId());
        imageIDs.append(info.id());
    }

    if (urls.isEmpty())
    {
        return 0;
    }

    return new DItemDrag(urls, kioURLs, albumIDs, imageIDs);
}

} // namespace Digikam
