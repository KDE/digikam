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

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kio/job.h>

#include <qfile.h>
#include <qdir.h>
#include <qdict.h>
#include <qintdict.h>
#include <qcstring.h>

#include <config.h>

#include "album.h"
#include "albumdb.h"
#include "albumitemhandler.h"
#include "syncjob.h"
#include "albumsettings.h"
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
    
    QString           libraryPath;

    PAlbum           *rootPAlbum;
    TAlbum           *rootTAlbum;
    DAlbum           *rootDAlbum;

    PAlbumList        pAlbumList;
    TAlbumList        tAlbumList;
    DAlbumList        dAlbumList;

    PAlbumDict        pAlbumDict;
    PAlbumIntDict     pAlbumIntDict;
    TAlbumIntDict     tAlbumIntDict;

    Album            *currentAlbum;

    KIO::TransferJob *dateListJob;
    QByteArray        buffer;
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
    d->dateListJob = 0;

    d->rootPAlbum = 0;
    d->rootTAlbum = 0;
    d->rootDAlbum = 0;

    d->itemHandler  = 0;
    d->currentAlbum = 0;
}

AlbumManager::~AlbumManager()
{
    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }
    
    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;

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

    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }
    
    d->currentAlbum = 0;
    emit signalAlbumCurrentChanged(0);
    emit signalAlbumsCleared();
    
    d->pAlbumDict.clear();
    d->pAlbumIntDict.clear();
    d->tAlbumIntDict.clear();
    d->pAlbumList.clear();
    d->tAlbumList.clear();
    d->dAlbumList.clear();

    if (d->rootPAlbum)
    {
        delete d->rootPAlbum;
        d->rootPAlbum = 0;
    }

    if (d->rootTAlbum)
    {
        delete d->rootTAlbum;
        d->rootTAlbum = 0;
    }

    if (d->rootDAlbum)
    {
        delete d->rootDAlbum;
        d->rootDAlbum = 0;
    }
    
    d->libraryPath = path;

    //TODO: rename back to digikam.db for production code
    QString dbPath = path + "/digikam-testing.db";

#ifdef NFS_HACK
    dbPath = locateLocal("appdata", KIO::encodeFileName(QDir::cleanDirPath(dbPath)));
#endif

    d->db->setDBPath(dbPath);

    QString currLocale(setlocale(0,0));
    QString dbLocale = d->db->getSetting("Locale");
    if (dbLocale.isNull())
    {
        kdDebug() << "No locale found in database" << endl;

        // Copy an existing locale from the settings file (used < 0.8)
        // to the database.
        KConfig* config = KGlobal::config();
        config->setGroup("General Settings");
        if (config->hasKey("Locale"))
        {
            kdDebug() << "Locale found in configfile" << endl;
            dbLocale = config->readEntry("Locale");
        }
        else
        {
            kdDebug() << "No locale found in config file"  << endl;
            dbLocale = currLocale;
        }
        d->db->setSetting("Locale",dbLocale);
    }

    if (dbLocale != currLocale)
    {
        int result = KMessageBox::warningYesNo(0,
                    i18n("Your locale has changed from the previous time "
                         "this album was opened. This can cause unexpected "
                         "problems. If you are sure that you want to "
                         "continue, click on 'Yes' to work with this album. "
                         "Otherwise, click on 'No' and correct your "
                         "locale setting before restarting digiKam"));
        if (result != KMessageBox::Yes)
            exit(0);

        d->db->setSetting("Locale",currLocale);
    }
}

QString AlbumManager::getLibraryPath() const
{
    return d->libraryPath;    
}

void AlbumManager::startScan()
{
    // List dates using kioslave

    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }

    d->buffer.resize(0);

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->libraryPath;
    ds << QString();
    ds << AlbumSettings::instance()->getAllFileFilter();

    KURL u;
    u.setProtocol("digikamdates");
    u.setPath("/");
    
    d->dateListJob = new KIO::TransferJob(u, KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);
    d->dateListJob->addMetaData("folders", "yes");

    connect(d->dateListJob, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(d->dateListJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
    
    // List albums directly from db
    
    d->rootPAlbum = new PAlbum(i18n("My Albums"), 0, true);
    insertPAlbum(d->rootPAlbum);
    
    AlbumInfo::List aList = d->db->scanAlbums();
    qHeapSort(aList);
    
    for (AlbumInfo::List::iterator it = aList.begin(); it != aList.end(); ++it)
    {
        AlbumInfo info = *it;
        if (info.url.isEmpty())
            continue;

        KURL u = QDir::cleanDirPath(info.url);
        QString purl = u.upURL().path(-1);

        PAlbum* parent = d->pAlbumDict.find(purl);
        if (!parent)
        {
            kdWarning() << k_funcinfo <<  "Could not find parent with url: "
                        << purl << " for: " << info.url << endl;
            continue;
        }

        PAlbum* album = new PAlbum(u.fileName(), info.id, false);
        album->setParent(parent);
        album->setCaption(info.caption, false);
        album->setCollection(info.collection, false);
        album->setDate(info.date, false);
        album->setIcon(info.icon);
        
        insertPAlbum(album);
    }

    // List tags directly from db
    
    d->rootTAlbum = new TAlbum(i18n("My Tags"), 0, true);
    insertTAlbum(d->rootTAlbum);

    TAlbumIntDict tagDict;
    TAlbumList    tagList;
    tagDict.insert(0, d->rootTAlbum);
    tagList.append(d->rootTAlbum);
    
    aList = d->db->scanTags();
    for (AlbumInfo::List::iterator it = aList.begin(); it != aList.end(); ++it)
    {
        AlbumInfo info = *it;

        TAlbum* album = new TAlbum(info.name, info.id, false);
        album->setPID(info.pid);
        album->setIcon(info.icon);
        tagDict.insert(info.id, album);
        tagList.append(album);
    }

    for (TAlbumList::iterator it = tagList.begin(); it != tagList.end(); ++it)
    {
        TAlbum* album  = *it;
        if (album->isRoot())
            continue;
        
        TAlbum* parent = tagDict.find(album->getPID());
        if (!parent)
        {
            kdWarning() << "Failed to find parent tag for tag " << album->getTitle()
                        << " with pid " << album->getPID();
            continue;
        }

        album->setParent(parent);
    }

    tagList.clear();
    tagDict.clear();

    AlbumIterator it(d->rootTAlbum);
    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);
        insertTAlbum(t);
        ++it;
    }
    
    emit signalAllAlbumsLoaded();
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
        (*it)->setTitle(newName);
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
    errMsg = i18n("Not implemented");
    return false;
    
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
    
    if (emitSignalChanged)    
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

    emit signalTAlbumMoved(album, parent);
    
    return true;
}

void AlbumManager::insertPAlbum(PAlbum *album)
{
    if (!album)
        return;

    d->pAlbumList.append(album);
    d->pAlbumDict.insert(album->getURL(), album);    
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
    QString path = url.path();
    path.remove(d->libraryPath);
    path = QDir::cleanDirPath(path);

    return d->pAlbumDict.find(path);
}

PAlbum* AlbumManager::findPAlbum(int id) const
{
    return d->pAlbumIntDict.find(id);    
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    return d->tAlbumIntDict.find(id);
}

void AlbumManager::slotResult(KIO::Job* job)
{
    d->dateListJob = 0;

    if (job->error())
    {
        kdWarning() << k_funcinfo << "Failed to list dates" << endl;
        return;
    }

    d->rootDAlbum = new DAlbum(QDate(), true);
    
    QDataStream ds(d->buffer, IO_ReadOnly);
    while (!ds.atEnd())
    {
        QDate date;
        ds >> date;
        DAlbum* album = new DAlbum(date);
        album->setParent(d->rootDAlbum);
        emit signalDAlbumAdded(album);
    }
}

void AlbumManager::slotData(KIO::Job* , const QByteArray& data)
{
    if (data.isEmpty())
        return;

    int oldSize = d->buffer.size();
    d->buffer.resize(d->buffer.size() + data.size());
    memcpy(d->buffer.data()+oldSize, data.data(), data.size());
}


#include "albummanager.moc"
