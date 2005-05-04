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

#ifndef THUMBVIEW_H
#define THUMBVIEW_H

#include <qscrollview.h>

class QPainter;
class QMouseEvent;
class QPaintEvent;
class QDropEvent;
class QPoint;

class ThumbItem;
class ThumbViewPrivate;

class ThumbView : public QScrollView {

    Q_OBJECT

    friend class ThumbItem;
    
public:

    ThumbView(QWidget* parent=0, const char* name=0,
              WFlags fl=0);
    ~ThumbView();

    ThumbItem* firstItem();
    ThumbItem* lastItem();
    ThumbItem* findItem(const QPoint& pos);
    ThumbItem* findItem(const QString& text);

    int count();
    int index(ThumbItem* item);
    
    virtual void clear(bool update=true);
    void rearrangeItems(bool update=true);
    void triggerUpdate();

    void repaintBanner();

    void clearSelection();
    void selectAll();
    void invertSelection();

    void selectItem(ThumbItem* item, bool select);

    virtual void insertItem(ThumbItem *item);
    virtual void takeItem(ThumbItem *item);
    void updateItemContainer(ThumbItem *item);
    QRect contentsRectToViewport(const QRect& r);

    void ensureItemVisible(ThumbItem *item);
    ThumbItem *findFirstVisibleItem(const QRect &r ) const;
    ThumbItem *findLastVisibleItem(const  QRect &r ) const;

    void sort();

    void setEnableToolTips(bool val);
    
protected:

    virtual void leaveEvent(QEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    void drawFrameRaised(QPainter* p);
    void drawFrameSunken(QPainter* p);
    
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *e);
    virtual void contentsWheelEvent(QWheelEvent *e);
    
    virtual void viewportPaintEvent(QPaintEvent *pe);
    virtual void resizeEvent(QResizeEvent* e);

    virtual void keyPressEvent(QKeyEvent *e);
        
    virtual void calcBanner();
    virtual void paintBanner(QPainter *p);
    virtual void setBannerRect(const QRect& r);
    QRect bannerRect();

    virtual void startDrag();
    virtual void contentsDropEvent(QDropEvent *e);
    
    virtual bool acceptToolTip(ThumbItem *item, const QPoint &mousePos);

private:

    void drawRubber(QPainter *p);

    void rebuildContainers();
    void appendContainer();
    void deleteContainers();
    void keySelectItem(ThumbItem* item, bool shift);
    void itemClickedToOpen(ThumbItem *item);
    
private:

    ThumbItem* makeRow(ThumbItem *begin, int &y, bool &changed);
    void emitRenamed(ThumbItem *item);

private:


    ThumbViewPrivate *d;
    ThumbItem *renamingItem;

signals:

    void signalSelectionChanged();
    void signalRightButtonClicked(const QPoint &pos);
    void signalRightButtonClicked(ThumbItem *item, const QPoint &pos);
    void signalDoubleClicked(ThumbItem *item);
    void signalReturnPressed(ThumbItem *item);
    void signalItemRenamed(ThumbItem *item);
    void signalShowToolTip(ThumbItem *item);

public slots:

    void slotUpdate();

private slots:

    void slotToolTip();
    
};

#endif
