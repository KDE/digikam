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

#ifndef ALBUMFOLDERITEM_DEPRECATED_H
#define ALBUMFOLDERITEM_DEPRECATED_H

#include "listitem.h"
#include <qpixmap.h>

class QPainter;
class QColorGroup;
class QString;
class QRect;

class AlbumFolderView_Deprecated;
class Album;

class AlbumFolderItem_Deprecated : public ListItem {

    friend class AlbumFolderView_Deprecated;

public:

    AlbumFolderItem_Deprecated(AlbumFolderView_Deprecated *parent, Album* album);

    AlbumFolderItem_Deprecated(AlbumFolderItem_Deprecated *parent, Album* album);

    AlbumFolderItem_Deprecated(AlbumFolderItem_Deprecated *parent, const QString& name,
                    int year, int month);

    bool isGroupItem();
    bool isHighlighted();
    void setPixmap(const QPixmap& pixmap);

    Album* album() const;

    int  compare(ListItem *item) const;

    void addDropHighlight();
    void removeDropHighlight();

protected:

    void paint(QPainter *p, const QColorGroup &cg, const QRect& r);

private:

    Album*  album_;
    bool    isGroup_;
    bool    isRoot_;
    int     year_;
    int     month_;
    QPixmap pix_;
    bool    highlighted_;
};

#endif
