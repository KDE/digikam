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

#include <QPainter>
#include <QMouseEvent>
#include <QtGlobal>

// Local includes

#include "advprintutils.h"

namespace Digikam
{

AdvPrintCropFrame::AdvPrintCropFrame(QWidget* const parent)
    : QWidget(parent),
      m_photo(0),
      m_mouseDown(false),
      m_pixmap(0),
      m_pixmapX(0),
      m_pixmapY(0),
      m_color(),
      m_cropRegion(),
      m_drawRec(true)
{
}

AdvPrintCropFrame::~AdvPrintCropFrame()
{
    delete m_pixmap;
}

// FIXME:  This method is doing way too much.  The cropFrame initialization
// should be a AdvPrintPhoto method, and should not require the scaling of
// pixmaps to get the desired effect, which are too slow.

void AdvPrintCropFrame::init(AdvPrintPhoto* const photo,
                             int width,
                             int height,
                             bool autoRotate,
                             bool paint)
{
    m_photo              = photo;
    QImage scaledImg     = m_photo->loadPhoto();//thumbnail().toImage();

    // has the cropRegion been set yet?
    bool resetCropRegion = (m_photo->m_cropRegion == QRect(-1, -1, -1, -1));

    if (resetCropRegion)
    {
        // first, let's see if we should rotate
        if (autoRotate)
        {
            if (m_photo->m_rotation == 0 &&
                ((width > height && m_photo->thumbnail().height() > m_photo->thumbnail().width()) ||
                (height > width  && m_photo->thumbnail().width()  > m_photo->thumbnail().height())))
            {
                // rotate
                m_photo->m_rotation = 90;
            }
        }
    }
    else
    {
        // does the crop region need updating (but the image shouldn't be rotated)?
        resetCropRegion = (m_photo->m_cropRegion == QRect(-2, -2, -2, -2));
    }

    // rotate
    QMatrix matrix;
    matrix.rotate(m_photo->m_rotation);
    scaledImg = scaledImg.transformed(matrix);

    scaledImg = scaledImg.scaled(this->width(), this->height(), Qt::KeepAspectRatio);

    //TODO check for cropping Qt::KeepAspectRatioByExpanding);
    //m_pixmap = new QPixmap();
    //   QPixmap pix(scaledImg.width(), scaledImg.height());
    QPixmap pix(this->width(), this->height());
    m_pixmap  = new QPixmap(pix.fromImage(scaledImg));

    //   m_pixmap = new QPixmap(scaledImg.width(), scaledImg.height());
    //   m_pixmap->fromImage(scaledImg);

    m_pixmapX = (this->width() / 2) - (m_pixmap->width() / 2);
    m_pixmapY = (this->height() / 2) - (m_pixmap->height() / 2);

    m_color = Qt::red;
    // size the rectangle based on the minimum image dimension
    int w = m_pixmap->width();
    int h = m_pixmap->height();

    if (w < h)
    {
        h = AdvPrintNint((double)w * ((double)height / (double)width));

        if (h > m_pixmap->height())
        {
            h = m_pixmap->height();
            w = AdvPrintNint((double)h * ((double)width / (double)height));
        }
    }
    else
    {
        w = AdvPrintNint((double)h * ((double)width / (double)height));

        if (w > m_pixmap->width())
        {
            w = m_pixmap->width();
            h = AdvPrintNint((double)w * ((double)height / (double)width));
        }
    }

    if (resetCropRegion)
    {
        m_cropRegion.setRect((this->width() / 2) - (w / 2), (this->height() / 2) - (h / 2), w, h);
        m_photo->m_cropRegion = _screenToPhotoRect(m_cropRegion);
    }
    else
    {
        m_cropRegion = _photoToScreenRect(m_photo->m_cropRegion);
    }

    if (paint)
      update();

    //repaint(m_cropRegion);
 }

QRect AdvPrintCropFrame::_screenToPhotoRect(const QRect& r) const
{
    // r is given in screen coordinates, and we want to convert that
    // to photo coordinates
    double xRatio = 0.0;
    double yRatio = 0.0;

    // flip the photo dimensions if rotated
    int photoW;
    int photoH;

    if (m_photo->m_rotation == 0 || m_photo->m_rotation == 180)
    {
        photoW = m_photo->width();
        photoH = m_photo->height();
    }
    else
    {
        photoW = m_photo->height();
        photoH = m_photo->width();
    }

    if (m_pixmap->width() > 0)
        xRatio = (double) photoW / (double) m_pixmap->width();

    if (m_pixmap->height() > 0)
        yRatio = (double) photoH / (double) m_pixmap->height();


    int x1 = AdvPrintNint((r.left() - m_pixmapX) * xRatio);
    int y1 = AdvPrintNint((r.top()  - m_pixmapY) * yRatio);

    int w  = AdvPrintNint(r.width() * xRatio);
    int h  = AdvPrintNint(r.height() * yRatio);

    QRect result;
    result.setRect(x1, y1, w, h);
    return result;
}

QRect AdvPrintCropFrame::_photoToScreenRect(const QRect& r) const
{
    // r is given in photo coordinates, and we want to convert that
    // to screen coordinates
    double xRatio = 0.0;
    double yRatio = 0.0;

    // flip the photo dimensions if rotated
    int photoW;
    int photoH;

    if (m_photo->m_rotation == 0 || m_photo->m_rotation == 180)
    {
        photoW = m_photo->width();
        photoH = m_photo->height();
    }
    else
    {
        photoW = m_photo->height();
        photoH = m_photo->width();
    }

    if (m_photo->width() > 0)
        xRatio = (double) m_pixmap->width() / (double) photoW;

    if (m_photo->height() > 0)
        yRatio = (double)m_pixmap->height() / (double)photoH;


    int x1 = AdvPrintNint(r.left() * xRatio + m_pixmapX);
    int y1 = AdvPrintNint(r.top()  * yRatio + m_pixmapY);

    int w  = AdvPrintNint(r.width() * xRatio);
    int h  = AdvPrintNint(r.height() * yRatio);

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
    p.drawPixmap(m_pixmapX, m_pixmapY, *m_pixmap);

    if (m_drawRec)
    {
        // draw the rectangle
        p.setPen(QPen(m_color, 2));
        p.drawRect(m_cropRegion);
        // draw the crosshairs
        int midX = m_cropRegion.left() + m_cropRegion.width() / 2;
        int midY = m_cropRegion.top()  + m_cropRegion.height() / 2;
        p.drawLine(midX - 10, midY, midX + 10, midY);
        p.drawLine(midX, midY - 10, midX, midY + 10);
    }
    p.end();

    QPainter newp(this);
    newp.drawPixmap(0, 0, bmp);
}

void AdvPrintCropFrame::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        m_mouseDown = true;
        this->mouseMoveEvent(e);
    }
}

void AdvPrintCropFrame::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
        m_mouseDown = false;
}

void AdvPrintCropFrame::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mouseDown)
    {
        // don't let the rectangle float off the image.
        int newW = m_cropRegion.width();
        int newH = m_cropRegion.height();

        int newX = e->x() - (newW / 2);
        newX     = qMax(m_pixmapX, newX);
        newX     = qMin(m_pixmapX + m_pixmap->width() - newW, newX);

        int newY = e->y() - (newH / 2);
        newY     = qMax(m_pixmapY, newY);
        newY     = qMin(m_pixmapY + m_pixmap->height() - newH, newY);

        m_cropRegion.setRect(newX, newY, newW, newH);
        m_photo->m_cropRegion = _screenToPhotoRect(m_cropRegion);
        //repaint(m_cropRegion);
        update();
    }
}

void AdvPrintCropFrame::keyPressEvent(QKeyEvent* e)
{
    int newX = m_cropRegion.x();
    int newY = m_cropRegion.y();

    switch (e->key())
    {
        case Qt::Key_Up : newY--;
            break;
        case Qt::Key_Down : newY++;
            break;
        case Qt::Key_Left : newX--;
            break;
        case Qt::Key_Right : newX++;
            break;
    }

    // keep inside the pixmap
    int w = m_cropRegion.width();
    int h = m_cropRegion.height();

    newX  = qMax(m_pixmapX, newX);
    newX  = qMin(m_pixmapX + m_pixmap->width() - w, newX);

    newY  = qMax(m_pixmapY, newY);
    newY  = qMin(m_pixmapY + m_pixmap->height() - h, newY);

    m_cropRegion.setRect(newX, newY, w, h);
    m_photo->m_cropRegion = _screenToPhotoRect(m_cropRegion);
    update();
    //repaint(m_cropRegion);
}

void AdvPrintCropFrame::setColor(const QColor& c)
{
    m_color = c;
    update();
    //repaint();
}

QColor AdvPrintCropFrame::color() const
{
    return m_color;
}

} // Namespace Digikam
