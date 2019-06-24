/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Items - drag and drop handling
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemdragdrop.h"

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
#include "importiconview.h"
#include "itemthumbnailbar.h"
#include "ddragobjects.h"
#include "dio.h"
#include "itemcategorizedview.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "tableview_treeview.h"

namespace Digikam
{

enum DropAction
{
    NoAction,
    CopyAction,
    MoveAction,
    GroupAction,
    SortAction,
    GroupAndMoveAction,
    AssignTagAction
};

static QAction* addGroupAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("go-bottom")),
                           i18nc("@action:inmenu Group images with this item",
                                 "Group here"));
}

static QAction* addSortAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("insert-image")),
                           i18nc("@action:inmenu Insert dragged images before this item",
                                 "Insert Items here"));
}

static QAction* addGroupAndMoveAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("go-bottom")),
                           i18nc("@action:inmenu Group images with this item and move them to its album",
                                 "Group here and move to album"));
}

static QAction* addCancelAction(QMenu* const menu)
{
    return menu->addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
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

    QMenu popMenu(view);

    QAction* moveAction = nullptr;

    if (allowMove)
    {
        moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")), i18n("&Move Here"));
    }

    QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
    popMenu.addSeparator();

    QAction* groupAction        = nullptr;
    QAction* groupAndMoveAction = nullptr;

    if (askForGrouping)
    {
        groupAction        = addGroupAction(&popMenu);
        groupAndMoveAction = addGroupAndMoveAction(&popMenu);
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
    else if (groupAndMoveAction && choice == groupAndMoveAction)
    {
        return GroupAndMoveAction;
    }

    return NoAction;
}

static DropAction tagAction(const QDropEvent* const, QWidget* const view, bool askForGrouping)
{
    QMenu popMenu(view);
    QAction* const tagAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                 i18n("Assign Tag to Dropped Items"));
    QAction* groupAction     = nullptr;

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
    ItemCategorizedView* const imgView = dynamic_cast<ItemCategorizedView*>(view);
    int sortOrder                      = ApplicationSettings::instance()->getImageSortOrder();

    QMenu popMenu(view);
    QAction* sortAction        = nullptr;

    if (imgView &&
        (sortOrder == ItemSortSettings::SortByManualOrderAndName ||
         sortOrder == ItemSortSettings::SortByManualOrderAndDate))
    {
        sortAction             = addSortAction(&popMenu);
        popMenu.addSeparator();
    }

    QAction* const groupAction = addGroupAction(&popMenu);
    popMenu.addSeparator();
    addCancelAction(&popMenu);

    QAction* const choice      = popMenu.exec(QCursor::pos());

    if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }

    if (sortAction && choice == sortAction)
    {
        return SortAction;
    }

    return NoAction;
}

// -------------------------------------------------------------------------------------

ItemDragDropHandler::ItemDragDropHandler(ItemModel* const model)
    : AbstractItemDragDropHandler(model),
      m_readOnly(false)
{
}

ItemModel* ItemDragDropHandler::model() const
{
    return static_cast<ItemModel*>(m_model);
}

ItemAlbumModel* ItemDragDropHandler::albumModel() const
{
    return qobject_cast<ItemAlbumModel*>(model());
}

void ItemDragDropHandler::setReadOnlyDrop(bool readOnly)
{
    m_readOnly = readOnly;
}

bool ItemDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    Album* album = nullptr;

    // Note that the drop event does not have to be in an ItemCategorizedView.
    // It can also be a TableViewTreeView.
    ItemCategorizedView* const view = qobject_cast<ItemCategorizedView*>(abstractview);

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

    PAlbum* palbum = nullptr;
    TAlbum* talbum = nullptr;

    if (album)
    {
        Album* currentAlbum = nullptr;

        if (albumModel() && !(albumModel()->currentAlbums().isEmpty()))
        {
            currentAlbum = albumModel()->currentAlbums().first();
        }

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
        QList<QUrl>      urls;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
        {
            return false;
        }

        DropAction action = NoAction;

        ItemInfo droppedOnInfo;

        if (droppedOn.isValid())
        {
            ItemThumbnailBar* const thumbBar = qobject_cast<ItemThumbnailBar*>(abstractview);

            if (thumbBar)
            {
                droppedOnInfo = model()->imageInfo(thumbBar->imageFilterModel()->mapToSourceItemModel(droppedOn));
            }
            else
            {
                droppedOnInfo = model()->imageInfo(droppedOn);
            }
        }

        if (m_readOnly)
        {
            emit itemInfosDropped(ItemInfoList(imageIDs));
            return true;
        }
        else if (palbum)
        {
            // Check if items dropped come from outside current album.
            QList<ItemInfo> extImages, intImages;

            if (imageIDs.isEmpty())
            {
                return false;
            }

            for (QList<qlonglong>::const_iterator it = imageIDs.constBegin() ;
                 it != imageIDs.constEnd() ; ++it)
            {
                ItemInfo info(*it);

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

            if (onlyInternal        &&
                droppedOn.isValid() &&
                e->keyboardModifiers() == Qt::NoModifier)
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
                    DIO::copy(extImages + intImages, palbum);
                    return true;
                }
            }
        }
        else if (talbum)
        {
            action = tagAction(e, view, droppedOn.isValid());

            if (action == AssignTagAction)
            {
                emit assignTags(ItemInfoList(imageIDs), QList<int>() << talbum->id());
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

            emit addToGroup(droppedOnInfo, ItemInfoList(imageIDs));
            return true;
        }

        if (action == SortAction)
        {
            if (droppedOnInfo.isNull())
            {
                return false;
            }

            emit dragDropSort(droppedOnInfo, ItemInfoList(imageIDs));
            return true;
        }

        if (action == GroupAndMoveAction)
        {
            if (droppedOnInfo.isNull())
            {
                return false;
            }

            emit addToGroup(droppedOnInfo, ItemInfoList(imageIDs));
            DIO::move(ItemInfoList(imageIDs), palbum);
            return true;
        }

        return false;
    }
    else if (e->mimeData()->hasUrls())
    {
        if (!palbum && !m_readOnly)
        {
            return false;
        }

        // Drag & drop outside of digiKam

        QList<QUrl> srcURLs = e->mimeData()->urls();

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

        if (!isDecoded)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Error: Decoding failed!";
            return false;
        }

        if (!view)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Error: View is null!";
            return false;
        }

        QMenu popMenu(view);

        QList<ItemInfo> selectedInfos  = view->selectedItemInfosCurrentFirst();
        QAction* assignToSelectedAction = nullptr;

        if (selectedInfos.count() > 1)
        {
            assignToSelectedAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                       i18n("Assign Tags to &Selected Items"));
        }

        QAction* assignToThisAction = nullptr;

        if (droppedOn.isValid())
        {
            assignToThisAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                   i18n("Assign Tags to &This Item"));
        }

        QAction* const assignToAllAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("tag")),
                                                             i18n("Assign Tags to &All Items"));

        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("&Cancel"));

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
                emit assignTags(view->allItemInfos(), tagIDs);
            }
            else if (choice == assignToThisAction)    // Dropped item only.
            {
                emit assignTags(QList<ItemInfo>() << model()->imageInfo(droppedOn), tagIDs);
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

        QMenu popMenu(iconView);
        popMenu.addSection(QIcon::fromTheme(QLatin1String("digikam")), i18n("Importing"));
        QAction* const downAction    = popMenu.addAction(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")),
                                                         i18n("Download From Camera"));
        QAction* const downDelAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("get-hot-new-stuff")),
                                                         i18n("Download && Delete From Camera"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice        = popMenu.exec(iconView->mapToGlobal(e->pos()));

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

Qt::DropAction ItemDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (albumModel() && albumModel()->currentAlbums().isEmpty())
    {
        return Qt::IgnoreAction;
    }

    if (DItemDrag::canDecode(e->mimeData()) || e->mimeData()->hasUrls())
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

QStringList ItemDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DItemDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << DCameraDragObject::mimeTypes()
              << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* ItemDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ItemInfo> infos = model()->imageInfos(indexes);

    QList<QUrl>      urls;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;

    foreach (const ItemInfo& info, infos)
    {
        urls.append(info.fileUrl());
        albumIDs.append(info.albumId());
        imageIDs.append(info.id());
    }

    if (urls.isEmpty())
    {
        return nullptr;
    }

    return new DItemDrag(urls, albumIDs, imageIDs);
}

} // namespace Digikam
