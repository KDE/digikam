/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-31
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

#include <qtimer.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>

#include "busyprogressbar.h"

BusyProgressBar::BusyProgressBar(QWidget* parent)
    : QProgressBar(parent)
{
    setFrameStyle(QFrame::Panel|QFrame::Sunken);
    m_pix = new QPixmap();
    
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotMove()));

    m_timer->start(50, false);
    m_pos = 0;
    m_dir = 1;
}

BusyProgressBar::~BusyProgressBar()
{
    delete m_timer;
    delete m_pix;
}

void BusyProgressBar::styleChange(QStyle& old)
{
    QFrame::styleChange(old);
}

void BusyProgressBar::resizeEvent(QResizeEvent* e)
{
    m_pix->resize(frameRect().width(), frameRect().height());
    QProgressBar::resizeEvent(e);    
}

void BusyProgressBar::drawContents(QPainter* painter)
{
    m_pix->fill(colorGroup().background());
    QPainter p(m_pix);
    if (m_dir == 1)
        p.fillRect(0,0,m_pos,frameRect().height(),
                   colorGroup().highlight());
    else
        p.fillRect(m_pos,0,frameRect().width(),frameRect().height(),
                   colorGroup().highlight());
    p.end();

    painter->drawPixmap(0, 0, *m_pix);
}

void BusyProgressBar::slotMove()
{
    m_pos += m_dir * 20;
    if (m_pos >= frameRect().width())
    {
        m_dir = -1;
        m_pos = frameRect().width();
    }
    else if (m_pos <= 0)
    {
        m_dir = 1;
        m_pos = 0;
    }
    update();
}

#include "busyprogressbar.moc"
