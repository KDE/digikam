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
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QtGlobal>

// Local includes

#include "digikam_debug.h"
#include "advprintphoto.h"
#include "advprintwizard.h"

namespace Digikam
{

class AdvPrintCropFrame::Private
{
public:

    explicit Private()
      : photo(0),
        mouseDown(false),
        image(0),
        imageX(0),
        imageY(0),
        color(Qt::red),
        drawRec(true)
    {
    }

    AdvPrintPhoto* photo;
    bool           mouseDown;
    QImage         image;
    int            imageX;
    int            imageY;

    QColor         color;

    QRect          cropRegion;
    bool           drawRec;

    QMatrix        matrix;
};

AdvPrintCropFrame::AdvPrintCropFrame(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
}

AdvPrintCropFrame::~AdvPrintCropFrame()
{
    delete d;
}

void AdvPrintCropFrame::init(AdvPrintPhoto* const photo,
                             int  woutlay,
                             int  houtlay,
                             bool autoRotate,
                             bool paint)
{
    d->photo  = photo;
    d->matrix = d->photo->updateCropRegion(woutlay, houtlay, autoRotate);

    if (paint)
    {
        updateImage();
        update();
    }
}

QRect AdvPrintCropFrame::screenToPhotoRect(const QRect& r) const
{
    // 'r' is given in screen coordinates, and we want to convert that to photo coordinates.
    double xRatio = 0.0;
    double yRatio = 0.0;

    // Flip the photo dimensions if rotated
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

    if (d->image.width() > 0)
    {
        xRatio = (double) photoW / (double) d->image.width();
    }

    if (d->image.height() > 0)
    {
        yRatio = (double) photoH / (double) d->image.height();
    }

    int x1 = AdvPrintWizard::normalizedInt((r.left() - d->imageX) * xRatio);
    int y1 = AdvPrintWizard::normalizedInt((r.top()  - d->imageY) * yRatio);

    int w  = AdvPrintWizard::normalizedInt(r.width()  * xRatio);
    int h  = AdvPrintWizard::normalizedInt(r.height() * yRatio);

    QRect result;
    result.setRect(x1, y1, w, h);

    return result;
}

QRect AdvPrintCropFrame::photoToScreenRect(const QRect& r) const
{
    // 'r' is given in photo coordinates, and we want to convert that to screen coordinates
    double xRatio = 0.0;
    double yRatio = 0.0;

    // Flip the photo dimensions if rotated
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
    {
        xRatio = (double) d->image.width() / (double) photoW;
    }

    if (d->photo->height() > 0)
    {
        yRatio = (double)d->image.height() / (double)photoH;
    }

    int x1 = AdvPrintWizard::normalizedInt(r.left() * xRatio + d->imageX);
    int y1 = AdvPrintWizard::normalizedInt(r.top()  * yRatio + d->imageY);

    int w  = AdvPrintWizard::normalizedInt(r.width()  * xRatio);
    int h  = AdvPrintWizard::normalizedInt(r.height() * yRatio);

    QRect result;
    result.setRect(x1, y1, w, h);
    return result;
}

void AdvPrintCropFrame::updateImage()
{
    if (d->photo)
    {
        d->image      = d->photo->loadPhoto().copyQImage();
        d->image      = d->image.transformed(d->matrix);
        d->image      = d->image.scaled(width(), height(), Qt::KeepAspectRatio);
        d->imageX     = (width()  / 2) - (d->image.width()  / 2);
        d->imageY     = (height() / 2) - (d->image.height() / 2);
        d->cropRegion = photoToScreenRect(d->photo->m_cropRegion);
    }
}

void AdvPrintCropFrame::resizeEvent(QResizeEvent*)
{
    updateImage();
    update();
}

void AdvPrintCropFrame::paintEvent(QPaintEvent*)
{
    updateImage();

    QPixmap bmp(this->width(), this->height());
    QPainter p;
    p.begin(&bmp);

    p.eraseRect(0, 0, this->width(), this->height());

    // Draw the background image
    p.drawImage(d->imageX, d->imageY, d->image);

    if (d->drawRec)
    {
        // Draw the rectangle
        p.setPen(QPen(d->color, 2));
        p.drawRect(d->cropRegion);

        // Draw the crosshairs
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
        // Don't let the rectangle float off the image.
        int newW = d->cropRegion.width();
        int newH = d->cropRegion.height();

        int newX = e->x() - (newW / 2);
        newX     = qMax(d->imageX, newX);
        newX     = qMin(d->imageX + d->image.width() - newW, newX);

        int newY = e->y() - (newH / 2);
        newY     = qMax(d->imageY, newY);
        newY     = qMin(d->imageY + d->image.height() - newH, newY);

        d->cropRegion.setRect(newX, newY, newW, newH);
        d->photo->m_cropRegion = screenToPhotoRect(d->cropRegion);
        update();
    }
}

void AdvPrintCropFrame::keyReleaseEvent(QKeyEvent* e)
{
    int newX = d->cropRegion.x();
    int newY = d->cropRegion.y();

    switch (e->key())
    {
        case Qt::Key_Up:
            newY--;
            break;
        case Qt::Key_Down:
            newY++;
            break;
        case Qt::Key_Left:
            newX--;
            break;
        case Qt::Key_Right:
            newX++;
            break;
    }

    // Keep inside the image

    int w = d->cropRegion.width();
    int h = d->cropRegion.height();

    newX  = qMax(d->imageX, newX);
    newX  = qMin(d->imageX + d->image.width() - w,  newX);

    newY  = qMax(d->imageY, newY);
    newY  = qMin(d->imageY + d->image.height() - h, newY);

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
