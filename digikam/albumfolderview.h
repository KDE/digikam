/* ============================================================
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-06
 * Copyright 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 *
 * Description : Albums folder view.
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
 
/** @file albumfoldeview.h */

#ifndef _ALBUMFOLDERVIEW_H_
#define _ALBUMFOLDERVIEW_H_

// KDE includes.

#include <kio/job.h>

// Local includes.

#include "folderview.h"

class QPixmap;

class KURL;

namespace Digikam
{

class Album;
class PAlbum;
class AlbumFolderViewItem;
class AlbumFolderViewPriv;

class AlbumFolderView : public FolderView
{
    Q_OBJECT

public:

    AlbumFolderView(QWidget *parent);
    ~AlbumFolderView();

    void albumImportFolder();
    void resort();

    void albumNew();
    void albumDelete();
    void albumEdit();
    void albumRename();

    void setAlbumThumbnail(PAlbum *album);

signals:
    
    void signalAlbumModified();

private slots:

    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotSelectionChanged();

    void slotAlbumAdded(Album *);
    void slotAlbumDeleted(Album *album);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumRenamed(Album *album);

    void slotContextMenu(QListViewItem*, const QPoint&, int);

    void slotDIOResult(KIO::Job* job);

protected:

    void contentsDropEvent(QDropEvent *e);
    QDragObject* dragObject();
    bool acceptDrop(const QDropEvent *e) const;

    void selectItem(int id);

private:

    void albumNew(AlbumFolderViewItem *item);
    void albumEdit(AlbumFolderViewItem *item);
    void albumRename(AlbumFolderViewItem *item);
    void albumDelete(AlbumFolderViewItem *item);

    void addAlbumChildrenToList(KURL::List &list, Album *album);

    AlbumFolderViewItem* findParent(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByFolder(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByCollection(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByDate(PAlbum* album, bool& failed);

    void reparentItem(AlbumFolderViewItem* folderItem);
    void clearEmptyGroupItems();

private:

    AlbumFolderViewPriv   *d;

};

}  // namespace Digikam

#endif // _ALBUMFOLDEVIEW_H_
