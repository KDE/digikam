/* ============================================================
 * File  : tagfolderview.h
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-03-22
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

/** @file tagfoldeview.h */

#ifndef _TAGFOLDERVIEW_H_
#define _TAGFOLDERVIEW_H_

#include "folderview.h"

class Album;
class TagFolderViewPriv;
class TagFolderViewItem;
class QDropEvent;

class TagFolderView : public FolderView
{
    Q_OBJECT

public:

    TagFolderView(QWidget *parent);
    ~TagFolderView();

    void tagNew();
    void tagEdit();    
    void tagDelete();

protected:

    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e); 
    void contentsDropEvent(QDropEvent *e);
    QDragObject* dragObject();
    
private slots:

    void slotAlbumAdded(Album *);
    void slotSelectionChanged();
    void slotAlbumDeleted(Album*);
    void slotAlbumsCleared();
    void slotContextMenu(QListViewItem*, const QPoint&, int);
    
private:

    void tagNew(TagFolderViewItem *item);
    void tagEdit(TagFolderViewItem *item);    
    void tagDelete(TagFolderViewItem *item);

    TagFolderViewPriv   *d;
};


#endif // _TAGFOLDEVIEW_H_
