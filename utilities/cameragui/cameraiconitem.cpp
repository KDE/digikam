/* ============================================================
 * File  : cameraiconitem.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-23
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kfileitem.h>

#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpalette.h>

#include "cameraiconview.h"
#include "cameraiconitem.h"


QPixmap* CameraIconItem::baseRegPix = 0;
QPixmap* CameraIconItem::baseSelPix = 0;

CameraIconItem::CameraIconItem(ThumbView* parent,
                               KFileItem* fileItem,
                               const QPixmap& pixmap)
    : ThumbItem(parent, fileItem->name(), pixmap),
      m_fileItem(fileItem)
{
    m_pixWidth  = pixmap.width();
    m_pixHeight = pixmap.height();
    loadBasePix();
}

CameraIconItem::~CameraIconItem()
{
}

KFileItem* CameraIconItem::fileItem() const
{
    return m_fileItem;    
}

void CameraIconItem::loadBasePix()
{
    if (!baseRegPix)
    {
        int w = rect().width();
        int h = rect().height();
        baseRegPix = new QPixmap(w, h);
        baseSelPix = new QPixmap(w, h);

        QPainter p;

        p.begin(baseRegPix);
        p.fillRect(0, 0, w, h, Qt::white);
        p.setPen(QColor("#cdcdcd"));
        p.drawLine(0,0,0,h-1);
        p.drawLine(0,0,w-1,0);
        p.setPen(QColor("#8d8d8d"));
        p.drawLine(0,h-1,w-1,h-1);
        p.drawLine(w-1,0,w-1,h-1);
        p.end();

        p.begin(baseSelPix);
        p.fillRect(0, 0, w, h,
                   iconView()->colorGroup().highlight());
        p.setPen(QColor("#888888"));
        p.drawLine(0,0,0,h-1);
        p.drawLine(0,0,w-1,0);
        p.setPen(QColor("#000000"));
        p.drawLine(0,h-1,w-1,h-1);
        p.drawLine(w-1,0,w-1,h-1);
        p.end();
    }
}

void CameraIconItem::setPixmap(const QPixmap& pix)
{
    pixmap()->resize(pix.width(), pix.height());
    bitBlt(pixmap(), 0, 0, &pix);

    repaint();
}

void CameraIconItem::paintItem(QPainter *, const QColorGroup& cg)
{

    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);

    QPixmap pix(rect().width(), rect().height());

    bitBlt(&pix, 0, 0, isSelected() ? baseSelPix : baseRegPix);
    QPainter painter(&pix);
    painter.drawPixmap(pRect.x() + (pRect.width()-pixmap()->width())/2,
                       pRect.y() + (pRect.height()-pixmap()->height())/2,
                       *pixmap() );

    painter.setPen( isSelected() ? cg.highlightedText() : cg.text() );

    painter.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
                     Qt::AlignHCenter|Qt::AlignTop,text());

    /*
    if (fileInfo_->downloaded == 0) {

        loadNewEmblem();

        int x = pRect.width()/2 + pixWidth_/2
                - newEmblem->width() - 2;
        int y = pRect.height()/2 - pixHeight_/2
                + 2;
        painter.drawPixmap(x, y, *newEmblem);
    }
    */
    
    painter.end();

    QRect r(rect());
    r = QRect(iconView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(iconView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}
