/* ============================================================
 * Authors: Joern Ahrens <joern.ahrens@kdemail.net>
 *          Caulier Gilles 
 * Date   : 2005-03-22
 * Copyright 2005-2006 by Joern Ahrens
 * Copyright 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
class TagFolderViewItem;
class TagFolderViewPriv;

class TagFolderView : public FolderView
{
    Q_OBJECT

public:

    TagFolderView(QWidget *parent);
    ~TagFolderView();

    void tagNew();
    void tagEdit();    
    void tagDelete();

signals:

    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);

protected:

    void contentsDropEvent(QDropEvent *e);
    QDragObject* dragObject();
    bool acceptDrop(const QDropEvent *e) const;

    void selectItem(int id);

private slots:

    void slotAlbumAdded(Album *);
    void slotSelectionChanged();
    void slotAlbumDeleted(Album*);
    void slotAlbumRenamed(Album*);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumMoved(TAlbum* tag, TAlbum* newParent);
    void slotContextMenu(QListViewItem*, const QPoint&, int);
    void slotABCContextMenu();
    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);

private:

    void tagNew(TagFolderViewItem *item, const QString& _title=QString(),
                const QString& _icon=QString() );
    void tagEdit(TagFolderViewItem *item);
    void tagDelete(TagFolderViewItem *item);
    void setTagThumbnail(TAlbum *album);

private:

    TagFolderViewPriv *d;
};

}  // namespace Digikam

#endif // _TAGFOLDEVIEW_H_
