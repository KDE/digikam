/* ============================================================
 * File  : perspectivewidget.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-01-18
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
 *
 * Matrix3 implementation inspired from gimp 2.0
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <cmath>

// Qt includes.

#include <qregion.h>
#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpointarray.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h> 
#include <kapplication.h>

// Local includes.

#include "triangle.h"
#include "perspectivewidget.h"

namespace DigikamPerspectiveImagesPlugin
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Widget class to edit perspective.

PerspectiveWidget::PerspectiveWidget(int w, int h, QWidget *parent)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    m_drawWhileMoving = true;
    m_currentResizing = ResizingNone;

    m_iface        = new Digikam::ImageIface(w, h);
    uchar *data    = m_iface->setPreviewImageSize(w, h);
    m_w            = m_iface->previewWidth();
    m_h            = m_iface->previewHeight();
    m_origW        = m_iface->originalWidth();
    m_origH        = m_iface->originalHeight();
    m_previewImage = Digikam::DImg(m_w, m_h, m_iface->previewSixteenBit(), m_iface->previewHasAlpha(), data, false);

    m_pixmap = new QPixmap(w, h);

    m_rect = QRect(w/2-m_w/2, h/2-m_h/2, m_w, m_h);

    reset();
}

PerspectiveWidget::~PerspectiveWidget()
{
    delete m_iface;
    delete m_pixmap;
}

Digikam::ImageIface* PerspectiveWidget::imageIface()
{
    return m_iface;
}

QPoint PerspectiveWidget::getTopLeftCorner(void)
{
    return QPoint( lroundf((float)(m_topLeftPoint.x()*m_origW) / (float)m_w),
                   lroundf((float)(m_topLeftPoint.y()*m_origH) / (float)m_h));
}

QPoint PerspectiveWidget::getTopRightCorner(void)
{
    return QPoint( lroundf((float)(m_topRightPoint.x()*m_origW) / (float)m_w),
                   lroundf((float)(m_topRightPoint.y()*m_origH) / (float)m_h));
}

QPoint PerspectiveWidget::getBottomLeftCorner(void)
{
    return QPoint( lroundf((float)(m_bottomLeftPoint.x()*m_origW) / (float)m_w),
                   lroundf((float)(m_bottomLeftPoint.y()*m_origH) / (float)m_h));
}

QPoint PerspectiveWidget::getBottomRightCorner(void)
{
    return QPoint( lroundf((float)(m_bottomRightPoint.x()*m_origW) / (float)m_w),
                   lroundf((float)(m_bottomRightPoint.y()*m_origH) / (float)m_h));
}

QRect PerspectiveWidget::getTargetSize(void)
{
    QPointArray perspectiveArea;

    perspectiveArea.putPoints( 0, 4,
                               getTopLeftCorner().x(), getTopLeftCorner().y(),
                               getTopRightCorner().x(), getTopRightCorner().y(),
                               getBottomRightCorner().x(), getBottomRightCorner().y(),
                               getBottomLeftCorner().x(), getBottomLeftCorner().y() );

    return perspectiveArea.boundingRect();
}

float PerspectiveWidget::getAngleTopLeft(void)
{
    Triangle topLeft(getTopLeftCorner(), getTopRightCorner(), getBottomLeftCorner());
    return topLeft.angleBAC();
}

float PerspectiveWidget::getAngleTopRight(void)
{
    Triangle topLeft(getTopRightCorner(), getBottomRightCorner(), getTopLeftCorner());
    return topLeft.angleBAC();
}

float PerspectiveWidget::getAngleBottomLeft(void)
{
    Triangle topLeft(getBottomLeftCorner(), getTopLeftCorner(), getBottomRightCorner());
    return topLeft.angleBAC();
}

float PerspectiveWidget::getAngleBottomRight(void)
{
    Triangle topLeft(getBottomRightCorner(), getBottomLeftCorner(), getTopRightCorner());
    return topLeft.angleBAC();
}

void PerspectiveWidget::reset(void)
{
    m_topLeftPoint.setX(0);
    m_topLeftPoint.setY(0);

    m_topRightPoint.setX(m_w-1);
    m_topRightPoint.setY(0);

    m_bottomLeftPoint.setX(0);
    m_bottomLeftPoint.setY(m_h-1);

    m_bottomRightPoint.setX(m_w-1);
    m_bottomRightPoint.setY(m_h-1);

    m_antiAlias = true;
    updatePixmap();
    repaint(false);
}

void PerspectiveWidget::applyPerspectiveAdjustment(void)
{
    Digikam::DImg *orgImage = m_iface->getOriginalImg();
    Digikam::DImg destImage(orgImage->width(), orgImage->height(), orgImage->sixteenBit(), orgImage->hasAlpha());

    Digikam::DColor background(0, 0, 0, orgImage->hasAlpha() ? 0 : 255, orgImage->sixteenBit());

    // Perform perspective adjustment.

    buildPerspective(QPoint(0, 0), QPoint(m_origW, m_origH),
                     getTopLeftCorner(), getTopRightCorner(),
                     getBottomLeftCorner(), getBottomRightCorner(),
                     orgImage, &destImage, background);

    // Perform an auto-croping around the image.

    Digikam::DImg targetImg = destImage.copy(getTargetSize());

    // Update target image.
    m_iface->putOriginalImage(i18n("Perspective Adjustment"),
                              targetImg.bits(), targetImg.width(), targetImg.height());
}

void PerspectiveWidget::toggleAntiAliasing(bool a)
{
    m_antiAlias = a; 
    updatePixmap();
    repaint(false);
}

void PerspectiveWidget::toggleDrawWhileMoving(bool draw)
{
    m_drawWhileMoving = draw;
}

void PerspectiveWidget::updatePixmap(void)
{
    m_topLeftCorner.setRect(m_topLeftPoint.x() + m_rect.topLeft().x(),
                            m_topLeftPoint.y() + m_rect.topLeft().y(), 8, 8);
    m_topRightCorner.setRect(m_topRightPoint.x() - 7 + m_rect.topLeft().x(),
                             m_topRightPoint.y() + m_rect.topLeft().y(), 8, 8);
    m_bottomLeftCorner.setRect(m_bottomLeftPoint.x() + m_rect.topLeft().x(),
                               m_bottomLeftPoint.y() - 7 + m_rect.topLeft().y(), 8, 8);
    m_bottomRightCorner.setRect(m_bottomRightPoint.x() - 7 + m_rect.topLeft().x(),
                                m_bottomRightPoint.y() - 7 + m_rect.topLeft().y(), 8, 8);

    // Draw background

    m_pixmap->fill(colorGroup().background());

    // if we are resizing with the mouse, compute and draw only if drawWhileMoving is set
    if (m_currentResizing == ResizingNone || m_drawWhileMoving)
    {
        // Create preview image

        Digikam::DImg destImage(m_previewImage.width(), m_previewImage.height(),
                                m_previewImage.sixteenBit(), m_previewImage.hasAlpha());

        Digikam::DColor background(colorGroup().background());

        m_transformedCenter = buildPerspective(QPoint(0, 0), QPoint(m_w, m_h),
                                               m_topLeftPoint, m_topRightPoint,
                                               m_bottomLeftPoint, m_bottomRightPoint,
                                               &m_previewImage, &destImage, background);

        m_iface->putPreviewImage(destImage.bits());

        // Draw image

        m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                       m_rect.width(), m_rect.height());
    }

    // Drawing selection borders.

    QPainter p(m_pixmap);
    p.setPen(QPen(QColor(255, 64, 64), 1, Qt::SolidLine));
    p.drawLine(m_topLeftPoint+m_rect.topLeft(),     m_topRightPoint+m_rect.topLeft());
    p.drawLine(m_topRightPoint+m_rect.topLeft(),    m_bottomRightPoint+m_rect.topLeft());
    p.drawLine(m_bottomRightPoint+m_rect.topLeft(), m_bottomLeftPoint+m_rect.topLeft());
    p.drawLine(m_bottomLeftPoint+m_rect.topLeft(),  m_topLeftPoint+m_rect.topLeft());

    // Drawing selection corners.

    QBrush brush(QColor(255, 64, 64));
    p.fillRect(m_topLeftCorner,     brush);
    p.fillRect(m_topRightCorner,    brush);
    p.fillRect(m_bottomLeftCorner,  brush);
    p.fillRect(m_bottomRightCorner, brush);

    // Drawing transformed center.

    p.setPen(QPen(QColor(255, 64, 64), 3, Qt::SolidLine));
    p.drawEllipse( m_transformedCenter.x()+m_rect.topLeft().x(), 
                   m_transformedCenter.y()+m_rect.topLeft().y(), 4, 4 ); 

    p.end();

    emit signalPerspectiveChanged( getTargetSize(), getAngleTopLeft(), getAngleTopRight(),
                                   getAngleBottomLeft(), getAngleBottomRight() );
}

void PerspectiveWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);
}

void PerspectiveWidget::resizeEvent(QResizeEvent * e)
{
    delete m_pixmap;
    int w          = e->size().width();
    int h          = e->size().height();
    uchar *data    = m_iface->setPreviewImageSize(w, h);
    m_w            = m_iface->previewWidth();
    m_h            = m_iface->previewHeight();
    m_previewImage = Digikam::DImg(m_w, m_h, m_iface->previewSixteenBit(), m_iface->previewHasAlpha(), data, false);

    m_pixmap      = new QPixmap(w, h);
    QRect oldRect = m_rect;
    m_rect        = QRect(w/2-m_w/2, h/2-m_h/2, m_w, m_h);

    float xFactor = (float)m_rect.width()/(float)(oldRect.width());
    float yFactor = (float)m_rect.height()/(float)(oldRect.height());

    m_topLeftPoint      = QPoint(lroundf(m_topLeftPoint.x()*xFactor),
                                 lroundf(m_topLeftPoint.y()*yFactor));
    m_topRightPoint     = QPoint(lroundf(m_topRightPoint.x()*xFactor),
                                 lroundf(m_topRightPoint.y()*yFactor));
    m_bottomLeftPoint   = QPoint(lroundf(m_bottomLeftPoint.x()*xFactor),
                                 lroundf(m_bottomLeftPoint.y()*yFactor));
    m_bottomRightPoint  = QPoint(lroundf(m_bottomRightPoint.x()*xFactor),
                                 lroundf(m_bottomRightPoint.y()*yFactor));
    m_transformedCenter = QPoint(lroundf(m_transformedCenter.x()*xFactor),
                                 lroundf(m_transformedCenter.y()*yFactor));

    updatePixmap();
}

void PerspectiveWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_rect.contains( e->x(), e->y() ))
    {
        if ( m_topLeftCorner.contains( e->x(), e->y() ) )
            m_currentResizing = ResizingTopLeft;
        else if ( m_bottomRightCorner.contains( e->x(), e->y() ) )
            m_currentResizing = ResizingBottomRight;
        else if ( m_topRightCorner.contains( e->x(), e->y() ) )
            m_currentResizing = ResizingTopRight;
        else if ( m_bottomLeftCorner.contains( e->x(), e->y() ) )
            m_currentResizing = ResizingBottomLeft;
    }
}

void PerspectiveWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_currentResizing != ResizingNone )
    {
        unsetCursor();
        m_currentResizing = ResizingNone;

        // in this case, the pixmap has not been drawn on mouse move
        if (!m_drawWhileMoving)
        {
            updatePixmap();
            repaint(false);
        }
    }
}

void PerspectiveWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
    {
        if ( m_currentResizing != ResizingNone )
        {
            QPointArray unsablePoints;
            QPoint pm(e->x(), e->y());

            if (!m_rect.contains( pm ))
            {
                if (pm.x() > m_rect.right())
                    pm.setX(m_rect.right());
                else if (pm.x() < m_rect.left())
                    pm.setX(m_rect.left());

                if (pm.y() > m_rect.bottom())
                    pm.setY(m_rect.bottom());
                else if (pm.y() < m_rect.top())
                    pm.setY(m_rect.top());
            }

            if ( m_currentResizing == ResizingTopLeft )
            {
                unsablePoints.putPoints(0, 7,
                                        m_w-1, m_h-1,
                                        0, m_h-1,
                                        0, m_bottomLeftPoint.y()-10,
                                        m_bottomLeftPoint.x(), m_bottomLeftPoint.y()-10,
                                        m_topRightPoint.x()-10, m_topRightPoint.y(),
                                        m_topRightPoint.x()-10, 0,
                                        m_w-1, 0 );
                QRegion unsableArea(unsablePoints);

                if ( unsableArea.contains(pm) ) return;

                m_topLeftPoint = pm - m_rect.topLeft();
                setCursor( KCursor::sizeFDiagCursor() );
            }

            else if ( m_currentResizing == ResizingTopRight )
            {
                unsablePoints.putPoints(0, 7,
                                        0, m_h-1,
                                        0, 0,
                                        m_topLeftPoint.x()+10, 0,
                                        m_topLeftPoint.x()+10, m_topLeftPoint.y(),
                                        m_bottomRightPoint.x(), m_bottomRightPoint.y()-10,
                                        m_w-1, m_bottomRightPoint.y()-10,
                                        m_w-1, m_h-1);
                QRegion unsableArea(unsablePoints);

                if ( unsableArea.contains(pm) ) return;

                m_topRightPoint = pm - m_rect.topLeft();
                setCursor( KCursor::sizeBDiagCursor() );
            }

            else if ( m_currentResizing == ResizingBottomLeft  )
            {
                unsablePoints.putPoints(0, 7,
                                        m_w-1, 0,
                                        m_w-1, m_h-1,
                                        m_bottomRightPoint.x()-10, m_h-1,
                                        m_bottomRightPoint.x()-10, m_bottomRightPoint.y()+10,
                                        m_topLeftPoint.x(), m_topLeftPoint.y()+10,
                                        0, m_topLeftPoint.y(),
                                        0, 0);
                QRegion unsableArea(unsablePoints);

                if ( unsableArea.contains(pm) ) return;

                m_bottomLeftPoint = pm - m_rect.topLeft();
                setCursor( KCursor::sizeBDiagCursor() );
            }

            else if ( m_currentResizing == ResizingBottomRight )
            {
                unsablePoints.putPoints(0, 7,
                                        0, 0,
                                        m_w-1, 0,
                                        m_w-1, m_topRightPoint.y()+10,
                                        m_topRightPoint.x(), m_topRightPoint.y()+10,
                                        m_bottomLeftPoint.x()+10, m_bottomLeftPoint.y(),
                                        m_bottomLeftPoint.x()+10, m_w-1,
                                        0, m_w-1);
                QRegion unsableArea(unsablePoints);

                if ( unsableArea.contains(pm) ) return;

                m_bottomRightPoint = pm - m_rect.topLeft();
                setCursor( KCursor::sizeFDiagCursor() );
            }

            updatePixmap();
            repaint(false);
        }
    }
    else
    {
        if ( m_topLeftCorner.contains( e->x(), e->y() ) ||
             m_bottomRightCorner.contains( e->x(), e->y() ) )
            setCursor( KCursor::sizeFDiagCursor() );

        else if ( m_topRightCorner.contains( e->x(), e->y() ) ||
                  m_bottomLeftCorner.contains( e->x(), e->y() ) )
            setCursor( KCursor::sizeBDiagCursor() );
        else
            unsetCursor();
    }
}

QPoint PerspectiveWidget::buildPerspective(QPoint orignTopLeft, QPoint orignBottomRight,
                                           QPoint transTopLeft, QPoint transTopRight,
                                           QPoint transBottomLeft, QPoint transBottomRight,
                                           Digikam::DImg *orgImage, Digikam::DImg *destImage,
                                           Digikam::DColor background)
{
    Matrix matrix, transform;
    double  scalex;
    double  scaley;

    double x1 = (double)orignTopLeft.x();
    double y1 = (double)orignTopLeft.y();

    double x2 = (double)orignBottomRight.x();
    double y2 = (double)orignBottomRight.y();

    double tx1 = (double)transTopLeft.x();
    double ty1 = (double)transTopLeft.y();

    double tx2 = (double)transTopRight.x();
    double ty2 = (double)transTopRight.y();

    double tx3 = (double)transBottomLeft.x();
    double ty3 = (double)transBottomLeft.y();

    double tx4 = (double)transBottomRight.x();
    double ty4 = (double)transBottomRight.y();

    scalex = scaley = 1.0;

    if ((x2 - x1) > 0)
      scalex = 1.0 / (double) (x2 - x1);

    if ((y2 - y1) > 0)
      scaley = 1.0 / (double) (y2 - y1);

    // Determine the perspective transform that maps from
    // the unit cube to the transformed coordinates

    double dx1, dx2, dx3, dy1, dy2, dy3;

    dx1 = tx2 - tx4;
    dx2 = tx3 - tx4;
    dx3 = tx1 - tx2 + tx4 - tx3;

    dy1 = ty2 - ty4;
    dy2 = ty3 - ty4;
    dy3 = ty1 - ty2 + ty4 - ty3;

    //  Is the mapping affine?  

    if ((dx3 == 0.0) && (dy3 == 0.0))
    {
        matrix.coeff[0][0] = tx2 - tx1;
        matrix.coeff[0][1] = tx4 - tx2;
        matrix.coeff[0][2] = tx1;
        matrix.coeff[1][0] = ty2 - ty1;
        matrix.coeff[1][1] = ty4 - ty2;
        matrix.coeff[1][2] = ty1;
        matrix.coeff[2][0] = 0.0;
        matrix.coeff[2][1] = 0.0;
    }
    else
    {
        double det1, det2;

        det1 = dx3 * dy2 - dy3 * dx2;
        det2 = dx1 * dy2 - dy1 * dx2;

        if (det1 == 0.0 && det2 == 0.0)
            matrix.coeff[2][0] = 1.0;
        else
            matrix.coeff[2][0] = det1 / det2;

        det1 = dx1 * dy3 - dy1 * dx3;

        if (det1 == 0.0 && det2 == 0.0)
            matrix.coeff[2][1] = 1.0;
        else
            matrix.coeff[2][1] = det1 / det2;

        matrix.coeff[0][0] = tx2 - tx1 + matrix.coeff[2][0] * tx2;
        matrix.coeff[0][1] = tx3 - tx1 + matrix.coeff[2][1] * tx3;
        matrix.coeff[0][2] = tx1;

        matrix.coeff[1][0] = ty2 - ty1 + matrix.coeff[2][0] * ty2;
        matrix.coeff[1][1] = ty3 - ty1 + matrix.coeff[2][1] * ty3;
        matrix.coeff[1][2] = ty1;
    }

    matrix.coeff[2][2] = 1.0;

    // transform is initialized to the identity matrix
    transform.translate(-x1, -y1);
    transform.scale    (scalex, scaley);
    transform.multiply (matrix);

    // perspective transformation
    transformAffine(orgImage, destImage, transform, background);

    // Calculate and return new image center
    double newCenterX, newCenterY;
    transform.transformPoint(x2/2.0, y2/2.0, &newCenterX, &newCenterY);

    return QPoint(lround(newCenterX), lround(newCenterY));
}

void PerspectiveWidget::transformAffine(Digikam::DImg *orgImage, Digikam::DImg *destImage,
                                        const Matrix &matrix, Digikam::DColor background)
{
    Matrix      m(matrix), inv(matrix);

    int         x1, y1, x2, y2;        // target bounding box 
    int         x, y;                  // target coordinates 
    int         u1, v1, u2, v2;        // source bounding box 
    double      uinc, vinc, winc;      // increments in source coordinates
                                       // pr horizontal target coordinate 

    double      u[5],v[5];             // source coordinates,
                                       //   2
                                       //  / \    0 is sample in the centre of pixel
                                       // 1 0 3   1..4 is offset 1 pixel in each
                                       //  \ /    direction (in target space)
                                       //   4

    double      tu[5],tv[5],tw[5];     // undivided source coordinates and divisor 

    uchar      *data, *newData;
    bool        sixteenBit;
    int         coords;
    int         width, height;
    int         bytesDepth;
    int         offset;
    uchar      *dest, *d;
    Digikam::DColor color;

    bytesDepth  = orgImage->bytesDepth();
    data        = orgImage->bits();
    sixteenBit  = orgImage->sixteenBit();
    width       = orgImage->width();
    height      = orgImage->height();
    newData     = destImage->bits();

    if (sixteenBit)
        background.convertToSixteenBit();

    //destImage->fill(background);

    Digikam::DImgImageFilters filters;

    // Find the inverse of the transformation matrix
    m.invert();

    u1 = 0;
    v1 = 0;
    u2 = u1 + width;
    v2 = v1 + height;

    x1 = u1;
    y1 = v1;
    x2 = u2;
    y2 = v2;

    dest = new uchar[width * bytesDepth];

    uinc = m.coeff[0][0];
    vinc = m.coeff[1][0];
    winc = m.coeff[2][0];

    coords = 1;

    // these loops could be rearranged, depending on which bit of code
    // you'd most like to write more than once.

    for (y = y1; y < y2; y++)
    {
       // set up inverse transform steps 

        tu[0] = uinc * (x1 + 0.5) + m.coeff[0][1] * (y + 0.5) + m.coeff[0][2] - 0.5;
        tv[0] = vinc * (x1 + 0.5) + m.coeff[1][1] * (y + 0.5) + m.coeff[1][2] - 0.5;
        tw[0] = winc * (x1 + 0.5) + m.coeff[2][1] * (y + 0.5) + m.coeff[2][2];

        d = dest;

        for (x = x1; x < x2; x++)
        {
            int i;     //  normalize homogeneous coords

            for (i = 0; i < coords; i++)
            {
                if (tw[i] == 1.0)
                {
                    u[i] = tu[i];
                    v[i] = tv[i];
                }
                else if (tw[i] != 0.0)
                {
                    u[i] = tu[i] / tw[i];
                    v[i] = tv[i] / tw[i];
                }
                else
                {
                    DDebug() << "homogeneous coordinate = 0...\n" << endl;
                }
            }

            //  Set the destination pixels  

            int   iu = lround( u [0] );
            int   iv = lround( v [0] );

            if (iu >= u1 && iu < u2 && iv >= v1 && iv < v2)
            {
                // u, v coordinates into source  

                int u = iu - u1;
                int v = iv - v1;

                //TODO: Check why antialiasing shows no effect
                /*if (m_antiAlias)
                {
                    if (sixteenBit)
                    {
                        unsigned short *d16 = (unsigned short *)d;
                        filters.pixelAntiAliasing16((unsigned short *)data,
                                                  width, height, u, v, d16+3, d16+2, d16+1, d16);
                    }
                    else
                    {
                        filters.pixelAntiAliasing(data, width, height, u, v,
                                                                  d+3, d+2, d+1, d);
                    }
                }
                else
                {*/
                offset = (v * width * bytesDepth) + (u * bytesDepth);
                color.setColor(data + offset, sixteenBit);
                color.setPixel(d);
                //}

                d += bytesDepth;
            }
            else // not in source range
            {
                // set to background color

                background.setPixel(d);
                d += bytesDepth;
            }

            for (i = 0; i < coords; i++)
            {
                tu[i] += uinc;
                tv[i] += vinc;
                tw[i] += winc;
            }
        }

        //  set the pixel region row

        offset = (y - y1) * width * bytesDepth;
        memcpy(newData + offset, dest, width * bytesDepth);
    }

    delete [] dest;
}


}  // NameSpace DigikamPerspectiveImagesPlugin


#include "perspectivewidget.moc"
