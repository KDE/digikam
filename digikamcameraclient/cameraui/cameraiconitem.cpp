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

#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpalette.h>

#include "gpfileiteminfo.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"


const char* CameraIconItem::new_xpm[] = {
"13 13 8 1",
"       c None",
".      c #232300",
"+      c #F6F611",
"@      c #000000",
"#      c #DBDA4D",
"$      c #FFFF00",
"%      c #AAA538",
"&      c #E8E540",
"      .      ",
"  .  .+.  .  ",
" @#@ .$. .#. ",
"  @$@#$#@$.  ",
"   @$%&%$@   ",
" ..#%&&&%#.. ",
".+$$&&&&&$$+@",
" ..#%&&&%#@@ ",
"   @$%&%$@   ",
"  .$@#$#@$.  ",
" @#. @$@ @#. ",
"  .  @+@  .  ",
"      @      "};


QPixmap* CameraIconItem::newEmblem = 0;


CameraIconItem::CameraIconItem(ThumbView* parent,
                               const GPFileItemInfo* fileInfo,
                               const QPixmap& pixmap)
    : ThumbItem(parent, fileInfo->name, pixmap),
      fileInfo_(fileInfo)
{
    pixWidth_  = pixmap.width();
    pixHeight_ = pixmap.height();
    loadNewEmblem();
}

CameraIconItem::~CameraIconItem()
{
    if (newEmblem) {
        delete newEmblem;
        newEmblem = 0;
    }
}

const GPFileItemInfo* CameraIconItem::fileInfo()
{
    return fileInfo_;    
}

void CameraIconItem::loadNewEmblem()
{
    if (!newEmblem) {
        newEmblem = new QPixmap(new_xpm);
    }
}

void CameraIconItem::setPixmap(const QImage& thumb)
{
    int size = pixmap()->width();

    pixWidth_  = thumb.width();
    pixHeight_ = thumb.height();

    QPainter painter;
    painter.begin(pixmap());
    painter.fillRect(0, 0, size, size, QBrush(iconView()->colorGroup().base()));
    painter.drawImage((size-thumb.width())/2,
                      (size-thumb.height())/2,
                      thumb);
    painter.end();

    repaint();

}

void CameraIconItem::paintItem(QPainter *, const QColorGroup& cg)
{

    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);

    QPixmap pix(rect().width(), rect().height());
    pix.fill(cg.base());
    QPainter painter(&pix);
    painter.drawPixmap(pRect.x(), pRect.y(), *pixmap() );

    if (isSelected()) {
        QPen pen;
        pen.setColor(cg.highlight());
        painter.setPen(pen);
        painter.drawRect(0, 0, pix.width(), pix.height());
        painter.fillRect(0, tRect.y(), pix.width(),
                     tRect.height(), cg.highlight() );
        painter.setPen( QPen( cg.highlightedText() ) );
    }
    else
        painter.setPen( cg.text() );

    painter.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
                     Qt::AlignHCenter|Qt::AlignTop,text());


    if (fileInfo_->downloaded == 0) {

        loadNewEmblem();

        int x = pRect.width()/2 + pixWidth_/2
                - newEmblem->width() - 2;
        int y = pRect.height()/2 - pixHeight_/2
                + 2;
        painter.drawPixmap(x, y, *newEmblem);
    }
        
    
    painter.end();

    QRect r(rect());
    r = QRect(iconView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(iconView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
        
}
