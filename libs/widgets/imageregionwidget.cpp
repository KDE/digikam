/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-17
 * Description : a widget to draw a image clip region.
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

// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qfont.h> 
#include <qfontmetrics.h> 

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
                 : QScrollView(parent, 0, WRepaintNoErase)
{
    m_separateView     = false;
    m_movingInProgress = false;
    m_pix              = 0L;
    
    if( !scrollBar ) 
       {
       setVScrollBarMode( QScrollView::AlwaysOff );
       setHScrollBarMode( QScrollView::AlwaysOff );
       }
    
    setMinimumSize(wp, hp);
    viewport()->setMouseTracking(true);

    Digikam::ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();
    uint *data = iface.getOriginalData();
    m_img.create( w, h, 32 );
    memcpy(m_img.bits(), data, m_img.numBytes());
    delete [] data;
            
    updateOriginalImage();
}

ImageRegionWidget::~ImageRegionWidget()
{
    if(m_pix) delete m_pix;
}

void ImageRegionWidget::viewportResizeEvent(QResizeEvent *)
{
    updateOriginalImage();
    QTimer::singleShot(0, this, SLOT(slotTimerResizeEvent())); 
}

void ImageRegionWidget::slotTimerResizeEvent()
{
    emit contentsMovedEvent(true);
}

void ImageRegionWidget::slotSeparateViewToggled(bool t)
{
    m_separateView = t;
    QTimer::singleShot(0, this, SLOT(slotTimerResizeEvent())); 
}

void ImageRegionWidget::updateOriginalImage()
{
    updatePixmap(&m_img);
}

void ImageRegionWidget::updatePreviewImage(QImage *img)
{
    QImage image = m_img.copy();
    QRect region = getImageRegionToRender();
    bitBlt( &image, region.topLeft().x(), region.topLeft().y(), img, 0, 0,
            img->width(), img->height());
    updatePixmap(&image);
}

void ImageRegionWidget::updatePixmap(QImage *img)
{
    int w = img->width();
    int h = img->height();
    m_pix = new QPixmap(w, h);
    m_pix->convertFromImage(*img);
    
    horizontalScrollBar()->setLineStep( 1 );
    horizontalScrollBar()->setPageStep( 1 );
    verticalScrollBar()->setLineStep( 1 );
    verticalScrollBar()->setPageStep( 1 );
    resizeContents(w, h);
    repaintContents(false);    
}

void ImageRegionWidget::drawContents(QPainter *p, int x, int y, int w, int h)
{
    if(!m_pix) return;
    else p->drawPixmap(x, y, *m_pix, x, y, w, h);
    
    if (m_separateView & !m_movingInProgress)
        {
        p->setPen(QPen(Qt::red, 2, Qt::DotLine));
        p->drawLine(getImageRegionToRender().topLeft().x(),    getImageRegionToRender().topLeft().y(),
                    getImageRegionToRender().bottomLeft().x(), getImageRegionToRender().bottomLeft().y());
                    
        p->setPen(QPen::QPen(Qt::red, 1)) ;                    
        QFontMetrics fontMt = p->fontMetrics();
        
        QString text(i18n("Target"));
        QRect textRect;
        QRect fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
        textRect.setTopLeft(QPoint::QPoint(getImageRegionToRender().topLeft().x()+20, 
                                           getImageRegionToRender().topLeft().y()+20));
        textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height()) );
        p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p->drawRect(textRect);
        p->drawText(textRect, Qt::AlignCenter, text);
                    
        text = i18n("Original");                    
        fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text); 
        textRect.setTopLeft(QPoint::QPoint(contentsX()+20, contentsY()+20));
        textRect.setSize( QSize::QSize(fontRect.width(), fontRect.height() ) );       
        p->fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p->drawRect(textRect);
        p->drawText(textRect, Qt::AlignCenter, text);
        }
}

void ImageRegionWidget::setCenterContentsPosition(void)
{
    center(contentsWidth()/2, contentsHeight()/2);    
    emit contentsMovedEvent(true);
}

void ImageRegionWidget::setContentsPosition(int x, int y, bool targetDone)
{
    setContentsPos(x, y);    
    
    if( targetDone )
       m_movingInProgress = false;
    else
       m_movingInProgress = true;
    
    repaintContents(false);    
    
    if( targetDone )
       emit contentsMovedEvent(true);
}

QRect ImageRegionWidget::getImageRegion(void)
{
    return( QRect::QRect(contentsX(), contentsY(), 
           (visibleWidth()  < m_img.width()) ? visibleWidth()  : m_img.width(),
           (visibleHeight() < m_img.height()) ? visibleHeight() : m_img.height()) );
}

QRect ImageRegionWidget::getImageRegionToRender(void)
{
    int normalizedW, normalizedH;
    
    // For large screen.
    if (visibleWidth()  > m_img.width())  normalizedW = m_img.width();
    else normalizedW = visibleWidth();
    if (visibleHeight() > m_img.height()) normalizedH = m_img.height();
    else normalizedH = visibleHeight();

    QRect region;
    
    if (m_separateView)
        region = QRect::QRect(contentsX()+normalizedW/2, contentsY(), normalizedW/2, normalizedH);
    else 
        region = QRect::QRect(contentsX(), contentsY(), normalizedW, normalizedH);
    
    kdDebug() << "Region: (" << region.x() << ", " 
                             << region.y() << ", "
                             << region.width() << ", "
                             << region.height() << ")" << endl;    
            
    return (region);
}

QImage ImageRegionWidget::getImageRegionData(void)
{
    return ( m_img.copy(getImageRegionToRender()) );
}

void ImageRegionWidget::contentsMousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
       {
       m_xpos = e->x();
       m_ypos = e->y();
       m_movingInProgress = true;
       setCursor( KCursor::sizeAllCursor() );    
       updateOriginalImage();
       }
}

void ImageRegionWidget::contentsMouseReleaseEvent ( QMouseEvent *  )
{
    m_movingInProgress = false;
    setCursor( KCursor::arrowCursor() );    
    emit contentsMovedEvent(true);
}

void ImageRegionWidget::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
       {
       uint newxpos = e->x();
       uint newypos = e->y();
       
       scrollBy (-(newxpos - m_xpos), -(newypos - m_ypos));
       repaintContents(false);    
       
       m_xpos = newxpos - (newxpos-m_xpos);
       m_ypos = newypos - (newypos-m_ypos);
       emit contentsMovedEvent(false);
       return;
       }

    setCursor( KCursor::handCursor() );    
}

}  // NameSpace Digikam

#include "imageregionwidget.moc"
