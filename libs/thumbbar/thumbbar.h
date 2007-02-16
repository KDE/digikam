/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-11-22
 * Description : a bar widget to display image thumbnails
 *
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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
 *
 * ============================================================ */

#ifndef THUMBBAR_H
#define THUMBBAR_H

// Qt includes.

#include <qscrollview.h>
#include <qtooltip.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

class KFileItem;

namespace Digikam
{

class ThumbBarItem;
class ThumbBarViewPriv;
class ThumbBarItemPriv;

class DIGIKAM_EXPORT ThumbBarView : public QScrollView
{
    Q_OBJECT

public:
    
    enum Orientation
    {
        Horizontal=0,      
        Vertical         
    };

public:

    ThumbBarView(QWidget* parent, int orientation=Vertical, bool exifRotate=false);
    ~ThumbBarView();

    int  countItems();
    KURL::List itemsURLs();
    
    void clear(bool updateView=true);
    void triggerUpdate();

    void removeItem(ThumbBarItem* item);

    ThumbBarItem* currentItem() const;
    void setSelected(ThumbBarItem* item);

    void setExifRotate(bool exifRotate);

    ThumbBarItem* firstItem() const;
    ThumbBarItem* lastItem()  const;
    ThumbBarItem* findItem(const QPoint& pos) const;
    ThumbBarItem* findItemByURL(const KURL& url) const;

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
    void signalItemAdded(void);
    
private slots:

    void slotUpdate();
    void slotGotPreview(const KFileItem *, const QPixmap &);
    void slotFailedPreview(const KFileItem *);
    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);
    
private:

    ThumbBarViewPriv* d;

    friend class ThumbBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarItem
{
public:

    ThumbBarItem(ThumbBarView* view, const KURL& url);
    ~ThumbBarItem();

    KURL url() const;
    
    ThumbBarItem* next() const;
    ThumbBarItem* prev() const;
    int           position() const;
    QRect         rect() const;

    void          repaint();

private:

    ThumbBarItemPriv* d;

    friend class ThumbBarView;
};

// -------------------------------------------------------------------------

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
