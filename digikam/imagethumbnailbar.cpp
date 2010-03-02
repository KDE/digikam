/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-06
 * Description : Thumbnail bar for images
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "imagethumbnaildelegate.h"
#include "imagealbumfiltermodel.h"
#include "imagealbummodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "imagethumbnaildelegate.h"
#include "metadatamanager.h"

namespace Digikam
{

class ImageThumbnailBarPriv
{
public:

    ImageThumbnailBarPriv()
    {
        scrollPolicy = Qt::ScrollBarAlwaysOn;
    }

    Qt::ScrollBarPolicy     scrollPolicy;
};

ImageThumbnailBar::ImageThumbnailBar(QWidget *parent)
                : ImageCategorizedView(parent), d(new ImageThumbnailBarPriv())
{
    setItemDelegate(new ImageThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(5);
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    AlbumSettings *settings = AlbumSettings::instance();

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());

    // rating overlay
    ImageRatingOverlay *ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(const QModelIndex &, int)),
            this, SLOT(assignRating(const QModelIndex&, int)));

    connect(settings, SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImageThumbnailBar::~ImageThumbnailBar()
{
    delete d;
}

void ImageThumbnailBar::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
        setFlow(TopToBottom);
    else
        setFlow(LeftToRight);
}

void ImageThumbnailBar::setScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    if (policy == Qt::ScrollBarAsNeeded)
    {
        // Delegate esizing will cause endless relayouting, see bug #228807
        kError() << "The Qt::ScrollBarAsNeeded policy is not supported by ImageThumbnailBar"
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

void ImageThumbnailBar::setFlow(QListView::Flow newFlow)
{
    setWrapping(false);

    if (newFlow == flow())
        return;

    ImageCategorizedView::setFlow(newFlow);

    ImageThumbnailDelegate *del = static_cast<ImageThumbnailDelegate*>(delegate());
    del->setFlow(newFlow);

    // Reset the minimum and maximum sizes.
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    // Adjust minimum and maximum width to thumbnail sizes.
    if (newFlow == TopToBottom)
    {
        setMinimumWidth(del->minimumSize() + verticalScrollBar()->sizeHint().width());
        setMaximumWidth(del->maximumSize() + verticalScrollBar()->sizeHint().width());
    }
    else
    {
        setMinimumHeight(del->minimumSize() + horizontalScrollBar()->sizeHint().height());
        setMaximumHeight(del->maximumSize() + horizontalScrollBar()->sizeHint().height());
    }

    setScrollBarPolicy(d->scrollPolicy);
}

void ImageThumbnailBar::slotSetupChanged()
{
    setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());
    setFont(AlbumSettings::instance()->getIconViewFont());

    ImageCategorizedView::slotSetupChanged();
}

void ImageThumbnailBar::activated(const ImageInfo& info)
{
    kDebug() << info.filePath();
    if (info.isNull())
        return;

    emit imageActivated(info);
}

void ImageThumbnailBar::assignRating(const QModelIndex &index, int rating)
{
    MetadataManager::instance()->assignRating(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), rating);
}

} // namespace Digikam
