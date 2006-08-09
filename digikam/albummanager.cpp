/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-15
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

#include <config.h>

// C Ansi includes.

extern "C"
{
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
}

// Qt includes.

#include <qfile.h>
#include <qdir.h>
#include <qdict.h>
#include <qintdict.h>
#include <qcstring.h>
#include <qtextcodec.h>

// KDE includes.

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kdirwatch.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albumitemhandler.h"
#include "dio.h"
#include "albumsettings.h"
#include "scanlib.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "albummanager.h"

namespace Digikam
{

typedef QDict<PAlbum>    PAlbumDict;
typedef QIntDict<Album>  AlbumIntDict;

class AlbumManagerPriv
{
public:
    
    AlbumDB          *db;
    AlbumItemHandler *itemHandler;
    
    QString           libraryPath;

    PAlbum           *rootPAlbum;
    TAlbum           *rootTAlbum;
    DAlbum           *rootDAlbum;
    SAlbum           *rootSAlbum;

    bool              changed;
    
    PAlbumDict        pAlbumDict;
    AlbumIntDict      albumIntDict;

    Album            *currentAlbum;

    KIO::TransferJob *dateListJob;

    KDirWatch        *dirWatch;
    QStringList       dirtyAlbums;
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
    d->rootSAlbum = 0;

    d->itemHandler  = 0;
    d->currentAlbum = 0;

    d->dirWatch = 0;

    d->changed  = false;
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
    delete d->rootSAlbum;

    delete d->dirWatch;
    
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
    QString cleanPath = QDir::cleanDirPath(path);
    
    if (cleanPath == d->libraryPath)
        return;

    d->changed = true;
    
    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }

    delete d->dirWatch;
    d->dirWatch = 0;
    d->dirtyAlbums.clear();
    
    d->currentAlbum = 0;
    emit signalAlbumCurrentChanged(0);
    emit signalAlbumsCleared();
    
    d->pAlbumDict.clear();
    d->albumIntDict.clear();

    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;
    
    d->rootPAlbum = 0;
    d->rootTAlbum = 0;
    d->rootDAlbum = 0;
    d->rootSAlbum = 0;
    
    d->libraryPath = cleanPath;

    QString dbPath = cleanPath + "/digikam3.db";

#ifdef NFS_HACK
    dbPath = locateLocal("appdata", KIO::encodeFileName(QDir::cleanDirPath(dbPath)));
#endif

    d->db->setDBPath(dbPath);

    // -- Locale Checking ---------------------------------------------------------
    
    QString currLocale(QTextCodec::codecForLocale()->name());
    QString dbLocale = d->db->getSetting("Locale");

    // guilty until proven innocent
    bool localeChanged = true;
    
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

            // this hack is necessary, as we used to store the entire
            // locale info LC_ALL (for eg: en_US.UTF-8) earlier, 
            // we now save only the encoding (UTF-8)

            QString oldConfigLocale = ::setlocale(0, 0);

            if (oldConfigLocale == dbLocale)
            {
                dbLocale = currLocale;
                localeChanged = false;
                d->db->setSetting("Locale", dbLocale);
            }
        }
        else
        {
            kdDebug() << "No locale found in config file"  << endl;
            dbLocale = currLocale;

            localeChanged = false;
            d->db->setSetting("Locale",dbLocale);
        }
    }
    else
    {
        if (dbLocale == currLocale)
            localeChanged = false;
    }

    if (localeChanged)
    {
        int result =
            KMessageBox::warningYesNo(0,
                                      i18n("Your locale has changed from the previous time "
                                           "this album was opened.\n"
                                           "Old Locale : %1, New Locale : %2\n"
                                           "This can cause unexpected problems. "
                                           "If you are sure that you want to "
                                           "continue, click on 'Yes' to work with this album. "
                                           "Otherwise, click on 'No' and correct your "
                                           "locale setting before restarting digiKam")
                                      .arg(dbLocale)
                                      .arg(currLocale));
        if (result != KMessageBox::Yes)
            exit(0);

        d->db->setSetting("Locale",currLocale);
    }

    // -- Check if we need to upgrade 0.7.x db to 0.8 db ---------------------

    if (!upgradeDB_Sqlite2ToSqlite3(d->libraryPath))
    {
        KMessageBox::error(0, i18n("Failed to update old Database to new Database format"));
        exit(0);
    }
    
    // -- Check if we need to do scanning -------------------------------------

    KConfig* config = KGlobal::config();
    config->setGroup("General Settings");
    if (config->readBoolEntry("Scan At Start", true) ||
        d->db->getSetting("Scanned").isEmpty())
    {
        ScanLib sLib;
        sLib.startScan();
    }
}

QString AlbumManager::getLibraryPath() const
{
    return d->libraryPath;    
}

void AlbumManager::startScan()
{
    if (!d->changed)
        return;
    d->changed = false;
    
    d->dirWatch = new KDirWatch(this);
    connect(d->dirWatch, SIGNAL(dirty(const QString&)),
            SLOT(slotDirty(const QString&)));

    d->dirWatch->addDir(d->libraryPath);

    d->rootPAlbum = new PAlbum(i18n("My Albums"), 0, true);
    insertPAlbum(d->rootPAlbum);
    
    d->rootTAlbum = new TAlbum(i18n("My Tags"), 0, true);
    insertTAlbum(d->rootTAlbum);

    d->rootSAlbum = new SAlbum(0, KURL(), true, true);

    d->rootDAlbum = new DAlbum(QDate(), true);

    refresh();
    
    emit signalAllAlbumsLoaded();
}

void AlbumManager::refresh()
{
    scanPAlbums();
    scanTAlbums();
    scanSAlbums();
    scanDAlbums();

    if (!d->dirtyAlbums.empty())
    {
        KURL u;
        u.setProtocol("digikamalbums");
        u.setPath(d->dirtyAlbums.first());
        d->dirtyAlbums.pop_front();
    
        DIO::scan(u);
    }
}

void AlbumManager::scanPAlbums()
{
    // first insert all the current PAlbums into a map for quick lookup
    typedef QMap<QString, PAlbum*> AlbumMap;
    AlbumMap aMap;

    AlbumIterator it(d->rootPAlbum);
    while (it.current())
    {
        PAlbum* a = (PAlbum*)(*it);
        aMap.insert(a->url(), a);
        ++it;
    }

    // scan db and get a list of all albums
    AlbumInfo::List aList = d->db->scanAlbums();
    qHeapSort(aList);

    AlbumInfo::List newAlbumList;
    
    // go through all the Albums and see which ones are already present
    for (AlbumInfo::List::iterator it = aList.begin(); it != aList.end(); ++it)
    {
        AlbumInfo info = *it;
        info.url = QDir::cleanDirPath(info.url);

        if (!aMap.contains(info.url))
        {
            newAlbumList.append(info);
        }
        else
        {
            aMap.remove(info.url);
        }
    }

    // now aMap contains all the deleted albums and
    // newAlbumList contains all the new albums

    // first inform all frontends of the deleted albums
    for (AlbumMap::iterator it = aMap.begin(); it != aMap.end(); ++it)
    {
        // the albums have to be removed with children being removed first.
        // removePAlbum takes care of that.
        // So never delete the PAlbum using it.data(). instead check if the
        // PAlbum is still in the Album Dict before trying to remove it.

        // this might look like there is memory leak here, since removePAlbum
        // doesn't delete albums and looks like child Albums don't get deleted.
        // But when the parent album gets deleted, the children are also deleted.
        
        PAlbum* album = d->pAlbumDict.find(it.key());
        if (!album)
            continue;

        removePAlbum(album);
        delete album;
    }

    qHeapSort(newAlbumList);
    for (AlbumInfo::List::iterator it = newAlbumList.begin(); it != newAlbumList.end(); ++it)
    {
        AlbumInfo info = *it;
        if (info.url.isEmpty() || info.url == "/")
            continue;

        // Despite its name info.url is a QString.
        // setPath takes care for escaping characters that are valid for files but not for URLs ('#')
        KURL u;
        u.setPath(info.url);
        QString name = u.fileName();
        // Get its parent
        QString purl = u.upURL().path(-1);

        PAlbum* parent = d->pAlbumDict.find(purl);
        if (!parent)
        {
            kdWarning() << k_funcinfo <<  "Could not find parent with url: "
                        << purl << " for: " << info.url << endl;
            continue;
        }

        // Create the new album
        PAlbum* album       = new PAlbum(name, info.id, false);
        album->m_caption    = info.caption;
        album->m_collection = info.collection;
        album->m_date       = info.date;
        album->m_icon       = info.icon;

        album->setParent(parent);
        d->dirWatch->addDir(album->folderPath());

        insertPAlbum(album);
    } 
}

void AlbumManager::scanTAlbums()
{
    // list TAlbums directly from the db
    // first insert all the current TAlbums into a map for quick lookup
    typedef QMap<int, TAlbum*> TagMap;
    TagMap tmap;

    tmap.insert(0, d->rootTAlbum);

    AlbumIterator it(d->rootTAlbum);
    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);
        tmap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of tags from the database
    TagInfo::List tList = d->db->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        QIntDict<TAlbum> tagDict;
        tagDict.setAutoDelete(false);

        // insert items into a dict for quick lookup
        for (TagInfo::List::iterator it = tList.begin(); it != tList.end(); ++it)
        {
            TagInfo info = *it;
            TAlbum* album = new TAlbum(info.name, info.id);
            album->m_icon = info.icon;
            album->m_pid  = info.pid;
            tagDict.insert(info.id, album);
        }
        tList.clear();

        // also add root tag
        TAlbum* rootTag = new TAlbum("root", 0, true);
        tagDict.insert(0, rootTag);

        // build tree
        QIntDictIterator<TAlbum> iter(tagDict);
        for ( ; iter.current(); ++iter )
        {
            TAlbum* album = iter.current();
            if (album->m_id == 0)
                continue;
            
            TAlbum* parent = tagDict.find(album->m_pid);
            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                kdWarning() << "Failed to find parent tag for tag "
                            << iter.current()->m_title
                            << " with pid "
                            << iter.current()->m_pid << endl;
            }
        }

        // now insert the items into the list. becomes sorted
        AlbumIterator it(rootTag);
        while (it.current())
        {
            TAlbum* album = (TAlbum*)it.current();
            TagInfo info;
            info.id   = album->m_id;
            info.pid  = album->m_pid;
            info.name = album->m_title;
            info.icon = album->m_icon;
            tList.append(info);
            ++it;
        }

        // this will also delete all child albums
        delete rootTag;
    }
    
    for (TagInfo::List::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TagInfo info = *it;

        // check if we have already added this tag
        if (tmap.contains(info.id))
            continue;

        // Its a new album. Find the parent of the album
        TagMap::iterator iter = tmap.find(info.pid);
        if (iter == tmap.end())
        {
            kdWarning() << "Failed to find parent tag for tag "
                        << info.name 
                        << " with pid "
                        << info.pid << endl;
            continue;
        }

        TAlbum* parent = iter.data();
        
        // Create the new TAlbum
        TAlbum* album = new TAlbum(info.name, info.id, false);
        album->m_icon = info.icon;
        album->setParent(parent);
        insertTAlbum(album);

        // also insert it in the map we are doing lookup of parent tags
        tmap.insert(info.id, album);
    }
}

void AlbumManager::scanSAlbums()
{
    // list SAlbums directly from the db
    // first insert all the current SAlbums into a map for quick lookup
    typedef QMap<int, SAlbum*> SearchMap;
    SearchMap sMap;

    AlbumIterator it(d->rootSAlbum);
    while (it.current())
    {
        SAlbum* t = (SAlbum*)(*it);
        sMap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of searches from the database
    SearchInfo::List sList = d->db->scanSearches();

    for (SearchInfo::List::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        SearchInfo info = *it;

        // check if we have already added this search
        if (sMap.contains(info.id))
            continue;

        bool simple = (info.url.queryItem("1.key") == QString::fromLatin1("keyword"));
        
        // Its a new album.
        SAlbum* album = new SAlbum(info.id, info.url, simple, false);
        album->setParent(d->rootSAlbum);
        d->albumIntDict.insert(album->globalID(), album);
        emit signalAlbumAdded(album);
    }
}

void AlbumManager::scanDAlbums()
{
    // List dates using kioslave

    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }

    KURL u;
    u.setProtocol("digikamdates");
    u.setPath("/");

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->libraryPath;
    ds << KURL();
    ds << AlbumSettings::instance()->getAllFileFilter();

    
    d->dateListJob = new KIO::TransferJob(u, KIO::CMD_SPECIAL,
                                          ba, QByteArray(), false);
    d->dateListJob->addMetaData("folders", "yes");

    connect(d->dateListJob, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(d->dateListJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
}

AlbumList AlbumManager::allPAlbums() const
{
    AlbumList list;
    if (d->rootPAlbum)
        list.append(d->rootPAlbum);

    AlbumIterator it(d->rootPAlbum);
    while (it.current())
    {
        list.append(*it);
        ++it;
    }
    
    return list;
}

AlbumList AlbumManager::allTAlbums() const
{
    AlbumList list;
    if (d->rootTAlbum)
        list.append(d->rootTAlbum);

    AlbumIterator it(d->rootTAlbum);
    while (it.current())
    {
        list.append(*it);
        ++it;
    }
    
    return list;
}

AlbumList AlbumManager::allSAlbums() const
{
    AlbumList list;
    if (d->rootSAlbum)
        list.append(d->rootSAlbum);

    AlbumIterator it(d->rootSAlbum);
    while (it.current())
    {
        list.append(*it);
        ++it;
    }
    
    return list;
}

AlbumList AlbumManager::allDAlbums() const
{
    AlbumList list;
    if (d->rootDAlbum)
        list.append(d->rootDAlbum);

    AlbumIterator it(d->rootDAlbum);
    while (it.current())
    {
        list.append(*it);
        ++it;
    }
    
    return list;
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
    if (!d->rootPAlbum)
        return 0;

    int gid = d->rootPAlbum->globalID() + id;

    return (PAlbum*)(d->albumIntDict.find(gid));
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    if (!d->rootTAlbum)
        return 0;

    int gid = d->rootTAlbum->globalID() + id;

    return (TAlbum*)(d->albumIntDict.find(gid));
}

SAlbum* AlbumManager::findSAlbum(int id) const
{
    if (!d->rootTAlbum)
        return 0;

    int gid = d->rootSAlbum->globalID() + id;

    return (SAlbum*)(d->albumIntDict.find(gid));
}

DAlbum* AlbumManager::findDAlbum(int id) const
{
    if (!d->rootDAlbum)
        return 0;

    int gid = d->rootDAlbum->globalID() + id;

    return (DAlbum*)(d->albumIntDict.find(gid));
}

Album* AlbumManager::findAlbum(int gid) const
{
    return d->albumIntDict.find(gid);
}


PAlbum* AlbumManager::createPAlbum(PAlbum* parent,
                                   const QString& name,
                                   const QString& caption,
                                   const QDate& date,
                                   const QString& collection,
                                   QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for album.");
        return 0;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Album name cannot be empty.");
        return 0;
    }
    
    if (name.contains("/"))
    {
        errMsg = i18n("Album name cannot contain '/'.");
        return 0;
    }
    
    // first check if we have another album with the same name
    Album *child = parent->m_firstChild;
    while (child)
    {
        if (child->title() == name)
        {
            errMsg = i18n("Another album with same name exists.");
            return 0;
        }
        child = child->m_next;
    }

    QString path = parent->folderPath();
    path += '/' + name;
    path = QDir::cleanDirPath(path);

    // make the directory synchronously, so that we can add the
    // album info to the database directly
    if (::mkdir(QFile::encodeName(path), 0777) != 0)
    {
        if (errno == EEXIST)
            errMsg = i18n("Another file or folder with same name exists");
        else if (errno == EACCES)
            errMsg = i18n("Access denied to path");
        else if (errno == ENOSPC)
            errMsg = i18n("Disk full");
        else
            errMsg = i18n("Unknown error"); // being lazy

        return 0;
    }

    // Now insert the album properties into the database
    path = path.remove(0, d->libraryPath.length());
    if (!path.startsWith("/"))
        path.prepend("/");

    int id = d->db->addAlbum(path, caption, date, collection);
    if (id == -1)
    {
        errMsg = i18n("Failed to add album to database");
        return 0;
    }

    PAlbum *album = new PAlbum(name, id, false);
    album->m_caption    = caption;
    album->m_collection = collection;
    album->m_date       = date;
    
    album->setParent(parent);
    
    d->dirWatch->addDir(album->folderPath());
        
    insertPAlbum(album);

    return album;
}

bool AlbumManager::renamePAlbum(PAlbum* album, const QString& newName,
                                QString& errMsg)
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
        if (sibling->title() == newName)
        {
            errMsg = i18n("Another Album with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    QString oldURL = album->url();
    
    KURL u = KURL(album->folderPath()).upURL();
    u.addPath(newName);
    u.cleanPath();

    if (::rename(QFile::encodeName(album->folderPath()),
                 QFile::encodeName(u.path(-1))) != 0)
    {
        errMsg = i18n("Failed to rename Album");
        return false;
    }
    
    // now rename the album and subalbums in the database

    // all we need to do is set the title of the album which is being
    // renamed correctly and all the sub albums will automatically get
    // their url set correctly

    album->setTitle(newName);
    d->db->setAlbumURL(album->id(), album->url());

    Album* subAlbum = 0;
    AlbumIterator it(album);
    while ((subAlbum = it.current()) != 0)
    {
        d->db->setAlbumURL(subAlbum->id(), ((PAlbum*)subAlbum)->url());
        ++it;
    }

    // Update AlbumDict. basically clear it and rebuild from scratch
    {
        d->pAlbumDict.clear();
        d->pAlbumDict.insert(d->rootPAlbum->url(), d->rootPAlbum);
        AlbumIterator it(d->rootPAlbum);
        PAlbum* subAlbum = 0;
        while ((subAlbum = (PAlbum*)it.current()) != 0)
        {
            d->pAlbumDict.insert(subAlbum->url(), subAlbum);
            ++it;
        }
    }
    
    emit signalAlbumRenamed(album);
    
    return true;
}

bool AlbumManager::updatePAlbumIcon(PAlbum *album, Q_LLONG iconID, QString& errMsg)
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

    d->db->setAlbumIcon(album->id(), iconID);
    album->m_icon = d->db->getAlbumIcon(album->id());

    emit signalAlbumIconChanged(album);    

    return true;
}

TAlbum* AlbumManager::createTAlbum(TAlbum* parent, const QString& name,
                                   const QString& iconkde, QString& errMsg)
{
    if (!parent)
    {
        errMsg = i18n("No parent found for tag");
        return 0;
    }

    // sanity checks
    if (name.isEmpty())
    {
        errMsg = i18n("Tag name cannot be empty");
        return 0;
    }
    
    if (name.contains("/"))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return 0;
    }
    
    // first check if we have another album with the same name
    Album *child = parent->m_firstChild;
    while (child)
    {
        if (child->title() == name)
        {
            errMsg = i18n("Another tag with same name exists");
            return 0;
        }
        child = child->m_next;
    }

    int id = d->db->addTag(parent->id(), name, iconkde, 0);
    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return 0;
    }
    
    TAlbum *album = new TAlbum(name, id, false);
    album->m_icon = iconkde;
    album->setParent(parent);

    insertTAlbum(album);
    
    return album;
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

    d->db->deleteTag(album->id());

    Album* subAlbum = 0;
    AlbumIterator it(album);
    while ((subAlbum = it.current()) != 0)
    {
        d->db->deleteTag(subAlbum->id());
        ++it;
    }
    
    removeTAlbum(album);

    d->albumIntDict.remove(album->globalID());
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
        if (sibling->title() == name)
        {
            errMsg = i18n("Another tag with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    d->db->setTagName(album->id(), name);
    album->setTitle(name);
    emit signalAlbumRenamed(album);
    
    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum *newParent, QString &errMsg)
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
    
    d->db->setTagParentID(album->id(), newParent->id());
    album->parent()->removeChild(album);
    album->setParent(newParent);

    emit signalTAlbumMoved(album, newParent);
    
    return true;
}

bool AlbumManager::updateTAlbumIcon(TAlbum* album, const QString& iconKDE,
                                    Q_LLONG iconID, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such tag");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot edit root tag");
        return false;
    }

    d->db->setTagIcon(album->id(), iconKDE, iconID);
    album->m_icon = d->db->getTagIcon(album->id());
    
    emit signalAlbumIconChanged(album);
    
    return true;
}

SAlbum* AlbumManager::createSAlbum(const KURL& url, bool simple)
{
    QString name = url.queryItem("name");

    // first iterate through all the search albums and see if there's an existing
    // SAlbum with same name. (Remember, SAlbums are arranged in a flat list)
    for (Album* album = d->rootSAlbum->firstChild(); album; album = album->next())
    {
        if (album->title() == name)
        {
            SAlbum* sa = (SAlbum*)album;
            sa->m_kurl = url;
            d->db->updateSearch(sa->id(), url.queryItem("name"), url);
            return sa;
        }
    }
    
    int id = d->db->addSearch(url.queryItem("name"), url);
    if (id == -1)
        return 0;
    
    SAlbum* album = new SAlbum(id, url, simple, false);
    album->setTitle(url.queryItem("name"));
    album->setParent(d->rootSAlbum);

    d->albumIntDict.insert(album->globalID(), album);
    emit signalAlbumAdded(album);
    
    return album;
}

bool AlbumManager::updateSAlbum(SAlbum* album, const KURL& newURL)
{
    if (!album)
        return false;

    d->db->updateSearch(album->id(), newURL.queryItem("name"), newURL);

    QString oldName = album->title();
    
    album->m_kurl = newURL;
    album->setTitle(newURL.queryItem("name"));
    if (oldName != album->title())
        emit signalAlbumRenamed(album);

    return true;
}

bool AlbumManager::deleteSAlbum(SAlbum* album)
{
    if (!album)
        return false;

    emit signalAlbumDeleted(album);

    d->db->deleteSearch(album->id());
    
    d->albumIntDict.remove(album->globalID());
    delete album;
    
    return true;
}

void AlbumManager::insertPAlbum(PAlbum *album)
{
    if (!album)
        return;

    d->pAlbumDict.insert(album->url(), album);
    d->albumIntDict.insert(album->globalID(), album);

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
    
    d->pAlbumDict.remove(album->url());
    d->albumIntDict.remove(album->globalID());

    d->dirtyAlbums.remove(album->url());
    d->dirWatch->removeDir(album->folderPath());

    if (album == d->currentAlbum)
    {
        d->currentAlbum = 0;
        emit signalAlbumCurrentChanged(0);
    }
    
    emit signalAlbumDeleted(album);
}

void AlbumManager::insertTAlbum(TAlbum *album)
{
    if (!album)
        return;

    d->albumIntDict.insert(album->globalID(), album);

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
    
    d->albumIntDict.remove(album->globalID());    

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

void AlbumManager::slotResult(KIO::Job* job)
{
    d->dateListJob = 0;

    if (job->error())
    {
        kdWarning() << k_funcinfo << "Failed to list dates" << endl;
        return;
    }
    
    emit signalAllDAlbumsLoaded();
}


void AlbumManager::slotData(KIO::Job* , const QByteArray& data)
{
    if (data.isEmpty())
        return;

    // insert all the DAlbums into a qmap for quick access
    QMap<QDate, DAlbum*> albumMap;
    
    AlbumIterator it(d->rootDAlbum);
    while (it.current())
    {
        DAlbum* a = (DAlbum*)(*it);
        albumMap.insert(a->date(), a);
        ++it;
    }
    
    QDataStream ds(data, IO_ReadOnly);
    while (!ds.atEnd())
    {
        QDate date;
        ds >> date;

        // Do we already have this album
        if (albumMap.contains(date))
        {
            // already there. remove from map
            albumMap.remove(date);
            continue;
        }

        // new album. create one
        DAlbum* album = new DAlbum(date);
        album->setParent(d->rootDAlbum);
        d->albumIntDict.insert(album->globalID(), album);
        emit signalAlbumAdded(album);
    }

    // Now the items contained in the map are the ones which
    // have been deleted. 
    for (QMap<QDate,DAlbum*>::iterator it = albumMap.begin();
         it != albumMap.end(); ++it)
    {
        DAlbum* album = it.data();
        emit signalAlbumDeleted(album);
        d->albumIntDict.remove(album->globalID());
        delete album;
    }
}

void AlbumManager::slotDirty(const QString& path)
{
    QString url = QDir::cleanDirPath(path);
    url = QDir::cleanDirPath(url.remove(d->libraryPath));

    if (url.isEmpty())
        url = "/";

    if (d->dirtyAlbums.contains(url))
        return;

    kdDebug() << "Dirty: " << url << endl;
    d->dirtyAlbums.append(url);

    if (DIO::running())
        return;

    KURL u;
    u.setProtocol("digikamalbums");
    u.setPath(d->dirtyAlbums.first());
    d->dirtyAlbums.pop_front();
    
    DIO::scan(u);
}

}  // namespace Digikam

#include "albummanager.moc"
