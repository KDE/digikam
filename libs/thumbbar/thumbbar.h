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

// Qt includes.

#include <qscrollview.h>
#include <qtooltip.h>

// KDE includes.

#include <kurl.h>

#include "digikam_export.h"

class QPixmap;

class KFileItem;

namespace Digikam
{

class ThumbBarItem;
class ThumbBarViewPriv;

class DIGIKAM_EXPORT ThumbBarView : public QScrollView
{
    Q_OBJECT

public:

    ThumbBarView(QWidget* parent);
    ~ThumbBarView();

    int  countItems();
    
    void clear(bool updateView=true);
    void triggerUpdate();

    void removeItem(ThumbBarItem* item);

    ThumbBarItem* currentItem() const;
    void setSelected(ThumbBarItem* item);

    ThumbBarItem* firstItem() const;
    ThumbBarItem* lastItem()  const;
    ThumbBarItem* findItem(const QPoint& pos) const;

    void invalidateThumb(ThumbBarItem* item);
        
protected:

    void viewportPaintEvent(QPaintEvent* e);
    void contentsMousePressEvent(QMouseEvent* e);

private:

    void insertItem(ThumbBarItem* item);
    void rearrangeItems();
    void repaintItem(ThumbBarItem* item);

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

class DIGIKAM_EXPORT ThumbBarItem
{
public:

    ThumbBarItem(ThumbBarView* view,
                 const KURL& url);
    ~ThumbBarItem();

    KURL url() const;
    
    ThumbBarItem* next() const;
    ThumbBarItem* prev() const;
    int           position() const;
    QRect         rect() const;

    void          repaint();

private:

    ThumbBarView*     m_view;
    KURL              m_url;
    ThumbBarItem*     m_next;
    ThumbBarItem*     m_prev;
    int               m_pos;
    QPixmap*          m_pixmap;

    friend class ThumbBarView;
};

class DIGIKAM_EXPORT ThumbBarToolTip : public QToolTip
{
public:

    ThumbBarToolTip(ThumbBarView* parent);

protected:
    
    void maybeTip(const QPoint& pos);

private:

    ThumbBarView* m_view;
};

}  // NameSpace Digikam

#endif /* THUMBBAR_H */
