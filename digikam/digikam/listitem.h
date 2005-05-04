/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-21
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

#ifndef LISTITEM_H
#define LISTITEM_H

#include <qstring.h>
#include <qrect.h>

class QPainter;
class QPixmap;
class QColorGroup;

class ListView;

class ListItem
{
public:

    ListItem(ListView* parent, const QString& text);
    ListItem(ListItem* parent, const QString& text);
    
    virtual ~ListItem();

    void    setText(const QString& text);
    QString text() const;

    void     setPixmap(const QPixmap& pixmap);
    QPixmap* pixmap() const;
    
    void insertChild(ListItem* child);
    void removeChild(ListItem* child);
    void adjustChildOffsets();
    
    void clear();

    bool isExpandable() const;
    bool isSelected() const;
    bool isTruncated() const;

    void setOpen(bool val);
    bool isOpen() const;

    bool isRoot() const;
    
    void repaint();

    ListView* listView() const;

    virtual int compare(ListItem* item) const;

    int childCount() const;

    ListItem* parent() const;

    ListItem* firstChild();
    ListItem* lastChild();
    ListItem* next();
    ListItem* prev();
    
protected:

    virtual void paint(QPainter *p, const QColorGroup& cg, const QRect& r);
    
private:

    ListItem();
    void init();
    
private:

    QString   m_text;
    QPixmap*  m_pixmap;
    
    bool      m_root;
    bool      m_open;
    bool      m_selected;
    bool      m_expandable;
    bool      m_clearing;
    bool      m_truncated;
    int       m_depth;
    int       m_offset;
    int       m_pos;
    int       m_childCount;

    ListItem* m_parent;
    ListItem* m_firstChild;
    ListItem* m_lastChild;
    ListItem* m_next;
    ListItem* m_prev;

    ListView* m_listView;
    
    friend class ListView;
};

#endif /* LISTITEM_H */
