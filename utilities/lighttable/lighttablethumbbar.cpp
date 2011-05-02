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

#include <QAction>
#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QContextMenuEvent>

// KDE includes

#include <kmenu.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes

#include "albumdb.h"
#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "globals.h"
#include "imagefiltermodel.h"
#include "imagedragdrop.h"
#include "metadatasettings.h"
#include "metadatahub.h"
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
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setModels(d->imageInfoModel, d->imageFilterModel);
}

LightTableThumbBar::~LightTableThumbBar()
{
    delete d;
}

void LightTableThumbBar::setItems(const ImageInfoList& list)
{
    d->imageInfoModel->setImageInfos(list);
}

void LightTableThumbBar::clear()
{
    setItems(ImageInfoList());
}

void LightTableThumbBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableThumbBar::showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info)
{
    // temporary actions ----------------------------------

    QAction* leftPanelAction=0, *rightPanelAction=0, *editAction=0, *removeAction=0, *clearAllAction=0;

    leftPanelAction  = new QAction(SmallIcon("arrow-left"),        i18n("Show on left panel"),  this);
    rightPanelAction = new QAction(SmallIcon("arrow-right"),       i18n("Show on right panel"), this);
    editAction       = new QAction(SmallIcon("editimage"),         i18n("Edit"),                this);
    removeAction     = new QAction(SmallIcon("dialog-close"),      i18n("Remove item"),         this);
    clearAllAction   = new QAction(SmallIcon("edit-delete-shred"), i18n("Clear all"),           this);

    leftPanelAction->setEnabled(d->navigateByPair  ? false : true);
    rightPanelAction->setEnabled(d->navigateByPair ? false : true);
    clearAllAction->setEnabled(countItems()        ? true  : false);

    // ----------------------------------------------------

    KMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    if (!info.isNull())
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
    }

    cmhelper.addAction(clearAllAction, true);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    QAction* choice = cmhelper.exec(event->globalPos());

    if (choice)
    {
        if (choice == leftPanelAction)
        {
            emit signalSetItemOnLeftPanel(info);
        }
        else if (choice == rightPanelAction)
        {
            emit signalSetItemOnRightPanel(info);
        }
        else if (choice == editAction)
        {
            emit signalEditItem(info);
        }
        else if (choice == removeAction)
        {
            emit signalRemoveItem(info);
        }
        else if (choice == clearAllAction)
        {
            emit signalClearAll();
        }
    }
}

void LightTableThumbBar::slotAssignPickLabel(int pickId)
{
    assignPickLabel(currentInfo(), pickId);
}

void LightTableThumbBar::slotAssignColorLabel(int colorId)
{
    assignColorLabel(currentInfo(), colorId);
}

void LightTableThumbBar::slotRatingChanged(const KUrl& url, int rating)
{
    assignRating(ImageInfo(url), rating);
}

void LightTableThumbBar::slotAssignRating(int rating)
{
    assignRating(currentInfo(), rating);
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

void LightTableThumbBar::toggleTag(int tagID)
{
    ImageInfo info = currentInfo();

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

bool LightTableThumbBar::isOnLeftPanel(const ImageInfo& info) const
{
    // FIXME
    return false;
}

bool LightTableThumbBar::isOnRightPanel(const ImageInfo& info) const
{
    // FIXME
    return false;
}

QModelIndex LightTableThumbBar::findItemByInfo(const ImageInfo& info) const
{
    if (!info.isNull())
        return d->imageInfoModel->indexForImageInfo(info);

    return QModelIndex();
}

ImageInfo LightTableThumbBar::findItemByIndex(const QModelIndex& index) const
{
    if (index.isValid())
        return d->imageInfoModel->imageInfo(index);

    return ImageInfo();
}

void LightTableThumbBar::removeItemByInfo(const ImageInfo& info)
{
    if (info.isNull())
        return;

    d->imageInfoModel->removeImageInfo(info);
}

int LightTableThumbBar::countItems() const
{
    return imageInfos().count();
}

void LightTableThumbBar::setSelectedItem(const ImageInfo& info)
{
    setSelectedImageInfos(QList<ImageInfo>() << info);
}

void LightTableThumbBar::setSelectedIndex(const QModelIndex& index)
{
    setSelectedItem(findItemByIndex(index));
}

void LightTableThumbBar::ensureItemVisible(const ImageInfo& info)
{
    if (!info.isNull())
        scrollTo(findItemByInfo(info), QAbstractItemView::PositionAtCenter);
}

void LightTableThumbBar::drawEmptyMessage(QPixmap& /*bgPix*/)
{
/*
    QPainter p4(&bgPix);
    p4.setPen(QPen(kapp->palette().color(QPalette::Text)));
    p4.drawText(0, 0, bgPix.width(), bgPix.height(),
                Qt::AlignCenter|Qt::TextWordWrap,
                i18n("Drag and drop images here"));
    p4.end();
*/
}

}  // namespace Digikam
