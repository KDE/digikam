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
        toolTip            = 0;
        scrollToItemId     = 0;
        currentMouseEvent  = 0;
        showToolTip        = false;
        ensureOneSelectedItem     = false;
        ensureInitialSelectedItem = false;
    }

    ImageAlbumModel         *model;
    ImageAlbumFilterModel   *filterModel;

    ImageDelegate           *delegate;
    ImageItemViewToolTip    *toolTip;
    bool                     showToolTip;

    ThumbnailSize            thumbnailSize;
    qlonglong                scrollToItemId;

    QMouseEvent             *currentMouseEvent;
    bool                     ensureOneSelectedItem;
    bool                     ensureInitialSelectedItem;
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
    // disable "feature" from KCategorizedView
    setDrawDraggedItems(false);

    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setDragEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewport()->setAcceptDrops(true);
    setMouseTracking(true);

    d->model = new ImageAlbumModel(this);
    d->filterModel = new ImageAlbumFilterModel(this);
    d->filterModel->setSourceImageModel(d->model);

    d->filterModel->setSortRole(ImageSortSettings::SortByFileName);
    d->filterModel->setCategorizationMode(ImageSortSettings::CategoryByAlbum);
    d->filterModel->sort(0); // an initial sorting is necessary

    // set flags that we want to get dataChanged() signals for
    d->model->setWatchFlags(d->filterModel->suggestedWatchFlags());

    d->delegate = new ImageDelegate(this);
    d->delegate->setSpacing(10);
    setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());

    d->toolTip = new ImageItemViewToolTip(this);

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(imageInfosAdded(const QList<ImageInfo> &)),
            this, SLOT(slotImageInfosAdded()));

    connect(d->delegate, SIGNAL(gridSizeChanged(const QSize &)),
            this, SLOT(slotGridSizeChanged(const QSize &)));

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
    d->delegate->removeAllOverlays();
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

KUrl ImageCategorizedView::currentUrl() const
{
    return currentInfo().fileUrl();
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
    foreach (const QModelIndex& index, indexes)
    {
        ImageInfo info = d->filterModel->imageInfo(index);
        if (index == current)
            infos.prepend(info);
        else
            infos.append(info);
    }
    return infos;
}

int ImageCategorizedView::numberOfSelectedIndexes() const
{
    return selectedIndexes().size();
}

QList<ImageInfo> ImageCategorizedView::imageInfos() const
{
    return d->filterModel->imageInfosSorted();
}

KUrl::List ImageCategorizedView::urls() const
{
    QList<ImageInfo> infos = imageInfos();
    KUrl::List urls;
    foreach (const ImageInfo& info, infos)
        urls << info.fileUrl();
    return urls;
}

KUrl::List ImageCategorizedView::selectedUrls() const
{
    QList<ImageInfo> infos = selectedImageInfos();
    KUrl::List urls;
    foreach (const ImageInfo& info, infos)
        urls << info.fileUrl();
    return urls;
}

ImageInfo ImageCategorizedView::nextInOrder(const ImageInfo &startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForImageInfo(startingPoint);
    if (!index.isValid())
        return ImageInfo();
    return d->filterModel->imageInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

void ImageCategorizedView::toFirstIndex()
{
    QModelIndex index = moveCursor(MoveHome, Qt::NoModifier);
    setCurrentIndex(index);
    scrollToTop();
}

void ImageCategorizedView::toLastIndex()
{
    QModelIndex index = moveCursor(MoveEnd, Qt::NoModifier);
    setCurrentIndex(index);
    scrollToBottom();
}

void ImageCategorizedView::toNextIndex()
{
    toIndex(moveCursor(MoveNext, Qt::NoModifier));
}

void ImageCategorizedView::toPreviousIndex()
{
    toIndex(moveCursor(MovePrevious, Qt::NoModifier));
}

void ImageCategorizedView::toIndex(const KUrl& url)
{
    QModelIndex index = d->model->indexForPath(url.path());
    toIndex(d->filterModel->mapFromSource(index));
}

void ImageCategorizedView::toIndex(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    setCurrentIndex(index);
    scrollTo(index);
}

void ImageCategorizedView::invertSelection()
{
    const QModelIndex topLeft = d->filterModel->index(0, 0);
    const QModelIndex bottomRight = d->filterModel->index(d->filterModel->rowCount() - 1, 0);

    const QItemSelection selection(topLeft, bottomRight);
    selectionModel()->select(selection, QItemSelectionModel::Toggle);
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

void ImageCategorizedView::setThumbnailSize(const ThumbnailSize& size)
{
    d->model->setThumbnailSize(size);
    d->delegate->setThumbnailSize(size);
    //viewport()->update();
}

void ImageCategorizedView::setToolTipEnabled(bool enable)
{
    d->showToolTip = enable;
}

bool ImageCategorizedView::isToolTipEnabled() const
{
    return d->showToolTip;
}

void ImageCategorizedView::scrollToWhenAvailable(qlonglong imageId)
{
    d->scrollToItemId = imageId;
}

void ImageCategorizedView::setCurrentUrl(const KUrl& url)
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
    viewport()->update();
}

void ImageCategorizedView::slotGridSizeChanged(const QSize& gridSize)
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

void ImageCategorizedView::slotFileChanged(const QString& filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);
    if (index.isValid())
        update(index);
}

void ImageCategorizedView::slotActivated(const QModelIndex& index)
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

void ImageCategorizedView::slotClicked(const QModelIndex& index)
{
    if (d->currentMouseEvent)
        emit clicked(d->currentMouseEvent, index);
}

void ImageCategorizedView::slotEntered(const QModelIndex& index)
{
    if (d->currentMouseEvent)
        emit entered(d->currentMouseEvent, index);
}

void ImageCategorizedView::reset()
{
    KCategorizedView::reset();

    emit selectionChanged();
    emit selectionCleared();

    d->ensureInitialSelectedItem = true;
}

void ImageCategorizedView::currentChanged(const QModelIndex& index, const QModelIndex& previous)
{
    KCategorizedView::currentChanged(index, previous);

    emit currentChanged(d->filterModel->imageInfo(index));
}

void ImageCategorizedView::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    KCategorizedView::selectionChanged(selectedItems, deselectedItems);

    emit selectionChanged();
    if (!selectedItems.isEmpty())
        emit selected(d->filterModel->imageInfos(selectedItems.indexes()));
    if (!deselectedItems.isEmpty())
        emit deselected(d->filterModel->imageInfos(deselectedItems.indexes()));
    if (!selectionModel()->hasSelection())
        emit selectionCleared();
    userInteraction();
}

void ImageCategorizedView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    KCategorizedView::rowsInserted(parent, start, end);
    if (start == 0)
        ensureSelectionAfterChanges();
}

void ImageCategorizedView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    KCategorizedView::rowsAboutToBeRemoved(parent, start, end);

    // Ensure one selected item
    int totalToRemove = end - start + 1;
    if (selectionModel()->hasSelection() && model()->rowCount(parent) > totalToRemove)
    {
        // find out which selected indexes are left after rows are removed
        QItemSelection selected = selectionModel()->selection();
        QItemSelection removed(model()->index(start, 0), model()->index(end, 0));
        selected.merge(removed, QItemSelectionModel::Deselect);
        if (selected.isEmpty())
        {
            selectionModel()->select(model()->index(start > 0 ? start - 1 : end + 1, 0),
                                     QItemSelectionModel::SelectCurrent);
        }
    }
}

void ImageCategorizedView::layoutAboutToBeChanged()
{
    d->ensureOneSelectedItem = selectionModel()->hasSelection();
}

void ImageCategorizedView::layoutWasChanged()
{
    // connected queued to layoutChanged()
    ensureSelectionAfterChanges();
    d->ensureOneSelectedItem = false;
}

void ImageCategorizedView::userInteraction()
{
    // as soon as the user did anything affecting selection, we don't interfer anymore
    d->ensureInitialSelectedItem = false;
}

void ImageCategorizedView::ensureSelectionAfterChanges()
{
    if (d->ensureInitialSelectedItem && model()->rowCount())
    {
        // Ensure the item (0,0) is selected, if the model was reset previously
        // and the user did not change the selection since reset.
        // Caveat: Item at (0,0) may have changed.
        bool hadInitial = d->ensureInitialSelectedItem;
        d->ensureInitialSelectedItem = false;

        QModelIndex index = model()->index(0,0);
        if (index.isValid())
        {
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Clear);
            // we want ensureInitial set to false if and only if the selection
            // is done from any other place than the previous line (i.e., by user action)
            // Effect: we select whatever is the current index(0,0)
            if (hadInitial)
                d->ensureInitialSelectedItem = true;
        }
    }
    else if (d->ensureOneSelectedItem)
    {
        // ensure we have a selection if there was one before
        d->ensureOneSelectedItem = false;
        if (model()->rowCount() && selectionModel()->selection().isEmpty())
        {
            QModelIndex index = currentIndex();
            if (!index.isValid())
                index = model()->index(0,0);
            if (index.isValid())
                selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
        }
    }
}

Album *ImageCategorizedView::albumAt(const QPoint& pos)
{
    if (d->filterModel->imageSortSettings().categorizationMode == ImageSortSettings::CategoryByAlbum)
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

/*
//Remove when API changes are accepted for KCategorizedView
QModelIndex ImageCategorizedView::indexForCategoryAt(const QPoint& pos) const
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
    const int bottomEnd = qMin(d->delegate->categoryDrawer()->maximumHeight() + 2*gridSize().height() + 4*spacing(), viewportRect.height());
    const int x = (gridSize().width() + 2*spacing()) / 2; // middle of first column
    for (; y<bottomEnd; y += verticalStep)
    {
        index = indexAt(QPoint(x, y));
        if (index.isValid())
            return index;
    }

    return QModelIndex();
}
*/

QModelIndex ImageCategorizedView::indexForCategoryAt(const QPoint& pos) const
{
    return categoryAt(pos);
}

QModelIndex ImageCategorizedView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = currentIndex();
    if (!current.isValid())
        return KCategorizedView::moveCursor(cursorAction, modifiers);

    // We want a simple wrapping navigation.
    // Default behavior we dont want: right/left does never change row; Next/Previous is equivalent to Down/Up
    switch (cursorAction)
    {
        case MoveNext:
        case MoveRight:
        {
            QModelIndex next = d->filterModel->index(current.row() + 1, 0);
            if (next.isValid())
                return next;
            else
                return current;
            break;
        }
        case MovePrevious:
        case MoveLeft:
        {
            QModelIndex previous = d->filterModel->index(current.row() - 1, 0);
            if (previous.isValid())
                return previous;
            else
                return current;
            break;
        }
        default:
            break;
    }

    return KCategorizedView::moveCursor(cursorAction, modifiers);
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
    userInteraction();
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
    userInteraction();
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
    userInteraction();
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

    if (event->modifiers() & Qt::ControlModifier) {
        const int delta = event->delta();
        if (delta > 0)
            emit zoomInStep();
        else if (delta < 0)
            emit zoomOutStep();
        event->accept();
    }

    KCategorizedView::wheelEvent(event);
}

void ImageCategorizedView::keyPressEvent(QKeyEvent *event)
{
    userInteraction();
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

/*
//Remove when API changes are accepted for KCategorizedView
//Also remove code from delegate.

    connect(d->delegate, SIGNAL(waitingForThumbnail(const QModelIndex &)),
            this, SLOT(slotDelegateWaitsForThumbnail(const QModelIndex &)));

bool modelIndexByRowLessThan(const QModelIndex& i1, const QModelIndex& i2)
{
    return i1.row() < i2.row();
}

void ImageCategorizedView::slotDelegateWaitsForThumbnail(const QModelIndex& index)
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
*/

void ImageCategorizedView::paintEvent(QPaintEvent *e)
{
    // We want the thumbnails to be loaded in order.
    QModelIndexList indexesToThumbnail = categorizedIndexesIn(viewport()->rect());
    d->filterModel->prepareThumbnails(indexesToThumbnail);

    KCategorizedView::paintEvent(e);
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
