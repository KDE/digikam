/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemviewimagedelegate.h"
#include "itemviewimagedelegatepriv.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCache>
#include <QPainter>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itempropertiestab.h"
#include "itemdelegateoverlay.h"
#include "thememanager.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"

namespace Digikam
{

ItemViewItemDelegatePrivate::ItemViewItemDelegatePrivate()
{
    q         = 0;
    spacing   = 0;
    thumbSize = ThumbnailSize(0);

    // painting constants
    radius    = 3;
    margin    = 5;

    makeStarPolygon();

    ratingPixmaps = QVector<QPixmap>(10);
}

void ItemViewItemDelegatePrivate::init(ItemViewItemDelegate* const _q)
{
    q = _q;

    q->connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ItemViewItemDelegatePrivate::clearRects()
{
    gridSize   = QSize(0, 0);
    rect       = QRect(0, 0, 0, 0);
    ratingRect = QRect(0, 0, 0, 0);
}

void ItemViewItemDelegatePrivate::makeStarPolygon()
{
    // Pre-computed star polygon for a 15x15 pixmap.
    starPolygon     = RatingWidget::starPolygon();
    starPolygonSize = QSize(15, 15);
}

ItemViewItemDelegate::ItemViewItemDelegate(QObject* const parent)
    : DItemDelegate(parent),
      d_ptr(new ItemViewItemDelegatePrivate)
{
    d_ptr->init(this);
}

ItemViewItemDelegate::ItemViewItemDelegate(ItemViewItemDelegatePrivate& dd, QObject* const parent)
    : DItemDelegate(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

ItemViewItemDelegate::~ItemViewItemDelegate()
{
    Q_D(ItemViewItemDelegate);

    removeAllOverlays();
    delete d;
}

ThumbnailSize ItemViewItemDelegate::thumbnailSize() const
{
    Q_D(const ItemViewItemDelegate);

    return d->thumbSize;
}

void ItemViewItemDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewItemDelegate);

    if (d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewItemDelegate::setSpacing(int spacing)
{
    Q_D(ItemViewItemDelegate);

    if (d->spacing == spacing)
    {
        return;
    }

    d->spacing = spacing;
    invalidatePaintingCache();
}

int ItemViewItemDelegate::spacing() const
{
    Q_D(const ItemViewItemDelegate);

    return d->spacing;
}

QRect ItemViewItemDelegate::rect() const
{
    Q_D(const ItemViewItemDelegate);

    return d->rect;
}

QRect ItemViewItemDelegate::pixmapRect() const
{
    return QRect();
}

QRect ItemViewItemDelegate::imageInformationRect() const
{
    return QRect();
}

QRect ItemViewItemDelegate::ratingRect() const
{
    Q_D(const ItemViewItemDelegate);

    return d->ratingRect;
}

void ItemViewItemDelegate::setRatingEdited(const QModelIndex& index)
{
    Q_D(ItemViewItemDelegate);

    d->editingRating = index;
}

QSize ItemViewItemDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ItemViewItemDelegate);

    return d->rect.size();
}

QSize ItemViewItemDelegate::gridSize() const
{
    Q_D(const ItemViewItemDelegate);

    return d->gridSize;
}

bool ItemViewItemDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ItemViewItemDelegate::acceptsActivation(const QPoint& , const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ItemViewItemDelegate::asDelegate()
{
    return this;
}

void ItemViewItemDelegate::overlayDestroyed(QObject* o)
{
    ItemDelegateOverlayContainer::overlayDestroyed(o);
}

void ItemViewItemDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection DItemDelegate -> ItemViewItemDelegate -> ItemDelegateOverlayContainer
    ItemDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ItemViewItemDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewItemDelegate);

    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewItemDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewItemDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewItemDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewItemDelegate);

    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();

    if (oldGridSize != d->gridSize)
    {
        emit gridSizeChanged(d->gridSize);
        //emit sizeHintChanged(QModelIndex());
    }

    emit visualChange();
}

QRect ItemViewItemDelegate::drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background,
                                           const QPixmap& thumbnail, bool isGrouped) const
{
    Q_D(const ItemViewItemDelegate);

    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
    {
        return QRect();
    }

    QRect r = thumbRect;
/*
    p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                  r.y() + (r.height()-thumbnail.height())/2,
                  thumbnail);
*/

    QRect actualPixmapRect(r.x() + (r.width()-thumbnail.width())/2,
                           r.y() + (r.height()-thumbnail.height())/2,
                           thumbnail.width(), thumbnail.height());
/*
    p->save();
    QRegion pixmapClipRegion = QRegion(d->rect) - QRegion(actualPixmapRect);
    p->setClipRegion(pixmapClipRegion);

    p->drawPixmap(0, 0, background);
*/
    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size(), isGrouped);

    if (isGrouped)
    {
        const int xPadding = (borderPix.width()-actualPixmapRect.width())/2;
        const int yPadding = (borderPix.height()-actualPixmapRect.height())/2;

        p->drawPixmap(actualPixmapRect.x()-xPadding,
                      actualPixmapRect.y()-yPadding, borderPix);

        QPixmap groupThumbnail = thumbnail.scaled(thumbnail.width()-10,
                                                  thumbnail.height()-10,
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);

        p->drawPixmap(r.x() + (r.width()-groupThumbnail.width())/2,
                      r.y() + (r.height()-groupThumbnail.height())/2,
                      groupThumbnail);
    }
    else
    {
        p->drawPixmap(actualPixmapRect.x()-d->radius,
                      actualPixmapRect.y()-d->radius, borderPix);

        p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                      r.y() + (r.height()-thumbnail.height())/2,
                      thumbnail);
    }

    //p->restore();
    return actualPixmapRect;
}

void ItemViewItemDelegate::drawRating(QPainter* p, const QModelIndex& index, const QRect& ratingRect,
                                       int rating, bool isSelected) const
{
    Q_D(const ItemViewItemDelegate);

    if (d->editingRating != index)
    {
        p->drawPixmap(ratingRect, ratingPixmap(rating, isSelected));
    }
}

void ItemViewItemDelegate::drawSpecialInfo(QPainter* p,const QRect& r, const QString& text) const
{
    Q_D(const ItemViewItemDelegate);

    if (!text.isEmpty() && !r.isNull())
    {
        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        int flags   = Qt::AlignCenter | Qt::TextWordWrap;
        QRect bRect = p->boundingRect(r, flags, text);

        if (bRect.width() > r.width())
        {
            flags = Qt::AlignCenter | Qt::TextWrapAnywhere;
            bRect = p->boundingRect(r, flags, text);
        }

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, flags, text);

        p->restore();
    }
}

void ItemViewItemDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontReg);
    // NOTE: in case of file name are long, use squeezedTextCached to adjust string elide mode.
    // See bug #278664 for details
    p->drawText(nameRect, Qt::AlignCenter, squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewItemDelegate::drawTitle(QPainter* p, const QRect& titleRect, const QString& title) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontReg);
    p->drawText(titleRect, Qt::AlignCenter, squeezedTextCached(p, titleRect.width(), title));
}

void ItemViewItemDelegate::drawComments(QPainter* p, const QRect& commentsRect, const QString& comments) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontCom);
    p->drawText(commentsRect, Qt::AlignCenter, squeezedTextCached(p, commentsRect.width(), comments));
}

void ItemViewItemDelegate::drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    //str         = i18nc("date of image creation", "created: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, str); //squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewItemDelegate::drawModificationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str         = i18nc("date of last image modification", "Mod.: %1",str);
    p->drawText(dateRect, Qt::AlignCenter, str); //squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewItemDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewItemDelegate);

    if (dims.isValid())
    {
        p->setFont(d->fontXtra);
        QString mpixels, resolution;
        mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);

        if (dims.isValid())
            resolution = i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                               dims.width(), dims.height(), mpixels);
        else
        {
            resolution = i18nc("unknown image resolution", "Unknown");
        }

        p->drawText(dimsRect, Qt::AlignCenter, resolution); //squeezedTextCached(p, dimsRect.width(), resolution));
    }
}

void ItemViewItemDelegate::drawAspectRatio(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontXtra);
    QString resolution;

    if (dims.isValid())
    {
        ItemPropertiesTab::aspectRatioToString(dims.width(), dims.height(), resolution);
    }
    else
    {
        resolution = i18nc("unknown image resolution", "Unknown");
    }

    p->drawText(dimsRect, Qt::AlignCenter, resolution); //squeezedTextCached(p, dimsRect.width(), resolution));
}

void ItemViewItemDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, ItemPropertiesTab::humanReadableBytesCount(bytes));
}

void ItemViewItemDelegate::drawTags(QPainter* p, const QRect& r, const QString& tagsString,
                                     bool isSelected) const
{
    Q_D(const ItemViewItemDelegate);

    p->setFont(d->fontCom);
    p->setPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                         : qApp->palette().color(QPalette::Link));

    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tagsString));
}

void ItemViewItemDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                                          bool isSelected) const
{
    Q_D(const ItemViewItemDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                                  : qApp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewItemDelegate::drawImageFormat(QPainter* p, const QRect& r, const QString& f, bool drawTop) const
{
    Q_D(const ItemViewItemDelegate);

    if (!f.isEmpty() && !r.isNull())
    {
        Qt::Alignment alignment = Qt::AlignBottom;

        if (drawTop)
            alignment = Qt::AlignTop;

        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        QRect bRect = p->boundingRect(r, alignment | Qt::AlignHCenter, f.toUpper());
        bRect.adjust(0, 0, 0, -2);

        if (!drawTop)
            bRect.translate(0, 1);

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, Qt::AlignBottom | Qt::AlignHCenter, f.toUpper());

        p->restore();
    }
}

void ItemViewItemDelegate::drawPickLabelIcon(QPainter* p, const QRect& r, int pickId) const
{
    // Draw Pick Label icon
    if (pickId != NoPickLabel)
    {
        QIcon icon;

        if (pickId == RejectedLabel)
        {
            icon = QIcon::fromTheme(QLatin1String("flag-red"));
        }
        else if (pickId == PendingLabel)
        {
            icon = QIcon::fromTheme(QLatin1String("flag-yellow"));
        }
        else if (pickId == AcceptedLabel)
        {
            icon = QIcon::fromTheme(QLatin1String("flag-green"));
        }

        icon.paint(p, r);
    }
}

void ItemViewItemDelegate::drawPanelSideIcon(QPainter* p, bool left, bool right) const
{
    Q_D(const ItemViewItemDelegate);

    const int iconSize = qBound(16, d->rect.width() / 8 - 2, 48);

    if (left)
    {
        QRect r(3, d->rect.height()/2 - iconSize/2, iconSize, iconSize);
        QIcon icon = QIcon::fromTheme(QLatin1String("go-previous"));
        icon.paint(p, r);
    }

    if (right)
    {
        QRect r(d->rect.width() - 3 - iconSize, d->rect.height()/2 - iconSize/2, iconSize, iconSize);
        QIcon icon = QIcon::fromTheme(QLatin1String("go-next"));
        icon.paint(p, r);
    }
}

void ItemViewItemDelegate::drawGeolocationIndicator(QPainter* p, const QRect& r) const
{
    if (!r.isNull())
    {
        QIcon icon(QIcon::fromTheme(QLatin1String("globe")).pixmap(r.size()));
        qreal op = p->opacity();
        p->setOpacity(0.5);
        icon.paint(p, r);
        p->setOpacity(op);
    }
}

void ItemViewItemDelegate::drawGroupIndicator(QPainter* p, const QRect& r,
                                               int numberOfGroupedImages, bool open) const
{
    if (numberOfGroupedImages)
    {
        QIcon icon;

        if (open)
        {
            icon = QIcon::fromTheme(QLatin1String("folder-open")); //image-stack-open
        }
        else
        {
            icon = QIcon::fromTheme(QLatin1String("folder")); //image-stack
        }

        qreal op = p->opacity();
        p->setOpacity(0.5);
        icon.paint(p, r);
        p->setOpacity(op);

        QString text = QString::number(numberOfGroupedImages);
        p->drawText(r, Qt::AlignCenter, text);
    }
}

void ItemViewItemDelegate::drawColorLabelRect(QPainter* p, const QStyleOptionViewItem& option,
                                               bool isSelected, int colorId) const
{
    Q_D(const ItemViewItemDelegate);
    Q_UNUSED(option);
    Q_UNUSED(isSelected);

    if (colorId > NoColorLabel)
    {
        // This draw a simple rectangle around item.
        p->setPen(QPen(ColorLabelWidget::labelColor((ColorLabel)colorId), 5, Qt::SolidLine));
        p->drawRect(3, 3, d->rect.width()-7, d->rect.height()-7);
    }
}

void ItemViewItemDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewItemDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewItemDelegate::prepareFonts()
{
    Q_D(ItemViewItemDelegate);

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
}

void ItemViewItemDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ItemViewItemDelegate);

    QFontMetrics fm(d->fontReg);
    d->oneRowRegRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       QLatin1String("XXXXXXXXX"));
    fm = QFontMetrics(d->fontCom);
    d->oneRowComRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       QLatin1String("XXXXXXXXX"));
    fm = QFontMetrics(d->fontXtra);
    d->oneRowXtraRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                        Qt::AlignTop | Qt::AlignHCenter,
                                        QLatin1String("XXXXXXXXX"));
}

void ItemViewItemDelegate::prepareBackground()
{
    Q_D(ItemViewItemDelegate);

    if (!d->rect.isValid())
    {
        d->regPixmap = QPixmap();
        d->selPixmap = QPixmap();
    }
    else
    {
        d->regPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->regPixmap.fill(qApp->palette().color(QPalette::Base));
        QPainter p1(&d->regPixmap);
        p1.setPen(qApp->palette().color(QPalette::Midlight));
        p1.drawRect(0, 0, d->rect.width()-1, d->rect.height()-1);

        d->selPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->selPixmap.fill(qApp->palette().color(QPalette::Highlight));
        QPainter p2(&d->selPixmap);
        p2.setPen(qApp->palette().color(QPalette::Midlight));
        p2.drawRect(0, 0, d->rect.width()-1, d->rect.height()-1);
    }
}

void ItemViewItemDelegate::prepareRatingPixmaps(bool composeOverBackground)
{
    // Please call this method after prepareBackground() and when d->ratingPixmap is set

    Q_D(ItemViewItemDelegate);

    if (!d->ratingRect.isValid())
    {
        return;
    }

    // We use antialiasing and want to pre-render the pixmaps.
    // So we need the background at the time of painting,
    // and the background may be a gradient, and will be different for selected items.
    // This makes 5*2 (small) pixmaps.

    for (int sel=0; sel<2; ++sel)
    {
        QPixmap basePix;

        if (composeOverBackground)
        {
            // do this once for regular, once for selected backgrounds
            if (sel)
            {
                basePix = d->selPixmap.copy(d->ratingRect);
            }
            else
            {
                basePix = d->regPixmap.copy(d->ratingRect);
            }
        }
        else
        {
            basePix = QPixmap(d->ratingRect.size());
            basePix.fill(Qt::transparent);
        }

        for (int rating = 1 ; rating <= 5 ; ++rating)
        {
            // we store first the 5 regular, then the 5 selected pixmaps, for simplicity
            int index = (sel * 5 + rating) - 1;

            // copy background
            d->ratingPixmaps[index] = basePix;
            // open a painter
            QPainter painter(&d->ratingPixmaps[index]);

            // use antialiasing
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setBrush(qApp->palette().color(QPalette::Link));
            QPen pen(qApp->palette().color(QPalette::Text));
            // set a pen which joins the lines at a filled angle
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);

            // move painter while drawing polygons
            painter.translate( lround((d->ratingRect.width() - d->margin - rating*(d->starPolygonSize.width()+1))/2.0) + 2, 0);

            for (int s = 0 ; s < rating ; ++s)
            {
                painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                painter.translate(d->starPolygonSize.width() + 1, 0);
            }
        }
    }
}

QPixmap ItemViewItemDelegate::ratingPixmap(int rating, bool selected) const
{
    Q_D(const ItemViewItemDelegate);

    if (rating < 1 || rating > 5)
    {
        return QPixmap();
    }

    --rating;

    if (selected)
    {
        return d->ratingPixmaps.at(5 + rating);
    }
    else
    {
        return d->ratingPixmaps.at(rating);
    }
}

} // namespace Digikam
