/* ============================================================
 * File  : superimposewidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-04
 * Description : 
 * 
 * Copyright 2005 Gilles Caulier
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

#include <qpainter.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "superimposewidget.h"

namespace DigikamSuperImposeImagesPlugin
{

SuperImposeWidget::SuperImposeWidget(int w, int h, QWidget *parent)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_pixmap     = new QPixmap(w, h);
    m_zoomFactor = 100;
    m_editMode   = MOVE;
    
    m_iface  = new Digikam::ImageIface(0, 0);
    m_data   = m_iface->getOriginalData();
    m_w      = m_iface->originalWidth();
    m_h      = m_iface->originalHeight();
    
    m_img.create( m_w, m_h, 32 );
    m_img.setAlphaBuffer(true);
    memcpy(m_img.bits(), m_data, m_img.numBytes());
    
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(w, h);
    setMouseTracking(true);

    m_rect = QRect(0, 0, w, h);
    m_currentSelection = QRect(m_w/2 - w/2, m_h/2 - h/2, w, h);
}

SuperImposeWidget::~SuperImposeWidget()
{
    delete [] m_data;
    delete m_iface;
    
    if(m_pixmap) 
       delete m_pixmap;    
}

QSize SuperImposeWidget::getTemplateSize(void)
{
    return m_template.size();
}

void SuperImposeWidget::makeSuperImpose(QPixmap *target, int w, int h)
{
    makePixmap(target, w, h);
}

void SuperImposeWidget::makePixmap(QPixmap *pixmap, int w, int h)
{
    pixmap->fill(colorGroup().background());
    
    QPainter p(pixmap);
    QPixmap pix(m_img.copy(m_currentSelection.x(), m_currentSelection.y(),
                           m_currentSelection.width(), m_currentSelection.height())
                .scale(w, h));
    p.drawPixmap(0, 0, pix, 0, 0, w, h);
    
    QPixmap tem(m_template.scale(w, h));
    
    p.drawPixmap(0, 0, tem, 0, 0, w, h);
    p.end();
}

void SuperImposeWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap, 0, 0, width(), height());
}

void SuperImposeWidget::slotEditModeChanged(int mode)
{
    m_editMode = mode;
}

void SuperImposeWidget::slotSetCurrentTemplate(const KURL& url)
{
    m_template.load(url.path());
    makePixmap(m_pixmap, width(), height());
    repaint(true);
}

void SuperImposeWidget::setZoomSelection(int deltaZoomFactor)
{
    m_zoomFactor = m_zoomFactor + deltaZoomFactor;
    int wf = (int)((float)width()  * (100-(float)m_zoomFactor) / 100);
    int hf = (int)((float)height() * (100-(float)m_zoomFactor) / 100);
    
    if (deltaZoomFactor > 0)  // Zoom in.
       {
       m_currentSelection.setLeft(m_currentSelection.left() + (wf /2));
       m_currentSelection.setTop(m_currentSelection.top() + (hf /2));
       m_currentSelection.setWidth(m_currentSelection.width() - wf);
       m_currentSelection.setHeight(m_currentSelection.height() - hf);
       }
    else                      // Zoom out.
       {
       m_currentSelection.setLeft(m_currentSelection.left() - (wf /2));
       m_currentSelection.setTop(m_currentSelection.top() - (hf /2));
       m_currentSelection.setWidth(m_currentSelection.width() + wf);
       m_currentSelection.setHeight(m_currentSelection.height() + hf);
       }
        
    makePixmap(m_pixmap, width(), height());
    repaint(false);
}

void SuperImposeWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_rect.contains( e->x(), e->y() ) )
       {
       switch (m_editMode)
           {
           case ZOOMIN:
              if (m_zoomFactor < 100)
                 {
                 setZoomSelection(+5);
                 }
              break;
              
           case ZOOMOUT:
              if (m_zoomFactor > 1)
                 {
                 setZoomSelection(-5);
                 }
              break;
           
           case MOVE:
              m_xpos = e->x();
              m_ypos = e->y();
              setCursor ( KCursor::sizeAllCursor() );
           }
       }
}

void SuperImposeWidget::mouseReleaseEvent ( QMouseEvent * )
{
    setCursor ( KCursor::arrowCursor() );
}

void SuperImposeWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( m_rect.contains( e->x(), e->y() ) )
       {
       if ( e->state() == Qt::LeftButton )
          {
          switch (m_editMode)
           {
           case ZOOMIN:
              break;
              
           case ZOOMOUT:
              break;
           
           case MOVE:
              uint newxpos = e->x();
              uint newypos = e->y();
              float wf = (float)m_currentSelection.width() / (float)width();
              float hf = (float)m_currentSelection.height() / (float)height();

              kdDebug() << "wf = " << wf << "   hf = " << hf 
                        << "   dx = " << -(int)(wf*((float)newxpos - (float)m_xpos)) << "   dy = " << -(int)(hf*((float)newypos - (float)m_ypos))
                        << endl;                    
              
              m_currentSelection.moveBy( -(int)(wf*((float)newxpos - (float)m_xpos)), -(int)(hf*((float)newypos - (float)m_ypos)) );
              makePixmap(m_pixmap, width(), height());
              repaint(false);
               
              m_xpos = newxpos;
              m_ypos = newypos;
              setCursor( KCursor::handCursor() );
              break;
           }
          }
       else
          {
          switch (m_editMode)
           {
           case ZOOMIN:
              setCursor( KCursor::crossCursor() );
              break;
              
           case ZOOMOUT:
              setCursor( KCursor::crossCursor() );
              break;
           
           case MOVE:
              setCursor ( KCursor::sizeAllCursor() );
           }
          }
       }
}

}  // NameSpace DigikamSuperImposeImagesPlugin


#include "superimposewidget.moc"
