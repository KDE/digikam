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

#include <qlistview.h>
#include <qptrlist.h>
#include <qguardedptr.h>

// KDE includes.

#include <kio/job.h>
#include <kurl.h>

class QDate;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QPixmap;
class KURL;

class AlbumFolderItem;

namespace Digikam
{
class AlbumInfo;
class AlbumManager;
class ThumbnailJob;
}

class AlbumFolderView : public QListView {

    Q_OBJECT

public:

    AlbumFolderView(QWidget *parent);
    ~AlbumFolderView();

    void applySettings();

    void albumNew();
    void albumDelete();
    void albumDelete(Digikam::AlbumInfo* album);

private:

    void resort();

    void reparentItem(AlbumFolderItem* folderItem);
    void reparentItemByDate(AlbumFolderItem* folderItem);
    void reparentItemByCollection(AlbumFolderItem* folderItem);
    void reparentItemFlat(AlbumFolderItem* folderItem);
    AlbumFolderItem* findParentByCollection(AlbumFolderItem* folderItem);
    AlbumFolderItem* findParentByDate(AlbumFolderItem* folderItem);

    void clearEmptyGroupItems();
    
    bool hasAlbum(const QString& path);

protected:

    void contentsDragEnterEvent(QDragEnterEvent*);
    void contentsDragMoveEvent(QDragMoveEvent*);
    void contentsDragLeaveEvent(QDragLeaveEvent*);
    void contentsDropEvent(QDropEvent*);

private:

    AlbumFolderItem*                   dropTarget_;
    int                                albumSortOrder_;
    QPtrList<AlbumFolderItem>          groupItems_;
    Digikam::AlbumManager*             albumMan_;
    QGuardedPtr<Digikam::ThumbnailJob> thumbJob_;
    KURL                               m_newAlbumURL;
    
public slots:

    void slot_albumPropsEdit(Digikam::AlbumInfo* album);
    void slot_albumHighlight(Digikam::AlbumInfo* album);
    void slotGotThumbnail(const KURL& url, const QPixmap& thumbnail);

private slots:

    void slot_selectionChanged();
    void slot_doubleClicked(QListViewItem* item);
    void slot_rightButtonClicked(QListViewItem* item,
                                 const QPoint& pos,
                                 int column);

    void slot_onAlbumDelete(KIO::Job* job);
    void slot_onAlbumCreate(KIO::Job* job);
    
    void slot_albumAdded(Digikam::AlbumInfo *album);
    void slot_albumDeleted(Digikam::AlbumInfo *album);
    void slot_albumsCleared();
    
};

#endif
