/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-30-09
 * Description : a tool to print images
 *
 * Copyright (C) 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "advprintcropframe.h"

// C++ includes

#include <cmath>
#include <cstdio>

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QtGlobal>

// Local includes

#include "advprintphoto.h"
#include "advprintwizard.h"

namespace Digikam
{

class AdvPrintCropFrame::Private
{
public:

    Private()
      : photo(0),
        mouseDown(false),
        pixmap(0),
        pixmapX(0),
        pixmapY(0),
        drawRec(true)
    {
    }

    AdvPrintPhoto*  photo;
    bool            mouseDown;
    QPixmap*        pixmap;
    int             pixmapX;
    int             pixmapY;

    QColor          color;

    QRect           cropRegion;
    bool            drawRec;
};

AdvPrintCropFrame::AdvPrintCropFrame(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
}

AdvPrintCropFrame::~AdvPrintCropFrame()
{
    delete d->pixmap;
    delete d;
}

// FIXME:  This method is doing way too much. The cropFrame initialization
// should be a AdvPrintPhoto method, and should not require the scaling of
// pixmaps to get the desired effect, which are too slow.

void AdvPrintCropFrame::init(AdvPrintPhoto* const photo,
                             int  wphoto,
                             int  hphoto,
                             bool autoRotate,
                             bool paint)
{
    d->photo             = photo;
    QImage scaledImg     = d->photo->loadPhoto();

    // has the cropRegion been set yet?
    bool resetCropRegion = (d->photo->m_cropRegion == QRect(-1, -1, -1, -1));

    if (resetCropRegion)
    {
        // first, let's see if we should rotate
        if (autoRotate)
        {
            if (d->photo->m_rotation == 0 &&
                ((wphoto > hphoto && d->photo->thumbnail().height() > d->photo->thumbnail().width()) ||
                 (hphoto > wphoto && d->photo->thumbnail().width()  > d->photo->thumbnail().height())))
            {
                // rotate
                d->photo->m_rotation = 90;
            }
        }
    }
    else
    {
        // does the crop region need updating (but the image shouldn't be rotated)?
        resetCropRegion = (d->photo->m_cropRegion == QRect(-2, -2, -2, -2));
    }

    // rotate
    QMatrix matrix;
    matrix.rotate(d->photo->m_rotation);
    scaledImg = scaledImg.transformed(matrix);

    scaledImg = scaledImg.scaled(width(), height(), Qt::KeepAspectRatio);

    //TODO check for cropping Qt::KeepAspectRatioByExpanding);
    //d->pixmap = new QPixmap();
    //QPixmap pix(scaledImg.width(), scaledImg.height());

    QPixmap pix(width(), height());
    d->pixmap = new QPixmap(pix.fromImage(scaledImg));

    //   d->pixmap = new QPixmap(scaledImg.width(), scaledImg.height());
    //   d->pixmap->fromImage(scaledImg);

    d->pixmapX = (width()  / 2) - (d->pixmap->width()  / 2);
    d->pixmapY = (height() / 2) - (d->pixmap->height() / 2);

    d->color = Qt::red;

    // size the rectangle based on the minimum image dimension
    int w = d->pixmap->width();
    int h = d->pixmap->height();

    if (w < h)
    {
        h = AdvPrintWizard::normalizedInt((double)w * ((double)hphoto / (double)wphoto));

        if (h > d->pixmap->height())
        {
            h = d->pixmap->height();
            w = AdvPrintWizard::normalizedInt((double)h * ((double)wphoto / (double)hphoto));
        }
    }
    else
    {
        w = AdvPrintWizard::normalizedInt((double)h * ((double)wphoto / (double)hphoto));

        if (w > d->pixmap->width())
        {
            w = d->pixmap->width();
            h = AdvPrintWizard::normalizedInt((double)w * ((double)hphoto / (double)wphoto));
        }
    }

    if (resetCropRegion)
    {
        d->cropRegion.setRect((width() / 2) - (w / 2), (height() / 2) - (h / 2), w, h);
        d->photo->m_cropRegion = screenToPhotoRect(d->cropRegion);
    }
    else
    {
        d->cropRegion = photoToScreenRect(d->photo->m_cropRegion);
    }

    if (paint)
    {
        update();
    }
}

QRect AdvPrintCropFrame::screenToPhotoRect(const QRect& r) const
{
    // r is given in screen coordinates, and we want to convert that
    // to photo coordinates
    double xRatio = 0.0;
    double yRatio = 0.0;

    // flip the photo dimensions if rotated
    int photoW;
    int photoH;

    if (d->photo->m_rotation == 0 || d->photo->m_rotation == 180)
    {
        photoW = d->photo->width();
        photoH = d->photo->height();
    }
    else
    {
        photoW = d->photo->height();
        photoH = d->photo->width();
    }

    if (d->pixmap->width() > 0)
        xRatio = (double) photoW / (double) d->pixmap->width();

    if (d->pixmap->height() > 0)
        yRatio = (double) photoH / (double) d->pixmap->height();


    int x1 = AdvPrintWizard::normalizedInt((r.left() - d->pixmapX) * xRatio);
    int y1 = AdvPrintWizard::normalizedInt((r.top()  - d->pixmapY) * yRatio);

    int w  = AdvPrintWizard::normalizedInt(r.width()  * xRatio);
    int h  = AdvPrintWizard::normalizedInt(r.height() * yRatio);

    QRect result;
    result.setRect(x1, y1, w, h);

    return result;
}

QRect AdvPrintCropFrame::photoToScreenRect(const QRect& r) const
{
    // r is given in photo coordinates, and we want to convert that
    // to screen coordinates
    double xRatio = 0.0;
    double yRatio = 0.0;

    // flip the photo dimensions if rotated
    int photoW;
    int photoH;

    if (d->photo->m_rotation == 0 || d->photo->m_rotation == 180)
    {
        photoW = d->photo->width();
        photoH = d->photo->height();
    }
    else
    {
        photoW = d->photo->height();
        photoH = d->photo->width();
    }

    if (d->photo->width() > 0)
        xRatio = (double) d->pixmap->width() / (double) photoW;

    if (d->photo->height() > 0)
        yRatio = (double)d->pixmap->height() / (double)photoH;


    int x1 = AdvPrintWizard::normalizedInt(r.left() * xRatio + d->pixmapX);
    int y1 = AdvPrintWizard::normalizedInt(r.top()  * yRatio + d->pixmapY);

    int w  = AdvPrintWizard::normalizedInt(r.width()  * xRatio);
    int h  = AdvPrintWizard::normalizedInt(r.height() * yRatio);

    QRect result;
    result.setRect(x1, y1, w, h);
    return result;
}

void AdvPrintCropFrame::paintEvent(QPaintEvent*)
{
    QPixmap bmp(this->width(), this->height());
    QPainter p;
    p.begin(&bmp);

    p.eraseRect(0, 0, this->width(), this->height());

    // draw the background pixmap
    p.drawPixmap(d->pixmapX, d->pixmapY, *d->pixmap);

    if (d->drawRec)
    {
        // draw the rectangle
        p.setPen(QPen(d->color, 2));
        p.drawRect(d->cropRegion);

        // draw the crosshairs
        int midX = d->cropRegion.left() + d->cropRegion.width()  / 2;
        int midY = d->cropRegion.top()  + d->cropRegion.height() / 2;
        p.drawLine(midX - 10, midY,      midX + 10, midY);
        p.drawLine(midX,      midY - 10, midX,      midY + 10);
    }
    p.end();

    QPainter newp(this);
    newp.drawPixmap(0, 0, bmp);
}

void AdvPrintCropFrame::mousePressEvent(QMouseEvent* e)
{
    if (e && e->button() == Qt::LeftButton)
    {
        d->mouseDown = true;
        QWidget::mouseMoveEvent(e);
    }
}

void AdvPrintCropFrame::mouseReleaseEvent(QMouseEvent* e)
{
    if (e && e->button() == Qt::LeftButton)
        d->mouseDown = false;
}

void AdvPrintCropFrame::mouseMoveEvent(QMouseEvent* e)
{
    if (d->mouseDown)
    {
        // don't let the rectangle float off the image.
        int newW = d->cropRegion.width();
        int newH = d->cropRegion.height();

        int newX = e->x() - (newW / 2);
        newX     = qMax(d->pixmapX, newX);
        newX     = qMin(d->pixmapX + d->pixmap->width() - newW, newX);

        int newY = e->y() - (newH / 2);
        newY     = qMax(d->pixmapY, newY);
        newY     = qMin(d->pixmapY + d->pixmap->height() - newH, newY);

        d->cropRegion.setRect(newX, newY, newW, newH);
        d->photo->m_cropRegion = screenToPhotoRect(d->cropRegion);
        update();
    }
}

void AdvPrintCropFrame::keyPressEvent(QKeyEvent* e)
{
    int newX = d->cropRegion.x();
    int newY = d->cropRegion.y();

    switch (e->key())
    {
        case Qt::Key_Up    : newY--;
            break;
        case Qt::Key_Down  : newY++;
            break;
        case Qt::Key_Left  : newX--;
            break;
        case Qt::Key_Right : newX++;
            break;
    }

    // keep inside the pixmap

    int w = d->cropRegion.width();
    int h = d->cropRegion.height();

    newX  = qMax(d->pixmapX, newX);
    newX  = qMin(d->pixmapX + d->pixmap->width() - w,  newX);

    newY  = qMax(d->pixmapY, newY);
    newY  = qMin(d->pixmapY + d->pixmap->height() - h, newY);

    d->cropRegion.setRect(newX, newY, w, h);
    d->photo->m_cropRegion = screenToPhotoRect(d->cropRegion);
    update();
}

void AdvPrintCropFrame::setColor(const QColor& c)
{
    d->color = c;
    update();
}

QColor AdvPrintCropFrame::color() const
{
    return d->color;
}

void AdvPrintCropFrame::drawCropRectangle(bool draw)
{
    d->drawRec = draw;
}

} // Namespace Digikam
