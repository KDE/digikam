/* ============================================================
 * File  : albumfolderview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-08
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <qstring.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qptrlist.h>
#include <qevent.h>
#include <qpoint.h>

#include <kdebug.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpropsdlg.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>

#include <interfaces/albummanager.h>
#include <interfaces/albuminfo.h>
#include <interfaces/thumbnailjob.h>
#include <interfaces/digikamio.h>

#include "thumbnailsize.h"
#include "albumfolderitem.h"
#include "albumfolderview.h"
#include "albumpropsedit.h"
#include "albumsettings.h"

#include "cameratype.h"
#include "cameradragobject.h"

AlbumFolderView::AlbumFolderView(QWidget *parent)
    : QListView(parent)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setSelectionMode(QListView::Single);

    dropTarget_     = 0;
    albumSortOrder_ = (int) AlbumSettings::ByCollection;
    groupItems_.setAutoDelete(false);
    groupItems_.clear();

    addColumn(i18n("My Albums"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    setItemMargin(5);

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slot_selectionChanged()));
    connect(this, SIGNAL(doubleClicked(QListViewItem*)),
            this, SLOT(slot_doubleClicked(QListViewItem*)));
    connect(this, SIGNAL(rightButtonPressed(QListViewItem*,
                                            const QPoint&, int)),
            this, SLOT(slot_rightButtonClicked(QListViewItem*,
                                               const QPoint&, int)));

    albumMan_ = Digikam::AlbumManager::instance();
    
    connect(albumMan_, SIGNAL(signalAlbumAdded(Digikam::AlbumInfo*)),
            this, SLOT(slot_albumAdded(Digikam::AlbumInfo*)));
    connect(albumMan_, SIGNAL(signalAlbumDeleted(Digikam::AlbumInfo*)),
            this, SLOT(slot_albumDeleted(Digikam::AlbumInfo*)));
    connect(albumMan_, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slot_albumsCleared()));
}

AlbumFolderView::~AlbumFolderView()
{
    if (!thumbJob_.isNull())
        thumbJob_->kill();
}

void AlbumFolderView::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    albumSortOrder_ = settings->getAlbumSortOrder();
    resort();
}



void AlbumFolderView::resort()
{
    QListViewItem* prevSelectedItem = selectedItem();

    for (Digikam::AlbumInfo *album = albumMan_->firstAlbum();
         album; album = album->nextAlbum()) {
        if (album->getViewItem()) {
            AlbumFolderItem *folderItem =
                static_cast<AlbumFolderItem*>(album->getViewItem());
            reparentItem(folderItem);
        }
    }
    
    // Clear any groupitems which have been
    // left empty
    clearEmptyGroupItems();

    if (prevSelectedItem) {
        setSelected(prevSelectedItem, true);
        ensureItemVisible(prevSelectedItem);
    }
}

void AlbumFolderView::reparentItem(AlbumFolderItem* folderItem)
{
    if (!folderItem || !folderItem->albumInfo())
        return;

    switch (albumSortOrder_) {

    case(AlbumSettings::Flat): {
        reparentItemFlat(folderItem);
        break;
    }

    case(AlbumSettings::ByDate): {
        reparentItemByDate(folderItem);
        break;
    }

    case(AlbumSettings::ByCollection): {
        reparentItemByCollection(folderItem);
        break;
    }

    default:
        break;
    }
}

void AlbumFolderView::reparentItemByDate(AlbumFolderItem* folderItem)
{
    if (!folderItem || !folderItem->albumInfo())
        return;
    
    QListViewItem *oldParent = folderItem->parent();
    QListViewItem* newParent = findParentByDate(folderItem);

    if (!newParent) {
        kdWarning() << "AlbumFolderView: Couldn't find new parent by Date: "
                    << folderItem->albumInfo()->getPath() << endl;
        return;
    }
    
    if (!oldParent) 
        // FolderItem was a top-level item
        takeItem(folderItem);
    else
        // Has a parent
        oldParent->takeItem(folderItem);

    // insert into new parent
    newParent->insertItem(folderItem);
}

void AlbumFolderView::reparentItemByCollection(AlbumFolderItem* folderItem)
{
    if (!folderItem || !folderItem->albumInfo())
        return;

    QListViewItem *oldParent = folderItem->parent();
    QListViewItem* newParent = findParentByCollection(folderItem);

    if (!newParent) {
        kdWarning() << "AlbumFolderView: Couldn't find new parent by collection: "
                    << folderItem->albumInfo()->getPath() << endl;
        return;
    }
    
    if (!oldParent) 
        // FolderItem was a top-level item
        takeItem(folderItem);
    else
        // Has a parent
        oldParent->takeItem(folderItem);

    // insert into new parent
    newParent->insertItem(folderItem);
}

void AlbumFolderView::reparentItemFlat(AlbumFolderItem* folderItem)
{
    if (!folderItem || !folderItem->albumInfo())
        return;

    QListViewItem *oldParent = folderItem->parent();

    // Already a toplevel item
    if (!oldParent) return;

    // Remove from old parent and insert
    // as toplevel item
    oldParent->takeItem(folderItem);
    insertItem(folderItem);
}

AlbumFolderItem* AlbumFolderView::findParentByDate(AlbumFolderItem* folderItem)
{
    if (!folderItem || !folderItem->albumInfo())
        return 0;
    
    QDate date = folderItem->albumInfo()->getDate();
    
    QString timeString = QString::number(date.year()) + ", " +
                         KGlobal::locale()->monthName(date.month());

    AlbumFolderItem* parentItem = 0;
    
    for (AlbumFolderItem* groupItem = groupItems_.first();
         groupItem; groupItem = groupItems_.next()) {
        if (groupItem->text(0) == timeString) {
            parentItem = groupItem;
            break;
        }
    }

    // Need to create a new parent item
    if (!parentItem) {
        parentItem = new AlbumFolderItem(this, timeString,
                                         date.year(), date.month());
        parentItem->setOpen(true);
        groupItems_.append(parentItem);
    }
    
    return parentItem;
}


AlbumFolderItem* AlbumFolderView::findParentByCollection(AlbumFolderItem* folderItem)
{
    if (!folderItem || !folderItem->albumInfo())
        return 0;

    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return 0;
    
    QStringList collectionList = settings->getAlbumCollectionNames();
    QString collection = folderItem->albumInfo()->getCollection();

    if (!collectionList.contains(collection))
        collection = i18n( "Unknown" );

    AlbumFolderItem* parentItem = 0;
    
    for (AlbumFolderItem* groupItem = groupItems_.first();
         groupItem; groupItem = groupItems_.next()) {
        if (groupItem->text(0) == collection) {
            parentItem = groupItem;
            break;
        }
    }

    // Need to create a new parent item
    if (!parentItem) {
        parentItem = new AlbumFolderItem(this, collection,
                                         0, 0);
        parentItem->setOpen(true);
        groupItems_.append(parentItem);
    }
    
    return parentItem;
}

void AlbumFolderView::clearEmptyGroupItems()
{
    AlbumFolderItem* groupItem = groupItems_.first();
    while (groupItem) {
        AlbumFolderItem* nextGroupItem = groupItems_.next();
        if (groupItem->childCount() == 0) {
            groupItems_.remove(groupItem);
            delete groupItem;
        }
        groupItem = nextGroupItem;
    }
}


void AlbumFolderView::slot_albumAdded(Digikam::AlbumInfo *album)
{
    if (!album) return;

    AlbumFolderItem* folderItem =
        new AlbumFolderItem(this, album);
    album->setViewItem(folderItem);
    reparentItem(folderItem);

    slot_albumHighlight(album);
}

void AlbumFolderView::slot_albumDeleted(Digikam::AlbumInfo *album)
{
    if (!album) return;

    if (!album->getViewItem())
        return;
    
    AlbumFolderItem* folderItem =
        static_cast<AlbumFolderItem*>(album->getViewItem());
    delete folderItem;

    clearEmptyGroupItems();
}

void AlbumFolderView::slot_albumsCleared()
{
    groupItems_.clear();
    clear();
}

void AlbumFolderView::albumNew()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) {
        kdWarning() << "AlbumFolderView: Couldn't get Album Settings" << endl;
        return;
    }

    QDir libraryDir(settings->getAlbumLibraryPath());
    if (!libraryDir.exists()) {
        KMessageBox::error(0,
                           i18n("Album Library has not been set correctly\n"
                                "Please run Setup"));
        return;
    }

    bool ok;
    QString newDir =
        KLineEditDlg::getText(i18n("Enter New Album Name: "),
                              "", &ok, this);
    if (!ok) return;

    KURL newAlbumURL(settings->getAlbumLibraryPath());
    newAlbumURL.addPath(newDir);

    KIO::SimpleJob* job = KIO::mkdir(newAlbumURL);
    connect(job, SIGNAL(result(KIO::Job*)),
            this, SLOT(slot_onAlbumCreate(KIO::Job*)));
}

void AlbumFolderView::slot_onAlbumCreate(KIO::Job* job)
{
    if (job->error()) {
        job->showErrorDialog(this);
    }
}


void AlbumFolderView::albumDelete()
{
    Digikam::AlbumInfo *album = albumMan_->currentAlbum();
    if (!album) return;

    albumDelete(album);
}

void AlbumFolderView::albumDelete(Digikam::AlbumInfo* album)
{
    if (!album) return;
    int result =
        KMessageBox::questionYesNo(0, i18n("Delete '%1' Album from HardDisk").arg(album->getTitle()));

    if (result == KMessageBox::Yes) {
        KIO::Job* job = KIO::del(album->getPath());
        connect(job, SIGNAL(result(KIO::Job*)),
                this, SLOT(slot_onAlbumDelete(KIO::Job*)));
    }
}

void AlbumFolderView::slot_onAlbumDelete(KIO::Job* job)
{
    if (job->error()) {
        job->showErrorDialog(this);
    }
}

void AlbumFolderView::slot_albumHighlight(Digikam::AlbumInfo* album)
{
    if (!album || !album->getViewItem()) {
        return;
    }

    AlbumFolderItem *folderItem =
        static_cast<AlbumFolderItem*>(album->getViewItem());

    if (folderItem->isGroupItem() || folderItem->isHighlighted()) {
        return;
    }

    if (thumbJob_.isNull()) {
        thumbJob_ = new Digikam::ThumbnailJob(KURL(album->getPath()),
                                              (int)ThumbnailSize::Tiny,
                                              true);
        connect(thumbJob_,
                SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));
    }
    else
        thumbJob_->addItem(KURL(album->getPath()));
}

void AlbumFolderView::slot_selectionChanged()
{
    QListViewItem* item = selectedItem();

    if (!item) {
        albumMan_->setCurrentAlbum(0);
        return;
    }

    AlbumFolderItem *folderItem
        = static_cast<AlbumFolderItem *>(item);

    if (folderItem->isGroupItem()) {
        albumMan_->setCurrentAlbum(0);
        return;
    }

    if (folderItem->albumInfo_) {
        albumMan_->setCurrentAlbum(
            const_cast<Digikam::AlbumInfo *>(folderItem->albumInfo_));
    }
}

void AlbumFolderView::slot_doubleClicked(QListViewItem* item)
{
     if (!item) return;

     AlbumFolderItem *folderItem
        = static_cast<AlbumFolderItem *>(item);

     if (folderItem->isGroupItem())
         return;

     Digikam::AlbumInfo* album = const_cast<Digikam::AlbumInfo *>(folderItem->albumInfo_);
     slot_albumPropsEdit(album);
}

void AlbumFolderView::slot_rightButtonClicked(QListViewItem* item,
                                              const QPoint&,
                                              int)
{
    if (!item) return;

     AlbumFolderItem *folderItem
         = static_cast<AlbumFolderItem *>(item);

     if (folderItem->isGroupItem())
         return;

     QPopupMenu popmenu(this);
     popmenu.insertItem(SmallIcon("pencil"),
                        i18n("Edit Album Properties"), 10);
     popmenu.insertItem(SmallIcon("edittrash"),
                        i18n("Delete Album from HardDisk"), 11);

     int id = popmenu.exec(QCursor::pos());

     switch(id) {

     case 10: {
         Digikam::AlbumInfo* album = const_cast<Digikam::AlbumInfo *>(folderItem->albumInfo_);
         slot_albumPropsEdit(album);
         break;
     }

     case 11: {
         Digikam::AlbumInfo *album =
             const_cast<Digikam::AlbumInfo *>(folderItem->albumInfo());
         albumDelete(album);
         break;
     }

     default:
        break;
    }

}


void AlbumFolderView::slot_albumPropsEdit(Digikam::AlbumInfo* album)
{
    if (!album || !album->getViewItem()) return;

    QString     oldTitle(album->getTitle());
    QString     oldComments(album->getComments());
    QString     oldCollection(album->getCollection());
    QDate       oldDate(album->getDate());
    QStringList oldAlbumCollections(AlbumSettings::instance()->getAlbumCollectionNames());

    QString     title, comments, collection;
    QDate       date;
    QStringList albumCollections;

    if (AlbumPropsEdit::editProps(album, title, comments,
                                  date, collection,
                                  albumCollections)) {

        if (comments != oldComments)
            album->setComments(comments);

        if (date != oldDate && date.isValid()) 
            album->setDate(date);

        if (collection != oldCollection)
            album->setCollection(collection);

        AlbumSettings::instance()->setAlbumCollectionNames(albumCollections);
        resort();

        // Do this last : so that if anything else changed we can
        // successfully save the db and reopen it
        if (title != oldTitle) {
            albumMan_->renameAlbum(album, title);
        }

    }
}

// Drag and Drop -----------------------------------------

void AlbumFolderView::contentsDragEnterEvent(QDragEnterEvent*)
{
    // override the default drag enter event avoid selection problems
    // in case of a dropevent
    return;    
}

void AlbumFolderView::contentsDragMoveEvent(QDragMoveEvent* event)
{
    if (!QUriDrag::canDecode(event) &&
        !CameraDragObject::canDecode(event)) {
        event->ignore();
        return;
    }

    // Get a pointer to the new drop item
    QPoint point(0, event->pos().y());
    AlbumFolderItem* newDropTarget =
        static_cast<AlbumFolderItem*>(
            itemAt(contentsToViewport(point)) );
    if (!newDropTarget || newDropTarget->isGroupItem()) {
        event->ignore();
        return;
    }

    event->accept();
    if (dropTarget_ == newDropTarget) return;

    if (dropTarget_)
        dropTarget_->removeDropHighlight();


    dropTarget_ = newDropTarget;
    dropTarget_->addDropHighlight();

}

void AlbumFolderView::contentsDragLeaveEvent(QDragLeaveEvent*)
{
    if (dropTarget_)
        dropTarget_->removeDropHighlight();
    dropTarget_ = 0;
}

void AlbumFolderView::contentsDropEvent(QDropEvent* event)
{
    if (!dropTarget_) return;

    if (QUriDrag::canDecode(event)) {

        Digikam::AlbumInfo *destAlbum =
            const_cast<Digikam::AlbumInfo*>(dropTarget_->albumInfo());
        KURL destURL = KURL(destAlbum->getPath());

        KURL::List srcURLs;
        KURLDrag::decode(event, srcURLs);

        KURL firstSrcURL(srcURLs.first());
        Digikam::AlbumInfo* srcAlbum =
            albumMan_->findAlbum(firstSrcURL.upURL().filename());

        QPopupMenu popMenu(this);
        popMenu.insertItem( i18n("&Copy"), 10 );
        popMenu.insertItem( i18n("&Move"), 11 );

        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());
        switch(id) {
        case 10: {

            if (!srcAlbum) {
                KIO::copy(srcURLs,destURL,true);
            }
            else {

                QStringList fileList;
                KURL::List::iterator it;
                for ( it = srcURLs.begin(); it != srcURLs.end(); ++it )
                    fileList.append((*it).filename());
                
                DigikamIO::copy(srcAlbum, destAlbum, fileList);
            }
            
            break;
        }
        case 11: {

            if (!srcAlbum) {
                KIO::move(srcURLs,destURL,true);
            }
            else {

                QStringList fileList;
                KURL::List::iterator it;
                for ( it = srcURLs.begin(); it != srcURLs.end(); ++it )
                    fileList.append((*it).filename());
                
                DigikamIO::move(srcAlbum, destAlbum, fileList);
            }

            break;
        }
        default:
            break;
        }
    }
    else if (CameraDragObject::canDecode(event)) {

        QPopupMenu popMenu(this);
        popMenu.insertItem( i18n("&Download"), 10 );
        popMenu.setMouseTracking(true);

        int id = popMenu.exec(QCursor::pos());
        switch(id) {
        case 10: {

            CameraType ctype;
            CameraDragObject::decode(event, ctype);

            QByteArray arg1;
            QDataStream stream1(arg1, IO_WriteOnly);
            stream1 << dropTarget_->albumInfo()->getTitle();

            DCOPClient *client = kapp->dcopClient();
            client->send("digikamcameraclient", "DigikamCameraClient",
                         "cameraChangeDownloadAlbum(QString)",
                         arg1);
            
            QByteArray arg2;

            client->send("digikamcameraclient", "DigikamCameraClient",
                         "cameraDownloadSelected()",
                         arg2);

            break;
        }
        default:
            break;
        }

    }

    dropTarget_->removeDropHighlight();
    dropTarget_ = 0;
}

void AlbumFolderView::slotGotThumbnail(const KURL& url, const QPixmap& thumbnail)
{
    QString title(url.fileName());
    if (title.isEmpty()) return;

    Digikam::AlbumInfo *album =
        Digikam::AlbumManager::instance()->findAlbum(title);
    if (!album) return;

    AlbumFolderItem *folderItem =
        static_cast<AlbumFolderItem*>(album->getViewItem());
    folderItem->setPixmap(thumbnail);
}
