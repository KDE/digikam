/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-09
 * Description : image selection widget used by ratio crop tool.
 *
 * Copyright 2004-2005 by Gilles Caulier
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

#define OPACITY   0.7
#define RCOL8     0xAA
#define GCOL8     0xAA
#define BCOL8     0xAA
#define RCOL16    0xAAAA
#define GCOL16    0xAAAA
#define BCOL16    0xAAAA

#define MINRANGE  50

// Fibanocci irrationel Golden Number.
#define PHI       1.618033988 
// 1/PHI
#define INVPHI    0.61803398903633

// C++ includes.

#include <cstdio>
#include <cmath>

// Qt includes.

#include <qregion.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qbrush.h> 
#include <qpixmap.h>
#include <qimage.h>
#include <qpen.h>
#include <qpoint.h> 
#include <qtimer.h> 
#include <qsizepolicy.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include "imageiface.h"

// Local includes.

#include "imageselectionwidget.h"

namespace Digikam
{

ImageSelectionWidget::ImageSelectionWidget(int w, int h, QWidget *parent, 
                                           float aspectRatioValue, int aspectRatioType, 
                                           int orient, int guideLinesType)
                    : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_currentAspectRatioType  = aspectRatioType; 
    m_currentAspectRatioValue = aspectRatioValue;
    m_currentOrientation      = orient; 
    m_currentResizing         = ResizingNone;
    m_guideLinesType          = guideLinesType;
    m_timerW                  = 0L;
    m_timerH                  = 0L;
    m_iface                   = 0L;
    m_pixmap                  = 0L;
    m_guideSize               = 1;

    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    m_iface         = new ImageIface(w, h);
    uchar *data     = m_iface->getPreviewImage();
    int width       = m_iface->previewWidth();
    int height      = m_iface->previewHeight();
    bool sixteenBit = m_iface->previewSixteenBit();
    bool hasAlpha   = m_iface->previewHasAlpha();
    m_preview       = DImg(width, height, sixteenBit, hasAlpha, data);
    delete [] data;
    
    m_pixmap  = new QPixmap(w, h);

    m_rect = QRect(w/2-m_preview.width()/2, h/2-m_preview.height()/2, m_preview.width(), m_preview.height());
    realToLocalRegion();
    updatePixmap();
    setGoldenGuideTypes(true, false, false, false, false, false);
}

ImageSelectionWidget::~ImageSelectionWidget()
{
    if (m_timerW)
       delete m_timerW;

    if (m_timerH)
       delete m_timerH;

    delete m_iface;
    delete m_pixmap;
}

ImageIface* ImageSelectionWidget::imageIface()
{
    return m_iface;
}

void ImageSelectionWidget::resizeEvent(QResizeEvent *e)
{
    delete m_pixmap;

    int w           = e->size().width();
    int h           = e->size().height();
    
    uchar *data     = m_iface->setPreviewImageSize(w, h);
    int width       = m_iface->previewWidth();
    int height      = m_iface->previewHeight();
    bool sixteenBit = m_iface->previewSixteenBit();
    bool hasAlpha   = m_iface->previewHasAlpha();
    m_preview       = DImg(width, height, sixteenBit, hasAlpha, data);
    delete [] data;
    
    m_pixmap = new QPixmap(w, h);

    m_rect = QRect(w/2-m_preview.width()/2, h/2-m_preview.height()/2, m_preview.width(), m_preview.height());
    realToLocalRegion();
    updatePixmap();
}

int ImageSelectionWidget::getOriginalImageWidth(void)
{
    return m_iface->originalWidth();
}

int ImageSelectionWidget::getOriginalImageHeight(void)
{
    return m_iface->originalHeight();
}

QRect ImageSelectionWidget::getRegionSelection(void)
{
    return m_regionSelection;
}

int ImageSelectionWidget::getMinWidthRange(void)
{
    return( (int)( ((float)MINRANGE - (float)m_rect.x() ) * 
                   ( (float)m_iface->originalWidth() / (float)m_preview.width() )) );
}

int ImageSelectionWidget::getMinHeightRange(void)
{
    return( (int)( ((float)MINRANGE - (float)m_rect.y() ) *
                   ( (float)m_iface->originalHeight() / (float)m_preview.height() )) );
}

void ImageSelectionWidget::resetSelection(void)
{
    m_regionSelection.moveTopLeft(QPoint::QPoint(0, 0));
    m_regionSelection.setWidth((int)(m_iface->originalWidth()/2.0));
    m_regionSelection.setHeight((int)(m_iface->originalHeight()/2.0));
    realToLocalRegion();
    applyAspectRatio(false, false);

    m_localRegionSelection.moveBy(
      m_rect.width()/2 - m_localRegionSelection.width()/2,
      m_rect.height()/2 - m_localRegionSelection.height()/2);

    applyAspectRatio(false, true, false);
    regionSelectionChanged(true);
}

void ImageSelectionWidget::setCenterSelection(int centerType)
{
    switch (centerType)
    {
       case CenterWidth:
          m_regionSelection.moveLeft(0);
          break;

       case CenterHeight:
          m_regionSelection.moveTop(0);
          break;

       case CenterImage:
          m_regionSelection.moveTopLeft(QPoint::QPoint(0, 0));
          break;
    }

    realToLocalRegion();
    applyAspectRatio(false, false);

    switch (centerType)
    {
       case CenterWidth:
          m_localRegionSelection.moveBy(
            m_rect.width()/2 - m_localRegionSelection.width()/2, 
            0);
          break;

       case CenterHeight:
          m_localRegionSelection.moveBy(
            0, 
            m_rect.height()/2 - m_localRegionSelection.height()/2);
          break;

       case CenterImage:
          m_localRegionSelection.moveBy(
            m_rect.width()/2 - m_localRegionSelection.width()/2,
            m_rect.height()/2 - m_localRegionSelection.height()/2);
          break;
    }

    applyAspectRatio(false, true, false);
    regionSelectionChanged(true);
}

void ImageSelectionWidget::maxAspectSelection(void)
{
    m_localRegionSelection.setTopLeft( m_rect.topLeft() );

    if ( !m_currentOrientation )   // Landscape
    {
       m_localRegionSelection.setWidth(m_rect.width());
       applyAspectRatio(false, false);

       if ( m_localRegionSelection.height() > m_rect.height() )
       {
          m_localRegionSelection.setHeight(m_rect.height());
          applyAspectRatio(true, false);
       }
    }
    else                          // Portrait
    {
       m_localRegionSelection.setHeight(m_rect.height());
       applyAspectRatio(true, false);

       if ( m_localRegionSelection.width() > m_rect.width() )
       {
          m_localRegionSelection.setWidth(m_rect.width());
          applyAspectRatio(false, false);
       }
    }

    setCenterSelection(CenterImage);
}

void ImageSelectionWidget::setGoldenGuideTypes(bool drawGoldenSection,  bool drawGoldenSpiralSection,
                                               bool drawGoldenSpiral,   bool drawGoldenTriangle,
                                               bool flipHorGoldenGuide, bool flipVerGoldenGuide)
{
    m_drawGoldenSection       = drawGoldenSection;
    m_drawGoldenSpiralSection = drawGoldenSpiralSection;
    m_drawGoldenSpiral        = drawGoldenSpiral;
    m_drawGoldenTriangle      = drawGoldenTriangle;
    m_flipHorGoldenGuide      = flipHorGoldenGuide;
    m_flipVerGoldenGuide      = flipVerGoldenGuide;
}

void ImageSelectionWidget::slotGuideLines(int guideLinesType)
{
    m_guideLinesType = guideLinesType;
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::slotChangeGuideColor(const QColor &color)
{
    m_guideColor = color;
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::slotChangeGuideSize(int size)
{
    m_guideSize = size;
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::setSelectionOrientation(int orient)
{
    m_currentOrientation = orient;
    applyAspectRatio(true);
}

void ImageSelectionWidget::setSelectionAspectRatioType(int aspectRatioType)
{
    m_currentAspectRatioType = aspectRatioType;

    switch(aspectRatioType)
    {
       case RATIO01X01:  
          m_currentAspectRatioValue = 1.0; 
          break;

       case RATIO03X04:  
          m_currentAspectRatioValue = 0.75; 
          break;

       case RATIO02x03:
          m_currentAspectRatioValue = 0.66666666666667; 
          break;

       case RATIO05x07:
          m_currentAspectRatioValue = 0.71428571428571; 
          break;

       case RATIO07x10:
          m_currentAspectRatioValue = 0.7; 
          break;

       case RATIO04X05:
          m_currentAspectRatioValue = 0.8; 
          break;

       case RATIOGOLDEN:
          m_currentAspectRatioValue = INVPHI;
          break;
    }

    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionAspectRatioValue(float aspectRatioValue)
{
    m_currentAspectRatioValue = aspectRatioValue;
    m_currentAspectRatioType  = RATIOCUSTOM;
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionX(int x)
{
    m_regionSelection.moveLeft(x);
    realToLocalRegion();
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::setSelectionY(int y)
{
    m_regionSelection.moveTop(y);
    realToLocalRegion();
    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::setSelectionWidth(int w)
{
    m_regionSelection.setWidth(w);
    realToLocalRegion(true);    
    applyAspectRatio(false, true, false);

    if (m_currentAspectRatioType == RATIONONE)
    {
       emit signalSelectionChanged( m_regionSelection );  
       return;
    }

    localToRealRegion();
    emit signalSelectionHeightChanged(m_regionSelection.height());

    if (m_timerW)
    {
       m_timerW->stop();
       delete m_timerW;
    }

    m_timerW = new QTimer( this );
    connect( m_timerW, SIGNAL(timeout()),
             this, SLOT(slotTimerDone()) );
    m_timerW->start(500, true);
}

void ImageSelectionWidget::setSelectionHeight(int h)
{
    m_regionSelection.setHeight(h);
    realToLocalRegion(true);
    applyAspectRatio(true, true, false);

    if (m_currentAspectRatioType == RATIONONE)
    {
       emit signalSelectionChanged( m_regionSelection );  
       return;
    }

    localToRealRegion();
    emit signalSelectionWidthChanged(m_regionSelection.width());

    if (m_timerH)
    {
       m_timerH->stop();
       delete m_timerH;
    }

    m_timerH = new QTimer( this );
    connect( m_timerH, SIGNAL(timeout()),
             this, SLOT(slotTimerDone()) );
    m_timerH->start(500, true);
}

void ImageSelectionWidget::slotTimerDone(void)
{
    regionSelectionChanged(true);
}

void ImageSelectionWidget::realToLocalRegion(bool updateSizeOnly)
{
    if (!updateSizeOnly)
    {
       if (m_regionSelection.x() == 0 )
          m_localRegionSelection.setX(m_rect.x());
       else
          m_localRegionSelection.setX( 1 + m_rect.x() + (int)((float)m_regionSelection.x() * 
                                      ( (float)m_preview.width() / (float)m_iface->originalWidth() )) );

       if (m_regionSelection.y() == 0 ) 
          m_localRegionSelection.setY(m_rect.y());
       else
          m_localRegionSelection.setY( 1 + m_rect.y() + (int)((float)m_regionSelection.y() * 
                                      ( (float)m_preview.height() / (float)m_iface->originalHeight() )) );
    }

    m_localRegionSelection.setWidth( (int)((float)m_regionSelection.width() *
                                          ( (float)m_preview.width() / (float)m_iface->originalWidth() )) );

    m_localRegionSelection.setHeight( (int)((float)m_regionSelection.height() *
                                           ( (float)m_preview.height() / (float)m_iface->originalHeight() )) );
}

void ImageSelectionWidget::localToRealRegion(void)
{
    int x = (int)( ((float)m_localRegionSelection.x() - (float)m_rect.x() ) * 
                   ( (float)m_iface->originalWidth() / (float)m_preview.width() ));

    int y = (int)( ((float)m_localRegionSelection.y() - (float)m_rect.y() ) *
                   ( (float)m_iface->originalHeight() / (float)m_preview.height() ));

    int w = (int)((float)m_localRegionSelection.width() *
                 ( (float)m_iface->originalWidth() / (float)m_preview.width() ));

    int h = (int)((float)m_localRegionSelection.height() *
                 ( (float)m_iface->originalHeight() / (float)m_preview.height() ));

    m_regionSelection.setRect(x, y, w, h);
}

void ImageSelectionWidget::applyAspectRatio(bool WOrH, bool repaintWidget, bool updateChange)
{
    // Save local selection area for re-adjustment after changing width and height.
    QRect oldLocalRegionSelection = m_localRegionSelection;

    if ( !WOrH )  // Width changed.
    {    
       int w = m_localRegionSelection.width();

       switch(m_currentAspectRatioType)
       {
          case RATIONONE:  
             break;

          default:
             if ( m_currentOrientation )  
                m_localRegionSelection.setHeight((int)(w / m_currentAspectRatioValue));  // Landscape
             else                       
                m_localRegionSelection.setHeight((int)(w * m_currentAspectRatioValue));  // Portrait
             break;
       }
    }  
    else      // Height changed.
    {
       int h = m_localRegionSelection.height();

       switch(m_currentAspectRatioType)
       {
          case RATIONONE:  
             break;

          default:
             if ( m_currentOrientation )  
                m_localRegionSelection.setWidth((int)(h * m_currentAspectRatioValue));   // Portrait
             else
                m_localRegionSelection.setWidth((int)(h / m_currentAspectRatioValue));   // Landscape
             break;
       }
    }

    // If we change local selection size by a corner, re-adjust the oposite corner position.
    // It unecessary to do that for Bottom Left corner because it's do by setWidth and setHeight
    // methods.

    switch(m_currentResizing)
    {
       case ResizingTopLeft:    
          m_localRegionSelection.moveBottomRight( oldLocalRegionSelection.bottomRight() );
          break;

       case ResizingTopRight:      
          m_localRegionSelection.moveBottomLeft( oldLocalRegionSelection.bottomLeft() );
          break;

       case ResizingBottomLeft:      
          m_localRegionSelection.moveTopRight( oldLocalRegionSelection.topRight() );
          break;
    }       
       
    // Recalculate the real selection values.
    
    if (updateChange) 
       regionSelectionChanged(false);
    
    if (repaintWidget)
    {
       updatePixmap();
       repaint(false);
    }
}

void ImageSelectionWidget::regionSelectionMoved( bool targetDone )
{
    if (targetDone)
    {
       if (m_localRegionSelection.left() < m_rect.left())    
           m_localRegionSelection.moveLeft(m_rect.left());
       if (m_localRegionSelection.top() < m_rect.top()) 
           m_localRegionSelection.moveTop(m_rect.top());
       if (m_localRegionSelection.right() > m_rect.right())
          m_localRegionSelection.moveRight(m_rect.right());
       if (m_localRegionSelection.bottom() > m_rect.bottom()) 
          m_localRegionSelection.moveBottom(m_rect.bottom());
       
       updatePixmap();
       repaint(false);
    }

    localToRealRegion();
       
    if (targetDone)    
       emit signalSelectionMoved( m_regionSelection );
}

void ImageSelectionWidget::regionSelectionChanged(bool targetDone)
{
    if (targetDone)
    {
       if (m_localRegionSelection.left() < m_rect.left()) 
       {
          m_localRegionSelection.setLeft(m_rect.left());
          applyAspectRatio(false);
       }
       if (m_localRegionSelection.top() < m_rect.top()) 
       {
          m_localRegionSelection.setTop(m_rect.top());
          applyAspectRatio(true);
       }
       if (m_localRegionSelection.right() > m_rect.right())
       {
          m_localRegionSelection.setRight(m_rect.right());
          applyAspectRatio(false);
       }
       if (m_localRegionSelection.bottom() > m_rect.bottom()) 
       {
          m_localRegionSelection.setBottom(m_rect.bottom());
          applyAspectRatio(true);
       }
    }

    localToRealRegion();

    if (targetDone)
       emit signalSelectionChanged( m_regionSelection );   
}

void ImageSelectionWidget::updatePixmap(void)
{
    // Updated draging corners region.

    m_localTopLeftCorner.setRect(m_localRegionSelection.left(),
                                 m_localRegionSelection.top(), 8, 8);
    m_localBottomLeftCorner.setRect(m_localRegionSelection.left(),
                                    m_localRegionSelection.bottom() - 7, 8, 8);
    m_localTopRightCorner.setRect(m_localRegionSelection.right() - 7,
                                  m_localRegionSelection.top(), 8, 8);
    m_localBottomRightCorner.setRect(m_localRegionSelection.right() - 7,
                                     m_localRegionSelection.bottom() - 7, 8, 8);

    // Drawing background and image.

    m_pixmap->fill(colorGroup().background());

    if (m_preview.isNull())
        return;

    // Drawing region outside selection grayed.

    int lx = m_localRegionSelection.left()   - m_rect.left();
    int rx = m_localRegionSelection.right()  - m_rect.left();
    int ty = m_localRegionSelection.top()    - m_rect.top();
    int by = m_localRegionSelection.bottom() - m_rect.top();

    QImage image = m_preview.copyQImage();

    uchar* ptr = image.bits();
    uchar  r, g, b, a;

    for (uint j=0 ; j < m_preview.height() ; j++)
    {
        for (uint i=0 ; i < m_preview.width() ; i++)
        {
            if (i < lx || i >= rx || j < ty || j >= by)
            {
                a = (*ptr >> 24) & 0xFF;
                r = (*ptr >> 16) & 0xFF;
                g = (*ptr >> 8)  & 0xFF;
                b = (*ptr)       & 0xFF;

                r += (uchar)((RCOL8 - r) * OPACITY);
                g += (uchar)((GCOL8 - g) * OPACITY);
                b += (uchar)((BCOL8 - b) * OPACITY);

                *ptr = a << 24 | r << 16 | g << 8 | b;
            }

            ptr+=4;
        }
    }

    QPixmap pix(image);
    bitBlt(m_pixmap, m_rect.x(), m_rect.y(), &pix);
    QPainter p(m_pixmap);

    // Drawing selection borders.

    p.setPen(QPen(QColor(250, 250, 255), 1, Qt::SolidLine));
    p.drawRect(m_localRegionSelection);

    // Drawing selection corners.

    p.drawRect(m_localTopLeftCorner);
    p.drawRect(m_localBottomLeftCorner);
    p.drawRect(m_localTopRightCorner);
    p.drawRect(m_localBottomRightCorner);

    // Drawing guide lines.

    // Constraint drawing only on local selection region.
    // This is needed because arcs and incurved lines can draw
    // outside a little of local selection region.
    p.setClipping(true);
    p.setClipRect(m_localRegionSelection);

    switch (m_guideLinesType)
    {
       case RulesOfThirds:
       {
            int xThird = m_localRegionSelection.width()  / 3;
            int yThird = m_localRegionSelection.height() / 3;

            p.setPen(QPen(Qt::white, m_guideSize, Qt::SolidLine));
            p.drawLine( m_localRegionSelection.left() + xThird,   m_localRegionSelection.top(),
                        m_localRegionSelection.left() + xThird,   m_localRegionSelection.bottom() );
            p.drawLine( m_localRegionSelection.left() + 2*xThird, m_localRegionSelection.top(),
                        m_localRegionSelection.left() + 2*xThird, m_localRegionSelection.bottom() );

            p.drawLine( m_localRegionSelection.left(),  m_localRegionSelection.top() + yThird,
                        m_localRegionSelection.right(), m_localRegionSelection.top() + yThird );
            p.drawLine( m_localRegionSelection.left(),  m_localRegionSelection.top() + 2*yThird,
                        m_localRegionSelection.right(), m_localRegionSelection.top() + 2*yThird );

            p.setPen(QPen(m_guideColor, m_guideSize, Qt::DotLine));
            p.drawLine( m_localRegionSelection.left() + xThird,   m_localRegionSelection.top(),
                        m_localRegionSelection.left() + xThird,   m_localRegionSelection.bottom() );
            p.drawLine( m_localRegionSelection.left() + 2*xThird, m_localRegionSelection.top(),
                        m_localRegionSelection.left() + 2*xThird, m_localRegionSelection.bottom() );

            p.drawLine( m_localRegionSelection.left(),  m_localRegionSelection.top() + yThird,
                        m_localRegionSelection.right(), m_localRegionSelection.top() + yThird );
            p.drawLine( m_localRegionSelection.left(),  m_localRegionSelection.top() + 2*yThird,
                        m_localRegionSelection.right(), m_localRegionSelection.top() + 2*yThird );
            break;
       }

       case HarmoniousTriangles:
       {
            // Move coordinates to local center selection.
            p.translate(m_localRegionSelection.center().x(), m_localRegionSelection.center().y());

            // Flip horizontal.
            if (m_flipHorGoldenGuide)
                p.scale(-1, 1);

            // Flip verical.
            if (m_flipVerGoldenGuide)
                p.scale(1, -1);

            float w = (float)m_localRegionSelection.width();
            float h = (float)m_localRegionSelection.height();
            int   d = (int)((h*cos(atan(w/h)) / (cos(atan(h/w)))));

            p.setPen(QPen(Qt::white, m_guideSize, Qt::SolidLine));
            p.drawLine( -m_localRegionSelection.width()/2, -m_localRegionSelection.height()/2,
                         m_localRegionSelection.width()/2,  m_localRegionSelection.height()/2);

            p.drawLine( -m_localRegionSelection.width()/2 + d, -m_localRegionSelection.height()/2,
                        -m_localRegionSelection.width()/2,      m_localRegionSelection.height()/2);

            p.drawLine( m_localRegionSelection.width()/2,     -m_localRegionSelection.height()/2,
                        m_localRegionSelection.width()/2 - d,  m_localRegionSelection.height()/2);

            p.setPen(QPen(m_guideColor, m_guideSize, Qt::DotLine));
            p.drawLine( -m_localRegionSelection.width()/2, -m_localRegionSelection.height()/2,
                         m_localRegionSelection.width()/2,  m_localRegionSelection.height()/2);

            p.drawLine( -m_localRegionSelection.width()/2 + d, -m_localRegionSelection.height()/2,
                        -m_localRegionSelection.width()/2,      m_localRegionSelection.height()/2);

            p.drawLine( m_localRegionSelection.width()/2,     -m_localRegionSelection.height()/2,
                        m_localRegionSelection.width()/2 - d,  m_localRegionSelection.height()/2);
            break;
       }

       case GoldenMean:
       {
            // Move coordinates to local center selection.
            p.translate(m_localRegionSelection.center().x(), m_localRegionSelection.center().y());

            // Flip horizontal.
            if (m_flipHorGoldenGuide)
                p.scale(-1, 1);

            // Flip verical.
            if (m_flipVerGoldenGuide)
                p.scale(1, -1);

            int w = m_localRegionSelection.width();
            int h = m_localRegionSelection.height();

            QRect R1(-w/2, -h/2, 
                     (int)(w/PHI), h);
            QRect R2((int)(w*(INVPHI - 0.5)), (int)(h*(0.5 - INVPHI)), 
                     (int)(w*(1 - INVPHI)), (int)(h/PHI)); 
            QRect R3((int)(w/2 - R2.width()/PHI), -h/2, 
                     (int)(R2.width()/PHI), h - R2.height());
            QRect R4(R2.x(), R1.y(), R3.x() - R2.x(), 
                     (int)(R3.height()/PHI));
            QRect R5(R4.x(), R4.bottom(), (int)(R4.width()/PHI), 
                     R3.height() - R4.height());
            QRect R6(R5.x() + R5.width(), R5.bottom() - (int)(R5.height()/PHI), 
                     R3.x() - R5.right(), (int)(R5.height()/PHI));
            QRect R7(R6.right() - (int)(R6.width()/PHI), R4.bottom(), 
                     (int)(R6.width()/PHI), R5.height() - R6.height());

            p.setPen(QPen(Qt::white, m_guideSize, Qt::SolidLine));

            // Drawing Golden sections.
            if (m_drawGoldenSection)
            {
               p.drawLine( R1.left(), R2.top(),
                           R2.right(), R2.top());

               p.drawLine( R1.left(), R1.top() + R2.height(),
                           R2.right(), R1.top() + R2.height());

               p.drawLine( R2.right() - R1.width(), R1.top(),
                           R2.right() - R1.width(), R1.bottom() );

               p.drawLine( R1.topRight(), R1.bottomRight() );
            }

            // Drawing Golden triangle guides.
            if (m_drawGoldenTriangle)
            {
               p.drawLine( R1.left(),  R1.bottom(),
                           R2.right(), R1.top() );

               p.drawLine( R1.left(), R1.top(),
                           R2.right() - R1.width(), R1.bottom());

               p.drawLine( R1.left() + R1.width(), R1.top(),
                           R2.right(), R1.bottom() );
            }

            // Drawing Golden spiral sections.
            if (m_drawGoldenSpiralSection)
            {
               p.drawLine( R1.topRight(),   R1.bottomRight() );
               p.drawLine( R2.topLeft(),    R2.topRight() );
               p.drawLine( R3.topLeft(),    R3.bottomLeft() );
               p.drawLine( R4.bottomLeft(), R4.bottomRight() );
               p.drawLine( R5.topRight(),   R5.bottomRight() );
               p.drawLine( R6.topLeft(),    R6.topRight() );
               p.drawLine( R7.topLeft(),    R7.bottomLeft() );
            }

            // Drawing Golden Spiral.
            if (m_drawGoldenSpiral)
            {
               p.drawArc ( R1.left(), 
                           R1.top() - R1.height(),
                           2*R1.width(), 2*R1.height(), 
                           180*16, 90*16);

               p.drawArc ( R2.right() - 2*R2.width(),
                           R1.bottom() - 2*R2.height(),
                           2*R2.width(), 2*R2.height(),
                           270*16, 90*16);

               p.drawArc ( R2.right() - 2*R3.width(),
                           R3.top(),
                           2*R3.width(), 2*R3.height(),
                           0, 90*16);

               p.drawArc ( R4.left(),
                           R4.top(),
                           2*R4.width(), 2*R4.height(),
                           90*16, 90*16);

               p.drawArc ( R5.left(),
                           R5.top()-R5.height(),
                           2*R5.width(), 2*R5.height(),
                           180*16, 90*16);

               p.drawArc ( R6.left()-R6.width(),
                           R6.top()-R6.height(),
                           2*R6.width(), 2*R6.height(),
                           270*16, 90*16);

               p.drawArc ( R7.left()-R7.width(),
                           R7.top(),
                           2*R7.width(), 2*R7.height(),
                           0, 90*16);
            }

            p.setPen(QPen(m_guideColor, m_guideSize, Qt::DotLine));

            // Drawing Golden sections.
            if (m_drawGoldenSection)
            {
               p.drawLine( R1.left(), R2.top(),
                           R2.right(), R2.top());

               p.drawLine( R1.left(), R1.top() + R2.height(),
                           R2.right(), R1.top() + R2.height());

               p.drawLine( R2.right() - R1.width(), R1.top(),
                           R2.right() - R1.width(), R1.bottom() );

               p.drawLine( R1.topRight(), R1.bottomRight() );
            }

            // Drawing Golden triangle guides.
            if (m_drawGoldenTriangle)
            {            
               p.drawLine( R1.left(),  R1.bottom(),
                           R2.right(), R1.top() );
                
               p.drawLine( R1.left(), R1.top(),
                           R2.right() - R1.width(), R1.bottom());

               p.drawLine( R1.left() + R1.width(), R1.top(),
                           R2.right(), R1.bottom() );
            }
                
            // Drawing Golden spiral sections.
            if (m_drawGoldenSpiralSection)
            {            
               p.drawLine( R1.topRight(),   R1.bottomRight() );
               p.drawLine( R2.topLeft(),    R2.topRight() );
               p.drawLine( R3.topLeft(),    R3.bottomLeft() );
               p.drawLine( R4.bottomLeft(), R4.bottomRight() );
               p.drawLine( R5.topRight(),   R5.bottomRight() );
               p.drawLine( R6.topLeft(),    R6.topRight() );
               p.drawLine( R7.topLeft(),    R7.bottomLeft() );
            }
                                        
            // Drawing Golden Spiral.
            if (m_drawGoldenSpiral)
            {
               p.drawArc ( R1.left(), 
                           R1.top() - R1.height(),
                           2*R1.width(), 2*R1.height(), 
                           180*16, 90*16);                       
               
               p.drawArc ( R2.right() - 2*R2.width(),
                           R1.bottom() - 2*R2.height(),
                           2*R2.width(), 2*R2.height(),
                           270*16, 90*16);                       
                
               p.drawArc ( R2.right() - 2*R3.width(),
                           R3.top(),
                           2*R3.width(), 2*R3.height(),
                           0, 90*16);                       
                
               p.drawArc ( R4.left(),
                           R4.top(),
                           2*R4.width(), 2*R4.height(),
                           90*16, 90*16);                       
                
               p.drawArc ( R5.left(),
                           R5.top()-R5.height(),
                           2*R5.width(), 2*R5.height(),
                           180*16, 90*16);                       
                
               p.drawArc ( R6.left()-R6.width(),
                           R6.top()-R6.height(),
                           2*R6.width(), 2*R6.height(),
                           270*16, 90*16);                       
                
               p.drawArc ( R7.left()-R7.width(),
                           R7.top(),
                           2*R7.width(), 2*R7.height(),
                           0, 90*16);                       
            }
                
            break;
       }
    }    
    
    p.setClipping(false);
              
    p.end();
}

void ImageSelectionWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);                   
}

void ImageSelectionWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
       if ( m_localTopLeftCorner.contains( e->x(), e->y() ) )
          m_currentResizing = ResizingTopLeft;
       else if ( m_localBottomRightCorner.contains( e->x(), e->y() ) )
          m_currentResizing = ResizingBottomRight;
       else if ( m_localTopRightCorner.contains( e->x(), e->y() ) )
          m_currentResizing = ResizingTopRight;
       else if ( m_localBottomLeftCorner.contains( e->x(), e->y() ) )
          m_currentResizing = ResizingBottomLeft;
       else if ( m_localRegionSelection.contains( e->x(), e->y() ) )
       {
          m_xpos = e->x();
          m_ypos = e->y();
          setCursor( KCursor::sizeAllCursor() );
       }
    }
}

void ImageSelectionWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_currentResizing != ResizingNone )
    {
       setCursor( KCursor::arrowCursor() );
       regionSelectionChanged(true);
       m_currentResizing = ResizingNone;
    } 
    else if ( m_localRegionSelection.contains( m_xpos, m_ypos ) ) 
    {    
       setCursor( KCursor::arrowCursor() );
       regionSelectionMoved(true);
    }      
}

void ImageSelectionWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
    {
       if ( m_currentResizing == ResizingNone )
       {
          setCursor( KCursor::sizeAllCursor() );
          int newxpos = e->x();
          int newypos = e->y();
               
          m_localRegionSelection.moveBy (newxpos - m_xpos, newypos - m_ypos);

          m_xpos = newxpos;
          m_ypos = newypos;
          
          // Perform normalization of selection area.
          
          if (m_localRegionSelection.left() < m_rect.left()) 
             m_localRegionSelection.moveLeft(m_rect.left());
             
          if (m_localRegionSelection.top() < m_rect.top()) 
             m_localRegionSelection.moveTop(m_rect.top());
             
          if (m_localRegionSelection.right() > m_rect.right())
             m_localRegionSelection.moveRight(m_rect.right());
             
          if (m_localRegionSelection.bottom() > m_rect.bottom()) 
             m_localRegionSelection.moveBottom(m_rect.bottom());
          
          updatePixmap();
          repaint(false);
          regionSelectionMoved(false);
          return;
       }    
       else if ( m_rect.contains(e->x(), e->y()) )
       {
          QPoint pm(e->x(), e->y());
          
          if ( m_currentResizing == ResizingTopLeft &&
               pm.x() < m_localRegionSelection.right() - MINRANGE &&
               pm.y() < m_localRegionSelection.bottom() - MINRANGE )
              m_localRegionSelection.setTopLeft(pm);             
             
          else if ( m_currentResizing == ResizingTopRight  &&
               pm.x() > m_localRegionSelection.left() + MINRANGE &&
               pm.y() < m_localRegionSelection.bottom() - MINRANGE )
             m_localRegionSelection.setTopRight(pm);
          
          else if ( m_currentResizing == ResizingBottomLeft  &&
               pm.x() < m_localRegionSelection.right() - MINRANGE &&
               pm.y() > m_localRegionSelection.top() + MINRANGE )
             m_localRegionSelection.setBottomLeft(pm);
             
          else if ( m_currentResizing == ResizingBottomRight  &&
               pm.x() > m_localRegionSelection.left() + MINRANGE &&
               pm.y() > m_localRegionSelection.top() + MINRANGE )
             m_localRegionSelection.setBottomRight(pm);
          else 
             return;
             
          applyAspectRatio(false, false);
          applyAspectRatio(true);
          
          return;
       }
    }
    else
    {
       if ( m_localTopLeftCorner.contains( e->x(), e->y() ) ||
            m_localBottomRightCorner.contains( e->x(), e->y() ) )
           setCursor( KCursor::sizeFDiagCursor() );
       else if ( m_localTopRightCorner.contains( e->x(), e->y() ) ||
                 m_localBottomLeftCorner.contains( e->x(), e->y() ) )
           setCursor( KCursor::sizeBDiagCursor() );
       else if ( m_localRegionSelection.contains( e->x(), e->y() ) )
           setCursor( KCursor::handCursor() );
       else
           setCursor( KCursor::arrowCursor() );
    }
}

}  // NameSpace Digikam

#include "imageselectionwidget.moc"

