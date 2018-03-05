/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : thumbnail bar for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "imagethumbnaildelegate.h"
#include "imagedelegatepriv.h"

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "imagecategorizedview.h"
#include "imagedelegateoverlay.h"
#include "imagemodel.h"
#include "imagefiltermodel.h"
#include "thumbnailloadthread.h"
#include "imagethumbnaildelegatepriv.h"

namespace Digikam
{

void ImageThumbnailDelegatePrivate::init(ImageThumbnailDelegate* const q)
{
    QObject::connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
                     q, SLOT(slotSetupChanged()));
}

ImageThumbnailDelegate::ImageThumbnailDelegate(ImageCategorizedView* parent)
    : ImageDelegate(*new ImageThumbnailDelegatePrivate, parent)
{
    Q_D(ImageThumbnailDelegate);
    d->init(this);
}

ImageThumbnailDelegate::~ImageThumbnailDelegate()
{
}

void ImageThumbnailDelegate::setFlow(QListView::Flow flow)
{
    Q_D(ImageThumbnailDelegate);
    d->flow = flow;
}

void ImageThumbnailDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ImageThumbnailDelegate);
    // store before calling parent class
    d->viewSize = option.rect;
    ImageDelegate::setDefaultViewOptions(option);
}

int ImageThumbnailDelegate::maximumSize() const
{
    Q_D(const ImageThumbnailDelegate);
    return ThumbnailLoadThread::maximumThumbnailPixmapSize(true) + 2*d->radius + 2*d->margin;
}

int ImageThumbnailDelegate::minimumSize() const
{
    Q_D(const ImageThumbnailDelegate);
    return ThumbnailSize::Small + 2*d->radius + 2*d->margin;
}

bool ImageThumbnailDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect,
        const QModelIndex& index, QRect* activationRect) const
{
    // reuse implementation from grandparent
    return ItemViewImageDelegate::acceptsActivation(pos, visualRect, index, activationRect);
}

void ImageThumbnailDelegate::updateContentWidth()
{
    Q_D(ImageThumbnailDelegate);
    int maxSize;

    if (d->flow == QListView::LeftToRight)
    {
        maxSize = d->viewSize.height();
    }
    else
    {
        maxSize = d->viewSize.width();
    }

    d->thumbSize = ThumbnailLoadThread::thumbnailToPixmapSize(true, maxSize - 2*d->radius - 2*d->margin);

    ImageDelegate::updateContentWidth();
}

void ImageThumbnailDelegate::updateRects()
{
    Q_D(ImageThumbnailDelegate);

    d->rect            = QRect(0, 0, d->contentWidth + 2*d->margin, d->contentWidth + 2*d->margin);
    d->pixmapRect      = QRect(d->margin, d->margin, d->contentWidth, d->contentWidth);
    const int iconSize = qBound(16, (d->contentWidth + 2*d->margin) / 8 - 2, 48);
    d->coordinatesRect = QRect(d->contentWidth - iconSize+2, d->pixmapRect.top(), iconSize, iconSize);
    d->pickLabelRect   = QRect(d->margin+2, d->rect.bottom() - d->margin - 18, 16, 16);
    d->drawImageFormat = ApplicationSettings::instance()->getIconShowImageFormat();
    d->drawCoordinates = ApplicationSettings::instance()->getIconShowCoordinates();

    if (ApplicationSettings::instance()->getIconShowRating())
    {
        int top       = d->rect.bottom() - d->margin - d->starPolygonSize.height() - 2;
        d->ratingRect = QRect(d->margin, top, d->contentWidth, d->starPolygonSize.height());
    }

    if (d->flow == QListView::LeftToRight)
    {
        d->gridSize = QSize(d->rect.width() + d->spacing, d->rect.height());
    }
    else
    {
        d->gridSize = QSize(d->rect.width(), d->rect.height() + d->spacing);
    }
}

} // namespace Digikam
