//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMFOLDERVIEW.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef ALBUMFOLDERVIEW_H
#define ALBUMFOLDERVIEW_H

// Qt includes.

#include "listview.h"
#include <qptrlist.h>
#include <qguardedptr.h>
#include <qpixmap.h>
#include <qmap.h>

// KDE includes.

#include <kio/job.h>

class QDate;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTimer;
class KURL;
class KFileItem;
class KFileMetaInfo;

class ListItem;
class Album;
class PAlbum;
class TAlbum;
class AlbumFolderItem;
class AlbumManager;
class ThumbnailJob;

class AlbumFolderView : public ListView 
{
    Q_OBJECT

public:

    AlbumFolderView(QWidget *parent);
    ~AlbumFolderView();

    void applySettings();

    void albumNew();
    void albumNew(PAlbum* parent);
    void albumDelete();
    void albumDelete(PAlbum* album);
    void albumEdit(PAlbum* album);
    void albumHighlight(PAlbum* album);
    void albumImportFolder();
    
    void tagNew();
    void tagNew(TAlbum* album);
    void tagDelete();
    void tagDelete(TAlbum* album);
    void tagEdit();
    void tagEdit(TAlbum* album);

    void setInFocus(bool val);
    
private:

    void resort();

    void reparentItem(AlbumFolderItem* folderItem);

    AlbumFolderItem* findParent(Album *album);
    AlbumFolderItem* findParentByFolder(Album *album);
    AlbumFolderItem* findParentByCollection(PAlbum *album);
    AlbumFolderItem* findParentByDate(PAlbum *album);

    void clearEmptyGroupItems();

    QPixmap getBlendedIcon(TAlbum* album) const;

    void contextMenuPAlbum(PAlbum* album);
    void contextMenuTAlbum(TAlbum* album);

    void phyAlbumDropEvent(QDropEvent* e, PAlbum *album);
    void tagAlbumDropEvent(QDropEvent* e, TAlbum *album);
    void clearDropTarget();

    void loadAlbumState();
    void saveAlbumState();
    
protected:

    void contentsDragEnterEvent(QDragEnterEvent*);
    void contentsDragMoveEvent(QDragMoveEvent*);
    void contentsDragLeaveEvent(QDragLeaveEvent*);
    void contentsDropEvent(QDropEvent*);
    
    virtual void contentsMousePressEvent(QMouseEvent *e);    
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    void leaveEvent(QEvent*);
    virtual void startDrag();
    
    void resizeEvent(QResizeEvent* e);
    void focusInEvent(QFocusEvent* e);
    void drawFrame(QPainter* p);

    void paintItemBase(QPainter* p, const QColorGroup& group,
                       const QRect& r, bool selected);
    
private:

    AlbumFolderItem*                   dropTarget_;
    int                                albumSortOrder_;
    QPtrList<AlbumFolderItem>          groupItems_;
    AlbumManager*                      albumMan_;
    QGuardedPtr<ThumbnailJob>          iconThumbJob_;    
    AlbumFolderItem*                   phyRootItem_;
    AlbumFolderItem*                   tagRootItem_;

    QPixmap                            itemRegPix_;
    QPixmap                            itemSelPix_;

    QPoint                             dragStartPos_;
    ListItem                           *dragItem_;
    QTimer                             *openAlbumTimer_;    
    
    QMap<int,int>                      stateAlbumOpen_;
    int                                stateAlbumSel_;
    bool                               stateInitialLoading_;

    bool                               inFocus_;
    
signals:

    void signalTagsAssigned();
    void signalInFocus();
    void signalAlbumModified();
    
private slots:

    void slotSelectionChanged(ListItem *item);
    void slotDoubleClicked(ListItem* item);
    void slotRightButtonClicked(ListItem* item);

    void slotNewAlbumCreated(Album* album);
        
    void slotAlbumAdded(Album *album);
    void slotAlbumDeleted(Album *album);
    void slotAlbumsCleared();
    void slotAllAlbumsLoaded();
    void slotAlbumImportResult(KIO::Job* job);

    void slotOpenAlbumFolderItem();
        
    void slotGotThumbnailFromIcon(const KURL& url, const QPixmap& thumbnail,
                                  const KFileMetaInfo*);
    void slotThumbnailLost(const KURL&, bool isDir);

    void slotPAlbumIconChanged(PAlbum*);    
    void slotTAlbumIconChanged(TAlbum*);    
    
    void slotThemeChanged();
};

#endif  // ALBUMFOLDERVIEW_H
