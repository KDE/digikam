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
#include <QContextMenuEvent>

// KDE includes

#include <kmenu.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "dpopupmenu.h"
#include "imagefiltermodel.h"
#include "imagedragdrop.h"
#include "metadatamanager.h"
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

    setModels(d->imageInfoModel, d->imageFilterModel);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(d->dragDropHandler, SIGNAL(imageInfosDropped(const QList<ImageInfo>&)),
            this, SIGNAL(signalDroppedItems(const QList<ImageInfo>&)));

    connect(d->imageInfoModel, SIGNAL(imageInfosAdded(const QList<ImageInfo>&)),
            this, SIGNAL(signalContentChanged()));
}

LightTableThumbBar::~LightTableThumbBar()
{
    delete d;
}

void LightTableThumbBar::setItems(const ImageInfoList& list)
{
    foreach(ImageInfo info, list)
    {
        if (!d->imageInfoModel->hasImage(info))
            d->imageInfoModel->addImageInfo(info);
    }
}

void LightTableThumbBar::clear()
{
    d->imageInfoModel->clearImageInfos();
    emit signalContentChanged();
}

void LightTableThumbBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableThumbBar::showContextMenuOnInfo(QContextMenuEvent* e, const ImageInfo& info)
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

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.addAction(leftPanelAction, true);
    cmhelper.addAction(rightPanelAction, true);
    cmhelper.addSeparator();
    cmhelper.addAction(editAction);
    cmhelper.addAction(removeAction);
    cmhelper.addSeparator();
    cmhelper.addLabelsAction();
    cmhelper.addSeparator();
    cmhelper.addAction(clearAllAction, true);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    QAction* choice = cmhelper.exec(e->globalPos());

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
    MetadataManager::instance()->assignPickLabel(info, pickId);
}

void LightTableThumbBar::assignRating(const ImageInfo& info, int rating)
{
    MetadataManager::instance()->assignRating(info, rating);
}

void LightTableThumbBar::assignColorLabel(const ImageInfo& info, int colorId)
{
    MetadataManager::instance()->assignColorLabel(info, colorId);
}

void LightTableThumbBar::toggleTag(int tagID)
{
    ImageInfo info = currentInfo();

    if (!info.isNull())
    {
        if (!info.tagIds().contains(tagID))
            MetadataManager::instance()->assignTag(info, tagID);
        else
            MetadataManager::instance()->removeTag(info, tagID);
    }
}

void LightTableThumbBar::setOnLeftPanel(const ImageInfo& info)
{
    for (QModelIndex index = firstIndex(); index.isValid(); index = nextIndex(index))
    {
        if (!info.isNull())
        {
            if (findItemByIndex(index) == info)
            {
                d->imageInfoModel->setData(index, true, ImageModel::LTLeftPanelRole);
            }
            else if (isOnLeftPanel(findItemByIndex(index)))
            {
                d->imageInfoModel->setData(index, false, ImageModel::LTLeftPanelRole);
            }
        }
        else if (isOnLeftPanel(findItemByIndex(index)))
        {
            d->imageInfoModel->setData(index, false, ImageModel::LTLeftPanelRole);
        }
    }
    viewport()->update();
}

void LightTableThumbBar::setOnRightPanel(const ImageInfo& info)
{
    for (QModelIndex index = firstIndex(); index.isValid(); index = nextIndex(index))
    {
        if (!info.isNull())
        {
            if (findItemByIndex(index) == info)
            {
                d->imageInfoModel->setData(index, true, ImageModel::LTRightPanelRole);
            }
            else if (isOnRightPanel(findItemByIndex(index)))
            {
                d->imageInfoModel->setData(index, false, ImageModel::LTRightPanelRole);
            }
        }
        else if (isOnRightPanel(findItemByIndex(index)))
        {
            d->imageInfoModel->setData(index, false, ImageModel::LTRightPanelRole);
        }
    }
    viewport()->update();
}

bool LightTableThumbBar::isOnLeftPanel(const ImageInfo& info) const
{
    QModelIndex index = findItemByInfo(info);
    if (index.isValid())
        return (d->imageInfoModel->data(index, ImageModel::LTLeftPanelRole).toBool());

    return false;
}

bool LightTableThumbBar::isOnRightPanel(const ImageInfo& info) const
{
    QModelIndex index = findItemByInfo(info);
    if (index.isValid())
        return (d->imageInfoModel->data(index, ImageModel::LTRightPanelRole).toBool());

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
    emit signalContentChanged();
}

int LightTableThumbBar::countItems() const
{
    return d->imageInfoModel->imageInfos().count();
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

void LightTableThumbBar::paintEvent(QPaintEvent* e)
{
    if (!countItems())
    {
        QPainter p(viewport());
        p.setPen(QPen(kapp->palette().color(QPalette::Text)));
        p.drawText(0, 0, width(), height(),
                    Qt::AlignCenter|Qt::TextWordWrap,
                    i18n("Drag and drop images here"));
        p.end();
        return;
    }

    ImageThumbnailBar::paintEvent(e);
}

}  // namespace Digikam
