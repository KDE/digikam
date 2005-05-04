/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
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

#include <cstdio>

#include <qregion.h>
#include <qpainter.h>

#include "imageiface.h"
#include "imagewidget.h"

namespace Digikam
{

ImageWidget::ImageWidget(int w, int h, QWidget *parent)
    : QWidget(parent,0,Qt::WDestructiveClose)
{
    setBackgroundMode(Qt::NoBackground);
    setFixedSize(w,h);

    m_iface = new ImageIface(w,h);
    
    m_data = m_iface->getPreviewData();
    m_w    = m_iface->previewWidth();
    m_h    = m_iface->previewHeight();

    m_rect = QRect(width()/2-m_w/2, height()/2-m_h/2,
                   m_w, m_h);
}

ImageWidget::~ImageWidget()
{
    delete [] m_data;

    delete m_iface;
}

ImageIface* ImageWidget::imageIface()
{
    return m_iface;
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    m_iface->paint(this, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());

    QRect r(0,0,width(),height());
    QRegion reg(r);
    reg -= m_rect;

    QPainter p(this);
    p.setClipRegion(reg);
    p.fillRect(r, colorGroup().background());
    p.end();
}

}
