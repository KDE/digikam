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
#include "imagealbumfiltermodel.h"
#include "imagecategorydrawer.h"
#include "imagedelegate.h"
#include "imagedelegateoverlay.h"
#include "imagethumbnailmodel.h"
#include "imageselectionoverlay.h"
#include "itemviewtooltip.h"
#include "loadingcacheinterface.h"
#include "thumbnailloadthread.h"
#include "tooltipfiller.h"
#include "digikamimagefacedelegate.h"

namespace Digikam
{

class ImageItemViewToolTip : public ItemViewToolTip
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
        ImageInfo info = ImageModel::retrieveImageInfo(currentIndex());
        return ToolTipFiller::imageInfoTipContents(info);
    }
};

// -------------------------------------------------------------------------------

class ImageCategorizedView::Private
{
public:

    Private() :
        model(0),
        filterModel(0),
        delegate(0),
        showToolTip(false),
        scrollToItemId(0),
        delayedEnterTimer(0),
        currentMouseEvent(0)
    {
    }

    ImageModel*           model;
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
    : ItemViewCategorized(parent), d(new Private)
{
    setToolTip(new ImageItemViewToolTip(this));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(QString)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged(ICCSettingsContainer, ICCSettingsContainer)),
            this, SLOT(slotIccSettingsChanged(ICCSettingsContainer, ICCSettingsContainer)));

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
    ImageAlbumFilterModel* filterModel = new ImageAlbumFilterModel(this);

    filterModel->setSourceImageModel(model);

    filterModel->setSortRole(ImageSortSettings::SortByFileName);
    filterModel->setCategorizationMode(ImageSortSettings::CategoryByAlbum);
    filterModel->sort(0); // an initial sorting is necessary

    // set flags that we want to get dataChanged() signals for
    model->setWatchFlags(filterModel->suggestedWatchFlags());

    setModels(model, filterModel);
}

void ImageCategorizedView::setModels(ImageModel* model, ImageSortFilterModel* filterModel)
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
        disconnect(d->model, SIGNAL(imageInfosAdded(QList<ImageInfo>)),
                   this, SLOT(slotImageInfosAdded()));
    }

    d->model       = model;
    d->filterModel = filterModel;

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(imageInfosAdded(QList<ImageInfo>)),
            this, SLOT(slotImageInfosAdded()));

    emit modelChanged();

    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(true);
    }
}

ImageModel* ImageCategorizedView::imageModel() const
{
    return d->model;
}

ImageSortFilterModel* ImageCategorizedView::imageSortFilterModel() const
{
    return d->filterModel;
}

ImageFilterModel* ImageCategorizedView::imageFilterModel() const
{
    return d->filterModel->imageFilterModel();
}

ImageThumbnailModel* ImageCategorizedView::imageThumbnailModel() const
{
    return qobject_cast<ImageThumbnailModel*>(d->model);
}

ImageAlbumModel* ImageCategorizedView::imageAlbumModel() const
{
    return qobject_cast<ImageAlbumModel*>(d->model);
}

ImageAlbumFilterModel* ImageCategorizedView::imageAlbumFilterModel() const
{
    return qobject_cast<ImageAlbumFilterModel*>(d->filterModel->imageFilterModel());
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

ImageInfo ImageCategorizedView::currentInfo() const
{
    return imageInfo(currentIndex());
}

QUrl ImageCategorizedView::currentUrl() const
{
    return currentInfo().fileUrl();
}

ImageInfo ImageCategorizedView::imageInfo(const QModelIndex& index) const
{
    return d->filterModel->imageInfo(index);
}

ImageInfoList ImageCategorizedView::imageInfos(const QList<QModelIndex>& indexes,
                                               ApplicationSettings::OperationType type) const
{
    return imageInfos(indexes, needGroupResolving(type, indexes));
}

ImageInfoList ImageCategorizedView::imageInfos(const QList<QModelIndex>& indexes, bool grouping) const
{
    if (grouping) {
        return resolveGrouping(indexes);
    }
    return d->filterModel->imageInfos(indexes);
}

ImageInfoList ImageCategorizedView::selectedImageInfos(bool grouping) const
{
    return imageInfos(selectedIndexes(), grouping);
}

ImageInfoList ImageCategorizedView::selectedImageInfos(
        ApplicationSettings::OperationType type) const
{
    return selectedImageInfos(needGroupResolving(type));
}

ImageInfoList ImageCategorizedView::selectedImageInfosCurrentFirst(bool grouping) const
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

    if (grouping) {
        return resolveGrouping(indexes);
    }
    return imageInfos(indexes);
}

ImageInfoList ImageCategorizedView::allImageInfos(bool grouping) const
{
    if (grouping) {
        return resolveGrouping(d->filterModel->imageInfosSorted());
    }
    return d->filterModel->imageInfosSorted();
}

QList<QUrl> ImageCategorizedView::allUrls(bool grouping) const
{
    ImageInfoList infos = allImageInfos(grouping);
    QList<QUrl>   urls;

    foreach(const ImageInfo& info, infos)
    {
        urls << info.fileUrl();
    }

    return urls;
}

bool ImageCategorizedView::needGroupResolving(ApplicationSettings::OperationType type, bool all) const
{
    if (all)
    {
        return needGroupResolving(type, allImageInfos());
    }

    return needGroupResolving(type, selectedIndexes());
}

QList<QUrl> ImageCategorizedView::selectedUrls(bool grouping) const
{
    ImageInfoList infos = selectedImageInfos(grouping);
    QList<QUrl>   urls;

    foreach(const ImageInfo& info, infos)
    {
        urls << info.fileUrl();
    }

    return urls;
}

QList<QUrl> ImageCategorizedView::selectedUrls(ApplicationSettings::OperationType type) const
{
    return selectedUrls(needGroupResolving(type));
}

void ImageCategorizedView::toIndex(const QUrl& url)
{
    ItemViewCategorized::toIndex(d->filterModel->indexForPath(url.toLocalFile()));
}

QModelIndex ImageCategorizedView::indexForInfo(const ImageInfo& info) const
{
    return d->filterModel->indexForImageInfo(info);
}

ImageInfo ImageCategorizedView::nextInOrder(const ImageInfo& startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForImageInfo(startingPoint);

    if (!index.isValid())
    {
        return ImageInfo();
    }

    return imageInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

QModelIndex ImageCategorizedView::nextIndexHint(const QModelIndex& anchor, const QItemSelectionRange& removed) const
{
    QModelIndex hint = ItemViewCategorized::nextIndexHint(anchor, removed);
    ImageInfo info   = imageInfo(anchor);

    //qCDebug(DIGIKAM_GENERAL_LOG) << "Having initial hint" << hint << "for" << anchor << d->model->numberOfIndexesForImageInfo(info);

    // Fixes a special case of multiple (face) entries for the same image.
    // If one is removed, any entry of the same image shall be preferred.
    if (d->model->numberOfIndexesForImageInfo(info) > 1)
    {
        // The hint is for a different info, but we may have a hint for the same info
        if (info != imageInfo(hint))
        {
            int minDiff                            = d->filterModel->rowCount();
            QList<QModelIndex> indexesForImageInfo = d->filterModel->mapListFromSource(d->model->indexesForImageInfo(info));

            foreach(const QModelIndex& index, indexesForImageInfo)
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

void ImageCategorizedView::openAlbum(QList<Album*> albums)
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
    ImageThumbnailModel* const thumbModel = imageThumbnailModel();
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

void ImageCategorizedView::setCurrentInfo(const ImageInfo& info)
{
    QModelIndex index = d->filterModel->indexForImageInfo(info);
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

void ImageCategorizedView::setSelectedImageInfos(const QList<ImageInfo>& infos)
{
    QItemSelection mySelection;

    foreach(const ImageInfo& info, infos)
    {
        QModelIndex index = d->filterModel->indexForImageInfo(info);
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ImageCategorizedView::hintAt(const ImageInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    QModelIndex index = d->filterModel->indexForImageInfo(info);

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

void ImageCategorizedView::slotImageInfosAdded()
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
    ImageInfo info = imageInfo(index);

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
    if (imageFilterModel()->imageSortSettings().categorizationMode == ImageSortSettings::CategoryByAlbum)
    {
        QModelIndex categoryIndex = indexForCategoryAt(pos);

        if (categoryIndex.isValid())
        {
            int albumId = categoryIndex.data(ImageFilterModel::CategoryAlbumIdRole).toInt();
            return AlbumManager::instance()->findPAlbum(albumId);
        }
    }

    return currentAlbum();
}

void ImageCategorizedView::activated(const ImageInfo&, Qt::KeyboardModifiers)
{
    // implemented in subclass
}

void ImageCategorizedView::showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)
{
    showContextMenuOnInfo(event, imageInfo(index));
}

void ImageCategorizedView::showContextMenuOnInfo(QContextMenuEvent*, const ImageInfo&)
{
    // implemented in subclass
}

ImageInfoList ImageCategorizedView::resolveGrouping(const QModelIndexList& indexes) const
{
    return resolveGrouping(imageInfos(indexes));
}

ImageInfoList ImageCategorizedView::resolveGrouping(const ImageInfoList& infos) const
{
    ImageInfoList outInfos;

    foreach(const ImageInfo& info, infos)
    {
        outInfos << info;

        if (info.hasGroupedImages() && !imageFilterModel()->isGroupOpen(info.id()))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

bool ImageCategorizedView::needGroupResolving(ApplicationSettings::OperationType type,
                                              const QList<QModelIndex>& indexes) const
{
    return needGroupResolving(type, imageInfos(indexes));
}

bool ImageCategorizedView::needGroupResolving(ApplicationSettings::OperationType type,
                                              const ImageInfoList& infos) const
{
    ApplicationSettings::ApplyToEntireGroup applyAll =
            ApplicationSettings::instance()->getGroupingOperateOnAll(type);

    if (applyAll == ApplicationSettings::No)
    {
        return false;
    }
    else if (applyAll == ApplicationSettings::Yes)
    {
        return true;
    }

    foreach(const ImageInfo& info, infos)
    {
        if (info.hasGroupedImages() && !imageFilterModel()->isGroupOpen(info.id()))
        {
            // Ask whether should be performed on all and return info if no
            return ApplicationSettings::instance()->askGroupingOperateOnAll(type);
        }
    }

    return false;
}

void ImageCategorizedView::paintEvent(QPaintEvent* e)
{
    // We want the thumbnails to be loaded in order.
    ImageThumbnailModel* const thumbModel = imageThumbnailModel();

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
