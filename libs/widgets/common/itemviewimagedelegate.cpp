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

#include "itemviewimagedelegate.moc"
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

#include "imagedelegateoverlay.h"
#include "themeengine.h"

namespace Digikam
{

ItemViewImageDelegatePrivate::ItemViewImageDelegatePrivate()
{
    spacing        = 0;
    thumbSize      = 0;

    // Pre-computed star polygon for a 15x15 pixmap.
    starPolygon << QPoint(0,  6);
    starPolygon << QPoint(5,  5);
    starPolygon << QPoint(7,  0);
    starPolygon << QPoint(9,  5);
    starPolygon << QPoint(14, 6);
    starPolygon << QPoint(10, 9);
    starPolygon << QPoint(11, 14);
    starPolygon << QPoint(7,  11);
    starPolygon << QPoint(3,  14);
    starPolygon << QPoint(4,  9);

    starPolygonSize = QSize(15, 15);

    ratingPixmaps   = QVector<QPixmap>(10);
}

void ItemViewImageDelegatePrivate::init(ItemViewImageDelegate *_q)
{
    q = _q;

    q->connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

ItemViewImageDelegate::ItemViewImageDelegate(DCategorizedView *parent)
             : DItemDelegate(parent), d_ptr(new ItemViewImageDelegatePrivate)
{
    d_ptr->init(this);
}

ItemViewImageDelegate::ItemViewImageDelegate(ItemViewImageDelegatePrivate &dd, DCategorizedView *parent)
             : DItemDelegate(parent), d_ptr(&dd)
{
    d_ptr->init(this);
}

ItemViewImageDelegate::~ItemViewImageDelegate()
{
    Q_D(ItemViewImageDelegate);
    delete d;
}

void ItemViewImageDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewImageDelegate);
    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewImageDelegate::setSpacing(int spacing)
{
    Q_D(ItemViewImageDelegate);
    if (d->spacing == spacing)
        return;
    d->spacing = spacing;
    invalidatePaintingCache();
}

void ItemViewImageDelegate::installOverlay(ImageDelegateOverlay *overlay)
{
    Q_D(ItemViewImageDelegate);
    overlay->setDelegate(this);
    d->overlays << overlay;
    overlay->setActive(true);
}

void ItemViewImageDelegate::removeOverlay(ImageDelegateOverlay *overlay)
{
    Q_D(ItemViewImageDelegate);
    overlay->setActive(false);
    overlay->setDelegate(0);
    d->overlays.removeAll(overlay);
}

void ItemViewImageDelegate::removeAllOverlays()
{
    Q_D(ItemViewImageDelegate);
    foreach (ImageDelegateOverlay *overlay, d->overlays)
    {
        overlay->setActive(false);
        overlay->setDelegate(0);
        overlay->setView(0);
    }
    d->overlays.clear();
}

QRect ItemViewImageDelegate::rect() const
{
    Q_D(const ItemViewImageDelegate);
    return d->rect;
}

void ItemViewImageDelegate::setRatingEdited(const QModelIndex &index)
{
    Q_D(ItemViewImageDelegate);
    d->editingRating = index;
}

void ItemViewImageDelegate::mouseMoved(QMouseEvent *e, const QRect& visualRect, const QModelIndex& index)
{
    Q_D(ItemViewImageDelegate);
    foreach (ImageDelegateOverlay *overlay, d->overlays)
        overlay->mouseMoved(e, visualRect, index);
}

/*
void ItemViewImageDelegate::paint(QPainter * p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(ItemViewImageDelegate);
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

    QVariant thumbData = index.data(ImageModel::ThumbnailRole);
    if (!thumbData.isNull())
    {
        QPixmap thumbnail = thumbData.value<QPixmap>();
        r = d->pixmapRect;
        /*p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                      r.y() + (r.height()-thumbnail.height())/2,
                      thumbnail);* /

        QRect actualPixmapRect(r.x() + (r.width()-thumbnail.width())/2,
                               r.y() + (r.height()-thumbnail.height())/2,
                               thumbnail.width(), thumbnail.height());
        const_cast<ItemViewImageDelegate*>(this)->updateActualPixmapRect(info.id(), actualPixmapRect);

        /*p->save();
        QRegion pixmapClipRegion = QRegion(d->rect) - QRegion(actualPixmapRect);
        p->setClipRegion(pixmapClipRegion);* /
        p->drawPixmap(0, 0, pix);

        QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());
        p->drawPixmap(actualPixmapRect.x()-3, actualPixmapRect.y()-3, borderPix);

        p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                      r.y() + (r.height()-thumbnail.height())/2,
                      thumbnail);
        //p->restore();
    }
    else
    {
        // simplified
        p->drawPixmap(0, 0, pix);
    }

    if (settings->getIconShowRating())
    {
        r = d->ratingRect;

        if (d->editingRating != index)
            p->drawPixmap(r, ratingPixmap(info.rating(), isSelected));
        else
            p->drawPixmap(r, ratingPixmap(-1, isSelected));
    }

    if (settings->getIconShowName())
    {
        r = d->nameRect;
        p->setFont(d->fontReg);
        p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), info.name()));
    }

    p->setFont(d->fontCom);

    if (settings->getIconShowComments())
    {
        QString comments = info.comment();

        r = d->commentsRect;
        p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), comments));
    }

    p->setFont(d->fontXtra);

    if (settings->getIconShowDate())
    {
        QDateTime date(info.dateTime());

        r = d->dateRect;
        p->setFont(d->fontXtra);
        QString str = dateToString(date);
        str = i18nc("date of image creation", "created: %1",str);
        p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), str));
    }

    if (settings->getIconShowModDate())
    {
        QDateTime date(info.modDateTime());

        r = d->modDateRect;
        p->setFont(d->fontXtra);
        QString str = dateToString(date);
        str = i18nc("date of last image modification", "modified: %1",str);
        p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), str));
    }

    if (settings->getIconShowResolution())
    {
        QSize dims = info.dimensions();
        if (dims.isValid())
        {
            QString mpixels, resolution;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            resolution = (!dims.isValid()) ? i18nc("unknown image resolution", "Unknown")
                                           : i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                                                   dims.width(),dims.height(),mpixels);
            r = d->resolutionRect;
            p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), resolution));
        }
    }

    if (settings->getIconShowSize())
    {
        r = d->sizeRect;
        p->drawText(r, Qt::AlignCenter,
                    squeezedTextCached(p, r.width(), KIO::convertSize(info.fileSize())));
    }

    p->setFont(d->fontCom);
    p->setPen(isSelected ? te->textSpecialSelColor() : te->textSpecialRegColor());

    if (settings->getIconShowTags())
    {
        QString tags = AlbumManager::instance()->tagNames(info.tagIds()).join(", ");

        r = d->tagRect;
        p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tags));
    }

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        r = d->rect;
        p->setPen(QPen(isSelected ? te->textSelColor() : te->textRegColor(), 1, Qt::DotLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }

    if (option.state & QStyle::State_MouseOver)
    {
        r = d->rect;
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, r.width()-3, r.height()-3);
    }

    p->restore();

    foreach (ImageDelegateOverlay *overlay, d->overlays)
        overlay->paint(p, option, index);
}

QPixmap ItemViewImageDelegate::pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const
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
}*/

QSize ItemViewImageDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    Q_D(const ItemViewImageDelegate);
    return d->rect.size();
}

QSize ItemViewImageDelegate::gridSize() const
{
    Q_D(const ItemViewImageDelegate);
    return d->gridSize;
}

bool ItemViewImageDelegate::acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect *toolTipRect) const
{
    return true;
}

bool ItemViewImageDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect *activationRect) const
{
    return true;
}

void ItemViewImageDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewImageDelegate);
    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewImageDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewImageDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewImageDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewImageDelegate);
    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();
    if (oldGridSize != d->gridSize)
    {
        emit gridSizeChanged(d->gridSize);
        // emit sizeHintChanged(QModelIndex());
    }

    emit visualChange();
}

/*
void ItemViewImageDelegate::updateSizeRectsAndPixmaps()
{
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
}*/

QPixmap ItemViewImageDelegate::ratingPixmap(int rating, bool selected) const
{
    Q_D(const ItemViewImageDelegate);
    if (rating < 1 || rating > 5)
    {
        /*
        QPixmap pix;
        if (selected)
            pix = d->selPixmap.copy(d->ratingRect);
        else
            pix = d->regPixmap.copy(d->ratingRect);

        return pix;
        */
        return QPixmap();
    }

    --rating;
    if (selected)
        return d->ratingPixmaps[5 + rating];
    else
        return d->ratingPixmaps[rating];
}

} // namespace Digikam
