/* ============================================================
 * File  : perspectivewidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-18
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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
 
 extern "C"
{
#include <math.h>
}

// C++ includes.

#include <cstdio>
#include <cstdlib>

// Qt includes.

#include <qregion.h>
#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpointarray.h>
#include <qregion.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "perspectivewidget.h"

namespace DigikamPerspectiveImagesPlugin
{

Triangle::Triangle(QPoint A, QPoint B, QPoint C)
{
    m_a = distanceP2P(B, C);
    m_b = distanceP2P(A, C);
    m_c = distanceP2P(A, B);
}

float Triangle::angleABC(void)
{
    return( 57.295779513082 * acos( (m_b*m_b - m_a*m_a - m_c*m_c ) / (-2*m_a*m_c ) ) );
}

float Triangle::angleACB(void)
{
    return( 57.295779513082 * acos( (m_c*m_c - m_a*m_a - m_b*m_b ) / (-2*m_a*m_b ) ) );
}

float Triangle::angleBAC(void)
{
    return( 57.295779513082 * acos( (m_a*m_a - m_b*m_b - m_c*m_c ) / (-2*m_b*m_c ) ) );
}

float Triangle::distanceP2P(QPoint p1, QPoint p2)
{
    return(sqrt( abs( p2.x()-p1.x() ) * abs( p2.x()-p1.x() ) +
                 abs( p2.y()-p1.y() ) * abs( p2.y()-p1.y() ) ));
}


PerspectiveWidget::PerspectiveWidget(int w, int h, QWidget *parent)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_iface  = new Digikam::ImageIface(w,h);

    m_data   = m_iface->getPreviewData();
    m_w      = m_iface->previewWidth();
    m_h      = m_iface->previewHeight();
    m_origW  = m_iface->originalWidth();
    m_origH  = m_iface->originalHeight();
    m_pixmap = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(m_w, m_h);
    setMouseTracking(true);
    
    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);

    resetSelection();
}

PerspectiveWidget::~PerspectiveWidget()
{
    delete [] m_data;
    delete m_iface;
    delete m_pixmap;
}

Digikam::ImageIface* PerspectiveWidget::imageIface()
{
    return m_iface;
}

QPoint PerspectiveWidget::getTopLeftCorner(void)
{
    return QPoint::QPoint( (int)((float)(m_topLeftPoint.x()*m_origW) / (float)m_w), 
                           (int)((float)(m_topLeftPoint.y()*m_origH) / (float)m_h));
}

QPoint PerspectiveWidget::getTopRightCorner(void)
{
    return QPoint::QPoint( (int)((float)(m_topRightPoint.x()*m_origW) / (float)m_w), 
                           (int)((float)(m_topRightPoint.y()*m_origH) / (float)m_h));
}

QPoint PerspectiveWidget::getBottomLeftCorner(void)
{
    return QPoint::QPoint( (int)((float)(m_bottomLeftPoint.x()*m_origW) / (float)m_w), 
                           (int)((float)(m_bottomLeftPoint.y()*m_origH) / (float)m_h));
}

QPoint PerspectiveWidget::getBottomRightCorner(void)
{
    return QPoint::QPoint( (int)((float)(m_bottomRightPoint.x()*m_origW) / (float)m_w), 
                           (int)((float)(m_bottomRightPoint.y()*m_origH) / (float)m_h));
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

void PerspectiveWidget::resetSelection(void)
{
    m_topLeftPoint.setX(0);
    m_topLeftPoint.setY(0);
    
    m_topRightPoint.setX(m_w-1);
    m_topRightPoint.setY(0);
    
    m_bottomLeftPoint.setX(0);
    m_bottomLeftPoint.setY(m_h-1);
    
    m_bottomRightPoint.setX(m_w-1);
    m_bottomRightPoint.setY(m_h-1);

    updatePixmap();
    repaint(false);
}

void PerspectiveWidget::applyPerspectiveAdjusment(void)
{
    uint *data    = m_iface->getOriginalData();
    uint *newData = new uint[m_origW * m_origH];
    
    // Perform perspective adjustment.
    
    m_transformedCenter = buildPerspective(QPoint::QPoint(0, 0), QPoint::QPoint(m_origW, m_origH),
                                           getTopLeftCorner(), getTopRightCorner(), 
                                           getBottomLeftCorner(), getBottomRightCorner(), 
                                           data, newData);

    // Perform an auto-croping around the image.
            
    QImage newImage, targetImg;
    newImage.create( m_origW, m_origH, 32 );
    memcpy(newImage.bits(), newData, newImage.numBytes());
    targetImg = newImage.copy(getTargetSize());
    
    // Update target image.
    Digikam::ImageFilters::smartBlurImage((uint*)targetImg.bits(), targetImg.width(), targetImg.height());
    m_iface->putOriginalData(i18n("Perspective Adjustment"),
                             (uint*)targetImg.bits(), targetImg.width(), targetImg.height());

    delete [] data;
    delete [] newData;
}

void PerspectiveWidget::updatePixmap(void)
{
    m_topLeftCorner.setRect(m_topLeftPoint.x(), m_topLeftPoint.y(), 8, 8);
    m_topRightCorner.setRect(m_topRightPoint.x() - 7, m_topRightPoint.y(), 8, 8);
    m_bottomLeftCorner.setRect(m_bottomLeftPoint.x(), m_bottomLeftPoint.y() - 7, 8, 8);   
    m_bottomRightCorner.setRect(m_bottomRightPoint.x() - 7, m_bottomRightPoint.y() - 7, 8, 8);
    
    // Drawing background and image transformation.
    
    m_pixmap->fill(colorGroup().background());
    
    uint *newData = new uint[m_w * m_h];
    
    m_transformedCenter = buildPerspective(QPoint::QPoint(0, 0), QPoint::QPoint(m_w, m_h),
                                                  m_topLeftPoint, m_topRightPoint, 
                                                  m_bottomLeftPoint, m_bottomRightPoint, 
                                                  m_data, newData);
                                                   
    m_iface->putPreviewData(newData);

    delete [] newData;
    
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());
    
    // Drawing selection borders.
    
    QPainter p(m_pixmap);
    p.setPen(QPen(QColor(255, 64, 64), 1, Qt::SolidLine));
    p.drawLine(m_topLeftPoint, m_topRightPoint);
    p.drawLine(m_topRightPoint, m_bottomRightPoint);
    p.drawLine(m_bottomRightPoint, m_bottomLeftPoint);
    p.drawLine(m_bottomLeftPoint, m_topLeftPoint);
    
    // Drawing selection corners.
    
    QBrush brush(QColor(255, 64, 64));
    p.fillRect(m_topLeftCorner, brush);
    p.fillRect(m_topRightCorner, brush);
    p.fillRect(m_bottomLeftCorner, brush);
    p.fillRect(m_bottomRightCorner, brush);

    // Drawing transformed center.
    
    p.setPen(QPen(QColor(255, 64, 64), 3, Qt::SolidLine));
    p.drawEllipse( m_transformedCenter.x(), m_transformedCenter.y(), 4, 4 ); 

    p.end();
    
    emit signalPerspectiveChanged( getTargetSize(), getAngleTopLeft(), getAngleTopRight(),
                                   getAngleBottomLeft(), getAngleBottomRight() );
}

void PerspectiveWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);                   
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
       setCursor ( KCursor::arrowCursor() );
       m_currentResizing = ResizingNone;
       } 
}

void PerspectiveWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton &&
         m_rect.contains( e->x(), e->y() ))
       {
       if ( m_currentResizing != ResizingNone )
          {
          QPointArray unsablePoints;
          QPoint pm(e->x(), e->y());        
                    
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
             
             m_topLeftPoint = pm;             
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
             
             m_topRightPoint = pm;
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
             
             m_bottomLeftPoint = pm;
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
             
             m_bottomRightPoint = pm;
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
           setCursor ( KCursor::arrowCursor() );
       }
}

QPoint PerspectiveWidget::buildPerspective(QPoint orignTopLeft, QPoint orignBottomRight,
                                           QPoint transTopLeft, QPoint transTopRight,
                                           QPoint transBottomLeft, QPoint transBottomRight,
                                           uint* data, uint* newData)
{
    Matrix3 matrix, transform;
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
 
    matrix3Identity (&transform);
    matrix3Translate(&transform, -x1, -y1);
    matrix3Scale    (&transform, scalex, scaley);
    matrix3Mult     (&matrix, &transform);
    
    // Calculate new image center after perspective transformation.
    
    transformAffine(data, newData, &transform, (int)x2, (int)y2);
    double newCenterX, newCenterY;
    matrix3TransformPoint(&transform, x2/2.0, y2/2.0, &newCenterX, &newCenterY);

    return( QPoint::QPoint((int)newCenterX, (int)newCenterY) );
}

void PerspectiveWidget::transformAffine(uint *data, uint *newData, const Matrix3 *matrix, int w, int h)
{
    Matrix3     m;
    Matrix3     inv;
  
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

    int         coords;
    int         width;
    int         alpha;
    int         bytes;
    uchar      *dest, *d;
    uchar       bg_color[4];

    m   = *matrix;
    inv = *matrix;

    // Background color  
  
    memset(bg_color, 0, sizeof(bg_color));
  
    // RGBA image !
  
    bg_color[4] = 0;
    alpha = 4;

    // "Outside" a channel is transparency, not the bg color  
  
    bg_color[0] = 0; 

    // Find the inverse of the transformation matrix  
  
    matrix3Invert (&m);

    u1 = 0;
    v1 = 0;
    u2 = u1 + w;
    v2 = v1 + h;

    x1 = u1;
    y1 = v1;
    x2 = u2;
    y2 = v2;
  
    width  = w;
    bytes  = 4;

    dest = new uchar[w * bytes];

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
                kdDebug() << "homogeneous coordinate = 0...\n" << endl;
                }
             }

          //  Set the destination pixels  
                
          uchar color[4];
          int   iu = (int) u [0];
          int   iv = (int) v [0];
          int   b;

          if (iu >= u1 && iu < u2 && iv >= v1 && iv < v2)
             {
             //  u, v coordinates into source  
           
             int u = iu - u1;
             int v = iv - v1;

             memcpy(color, (uchar*)(data + (u + v*w)), 4);

             for (b = 0; b < bytes; b++)
                *d++ = color[b];
             }
          else // not in source range 
             {
             //  increment the destination pointers  
            
             for (b = 0; b < bytes; b++)
                *d++ = bg_color[b];
             }
            
          for (i = 0; i < coords; i++)
             {
             tu[i] += uinc;
             tv[i] += vinc;
             tw[i] += winc;
             }
          }

       //  set the pixel region row  
      
       memcpy((uchar*)(newData + (y - y1)*w), dest, width*bytes);
       }

    delete [] dest;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix 3x3 perspective transformation implementations.

/**
 * matrix3Identity:
 * @matrix: A matrix.
 *
 * Sets the matrix to the identity matrix.
 */
void PerspectiveWidget::matrix3Identity(Matrix3 *matrix)
{
    static const Matrix3 identity = { { { 1.0, 0.0, 0.0 },
                                        { 0.0, 1.0, 0.0 },
                                        { 0.0, 0.0, 1.0 } } };

    *matrix = identity;
}

/**
 * matrix3Translate:
 * @matrix: The matrix that is to be translated.
 * @x: Translation in X direction.
 * @y: Translation in Y direction.
 *
 * Translates the matrix by x and y.
 */
void PerspectiveWidget::matrix3Translate (Matrix3 *matrix, double x, double y)
{
    double g, h, i;

    g = matrix->coeff[2][0];
    h = matrix->coeff[2][1];
    i = matrix->coeff[2][2];

    matrix->coeff[0][0] += x * g;
    matrix->coeff[0][1] += x * h;
    matrix->coeff[0][2] += x * i;
    matrix->coeff[1][0] += y * g;
    matrix->coeff[1][1] += y * h;
    matrix->coeff[1][2] += y * i;
}


/**
 * matrix3Scale:
 * @matrix: The matrix that is to be scaled.
 * @x: X scale factor.
 * @y: Y scale factor.
 *
 * Scales the matrix by x and y
 */
void PerspectiveWidget::matrix3Scale(Matrix3 *matrix, double x, double y)
{
    matrix->coeff[0][0] *= x;
    matrix->coeff[0][1] *= x;
    matrix->coeff[0][2] *= x;

    matrix->coeff[1][0] *= y;
    matrix->coeff[1][1] *= y;
    matrix->coeff[1][2] *= y;
}

/**
 * matrix3Mult:
 * @matrix1: The first input matrix.
 * @matrix2: The second input matrix which will be overwritten by the result.
 *
 * Multiplies two matrices and puts the result into the second one.
 */
void PerspectiveWidget::matrix3Mult(const Matrix3 *matrix1, Matrix3 *matrix2)
{
    int      i, j;
    Matrix3  tmp;
    double   t1, t2, t3;

    for (i = 0; i < 3; i++)
       {
       t1 = matrix1->coeff[i][0];
       t2 = matrix1->coeff[i][1];
       t3 = matrix1->coeff[i][2];

       for (j = 0; j < 3; j++)
          {
          tmp.coeff[i][j]  = t1 * matrix2->coeff[0][j];
          tmp.coeff[i][j] += t2 * matrix2->coeff[1][j];
          tmp.coeff[i][j] += t3 * matrix2->coeff[2][j];
          }
       }

    *matrix2 = tmp;
}

/**
 * matrix3TransformPoint:
 * @matrix: The transformation matrix.
 * @x: The source X coordinate.
 * @y: The source Y coordinate.
 * @newx: The transformed X coordinate.
 * @newy: The transformed Y coordinate.
 *
 * Transforms a point in 2D as specified by the transformation matrix.
 */
void PerspectiveWidget::matrix3TransformPoint(const Matrix3 *matrix, double x, double y,
                                              double *newx, double *newy)
{
    double  w;

    w = matrix->coeff[2][0] * x + matrix->coeff[2][1] * y + matrix->coeff[2][2];

    if (w == 0.0)
       w = 1.0;
    else
       w = 1.0/w;

    *newx = (matrix->coeff[0][0] * x +
             matrix->coeff[0][1] * y +
             matrix->coeff[0][2]) * w;
    *newy = (matrix->coeff[1][0] * x +
             matrix->coeff[1][1] * y +
             matrix->coeff[1][2]) * w;
}

/**
 * matrix3Invert:
 * @matrix: The matrix that is to be inverted.
 *
 * Inverts the given matrix.
 */
void PerspectiveWidget::matrix3Invert(Matrix3 *matrix)
{
    Matrix3 inv;
    double  det;

    det = matrix3Determinant(matrix);

    if (det == 0.0)
       return;

    det = 1.0 / det;

    inv.coeff[0][0] =   (matrix->coeff[1][1] * matrix->coeff[2][2] -
                         matrix->coeff[1][2] * matrix->coeff[2][1]) * det;

    inv.coeff[1][0] = - (matrix->coeff[1][0] * matrix->coeff[2][2] -
                         matrix->coeff[1][2] * matrix->coeff[2][0]) * det;

    inv.coeff[2][0] =   (matrix->coeff[1][0] * matrix->coeff[2][1] -
                         matrix->coeff[1][1] * matrix->coeff[2][0]) * det;

    inv.coeff[0][1] = - (matrix->coeff[0][1] * matrix->coeff[2][2] -
                         matrix->coeff[0][2] * matrix->coeff[2][1]) * det;

    inv.coeff[1][1] =   (matrix->coeff[0][0] * matrix->coeff[2][2] -
                         matrix->coeff[0][2] * matrix->coeff[2][0]) * det;

    inv.coeff[2][1] = - (matrix->coeff[0][0] * matrix->coeff[2][1] -
                         matrix->coeff[0][1] * matrix->coeff[2][0]) * det;

    inv.coeff[0][2] =   (matrix->coeff[0][1] * matrix->coeff[1][2] -
                         matrix->coeff[0][2] * matrix->coeff[1][1]) * det;

    inv.coeff[1][2] = - (matrix->coeff[0][0] * matrix->coeff[1][2] -
                         matrix->coeff[0][2] * matrix->coeff[1][0]) * det;

    inv.coeff[2][2] =   (matrix->coeff[0][0] * matrix->coeff[1][1] -
                         matrix->coeff[0][1] * matrix->coeff[1][0]) * det;

    *matrix = inv;
}

/**
 * matrix3Determinant:
 * @matrix: The input matrix.
 *
 * Calculates the determinant of the given matrix.
 *
 * Returns: The determinant.
 */
double PerspectiveWidget::matrix3Determinant(const Matrix3 *matrix)
{
    double determinant;

    determinant  = (matrix->coeff[0][0] *
                    (matrix->coeff[1][1] * matrix->coeff[2][2] -
                     matrix->coeff[1][2] * matrix->coeff[2][1]));
    determinant -= (matrix->coeff[1][0] *
                    (matrix->coeff[0][1] * matrix->coeff[2][2] -
                     matrix->coeff[0][2] * matrix->coeff[2][1]));
    determinant += (matrix->coeff[2][0] *
                    (matrix->coeff[0][1] * matrix->coeff[1][2] -
                     matrix->coeff[0][2] * matrix->coeff[1][1]));

    return determinant;
}

}  // NameSpace DigikamPerspectiveImagesPlugin


#include "perspectivewidget.moc"
