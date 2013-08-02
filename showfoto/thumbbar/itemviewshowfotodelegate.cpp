/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

    makeStarPolygon();

    ratingPixmaps = QVector<QPixmap>(10);
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
    ratingRect = QRect(0, 0, 0, 0);
}

void ItemViewShowfotoDelegatePrivate::makeStarPolygon()
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

// ---- ItemViewShowfotoDelegate -----------------------------------------------

ItemViewShowfotoDelegate::ItemViewShowfotoDelegate(QObject* const parent)
    : DItemDelegate(parent), d_ptr(new ItemViewShowfotoDelegatePrivate)
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

QRect ItemViewShowfotoDelegate::ratingRect() const
{
    Q_D(const ItemViewShowfotoDelegate);
    return d->ratingRect;
}

void ItemViewShowfotoDelegate::setRatingEdited(const QModelIndex& index)
{
    Q_D(ItemViewShowfotoDelegate);
    d->editingRating = index;
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

void ItemViewShowfotoDelegate::drawRating(QPainter* p, const QModelIndex& index, const QRect& ratingRect,
                                       int rating, bool isSelected) const
{
    Q_D(const ItemViewShowfotoDelegate);

    if (d->editingRating != index)
    {
        p->drawPixmap(ratingRect, ratingPixmap(rating, isSelected));
    }
/*
    else
        p->drawPixmap(r, ratingPixmap(-1, isSelected));
*/
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
        QString type = mime.split('/').at(1);
        //TODO
        //type         = ImageScanner::formatToString(type);

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

void ItemViewShowfotoDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewShowfotoDelegate);

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

void ItemViewShowfotoDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ItemViewShowfotoDelegate);
    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, KIO::convertSize(bytes));//squeezedTextCached(p, r.width(), KIO::convertSize(bytes)));
}

void ItemViewShowfotoDelegate::drawTags(QPainter* p, const QRect& r, const QString& tagsString,
                                     bool isSelected) const
{
    Q_D(const ItemViewShowfotoDelegate);
    p->setFont(d->fontCom);
    p->setPen(isSelected ? kapp->palette().color(QPalette::HighlightedText)
                         : kapp->palette().color(QPalette::Link));

    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tagsString));
}

void ItemViewShowfotoDelegate::drawPickLabelIcon(QPainter* p, const QRect& r, int pickId) const
{
    // Draw Pick Label icon
    if (pickId != NoPickLabel)
    {
        QIcon icon;

        if (pickId == RejectedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-red", KIconLoader::NoGroup, r.width());
        }
        else if (pickId == PendingLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-yellow", KIconLoader::NoGroup, r.width());
        }
        else if (pickId == AcceptedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-green", KIconLoader::NoGroup, r.width());
        }

        icon.paint(p, r);
    }
}

void ItemViewShowfotoDelegate::drawColorLabelRect(QPainter* p, const QStyleOptionViewItem& option,
                                               bool isSelected, int colorId) const
{
    Q_D(const ItemViewShowfotoDelegate);
    Q_UNUSED(option);
    Q_UNUSED(isSelected);

    if (colorId > NoColorLabel)
    {
        // This draw a simple rectangle around item.
        p->setPen(QPen(ColorLabelWidget::labelColor((ColorLabel)colorId), 5, Qt::SolidLine));
        p->drawRect(3, 3, d->rect.width()-7, d->rect.height()-7);
    }
}

void ItemViewShowfotoDelegate::drawPanelSideIcon(QPainter* p, bool left, bool right) const
{
    Q_D(const ItemViewShowfotoDelegate);
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


//void ItemViewShowfotoDelegate::drawDownloadIndicator(QPainter* p, const QRect& r, int itemType) const
//{
//    QIcon icon;

//    if (itemType == ShowfotoItemInfo::DownloadUnknown)
//    {
//        icon = KIconLoader::global()->loadIcon("dialog-information", KIconLoader::NoGroup, KIconLoader::SizeSmall);
//    }

//    if (itemType == ShowfotoItemInfo::NewPicture)
//    {
//        icon = KIconLoader::global()->loadIcon("get-hot-new-stuff", KIconLoader::NoGroup, KIconLoader::SizeSmall);
//    }

//    if (itemType == ShowfotoItemInfo::DownloadedYes)
//    {
//        icon = KIconLoader::global()->loadIcon("dialog-ok", KIconLoader::NoGroup, KIconLoader::SizeSmall);
//    }

//    qreal op = p->opacity();
//    p->setOpacity(0.5);
//    icon.paint(p, r);
//    p->setOpacity(op);
//}

void ItemViewShowfotoDelegate::drawLockIndicator(QPainter* p, const QRect& r, int lockStatus) const
{
    QIcon icon;

    if (lockStatus == 0)
    {
        icon = KIconLoader::global()->loadIcon("object-locked", KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }

    if (lockStatus == 1)
    {
        icon = KIconLoader::global()->loadIcon("object-unlocked", KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }

    qreal op = p->opacity();
    p->setOpacity(0.5);
    icon.paint(p, r);
    p->setOpacity(op);
}

void ItemViewShowfotoDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                                          bool isSelected) const
{
    Q_D(const ItemViewShowfotoDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? kapp->palette().color(QPalette::HighlightedText)
                                  : kapp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewShowfotoDelegate::drawGroupIndicator(QPainter* p, const QRect& r,
                                                int numberOfGroupedImages, bool open) const
{
    if (numberOfGroupedImages)
    {
        QIcon icon;

        if (open)
        {
            icon = KIconLoader::global()->loadIcon("document-Showfoto", KIconLoader::NoGroup, r.width());
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

void ItemViewShowfotoDelegate::prepareRatingPixmaps(bool composeOverBackground)
{
    /// Please call this method after prepareBackground() and when d->ratingPixmap is set

    Q_D(ItemViewShowfotoDelegate);

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
            painter.setBrush(kapp->palette().color(QPalette::Link));
            QPen pen(kapp->palette().color(QPalette::Text));
            // set a pen which joins the lines at a filled angle
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);

            // move painter while drawing polygons
            painter.translate( lround((d->ratingRect.width() - d->margin - rating*(d->starPolygonSize.width()+1))/2.0) + 2, 1 );

            for (int s=0; s<rating; ++s)
            {
                painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                painter.translate(d->starPolygonSize.width() + 1, 0);
            }
        }
    }
}

//TODO: make sure that u won't use this
QPixmap ItemViewShowfotoDelegate::ratingPixmap(int rating, bool selected) const
{
    Q_D(const ItemViewShowfotoDelegate);

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
    {
        return d->ratingPixmaps.at(5 + rating);
    }
    else
    {
        return d->ratingPixmaps.at(rating);
    }
}

} // namespace ShowFoto
