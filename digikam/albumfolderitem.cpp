/* ============================================================
 * File  : albumfolderitem.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-08
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

#include <qpainter.h>
#include <qpalette.h>
#include <qfont.h>
#include <qstring.h>
#include <qscrollbar.h>
#include <qstyle.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "albuminfo.h"

#include "albumfolderview.h"
#include "albumfolderitem.h"

AlbumFolderItem::AlbumFolderItem(AlbumFolderView *parent,
                                 const QString& name,
                                 int year, int month)
    : QListViewItem(parent, name)
{
    isGroup_ = true;
    setSelectable(false);
    year_  = year;
    month_ = month;
    albumInfo_ = 0;
    highlighted_ = false;
}

AlbumFolderItem::AlbumFolderItem(AlbumFolderView *parent,
                                 const Digikam::AlbumInfo* album)
    : QListViewItem(parent, album->getTitle())
{
    isGroup_ = false;
    month_ = 0;
    year_  = 0;
    albumInfo_ = album;
    highlighted_ = false;
}

AlbumFolderItem::AlbumFolderItem(AlbumFolderItem *parent,
                                 const Digikam::AlbumInfo* album)
    : QListViewItem(parent, album->getTitle())
{
    isGroup_ = false;
    month_ = 0;
    year_  = 0;
    albumInfo_ = album;
    highlighted_ = false;
}


bool AlbumFolderItem::isGroupItem()
{
    return isGroup_;
}


bool AlbumFolderItem::isHighlighted()
{
    return highlighted_;
}

void AlbumFolderItem::setPixmap(const QPixmap& pixmap)
{
    QListViewItem::setPixmap(0, pixmap);
    highlighted_ = true;
}

void AlbumFolderItem::paintCell(QPainter *p, const QColorGroup &cg,
                                int column, int width, int align)
{

    if (!isGroup_) {
        QListViewItem::paintCell(p, cg, column, width,  align);
        return;
    }

    QFont f(listView()->font());
    int size = f.pixelSize();
    if (size <= 0){
        size = f.pointSize();
        f.setPointSize(size - 1);
    }else{
        f.setPixelSize(size - 1);
    }
    f.setBold(true);
    f.setItalic(true);
    p->setFont(f);
    paint(p, text(0), cg);

}

void AlbumFolderItem::paintFocus(QPainter *p, const QColorGroup &cg,
                                 const QRect &r)
{
    if (!isGroup_)
        QListViewItem::paintFocus(p, cg, r);
}

void AlbumFolderItem::paint(QPainter *p, const QString& s,
                            const QColorGroup &c)
{
    QColorGroup cg(c);
    int width = listView()->width();
    const QScrollBar *vBar = listView()->verticalScrollBar();

    if (vBar && vBar->isVisible()) width -= vBar->width();

    if (isSelected()){
        cg.setBrush(QColorGroup::Base, cg.brush(QColorGroup::Highlight));
        p->setPen(cg.color(QColorGroup::HighlightedText));
    }
    else {
        p->setPen(cg.color(QColorGroup::Dark));
    }

    p->fillRect( 0, 0, width, height(), cg.base());

    int x = 2;

    if (isSelected()){
        cg.setBrush(QColorGroup::Base, cg.brush(QColorGroup::Highlight));
        p->setPen(cg.color(QColorGroup::HighlightedText));
    }
    else{
        p->setPen(cg.color(QColorGroup::Dark));
    }

    QRect br;
    p->drawText(x, 0, width - x, height(), AlignLeft | AlignVCenter, s, -1, &br);
    x = br.right() + 5;

    if ((x < width - 6)){
        QRect rcSep(x, height()/2, width-6-x, 1);
        listView()->style().drawPrimitive(QStyle::PE_Separator, p, rcSep, cg);
    }


}

int AlbumFolderItem::compare(QListViewItem *i,
                             int col, bool ascending) const
{
    if (col != 0 || !isGroup_)
        return QListViewItem::compare(i, col, ascending);

    if (year_ == 0 && month_ == 0) 
        return QListViewItem::compare(i, col, ascending);
    
    AlbumFolderItem *folderItem = static_cast<AlbumFolderItem *>(i);

    int myWeight  = year_*100 + month_;
    int hisWeight = folderItem->year_*100 + folderItem->month_;

    if (myWeight == hisWeight)
        return 0;
    else if (myWeight > hisWeight)
        return 1;
    else
        return -1;
}

void AlbumFolderItem::addDropHighlight()
{
    if (!pixmap(0)) return;
    pix_ = *pixmap(0);

    KPixmap p(pix_);
    KPixmapEffect::fade(p, 0.5, listView()->colorGroup().base());

    QListViewItem::setPixmap(0, p);

}

void AlbumFolderItem::removeDropHighlight()
{
    QListViewItem::setPixmap(0, pix_);
}
