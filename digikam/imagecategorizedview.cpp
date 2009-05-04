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
#include <QScrollBar>
#include <QStyle>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databasefields.h"
#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "imagecategorydrawer.h"
#include "imagedelegate.h"
#include "imagedelegateoverlay.h"
#include "imagemodeldragdrophandler.h"
#include "imagethumbnailmodel.h"
#include "imageselectionoverlay.h"
#include "itemviewtooltip.h"
#include "loadingcacheinterface.h"
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
        model              = 0;
        filterModel        = 0;
        delegate           = 0;
        categoryDrawer     = 0;
        toolTip            = 0;
        scrollToItemId     = 0;
        currentMouseEvent  = 0;
        showToolTip        = false;
    }

    ImageAlbumModel         *model;
    ImageAlbumFilterModel   *filterModel;

    ImageDelegate           *delegate;
    ImageCategoryDrawer     *categoryDrawer;
    ImageItemViewToolTip    *toolTip;
    bool                     showToolTip;

    ThumbnailSize            thumbnailSize;
    qlonglong                scrollToItemId;

    QMouseEvent             *currentMouseEvent;
    QList<QModelIndex>       indexesToThumbnail;
};

// -------------------------------------------------------------------------------

ImageCategorizedView::ImageCategorizedView(QWidget *parent)
                    : KCategorizedView(parent), d(new ImageCategorizedViewPriv)
{
    setViewMode(QListView::IconMode);
    setLayoutDirection(Qt::LeftToRight);
    setFlow(QListView::LeftToRight);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setWrapping(true);
    // important optimization for layouting
    setUniformItemSizes(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setDragEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);

    d->model = new ImageAlbumModel(this);
    d->filterModel = new ImageAlbumFilterModel(this);
    d->filterModel->setSourceImageModel(d->model);

    d->filterModel->setSortOrder(ImageFilterModel::SortByFileName);
    d->filterModel->setCategorizationMode(ImageFilterModel::CategoryByAlbum);
    d->filterModel->sort(0); // an initial sorting is necessary

    // set flags that we want to get dataChanged() signals for
    DatabaseFields::Set watchFlags;
    watchFlags |= DatabaseFields::Name | DatabaseFields::FileSize | DatabaseFields::ModificationDate;
    watchFlags |= DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::Orientation |
                  DatabaseFields::Width | DatabaseFields::Height;
    watchFlags |= DatabaseFields::Comment;
    d->model->setWatchFlags(watchFlags);

    d->delegate = new ImageDelegate(this);
    d->delegate->setSpacing(10);
    setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());

    d->toolTip = new ImageItemViewToolTip(this);
    d->showToolTip = AlbumSettings::instance()->getShowToolTips();

    setModel(d->filterModel);

    connect(d->model, SIGNAL(imageInfosAdded(const QList<ImageInfo> &)),
            this, SLOT(slotImageInfosAdded()));

    connect(d->delegate, SIGNAL(gridSizeChanged(const QSize &)),
            this, SLOT(slotGridSizeChanged(const QSize &)));

    connect(d->delegate, SIGNAL(waitingForThumbnail(const QModelIndex &)),
            this, SLOT(slotDelegateWaitsForThumbnail(const QModelIndex &)));

    connect(this, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(slotActivated(const QModelIndex &)));

    connect(this, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(slotClicked(const QModelIndex &)));

    connect(this, SIGNAL(entered(const QModelIndex &)),
            this, SLOT(slotEntered(const QModelIndex &)));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(const QString &)));

    updateDelegateSizes();
    addSelectionOverlay();

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

Album *ImageCategorizedView::currentAlbum() const
{
    return d->model->currentAlbum();
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

void ImageCategorizedView::openAlbum(Album *album)
{
    d->model->openAlbum(album);
}

ThumbnailSize ImageCategorizedView::thumbnailSize() const
{
    return d->model->thumbnailSize();
}

void ImageCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(ThumbnailSize(size));
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

void ImageCategorizedView::addOverlay(ImageDelegateOverlay *overlay)
{
    overlay->setView(this);
    d->delegate->installOverlay(overlay);
}

void ImageCategorizedView::removeOverlay(ImageDelegateOverlay *overlay)
{
    d->delegate->removeOverlay(overlay);
    overlay->setView(0);
}

void ImageCategorizedView::addSelectionOverlay()
{
    addOverlay(new ImageSelectionOverlay(this));
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
    d->showToolTip = AlbumSettings::instance()->getShowToolTips();
    viewport()->update();
}

void ImageCategorizedView::slotGridSizeChanged(const QSize &gridSize)
{
    setGridSize(gridSize);

    horizontalScrollBar()->setSingleStep(gridSize.width() / 20);
    verticalScrollBar()->setSingleStep(gridSize.height() / 20);
}

void ImageCategorizedView::updateDelegateSizes()
{
    QStyleOptionViewItem option = viewOptions();
    /*int frameAroundContents = 0;
    if (style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents)) {
        frameAroundContents = style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;
    }
    const int contentWidth = viewport()->width() - 1
                                - frameAroundContents
                                - style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, verticalScrollBar());
    const int contentHeight = viewport()->height() - 1
                                - frameAroundContents
                                - style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, horizontalScrollBar());
    option.rect = QRect(0, 0, contentWidth, contentHeight);
    */
    option.rect = QRect(QPoint(0,0), viewport()->size());
    d->delegate->setDefaultViewOptions(option);
}

void ImageCategorizedView::slotFileChanged(const QString &filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);
    if (index.isValid())
        update(index);
}

void ImageCategorizedView::slotActivated(const QModelIndex &index)
{
    if (d->currentMouseEvent)
    {
        // if the activation is caused by mouse click (not keyboard)
        // we need to check the hot area
        if (!d->delegate->acceptsActivation(d->currentMouseEvent->pos(), visualRect(index), index))
            return;
    }

    ImageInfo info = d->filterModel->imageInfo(index);
    if (!info.isNull())
        activated(info);
}

void ImageCategorizedView::slotClicked(const QModelIndex &index)
{
    if (d->currentMouseEvent)
        emit clicked(d->currentMouseEvent, index);
}

void ImageCategorizedView::slotEntered(const QModelIndex &index)
{
    if (d->currentMouseEvent)
        emit entered(d->currentMouseEvent, index);
}

void ImageCategorizedView::reset()
{
    KCategorizedView::reset();

    emit selectionChanged();
    emit selectionCleared();
}

void ImageCategorizedView::currentChanged(const QModelIndex &index, const QModelIndex &previous)
{
    KCategorizedView::currentChanged(index, previous);

    emit currentChanged(d->filterModel->imageInfo(index));
}

void ImageCategorizedView::selectionChanged(const QItemSelection &selectedItems, const QItemSelection &deselectedItems)
{
    KCategorizedView::selectionChanged(selectedItems, deselectedItems);

    emit selectionChanged();
    if (!selectedItems.isEmpty())
        emit selected(d->filterModel->imageInfos(selectedItems.indexes()));
    if (!deselectedItems.isEmpty())
        emit deselected(d->filterModel->imageInfos(deselectedItems.indexes()));
    if (!selectionModel()->hasSelection())
        emit selectionCleared();
}

Album *ImageCategorizedView::albumAt(const QPoint &pos)
{
    if (d->filterModel->categorizationMode() == ImageFilterModel::CategoryByAlbum)
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

QModelIndex ImageCategorizedView::indexForCategoryAt(const QPoint &pos) const
{
    QModelIndex index = indexAt(pos);
    if (index.isValid())
        return index;

    QRect viewportRect = viewport()->rect();
    if (!viewportRect.contains(pos))
        return QModelIndex();

    // There is no indexesInRect() method. And no categoryRect(). We have to probe.
    // Assumption: we test at the same y pos, going right from the left edge;
    // then we go down in the middle of the first column until the bottom of the viewport is reached.

    const int horizontalStep = 10;
    const int rightEnd = qMin(gridSize().width() + 2*spacing() + 2*horizontalStep, viewportRect.width());
    int y = pos.y();
    for (int x=0; x<rightEnd; x+=horizontalStep)
    {
        index = indexAt(QPoint(x, y));
        if (index.isValid())
            return index;
    }

    const int verticalStep = 10;
    const int bottomEnd = qMin(d->categoryDrawer->maximumHeight() + 2*gridSize().height() + 4*spacing(), viewportRect.height());
    const int x = (gridSize().width() + 2*spacing()) / 2; // middle of first column
    for (; y<bottomEnd; y += verticalStep)
    {
        index = indexAt(QPoint(x, y));
        if (index.isValid())
            return index;
    }

    return QModelIndex();
}

void ImageCategorizedView::activated(const ImageInfo &)
{
    // implemented in subclass
}

void ImageCategorizedView::showContextMenu(QContextMenuEvent *, const ImageInfo &)
{
    // implemented in subclass
}

void ImageCategorizedView::showContextMenu(QContextMenuEvent *)
{
    // implemented in subclass
}

void ImageCategorizedView::copy()
{
    // implemented in subclass
}

void ImageCategorizedView::paste()
{
    // implemented in subclass
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
        showContextMenu(event);
}

void ImageCategorizedView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());

    // Clear selection on click on empty area. Standard behavior, but not done by QAbstractItemView for some reason.
    if (!index.isValid()) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        const Qt::MouseButton button = event->button();
        const bool rightButtonPressed = button & Qt::RightButton;
        const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
        const bool controlKeyPressed = modifiers & Qt::ControlModifier;
        if (!index.isValid() && !rightButtonPressed && !shiftKeyPressed && !controlKeyPressed)
            clearSelection();
    }

    // store event for entered(), clicked(), activated() signal handlers
    d->currentMouseEvent = event;
    KCategorizedView::mousePressEvent(event);
    d->currentMouseEvent = 0;
}

void ImageCategorizedView::mouseReleaseEvent(QMouseEvent *event)
{
    d->currentMouseEvent = event;
    KCategorizedView::mouseReleaseEvent(event);
    d->currentMouseEvent = 0;
}

void ImageCategorizedView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    QRect indexVisualRect;
    if (index.isValid())
    {
        indexVisualRect = visualRect(index);
        if (KGlobalSettings::changeCursorOverIcon() &&
            d->delegate->acceptsActivation(event->pos(), indexVisualRect, index))
        {
            setCursor(Qt::PointingHandCursor);
        }
    }
    else
    {
        unsetCursor();
    }

    d->currentMouseEvent = event;
    KCategorizedView::mouseMoveEvent(event);
    d->currentMouseEvent = 0;

    d->delegate->mouseMoved(event, indexVisualRect, index);
}

void ImageCategorizedView::wheelEvent(QWheelEvent* event)
{
    // KCategorizedView updates the single step at some occasions in a private methody
    horizontalScrollBar()->setSingleStep(d->delegate->gridSize().height() / 10);
    verticalScrollBar()->setSingleStep(d->delegate->gridSize().width() / 10);

    KCategorizedView::wheelEvent(event);
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

    emit keyPressed(event);
}

bool modelIndexByRowLessThan(const QModelIndex &i1, const QModelIndex &i2)
{
    return i1.row() < i2.row();
}

void ImageCategorizedView::slotDelegateWaitsForThumbnail(const QModelIndex &index)
{
    d->indexesToThumbnail << index;
}

void ImageCategorizedView::paintEvent(QPaintEvent *e)
{
    // We want the thumbnails to be loaded in order.
    // We cannot easily know which indexes are repainted, so we have to listen
    // to our delegate telling us for which thumbnails he waits.
    // After that we reorder them. See slotDelegateWaitsForThumbnail().
    d->indexesToThumbnail.clear();

    KCategorizedView::paintEvent(e);

    if (!d->indexesToThumbnail.isEmpty())
    {
        qSort(d->indexesToThumbnail.begin(), d->indexesToThumbnail.end(), modelIndexByRowLessThan);
        d->filterModel->prepareThumbnails(d->indexesToThumbnail);
        d->indexesToThumbnail.clear();
    }
}

void ImageCategorizedView::resizeEvent(QResizeEvent *e)
{
    KCategorizedView::resizeEvent(e);
    updateDelegateSizes();
}

bool ImageCategorizedView::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::FontChange:
        {
            updateDelegateSizes();
            break;
        }
        case QEvent::ToolTip:
        {
            if (!d->showToolTip)
                return true;
            QHelpEvent *he = static_cast<QHelpEvent*>(event);
            const QModelIndex index = indexAt(he->pos());
            if (!index.isValid())
                break;
            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(index);
            option.state |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
            QRect innerRect;
            if (d->delegate->acceptsToolTip(he->pos(), option.rect, index, &innerRect))
            {
                if (!innerRect.isNull())
                    option.rect = innerRect;
                d->toolTip->show(he, option, index);
            }
            return true;
        }
        default:
            break;
    }
    return KCategorizedView::viewportEvent(event);
}

void ImageCategorizedView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.count() > 0) {
        QMimeData *data = d->filterModel->mimeData(indexes);
        if (!data)
            return;
        QStyleOptionViewItem option = viewOptions();
        option.rect = viewport()->rect();
        QPixmap pixmap = d->delegate->pixmapForDrag(option, indexes);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions);
    }
}

void ImageCategorizedView::dragMoveEvent(QDragMoveEvent *e)
{
    KCategorizedView::dragMoveEvent(e);
    ImageModelDragDropHandler *handler = d->model->dragDropHandler();
    if (handler)
    {
        QModelIndex index = indexAt(e->pos());
        Qt::DropAction action = handler->accepts(e, d->filterModel->mapToSource(index));
        if (action == Qt::IgnoreAction)
            e->ignore();
        else
        {
            e->setDropAction(action);
            e->accept();
        }
    }
}

void ImageCategorizedView::dropEvent(QDropEvent *e)
{
    KCategorizedView::dropEvent(e);
    ImageModelDragDropHandler *handler = d->model->dragDropHandler();
    if (handler)
    {
        QModelIndex index = indexAt(e->pos());
        if (handler->dropEvent(this, e, d->filterModel->mapToSource(index)))
            e->accept();
    }
}

} // namespace Digikam
