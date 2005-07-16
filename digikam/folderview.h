/* ============================================================
 * File  : folderview.h
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-28
 * Copyright 2005 by Joern Ahrens
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

/** @file foldeview.h */

#ifndef _FOLDERVIEW_H_
#define _FOLDERVIEW_H_

#include <qlistview.h>

class FolderViewPriv;
class FolderItem;

/**
 * \class FolderView
 * \brief Base class for a tree view
 */


class FolderView : public QListView
{
    Q_OBJECT

public:

    FolderView(QWidget *parent);
    virtual ~FolderView();

    void setActive(bool val);
    bool active() const;

    int      itemHeight() const;
    QRect    itemRect(QListViewItem *item) const;
    QPixmap  itemBasePixmapRegular() const;
    QPixmap  itemBasePixmapSelected() const;
    
    /**
     * load the last state from the view from disk
     */
    virtual void loadViewState(QDataStream &stream);
    
    /**
     * writes the views state to disk
     */
    virtual void saveViewState(QDataStream &stream);
    
protected:

    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDragLeaveEvent(QDragLeaveEvent * e);
    void contentsDropEvent(QDropEvent *e);
            
    virtual bool acceptDrop(const QDropEvent *e) const;
        
    void startDrag();
    FolderItem* dragItem() const;
    
    void resizeEvent(QResizeEvent* e);
    void fontChange(const QFont& oldFont);
    
    virtual void selectItem(int id);
 
protected slots:
    
    virtual void slotSelectionChanged();

private slots:

    void slotThemeChanged();
    
private:

    bool mouseInItemRect(QListViewItem* item, int x) const;

    FolderViewPriv      *d;
};

#endif // _FOLDERVIEW_H
