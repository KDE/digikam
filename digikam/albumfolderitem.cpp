/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-08
 * Description :
 *
 * Copyright 2003 by Renchi Raju

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
#include <qpalette.h>
#include <qfont.h>
#include <qstring.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "themeengine.h"
#include "album.h"

#include "albumfolderview.h"
#include "albumfolderitem.h"

AlbumFolderItem::AlbumFolderItem(AlbumFolderItem *parent,
                                 const QString& name,
                                 int year, int month)
    : ListItem(parent, name)
{
    isGroup_     = true;
    isRoot_      = false;
    year_        = year;
    month_       = month;
    album_       = 0;
    highlighted_ = false;
}

AlbumFolderItem::AlbumFolderItem(AlbumFolderView *parent,
                                 Album* album)
    : ListItem(parent, album->getTitle())
{
    isGroup_     = false;
    isRoot_      = true;
    month_       = 0;
    year_        = 0;
    album_       = album;
    highlighted_ = false;
}

AlbumFolderItem::AlbumFolderItem(AlbumFolderItem *parent,
                                 Album* album)
    : ListItem(parent, album->getTitle())
{
    isGroup_     = false;
    isRoot_      = false;
    month_       = 0;
    year_        = 0;
    album_       = album;
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
    ListItem::setPixmap(pixmap);
    highlighted_ = true;
}

Album* AlbumFolderItem::album() const
{
    return album_;
}

int AlbumFolderItem::compare(ListItem *item) const
{
    if (!isGroup_ || (year_ == 0 && month_ == 0)) {
        return ListItem::compare(item);
    }
    else {
        AlbumFolderItem *folderItem = static_cast<AlbumFolderItem *>(item);

        int myWeight  = year_*100 + month_;
        int hisWeight = folderItem->year_*100 + folderItem->month_;

        if (myWeight == hisWeight)
            return 0;
        else if (myWeight > hisWeight)
            return 1;
        else
            return -1;
    }
}

void AlbumFolderItem::addDropHighlight()
{
    if (!pixmap()) return;
    pix_ = *pixmap();

    KPixmap p(pix_);
    KPixmapEffect::fade(p, 0.5, listView()->colorGroup().base());

    setPixmap(p);

}

void AlbumFolderItem::removeDropHighlight()
{
    setPixmap(pix_);
}

void AlbumFolderItem::paint(QPainter *p, const QColorGroup &cg, const QRect& r)
{
    ThemeEngine* te = ThemeEngine::instance();
    
    if (isRoot_) {
        p->save();

        QRect tr(r);
        if (pixmap()) {
            p->drawPixmap(r.x(), (tr.height()-pixmap()->height())/2, *pixmap());
            tr.setLeft(tr.x() + pixmap()->width() + 5);
        }
    
        QFont f(p->font());
        f.setBold(true);
        p->setFont(f);
        QRect br;    
        p->setPen( QPen(isSelected() ? te->textSelColor() : te->textRegColor(), 2) );
        p->drawText(tr.x(), tr.y(), tr.width(), r.height(),
                    Qt::AlignLeft|Qt::AlignVCenter, text(), -1, &br);
        p->drawLine(br.right() + 5, tr.height()/2, tr.right(), tr.height()/2);
        p->restore();
    }
    else if (isGroup_) {
        p->save();

        QRect tr(r);
        if (pixmap()) {
            p->drawPixmap(r.x(), (tr.height()-pixmap()->height())/2, *pixmap());
            tr.setLeft(tr.x() + pixmap()->width() + 5);
        }
    
        QFont f(p->font());
        f.setItalic(true);
        p->setFont(f);
        QRect br;    
        p->setPen( QPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor(), 1) );
        p->drawText(tr.x(), tr.y(), tr.width(), r.height(),
                    Qt::AlignLeft|Qt::AlignVCenter, text(), -1, &br);
        p->drawLine(br.right() + 5, tr.height()/2, tr.right(), tr.height()/2);
        p->restore();
    }
    else
        ListItem::paint(p, cg, r);
}
