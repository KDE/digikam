/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "itemviewshowfotodelegate.h"

// Qt includes

#include <QApplication>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "thememanager.h"
#include "imagescanner.h"
#include "imagepropertiestab.h"
#include "showfotoiteminfo.h"
#include "colorlabelwidget.h"
#include "itemviewshowfotodelegatepriv.h"

namespace ShowFoto
{

ItemViewShowfotoDelegatePrivate::ItemViewShowfotoDelegatePrivate()
{
    q             = 0;
    spacing       = 0;
    thumbSize     = 0;

    // painting constants
    radius        = 3;
    margin        = 5;
}

void ItemViewShowfotoDelegatePrivate::init(ItemViewShowfotoDelegate* const _q)
{
    q = _q;

    q->connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ItemViewShowfotoDelegatePrivate::clearRects()
{
    gridSize   = QSize(0, 0);
    rect       = QRect(0, 0, 0, 0);
}

// ---- ItemViewShowfotoDelegate -----------------------------------------------

ItemViewShowfotoDelegate::ItemViewShowfotoDelegate(QObject* const parent)
    : DItemDelegate(parent),
      d_ptr(new ItemViewShowfotoDelegatePrivate)
{
    d_ptr->init(this);
}

ItemViewShowfotoDelegate::ItemViewShowfotoDelegate(ItemViewShowfotoDelegatePrivate& dd, QObject* const parent)
    : DItemDelegate(parent), d_ptr(&dd)
{
    d_ptr->init(this);
}

ItemViewShowfotoDelegate::~ItemViewShowfotoDelegate()
{
    Q_D(ItemViewShowfotoDelegate);
    removeAllOverlays();
    delete d;
}

ThumbnailSize ItemViewShowfotoDelegate::thumbnailSize() const
{
    Q_D(const ItemViewShowfotoDelegate);
    return d->thumbSize;
}

void ItemViewShowfotoDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewShowfotoDelegate);

    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewShowfotoDelegate::setSpacing(int spacing)
{
    Q_D(ItemViewShowfotoDelegate);

    if (d->spacing == spacing)
    {
        return;
    }

    d->spacing = spacing;
    invalidatePaintingCache();
}

int ItemViewShowfotoDelegate::spacing() const
{
    Q_D(const ItemViewShowfotoDelegate);
    return d->spacing;
}

QRect ItemViewShowfotoDelegate::rect() const
{
    Q_D(const ItemViewShowfotoDelegate);
    return d->rect;
}

QRect ItemViewShowfotoDelegate::pixmapRect() const
{
    return QRect();
}

QRect ItemViewShowfotoDelegate::imageInformationRect() const
{
    return QRect();
}

QSize ItemViewShowfotoDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ItemViewShowfotoDelegate);
    return d->rect.size();
}

QSize ItemViewShowfotoDelegate::gridSize() const
{
    Q_D(const ItemViewShowfotoDelegate);
    return d->gridSize;
}

bool ItemViewShowfotoDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ItemViewShowfotoDelegate::acceptsActivation(const QPoint& , const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ItemViewShowfotoDelegate::asDelegate()
{
    return this;
}

void ItemViewShowfotoDelegate::overlayDestroyed(QObject* o)
{
    ImageDelegateOverlayContainer::overlayDestroyed(o);
}

void ItemViewShowfotoDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection AbstractShowfotoItemDelegate -> ItemViewShowfotoDelegate -> ImageDelegateOverlayContainer
    ImageDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ItemViewShowfotoDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewShowfotoDelegate);
    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewShowfotoDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewShowfotoDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewShowfotoDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewShowfotoDelegate);
    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();

    if (oldGridSize != d->gridSize)
    {
        emit gridSizeChanged(d->gridSize);
        // emit sizeHintChanged(QModelIndex());
    }

    emit visualChange();
}

QRect ItemViewShowfotoDelegate::drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background,
                                              const QPixmap& thumbnail) const
{
    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
    {
        using namespace Digikam;
        return QRect();
    }

    QRect r = thumbRect;

    QRect actualPixmapRect(r.x() + (r.width()-thumbnail.width())/2,
                           r.y() + (r.height()-thumbnail.height())/2,
                           thumbnail.width(), thumbnail.height());

    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());
    p->drawPixmap(actualPixmapRect.x()-3, actualPixmapRect.y()-3, borderPix);

    p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                  r.y() + (r.height()-thumbnail.height())/2,
                  thumbnail);
    return actualPixmapRect;
}

void ItemViewShowfotoDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewShowfotoDelegate);
    p->setFont(d->fontReg);
    p->drawText(nameRect, Qt::AlignCenter, name);//squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewShowfotoDelegate::drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewShowfotoDelegate);
    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str         = i18nc("date of image creation", "created: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, str);//squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewShowfotoDelegate::drawImageFormat(QPainter* p, const QRect& r, const QString& mime) const
{
    Q_D(const ItemViewShowfotoDelegate);

    if (!mime.isEmpty() && !r.isNull())
    {
        QString type = mime;

        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        QRect bRect = p->boundingRect(r, Qt::AlignBottom | Qt::AlignHCenter, type.toUpper());
        bRect.adjust(1, 1, -1, -1);
        bRect.translate(0, 1);

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, Qt::AlignBottom | Qt::AlignHCenter, type.toUpper());

        p->restore();
    }
}

void ItemViewShowfotoDelegate::drawGeolocationIndicator(QPainter* p, const QRect& r) const
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

void ItemViewShowfotoDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewShowfotoDelegate);

    if (dims.isValid())
    {
        p->setFont(d->fontXtra);
        QString mpixels, resolution;
        mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);

        if (dims.isValid())
        {
            resolution = i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                               dims.width(), dims.height(), mpixels);
        }
        else
        {
            resolution = i18nc("unknown image resolution", "Unknown");
        }

        p->drawText(dimsRect, Qt::AlignCenter, resolution);
    }
}

void ItemViewShowfotoDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ItemViewShowfotoDelegate);
    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, ImagePropertiesTab::humanReadableBytesCount(bytes));
}

void ItemViewShowfotoDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                                             bool isSelected) const
{
    Q_D(const ItemViewShowfotoDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                                  : qApp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewShowfotoDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewShowfotoDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewShowfotoDelegate::prepareFonts()
{
    Q_D(ItemViewShowfotoDelegate);

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

void ItemViewShowfotoDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ItemViewShowfotoDelegate);

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

void ItemViewShowfotoDelegate::prepareBackground()
{
    Q_D(ItemViewShowfotoDelegate);

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

} // namespace ShowFoto
