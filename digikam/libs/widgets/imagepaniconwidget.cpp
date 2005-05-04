/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-22
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

#include "imagepaniconwidget.h"

namespace Digikam
{

ImagePanIconWidget::ImagePanIconWidget(int w, int h, QWidget *parent)
                  : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_moveSelection = false;
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

ImagePanIconWidget::~ImagePanIconWidget()
{
    delete [] m_data;
    delete m_iface;
    delete m_pixmap;
}

void ImagePanIconWidget::setRegionSelection(QRect regionSelection)
{
    m_regionSelection = regionSelection;
    m_localRegionSelection.setX( 1 + m_rect.x() + (int)((float)m_regionSelection.x() * 
                                              ( (float)m_w / (float)m_iface->originalWidth() )) );
                                            
    m_localRegionSelection.setY( 1 + m_rect.y() + (int)((float)m_regionSelection.y() * 
                                              ( (float)m_h / (float)m_iface->originalHeight() )) );
                                            
    m_localRegionSelection.setWidth( (int)((float)m_regionSelection.width() *
                                          ( (float)m_w / (float)m_iface->originalWidth() )) );
                                     
    m_localRegionSelection.setHeight( (int)((float)m_regionSelection.height() *
                                           ( (float)m_h / (float)m_iface->originalHeight() )) );
    
    updatePixmap();
    repaint(false);
}

QRect ImagePanIconWidget::getRegionSelection(void)
{
    return (m_regionSelection);
}

void ImagePanIconWidget::setCenterSelection(void)
{
    setRegionSelection(QRect::QRect( 
             (int)(((float)m_iface->originalWidth() / 2.0)  - ((float)m_regionSelection.width() / 2.0)),
             (int)(((float)m_iface->originalHeight() / 2.0) - ((float)m_regionSelection.height() / 2.0)),
             m_regionSelection.width(),
             m_regionSelection.height()));
}

void ImagePanIconWidget::regionSelectionMoved( bool targetDone )
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
                     
    m_regionSelection.setX(x);
    m_regionSelection.setY(y);
    m_regionSelection.setWidth(w);                                     
    m_regionSelection.setHeight(h);
       
    emit signalSelectionMoved( m_regionSelection, targetDone );
}

void ImagePanIconWidget::updatePixmap( void )
{
    // Drawing background and image.
    m_pixmap->fill(colorGroup().background());
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());
    
    QPainter p(m_pixmap);
    
    // Drawing selection border
    p.setPen(QPen(Qt::red, 2, Qt::SolidLine));
    p.drawRect(m_localRegionSelection);
    
    p.end();
}

void ImagePanIconWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);                   
}

void ImagePanIconWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_localRegionSelection.contains( e->x(), e->y() ) )
       {
       m_xpos = e->x();
       m_ypos = e->y();
       m_moveSelection = true;
       setCursor ( KCursor::sizeAllCursor() );
       }
}

void ImagePanIconWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_moveSelection && m_localRegionSelection.contains( m_xpos, m_ypos ) ) 
       {    
       setCursor ( KCursor::arrowCursor() );
       regionSelectionMoved(true);
       m_moveSelection = false;
       }
}

void ImagePanIconWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( m_moveSelection && e->state() == Qt::LeftButton )
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
       if ( m_localRegionSelection.contains( e->x(), e->y() ) )
           setCursor( KCursor::handCursor() );
       else
           setCursor( KCursor::arrowCursor() );
       }
}

}  // NameSpace Digikam

#include "imagepaniconwidget.moc"

