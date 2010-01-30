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

bool ItemViewImageDelegate::acceptsToolTip(const QPoint&, const QRect&, const QModelIndex&, QRect *) const
{
    return true;
}

bool ItemViewImageDelegate::acceptsActivation(const QPoint& , const QRect&, const QModelIndex&, QRect *) const
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

QRect ItemViewImageDelegate::drawThumbnail(QPainter *p, const QRect& thumbRect, const QPixmap& background, const QPixmap& thumbnail) const
{
    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
        return QRect();

    QRect r = thumbRect;
    /*p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                    r.y() + (r.height()-thumbnail.height())/2,
                    thumbnail);*/

    QRect actualPixmapRect(r.x() + (r.width()-thumbnail.width())/2,
                           r.y() + (r.height()-thumbnail.height())/2,
                           thumbnail.width(), thumbnail.height());

    /*p->save();
    QRegion pixmapClipRegion = QRegion(d->rect) - QRegion(actualPixmapRect);
    p->setClipRegion(pixmapClipRegion);*/
    //p->drawPixmap(0, 0, background);

    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());
    p->drawPixmap(actualPixmapRect.x()-3, actualPixmapRect.y()-3, borderPix);

    p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                  r.y() + (r.height()-thumbnail.height())/2,
                  thumbnail);
    //p->restore();
    return actualPixmapRect;
}

void ItemViewImageDelegate::drawRating(QPainter *p, const QModelIndex& index, const QRect& ratingRect, int rating, bool isSelected) const
{
    Q_D(const ItemViewImageDelegate);
    if (d->editingRating != index)
        p->drawPixmap(ratingRect, ratingPixmap(rating, isSelected));
    /*else
        p->drawPixmap(r, ratingPixmap(-1, isSelected));*/
}

void ItemViewImageDelegate::drawName(QPainter *p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontReg);
    p->drawText(nameRect, Qt::AlignCenter, squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewImageDelegate::drawComments(QPainter *p, const QRect& commentsRect, const QString& comments) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontCom);
    p->drawText(commentsRect, Qt::AlignCenter, squeezedTextCached(p, commentsRect.width(), comments));
}

void ItemViewImageDelegate::drawCreationDate(QPainter *p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str = i18nc("date of image creation", "created: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewImageDelegate::drawModificationDate(QPainter *p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str = i18nc("date of last image modification", "modified: %1",str);
    p->drawText(dateRect, Qt::AlignCenter, squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewImageDelegate::drawImageSize(QPainter *p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewImageDelegate);
    if (dims.isValid())
    {
        p->setFont(d->fontXtra);
        QString mpixels, resolution;
        mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
        if (dims.isValid())
            resolution = i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                               dims.width(), dims.height(), mpixels);
        else
            resolution = i18nc("unknown image resolution", "Unknown");
        p->drawText(dimsRect, Qt::AlignCenter, squeezedTextCached(p, dimsRect.width(), resolution));
    }
}

void ItemViewImageDelegate::drawFileSize(QPainter *p, const QRect& r, int bytes) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), KIO::convertSize(bytes)));
}

void ItemViewImageDelegate::drawTags(QPainter *p, const QRect& r, const QString& tagsString, bool isSelected) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontCom);
    p->setPen(isSelected ? ThemeEngine::instance()->textSpecialSelColor()
                         : ThemeEngine::instance()->textSpecialRegColor());

    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tagsString));
}

void ItemViewImageDelegate::drawStateRects(QPainter *p, const QStyleOptionViewItem& option, bool isSelected) const
{
    Q_D(const ItemViewImageDelegate);
    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? ThemeEngine::instance()->textSelColor()
                                  : ThemeEngine::instance()->textRegColor(),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewImageDelegate::drawDelegates(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const ItemViewImageDelegate);
    foreach (ImageDelegateOverlay *overlay, d->overlays)
        overlay->paint(p, option, index);
}

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
