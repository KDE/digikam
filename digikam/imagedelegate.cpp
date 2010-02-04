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
#include "itemviewimagedelegatepriv.h"

// C++ includes

#include <cmath>

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
#include "albumsettings.h"
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

class ImageDelegatePrivate : public ItemViewImageDelegatePrivate
{
public:

    ImageDelegatePrivate()
    {
        categoryDrawer = 0;

        actualPixmapRectCache.setMaxCost(250);
    }

    QRect                     dateRect;
    QRect                     modDateRect;
    QRect                     pixmapRect;
    QRect                     nameRect;
    QRect                     commentsRect;
    QRect                     resolutionRect;
    QRect                     sizeRect;
    QRect                     tagRect;

    QCache<qlonglong, QRect>  actualPixmapRectCache;
    ImageCategoryDrawer      *categoryDrawer;

    virtual void clearRects();
};

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
    Q_D(ImageDelegate);
    d->categoryDrawer = new ImageCategoryDrawer(parent);

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

ImageDelegate::~ImageDelegate()
{
    Q_D(ImageDelegate);
    delete d->categoryDrawer;
}

void ImageDelegate::setSpacing(int spacing)
{
    Q_D(ImageDelegate);
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
    const AlbumSettings *settings = AlbumSettings::instance();
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
    model->setData(index, d->thumbSize.size(), ImageModel::ThumbnailRole);

    QRect actualPixmapRect = drawThumbnail(p, d->pixmapRect, pix, thumbData.value<QPixmap>());
    if (!actualPixmapRect.isNull())
        const_cast<ImageDelegate*>(this)->updateActualPixmapRect(info.id(), actualPixmapRect);

    if (settings->getIconShowRating())
    {
        drawRating(p, index, d->ratingRect, info.rating(), isSelected);
    }

    if (settings->getIconShowName())
    {
        drawName(p, d->nameRect, info.name());
    }

    if (settings->getIconShowComments())
    {
        drawComments(p, d->commentsRect, info.comment());
    }

    if (settings->getIconShowDate())
    {
        drawCreationDate(p, d->dateRect, info.dateTime());
    }

    if (settings->getIconShowModDate())
    {
        drawModificationDate(p, d->modDateRect, info.modDateTime());
    }

    if (settings->getIconShowResolution())
    {
        drawImageSize(p, d->resolutionRect, info.dimensions());
    }

    if (settings->getIconShowSize())
    {
        drawFileSize(p, d->sizeRect, info.fileSize());
    }

    if (settings->getIconShowTags())
    {
        QString tags = AlbumManager::instance()->tagNames(info.tagIds()).join(", ");
        drawTags(p, d->tagRect, tags, isSelected);
    }

    drawStateRects(p, option, isSelected);

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
    d->categoryDrawer->setDefaultViewOptions(option);
    ItemViewImageDelegate::setDefaultViewOptions(option);
}

void ImageDelegate::invalidatePaintingCache()
{
    Q_D(ImageDelegate);
    d->categoryDrawer->invalidatePaintingCache();
    ItemViewImageDelegate::invalidatePaintingCache();
}

void ImageDelegate::updateSizeRectsAndPixmaps()
{
    Q_D(ImageDelegate);

    // ---- Reset rects and prepare fonts ----

    d->clearRects();
    prepareFonts();

    // ---- Fixed sizes and metrics ----

    int w = d->thumbSize.size() + 2*d->radius;
    prepareMetrics(w);

    // ---- Calculate rects ----

    int y = d->margin;

    d->pixmapRect = QRect(d->margin, y, w, d->thumbSize.size() + 2*d->radius);
    y = d->pixmapRect.bottom();

    const AlbumSettings *albumSettings = AlbumSettings::instance();
    if (albumSettings->getIconShowRating())
    {
        d->ratingRect = QRect(d->margin, y, w, d->starPolygonSize.height());
        y = d->ratingRect.bottom();
    }

    if (albumSettings->getIconShowName())
    {
        d->nameRect = QRect(d->margin, y, w-d->margin, d->oneRowRegRect.height());
        y = d->nameRect.bottom();
    }

    if (albumSettings->getIconShowComments())
    {
        d->commentsRect = QRect(d->margin, y, w, d->oneRowComRect.height());
        y = d->commentsRect.bottom();
    }

    if (albumSettings->getIconShowDate())
    {
        d->dateRect = QRect(d->margin, y, w, d->oneRowXtraRect.height());
        y = d->dateRect.bottom();
    }

    if (albumSettings->getIconShowModDate())
    {
        d->modDateRect = QRect(d->margin, y, w, d->oneRowXtraRect.height());
        y = d->modDateRect.bottom();
    }

    if (albumSettings->getIconShowResolution())
    {
        d->resolutionRect = QRect(d->margin, y, w, d->oneRowXtraRect.height());
        y = d->resolutionRect.bottom() ;
    }

    if (albumSettings->getIconShowSize())
    {
        d->sizeRect = QRect(d->margin, y, w, d->oneRowXtraRect.height());
        y = d->sizeRect.bottom();
    }

    if (albumSettings->getIconShowTags())
    {
        d->tagRect = QRect(d->margin, y, w, d->oneRowComRect.height());
        y = d->tagRect.bottom();
    }

    d->rect = QRect(0, 0, w + 2*d->margin, y+d->margin+d->radius);

    d->gridSize  = QSize(d->rect.width() + d->spacing, d->rect.height() + d->spacing);

    // ---- Cached pixmaps ----

    prepareBackground();

    if (albumSettings->getIconShowRating())
    {
        prepareRatingPixmaps();
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
