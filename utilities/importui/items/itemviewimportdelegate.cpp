/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "itemviewimportdelegate.moc"
#include "itemviewimportdelegatepriv.h"

// Qt includes

#include <QCache>
#include <QPainter>
#include <QIcon>

// KDE includes

#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kapplication.h>

// Local includes

#include "imagedelegateoverlay.h"
#include "thememanager.h"
#include "imagescanner.h"

namespace Digikam
{

ItemViewImportDelegatePrivate::ItemViewImportDelegatePrivate()
{
    q             = 0;
    spacing       = 0;
    thumbSize     = 0;

    // painting constants
    radius        = 3;
    margin        = 5;

    makeStarPolygon();

    ratingPixmaps = QVector<QPixmap>(10);
}

void ItemViewImportDelegatePrivate::init(ItemViewImportDelegate* const _q)
{
    q = _q;

    q->connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ItemViewImportDelegatePrivate::clearRects()
{
    gridSize   = QSize(0, 0);
    rect       = QRect(0, 0, 0, 0);
    ratingRect = QRect(0, 0, 0, 0);
}

void ItemViewImportDelegatePrivate::makeStarPolygon()
{
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
}

// ---- ItemViewImportDelegate -----------------------------------------------

ItemViewImportDelegate::ItemViewImportDelegate(QObject* const parent)
    : DItemDelegate(parent), d_ptr(new ItemViewImportDelegatePrivate)
{
    d_ptr->init(this);
}

ItemViewImportDelegate::ItemViewImportDelegate(ItemViewImportDelegatePrivate& dd, QObject* const parent)
    : DItemDelegate(parent), d_ptr(&dd)
{
    d_ptr->init(this);
}

ItemViewImportDelegate::~ItemViewImportDelegate()
{
    Q_D(ItemViewImportDelegate);
    removeAllOverlays();
    delete d;
}

ThumbnailSize ItemViewImportDelegate::thumbnailSize() const
{
    Q_D(const ItemViewImportDelegate);
    return d->thumbSize;
}

void ItemViewImportDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewImportDelegate);

    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewImportDelegate::setSpacing(int spacing)
{
    Q_D(ItemViewImportDelegate);

    if (d->spacing == spacing)
    {
        return;
    }

    d->spacing = spacing;
    invalidatePaintingCache();
}

int ItemViewImportDelegate::spacing() const
{
    Q_D(const ItemViewImportDelegate);
    return d->spacing;
}

QRect ItemViewImportDelegate::rect() const
{
    Q_D(const ItemViewImportDelegate);
    return d->rect;
}

QRect ItemViewImportDelegate::pixmapRect() const
{
    return QRect();
}

QRect ItemViewImportDelegate::imageInformationRect() const
{
    return QRect();
}

QSize ItemViewImportDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ItemViewImportDelegate);
    return d->rect.size();
}

QSize ItemViewImportDelegate::gridSize() const
{
    Q_D(const ItemViewImportDelegate);
    return d->gridSize;
}

bool ItemViewImportDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ItemViewImportDelegate::acceptsActivation(const QPoint& , const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ItemViewImportDelegate::asDelegate()
{
    return this;
}

void ItemViewImportDelegate::overlayDestroyed(QObject* o)
{
    ImageDelegateOverlayContainer::overlayDestroyed(o);
}

void ItemViewImportDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection AbstractImportItemDelegate -> ItemViewImportDelegate -> ImageDelegateOverlayContainer
    ImageDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ItemViewImportDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewImportDelegate);
    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewImportDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewImportDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewImportDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewImportDelegate);
    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();

    if (oldGridSize != d->gridSize)
    {
        emit gridSizeChanged(d->gridSize);
        // emit sizeHintChanged(QModelIndex());
    }

    emit visualChange();
}

QRect ItemViewImportDelegate::drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background,
                                            const QPixmap& thumbnail) const
{
    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
    {
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

void ItemViewImportDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewImportDelegate);
    p->setFont(d->fontReg);
    p->drawText(nameRect, Qt::AlignCenter, name);//squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewImportDelegate::drawModificationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewImportDelegate);
    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str         = i18nc("date of last image modification", "modified: %1",str);
    p->drawText(dateRect, Qt::AlignCenter, str);//squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewImportDelegate::drawImageFormat(QPainter* p, const QRect& r, const QString& mime) const
{
    Q_D(const ItemViewImportDelegate);

    if (!mime.isEmpty() && !r.isNull())
    {
        QString type = mime.split("/").at(1);
        type = ImageScanner::formatToString(type);

        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        QRect bRect = p->boundingRect(r, Qt::AlignTop | Qt::AlignHCenter, type.toUpper());
        bRect.adjust(-1, -1, 1, 1);
        bRect.translate(0, 1);

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, Qt::AlignTop | Qt::AlignHCenter, type.toUpper());

        p->restore();
    }
}

void ItemViewImportDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewImportDelegate);

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

        p->drawText(dimsRect, Qt::AlignCenter, resolution);
    }
}

void ItemViewImportDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ItemViewImportDelegate);
    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, KIO::convertSize(bytes));//squeezedTextCached(p, r.width(), KIO::convertSize(bytes)));
}

void ItemViewImportDelegate::drawPanelSideIcon(QPainter* p, bool left, bool right) const
{
    Q_D(const ItemViewImportDelegate);
    int iconSize = KIconLoader::SizeSmall;

    if (left)
    {
        QRect r(3, d->rect.height()/2 - iconSize/2, iconSize, iconSize);
        QIcon icon = KIconLoader::global()->loadIcon("arrow-left", KIconLoader::NoGroup, iconSize);
        icon.paint(p, r);
    }

    if (right)
    {
        QRect r(d->rect.width() - 3 - iconSize, d->rect.height()/2 - iconSize/2, iconSize, iconSize);
        QIcon icon = KIconLoader::global()->loadIcon("arrow-right", KIconLoader::NoGroup, iconSize);
        icon.paint(p, r);
    }
}

void ItemViewImportDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                                          bool isSelected) const
{
    Q_D(const ItemViewImportDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? kapp->palette().color(QPalette::HighlightedText)
                                  : kapp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewImportDelegate::drawGroupIndicator(QPainter* p, const QRect& r,
                                                int numberOfGroupedImages, bool open) const
{
    if (numberOfGroupedImages)
    {
        QIcon icon;

        if (open)
        {
            icon = KIconLoader::global()->loadIcon("document-import", KIconLoader::NoGroup, r.width());
        }
        else
        {
            icon = KIconLoader::global()->loadIcon("document-multiple", KIconLoader::NoGroup, r.width());
        }

        qreal op = p->opacity();
        p->setOpacity(0.5);
        icon.paint(p, r);
        p->setOpacity(op);

        QString text = QString::number(numberOfGroupedImages);
        p->drawText(r, Qt::AlignCenter, text);
    }
}

void ItemViewImportDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewImportDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewImportDelegate::prepareFonts()
{
    Q_D(ItemViewImportDelegate);

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

void ItemViewImportDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ItemViewImportDelegate);

    QFontMetrics fm(d->fontReg);
    d->oneRowRegRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       "XXXXXXXXX");
    fm = QFontMetrics(d->fontCom);
    d->oneRowComRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       "XXXXXXXXX");
    fm = QFontMetrics(d->fontXtra);
    d->oneRowXtraRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                        Qt::AlignTop | Qt::AlignHCenter,
                                        "XXXXXXXXX");
}

void ItemViewImportDelegate::prepareBackground()
{
    Q_D(ItemViewImportDelegate);

    if (!d->rect.isValid())
    {
        d->regPixmap = QPixmap();
        d->selPixmap = QPixmap();
    }
    else
    {
        d->regPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->regPixmap.fill(kapp->palette().color(QPalette::Base));
        QPainter p1(&d->regPixmap);
        p1.setPen(kapp->palette().color(QPalette::Midlight));
        p1.drawRect(0, 0, d->rect.width()-1, d->rect.height()-1);

        d->selPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->selPixmap.fill(kapp->palette().color(QPalette::Highlight));
        QPainter p2(&d->selPixmap);
        p2.setPen(kapp->palette().color(QPalette::Midlight));
        p2.drawRect(0, 0, d->rect.width()-1, d->rect.height()-1);
    }
}

} // namespace Digikam
