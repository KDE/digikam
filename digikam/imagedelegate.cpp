/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "imagedelegate.moc"
#include "imagedelegatepriv.h"

// Qt includes

#include <QCache>
#include <QPainter>

// KDE includes

#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "imagecategorydrawer.h"
#include "imagecategorizedview.h"
#include "imagedelegateoverlay.h"
#include "imagemodel.h"
#include "imagefiltermodel.h"
#include "themeengine.h"
#include "thumbbar.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

void ImageDelegatePrivate::clearRects()
{
    ItemViewImageDelegatePrivate::clearRects();
    dateRect       = QRect(0, 0, 0, 0);
    modDateRect    = QRect(0, 0, 0, 0);
    pixmapRect     = QRect(0, 0, 0, 0);
    nameRect       = QRect(0, 0, 0, 0);
    commentsRect   = QRect(0, 0, 0, 0);
    resolutionRect = QRect(0, 0, 0, 0);
    sizeRect       = QRect(0, 0, 0, 0);
    tagRect        = QRect(0, 0, 0, 0);
}

ImageDelegate::ImageDelegate(ImageCategorizedView *parent)
             : ItemViewImageDelegate(*new ImageDelegatePrivate, parent)
{
}

ImageDelegate::ImageDelegate(ImageDelegatePrivate &dd, ImageCategorizedView *parent)
             : ItemViewImageDelegate(dd, parent)
{
}

ImageDelegate::~ImageDelegate()
{
    Q_D(ImageDelegate);
    delete d->categoryDrawer;
}

void ImageDelegate::setSpacing(int spacing)
{
    Q_D(ImageDelegate);
    if (d->categoryDrawer)
        d->categoryDrawer->setLowerSpacing(spacing);
    ItemViewImageDelegate::setSpacing(spacing);
}

ImageCategoryDrawer *ImageDelegate::categoryDrawer() const
{
    Q_D(const ImageDelegate);
    return d->categoryDrawer;
}

QRect ImageDelegate::ratingRect() const
{
    Q_D(const ImageDelegate);
    return d->ratingRect;
}

QRect ImageDelegate::commentsRect() const
{
    Q_D(const ImageDelegate);
    return d->commentsRect;
}

QRect ImageDelegate::tagsRect() const
{
    Q_D(const ImageDelegate);
    return d->tagRect;
}

void ImageDelegate::paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const ImageDelegate);
    ImageInfo info = ImageModel::retrieveImageInfo(index);

    if (info.isNull())
        return;

    // state of painter must not be changed
    p->save();
    p->translate(option.rect.topLeft());

    QRect r;
    ThemeEngine* te               = ThemeEngine::instance();

    bool isSelected = (option.state & QStyle::State_Selected);

    QPixmap pix;
    if (isSelected)
        pix = d->selPixmap;
    else
        pix = d->regPixmap;

    p->setPen(isSelected ? te->textSelColor() : te->textRegColor());

    // Thumbnail
    QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
    model->setData(index, d->thumbSize.size(), ImageModel::ThumbnailRole);
    QVariant thumbData = index.data(ImageModel::ThumbnailRole);
    model->setData(index, QVariant(), ImageModel::ThumbnailRole);

    QRect actualPixmapRect = drawThumbnail(p, d->pixmapRect, pix, thumbData.value<QPixmap>());
    if (!actualPixmapRect.isNull())
        const_cast<ImageDelegate*>(this)->updateActualPixmapRect(info.id(), actualPixmapRect);

    if (!d->ratingRect.isNull())
    {
        drawRating(p, index, d->ratingRect, info.rating(), isSelected);
    }

    if (!d->nameRect.isNull())
    {
        drawName(p, d->nameRect, info.name());
    }

    if (!d->commentsRect.isNull())
    {
        drawComments(p, d->commentsRect, info.comment());
    }

    if (!d->dateRect.isNull())
    {
        drawCreationDate(p, d->dateRect, info.dateTime());
    }

    if (!d->modDateRect.isNull())
    {
        drawModificationDate(p, d->modDateRect, info.modDateTime());
    }

    if (!d->resolutionRect.isNull())
    {
        drawImageSize(p, d->resolutionRect, info.dimensions());
    }

    if (!d->sizeRect.isNull())
    {
        drawFileSize(p, d->sizeRect, info.fileSize());
    }

    if (!d->tagRect.isNull())
    {
        QString tags = AlbumManager::instance()->tagNames(info.tagIds()).join(", ");
        drawTags(p, d->tagRect, tags, isSelected);
    }

    if (d->drawFocusFrame)
        drawFocusRect(p, option, isSelected);
    if (d->drawMouseOverFrame)
        drawMouseOverRect(p, option);

    p->restore();

    drawDelegates(p, option, index);
}

QPixmap ImageDelegate::pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const
{
    QPixmap icon;

    if (indexes.count() == 1)
    {
        QVariant thumbData = indexes.first().data(ImageModel::ThumbnailRole);
        if (!thumbData.isNull())
            icon = thumbData.value<QPixmap>();
    }

    return makeDragPixmap(option, indexes, icon);
}

bool ImageDelegate::acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect *toolTipRect) const
{
    return onActualPixmapRect(pos, visualRect, index, toolTipRect);
}

bool ImageDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect *activationRect) const
{
    return onActualPixmapRect(pos, visualRect, index, activationRect);
}

bool ImageDelegate::onActualPixmapRect(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect *returnRect) const
{
    qlonglong id = ImageModel::retrieveImageId(index);

    if (!id)
        return false;

    QRect actualRect = actualPixmapRect(id);
    if (actualRect.isNull())
        return false;

    actualRect.translate(visualRect.topLeft());
    if (returnRect)
        *returnRect = actualRect;
    return actualRect.contains(pos);
}

void ImageDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ImageDelegate);
    if (d->categoryDrawer)
        d->categoryDrawer->setDefaultViewOptions(option);
    ItemViewImageDelegate::setDefaultViewOptions(option);
}

void ImageDelegate::invalidatePaintingCache()
{
    Q_D(ImageDelegate);
    if (d->categoryDrawer)
        d->categoryDrawer->invalidatePaintingCache();
    ItemViewImageDelegate::invalidatePaintingCache();
}

void ImageDelegate::updateContentWidth()
{
    Q_D(ImageDelegate);
    d->contentWidth = d->thumbSize.size() + 2*d->radius;
}

void ImageDelegate::updateSizeRectsAndPixmaps()
{
    Q_D(ImageDelegate);

    // ---- Reset rects and prepare fonts ----

    d->clearRects();
    prepareFonts();

    // ---- Fixed sizes and metrics ----

    updateContentWidth();
    prepareMetrics(d->contentWidth);

    // ---- Calculate rects ----

    updateRects();

    // ---- Cached pixmaps ----

    prepareBackground();

    if (!d->ratingRect.isNull())
    {
        // Normally we prepare the pixmaps over the background of the rating rect.
        // If the rating is drawn over the thumbnail, we can only draw over a transparent pixmap.
        prepareRatingPixmaps(!d->ratingOverThumbnail);
    }

    // ---- Drawing related caches ----

    clearCaches();
}

void ImageDelegate::clearCaches()
{
    Q_D(ImageDelegate);
    ItemViewImageDelegate::clearCaches();
    d->actualPixmapRectCache.clear();
}

QRect ImageDelegate::actualPixmapRect(qlonglong imageid) const
{
    Q_D(const ImageDelegate);
    // We do not recompute if not found. Assumption is cache is always properly updated.
    QRect *rect = d->actualPixmapRectCache.object(imageid);
    if (rect)
        return *rect;
    else
        return d->pixmapRect;
}

void ImageDelegate::updateActualPixmapRect(qlonglong imageid, const QRect& rect)
{
    Q_D(ImageDelegate);
    QRect *old = d->actualPixmapRectCache.object(imageid);
    if (!old || *old != rect)
        d->actualPixmapRectCache.insert(imageid, new QRect(rect));
}

} // namespace Digikam
