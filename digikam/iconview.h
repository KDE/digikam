/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-24
 * Copyright 2005 by Renchi Raju
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

#ifndef ICONVIEW_H
#define ICONVIEW_H

// Qt includes.

#include <qscrollview.h>

class QPainter;
class QMouseEvent;
class QPaintEvent;
class QDropEvent;
class QPoint;

namespace Digikam
{

class IconItem;
class IconGroupItem;
class IconViewPriv;

class IconView : public QScrollView
{
    Q_OBJECT

public:

    IconView(QWidget* parent=0, const char* name=0);
    virtual ~IconView();

    IconGroupItem* firstGroup() const;
    IconGroupItem* lastGroup() const;
    
    IconItem* firstItem() const;
    IconItem* lastItem() const;
    IconItem* currentItem() const;
    IconItem* findItem(const QPoint& pos);
    
    void setCurrentItem(IconItem* item);

    int  count() const;
    int  groupCount() const;

    virtual void clear(bool update=true);
    void sort();

    void clearSelection();
    void selectAll();
    void invertSelection();
    
    void selectItem(IconItem* item, bool select);

    void triggerUpdate();
    void rearrangeItems(bool update=true);
    
    void insertGroup(IconGroupItem* group);
    void takeGroup(IconGroupItem* group);

    void insertItem(IconItem* item);
    void takeItem(IconItem* item);

    void ensureItemVisible(IconItem *item);
    IconItem* findFirstVisibleItem(const QRect& r) const;
    IconItem* findLastVisibleItem(const QRect& r) const;
    
    virtual QRect itemRect() const;
    virtual QRect bannerRect() const;

    QRect contentsRectToViewport(const QRect& r) const;

    void setEnableToolTips(bool val);
    
protected:

    virtual void viewportPaintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* e);
    virtual void contentsMousePressEvent(QMouseEvent* e);
    virtual void contentsMouseMoveEvent(QMouseEvent* e);
    virtual void contentsMouseReleaseEvent(QMouseEvent* e);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *e);
    virtual void contentsWheelEvent(QWheelEvent* e);
    virtual void leaveEvent(QEvent *e);
    virtual void focusOutEvent(QFocusEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    
    virtual void startDrag();

    void drawFrameRaised(QPainter* p);
    void drawFrameSunken(QPainter* p);

    virtual bool acceptToolTip(IconItem* , const QPoint&);
    
private:

    void rebuildContainers();
    void appendContainer();
    void deleteContainers();

    void drawRubber(QPainter* p);

    void itemClickedToOpen(IconItem* item);

    bool anchorIsBehind() const;
    
signals:

    void signalSelectionChanged();
    void signalRightButtonClicked(IconItem* item, const QPoint& pos);
    void signalRightButtonClicked(const QPoint& pos);
    void signalDoubleClicked(IconItem* item);
    void signalReturnPressed(IconItem* item);
    void signalShowToolTip(IconItem* item);
    
public slots:

    void slotUpdate();

private slots:

    void slotToolTip();

private:

    IconViewPriv* d;

};

}  // namespace Digikam
    
#endif /* ICONVIEW_H */
