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
    m_pixmap   = new QPixmap(w, h);
    m_editMode = MOVE;
    
    m_iface  = new Digikam::ImageIface(0, 0);
    m_data   = m_iface->getOriginalData();
    m_w      = m_iface->originalWidth();
    m_h      = m_iface->originalHeight();
    
    m_img.create( m_w, m_h, 32 );
    memcpy(m_img.bits(), m_data, m_img.numBytes());
        
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(w, h);
    setMouseTracking(true);
    
    resetEdit();
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

QImage SuperImposeWidget::makeSuperImpose(void)
{
    QSize size = getTemplateSize();
    QPixmap target(size);
    
    target.fill(colorGroup().background());
    
    QPainter p(&target);
    QPixmap pix(m_img.copy(m_currentSelection.x(), m_currentSelection.y(),
                           m_currentSelection.width(), m_currentSelection.height())
                .scale(size));
    p.drawPixmap(0, 0, pix, 0, 0, size.width(), size.height());
    QPixmap pixTemplate(m_template);
    p.drawPixmap(0, 0, pixTemplate, 0, 0, size.width(), size.height());
    p.end();    
    
    return ( target.convertToImage().convertDepth(32) );
}

void SuperImposeWidget::resetEdit(void)
{
    m_zoomFactor = 100;
    m_currentSelection = QRect(m_w/2 - m_rect.width()/2, m_h/2 - m_rect.height()/2, m_rect.width(), m_rect.height());
    makePixmap();
    repaint(false);
}

void SuperImposeWidget::makePixmap(void)
{
    m_pixmap->fill(colorGroup().background());
    
    QPainter p(m_pixmap);
    QPixmap pix(m_img.copy(m_currentSelection.x(), m_currentSelection.y(),
                           m_currentSelection.width(), m_currentSelection.height())
                .scale(m_rect.width(), m_rect.height()));
    p.drawPixmap(m_rect.x(), m_rect.y(), pix, 0, 0, m_rect.width(), m_rect.height());
    p.drawPixmap(m_rect.x(), m_rect.y(), m_templatePix, 0, 0, m_rect.width(), m_rect.height());
    p.end();
}

void SuperImposeWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);
}

void SuperImposeWidget::slotEditModeChanged(int mode)
{
    m_editMode = mode;
}

void SuperImposeWidget::slotSetCurrentTemplate(const KURL& url)
{
    m_template.load(url.path());
    QSize size = m_template.size();
    
    if (size.width() < size.height())
       {
       int neww = (int) ((float)height() / (float)size.height() * (float)size.width());
       m_rect = QRect(width()/2-neww/2, 0, neww, height());
       }
    else
       {
       int newh = (int) ((float)width() / (float)size.width() * (float)size.height());
       m_rect = QRect(0, height()/2-newh/2, width(), newh);
       }
    
    m_templatePix.convertFromImage(m_template.scale(m_rect.width(), m_rect.height()));
    
    m_currentSelection = QRect(m_w/2 - m_rect.width()/2, m_h/2 - m_rect.height()/2, m_rect.width(), m_rect.height());
    int z = m_zoomFactor;
    m_zoomFactor = 100;
    zoomSelection(z-100);
}

void SuperImposeWidget::moveSelection(int dx, int dy)
{
    float wf = (float)m_currentSelection.width() / (float)m_rect.width();
    float hf = (float)m_currentSelection.height() / (float)m_rect.height();
    
    m_currentSelection.moveBy( -(int)(wf*(float)dx), -(int)(hf*(float)dy) );
}

void SuperImposeWidget::zoomSelection(int deltaZoomFactor)
{
    m_zoomFactor = m_zoomFactor + deltaZoomFactor;
    int wf = (int)((float)m_rect.width()  * (100-(float)m_zoomFactor) / 100);
    int hf = (int)((float)m_rect.height() * (100-(float)m_zoomFactor) / 100);
    
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
        
    makePixmap();
    repaint(false);
}

void SuperImposeWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         rect().contains( e->x(), e->y() ) )
       {
       switch (m_editMode)
           {
           case ZOOMIN:
              if (m_zoomFactor < 100)
                 {
                 moveSelection(width()/2 - e->x(), height()/2 - e->y());
                 zoomSelection(+5);
                 }
              break;
              
           case ZOOMOUT:
              if (m_zoomFactor > 1)
                 {
                 moveSelection(width()/2 - e->x(), height()/2 - e->y());
                 zoomSelection(-5);
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
    if ( rect().contains( e->x(), e->y() ) )
       {
       if ( e->state() == Qt::LeftButton )
          {
          switch (m_editMode)
           {
           case ZOOMIN:
           case ZOOMOUT:
              break;
           
           case MOVE:
              uint newxpos = e->x();
              uint newypos = e->y();
              
              moveSelection(newxpos - m_xpos, newypos - m_ypos);
              makePixmap();
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
