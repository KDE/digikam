/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-17
 * Description : a widget for to draw a image clip region.
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

// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <imageiface.h>

// Local includes.

#include "imageregionwidget.h"

namespace Digikam
{

ImageRegionWidget::ImageRegionWidget(int wp, int hp, QWidget *parent, bool scrollBar)
                 : QScrollView(parent)
{
    if( !scrollBar ) 
       {
       setVScrollBarMode( QScrollView::AlwaysOff );
       setHScrollBarMode( QScrollView::AlwaysOff );
       }
    
    setFixedSize(wp, hp);
    viewport()->setMouseTracking(true);
    setImage();
}

ImageRegionWidget::~ImageRegionWidget()
{
    if(m_pix) delete m_pix;
}

void ImageRegionWidget::setImage(void)
{
    Digikam::ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();
    uint *data = iface.getOriginalData();
    
    m_img.create( w, h, 32 );
    memcpy(m_img.bits(), data, m_img.numBytes());
    m_pix = new QPixmap(w, h);
    m_pix->convertFromImage(m_img);

    horizontalScrollBar()->setLineStep( 1 );
    horizontalScrollBar()->setPageStep( 1 );
    verticalScrollBar()->setLineStep( 1 );
    verticalScrollBar()->setPageStep( 1 );
    resizeContents(w, h);
    repaintContents(false);    
    delete [] data;
}

void ImageRegionWidget::setCenterClipPosition(void)
{
    center(contentsWidth()/2, contentsHeight()/2);    
    emit contentsMovedEvent();
}

void ImageRegionWidget::setClipPosition(int x, int y, bool targetDone)
{
    setContentsPos(x, y);    
    
    if( targetDone )
       emit contentsMovedEvent();
}

QRect ImageRegionWidget::getImageRegion(void)
{
    return( QRect::QRect(horizontalScrollBar()->value(), verticalScrollBar()->value(), 
                         visibleWidth(), visibleHeight()) );
}

QImage ImageRegionWidget::getImageRegionData(void)
{
    return ( m_img.copy(getImageRegion()) );
}

void ImageRegionWidget::drawContents(QPainter *p, int x, int y, int w, int h)
{
    if(!m_pix) return;
    else p->drawPixmap(x, y, *m_pix, x, y, w, h);
}

void ImageRegionWidget::contentsMousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
       {
       m_xpos = e->x();
       m_ypos = e->y();
       setCursor ( KCursor::sizeAllCursor() );
       }
}

void ImageRegionWidget::contentsMouseReleaseEvent ( QMouseEvent *  )
{
    setCursor ( KCursor::arrowCursor() );
    emit contentsMovedEvent();
}

void ImageRegionWidget::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
       {
       uint newxpos = e->x();
       uint newypos = e->y();
      
       scrollBy (-(newxpos - m_xpos), -(newypos - m_ypos));
     
       m_xpos = newxpos - (newxpos-m_xpos);
       m_ypos = newypos - (newypos-m_ypos);
       return;
       }
        
    setCursor( KCursor::handCursor() );
}

}  // NameSpace Digikam

#include "imageregionwidget.moc"
