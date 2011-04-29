/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-06
 * Description : Thumbnail bar for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagethumbnailbar.moc"

// Qt includes

#include <QClipboard>
#include <QFileInfo>
#include <QPointer>
#include <QScrollBar>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kwindowsystem.h>

// Local includes

#include "albumsettings.h"
#include "imagealbumfiltermodel.h"
#include "imagealbummodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "imagethumbnaildelegate.h"
#include "metadatamanager.h"

namespace Digikam
{

class ImageThumbnailBar::ImageThumbnailBarPriv
{
public:

    ImageThumbnailBarPriv()
    {
        scrollPolicy     = Qt::ScrollBarAlwaysOn;
        duplicatesFilter = 0;
    }

    Qt::ScrollBarPolicy           scrollPolicy;
    NoDuplicatesImageFilterModel* duplicatesFilter;
};

ImageThumbnailBar::ImageThumbnailBar(QWidget* parent)
    : ImageCategorizedView(parent), d(new ImageThumbnailBarPriv())
{
    setItemDelegate(new ImageThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(5);
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImageThumbnailBar::~ImageThumbnailBar()
{
    delete d;
}

void ImageThumbnailBar::setModelsFiltered(ImageModel* model, ImageSortFilterModel* filterModel)
{
    if (!d->duplicatesFilter)
    {
        d->duplicatesFilter = new NoDuplicatesImageFilterModel(this);
    }

    d->duplicatesFilter->setSourceFilterModel(filterModel);
    ImageCategorizedView::setModels(model, d->duplicatesFilter);
}

void ImageThumbnailBar::installRatingOverlay()
{
    ImageRatingOverlay* ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(const QList<QModelIndex>&, int)),
            this, SLOT(assignRating(const QList<QModelIndex>&, int)));
}

void ImageThumbnailBar::slotDockLocationChanged(Qt::DockWidgetArea area)
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

void ImageThumbnailBar::setScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    if (policy == Qt::ScrollBarAsNeeded)
    {
        // Delegate resizing will cause endless relayouting, see bug #228807
        kError() << "The Qt::ScrollBarAsNeeded policy is not supported by ImageThumbnailBar";
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

void ImageThumbnailBar::setFlow(QListView::Flow flow)
{
    setWrapping(false);

    ImageCategorizedView::setFlow(flow);

    ImageThumbnailDelegate* del = static_cast<ImageThumbnailDelegate*>(delegate());
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

void ImageThumbnailBar::slotSetupChanged()
{
    setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());
    setFont(AlbumSettings::instance()->getIconViewFont());

    ImageCategorizedView::slotSetupChanged();
}

void ImageThumbnailBar::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    MetadataManager::instance()->assignRating(imageSortFilterModel()->imageInfos(indexes), rating);
}

bool ImageThumbnailBar::event(QEvent* e)
{
    // reset widget max/min sizes
    if (e->type() == QEvent::StyleChange)
    {
        setFlow(flow());
    }

    return ImageCategorizedView::event(e);
}

QModelIndex ImageThumbnailBar::nextIndex(const QModelIndex& index) const
{
    return imageFilterModel()->index(index.row() + 1, 0);
}

QModelIndex ImageThumbnailBar::previousIndex(const QModelIndex& index) const
{
    return imageFilterModel()->index(index.row() - 1, 0);
}

QModelIndex ImageThumbnailBar::firstIndex() const
{
    return imageFilterModel()->index(0, 0);
}

QModelIndex ImageThumbnailBar::lastIndex() const
{
    return imageFilterModel()->index(imageFilterModel->rowCount() - 1, 0);
}

} // namespace Digikam
