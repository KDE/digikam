/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-09
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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
 
#define OPACITY  0.7
#define RCOL     0xAA
#define GCOL     0xAA
#define BCOL     0xAA

#define MINRANGE 50
 
// C++ includes.

#include <cstdio>

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

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <imageiface.h>

// Local includes.

#include "imageselectionwidget.h"

namespace Digikam
{

ImageSelectionWidget::ImageSelectionWidget(int w, int h, QWidget *parent, 
                      float aspectRatioValue, int aspectRatioType, int orient,
                      bool ruleThirdLines)
                    : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_currentAspectRatioType  = aspectRatioType; 
    m_currentAspectRatioValue = aspectRatioValue;
    m_currentOrientation      = orient; 
    m_currentResizing         = ResizingNone;
    m_ruleThirdLines          = ruleThirdLines;
    m_timerW = 0;
    m_timerH = 0;

    m_iface  = new ImageIface(w,h);

    m_data   = m_iface->getPreviewData();
    m_w      = m_iface->previewWidth();
    m_h      = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(m_w, m_h);
    setMouseTracking(true);
    
    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
    updatePixmap();
}

ImageSelectionWidget::~ImageSelectionWidget()
{
    if (m_timerW)
       delete m_timerW;

    if (m_timerH)
       delete m_timerH;
           
    delete [] m_data;
    delete m_iface;
    delete m_pixmap;
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
                   ( (float)m_iface->originalWidth() / (float)m_w )) );
}
                                            
int ImageSelectionWidget::getMinHeightRange(void)
{    
    return( (int)( ((float)MINRANGE - (float)m_rect.y() ) *
                   ( (float)m_iface->originalHeight() / (float)m_h )) );    
}

void ImageSelectionWidget::resetSelection(void)
{
    m_regionSelection.setX(0);
    m_regionSelection.setY(0);
    m_regionSelection.setWidth((int)(m_iface->originalWidth()/2.0));
    m_regionSelection.setHeight((int)(m_iface->originalHeight()/2.0));
    realToLocalRegion();
    applyAspectRatio(false, false);
    setCenterSelection();
}

void ImageSelectionWidget::maxAspectSelection(void)
{
    m_localRegionSelection.setX(0);
    m_localRegionSelection.setY(0);
    
    if ( !m_currentOrientation )   // Landscape
       {
       m_localRegionSelection.setWidth(m_w);
       applyAspectRatio(false, false);
       
       if ( m_localRegionSelection.height() > m_h )
          {
          m_localRegionSelection.setHeight(m_h);
          applyAspectRatio(true, false);
          }
       }
    else                          // Portrait
       {
       m_localRegionSelection.setHeight(m_h);
       applyAspectRatio(true, false);
       
       if ( m_localRegionSelection.width() > m_w )
          {
          m_localRegionSelection.setWidth(m_w);
          applyAspectRatio(false, false);
          }
       }

    updatePixmap();
    repaint(false);
}

void ImageSelectionWidget::slotRuleThirdLines(bool ruleThirdLines)
{
    m_ruleThirdLines = ruleThirdLines;
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

void ImageSelectionWidget::setCenterSelection(void)
{
    m_localRegionSelection.moveBy(
      (int)((m_w / 2.0)  - (m_localRegionSelection.width() / 2.0)), 
      (int)((m_h / 2.0) -  (m_localRegionSelection.height() / 2.0)));
    applyAspectRatio(false, true, false);
    regionSelectionChanged(true);
}

void ImageSelectionWidget::realToLocalRegion(bool updateSizeOnly)
{
    if (!updateSizeOnly)
       {
       if (m_regionSelection.x() == 0 ) m_localRegionSelection.setX(0);
       else
          m_localRegionSelection.setX( 1 + m_rect.x() + (int)((float)m_regionSelection.x() * 
                                      ( (float)m_w / (float)m_iface->originalWidth() )) );
                                            
       if (m_regionSelection.y() == 0 ) m_localRegionSelection.setY(0);
       else
          m_localRegionSelection.setY( 1 + m_rect.y() + (int)((float)m_regionSelection.y() * 
                                      ( (float)m_h / (float)m_iface->originalHeight() )) );
       }
                                            
    m_localRegionSelection.setWidth( (int)((float)m_regionSelection.width() *
                                          ( (float)m_w / (float)m_iface->originalWidth() )) );
                                     
    m_localRegionSelection.setHeight( (int)((float)m_regionSelection.height() *
                                           ( (float)m_h / (float)m_iface->originalHeight() )) );    
}

void ImageSelectionWidget::localToRealRegion(void)
{
    int x = (int)( ((float)m_localRegionSelection.x() - (float)m_rect.x() ) * 
                   ( (float)m_iface->originalWidth() / (float)m_w ));
                                            
    int y = (int)( ((float)m_localRegionSelection.y() - (float)m_rect.y() ) *
                   ( (float)m_iface->originalHeight() / (float)m_h ));
                                            
    int w = (int)((float)m_localRegionSelection.width() *
                 ( (float)m_iface->originalWidth() / (float)m_w ));
                                     
    int h = (int)((float)m_localRegionSelection.height() *
                 ( (float)m_iface->originalHeight() / (float)m_h ));
                     
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
       if (m_localRegionSelection.left() < 0) m_localRegionSelection.moveLeft(0);
       if (m_localRegionSelection.top() < 0) m_localRegionSelection.moveTop(0);
       if (m_localRegionSelection.right() > m_rect.width())
          m_localRegionSelection.moveRight(m_rect.width());
       if (m_localRegionSelection.bottom() > m_rect.height()) 
          m_localRegionSelection.moveBottom(m_rect.height());
       
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
       if (m_localRegionSelection.left() < 0) 
          {
          m_localRegionSelection.setLeft(0);
          applyAspectRatio(false);
          }
       if (m_localRegionSelection.top() < 0) 
          {
          m_localRegionSelection.setTop(0);
          applyAspectRatio(true);
          }
       if (m_localRegionSelection.right() > m_rect.width())
          {
          m_localRegionSelection.setRight(m_rect.width());
          applyAspectRatio(false);
          }
       if (m_localRegionSelection.bottom() > m_rect.height()) 
          {
          m_localRegionSelection.setBottom(m_rect.height());
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

    if (!m_data)
        return;
    
    // Drawing region outside selection grayed.
    
    QImage image((uchar*)m_data, m_w, m_h, 32, 0, 0, QImage::IgnoreEndian);
    image = image.copy();

    uint* ptr = (uint*)image.bits();
    uchar r, g, b, a;

    int lx = m_localRegionSelection.x();
    int rx = m_localRegionSelection.x() + m_localRegionSelection.width();
    int ty = m_localRegionSelection.y();
    int by = m_localRegionSelection.y() + m_localRegionSelection.height();
    
    for (int j=0 ; j<m_h ; ++j)
    {
        for (int i=0 ; i<m_w ; ++i)
        {
            if (i < lx || i >= rx || j < ty || j >= by)
            {
                a = (*ptr >> 24) & 0xff;
                r = (*ptr >> 16) & 0xff;
                g = (*ptr >> 8)  & 0xff;
                b = (*ptr)       & 0xff;
                
                r += (uchar)((RCOL - r) * OPACITY);
                g += (uchar)((GCOL - g) * OPACITY);
                b += (uchar)((BCOL - b) * OPACITY);
                
                *ptr = a << 24 | r << 16 | g << 8 | b;
            }
                
            ptr++;
        }
    }

    QPixmap pix(image);
    bitBlt(m_pixmap, m_rect.x(), m_rect.y(), &pix);
    QPainter p(m_pixmap);
    
    // Drawing 'rule of thirds' lines.
    
    if (m_ruleThirdLines)
       {
       p.setPen(QPen(QColor(250, 250, 255), 0, Qt::DotLine));
    
       int xThird = m_localRegionSelection.width() / 3;
       int yThird = m_localRegionSelection.height() / 3;
    
       p.drawLine( m_localRegionSelection.left() + xThird,   m_localRegionSelection.top(),
                   m_localRegionSelection.left() + xThird,   m_localRegionSelection.bottom() );
       p.drawLine( m_localRegionSelection.left() + 2*xThird, m_localRegionSelection.top(),
                   m_localRegionSelection.left() + 2*xThird, m_localRegionSelection.bottom() );
    
       p.drawLine( m_localRegionSelection.left(),  m_localRegionSelection.top() + yThird,
                   m_localRegionSelection.right(), m_localRegionSelection.top() + yThird );
       p.drawLine( m_localRegionSelection.left(),  m_localRegionSelection.top() + 2*yThird,
                   m_localRegionSelection.right(), m_localRegionSelection.top() + 2*yThird );
       }
           
    // Drawing selection borders.
    
    p.setPen(QPen(QColor(250, 250, 255), 1, Qt::SolidLine));
    p.drawRect(m_localRegionSelection);
    
    // Drawing selection corners.
    
    p.drawRect(m_localTopLeftCorner);
    p.drawRect(m_localBottomLeftCorner);
    p.drawRect(m_localTopRightCorner);
    p.drawRect(m_localBottomRightCorner);

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
          setCursor ( KCursor::sizeAllCursor() );
          }
       }
}

void ImageSelectionWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_currentResizing != ResizingNone )
       {
       setCursor ( KCursor::arrowCursor() );
       regionSelectionChanged(true);
       m_currentResizing = ResizingNone;
       } 
    else if ( m_localRegionSelection.contains( m_xpos, m_ypos ) ) 
       {    
       setCursor ( KCursor::arrowCursor() );
       regionSelectionMoved(true);
       }      
}

void ImageSelectionWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton && 
         m_rect.contains(e->x(), e->y()) )
       {
       if ( m_currentResizing == ResizingNone )
          {
          setCursor ( KCursor::sizeAllCursor() );
          int newxpos = e->x();
          int newypos = e->y();
               
          m_localRegionSelection.moveBy (newxpos - m_xpos, newypos - m_ypos);

          updatePixmap();
          repaint(false);
     
          m_xpos = newxpos;
          m_ypos = newypos;
          regionSelectionMoved(false);
          return;
          }    
       else
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
           setCursor ( KCursor::arrowCursor() );
       }
}

}  // NameSpace Digikam

#include "imageselectionwidget.moc"

