/* ============================================================
 * File  : imagetoolswidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-16
 * Description : this widget is dedicaced to provide some image 
 *               tools like pixel selection, pickup pixel color, 
 *               etc.
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

#include "imagetoolswidget.h"

namespace Digikam
{

ImageToolsWidget::ImageToolsWidget(int w, int h, QWidget *parent)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_iface = new ImageIface(w,h);

    m_data = m_iface->getPreviewData();
    m_w    = m_iface->previewWidth();
    m_h    = m_iface->previewHeight();
    m_pixmap = new QPixmap(w, h);
    
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(m_w, m_h);
    setMouseTracking(true);

    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2, m_w, m_h);
    
    m_freeze = false;
    m_focus = false;
}

ImageToolsWidget::~ImageToolsWidget()
{
    delete [] m_data;
    delete m_iface;
    delete m_pixmap;
}

ImageIface* ImageToolsWidget::imageIface()
{
    return m_iface;
}

void ImageToolsWidget::paintEvent( QPaintEvent * )
{
    m_pixmap->fill(colorGroup().background());
    m_iface->paint(m_pixmap, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());

    QPainter p(m_pixmap);
    p.setPen(QPen(Qt::red, 2, Qt::DotLine));
    p.drawLine(m_xpos, m_rect.top(), m_xpos, m_rect.bottom());
    p.drawLine(m_rect.left(), m_ypos, m_rect.right(), m_ypos);
    p.end();

    bitBlt(this, 0, 0, m_pixmap);
}

void ImageToolsWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton &&
         m_rect.contains( e->x(), e->y() ) )
       {
       m_focus = true;
       }
}

void ImageToolsWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( m_rect.contains( m_xpos, m_ypos ) && m_focus ) 
       {    
       m_freeze = !m_freeze;
       m_focus = false;
       }
}

void ImageToolsWidget::mouseMoveEvent ( QMouseEvent * e )
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


#include "imagetoolswidget.moc"
