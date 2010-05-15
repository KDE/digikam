/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "graphicsdimgitem.h"
#include "dimgitemspriv.h"

// Qt includes

#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QStyleOptionGraphicsItem>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimgchilditem.h"
#include "imagezoomsettings.h"


namespace Digikam
{


GraphicsDImgItem::GraphicsDImgItem(QGraphicsItem *parent)
    : QGraphicsItem(parent), d_ptr(new GraphicsDImgItemPrivate)
{
    d_ptr->init(this);
}

GraphicsDImgItem::GraphicsDImgItem(GraphicsDImgItemPrivate& dd, QGraphicsItem *parent)
    : QGraphicsItem(parent), d_ptr(&dd)
{
    d_ptr->init(this);
}

void GraphicsDImgItemPrivate::init(GraphicsDImgItem *q)
{
    // ItemCoordinateCache is very slow, DeviceCoordinateCache makes severe render artifacts
    q->setCacheMode(QGraphicsItem::NoCache);
    #if QT_VERSION >= 0x040600
    // This flag is crucial for our performance! Limits redrawing area.
    q->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
    #endif
    q->setAcceptedMouseButtons(Qt::NoButton);
}

GraphicsDImgItem::~GraphicsDImgItem()
{
    Q_D(GraphicsDImgItem);
    delete d;
}

void GraphicsDImgItem::setImage(const DImg& img)
{
    Q_D(GraphicsDImgItem);
    d->image = img;
    d->zoomSettings.setImageSize(img.size(), img.originalSize());
    sizeHasChanged();
    emit imageChanged();
}

DImg GraphicsDImgItem::image() const
{
    Q_D(const GraphicsDImgItem);
    return d->image;
}

void GraphicsDImgItem::sizeHasChanged()
{
    QGraphicsItem::prepareGeometryChange();

    foreach (QGraphicsItem *child, childItems())
    {
        DImgChildItem *item = dynamic_cast<DImgChildItem*>(child);
        if (item)
        {
            item->sizeHasChanged();
        }
    }
}

const ImageZoomSettings* GraphicsDImgItem::zoomSettings() const
{
    Q_D(const GraphicsDImgItem);
    return &d->zoomSettings;
}

ImageZoomSettings* GraphicsDImgItem::zoomSettings()
{
    Q_D(GraphicsDImgItem);
    return &d->zoomSettings;
}

QRectF GraphicsDImgItem::boundingRect() const
{
    Q_D(const GraphicsDImgItem);
    return QRectF(QPointF(0,0), d->zoomSettings.zoomedSize());
}

void GraphicsDImgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    Q_D(GraphicsDImgItem);
    QRectF drawRect = option->exposedRect.intersected(boundingRect());
    QRect sourceRect = d->zoomSettings.sourceRect(drawRect).toRect();
    QSize destSize = drawRect.size().toSize();
    kDebug() << sourceRect << drawRect << drawRect.topLeft().toPoint() << destSize;
    DImg scaledImage = d->image.smoothScaleSection(sourceRect.x(), sourceRect.y(),
                                                   sourceRect.width(), sourceRect.height(),
                                                   destSize.width(), destSize.height());
    painter->drawPixmap(drawRect.topLeft().toPoint(), scaledImage.convertToPixmap());
    /*
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Cannot display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        // three copies - but the image is small
        setImage(DImg(pix.toImage()));
    */
}


/*
void ImagePreviewViewV2::viewportPaintExtraData()
{
    if (!m_movingInProgress && d->isLoaded)
    {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBackgroundMode(Qt::TransparentMode);
        QFontMetrics fontMt = p.fontMetrics();

        QString text;
        QRect textRect, fontRect;
        QRect region = contentsRect();
        p.translate(region.topLeft());

        if (!d->loadFullImageSize)
        {
            if (d->imageInfo.format().startsWith(QLatin1String("RAW")))
                text = i18n("Embedded JPEG Preview");
            else
                text = i18n("Reduced Size Preview");
        }
        else
        {
            if (d->imageInfo.format().startsWith(QLatin1String("RAW")))
                text = i18n("Half Size Raw Preview");
            else
                text = i18n("Full Size Preview");
        }

        fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
        drawText(&p, QPoint(region.topRight().x()-fontRect.width()-10, region.topRight().y()+5), text);
        p.end();
    }
}
*/

}

