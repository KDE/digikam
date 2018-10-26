/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-22
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
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

// Qt includes

#include <QApplication>
#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "coredbfields.h"
#include "iccsettings.h"
#include "imagealbummodel.h"
#include "itemalbumfiltermodel.h"
#include "imagecategorydrawer.h"
#include "imagedelegate.h"
#include "imagedelegateoverlay.h"
#include "itemthumbnailmodel.h"
#include "imageselectionoverlay.h"
#include "itemviewtooltip.h"
#include "loadingcacheinterface.h"
#include "thumbnailloadthread.h"
#include "tooltipfiller.h"
#include "digikamimagefacedelegate.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageItemViewToolTip : public ItemViewToolTip
{
public:

    explicit ImageItemViewToolTip(ImageCategorizedView* const view)
        : ItemViewToolTip(view)
    {
    }

    ImageCategorizedView* view() const
    {
        return static_cast<ImageCategorizedView*>(ItemViewToolTip::view());
    }

protected:

    virtual QString tipContents()
    {
        ItemInfo info = ItemModel::retrieveItemInfo(currentIndex());
        return ToolTipFiller::imageInfoTipContents(info);
    }
};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN ImageCategorizedView::Private
{
public:

    explicit Private()
      : model(0),
        filterModel(0),
        delegate(0),
        showToolTip(false),
        scrollToItemId(0),
        delayedEnterTimer(0),
        currentMouseEvent(0)
    {
    }

    ItemModel*           model;
    ImageSortFilterModel* filterModel;

    ImageDelegate*        delegate;
    bool                  showToolTip;

    qlonglong             scrollToItemId;

    QUrl                  unknownCurrentUrl;

    QTimer*               delayedEnterTimer;

    QMouseEvent*          currentMouseEvent;
};

// -------------------------------------------------------------------------------

ImageCategorizedView::ImageCategorizedView(QWidget* const parent)
    : ItemViewCategorized(parent),
      d(new Private)
{
    setToolTip(new ImageItemViewToolTip(this));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(QString)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged(ICCSettingsContainer,ICCSettingsContainer)),
            this, SLOT(slotIccSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)));

    d->delayedEnterTimer = new QTimer(this);
    d->delayedEnterTimer->setInterval(10);
    d->delayedEnterTimer->setSingleShot(true);

    connect(d->delayedEnterTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedEnter()));
}

ImageCategorizedView::~ImageCategorizedView()
{
    d->delegate->removeAllOverlays();
    delete d;
}

void ImageCategorizedView::installDefaultModels()
{
    ImageAlbumModel* model             = new ImageAlbumModel(this);
    ItemAlbumFilterModel* filterModel = new ItemAlbumFilterModel(this);

    filterModel->setSourceItemModel(model);

    filterModel->setSortRole(ItemSortSettings::SortByFileName);
    filterModel->setCategorizationMode(ItemSortSettings::CategoryByAlbum);
    filterModel->sort(0); // an initial sorting is necessary

    // set flags that we want to get dataChanged() signals for
    model->setWatchFlags(filterModel->suggestedWatchFlags());

    setModels(model, filterModel);
}

void ImageCategorizedView::setModels(ItemModel* model, ImageSortFilterModel* filterModel)
{
    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(false);
    }

    if (d->filterModel)
    {
        disconnect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(layoutAboutToBeChanged()));

        disconnect(d->filterModel, SIGNAL(layoutChanged()),
                   this, SLOT(layoutWasChanged()));
    }

    if (d->model)
    {
        disconnect(d->model, SIGNAL(imageInfosAdded(QList<ItemInfo>)),
                   this, SLOT(slotItemInfosAdded()));
    }

    d->model       = model;
    d->filterModel = filterModel;

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(imageInfosAdded(QList<ItemInfo>)),
            this, SLOT(slotItemInfosAdded()));

    emit modelChanged();

    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(true);
    }
}

ItemModel* ImageCategorizedView::imageModel() const
{
    return d->model;
}

ImageSortFilterModel* ImageCategorizedView::imageSortFilterModel() const
{
    return d->filterModel;
}

ItemFilterModel* ImageCategorizedView::imageFilterModel() const
{
    return d->filterModel->imageFilterModel();
}

ItemThumbnailModel* ImageCategorizedView::imageThumbnailModel() const
{
    return qobject_cast<ItemThumbnailModel*>(d->model);
}

ImageAlbumModel* ImageCategorizedView::imageAlbumModel() const
{
    return qobject_cast<ImageAlbumModel*>(d->model);
}

ItemAlbumFilterModel* ImageCategorizedView::imageAlbumFilterModel() const
{
    return qobject_cast<ItemAlbumFilterModel*>(d->filterModel->imageFilterModel());
}

QSortFilterProxyModel* ImageCategorizedView::filterModel() const
{
    return d->filterModel;
}

ImageDelegate* ImageCategorizedView::delegate() const
{
    return d->delegate;
}

void ImageCategorizedView::setItemDelegate(ImageDelegate* delegate)
{
    ThumbnailSize oldSize      = thumbnailSize();
    ImageDelegate* oldDelegate = d->delegate;

    if (oldDelegate)
    {
        hideIndexNotification();
        d->delegate->setAllOverlaysActive(false);
        d->delegate->setViewOnAllOverlays(0);
        // Note: Be precise, no wildcard disconnect!
        disconnect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
                   this, SLOT(showIndexNotification(QModelIndex,QString)));
        disconnect(d->delegate, SIGNAL(hideNotification()),
                   this, SLOT(hideIndexNotification()));
    }

    d->delegate = delegate;
    d->delegate->setThumbnailSize(oldSize);

    if (oldDelegate)
    {
        d->delegate->setSpacing(oldDelegate->spacing());
    }

    ItemViewCategorized::setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());
    updateDelegateSizes();

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
            this, SLOT(showIndexNotification(QModelIndex,QString)));

    connect(d->delegate, SIGNAL(hideNotification()),
            this, SLOT(hideIndexNotification()));
}

Album* ImageCategorizedView::currentAlbum() const
{
    ImageAlbumModel* albumModel = imageAlbumModel();
    /** TODO: Change to QList return type **/
    if (albumModel && !(albumModel->currentAlbums().isEmpty()))
    {
        return albumModel->currentAlbums().first();
    }

    return 0;
}

ItemInfo ImageCategorizedView::currentInfo() const
{
    return imageInfo(currentIndex());
}

QUrl ImageCategorizedView::currentUrl() const
{
    return currentInfo().fileUrl();
}

ItemInfo ImageCategorizedView::imageInfo(const QModelIndex& index) const
{
    return d->filterModel->imageInfo(index);
}

ItemInfoList ImageCategorizedView::imageInfos(const QList<QModelIndex>& indexes) const
{
    return ItemInfoList(d->filterModel->imageInfos(indexes));
}

ItemInfoList ImageCategorizedView::allItemInfos() const
{
    return ItemInfoList(d->filterModel->imageInfosSorted());
}

QList<QUrl> ImageCategorizedView::allUrls() const
{
    return allItemInfos().toImageUrlList();
}

ItemInfoList ImageCategorizedView::selectedItemInfos() const
{
    return imageInfos(selectedIndexes());
}

ItemInfoList ImageCategorizedView::selectedItemInfosCurrentFirst() const
{
    QModelIndexList indexes   = selectedIndexes();
    const QModelIndex current = currentIndex();

    if (!indexes.isEmpty())
    {
        if (indexes.first() != current)
        {
            if (indexes.removeOne(current))
            {
                indexes.prepend(current);
            }
        }
    }

    return imageInfos(indexes);
}

void ImageCategorizedView::toIndex(const QUrl& url)
{
    ItemViewCategorized::toIndex(d->filterModel->indexForPath(url.toLocalFile()));
}

QModelIndex ImageCategorizedView::indexForInfo(const ItemInfo& info) const
{
    return d->filterModel->indexForItemInfo(info);
}

ItemInfo ImageCategorizedView::nextInOrder(const ItemInfo& startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForItemInfo(startingPoint);

    if (!index.isValid())
    {
        return ItemInfo();
    }

    return imageInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

QModelIndex ImageCategorizedView::nextIndexHint(const QModelIndex& anchor, const QItemSelectionRange& removed) const
{
    QModelIndex hint = ItemViewCategorized::nextIndexHint(anchor, removed);
    ItemInfo info   = imageInfo(anchor);

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Having initial hint" << hint << "for" << anchor << d->model->numberOfIndexesForItemInfo(info);

    // Fixes a special case of multiple (face) entries for the same image.
    // If one is removed, any entry of the same image shall be preferred.
    if (d->model->numberOfIndexesForItemInfo(info) > 1)
    {
        // The hint is for a different info, but we may have a hint for the same info
        if (info != imageInfo(hint))
        {
            int minDiff                            = d->filterModel->rowCount();
            QList<QModelIndex> indexesForItemInfo = d->filterModel->mapListFromSource(d->model->indexesForItemInfo(info));

            foreach(const QModelIndex& index, indexesForItemInfo)
            {
                if (index == anchor || !index.isValid() || removed.contains(index))
                {
                    continue;
                }

                int distance = qAbs(index.row() - anchor.row());

                if (distance < minDiff)
                {
                    minDiff = distance;
                    hint    = index;
                    //qCDebug(DIGIKAM_GENERAL_LOG) << "Chose index" << hint << "at distance" << minDiff << "to" << anchor;
                }
            }
        }
    }

    return hint;
}

void ImageCategorizedView::openAlbum(const QList<Album*>& albums)
{
    ImageAlbumModel* const albumModel = imageAlbumModel();

    if (albumModel)
    {
        albumModel->openAlbum(albums);
    }
}

ThumbnailSize ImageCategorizedView::thumbnailSize() const
{
/*
    ItemThumbnailModel* const thumbModel = imageThumbnailModel();
    if (thumbModel)
        return thumbModel->thumbnailSize();
*/
    if (d->delegate)
    {
        return d->delegate->thumbnailSize();
    }

    return ThumbnailSize();
}

void ImageCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(ThumbnailSize(size));
}

void ImageCategorizedView::setThumbnailSize(const ThumbnailSize& s)
{
    // we abuse this pair of method calls to restore scroll position
    layoutAboutToBeChanged();
    d->delegate->setThumbnailSize(s);
    layoutWasChanged();
}

void ImageCategorizedView::setCurrentWhenAvailable(qlonglong imageId)
{
    d->scrollToItemId = imageId;
}

void ImageCategorizedView::setCurrentUrlWhenAvailable(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());
        return;
    }

    QString path      = url.toLocalFile();
    QModelIndex index = d->filterModel->indexForPath(path);

    if (!index.isValid())
    {
        d->unknownCurrentUrl = url;
        return;
    }

    clearSelection();
    setCurrentIndex(index);
    d->unknownCurrentUrl.clear();
}

void ImageCategorizedView::setCurrentUrl(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());
        return;
    }

    QString path      = url.toLocalFile();
    QModelIndex index = d->filterModel->indexForPath(path);

    if (!index.isValid())
    {
        return;
    }

    clearSelection();
    setCurrentIndex(index);
}

void ImageCategorizedView::setCurrentInfo(const ItemInfo& info)
{
    QModelIndex index = d->filterModel->indexForItemInfo(info);
    clearSelection();
    setCurrentIndex(index);
}

void ImageCategorizedView::setSelectedUrls(const QList<QUrl>& urlList)
{
    QItemSelection mySelection;

    for (QList<QUrl>::const_iterator it = urlList.constBegin(); it!=urlList.constEnd(); ++it)
    {
        const QString path      = it->toLocalFile();
        const QModelIndex index = d->filterModel->indexForPath(path);

        if (!index.isValid())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "no QModelIndex found for" << *it;
        }
        else
        {
            // TODO: is there a better way?
            mySelection.select(index, index);
        }
    }

    clearSelection();
    selectionModel()->select(mySelection, QItemSelectionModel::Select);
}

void ImageCategorizedView::setSelectedItemInfos(const QList<ItemInfo>& infos)
{
    QItemSelection mySelection;

    foreach(const ItemInfo& info, infos)
    {
        QModelIndex index = d->filterModel->indexForItemInfo(info);
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ImageCategorizedView::hintAt(const ItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    QModelIndex index = d->filterModel->indexForItemInfo(info);

    if (!index.isValid())
    {
        return;
    }

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    scrollTo(index);
}

void ImageCategorizedView::addOverlay(ImageDelegateOverlay* overlay, ImageDelegate* delegate)
{
    if (!delegate)
    {
        delegate = d->delegate;
    }

    delegate->installOverlay(overlay);

    if (delegate == d->delegate)
    {
        overlay->setView(this);
        overlay->setActive(true);
    }
}

void ImageCategorizedView::removeOverlay(ImageDelegateOverlay* overlay)
{
    ImageDelegate* delegate = dynamic_cast<ImageDelegate*>(overlay->delegate());

    if (delegate)
    {
        delegate->removeOverlay(overlay);
    }

    overlay->setView(0);
}

void ImageCategorizedView::updateGeometries()
{
    ItemViewCategorized::updateGeometries();
    d->delayedEnterTimer->start();
}

void ImageCategorizedView::slotDelayedEnter()
{
    // re-emit entered() for index under mouse (after layout).
    QModelIndex mouseIndex = indexAt(mapFromGlobal(QCursor::pos()));

    if (mouseIndex.isValid())
    {
        emit DCategorizedView::entered(mouseIndex);
    }
}

void ImageCategorizedView::addSelectionOverlay(ImageDelegate* delegate)
{
    addOverlay(new ImageSelectionOverlay(this), delegate);
}

void ImageCategorizedView::scrollToStoredItem()
{
    if (d->scrollToItemId)
    {
        if (d->model->hasImage(d->scrollToItemId))
        {
            QModelIndex index = d->filterModel->indexForImageId(d->scrollToItemId);
            setCurrentIndex(index);
            scrollToRelaxed(index, QAbstractItemView::PositionAtCenter);
            d->scrollToItemId = 0;
        }
    }
}

void ImageCategorizedView::slotItemInfosAdded()
{
    if (d->scrollToItemId)
    {
        scrollToStoredItem();
    }
    else if (!d->unknownCurrentUrl.isEmpty())
    {
        QTimer::singleShot(100, this, SLOT(slotCurrentUrlTimer()));
    }
}

void ImageCategorizedView::slotCurrentUrlTimer()
{
    setCurrentUrl(d->unknownCurrentUrl);
    d->unknownCurrentUrl.clear();
}

void ImageCategorizedView::slotFileChanged(const QString& filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);

    if (index.isValid())
    {
        update(index);
    }
}

void ImageCategorizedView::indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
{
    ItemInfo info = imageInfo(index);

    if (!info.isNull())
    {
        activated(info, modifiers);
        emit imageActivated(info);
    }
}

void ImageCategorizedView::currentChanged(const QModelIndex& index, const QModelIndex& previous)
{
    ItemViewCategorized::currentChanged(index, previous);

    emit currentChanged(imageInfo(index));
}

void ImageCategorizedView::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    ItemViewCategorized::selectionChanged(selectedItems, deselectedItems);

    if (!selectedItems.isEmpty())
    {
        emit selected(imageInfos(selectedItems.indexes()));
    }

    if (!deselectedItems.isEmpty())
    {
        emit deselected(imageInfos(deselectedItems.indexes()));
    }
}

Album* ImageCategorizedView::albumAt(const QPoint& pos) const
{
    if (imageFilterModel()->imageSortSettings().categorizationMode == ItemSortSettings::CategoryByAlbum)
    {
        QModelIndex categoryIndex = indexForCategoryAt(pos);

        if (categoryIndex.isValid())
        {
            int albumId = categoryIndex.data(ItemFilterModel::CategoryAlbumIdRole).toInt();
            return AlbumManager::instance()->findPAlbum(albumId);
        }
    }

    return currentAlbum();
}

void ImageCategorizedView::activated(const ItemInfo&, Qt::KeyboardModifiers)
{
    // implemented in subclass
}

void ImageCategorizedView::showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)
{
    showContextMenuOnInfo(event, imageInfo(index));
}

void ImageCategorizedView::showContextMenuOnInfo(QContextMenuEvent*, const ItemInfo&)
{
    // implemented in subclass
}

void ImageCategorizedView::paintEvent(QPaintEvent* e)
{
    // We want the thumbnails to be loaded in order.
    ItemThumbnailModel* const thumbModel = imageThumbnailModel();

    if (thumbModel)
    {
        QModelIndexList indexesToThumbnail = imageFilterModel()->mapListToSource(categorizedIndexesIn(viewport()->rect()));
        d->delegate->prepareThumbnails(thumbModel, indexesToThumbnail);
    }

    ItemViewCategorized::paintEvent(e);
}

QItemSelectionModel* ImageCategorizedView::getSelectionModel() const
{
    return selectionModel();
}

AbstractItemDragDropHandler* ImageCategorizedView::dragDropHandler() const
{
    return d->model->dragDropHandler();
}

void ImageCategorizedView::slotIccSettingsChanged(const ICCSettingsContainer&, const ICCSettingsContainer&)
{
    viewport()->update();
}

} // namespace Digikam
