/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright 2002-2004 by Renchi Raju
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
 * ============================================================ */

#ifndef THUMBITEM_H
#define THUMBITEM_H

#include <qrect.h>
#include <qstring.h>

class QPixmap;
class QPainter;
class QColorGroup;

class ThumbView;
class ThumbItemLineEdit;
class ThumbItemPrivate;

class ThumbItem {

    friend class ThumbView;
    friend class ThumbItemLineEdit;

public:

    ThumbItem(ThumbView* parent,
              const QString& text,
              const QPixmap& pixmap);
    virtual ~ThumbItem();

    QPixmap *pixmap() const;
    QString text() const;

    ThumbItem *nextItem();
    ThumbItem *prevItem();
    
    int x() const;
    int y() const;
    int width() const;
    int height() const;

    QRect rect() const;
    QRect textRect(bool relative=true) const;
    QRect pixmapRect(bool relative=true) const;

    bool move(int x, int y);

    void setSelected(bool val, bool cb=true);
    bool isSelected();

    virtual void setPixmap(const QPixmap& pixmap);
    virtual void setText(const QString& text);
    void repaint();
    
    ThumbView* iconView();

    void rename();
    virtual int compare(ThumbItem *item);

    virtual QString key() const;
    
protected:

    virtual void calcRect();
    void setRect(const QRect& rect);
    void setTextRect(const QRect& rect);
    void setPixmapRect(const QRect& rect);

    virtual void paintItem(QPainter *p, const QColorGroup& cg);

    void renameItem();
    void cancelRenameItem();

private:

    ThumbItemPrivate *d;
    ThumbView *view;
    ThumbItem *next;
    ThumbItem *prev;
    ThumbItemLineEdit *renameBox;


};

#endif
