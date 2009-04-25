/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-22
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagecategorizedview.h"
#include "imagecategorizedview.moc"

// Qt includes

#include <QHelpEvent>

// KDE includes

#include <kdebug.h>

// Local includes

#include "databasefields.h"
#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "imagethumbnailmodel.h"
#include "imagedelegate.h"
#include "imagecategorydrawer.h"
#include "itemviewtooltip.h"
#include "thumbnailloadthread.h"
#include "tooltipfiller.h"

namespace Digikam
{

class ImageItemViewToolTip : public ItemViewToolTip
{
public:

    ImageItemViewToolTip(ImageCategorizedView *view)
        : ItemViewToolTip(view)
    {
    }

    ImageCategorizedView *view() const
    { return static_cast<ImageCategorizedView*>(ItemViewToolTip::view()); }

protected:

    virtual QString tipContents()
    {
        ImageInfo info = ImageModel::retrieveImageInfo(currentIndex());
        return ToolTipFiller::imageInfoTipContents(info);
    }
};

// -------------------------------------------------------------------------------

class ImageCategorizedViewPriv
{
public:

    ImageCategorizedViewPriv()
    {
        model          = 0;
        filterModel    = 0;
        delegate       = 0;
        categoryDrawer = 0;
        toolTip        = 0;
        scrollToItemId = 0;
    }

    ImageAlbumModel         *model;
    ImageAlbumFilterModel   *filterModel;

    ImageDelegate           *delegate;
    ImageCategoryDrawer     *categoryDrawer;
    ImageItemViewToolTip    *toolTip;

    ThumbnailSize            thumbnailSize;
    qlonglong                scrollToItemId;
};

// -------------------------------------------------------------------------------

ImageCategorizedView::ImageCategorizedView(QWidget *parent)
                    : KCategorizedView(parent), d(new ImageCategorizedViewPriv)
{
    setSpacing(10);
    setViewMode(QListView::IconMode);
    setLayoutDirection(Qt::LeftToRight);
    setFlow(QListView::LeftToRight);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setWrapping(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setDragEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);

    d->model = new ImageAlbumModel(this);
    d->model->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    d->filterModel = new ImageAlbumFilterModel(this);
    d->filterModel->setSourceImageModel(d->model);

    // set flags that we want to get dataChanged() signals for
    DatabaseFields::Set watchFlags;
    watchFlags |= DatabaseFields::Name | DatabaseFields::FileSize | DatabaseFields::ModificationDate;
    watchFlags |= DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::Orientation |
                  DatabaseFields::Width | DatabaseFields::Height;
    watchFlags |= DatabaseFields::Comment;
    d->model->setWatchFlags(watchFlags);

    d->delegate = new ImageDelegate(this);
    setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());

    d->toolTip = new ImageItemViewToolTip(this);

    setModel(d->filterModel);

    connect(d->model, SIGNAL(signalImageInfosAdded(const QList<ImageInfo> &)),
            this, SLOT(slotImageInfosAdded()));

    connect(this, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(slotActivated(const ImageInfo &)));
    /*
    if (KGlobalSettings::singleClick())
    {
        connect(this, SIGNAL(clicked(const QModelIndex&)),
                controller, SLOT(triggerItem(const QModelIndex&)));
    }
    else
    {
        connect(this, SIGNAL(doubleClicked(const QModelIndex&)),
                controller, SLOT(triggerItem(const QModelIndex&)));
    }
    */
}

ImageCategorizedView::~ImageCategorizedView()
{
    delete d->toolTip;
    delete d;
}

ImageAlbumModel *ImageCategorizedView::imageModel() const
{
    return d->model;
}

ImageAlbumFilterModel *ImageCategorizedView::imageFilterModel() const
{
    return d->filterModel;
}

ImageDelegate *ImageCategorizedView::delegate() const
{
    return d->delegate;
}

ImageInfo ImageCategorizedView::currentInfo() const
{
    return d->filterModel->imageInfo(currentIndex());
}

QList<ImageInfo> ImageCategorizedView::selectedImageInfos() const
{
    return d->filterModel->imageInfos(selectedIndexes());
}

QList<ImageInfo> ImageCategorizedView::selectedImageInfosCurrentFirst() const
{
    QList<QModelIndex> indexes = selectedIndexes();
    QModelIndex        current = currentIndex();
    QList<ImageInfo>   infos;
    foreach (const QModelIndex &index, indexes)
    {
        ImageInfo info = d->filterModel->imageInfo(index);
        if (index == current)
            infos.prepend(info);
        else
            infos.append(info);
    }
    return infos;
}

QList<ImageInfo> ImageCategorizedView::imageInfos() const
{
    return d->filterModel->imageInfosSorted();
}

KUrl::List ImageCategorizedView::urls() const
{
    QList<ImageInfo> infos = imageInfos();
    KUrl::List urls;
    foreach (const ImageInfo &info, infos)
        urls << info.fileUrl();
    return urls;
}

KUrl::List ImageCategorizedView::selectedUrls() const
{
    QList<ImageInfo> infos = selectedImageInfos();
    KUrl::List urls;
    foreach (const ImageInfo &info, infos)
        urls << info.fileUrl();
    return urls;
}

ThumbnailSize ImageCategorizedView::thumbnailSize() const
{
    return d->model->thumbnailSize();
}

void ImageCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(size);
}

void ImageCategorizedView::setThumbnailSize(const ThumbnailSize &size)
{
    d->model->setThumbnailSize(size);
    d->delegate->setThumbnailSize(size);
    //viewport()->update();
}

void ImageCategorizedView::scrollToWhenAvailable(qlonglong imageId)
{
    d->scrollToItemId = imageId;
}

void ImageCategorizedView::setCurrentUrl(const KUrl &url)
{
    QString path = url.path();
    QModelIndex index = d->filterModel->indexForPath(path);
    if (!index.isValid())
    {
        kWarning() << "no QModelIndex found for" << url;
        return;
    }
    setCurrentIndex(index);
}

void ImageCategorizedView::scrollToStoredItem()
{
    if (d->scrollToItemId)
    {
        if (d->model->hasImage(d->scrollToItemId))
        {
            scrollTo(d->filterModel->indexForImageId(d->scrollToItemId));
            d->scrollToItemId = 0;
        }
    }
}

void ImageCategorizedView::slotImageInfosAdded()
{
    if (d->scrollToItemId)
        scrollToStoredItem();
}

void ImageCategorizedView::slotThemeChanged()
{
    viewport()->update();
}

void ImageCategorizedView::slotSetupChanged()
{
    viewport()->update();
}

void ImageCategorizedView::slotActivated(const QModelIndex &index)
{
    ImageInfo info = d->filterModel->imageInfo(index);
    if (!info.isNull())
        activated(info);
}

void ImageCategorizedView::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid())
    {
        ImageInfo info = d->filterModel->imageInfo(index);
        showContextMenu(event, info);
    }
    else
    {
        showContextMenu(event);
    }
}

void ImageCategorizedView::activated(const ImageInfo &)
{
}

void ImageCategorizedView::showContextMenu(QContextMenuEvent *, const ImageInfo &)
{
}

void ImageCategorizedView::showContextMenu(QContextMenuEvent *)
{
}

void ImageCategorizedView::copy()
{
}

void ImageCategorizedView::paste()
{
}

void ImageCategorizedView::keyPressEvent(QKeyEvent *event)
{
    if (event == QKeySequence::Copy)
    {
        copy();
        event->accept();
        return;
    }
    else if (event == QKeySequence::Paste)
    {
        paste();
        event->accept();
        return;
    }

    /*
    // from dolphincontroller.cpp
    const QItemSelectionModel* selModel = m_itemView->selectionModel();
    const QModelIndex currentIndex = selModel->currentIndex();
    const bool trigger = currentIndex.isValid()
                         && ((event->key() == Qt::Key_Return)
                            || (event->key() == Qt::Key_Enter))
                         && (selModel->selectedIndexes().count() > 0);
    if (trigger) {
        const QModelIndexList indexList = selModel->selectedIndexes();
        foreach (const QModelIndex& index, indexList) {
            emit itemTriggered(itemForIndex(index));
        }
    }
    */
    KCategorizedView::keyPressEvent(event);
}

bool ImageCategorizedView::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::FontChange:
            d->delegate->setDefaultViewOptions(viewOptions());
            break;
        case QEvent::ToolTip:
        {
            QHelpEvent *he = static_cast<QHelpEvent*>(event);
            const QModelIndex index = indexAt(he->pos());
            if (!index.isValid())
                break;
            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(index);
            option.state |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
            if (d->delegate->acceptsToolTip(he->pos(), option, index))
                d->toolTip->show(he, option, index);
            return true;
        }
        default:
            break;
    }
    return KCategorizedView::viewportEvent(event);
}

} // namespace Digikam
