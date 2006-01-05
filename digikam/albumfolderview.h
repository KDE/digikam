/* ============================================================
 * File  : albumfolderview.h
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-06
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
 
/** @file albumfoldeview.h */

#ifndef _ALBUMFOLDERVIEW_H_
#define _ALBUMFOLDERVIEW_H_

// KDE includes.

#include <kio/job.h>

// Local includes.

#include "folderview.h"

class QPixmap;
class QDataStream;

class KURL;

namespace Digikam
{

class Album;
class PAlbum;
class AlbumFolderViewPriv;
class AlbumFolderViewItem;

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
    
    void setAlbumThumbnail(PAlbum *album);    
    
private slots:

    void slotGotThumbnailFromIcon(const KURL& url, const QPixmap& thumbnail);
    void slotThumbnailLost(const KURL &url);    
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
    void albumDelete(AlbumFolderViewItem *item);

    AlbumFolderViewItem* findParent(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByFolder(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByCollection(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByDate(PAlbum* album, bool& failed);

    void reparentItem(AlbumFolderViewItem* folderItem);
    void clearEmptyGroupItems();
    
    AlbumFolderViewPriv   *d;
    
signals:
    void signalAlbumModified();
};

}  // namespace Digikam

#endif // _ALBUMFOLDEVIEW_H_
