/* ============================================================
 * File  : imageguidewidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-16
 * Description : 
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

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qregion.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <imageiface.h>

// Local includes.

#include "imageguidewidget.h"

namespace Digikam
{

ImageGuideWidget::ImageGuideWidget(int w, int h, QWidget *parent, bool crossVisible, int guideMode)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_iface = new Digikam::ImageIface(w,h);

    m_data = m_iface->getPreviewData();
    m_w    = m_iface->previewWidth();
    m_h    = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(m_w, m_h);
    setMouseTracking(true);

    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
    
    m_crossVisible = crossVisible;
    m_guideMode = guideMode;
    m_freeze = true;
    m_focus = false;
    resetCrossPosition();
}

ImageGuideWidget::~ImageGuideWidget()
{
    delete [] m_data;
    delete m_iface;
    
    if (m_pixmap)
       delete m_pixmap;
}

Digikam::ImageIface* ImageGuideWidget::imageIface()
{
    return m_iface;
}

void ImageGuideWidget::resetCrossPosition(void)
{
    m_xpos = m_w / 2;
    m_ypos = m_h / 2;
    repaint(false);
}

QPoint ImageGuideWidget::getCrossCenterPosition(void)
{
    return (QPoint::QPoint( (int)((float)m_xpos * (float)m_iface->originalWidth() / (float)m_w), 
                            (int)((float)m_ypos * (float)m_iface->originalHeight() / (float)m_h)));
}

QColor ImageGuideWidget::getCrossCenterColor(void)
{
    // Get cross position in real image.
    QPoint currentPointPosition = getCrossCenterPosition();
    
    uint *currentPointData = m_iface->getOriginalData() + currentPointPosition.x() + 
                             (m_iface->originalWidth() * currentPointPosition.y());
    QColor currentPointColor(
                             (*currentPointData >> 16) & 0xff,        // Red.
                             (*currentPointData >>  8) & 0xff,        // Green.
                             (*currentPointData)       & 0xff         // Blue.
                             );
    return(currentPointColor);
}

void ImageGuideWidget::setCrossVisible(bool crossVisible)
{
    m_crossVisible = crossVisible;
    repaint(false);
}

void ImageGuideWidget::paintEvent( QPaintEvent * )
{
    m_pixmap->fill(colorGroup().background());
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());

    if (m_crossVisible)
       {
       switch (m_guideMode)
          {
          case HVGuideMode:
             {
             QPainter p(m_pixmap);
             p.setPen(QPen(Qt::red, 1, Qt::DotLine));
             p.drawLine(m_xpos, m_rect.top(), m_xpos, m_rect.bottom());
             p.drawLine(m_rect.left(), m_ypos, m_rect.right(), m_ypos);
             p.end();
             break;
             }
            
          case PickColorMode:
             {
             QPainter p(m_pixmap);
             p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
             p.drawLine(m_xpos-10, m_ypos-10, m_xpos+10, m_ypos+10);
             p.drawLine(m_xpos+10, m_ypos-10, m_xpos-10, m_ypos+10);
             p.drawEllipse( m_xpos-5, m_ypos-5, 11, 11 );
             p.end();
             break;
             }
          }
       }
    
    bitBlt(this, 0, 0, m_pixmap);
}

void ImageGuideWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_rect.contains( e->x(), e->y() ) )
       {
       m_focus = true;
       }
}

void ImageGuideWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_rect.contains( m_xpos, m_ypos ) && m_focus ) 
       {    
       m_freeze = !m_freeze;
       m_focus = false;
       emit crossCenterColorChanged( getCrossCenterColor() );
       emit crossCenterPositionChanged( getCrossCenterPosition() );
       }
}

void ImageGuideWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( m_rect.contains( e->x(), e->y() ) && !m_freeze )
        {
        setCursor( KCursor::crossCursor() );
        m_xpos = e->x();
        m_ypos = e->y();
        repaint(false);
        }
    else
        setCursor ( KCursor::arrowCursor() );
}

}  // NameSpace Digikam


#include "imageguidewidget.moc"
