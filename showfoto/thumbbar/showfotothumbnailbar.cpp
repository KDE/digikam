/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 02-08-2013
 * Description : Thumbnail bar for Showfoto
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "showfotothumbnailbar.h"

#include "KDebug"
#include "QTimer"

// Local includes

#include "showfotodelegate.h"
#include "showfotofiltermodel.h"
#include "showfotosettings.h"
#include "showfotooverlays.h"
#include "showfotodragdrop.h"
#include "itemviewtooltip.h"
#include "showfototooltipfiller.h"
#include "showfotocategorizedview.h"
#include "imageselectionoverlay.h"

namespace ShowFoto {

class ShowfotoThumbnailBar::Private
{
public:

    Private()
    {
        scrollPolicy     = Qt::ScrollBarAlwaysOn;
        duplicatesFilter = 0;
    }

    Qt::ScrollBarPolicy              scrollPolicy;
    NoDuplicatesShowfotoFilterModel* duplicatesFilter;
};

ShowfotoThumbnailBar::ShowfotoThumbnailBar(QWidget* const parent)
    : ShowfotoCategorizedView(parent), d(new Private())
{
    setItemDelegate(new ShowfotoThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(5);
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(ShowfotoSettings::instance()->showToolTipsIsValid());

    connect(ShowfotoSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
    setFlow(LeftToRight);
}

ShowfotoThumbnailBar::~ShowfotoThumbnailBar()
{
    delete d;
}

void ShowfotoThumbnailBar::setModelsFiltered(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel)
{
    if (!d->duplicatesFilter)
    {
        d->duplicatesFilter = new NoDuplicatesShowfotoFilterModel(this);
    }

    d->duplicatesFilter->setSourceFilterModel(filterModel);
    ShowfotoCategorizedView::setModels(model, d->duplicatesFilter);
}

void ShowfotoThumbnailBar::installRatingOverlay()
{
    ShowfotoRatingOverlay* ratingOverlay = new ShowfotoRatingOverlay(this);
    addOverlay(ratingOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));
}

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

//TODO:
//void ShowfotoThumbnailBar::addSelectionOverlay(ShowfotoDelegate* delegate)
//{
//    addOverlay(new ImageSelectionOverlay(this), delegate);
//}

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

    ShowfotoCategorizedView::setFlow(flow);

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

    ShowfotoCategorizedView::slotSetupChanged();
}

void ShowfotoThumbnailBar::assignRating(const QList<QModelIndex>& indexes, int rating)
{
   QList<QModelIndex> mappedIndexes = showfotoSortFilterModel()->mapListToSource(indexes);

   foreach(QModelIndex index, mappedIndexes)
   {
       if (index.isValid())
       {
            showfotoImageModel()->showfotoItemInfoRef(index).rating = rating;
       }
   }
}

bool ShowfotoThumbnailBar::event(QEvent* e)
{
    // reset widget max/min sizes
    if (e->type() == QEvent::StyleChange)
    {
        setFlow(flow());
    }

    return ShowfotoCategorizedView::event(e);
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

ShowfotoItemInfo ShowfotoThumbnailBar::findItemByUrl(const KUrl url)
{
    ShowfotoItemInfoList lst = showfotoItemInfos();
    int i;

    for(i = 0 ; i< lst.size() ; i++)
    {
        if(lst.at(i).url == url)
        {
            return lst.at(i);
        }
    }

    return ShowfotoItemInfo();
}
} // namespace ShowFoto
