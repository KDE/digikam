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

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <qscrollview.h>

class ListItem;
class ListViewPriv;

class ListView : public QScrollView
{
    Q_OBJECT
    
public:

    ListView(QWidget* parent=0, const char* name=0,
             WFlags f=0);
    virtual ~ListView();

    void      setSelected(ListItem* item);
    ListItem* getSelected() const;
    
    ListItem* itemAt(const QPoint& pt);
    QRect     itemRect(ListItem* item) const;

    void      repaintItem(ListItem* item);

    void      sort();

    void      clear();

    void      ensureItemVisible(ListItem *item);

    int       itemHeight() const;

    QSize     sizeHint() const;
    
protected:

    void fontChange(const QFont & oldFont);
    void viewportPaintEvent(QPaintEvent *pe);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void drawFrameRaised(QPainter* p);
    void drawFrameSunken(QPainter* p);

    virtual void paintItemBase(QPainter* p, const QColorGroup& group,
                               const QRect& r, bool selected);

signals:

    void signalSelectionChanged(ListItem* item);
    void signalCleared();
    void signalRightButtonPressed(ListItem* item);
    void signalDoubleClicked(ListItem* item);

public slots:

    void slotUpdateContents();
    
private:

    void rearrangeItems();
    void layoutItem(ListItem* item, uint& pos, uint& count);
    void triggerUpdate();
    void sortChildItems(ListItem* parent);

    void takeItem(ListItem* item);
    
    void drawArrow(QPainter* p, const QRect& r, bool open,
                   bool selected);
    
    ListViewPriv* d;
    
    friend class ListItem;
};
    

#endif /* LISTVIEW_H */
