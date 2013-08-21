#include "showfotocategorizedview.h"

#include "QTimer"
#include "kdebug.h"

#include "loadingcacheinterface.h"
#include "imageselectionoverlay.h"
#include "showfotoitemsortsettings.h"
#include "itemviewtooltip.h"
#include "showfotodelegate.h"
#include "showfototooltipfiller.h"
#include "thumbnailloadthread.h"
#include "showfotosettings.h"
#include "imageselectionoverlay.h"

using namespace Digikam;

namespace ShowFoto {

class ShowfotoItemViewToolTip : public ItemViewToolTip
{
public:

    explicit ShowfotoItemViewToolTip(ShowfotoCategorizedView* const view)
        : ItemViewToolTip(view)
    {
    }

    ShowfotoCategorizedView* view() const
    {
        return static_cast<ShowfotoCategorizedView*>(ItemViewToolTip::view());
    }

protected:

    virtual QString tipContents()
    {
        ShowfotoItemInfo info = ShowfotoImageModel::retrieveShowfotoItemInfo(currentIndex());
        return ShowfotoToolTipFiller::ShowfotoItemInfoTipContents(info);
    }
};

class ShowfotoCategorizedView::Private
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

    ShowfotoImageModel*      model;
    ShowfotoSortFilterModel* filterModel;

    ShowfotoDelegate*        delegate;
    bool                   showToolTip;

    qlonglong              scrollToItemId;

    QTimer*                delayedEnterTimer;

    QMouseEvent*           currentMouseEvent;
};

ShowfotoCategorizedView::ShowfotoCategorizedView(QWidget* const parent)
    : DCategorizedView(parent), d(new Private)
{
    setToolTip(new ShowfotoItemViewToolTip(this));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(QString)));

    d->delayedEnterTimer = new QTimer(this);
    d->delayedEnterTimer->setInterval(10);
    d->delayedEnterTimer->setSingleShot(true);

    connect(d->delayedEnterTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedEnter()));
}

ShowfotoCategorizedView::~ShowfotoCategorizedView()
{
    d->delegate->removeAllOverlays();
    delete d;
}

void ShowfotoCategorizedView::setModels(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel)
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
        disconnect(d->model, SIGNAL(itemInfosAdded(QList<ShowfotoItemInfo>)),
                   this, SLOT(slotShowfotoItemInfosAdded()));
    }

    d->model       = model;
    d->filterModel = filterModel;

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(itemInfosAdded(QList<ShowfotoItemInfo>)),
            this, SLOT(slotShowfotoItemInfosAdded()));

    emit modelChanged();

    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(true);
    }
}

ShowfotoImageModel* ShowfotoCategorizedView::showfotoImageModel() const
{
    return d->model;
}

ShowfotoSortFilterModel* ShowfotoCategorizedView::showfotoSortFilterModel() const
{
    return d->filterModel;
}

ShowfotoFilterModel* ShowfotoCategorizedView::showfotoFilterModel() const
{
    return d->filterModel->showfotoFilterModel();
}

ShowfotoThumbnailModel* ShowfotoCategorizedView::showfotoThumbnailModel() const
{
    return qobject_cast<ShowfotoThumbnailModel*>(d->model);
}

QSortFilterProxyModel* ShowfotoCategorizedView::filterModel() const
{
    return d->filterModel;
}

ShowfotoDelegate* ShowfotoCategorizedView::delegate() const
{
    return d->delegate;
}

void ShowfotoCategorizedView::setItemDelegate(ShowfotoDelegate* delegate)
{
    ThumbnailSize oldSize       = thumbnailSize();
    ShowfotoDelegate* oldDelegate = d->delegate;

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
    delegate->setThumbnailSize(oldSize);

    if (oldDelegate)
    {
        d->delegate->setSpacing(oldDelegate->spacing());
    }

    DCategorizedView::setItemDelegate(d->delegate);
    //setCategoryDrawer(d->delegate->categoryDrawer());
    updateDelegateSizes();

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
            this, SLOT(showIndexNotification(QModelIndex,QString)));

    connect(d->delegate, SIGNAL(hideNotification()),
            this, SLOT(hideIndexNotification()));
}

ShowfotoItemInfo ShowfotoCategorizedView::currentInfo() const
{
    return d->filterModel->showfotoItemInfo(currentIndex());
}

KUrl ShowfotoCategorizedView::currentUrl() const
{
    return currentInfo().url;
}

QList<ShowfotoItemInfo> ShowfotoCategorizedView::selectedShowfotoItemInfos() const
{
    return d->filterModel->showfotoItemInfos(selectedIndexes());
}

QList<ShowfotoItemInfo> ShowfotoCategorizedView::selectedShowfotoItemInfosCurrentFirst() const
{
    QList<QModelIndex> indexes = selectedIndexes();
    QModelIndex        current = currentIndex();
    QList<ShowfotoItemInfo>   infos;

    foreach(const QModelIndex& index, indexes)
    {
        ShowfotoItemInfo info = d->filterModel->showfotoItemInfo(index);

        if (index == current)
        {
            infos.prepend(info);
        }
        else
        {
            infos.append(info);
        }
    }

    return infos;
}

QList<ShowfotoItemInfo> ShowfotoCategorizedView::showfotoItemInfos() const
{
    return d->filterModel->showfotoItemInfosSorted();
}

KUrl::List ShowfotoCategorizedView::urls() const
{
    QList<ShowfotoItemInfo> infos = showfotoItemInfos();
    KUrl::List       urls;

    foreach(const ShowfotoItemInfo& info, infos)
    {
        urls << info.url;
    }

    return urls;
}

KUrl::List ShowfotoCategorizedView::selectedUrls() const
{
    QList<ShowfotoItemInfo> infos = selectedShowfotoItemInfos();
    KUrl::List       urls;

    foreach(const ShowfotoItemInfo& info, infos)
    {
        urls << info.url;
    }

    return urls;
}

void ShowfotoCategorizedView::toIndex(const KUrl& url)
{
    DCategorizedView::toIndex(d->filterModel->indexForPath(url.toLocalFile()));
}

ShowfotoItemInfo ShowfotoCategorizedView::nextInOrder(const ShowfotoItemInfo& startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForShowfotoItemInfo(startingPoint);

    if (!index.isValid())
    {
        return ShowfotoItemInfo();
    }

    return d->filterModel->showfotoItemInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

QModelIndex ShowfotoCategorizedView::nextIndexHint(const QModelIndex& anchor, const QItemSelectionRange& removed) const
{
    QModelIndex hint = DCategorizedView::nextIndexHint(anchor, removed);
    ShowfotoItemInfo info   = d->filterModel->showfotoItemInfo(anchor);

    //kDebug() << "Having initial hint" << hint << "for" << anchor << d->model->numberOfIndexesForShowfotoItemInfo(info);

    // Fixes a special case of multiple (face) entries for the same image.
    // If one is removed, any entry of the same image shall be preferred.
    if (d->model->numberOfIndexesForShowfotoItemInfo(info) > 1)
    {
        // The hint is for a different info, but we may have a hint for the same info
        if (info != d->filterModel->showfotoItemInfo(hint))
        {
            int minDiff                            = d->filterModel->rowCount();
            QList<QModelIndex> indexesForShowfotoItemInfo = d->filterModel->mapListFromSource(d->model->indexesForShowfotoItemInfo(info));

            foreach(const QModelIndex& index, indexesForShowfotoItemInfo)
            {
                if (index == anchor || !index.isValid() || removed.contains(index))
                {
                    continue;
                }

                int distance = qAbs(index.row() - anchor.row());

                if (distance < minDiff)
                {
                    minDiff = distance;
                    hint = index;
                    //kDebug() << "Chose index" << hint << "at distance" << minDiff << "to" << anchor;
                }
            }
        }
    }

    return hint;
}

ThumbnailSize ShowfotoCategorizedView::thumbnailSize() const
{
/*
    ShowfotoThumbnailModel *thumbModel = ShowfotoThumbnailModel();
    if (thumbModel)
        return thumbModel->thumbnailSize();
*/
    if (d->delegate)
    {
        return d->delegate->thumbnailSize();
    }

    return ThumbnailSize();
}

void ShowfotoCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(ThumbnailSize(size));
}

void ShowfotoCategorizedView::setThumbnailSize(const ThumbnailSize& s)
{
    // we abuse this pair of method calls to restore scroll position
    layoutAboutToBeChanged();
    d->delegate->setThumbnailSize(s);
    layoutWasChanged();
}

void ShowfotoCategorizedView::setCurrentWhenAvailable(qlonglong showfotoItemId)
{
    d->scrollToItemId = showfotoItemId;
}

void ShowfotoCategorizedView::setCurrentUrl(const KUrl& url)
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

void ShowfotoCategorizedView::setCurrentInfo(const ShowfotoItemInfo& info)
{
    QModelIndex index = d->filterModel->indexForShowfotoItemInfo(info);
    clearSelection();
    setCurrentIndex(index);
}

void ShowfotoCategorizedView::setSelectedUrls(const KUrl::List& urlList)
{
    QItemSelection mySelection;

    for (KUrl::List::const_iterator it = urlList.constBegin(); it!=urlList.constEnd(); ++it)
    {
        const QString path = it->path();
        const QModelIndex index = d->filterModel->indexForPath(path);

        if (!index.isValid())
        {
            kWarning() << "no QModelIndex found for" << *it;
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

void ShowfotoCategorizedView::setSelectedShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    QItemSelection mySelection;

    foreach(const ShowfotoItemInfo& info, infos)
    {
        QModelIndex index = d->filterModel->indexForShowfotoItemInfo(info);
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ShowfotoCategorizedView::hintAt(const ShowfotoItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    QModelIndex index = d->filterModel->indexForShowfotoItemInfo(info);

    if (!index.isValid())
    {
        return;
    }

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    scrollTo(index);
}

void ShowfotoCategorizedView::addOverlay(ImageDelegateOverlay* overlay, ShowfotoDelegate* delegate)
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

void ShowfotoCategorizedView::removeOverlay(ImageDelegateOverlay* overlay)
{
    ShowfotoDelegate* delegate = dynamic_cast<ShowfotoDelegate*>(overlay->delegate());

    if (delegate)
    {
        delegate->removeOverlay(overlay);
    }

    overlay->setView(0);
}

void ShowfotoCategorizedView::updateGeometries()
{
    DCategorizedView::updateGeometries();
    d->delayedEnterTimer->start();
}

void ShowfotoCategorizedView::slotDelayedEnter()
{
    // re-emit entered() for index under mouse (after layout).
    QModelIndex mouseIndex = indexAt(mapFromGlobal(QCursor::pos()));

    if (mouseIndex.isValid())
    {
        emit DigikamKCategorizedView::entered(mouseIndex);
    }
}

//void ShowfotoCategorizedView::addSelectionOverlay(ShowfotoDelegate* delegate)
//{
//    addOverlay(new Digikam::ImageSelectionOverlay(this), delegate);
//}

void ShowfotoCategorizedView::scrollToStoredItem()
{
    if (d->scrollToItemId)
    {
        if (d->model->hasImage(d->scrollToItemId))
        {
            QModelIndex index = d->filterModel->indexForShowfotoItemId(d->scrollToItemId);
            setCurrentIndex(index);
            scrollToRelaxed(index, QAbstractItemView::PositionAtCenter);
            d->scrollToItemId = 0;
        }
    }
}

void ShowfotoCategorizedView::slotShowfotoItemInfosAdded()
{
    if (d->scrollToItemId)
    {
        scrollToStoredItem();
    }
}

void ShowfotoCategorizedView::slotFileChanged(const QString& filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);

    if (index.isValid())
    {
        update(index);
    }
}

void ShowfotoCategorizedView::indexActivated(const QModelIndex& index)
{
    ShowfotoItemInfo info = d->filterModel->showfotoItemInfo(index);

    if (!info.isNull())
    {
        activated(info);
        emit showfotoItemInfoActivated(info);
    }
}

void ShowfotoCategorizedView::currentChanged(const QModelIndex& index, const QModelIndex& previous)
{
    DCategorizedView::currentChanged(index, previous);

    emit currentChanged(d->filterModel->showfotoItemInfo(index));
}

void ShowfotoCategorizedView::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    DCategorizedView::selectionChanged(selectedItems, deselectedItems);

    if (!selectedItems.isEmpty())
    {
        emit selected(d->filterModel->showfotoItemInfos(selectedItems.indexes()));
    }

    if (!deselectedItems.isEmpty())
    {
        emit deselected(d->filterModel->showfotoItemInfos(deselectedItems.indexes()));
    }
}

void ShowfotoCategorizedView::activated(const ShowfotoItemInfo&)
{
    // implemented in subclass
}

void ShowfotoCategorizedView::showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)
{
    ShowfotoItemInfo info = d->filterModel->showfotoItemInfo(index);
    showContextMenuOnInfo(event, info);
}

void ShowfotoCategorizedView::showContextMenuOnInfo(QContextMenuEvent*, const ShowfotoItemInfo&)
{
    // implemented in subclass
}

void ShowfotoCategorizedView::paintEvent(QPaintEvent* e)
{
    DCategorizedView::paintEvent(e);
}

QItemSelectionModel* ShowfotoCategorizedView::getSelectionModel() const
{
    return selectionModel();
}

AbstractItemDragDropHandler* ShowfotoCategorizedView::dragDropHandler() const
{
    return d->model->dragDropHandler();
}


}
