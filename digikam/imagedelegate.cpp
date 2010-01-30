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

    QRect                     ratingRect;
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
};

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
    QVariant thumbData = index.data(ImageModel::ThumbnailRole);
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
        {
            icon = thumbData.value<QPixmap>();
            if (qMax(icon.width(), icon.height()) > KIconLoader::SizeMedium)
                icon = icon.scaled(KIconLoader::SizeMedium, KIconLoader::SizeMedium,
                                   Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    if (icon.isNull())
    {
        icon = QPixmap(DesktopIcon("image-jp2", KIconLoader::SizeMedium));
    }

    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4, h+4);
    QString text(QString::number(indexes.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width()-1, pix.height()-1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2, 2, w, h, Qt::AlignLeft|Qt::AlignTop, text);
    r.setWidth(qMax(r.width(), r.height()));
    r.setHeight(qMax(r.width(), r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(option.font);
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);

    return pix;
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
    // ---- Reset values ----

    d->gridSize       = QSize(0, 0);
    d->rect           = QRect(0, 0, 0, 0);
    d->ratingRect     = QRect(0, 0, 0, 0);
    d->dateRect       = QRect(0, 0, 0, 0);
    d->modDateRect    = QRect(0, 0, 0, 0);
    d->pixmapRect     = QRect(0, 0, 0, 0);
    d->nameRect       = QRect(0, 0, 0, 0);
    d->commentsRect   = QRect(0, 0, 0, 0);
    d->resolutionRect = QRect(0, 0, 0, 0);
    d->sizeRect       = QRect(0, 0, 0, 0);
    d->tagRect        = QRect(0, 0, 0, 0);

    // ---- Calculate fonts ----

    d->fontReg  = d->font;
    d->fontCom  = d->font;
    d->fontXtra = d->font;
    d->fontCom.setItalic(true);

    int fnSz = d->fontReg.pointSize();
    if (fnSz > 0)
    {
        d->fontCom.setPointSize(fnSz-1);
        d->fontXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fontReg.pixelSize();
        d->fontCom.setPixelSize(fnSz-1);
        d->fontXtra.setPixelSize(fnSz-2);
    }

    // ---- Fixed sizes and metrics ----

    const int radius = 3;
    const int margin = 5;
    int w            = d->thumbSize.size() + 2*radius;

    QFontMetrics fm(d->fontReg);
    QRect oneRowRegRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fontCom);
    QRect oneRowComRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fontXtra);
    QRect oneRowXtraRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                           Qt::AlignTop | Qt::AlignHCenter,
                                           "XXXXXXXXX");

    QSize starPolygonSize(15, 15);

    // ---- Calculate rects ----

    int y = margin;

    d->pixmapRect = QRect(margin, y, w, d->thumbSize.size() + 2*radius);
    y = d->pixmapRect.bottom();

    const AlbumSettings *albumSettings = AlbumSettings::instance();
    if (albumSettings->getIconShowRating())
    {
        d->ratingRect = QRect(margin, y, w, starPolygonSize.height());
        y = d->ratingRect.bottom();
    }

    if (albumSettings->getIconShowName())
    {
        d->nameRect = QRect(margin, y, w-margin, oneRowRegRect.height());
        y = d->nameRect.bottom();
    }

    if (albumSettings->getIconShowComments())
    {
        d->commentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->commentsRect.bottom();
    }

    if (albumSettings->getIconShowDate())
    {
        d->dateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->dateRect.bottom();
    }

    if (albumSettings->getIconShowModDate())
    {
        d->modDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->modDateRect.bottom();
    }

    if (albumSettings->getIconShowResolution())
    {
        d->resolutionRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->resolutionRect.bottom() ;
    }

    if (albumSettings->getIconShowSize())
    {
        d->sizeRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->sizeRect.bottom();
    }

    if (albumSettings->getIconShowTags())
    {
        d->tagRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->tagRect.bottom();
    }

    d->rect = QRect(0, 0, w + 2*margin, y+margin+radius);

    d->gridSize  = QSize(d->rect.width() + d->spacing, d->rect.height() + d->spacing);

    // ---- Cached pixmaps ----

    d->regPixmap = ThemeEngine::instance()->thumbRegPixmap(d->rect.width(),
                                                               d->rect.height());

    d->selPixmap = ThemeEngine::instance()->thumbSelPixmap(d->rect.width(),
                                                               d->rect.height());

    // We use antialiasing and want to pre-render the pixmaps.
    // So we need the background at the time of painting,
    // and the background may be a gradient, and will be different for selected items.
    // This makes 5*2 (small) pixmaps.
    if (albumSettings->getIconShowRating())
    {
        for (int sel=0; sel<2; ++sel)
        {
            QPixmap basePix;

            // do this once for regular, once for selected backgrounds
            if (sel)
                basePix = d->selPixmap.copy(d->ratingRect);
            else
                basePix = d->regPixmap.copy(d->ratingRect);

            for (int rating=1; rating<=5; ++rating)
            {
                // we store first the 5 regular, then the 5 selected pixmaps, for simplicity
                int index = (sel * 5 + rating) - 1;

                // copy background
                d->ratingPixmaps[index] = basePix;
                // open a painter
                QPainter painter(&d->ratingPixmaps[index]);

                // use antialiasing
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setBrush(ThemeEngine::instance()->textSpecialRegColor());
                QPen pen(ThemeEngine::instance()->textRegColor());
                // set a pen which joins the lines at a filled angle
                pen.setJoinStyle(Qt::MiterJoin);
                painter.setPen(pen);

                // move painter while drawing polygons
                painter.translate( lround((d->ratingRect.width() - margin - rating*(starPolygonSize.width()+1))/2.0) + 2, 1 );
                for (int s=0; s<rating; ++s)
                {
                    painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                    painter.translate(starPolygonSize.width() + 1, 0);
                }
            }
        }
    }

    // ---- Drawing related caches ----

    d->actualPixmapRectCache.clear();
    clearCaches();
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
