/* ============================================================
 * File  : imageguidewidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-16
 * Description : 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#include <qregion.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qtimer.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Local includes.

#include "imageiface.h"
#include "imageguidewidget.h"

namespace Digikam
{

ImageGuideWidget::ImageGuideWidget(int w, int h, QWidget *parent, 
                                   bool spotVisible, int guideMode, 
                                   QColor guideColor, int guideSize, bool blink,
                                   int getColorFrom)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_spotVisible  = spotVisible;
    m_guideMode    = guideMode;
    m_guideColor   = guideColor;
    m_guideSize    = guideSize;
    m_getColorFrom = getColorFrom;
    m_focus        = false;
    m_flicker      = 0;
    m_timerID      = 0;
    
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    m_iface         = new ImageIface(w, h);
    uchar *data     = m_iface->getPreviewImage();
    m_w             = m_iface->previewWidth();
    m_h             = m_iface->previewHeight();
    bool sixteenBit = m_iface->previewSixteenBit();
    bool hasAlpha   = m_iface->previewHasAlpha();
    m_preview       = DImg(m_w, m_h, sixteenBit, hasAlpha, data);
    delete [] data;

    m_pixmap  = new QPixmap(w, h);
    m_rect    = QRect(w/2-m_w/2, h/2-m_h/2, m_w, m_h);

    resetSpotPosition();
    setSpotVisible(m_spotVisible, blink);
}

ImageGuideWidget::~ImageGuideWidget()
{
    delete m_iface;
    
    if (m_timerID) killTimer(m_timerID);
           
    if (m_pixmap) delete m_pixmap;
}

Digikam::ImageIface* ImageGuideWidget::imageIface()
{
    return m_iface;
}

void ImageGuideWidget::resetSpotPosition(void)
{
    m_spot.setX( m_w / 2 );
    m_spot.setY( m_h / 2 );
    updatePreview();
}

QPoint ImageGuideWidget::getSpotPosition(void)
{
    return (QPoint::QPoint( (int)((float)m_spot.x() * (float)m_iface->originalWidth() / (float)m_w), 
                            (int)((float)m_spot.y() * (float)m_iface->originalHeight() / (float)m_h)));
}

DColor ImageGuideWidget::getSpotColor(int getColorFrom)
{
    if (getColorFrom == OriginalImage)                          // Get point color from original image
        return (m_iface->getColorInfoFromOriginalImage(getSpotPosition()));
    else if (getColorFrom == PreviewImage)                      // Get point color from preview image
        return (m_iface->getColorInfoFromPreviewImage(m_spot));

    // In other cases, get point color from target preview image
    return (m_iface->getColorInfoFromTargetPreviewImage(m_spot));
}

void ImageGuideWidget::setSpotVisible(bool spotVisible, bool blink)
{
    m_spotVisible = spotVisible;
    
    if (blink)
    {
        if (m_spotVisible)
            m_timerID = startTimer(800);
        else
        {
            killTimer(m_timerID);
            m_timerID = 0;
        }
    }
       
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideColor(const QColor &color)
{
    m_guideColor = color;
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideSize(int size)
{
    m_guideSize = size;
    updatePreview();
}

void ImageGuideWidget::updatePixmap( void )
{
    m_pixmap->fill(colorGroup().background());
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());

    if (m_spotVisible)
    {
       // Adapt spot from image coordinate to widget coordinate.
       int xspot = m_spot.x() + m_rect.x();
       int yspot = m_spot.y() + m_rect.y();
       
       switch (m_guideMode)
       {
          case HVGuideMode:
          {
             QPainter p(m_pixmap);
             p.setPen(QPen(Qt::white, m_guideSize, Qt::SolidLine));
             p.drawLine(xspot, m_rect.top() + m_flicker, xspot, m_rect.bottom() - m_flicker);
             p.drawLine(m_rect.left() + m_flicker, yspot, m_rect.right() - m_flicker, yspot);
             p.setPen(QPen(m_guideColor, m_guideSize, Qt::DotLine));
             p.drawLine(xspot, m_rect.top() + m_flicker, xspot, m_rect.bottom() - m_flicker);
             p.drawLine(m_rect.left() + m_flicker, yspot, m_rect.right() - m_flicker, yspot);
             p.end();
             break;
          }
            
          case PickColorMode:
          {
             QPainter p(m_pixmap);
             p.setPen(QPen(m_guideColor, 1, Qt::SolidLine));
             p.drawLine(xspot-10, yspot-10, xspot+10, yspot+10);
             p.drawLine(xspot+10, yspot-10, xspot-10, yspot+10);
             p.setPen(QPen(m_guideColor, 3, Qt::SolidLine));
             p.drawEllipse( xspot-5, yspot-5, 11, 11 );

             if (m_flicker%2 != 0)
             {
                p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                p.drawEllipse( xspot-5, yspot-5, 11, 11 );
             }
                
             p.end();
             break;
          }
       }
    }
}    

void ImageGuideWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);
}

void ImageGuideWidget::updatePreview( void )
{
    updatePixmap();
    repaint(false);
}

void ImageGuideWidget::timerEvent(QTimerEvent * e)
{
    if (e->timerId() == m_timerID)
    {
       if (m_flicker == 5) m_flicker=0;
       else m_flicker++;
       updatePreview();
    }
    else
       QWidget::timerEvent(e);
}

void ImageGuideWidget::resizeEvent(QResizeEvent * e)
{
    blockSignals(true);
    delete m_pixmap;
    int w = e->size().width();
    int h = e->size().height();
    int old_w = m_w;
    int old_h = m_h;

    uchar *data     = m_iface->setPreviewImageSize(w, h);
    m_w             = m_iface->previewWidth();
    m_h             = m_iface->previewHeight();
    bool sixteenBit = m_iface->previewSixteenBit();
    bool hasAlpha   = m_iface->previewHasAlpha();
    m_preview       = DImg(m_w, m_h, sixteenBit, hasAlpha, data);
    delete [] data;

    m_pixmap  = new QPixmap(w, h);
    m_rect    = QRect(w/2-m_w/2, h/2-m_h/2, m_w, m_h);

    m_spot.setX((int)((float)m_spot.x() * ( (float)m_w / (float)old_w)));
    m_spot.setY((int)((float)m_spot.y() * ( (float)m_h / (float)old_h)));
    updatePixmap();
    blockSignals(false);
    emit signalResized();
}

void ImageGuideWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( !m_focus && e->button() == Qt::LeftButton &&
         m_rect.contains( e->x(), e->y() ) && m_spotVisible )
    {
       m_focus = true;
       m_spot.setX(e->x()-m_rect.x());
       m_spot.setY(e->y()-m_rect.y());;
       updatePreview();
    }
}

void ImageGuideWidget::mouseReleaseEvent ( QMouseEvent *e )
{
    if ( m_rect.contains( e->x(), e->y() ) && m_focus && m_spotVisible) 
    {
       m_focus = false;
       updatePreview();
       m_spot.setX(e->x()-m_rect.x());
       m_spot.setY(e->y()-m_rect.y());
       
       DColor color = getSpotColor(m_getColorFrom);
       QPoint point = getSpotPosition();
       emit spotPositionChanged( color, true, m_spot );
       QToolTip::add( this, i18n("(%1,%2)<br>RGBA:%3,%4,%5,%6")
                                 .arg(point.x()).arg(point.y())
                                 .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()) );
    }
}

void ImageGuideWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( m_rect.contains( e->x(), e->y() ) && !m_focus && m_spotVisible )
    {
       setCursor( KCursor::crossCursor() );
    }
    else if ( m_rect.contains( e->x(), e->y() ) && m_focus && m_spotVisible )
    {
       m_spot.setX(e->x()-m_rect.x());
       m_spot.setY(e->y()-m_rect.y());
    }
    else
       setCursor( KCursor::arrowCursor() );
}

}  // NameSpace Digikam

#include "imageguidewidget.moc"
