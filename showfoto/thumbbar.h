/* ============================================================
 * File  : thumbbar.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Copyright 2004 by Renchi Raju
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

#ifndef THUMBBAR_H
#define THUMBBAR_H

#include <qscrollview.h>
#include <kurl.h>

class QPixmap;
class KFileItem;

class ThumbBarItem;
class ThumbBarViewPriv;

class ThumbBarView : public QScrollView
{
    Q_OBJECT

public:

    ThumbBarView(QWidget* parent);
    ~ThumbBarView();

    void clear(bool updateView=true);
    void triggerUpdate();

    ThumbBarItem* currentItem() const;
    void setSelected(ThumbBarItem* item);

    ThumbBarItem* firstItem() const;
    ThumbBarItem* lastItem()  const;

    void invalidateThumb(ThumbBarItem* item);
    
protected:

    void viewportPaintEvent(QPaintEvent* e);
    void contentsMousePressEvent(QMouseEvent* e);

private:

    void insertItem(ThumbBarItem* item);
    void removeItem(ThumbBarItem* item);
    void rearrangeItems();

signals:

    void signalURLSelected(const KURL& url);
    
private slots:

    void slotUpdate();
    void slotGotPreview(const KFileItem *, const QPixmap &);
    void slotFailedPreview(const KFileItem *);
    
private:

    ThumbBarViewPriv* d;

    friend class ThumbBarItem;
};

class ThumbBarItem
{
public:

    ThumbBarItem(ThumbBarView* view,
                 const KURL& url);
    ~ThumbBarItem();

    KURL url() const;
    
    ThumbBarItem* next() const;
    ThumbBarItem* prev() const;
    int           position() const;

private:

    ThumbBarView*     m_view;
    KURL              m_url;
    ThumbBarItem*     m_next;
    ThumbBarItem*     m_prev;
    int               m_pos;
    QPixmap*          m_pixmap;

    friend class ThumbBarView;
};
    
#endif /* THUMBBAR_H */
