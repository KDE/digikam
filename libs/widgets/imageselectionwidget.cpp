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

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qregion.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qbrush.h> 
#include <qpixmap.h>
#include <qpen.h>

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

    m_iface  = new ImageIface(w,h);

    m_data   = m_iface->getPreviewData();
    m_w      = m_iface->previewWidth();
    m_h      = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(m_w, m_h);
    setMouseTracking(true);
    
    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
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
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionY(int y)
{
    m_regionSelection.setY(y);
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionWidth(int w)
{
    m_regionSelection.setWidth(w);
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionHeight(int h)
{
    m_regionSelection.setHeight(h);
    applyAspectRatio(true);
}

void ImageSelectionWidget::applyAspectRatio(bool WOrH)
{
    if ( !WOrH )  // Width changed.
       {    
       int w = m_regionSelection.width();
       
       switch(m_currentAspectRatio)
          {
          case RATIO01X01:  
                m_regionSelection.setHeight(w); 
             break;
          
          case RATIO03X04:  
             if ( m_currentOrientation )  
                m_regionSelection.setHeight((int)(w * 1.3333333333333));   // Landscape
             else                       
                m_regionSelection.setHeight((int)(w * 0.75));              // Portrait
             break;
       
          case RATIO02x03:           
             if ( m_currentOrientation )  
                m_regionSelection.setHeight((int)(w * 1.5));               // Landscape
             else                       
                m_regionSelection.setHeight((int)(w * 0.66666666666667));  // Portrait
             break;

          case RATIO05x07:          
             if ( m_currentOrientation )  
                m_regionSelection.setHeight((int)(w * 1.4));               // Landscape
             else                       
                m_regionSelection.setHeight((int)(w * 0.71428571428571));  // Portrait
             break;          
       
          case RATIO07x10:          
             if ( m_currentOrientation )  
                m_regionSelection.setHeight((int)(w * 1.4285714285714));   // Landscape
             else                       
                m_regionSelection.setHeight((int)(w * 0.7));               // Portrait
             break;                            

          case RATIO04X05:          
             if ( m_currentOrientation )  
                m_regionSelection.setHeight((int)(w * 1.25));              // Landscape
             else                       
                m_regionSelection.setHeight((int)(w * 0.8));               // Portrait
             break;                                         
          }
       }  
    else      // Height changed.
       {
       int h = m_regionSelection.height();
       
       switch(m_currentAspectRatio)
          {
          case RATIO01X01:      
             m_regionSelection.setWidth(h);
             break;
       
          case RATIO03X04:      
             if ( m_currentOrientation )  
                m_regionSelection.setWidth((int)(h * 0.75));             // Landscape
             else                       
                m_regionSelection.setWidth((int)(h * 1.3333333333333));  // Portrait
             break;
       
          case RATIO02x03:    
             if ( m_currentOrientation )  
                m_regionSelection.setWidth((int)(h * 0.66666666666667)); // Landscape
             else                       
                m_regionSelection.setWidth((int)(h * 1.5));              // Portrait
             break;
       
          case RATIO05x07:          
             if ( m_currentOrientation )  
                m_regionSelection.setWidth((int)(h * 0.71428571428571)); // Landscape
             else                       
                m_regionSelection.setWidth((int)(h * 1.4));              // Portrait
             break; 

          case RATIO07x10:          
             if ( m_currentOrientation )  
                m_regionSelection.setWidth((int)(h * 0.7));              // Landscape
             else                       
                m_regionSelection.setWidth((int)(h * 1.4285714285714));  // Portrait
             break;           
       
          case RATIO04X05:          
             if ( m_currentOrientation )  
                m_regionSelection.setWidth((int)(h * 0.8));              // Landscape
             else                       
                m_regionSelection.setWidth((int)(h * 1.25));             // Portrait
             break;        
          }
       }
       
    m_localRegionSelection.setX( 1 + m_rect.x() + (int)((float)m_regionSelection.x() * 
                                              ( (float)m_w / (float)m_iface->originalWidth() )) );
                                            
    m_localRegionSelection.setY( 1 + m_rect.y() + (int)((float)m_regionSelection.y() * 
                                              ( (float)m_h / (float)m_iface->originalHeight() )) );
                                            
    m_localRegionSelection.setWidth( (int)((float)m_regionSelection.width() *
                                          ( (float)m_w / (float)m_iface->originalWidth() )) );
                                     
    m_localRegionSelection.setHeight( (int)((float)m_regionSelection.height() *
                                           ( (float)m_h / (float)m_iface->originalHeight() )) );
    
    repaint(false);
    emit signalSelectionChanged( m_regionSelection );       
}

QRect ImageSelectionWidget::getRegionSelection(void)
{
    return (m_regionSelection);
}

void ImageSelectionWidget::setCenterSelection(void)
{
    m_regionSelection.moveBy(
      (int)((m_iface->originalWidth() / 2.0)  - (m_regionSelection.width() / 2.0)), 
      (int)((m_iface->originalHeight() / 2.0) - (m_regionSelection.height() / 2.0)));
    applyAspectRatio(false);
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
                     
    m_regionSelection.setX(x);
    m_regionSelection.setY(y);
    m_regionSelection.setWidth(w);                                     
    m_regionSelection.setHeight(h);
       
    emit signalSelectionMoved( m_regionSelection, targetDone );
}

void ImageSelectionWidget::paintEvent( QPaintEvent * )
{
    // Drawing background and image.
    m_pixmap->fill(colorGroup().background());
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());
    
    QPainter p(m_pixmap);
    
    // Drawing region outside selection grayed.
    QRegion sel(m_localRegionSelection);    
    QRegion img(m_rect);
    p.setRasterOp(Qt::AndROP);
    p.setClipRegion( img.eor(sel) );
    p.fillRect(m_rect ,QBrush::QBrush(Qt::Dense5Pattern));
    p.setRasterOp(Qt::CopyROP);
    p.setClipping(false);
    
    // Drawing selection border
    p.setPen(QPen(Qt::red, 2, Qt::SolidLine));
    p.drawRect(m_localRegionSelection);
    
    p.end();

    bitBlt(this, 0, 0, m_pixmap);                   
}

void ImageSelectionWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_localRegionSelection.contains( e->x(), e->y() ) )
       {
       m_xpos = e->x();
       m_ypos = e->y();
       
       setCursor ( KCursor::sizeAllCursor() );
       }
}

void ImageSelectionWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_localRegionSelection.contains( m_xpos, m_ypos ) ) 
       {    
       setCursor ( KCursor::arrowCursor() );
       regionSelectionMoved(true);
       }
}

void ImageSelectionWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
       {
       int newxpos = e->x();
       int newypos = e->y();
       
       m_localRegionSelection.moveBy (newxpos - m_xpos, newypos - m_ypos);

       repaint(false);
     
       m_xpos = newxpos;
       m_ypos = newypos;
       regionSelectionMoved(false);
       return;
       }        
    else 
       {
       if ( m_localRegionSelection.contains( e->x(), e->y() ) )
           setCursor( KCursor::handCursor() );
       else
           setCursor ( KCursor::arrowCursor() );
       }
}

}  // NameSpace Digikam

#include "imageselectionwidget.moc"

