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

#define opacity 0.7
#define rcol    255
#define gcol    255
#define bcol    255
 
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

ImageSelectionWidget::ImageSelectionWidget(int w, int h, QWidget *parent)
                    : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_currentAspectRatio = RATIO01X01;
    m_currentOrientation = Landscape;
    m_currentResizing    = ResizingNone;

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

void ImageSelectionWidget::setSelectionOrientation(int orient)
{
    m_currentOrientation = orient;
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionAspectRatio(int aspectRatio)
{
    m_currentAspectRatio = aspectRatio;
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionX(int x)
{
    m_regionSelection.setX(x);
    realToLocalRegion();
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionY(int y)
{
    m_regionSelection.setY(y);
    realToLocalRegion();
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionWidth(int w)
{
    m_regionSelection.setWidth(w);
    realToLocalRegion();
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionHeight(int h)
{
    m_regionSelection.setHeight(h);
    realToLocalRegion();
    applyAspectRatio(true);
}

void ImageSelectionWidget::setCenterSelection(void)
{
    m_localRegionSelection.moveBy(
      (int)((m_w / 2.0)  - (m_localRegionSelection.width() / 2.0)), 
      (int)((m_h / 2.0) -  (m_localRegionSelection.height() / 2.0)));
    applyAspectRatio(false);
}

void ImageSelectionWidget::realToLocalRegion(void)
{
    m_localRegionSelection.setX( 1 + m_rect.x() + (int)((float)m_regionSelection.x() * 
                                              ( (float)m_w / (float)m_iface->originalWidth() )) );
                                            
    m_localRegionSelection.setY( 1 + m_rect.y() + (int)((float)m_regionSelection.y() * 
                                              ( (float)m_h / (float)m_iface->originalHeight() )) );
                                            
    m_localRegionSelection.setWidth( (int)((float)m_regionSelection.width() *
                                          ( (float)m_w / (float)m_iface->originalWidth() )) );
                                     
    m_localRegionSelection.setHeight( (int)((float)m_regionSelection.height() *
                                           ( (float)m_h / (float)m_iface->originalHeight() )) );    
}

void ImageSelectionWidget::applyAspectRatio(bool WOrH, bool repaintWidget)
{
    // Save local selection area for re-adjustment after changing width and height.
    QRect oldLocalRegionSelection = m_localRegionSelection;

    if ( !WOrH )  // Width changed.
       {    
       int w = m_localRegionSelection.width();
       
       switch(m_currentAspectRatio)
          {
          case RATIO01X01:  
                m_localRegionSelection.setHeight(w); 
             break;
          
          case RATIO03X04:  
             if ( m_currentOrientation )  
                m_localRegionSelection.setHeight((int)(w * 1.3333333333333));   // Landscape
             else                       
                m_localRegionSelection.setHeight((int)(w * 0.75));              // Portrait
             break;
       
          case RATIO02x03:           
             if ( m_currentOrientation )  
                m_localRegionSelection.setHeight((int)(w * 1.5));               // Landscape
             else                       
                m_localRegionSelection.setHeight((int)(w * 0.66666666666667));  // Portrait
             break;

          case RATIO05x07:          
             if ( m_currentOrientation )  
                m_localRegionSelection.setHeight((int)(w * 1.4));               // Landscape
             else                       
                m_localRegionSelection.setHeight((int)(w * 0.71428571428571));  // Portrait
             break;          
       
          case RATIO07x10:          
             if ( m_currentOrientation )  
                m_localRegionSelection.setHeight((int)(w * 1.4285714285714));   // Landscape
             else                       
                m_localRegionSelection.setHeight((int)(w * 0.7));               // Portrait
             break;                            

          case RATIO04X05:          
             if ( m_currentOrientation )  
                m_localRegionSelection.setHeight((int)(w * 1.25));              // Landscape
             else                       
                m_localRegionSelection.setHeight((int)(w * 0.8));               // Portrait
             break;                                         
          }
       }  
    else      // Height changed.
       {
       int h = m_localRegionSelection.height();
       
       switch(m_currentAspectRatio)
          {
          case RATIO01X01:      
             m_localRegionSelection.setWidth(h);
             break;
       
          case RATIO03X04:      
             if ( m_currentOrientation )  
                m_localRegionSelection.setWidth((int)(h * 0.75));             // Portrait
             else                       
                m_localRegionSelection.setWidth((int)(h * 1.3333333333333));  // Landscape
             break;
       
          case RATIO02x03:    
             if ( m_currentOrientation )  
                m_localRegionSelection.setWidth((int)(h * 0.66666666666667)); // Portrait
             else                       
                m_localRegionSelection.setWidth((int)(h * 1.5));              // Landscape
             break;
       
          case RATIO05x07:          
             if ( m_currentOrientation )  
                m_localRegionSelection.setWidth((int)(h * 0.71428571428571)); // Portrait
             else                       
                m_localRegionSelection.setWidth((int)(h * 1.4));              // Landscape
             break; 

          case RATIO07x10:          
             if ( m_currentOrientation )  
                m_localRegionSelection.setWidth((int)(h * 0.7));              // Portrait
             else                       
                m_localRegionSelection.setWidth((int)(h * 1.4285714285714));  // Landscape
             break;           
       
          case RATIO04X05:          
             if ( m_currentOrientation )  
                m_localRegionSelection.setWidth((int)(h * 0.8));              // Portrait
             else                       
                m_localRegionSelection.setWidth((int)(h * 1.25));             // Landscape
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
    regionSelectionChanged();
    
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
    
    int x = (int)( ((float)m_localRegionSelection.x() - (float)m_rect.x() ) * 
                   ( (float)m_iface->originalWidth() / (float)m_w ));
                                            
    int y = (int)( ((float)m_localRegionSelection.y() - (float)m_rect.y() ) *
                   ( (float)m_iface->originalHeight() / (float)m_h ));
                                            
    int w = (int)((float)m_localRegionSelection.width() *
                 ( (float)m_iface->originalWidth() / (float)m_w ));
                                     
    int h = (int)((float)m_localRegionSelection.height() *
                 ( (float)m_iface->originalHeight() / (float)m_h ));
                     
    m_regionSelection.setRect(x, y, w, h);
       
    emit signalSelectionMoved( m_regionSelection, targetDone );
}

void ImageSelectionWidget::regionSelectionChanged(void)
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
    
    emit signalSelectionChanged( m_regionSelection );   
}

void ImageSelectionWidget::updatePixmap(void)
{
    m_localTopLeftCorner.setRect(m_localRegionSelection.left(), 
                                 m_localRegionSelection.top(), 10, 10);
    m_localBottomLeftCorner.setRect(m_localRegionSelection.left(), 
                                    m_localRegionSelection.bottom() - 9, 10, 10);   
    m_localTopRightCorner.setRect(m_localRegionSelection.right() - 9, 
                                  m_localRegionSelection.top(), 10, 10);
    m_localBottomRightCorner.setRect(m_localRegionSelection.right() - 9, 
                                     m_localRegionSelection.bottom() - 9, 10, 10);
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
    
    for (int j=0; j<m_h; j++)
    {
        for (int i=0; i<m_w; i++)
        {
            if (i < lx || i >= rx || j < ty || j >= by)
            {
                a = (*ptr >> 24) & 0xff;
                r = (*ptr >> 16) & 0xff;
                g = (*ptr >> 8)  & 0xff;
                b = (*ptr)       & 0xff;
                
                r += (uchar)((rcol - r) * opacity);
                g += (uchar)((gcol - g) * opacity);
                b += (uchar)((bcol - b) * opacity);
                
                *ptr = a << 24 | r << 16 | g << 8 | b;
            }
                
            ptr++;
        }
    }

    QPixmap pix(image);
    bitBlt(m_pixmap, m_rect.x(), m_rect.y(), &pix);

    // Drawing selection borders.
    QPainter p(m_pixmap);
    p.setPen(QPen(Qt::red, 2, Qt::SolidLine));
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
       regionSelectionChanged();
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
    if ( e->state() == Qt::LeftButton )
       {
       if ( m_currentResizing == ResizingNone )
          {
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
          if ( m_currentResizing == ResizingTopLeft )
             m_localRegionSelection.setTopLeft(QPoint::QPoint(e->x(), e->y()));             
             
          else if ( m_currentResizing == ResizingTopRight )
             m_localRegionSelection.setTopRight(QPoint::QPoint(e->x(), e->y()));
          
          else if ( m_currentResizing == ResizingBottomLeft )
             m_localRegionSelection.setBottomLeft(QPoint::QPoint(e->x(), e->y()));
             
          else if ( m_currentResizing == ResizingBottomRight )
             m_localRegionSelection.setBottomRight(QPoint::QPoint(e->x(), e->y()));
          
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

