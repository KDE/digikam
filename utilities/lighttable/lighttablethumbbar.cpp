/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablethumbbar.moc"

// Qt includes

#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QCursor>
#include <QPaintEvent>

// KDE includes

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "globals.h"
#include "imageattributeswatch.h"
#include "imagefiltermodel.h"
#include "imagedragdrop.h"
#include "metadatasettings.h"
#include "metadatahub.h"
#include "databasewatch.h"
#include "databasechangesets.h"
#include "themeengine.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class LightTableThumbBar::LightTableThumbBarPriv
{

public:

    LightTableThumbBarPriv()
    {
        navigateByPair   = false;
        imageInfoModel   = 0;
        imageFilterModel = 0;
        dragDropHandler  = 0;
    }

    bool                  navigateByPair;

    ImageListModel*       imageInfoModel;
    ImageFilterModel*     imageFilterModel;
    ImageDragDropHandler* dragDropHandler;
};

LightTableThumbBar::LightTableThumbBar(QWidget* parent)
    : ImageThumbnailBar(parent),
      d(new LightTableThumbBarPriv)
{
    d->imageInfoModel   = new ImageListModel(this);

    d->imageFilterModel = new ImageFilterModel(this);
    d->imageFilterModel->setSourceImageModel(d->imageInfoModel);

    d->imageInfoModel->setWatchFlags(d->imageFilterModel->suggestedWatchFlags());
    d->imageInfoModel->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    d->imageFilterModel->setCategorizationMode(ImageSortSettings::NoCategories);
    d->imageFilterModel->setSortRole((ImageSortSettings::SortRole)AlbumSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ImageSortSettings::SortOrder)AlbumSettings::instance()->getImageSorting());

    d->dragDropHandler = new ImageDragDropHandler(d->imageInfoModel);
    d->dragDropHandler->setReadOnlyDrop(true);
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset&)),
            this, SLOT(slotCollectionImageChange(const CollectionImageChangeset&)),
            Qt::QueuedConnection);
    /*
        connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
                this, SLOT(slotItemSelected(ThumbBarItem*)));
    */
}

LightTableThumbBar::~LightTableThumbBar()
{
    delete d;
}

void LightTableThumbBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableThumbBar::contentsMouseReleaseEvent(QMouseEvent* /*e*/)
{
/*    if (!e)
    {
        return;
    }

    ThumbBarView::contentsMouseReleaseEvent(e);

    QPoint pos = QCursor::pos();
    LightTableThumbBarItem* item = dynamic_cast<LightTableThumbBarItem*>(findItemByPos(e->pos()));

    if (e->button() == Qt::RightButton)
    {
        // temporary actions ----------------------------------

        QAction* leftPanelAction, *rightPanelAction, *editAction, *removeAction, *clearAllAction = 0;

        leftPanelAction  = new QAction(SmallIcon("arrow-left"),         i18n("Show on left panel"), this);
        rightPanelAction = new QAction(SmallIcon("arrow-right"),        i18n("Show on right panel"), this);
        editAction       = new QAction(SmallIcon("editimage"),          i18n("Edit"), this);
        removeAction     = new QAction(SmallIcon("dialog-close"),       i18n("Remove item"), this);
        clearAllAction   = new QAction(SmallIcon("edit-delete-shred"),  i18n("Clear all"), this);

        leftPanelAction->setEnabled(d->navigateByPair  ? false : true);
        rightPanelAction->setEnabled(d->navigateByPair ? false : true);
        clearAllAction->setEnabled(itemsUrls().count() ? true  : false);

        // ----------------------------------------------------

        KMenu popmenu(this);
        ContextMenuHelper cmhelper(&popmenu);

        if (item)
        {
            cmhelper.addAction(leftPanelAction, true);
            cmhelper.addAction(rightPanelAction, true);
            cmhelper.addSeparator();
            // ------------------------------------------------
            cmhelper.addAction(editAction);
            cmhelper.addAction(removeAction);
            cmhelper.addSeparator();
            // ------------------------------------------------
            cmhelper.addLabelsAction();
            cmhelper.addSeparator();
            // ------------------------------------------------
        }

        cmhelper.addAction(clearAllAction, true);

        // special action handling --------------------------------

        connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
                this, SLOT(slotAssignPickLabel(int)));

        connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
                this, SLOT(slotAssignColorLabel(int)));

        connect(&cmhelper, SIGNAL(signalAssignRating(int)),
                this, SLOT(slotAssignRating(int)));

        QAction* choice = cmhelper.exec(pos);

        if (choice)
        {
            if (choice == leftPanelAction)
            {
                emit signalSetItemOnLeftPanel(item->info());
            }
            else if (choice == rightPanelAction)
            {
                emit signalSetItemOnRightPanel(item->info());
            }
            else if (choice == editAction)
            {
                emit signalEditItem(item->info());
            }
            else if (choice == removeAction)
            {
                emit signalRemoveItem(item->info());
            }
            else if (choice == clearAllAction)
            {
                emit signalClearAll();
            }
        }
    }
*/
}

void LightTableThumbBar::slotAssignPickLabel(int /*pickId*/)
{
//    assignPickLabel(currentItemImageInfo(), pickId);
}

void LightTableThumbBar::slotAssignColorLabel(int /*colorId*/)
{
//    assignColorLabel(currentItemImageInfo(), colorId);
}

void LightTableThumbBar::assignPickLabel(const ImageInfo& info, int pickId)
{
    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setPickLabel(pickId);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableThumbBar::assignColorLabel(const ImageInfo& info, int colorId)
{
    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setColorLabel(colorId);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableThumbBar::slotRatingChanged(const KUrl& url, int rating)
{
    assignRating(ImageInfo(url), rating);
}

void LightTableThumbBar::slotAssignRating(int /*rating*/)
{
    //    assignRating(currentItemImageInfo(), rating);
}

void LightTableThumbBar::assignRating(const ImageInfo& info, int rating)
{
    rating = qMin(5, qMax(0, rating));

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableThumbBar::toggleTag(int tagID)
{
    ImageInfo info/* = currentItemImageInfo()*/;

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setTag(tagID, !info.tagIds().contains(tagID));
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableThumbBar::setOnLeftPanel(const ImageInfo& /*info*/)
{
/* FIXME
    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        LightTableThumbBarItem* ltItem = dynamic_cast<LightTableThumbBarItem*>(item);

        if (ltItem)
        {
            if (!info.isNull())
            {
                if (ltItem->info() == info)
                {
                    ltItem->setOnLeftPanel(true);
                    repaintItem(item);
                }
                else if (ltItem->isOnLeftPanel() == true)
                {
                    ltItem->setOnLeftPanel(false);
                    repaintItem(item);
                }
            }
            else if (ltItem->isOnLeftPanel() == true)
            {
                ltItem->setOnLeftPanel(false);
                repaintItem(item);
            }
        }
    }
*/
}

void LightTableThumbBar::setOnRightPanel(const ImageInfo& /*info*/)
{
/* FIXME
    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        LightTableThumbBarItem* ltItem = dynamic_cast<LightTableThumbBarItem*>(item);

        if (ltItem)
        {
            if (!info.isNull())
            {
                if (ltItem->info() == info)
                {
                    ltItem->setOnRightPanel(true);
                    repaintItem(item);
                }
                else if (ltItem->isOnRightPanel() == true)
                {
                    ltItem->setOnRightPanel(false);
                    repaintItem(item);
                }
            }
            else if (ltItem->isOnRightPanel() == true)
            {
                ltItem->setOnRightPanel(false);
                repaintItem(item);
            }
        }
    }
*/
}


QModelIndex LightTableThumbBar::findItemByInfo(const ImageInfo& info) const
{
    if (!info.isNull())
        return d->imageInfoModel->indexForImageInfo(info);

    return QModelIndex();
}

void LightTableThumbBar::removeItemByInfo(const ImageInfo& /*info*/)
{
/*
    if (info.isNull())
    {
        return;
    }

    ImagePreviewBarItem* ltItem = findItemByInfo(info);
    ThumbBarItem* item          = dynamic_cast<ThumbBarItem*>(ltItem);

    if (item)
    {
        removeItem(item);
    }
*/
}

void LightTableThumbBar::removeItemById(qlonglong /*id*/)
{
/*
    ImagePreviewBarItem* item = findItemById(id);

    if (item)
    {
        removeItem(item);
    }
*/
}

void LightTableThumbBar::drawEmptyMessage(QPixmap& /*bgPix*/)
{
/*
    ThemeEngine* te = ThemeEngine::instance();

    QPainter p4(&bgPix);
    p4.setPen(QPen(te->textRegColor()));
    p4.drawText(0, 0, bgPix.width(), bgPix.height(),
                Qt::AlignCenter|Qt::TextWordWrap,
                i18n("Drag and drop images here"));
    p4.end();
*/
}

void LightTableThumbBar::startDrag()
{
/*
    if (!currentItem())
    {
        return;
    }

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<int> imageIDs;

    LightTableThumbBarItem* item = dynamic_cast<LightTableThumbBarItem*>(currentItem());

    urls.append(item->info().fileUrl());
    kioURLs.append(item->info().databaseUrl());
    imageIDs.append(item->info().id());
    albumIDs.append(item->info().albumId());

    QPixmap icon(DesktopIcon("image-jp2", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4, h+4);
    QPainter p(&pix);
    p.fillRect(0, 0, pix.width()-1, pix.height()-1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
    p.drawPixmap(2, 2, icon);
    p.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(new DItemDrag(urls, kioURLs, albumIDs, imageIDs));
    drag->setPixmap(pix);
    drag->exec();
*/
}

void LightTableThumbBar::contentsDragEnterEvent(QDragEnterEvent* /*e*/)
{
/*
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID) ||
        DTagDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
*/
}

void LightTableThumbBar::contentsDropEvent(QDropEvent* /*e*/)
{
/*
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QList<int>::const_iterator it = imageIDs.constBegin();
             it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if (DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;

        if (!DTagDrag::decode(e->mimeData(), tagID))
        {
            return;
        }

        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else
    {
        e->ignore();
    }
*/
}

void LightTableThumbBar::slotCollectionImageChange(const CollectionImageChangeset& /*changeset*/)
{
/*
    switch (changeset.operation())
    {
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
        {
            ImageInfo info;
            foreach (const qlonglong& id, changeset.ids())
            {
                ImagePreviewBarItem* item = findItemById(id);

                if (item)
                {
                    info = item->info();
                    removeItem(item);
                    emit signalRemoveItem(info);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
*/
}

int LightTableThumbBar::countItems() const
{
    return imageInfos().count();
}

}  // namespace Digikam
