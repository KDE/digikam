/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qtimer.h>

#include "animwidget.h"

AnimWidget::AnimWidget(QWidget* parent, int size )
    : QWidget(parent, 0, WResizeNoErase|WRepaintNoErase)
{
    setBackgroundMode(Qt::NoBackground);
    
    m_size = size;
    m_pos  = 0;
    m_pix  = new QPixmap(m_size,m_size);
    setFixedSize(m_size, m_size);

    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeout()));
}

AnimWidget::~AnimWidget()
{
    delete m_timer;
    delete m_pix;
}

void AnimWidget::start()
{
    m_pos  = 0;
    m_timer->start(100);
}

void AnimWidget::stop()
{
    m_pos  = 0;
    m_timer->stop();
    repaint();
}

void AnimWidget::paintEvent(QPaintEvent*)
{
    m_pix->fill(colorGroup().background());
    QPainter p(m_pix);

    p.translate(m_size/2, m_size/2);

    if (m_timer->isActive())
    {
        p.setPen(QPen(colorGroup().text()));
        p.rotate( m_pos );
    }
    else
    {
        p.setPen(QPen(colorGroup().dark()));
    }
            
    for ( int i=0; i<12; i++ )
    {
        p.drawLine(m_size/2-4,0,m_size/2-2,0);
        p.rotate(30);
    }

    
    p.end();
    bitBlt(this, 0, 0, m_pix);
}

void AnimWidget::slotTimeout()
{
    m_pos = (m_pos + 10) % 360;
    repaint();    
}

bool AnimWidget::running() const
{
    return m_timer->isActive();    
}

#include "animwidget.moc"
