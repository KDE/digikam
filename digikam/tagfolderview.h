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
 *
 * ============================================================ */

/** @file tagfoldeview.h */

#ifndef _TAGFOLDERVIEW_H_
#define _TAGFOLDERVIEW_H_

// Local includes.

#include "folderview.h"

class QDropEvent;

namespace Digikam
{

class Album;
class TAlbum;
class TagFolderViewPriv;
class TagFolderViewItem;

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

    void contentsDropEvent(QDropEvent *e);
    QDragObject* dragObject();
    bool acceptDrop(const QDropEvent *e) const;
    
    void selectItem(int id);    

signals:

    void signalTagsAssigned();
    
private slots:

    void slotAlbumAdded(Album *);
    void slotSelectionChanged();
    void slotAlbumDeleted(Album*);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumMoved(TAlbum* tag, TAlbum* newParent);
    void slotContextMenu(QListViewItem*, const QPoint&, int);
    void slotABCContextMenu();

private:

    void tagNew(TagFolderViewItem *item,
                const QString& _title=QString(),
                const QString& _icon=QString() );
    void tagEdit(TagFolderViewItem *item);
    void tagDelete(TagFolderViewItem *item);

    TagFolderViewPriv   *d;
};

}  // namespace Digikam

#endif // _TAGFOLDEVIEW_H_
