/* ============================================================
 * File  : albumfolderitem.h
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

#ifndef ALBUMFOLDERITEM_H
#define ALBUMFOLDERITEM_H

#include <qlistview.h>
#include <qpixmap.h>

class QPainter;
class QColorGroup;
class QString;
class QRect;

class AlbumFolderView;

namespace Digikam
{
class AlbumInfo;
}

class AlbumFolderItem : public QListViewItem {

    friend class AlbumFolderView;

public:

    AlbumFolderItem(AlbumFolderView *parent, const QString& name,
                    int year, int month);

    AlbumFolderItem(AlbumFolderView *parent, const Digikam::AlbumInfo* album);

    AlbumFolderItem(AlbumFolderItem *parent, const Digikam::AlbumInfo* album);

    bool isGroupItem();

    bool isHighlighted();
    void setPixmap(const QPixmap& pixmap);

    const Digikam::AlbumInfo* albumInfo() {
        return albumInfo_;
    }

    void paintCell(QPainter *p, const QColorGroup &cg,
                   int column, int width, int align);
    void paintFocus(QPainter *p, const QColorGroup &cg,
                    const QRect &r);

    int compare(QListViewItem *i, int col, bool ascending) const;

    void addDropHighlight();
    void removeDropHighlight();

private:

    void paint(QPainter *p, const QString& s,
               const QColorGroup &cg);

private:

    const Digikam::AlbumInfo* albumInfo_;
    bool isGroup_;
    int  year_;
    int  month_;
    QPixmap pix_;
    bool    highlighted_;
};

#endif
