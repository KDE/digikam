/* ============================================================
 * File  : albummanager.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-09-22
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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
#include <qdict.h>

#include <kdirlister.h>
#include <kurl.h>
#include <klocale.h>
#include <kmessagebox.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
}

#include "albumxmlparser.h"
#include "albumxmleditor.h"
#include "albuminfo.h"
#include "albumsettings.h"
#include "albumitemhandler.h"
#include "albummanager.h"

// -------------------------------------------------------------------

namespace Digikam
{

class AlbumManagerPriv
{
public:

    AlbumManagerPriv() {

        dict.setAutoDelete(false);
        dict.clear();

        dirLister    = new KDirLister;
        dirLister->setDirOnlyMode(true);

        clearing     = false;
        firstAlbum   = 0;
        lastAlbum    = 0;
        currentAlbum = 0;
        xmlParser    = new AlbumXMLParser();
        itemHandler  = 0;
    }

    ~AlbumManagerPriv() {
        delete dirLister;
        delete xmlParser;
    }
    
    QDict<Digikam::AlbumInfo>    dict;
    QString             libraryPath;
    bool                clearing;

    AlbumInfo          *firstAlbum;
    AlbumInfo          *lastAlbum;
    AlbumInfo          *currentAlbum;
    
    KDirLister         *dirLister;
    AlbumXMLParser     *xmlParser;
    AlbumItemHandler   *itemHandler;
};

// -------------------------------------------------------------------

AlbumManager* AlbumManager::m_instance = 0;

AlbumManager* AlbumManager::instance()
{
    return m_instance;    
}

// -------------------------------------------------------------------

AlbumManager::AlbumManager(QObject *parent)
    : QObject(parent)
{
    m_instance     = this;
    d              = new AlbumManagerPriv;

    connect(d->dirLister, SIGNAL(newItems(const KFileItemList&)),
            this, SLOT(slotNewItems(const KFileItemList&)));
    connect(d->dirLister, SIGNAL(deleteItem(KFileItem*)),
            this, SLOT(slotDeleteItem(KFileItem*)));
    connect(d->dirLister, SIGNAL(clear()),
            this, SLOT(slotClearItems()));
}

AlbumManager::~AlbumManager()
{
    clearAlbums();
    
    delete d;
    m_instance = 0;
}

void AlbumManager::setLibraryPath(const QString& libraryPath)
{
    if (d->libraryPath == libraryPath) return;

    d->libraryPath = libraryPath;
    d->dirLister->stop();
    d->dirLister->openURL(KURL(libraryPath), false);
}

QString AlbumManager::getLibraryPath()
{
    return d->libraryPath;    
}

AlbumInfo* AlbumManager::findAlbum(const QString& name)
{
    return name.isEmpty() ? 0 : d->dict.find(name);
}

AlbumInfo* AlbumManager::firstAlbum()
{
    return d->firstAlbum;
}

AlbumInfo* AlbumManager::lastAlbum()
{
    return d->lastAlbum;
}

void AlbumManager::setCurrentAlbum(AlbumInfo *album)
{
    if (d->currentAlbum) {
        d->currentAlbum->db_->close();
    }

    d->currentAlbum = album;    

    if (d->currentAlbum)
        d->currentAlbum->db_->open();

    emit signalAlbumCurrentChanged(album);
}

AlbumInfo* AlbumManager::currentAlbum()
{
    return d->currentAlbum;
}

void AlbumManager::renameAlbum(AlbumInfo *album,
                               const QString& newTitle)
{
    if (!album || newTitle.isEmpty())
        return;

    // If we are changing the name of this album
    // close the xmlfile and reopen it again to sync it
    // to hard disk
    if (d->currentAlbum) {
        d->currentAlbum->db_->close();
        d->currentAlbum->db_->open();
    }

    KURL src(album->getPath());
    KURL dest(src.directory(false,true) + newTitle);

    QString error;
    if (!renameDirectory(src.path(), dest.path(),
                         error)) {
        KMessageBox::sorry(0, error);
    }
}


void AlbumManager::insertAlbum(AlbumInfo* album)
{
    if (!album) return;

    d->dict.replace(album->title_, album);

    if (!d->firstAlbum) {
        d->firstAlbum = album;
        d->lastAlbum  = album;
        album->prev_  = 0;
        album->next_  = 0;
    }
    else {
        d->lastAlbum->next_ = album;
        album->prev_  = d->lastAlbum;
        album->next_  = 0;
        d->lastAlbum  = album;
    }

    // Get the properties of the album if possible
    d->xmlParser->setAlbum(album);

    // At least set the date of the album if didn't get it from the
    // xml file
    if (album->getDate().isNull()) {
        QFileInfo dirInfo(album->getPath());
        album->setDate(dirInfo.created().date());
    }
    
    emit signalAlbumAdded(album);
}

void AlbumManager::takeAlbum(AlbumInfo* album)
{
    if (!album || d->clearing) return;    

    d->dict.remove(album->title_);
    
    if (album == d->firstAlbum) {
        d->firstAlbum = d->firstAlbum->next_;
        if (d->firstAlbum)
            d->firstAlbum->prev_ = 0;
        else
            d->firstAlbum = d->lastAlbum = 0;
    }
    else if (album == d->lastAlbum) {
        d->lastAlbum = d->lastAlbum->prev_;
        if ( d->lastAlbum )
            d->lastAlbum->next_ = 0;
        else
            d->firstAlbum = d->lastAlbum = 0;
    }
    else {
        AlbumInfo *i = album;
        if (i) {
            if (i->prev_ )
                i->prev_->next_ = i->next_;
            if ( i->next_ )
                i->next_->prev_ = i->prev_;
        }
    }

    if (album == d->currentAlbum) {
        d->currentAlbum = 0;
        emit signalAlbumCurrentChanged(0);
    }
    
    emit signalAlbumDeleted(album);
}

void AlbumManager::clearAlbums()
{
    if (d->currentAlbum)
        d->currentAlbum->db_->close();

    d->clearing = true;

    AlbumInfo *album = d->firstAlbum;
    while (album) {
        AlbumInfo *tmp = album->next_;
        delete album;
        album = tmp;
    }

    d->firstAlbum   = 0;
    d->lastAlbum    = 0;
    d->currentAlbum = 0;
    d->dict.clear();

    d->clearing = false;
}

void AlbumManager::addAlbum(const QString& name)
{
    AlbumInfo *album = findAlbum(name);
    if (album) return;

    (void) new AlbumInfo(this, name);
}

void AlbumManager::slotNewItems(const KFileItemList& itemList)
{
    KFileItem* item = 0;

    for (KFileItemListIterator it(itemList);
         (item = it.current()); ++it) {
        addAlbum(item->name());
    }
}

void AlbumManager::slotDeleteItem(KFileItem* item)
{
    if (!item) return;

    AlbumInfo *album = findAlbum(item->name());

    if (album) delete album;        

}
        
void AlbumManager::slotClearItems()
{
    clearAlbums();
    emit signalAlbumsCleared();
}

void AlbumManager::setItemHandler(AlbumItemHandler *handler)
{
    d->itemHandler = handler;    
}

AlbumItemHandler* AlbumManager::getItemHandler()
{
    return d->itemHandler;
}

void AlbumManager::refreshItemHandler(const QStringList& itemList)
{
    if (itemList.empty())
        d->itemHandler->refresh();
    else
        d->itemHandler->refreshItems(itemList);
}

void AlbumManager::emitAlbumItemsSelected(bool val)
{
    emit signalAlbumItemsSelected(val);
}

bool AlbumManager::renameDirectory(const QString& oldPath,
                                   const QString& newPath,
                                   QString& error)
{
    QString name  = KURL(oldPath).filename();
    
    struct stat info;

    int rc = stat(oldPath.latin1(), &info);
    if (rc != 0) {
        error = i18n("Album '%1' does not exist").arg(name);
        return false;;
    }

    rc = stat(newPath.latin1(), &info);
    if (rc == 0) {
        error = i18n("Album '%1' with same name present").
                arg(KURL(newPath).filename());
        return false;
    }

    rc = ::rename(oldPath.latin1(), newPath.latin1());
    if (rc != 0) {
        error = i18n("Failed to rename Album '%1'").arg(name);
        return false;
    }

    return true;
}


}
