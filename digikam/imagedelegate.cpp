/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2002-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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
#include "imagethumbnailmodel.h"
#include "themeengine.h"
#include "thumbbar.h"
#include "thumbnailloadthread.h"
#include <KIcon>

namespace Digikam
{

void ImageDelegate::ImageDelegatePrivate::clearRects()
{
    ItemViewImageDelegatePrivate::clearRects();
    dateRect             = QRect(0, 0, 0, 0);
    modDateRect          = QRect(0, 0, 0, 0);
    pixmapRect           = QRect(0, 0, 0, 0);
    nameRect             = QRect(0, 0, 0, 0);
    commentsRect         = QRect(0, 0, 0, 0);
    resolutionRect       = QRect(0, 0, 0, 0);
    sizeRect             = QRect(0, 0, 0, 0);
    tagRect              = QRect(0, 0, 0, 0);
    imageInformationRect = QRect(0, 0, 0, 0);
}

ImageDelegate::ImageDelegate(QObject* parent)
    : ItemViewImageDelegate(*new ImageDelegatePrivate, parent)
{
}

ImageDelegate::ImageDelegate(ImageDelegate::ImageDelegatePrivate& dd, QObject* parent)
    : ItemViewImageDelegate(dd, parent)
{
}

ImageDelegate::~ImageDelegate()
{
    Q_D(ImageDelegate);
    // crashes for a lot of people, see bug 230515. Cause unknown.
    //delete d->categoryDrawer;
    Q_UNUSED(d); // To please compiler about warnings.
}

void ImageDelegate::setView(ImageCategorizedView* view)
{
    Q_D(ImageDelegate);
    setViewOnAllOverlays(view);

    if (d->currentView)
    {
        disconnect(d->currentView, SIGNAL(modelChanged()),
                   this, SLOT(modelChanged()));
    }

    d->currentView = view;

    setModel(view ? view->model() : 0);

    if (d->currentView)
    {
        connect(d->currentView, SIGNAL(modelChanged()),
                this, SLOT(modelChanged()));
    }
}

void ImageDelegate::setModel(QAbstractItemModel* model)
{
    Q_D(ImageDelegate);

    // 1) We only need the model to invalidate model-index based caches on change
    // 2) We do not need to care for overlays. The view calls setActive() on them on model change

    if (model == d->currentModel)
    {
        return;
    }

    if (d->currentModel)
    {
        disconnect(d->currentModel, 0, this, 0);
    }

    d->currentModel = model;

    if (d->currentModel)
    {
        connect(d->currentModel, SIGNAL(layoutAboutToBeChanged()),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(modelAboutToBeReset()),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(modelContentsChanged()));
    }
}

void ImageDelegate::setSpacing(int spacing)
{
    Q_D(ImageDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->setLowerSpacing(spacing);
    }

    ItemViewImageDelegate::setSpacing(spacing);
}

ImageCategoryDrawer* ImageDelegate::categoryDrawer() const
{
    Q_D(const ImageDelegate);
    return d->categoryDrawer;
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

QRect ImageDelegate::pixmapRect() const
{
    Q_D(const ImageDelegate);
    return d->pixmapRect;
}

QRect ImageDelegate::imageInformationRect() const
{
    Q_D(const ImageDelegate);
    return d->imageInformationRect;
}

void ImageDelegate::prepareThumbnails(ImageThumbnailModel* thumbModel, const QList<QModelIndex>& indexes)
{
    thumbModel->prepareThumbnails(indexes, thumbnailSize());
}

QPixmap ImageDelegate::retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize)
{
    // work around constness
    QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
    // set requested thumbnail size
    model->setData(index, thumbnailSize, ImageModel::ThumbnailRole);
    // get data from model
    QVariant thumbData = index.data(ImageModel::ThumbnailRole);
    // reset to default thumbnail size
    model->setData(index, QVariant(), ImageModel::ThumbnailRole);

    return thumbData.value<QPixmap>();
}

QPixmap ImageDelegate::thumbnailPixmap(const QModelIndex& index) const
{
    Q_D(const ImageDelegate);
    return retrieveThumbnailPixmap(index, d->thumbSize.size());
}

void ImageDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const ImageDelegate);
    ImageInfo info = ImageModel::retrieveImageInfo(index);

    if (info.isNull())
    {
        return;
    }

    // state of painter must not be changed
    p->save();
    p->translate(option.rect.topLeft());

    QRect r;
    ThemeEngine* te = ThemeEngine::instance();
    bool isSelected = (option.state & QStyle::State_Selected);

    // Thumbnail
    QPixmap pix;

    if (isSelected)
    {
        pix = d->selPixmap;
    }
    else
    {
        pix = d->regPixmap;
    }

    QRect actualPixmapRect = drawThumbnail(p, d->pixmapRect, pix, thumbnailPixmap(index));

    if (!actualPixmapRect.isNull())
    {
        const_cast<ImageDelegate*>(this)->updateActualPixmapRect(index, actualPixmapRect);
    }

    if (!d->ratingRect.isNull())
    {
        drawRating(p, index, d->ratingRect, info.rating(), isSelected);
    }

    // Draw Color Label rectangle
    drawColorLabelRect(p, option, isSelected, info.colorLabel());

    p->setPen(isSelected ? te->textSelColor() : te->textRegColor());

    // If there is ImageHistory present, paint a small icon over the thumbnail to indicate that this is derived image
    if (info.hasImageHistory())
    {
        p->drawPixmap(d->pixmapRect.right()-24, d->pixmapRect.bottom()-24, KIcon("svn_switch").pixmap(22, 22));
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
    {
        drawFocusRect(p, option, isSelected);
    }

    if (d->drawMouseOverFrame)
    {
        drawMouseOverRect(p, option);
    }

    // Draw Pick Label icon
    int pickId = info.pickLabel();
    if (pickId != NoPickLabel)
    {
        QIcon icon;
        int size = KIconLoader::SizeSmallMedium;

        if (pickId == RejectedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-red", KIconLoader::NoGroup, size);
        }
        else if (pickId == PendingLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-yellow", KIconLoader::NoGroup, size);
        }
        else if (pickId == AcceptedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-green", KIconLoader::NoGroup, size);
        }
        icon.paint(p, d->pixmapRect.left()+2, d->pixmapRect.bottom()-size+2, size, size);
    }

    p->restore();

    drawDelegates(p, option, index);
}

QPixmap ImageDelegate::pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const
{
    QPixmap icon;

    if (!indexes.isEmpty())
    {
        icon = thumbnailPixmap(indexes.first());
    }

    return makeDragPixmap(option, indexes, icon);
}

bool ImageDelegate::acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                   QRect* toolTipRect) const
{
    return onActualPixmapRect(pos, visualRect, index, toolTipRect);
}

bool ImageDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                      QRect* activationRect) const
{
    return onActualPixmapRect(pos, visualRect, index, activationRect);
}

bool ImageDelegate::onActualPixmapRect(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                       QRect* returnRect) const
{
    QRect actualRect = actualPixmapRect(index);

    if (actualRect.isNull())
    {
        return false;
    }

    actualRect.translate(visualRect.topLeft());

    if (returnRect)
    {
        *returnRect = actualRect;
    }

    return actualRect.contains(pos);
}

void ImageDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ImageDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->setDefaultViewOptions(option);
    }

    ItemViewImageDelegate::setDefaultViewOptions(option);
}

void ImageDelegate::invalidatePaintingCache()
{
    Q_D(ImageDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->invalidatePaintingCache();
    }

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

void ImageDelegate::clearModelDataCaches()
{
    Q_D(ImageDelegate);
    d->actualPixmapRectCache.clear();
}

void ImageDelegate::modelChanged()
{
    Q_D(ImageDelegate);
    clearModelDataCaches();
    setModel(d->currentView ? d->currentView->model() : 0);
}

void ImageDelegate::modelContentsChanged()
{
    clearModelDataCaches();
}

QRect ImageDelegate::actualPixmapRect(const QModelIndex& index) const
{
    Q_D(const ImageDelegate);
    // We do not recompute if not found. Assumption is cache is always properly updated.
    QRect* rect = d->actualPixmapRectCache.object(index.row());

    if (rect)
    {
        return *rect;
    }
    else
    {
        return d->pixmapRect;
    }
}

void ImageDelegate::updateActualPixmapRect(const QModelIndex& index, const QRect& rect)
{
    Q_D(ImageDelegate);
    QRect* old = d->actualPixmapRectCache.object(index.row());

    if (!old || *old != rect)
    {
        d->actualPixmapRectCache.insert(index.row(), new QRect(rect));
    }
}

} // namespace Digikam
