/* ============================================================
 * File  : imagepaniconwidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-22
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(w,h);
    setMouseTracking(true);
    
    m_iface = new ImageIface(w,h);
    
    m_data = m_iface->getPreviewData();
    m_w    = m_iface->previewWidth();
    m_h    = m_iface->previewHeight();

    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
}

ImagePanIconWidget::~ImagePanIconWidget()
{
    delete [] m_data;
    delete m_iface;
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
    
    repaint(false);
}

void ImagePanIconWidget::regionSelectionChanged( bool targetDone )
{
    m_regionSelection.setX( (int)( ((float)m_localRegionSelection.x() - (float)m_rect.x() ) * 
                            ( (float)m_iface->originalWidth() / (float)m_w )) );
                                            
    m_regionSelection.setY( (int)( ((float)m_localRegionSelection.y() - (float)m_rect.y() ) *
                            ( (float)m_iface->originalHeight() / (float)m_h )) );
                                            
    m_regionSelection.setWidth( (int)((float)m_localRegionSelection.width() *
                                ( (float)m_iface->originalWidth() / (float)m_w )) );
                                     
    m_regionSelection.setHeight( (int)((float)m_localRegionSelection.height() *
                                 ( (float)m_iface->originalHeight() / (float)m_h )) );
    
    emit signalSelectionMoved( m_regionSelection, targetDone );
}

void ImagePanIconWidget::paintEvent( QPaintEvent * )
{
    m_iface->paint(this, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());

    QRect r(0, 0, width(), height());
    QRegion reg(r);
    reg -= m_rect;

    QPainter p(this);
    p.setPen(QPen::QPen(Qt::red, 2, Qt::SolidLine));
    p.drawRect(m_localRegionSelection);
    p.setClipRegion(reg);
    p.fillRect(r, colorGroup().background());
    p.end();
}

void ImagePanIconWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
       m_localRegionSelection.contains( e->x(), e->y() ) )
       {
       m_xpos = e->x();
       m_ypos = e->y();

       setCursor ( KCursor::sizeAllCursor() );
       }
}

void ImagePanIconWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_localRegionSelection.contains( m_xpos, m_ypos ) ) 
       {    
       setCursor ( KCursor::arrowCursor() );
       regionSelectionChanged(true);
       }
}

void ImagePanIconWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
       {
       uint newxpos = e->x();
       uint newypos = e->y();
       
       m_localRegionSelection.moveBy (newxpos - m_xpos, newypos - m_ypos);
       repaint(false);
     
       m_xpos = newxpos;
       m_ypos = newypos;
       regionSelectionChanged(false);
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

#include "imagepaniconwidget.moc"

