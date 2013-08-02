
#include "showfotothumbnailbar.h"

// KDE includes

#include <kdebug.h>

// Local includes

#include "showfotosettings.h"
#include "showfotodelegate.h"
#include "showfotofiltermodel.h"
//#include "showfotooverlays.h"

namespace ShowFoto
{

class ShowfotoThumbnailBar::Private
{
public:

    Private()
        : model(0),
          filterModel(0),
          delegate(0),
          showToolTip(false),
          scrollToItemId(0)
    {
        scrollPolicy     = Qt::ScrollBarAlwaysOn;
        duplicatesFilter = 0;
    }

    Qt::ScrollBarPolicy              scrollPolicy;
    NoDuplicatesShowfotoFilterModel* duplicatesFilter;
    ShowfotoImageModel*              model;
    ShowfotoSortFilterModel*         filterModel;
    ShowfotoDelegate*                delegate;
    bool                             showToolTip;
    qlonglong                        scrollToItemId;

    QTimer*                          delayedEnterTimer;

    QMouseEvent*                     currentMouseEvent;

};

ShowfotoThumbnailBar::ShowfotoThumbnailBar(QWidget* const parent)
    : DCategorizedView(parent), d(new Private())
{
    setItemDelegate(new ShowfotoThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(5);
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    //TODO: Implement Showfoto Tool settings
    //setToolTipEnabled(ShowfotoSettings::instance()->showToolTipsIsValid());

    connect(ShowfotoSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
    setFlow(LeftToRight);
}

ShowfotoThumbnailBar::~ShowfotoThumbnailBar()
{
    delete d;
}

//TODO: Take what U want from the categorized view
void ShowfotoThumbnailBar::setModels(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel)
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

ShowfotoImageModel* ShowfotoThumbnailBar::showfotoImageModel() const
{
    return d->model;
}

ShowfotoSortFilterModel* ShowfotoThumbnailBar::showfotoSortFilterModel() const
{
    return d->filterModel;
}

ShowfotoFilterModel* ShowfotoThumbnailBar::showfotoFilterModel() const
{
    return d->filterModel->showfotoFilterModel();
}

ShowfotoThumbnailModel* ShowfotoThumbnailBar::showfotoThumbnailModel() const
{
    return qobject_cast<ShowfotoThumbnailModel*>(d->model);
}

QSortFilterProxyModel* ShowfotoThumbnailBar::filterModel() const
{
    return d->filterModel;
}

ShowfotoDelegate* ShowfotoThumbnailBar::delegate() const
{
    return d->delegate;
}

ThumbnailSize ShowfotoThumbnailBar::thumbnailSize() const
{
/*
    ImportThumbnailModel *thumbModel = importThumbnailModel();
    if (thumbModel)
        return thumbModel->thumbnailSize();
*/
    if (d->delegate)
    {
        return d->delegate->thumbnailSize();
    }

    return ThumbnailSize();
}

void ShowfotoThumbnailBar::setItemDelegate(ShowfotoDelegate* delegate)
{
    ThumbnailSize oldSize         = thumbnailSize();
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
//  setCategoryDrawer(d->delegate->categoryDrawer());
    updateDelegateSizes();

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
            this, SLOT(showIndexNotification(QModelIndex,QString)));

    connect(d->delegate, SIGNAL(hideNotification()),
            this, SLOT(hideIndexNotification()));
}

void ShowfotoThumbnailBar::scrollToStoredItem()
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

void ShowfotoThumbnailBar::slotShowfotoItemInfosAdded()
{
    if (d->scrollToItemId)
    {
        scrollToStoredItem();
    }
}

void ShowfotoThumbnailBar::setModelsFiltered(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel)
{
    if (!d->duplicatesFilter)
    {
        d->duplicatesFilter = new NoDuplicatesShowfotoFilterModel(this);
    }

    d->duplicatesFilter->setSourceFilterModel(filterModel);
    setModels(model, d->duplicatesFilter);
}

//TODO: make sure that u won't use the rating overlays
//void ShowfotoThumbnailBar::installRatingOverlay()
//{
//    ShowfotoRatingOverlay* ratingOverlay = new ShowfotoRatingOverlay(this);
//    addOverlay(ratingOverlay);

//    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
//            this, SLOT(assignRating(QList<QModelIndex>,int)));
//}

void ShowfotoThumbnailBar::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
    {
        setFlow(TopToBottom);
    }
    else
    {
        setFlow(LeftToRight);
    }

    scrollTo(currentIndex());
}

void ShowfotoThumbnailBar::setScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    if (policy == Qt::ScrollBarAsNeeded)
    {
        // Delegate resizing will cause endless relayouting, see bug #228807
        kError() << "The Qt::ScrollBarAsNeeded policy is not supported by ShowfotoThumbnailBar";
    }

    d->scrollPolicy = policy;

    if (flow() == TopToBottom)
    {
        setVerticalScrollBarPolicy(d->scrollPolicy);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        setHorizontalScrollBarPolicy(d->scrollPolicy);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void ShowfotoThumbnailBar::setFlow(QListView::Flow flow)
{
    setWrapping(false);

    DCategorizedView::setFlow(flow);

    ShowfotoThumbnailDelegate* del = static_cast<ShowfotoThumbnailDelegate*>(delegate());
    del->setFlow(flow);

    // Reset the minimum and maximum sizes.
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    // Adjust minimum and maximum width to thumbnail sizes.
    if (flow == TopToBottom)
    {
        int viewportFullWidgetOffset = size().width() - viewport()->size().width();
        setMinimumWidth(del->minimumSize() + viewportFullWidgetOffset);
        setMaximumWidth(del->maximumSize() + viewportFullWidgetOffset);
    }
    else
    {
        int viewportFullWidgetOffset = size().height() - viewport()->size().height();
        setMinimumHeight(del->minimumSize() + viewportFullWidgetOffset);
        setMaximumHeight(del->maximumSize() + viewportFullWidgetOffset);
    }

    setScrollBarPolicy(d->scrollPolicy);
}

void ShowfotoThumbnailBar::slotSetupChanged()
{
    setToolTipEnabled(ShowfotoSettings::instance()->showToolTipsIsValid());
    setFont(ShowfotoSettings::instance()->getIconViewFont());

    DCategorizedView::slotSetupChanged();
}

//TODO:make sure u won't need ratings
//void ShowfotoThumbnailBar::assignRating(const QList<QModelIndex>& indexes, int rating)
//{
//   QList<QModelIndex> mappedIndexes = showfotoSortFilterModel()->mapListToSource(indexes);

//   foreach(QModelIndex index, mappedIndexes)
//   {
//       if (index.isValid())
//       {
//            showfotoImageModel()->showfotoItemInfoRef(index).rating = rating;
//       }
//   }
//}

bool ShowfotoThumbnailBar::event(QEvent* e)
{
    // reset widget max/min sizes
    if (e->type() == QEvent::StyleChange)
    {
        setFlow(flow());
    }

    return DCategorizedView::event(e);
}

QModelIndex ShowfotoThumbnailBar::nextIndex(const QModelIndex& index) const
{
    return showfotoFilterModel()->index(index.row() + 1, 0);
}

QModelIndex ShowfotoThumbnailBar::previousIndex(const QModelIndex& index) const
{
    return showfotoFilterModel()->index(index.row() - 1, 0);
}

QModelIndex ShowfotoThumbnailBar::firstIndex() const
{
    return showfotoFilterModel()->index(0, 0);
}

QModelIndex ShowfotoThumbnailBar::lastIndex() const
{
    return showfotoFilterModel()->index(showfotoFilterModel()->rowCount() - 1, 0);
}

} // namespace Digikam
