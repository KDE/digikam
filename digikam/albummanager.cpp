/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-15
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kdebug.h>
#include <kdirlister.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/global.h>

#include <qfile.h>
#include <qdir.h>
#include <qdict.h>
#include <qintdict.h>

#include <config.h>

#include "album.h"
#include "albumdb.h"
#include "albumitemhandler.h"
#include "syncjob.h"
#include "albummanager.h"

extern "C"
{
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
}

typedef QDict<PAlbum>    PAlbumDict;
typedef QIntDict<PAlbum> PAlbumIntDict;
typedef QIntDict<TAlbum> TAlbumIntDict;

class AlbumManagerPriv
{
public:
    
    AlbumDB          *db;
    AlbumItemHandler *itemHandler;
    
    KDirLister       *albumLister;
    QString           libraryPath;

    PAlbum           *rootPAlbum;
    TAlbum           *rootTAlbum;

    PAlbumList        pAlbumList;
    TAlbumList        tAlbumList;

    PAlbumDict        pAlbumDict;
    PAlbumIntDict     pAlbumIntDict;
    TAlbumIntDict     tAlbumIntDict;

    Album            *currentAlbum;
    
    KURL::List        urlsToOpen;
    bool              initialLoad;
};
    

AlbumManager* AlbumManager::m_instance = 0;

AlbumManager* AlbumManager::instance()
{
    return m_instance;
}

AlbumManager::AlbumManager()
{
    m_instance    = this;

    d = new AlbumManagerPriv;
    
    d->db = new AlbumDB;

    d->rootPAlbum = 0;
    d->rootTAlbum = 0;

    d->itemHandler  = 0;
    d->currentAlbum = 0;

    d->albumLister  = 0;
    
    d->initialLoad  = true;
}

AlbumManager::~AlbumManager()
{
    if (d->albumLister)
        delete d->albumLister;

    if (d->rootPAlbum)
    {
        KFileItem *fileItem = d->rootPAlbum->fileItem();
        if (fileItem)
        {
            delete fileItem;
        }
        delete d->rootPAlbum;
    }
    if (d->rootTAlbum)
        delete d->rootTAlbum;

    delete d->db;
    delete d;

    m_instance = 0;
}


AlbumDB* AlbumManager::albumDB()
{
    return d->db;    
}

void AlbumManager::setLibraryPath(const QString& path)
{
    if (KURL(path).equals(d->libraryPath))
        return;
    
    createAlbumLister();

    d->currentAlbum = 0;
    emit signalAlbumCurrentChanged(0);
    emit signalAlbumsCleared();
    
    d->pAlbumDict.clear();
    d->pAlbumIntDict.clear();
    d->tAlbumIntDict.clear();
    d->pAlbumList.clear();
    d->tAlbumList.clear();
    
    if (d->rootPAlbum)
    {
        KFileItem *fileItem = d->rootPAlbum->fileItem();
        if (fileItem)
        {
            delete fileItem;
        }
        delete d->rootPAlbum;
        d->rootPAlbum = 0;
    }

    if (d->rootTAlbum)
    {
        delete d->rootTAlbum;
        d->rootTAlbum = 0;
    }

    d->libraryPath = path;

    QString dbPath = path + "/digikam.db";

#ifdef NFS_HACK
    dbPath = locateLocal("appdata", KIO::encodeFileName(QDir::cleanDirPath(dbPath)));
#endif

    kdDebug() << "Album libray path set to " << dbPath << endl;
    
    d->db->setDBPath(dbPath);

    d->albumLister->openURL(path, true);

    KURL u(path);
    u.cleanPath();
    u.adjustPath(-1);

    KFileItem* fileItem = new KFileItem(KFileItem::Unknown,
                                        KFileItem::Unknown,
                                        u);
    d->rootPAlbum = new PAlbum(fileItem, i18n("My Albums"), 0, true);
    insertPAlbum(d->rootPAlbum);

    d->rootTAlbum = new TAlbum(i18n("My Tags"), 0, true);
    insertTAlbum(d->rootTAlbum);

    d->db->scanTags(d->rootTAlbum);
}

QString AlbumManager::getLibraryPath() const
{
    return d->libraryPath;    
}

void AlbumManager::addPAlbum(KFileItem* fileItem)
{
    KURL u(fileItem->url());
    u.cleanPath();
    u.adjustPath(-1);
    
    PAlbum* album = new PAlbum(fileItem, u.fileName(), -1);

    u = u.upURL();
    u.adjustPath(-1);
    PAlbum *parent = d->pAlbumDict.find(u.url());

    if (parent)
    {
        album->setParent(parent);
    }
    else
    {
        kdWarning() << k_funcinfo <<  "Could not find parent for "
                    << fileItem->url().prettyURL() << endl;
        return;
    }

    d->db->readAlbum(album);
    
    insertPAlbum(album);

    d->urlsToOpen.append(fileItem->url());
    if (d->albumLister->isFinished())
        slotCompleted();
}

void AlbumManager::slotNewItems(const KFileItemList& itemList)
{
    d->db->beginTransaction();
    
    KFileItem* item = 0;
    for (KFileItemListIterator it(itemList);
         (item = it.current()) != 0; ++it)
    {
        addPAlbum(item);
    }

    d->db->commitTransaction();
}

void AlbumManager::slotDeleteItem(KFileItem* item)
{
    KURL url(item->url());
    url.cleanPath();
    url.adjustPath(-1);

    PAlbum *album = d->pAlbumDict.find(url.url());
    if (!album)
        return;

    removePAlbum(album);
    delete album;
}

void AlbumManager::slotClearItems()
{
    emit signalAlbumsCleared();
    // todo:
    // clear all albums
}

PAlbumList AlbumManager::pAlbums() const
{
    return d->pAlbumList;
}

TAlbumList AlbumManager::tAlbums() const
{
    return d->tAlbumList;
}

bool AlbumManager::createPAlbum(PAlbum* parent,
                                const QString& name,
                                const QString& caption,
                                const QDate& date,
                                const QString& collection,
                                QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for album.");
        return false;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Album name cannot be empty.");
        return false;
    }
    
    if (name.contains("/"))
    {
        errMsg = i18n("Album name cannot contain '/'.");
        return false;
    }
    
    // first check if we have another album with the same name
    Album *child = parent->m_firstChild;
    while (child)
    {
        if (child->getTitle() == name)
        {
            errMsg = i18n("Another album with same name exists.");
            return false;
        }
        child = child->m_next;
    }

    KURL url = parent->getKURL();
    url.addPath(name);
    url.cleanPath();

    // make the directory synchronously, so that we can add the
    // album info to the database directly
    if (::mkdir(QFile::encodeName(url.path(-1)), 0777) != 0)
    {
        if (errno == EACCES)
            errMsg = i18n("Access denied to path");
        else if (errno == ENOSPC)
            errMsg = i18n("Disk full");
        else
            errMsg = i18n("Unknown error"); // being lazy

        return false;
    }

    // Now insert the album properties into the database

    QString u(QDir::cleanDirPath(url.path()));
    u.remove(0, QDir::cleanDirPath(d->libraryPath).length());
    if (!u.startsWith("/"))
        u.prepend("/");

    d->db->addPAlbum(u, caption, date, collection);

    return true;
}

bool AlbumManager::deletePAlbum(PAlbum* album, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootPAlbum )
    {
        errMsg = i18n("Cannot delete Root Album");
        return false;
    }

    if (SyncJob::userDelete(album->getKURL()))
    {
        d->db->deleteAlbum(album);
        return true;
    }
    
    return false;
}

bool AlbumManager::renamePAlbum(PAlbum* album, const QString& newName, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootPAlbum)
    {
        errMsg = i18n("Cannot rename root album");
        return false;
    }

    if (newName.contains("/"))
    {
        errMsg = i18n("Album name cannot contain '/'");
        return false;
    }
    
    // first check if we have another sibling with the same name
    Album *sibling = album->m_parent->m_firstChild;
    while (sibling)
    {
        if (sibling->getTitle() == newName)
        {
            errMsg = i18n("Another Album with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    KURL newURL = album->getKURL();
    newURL = newURL.upURL();
    newURL.addPath(newName);
    newURL.cleanPath();
    newURL.adjustPath(-1);

    kdDebug() << "Renaming : " << album->getKURL().path(-1) << " to "
              << newURL.path(-1) << endl;
    
    if (::rename(QFile::encodeName(album->getKURL().path(-1)),
                 QFile::encodeName(newURL.path(-1))) != 0)
    {
        errMsg = i18n("Failed to rename Album");
        return false;
    }

    // now rename the album and subalbums in the database

    // all we need to do is set the title of the album which is being
    // renamed correctly and all the sub albums will automatically get
    // their url (not KURL) set correctly

    album->setTitle(newName);
    d->db->renameAlbum(album, newName);
    
    AlbumIterator it(album);
    while ( it.current() )
    {
        d->db->renameAlbum(it.current(), QString(""));
        ++it;
    }
    
    return true;
}

bool AlbumManager::createTAlbum(TAlbum* parent, const QString& name,
                                const QString& icon, QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for tag");
        return false;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Tag name cannot be empty");
        return false;
    }
    
    if (name.contains("/"))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return false;
    }
    
    // first check if we have another album with the same name
    Album *child = parent->m_firstChild;
    while (child)
    {
        if (child->getTitle() == name)
        {
            errMsg = i18n("Another tag with same name exists");
            return false;
        }
        child = child->m_next;
    }

    if (!d->db->createTAlbum(parent, name, icon))
        return false;
    
    return true;
}

bool AlbumManager::deleteTAlbum(TAlbum* album, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot delete Root Tag");
        return false;
    }

    d->db->deleteAlbum(album);
    removeTAlbum(album);

    delete album;
    
    return true;
}

bool AlbumManager::renameTAlbum(TAlbum* album, const QString& name,
                                QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    if (name.contains("/"))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name
    Album *sibling = album->m_parent->m_firstChild;
    while (sibling)
    {
        if (sibling->getTitle() == name)
        {
            errMsg = i18n("Another tag with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    d->db->renameAlbum(album, name);
    
    return true;
}

bool AlbumManager::updateTAlbumIcon(TAlbum* album, const QString& icon, 
                                    bool emitSignalChanged, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }
    
    d->db->setIcon(album, icon);
    
    if(emitSignalChanged)    
        emit signalTAlbumIconChanged(album);        
    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum *parent, QString &errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot move root tag");
        return false;
    }
    
    d->db->moveTAlbum(album, parent);
    album->getParent()->removeChild(album);
    album->setParent(parent);
    album->setPID(parent->getID());    
    
    return true;
}

void AlbumManager::insertPAlbum(PAlbum *album)
{
    if (!album)
        return;

    KURL url(album->getKURL().url());
    url.cleanPath();
    url.adjustPath(-1);
    
    d->pAlbumList.append(album);
    d->pAlbumDict.insert(url.url(), album);    
    d->pAlbumIntDict.insert(album->getID(), album);

    emit signalAlbumAdded(album);
}

void AlbumManager::removePAlbum(PAlbum *album)
{
    if (!album)
        return;

    // remove all children of this album
    Album* child = album->m_firstChild;
    while (child)
    {
        Album *next = child->m_next;
        removePAlbum((PAlbum*)child);
        child = next;
    }
    
    KURL url(album->getKURL().url());
    url.cleanPath();
    url.adjustPath(-1);
    
    d->pAlbumList.remove(album);
    d->pAlbumDict.remove(url.url());    

    if (album == d->currentAlbum)
    {
        d->currentAlbum = 0;
        emit signalAlbumCurrentChanged(0);
    }
    
    emit signalAlbumDeleted(album);
}

bool AlbumManager::updatePAlbumIcon(PAlbum *album, const QString& icon, 
                                    bool emitSignalChanged, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootPAlbum)
    {
        errMsg = i18n("Cannot edit root album");
        return false;
    }

    d->db->setIcon(album, icon);

    if(emitSignalChanged)
        emit signalPAlbumIconChanged(album);    
    return true;
}

void AlbumManager::insertTAlbum(TAlbum *album)
{
    if (!album)
        return;

    d->tAlbumList.append(album);
    d->tAlbumIntDict.insert(album->getID(), album);    

    emit signalAlbumAdded(album);
}

void AlbumManager::removeTAlbum(TAlbum *album)
{
    if (!album)
        return;

    // remove all children of this album
    Album* child = album->m_firstChild;
    while (child)
    {
        Album *next = child->m_next;
        removeTAlbum((TAlbum*)child);
        child = next;
    }
    
    d->tAlbumList.remove(album);
    d->tAlbumIntDict.remove(album->getID());    

    if (album == d->currentAlbum)
    {
        d->currentAlbum = 0;
        emit signalAlbumCurrentChanged(0);
    }
    
    emit signalAlbumDeleted(album);
}

void AlbumManager::slotCompleted()
{
    if (d->urlsToOpen.isEmpty())
    {
        if (d->initialLoad)
        {
            d->initialLoad = false;
            emit signalAllAlbumsLoaded();
        }
        return;
    }

    KURL url(d->urlsToOpen.first());
    d->urlsToOpen.pop_front();

    d->albumLister->openURL(url, true);
}

void AlbumManager::slotRedirection(const KURL& oldURL, const KURL& newURL)
{
    kdDebug() << oldURL.prettyURL() << " redirected to "
              << newURL.prettyURL() << endl;
}

void AlbumManager::emitAlbumItemsSelected(bool val)
{
    emit signalAlbumItemsSelected(val);    
}

void AlbumManager::setItemHandler(AlbumItemHandler *handler)
{
    d->itemHandler = handler;    
}

AlbumItemHandler* AlbumManager::getItemHandler()
{
    return d->itemHandler;
}

void AlbumManager::refreshItemHandler(const KURL::List& itemList)
{
    if (itemList.empty())
        d->itemHandler->refresh();
    else
        d->itemHandler->refreshItems(itemList);
}

void AlbumManager::setCurrentAlbum(Album *album)
{
    d->currentAlbum = album;
    emit signalAlbumCurrentChanged(album);
}

Album* AlbumManager::currentAlbum() const
{
    return d->currentAlbum;
}

PAlbum* AlbumManager::findPAlbum(const KURL& url) const
{
    KURL u(url);
    u.cleanPath();
    u.adjustPath(-1);

    return d->pAlbumDict.find(u.url());
}

PAlbum* AlbumManager::findPAlbum(int id) const
{
    return d->pAlbumIntDict.find(id);    
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    return d->tAlbumIntDict.find(id);
}

void AlbumManager::createAlbumLister()
{
    if (d->albumLister)
        delete d->albumLister;
    d->albumLister = 0;
    
    d->albumLister = new KDirLister;
    d->albumLister->setDirOnlyMode(true);

    connect(d->albumLister, SIGNAL(newItems(const KFileItemList&)),
            SLOT(slotNewItems(const KFileItemList&)));
    connect(d->albumLister, SIGNAL(deleteItem(KFileItem*)),
            SLOT(slotDeleteItem(KFileItem*)));
    connect(d->albumLister, SIGNAL(clear()),
            SLOT(slotClearItems()));
    connect(d->albumLister, SIGNAL(completed()),
            SLOT(slotCompleted()));
    connect(d->albumLister, SIGNAL(redirection(const KURL&, const KURL&)),
            SLOT(slotRedirection(const KURL&, const KURL&)));
}

#include "albummanager.moc"

/*
  struct stat statBuf;
  QString filename = _url.directory( false, true ) + _url.fileName();
      
  if(::stat( QFile::encodeName( filename ), &statBuf ) == 0 )
  {
  // Count of hard links
  int hardLinks = statBuf.st_nlink;  

  // If the link count is > 2, the directory likely has subdirs.
  if( hardLinks > 2 ) // "Normal" directory with subdirs
  {
  d->urlsToOpen.append(_url);
  }
  }
*/

