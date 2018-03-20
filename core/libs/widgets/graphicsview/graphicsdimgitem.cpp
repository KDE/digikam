/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "digikam_config.h"
#include "dimg.h"
#include "imagezoomsettings.h"

namespace Digikam
{

CachedPixmaps::CachedPixmaps(int maxCount)
    : maxCount(maxCount)
{
}

CachedPixmaps::~CachedPixmaps()
{
    clear();
}

void CachedPixmaps::setMaxCount(int count)
{
    maxCount = count;
}

void CachedPixmaps::clear()
{
    foreach(const CachedPixmapKey& key, keys)
    {
        QPixmapCache::remove(key.key);
    }

    keys.clear();
}

bool CachedPixmaps::find(const QRect& region, QPixmap* const pix, QRect* const source)
{
    QQueue<CachedPixmapKey>::iterator key;

    for (key = keys.begin(); key != keys.end(); )
    {
        if (!key->region.contains(region))
        {
            ++key;
            continue;
        }

        if (!QPixmapCache::find(key->key, pix))
        {
            key = keys.erase(key);
            continue;
        }

        if (key->region == region)
        {
            *source = QRect();
        }
        else
        {
            QPoint startPoint = region.topLeft() - key->region.topLeft();
            *source           = QRect(startPoint, region.size());
        }

        return true;
    }

    return false;
}

void CachedPixmaps::insert(const QRect& region, const QPixmap& pixmap)
{
    if (keys.size() >= maxCount)
    {
        CachedPixmapKey key = keys.dequeue();
        QPixmapCache::remove(key.key);
    }

    CachedPixmapKey key;
    key.region = region;
    key.key    = QPixmapCache::insert(pixmap);
    keys.enqueue(key);
}

// ---------------------------------------------------------------------------------------

GraphicsDImgItem::GraphicsDImgItem(QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d_ptr(new GraphicsDImgItemPrivate)
{
    d_ptr->init(this);
}

GraphicsDImgItem::GraphicsDImgItem(GraphicsDImgItemPrivate& dd, QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

void GraphicsDImgItem::GraphicsDImgItemPrivate::init(GraphicsDImgItem* const q)
{
    // ItemCoordinateCache is very slow, DeviceCoordinateCache makes severe render artifacts
    q->setCacheMode(QGraphicsItem::NoCache);
    // This flag is crucial for our performance! Limits redrawing area.
    q->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
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
    d->cachedPixmaps.clear();
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
    Q_D(GraphicsDImgItem);
    QGraphicsItem::prepareGeometryChange();
    d->cachedPixmaps.clear();
    emit imageSizeChanged(d->zoomSettings.zoomedSize());
}

void GraphicsDImgItem::clearCache()
{
    Q_D(GraphicsDImgItem);
    d->cachedPixmaps.clear();
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
    // always return full integer sizes, we can only scale to integer
    return QRectF(QPointF(0,0), d->zoomSettings.zoomedSize()).toAlignedRect();
}

void GraphicsDImgItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    Q_D(GraphicsDImgItem);

    QRect   drawRect = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;

    QSize completeSize = boundingRect().size().toSize();


    /* For high resolution ("retina") displays, Mac OS X / Qt
       report only half of the physical resolution in terms of
       pixels, i.e. every logical pixels corresponds to 2x2
       physical pixels. However, UI elements and fonts are
       nevertheless rendered at full resolution, and pixmaps
       as well, provided their resolution is high enough (that
       is, higher than the reported, logical resolution).

       To work around this, we render the photos not a logical
       resolution, but with the photo's full resolution, but
       at the screen's aspect ratio. When we later draw this
       high resolution bitmap, it is up to Qt to scale the
       photo to the true physical resolution.  The ratio
       computed below is the ratio between the photo and
       screen resolutions, or equivalently the factor by which
       we need to increase the pixel size of the rendered
       pixmap.
    */
#ifdef USE_QT_SCALING
    double xratio = double(d->image.width()) / completeSize.width();
    double yratio = double(d->image.height()) / completeSize.height();
    double ratio = qMax(qMin(xratio, yratio), 1.0);
#else
    double ratio = 1.0;
#endif

    QRect  scaledDrawRect = QRectF(ratio*drawRect.x(), ratio*drawRect.y(),
                                   ratio*drawRect.width(), ratio*drawRect.height()).toRect();

    if (d->cachedPixmaps.find(scaledDrawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(drawRect, pix);
        }
        else
        {
            painter->drawPixmap(drawRect, pix, pixSourceRect);
        }
    }
    else
    {
        // scale "as if" scaling to whole image, but clip output to our exposed region
        QSize scaledCompleteSize = QSizeF(ratio*completeSize.width(), ratio*completeSize.height()).toSize();
        DImg scaledImage   = d->image.smoothScaleClipped(scaledCompleteSize.width(), scaledCompleteSize.height(),
                                                         scaledDrawRect.x(), scaledDrawRect.y(),
                                                         scaledDrawRect.width(), scaledDrawRect.height());
        pix                = scaledImage.convertToPixmap();
        d->cachedPixmaps.insert(scaledDrawRect, pix);
        painter->drawPixmap(drawRect, pix);
    }
}

void GraphicsDImgItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    emit showContextMenu(e);
}

} // namespace Digikam
