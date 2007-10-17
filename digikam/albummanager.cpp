/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 * 
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cerrno> 


// Qt includes.

#include <Q3ValueList>
#include <Q3Dict>
#include <Q3IntDict>
#include <QList>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QTextCodec>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kdirwatch.h>
#include <kconfiggroup.h>
// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albumitemhandler.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "databaseparameters.h"
#include "dio.h"
#include "imagelister.h"
#include "scanlib.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "albummanager.h"
#include "albummanager.moc"

namespace Digikam
{

typedef Q3Dict<PAlbum>   PAlbumDict;
typedef Q3IntDict<Album> AlbumIntDict;

class AlbumManagerPriv
{
public:

    AlbumManagerPriv()
    {
        changed      = false;
        dateListJob  = 0;
        dirWatch     = 0;
        itemHandler  = 0;
        rootPAlbum   = 0;
        rootTAlbum   = 0;
        rootDAlbum   = 0;
        rootSAlbum   = 0;
        currentAlbum = 0;
    }

    bool              changed;

    QString           priorityAlbumRoot;

    QStringList       dirtyAlbums;

    KIO::TransferJob *dateListJob;

    KDirWatch        *dirWatch;

    AlbumItemHandler *itemHandler;

    PAlbum           *rootPAlbum;
    TAlbum           *rootTAlbum;
    DAlbum           *rootDAlbum;
    SAlbum           *rootSAlbum;

    PAlbumDict        pAlbumDict;
    AlbumIntDict      albumIntDict;

    Album            *currentAlbum;
};

class AlbumManagerCreator { public: AlbumManager object; };
K_GLOBAL_STATIC(AlbumManagerCreator, creator);

AlbumManager* AlbumManager::instance()
{
    return &creator->object;
}

AlbumManager::AlbumManager()
{
    d = new AlbumManagerPriv;

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

    delete d;
}

void AlbumManager::setAlbumRoot(const QString &albumRoot, bool priority)
{
    // TEMPORARY SOLUTION
    // This is to ensure that the setup does not overrule the command line.
    // TODO: Replace with a better solution
    if (priority)
    {
        d->priorityAlbumRoot = albumRoot;
    }
    else if (!d->priorityAlbumRoot.isNull())
    {
        // ignore change without priority
        return;
    }

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

    // -- Database initialization -------------------------------------------------

    Digikam::DatabaseAccess::setParameters(Digikam::DatabaseParameters::parametersForSQLiteDefaultFile(albumRoot));

    if (!Digikam::DatabaseAccess::checkReadyForUse())
    {
        QString errorMsg = DatabaseAccess().lastError();
        if (errorMsg.isEmpty())
        {
            KMessageBox::error(0, i18n("<qt><p>Failed to open the database. "
                                    "</p><p>You cannot use digiKam without a working database. "
                                    "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                    "Please check the database settings in the <b>configuration menu</b>.</p></qt>"
                                    ));
        }
        else
        {
            KMessageBox::error(0, i18n("<qt><p>Failed to open the database. "
                                    " Error message from database: %1 "
                                    "</p><p>You cannot use digiKam without a working database. "
                                    "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                    "Please check the database settings in the <b>configuration menu</b>.</p></qt>",
                                    errorMsg));
        }
        return;
    }

    // -- Locale Checking ---------------------------------------------------------

    QString currLocale(QTextCodec::codecForLocale()->name());
    QString dbLocale = DatabaseAccess().db()->getSetting("Locale");

    // guilty until proven innocent
    bool localeChanged = true;
    
    if (dbLocale.isNull())
    {
        DDebug() << "No locale found in database" << endl;

        // Copy an existing locale from the settings file (used < 0.8)
        // to the database.
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group("General Settings");
        if (group.hasKey("Locale"))
        {
            DDebug() << "Locale found in configfile" << endl;
            dbLocale = group.readEntry("Locale", QString());

            // this hack is necessary, as we used to store the entire
            // locale info LC_ALL (for eg: en_US.UTF-8) earlier, 
            // we now save only the encoding (UTF-8)

            QString oldConfigLocale = ::setlocale(0, 0);

            if (oldConfigLocale == dbLocale)
            {
                dbLocale = currLocale;
                localeChanged = false;
                DatabaseAccess().db()->setSetting("Locale", dbLocale);
            }
        }
        else
        {
            DDebug() << "No locale found in config file"  << endl;
            dbLocale = currLocale;

            localeChanged = false;
            DatabaseAccess().db()->setSetting("Locale",dbLocale);
        }
    }
    else
    {
        if (dbLocale == currLocale)
            localeChanged = false;
    }

    if (localeChanged)
    {
        // TODO it would be better to replace all yes/no confirmation dialogs with ones that has custom
        // buttons that denote the actions directly, i.e.:  ["Ignore and Continue"]  ["Adjust locale"]
        int result =
            KMessageBox::warningYesNo(0,
                                      i18n("Your locale has changed from the previous time "
                                           "this album was opened.\n"
                                           "Old Locale : %1, New Locale : %2\n"
                                           "This can cause unexpected problems. "
                                           "If you are sure that you want to "
                                           "continue, click 'Yes' to work with this album. "
                                           "Otherwise, click 'No' and correct your "
                                           "locale setting before restarting digiKam",
                                           dbLocale, currLocale));
        if (result != KMessageBox::Yes)
            exit(0);

        DatabaseAccess().db()->setSetting("Locale",currLocale);
    }

    // -- Check if we need to do scanning -------------------------------------

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("General Settings");
    if (group.readEntry("Scan At Start", true) ||
        DatabaseAccess().db()->getSetting("Scanned").isEmpty())
    {
        ScanLib sLib;
        sLib.startScan();
    }
}

void AlbumManager::startScan()
{
    if (!d->changed)
        return;
    d->changed = false;

    d->dirWatch = new KDirWatch(this);
    connect(d->dirWatch, SIGNAL(dirty(const QString&)),
            this, SLOT(slotDirty(const QString&)));

    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();
    for (QStringList::iterator it = albumRootPaths.begin(); it != albumRootPaths.end(); ++it)
        d->dirWatch->addDir(*it);

    d->rootPAlbum = new PAlbum(i18n("My Albums"));
    insertPAlbum(d->rootPAlbum);

    d->rootTAlbum = new TAlbum(i18n("My Tags"), 0, true);
    insertTAlbum(d->rootTAlbum);

    d->rootSAlbum = new SAlbum(0, KUrl(), true, true);

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
        KUrl u(d->dirtyAlbums.first());
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
        aMap.insert(a->albumRootPath() + a->albumPath(), a);
        ++it;
    }

    // scan db and get a list of all albums
    AlbumInfo::List aList = DatabaseAccess().db()->scanAlbums();

    qSort(aList);

    /*
    QList<CollectionLocation *> allLocations = CollectionManager::instance()->allAvailableLocations();
    QHash<int, CollectionLocation::Status> statusHash;
    foreach (CollectionLocation *location, allLocations)
        statusHash[location->id()] = location->status();
    */

    AlbumInfo::List newAlbumList;

    // go through all the Albums and see which ones are already present
    for (AlbumInfo::List::iterator it = aList.begin(); it != aList.end(); ++it)
    {
        AlbumInfo info = *it;
        info.url = QDir::cleanPath(info.url);

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

    qSort(newAlbumList);

    for (AlbumInfo::List::iterator it = newAlbumList.begin(); it != newAlbumList.end(); ++it)
    {
        AlbumInfo info = *it;
        if (info.url.isEmpty() || info.url == "/")
            continue;

        // Despite its name info.url is a QString.
        // setPath takes care for escaping characters that are valid for files but not for URLs ('#')
        KUrl u;
        u.setPath(info.url);
        QString name = u.fileName();
        // Get its parent
        QString purl = u.upUrl().path(KUrl::RemoveTrailingSlash);

        PAlbum* parent = d->pAlbumDict.find(purl);
        if (!parent)
        {
            DWarning() <<  "Could not find parent with url: "
                       << purl << " for: " << info.url << endl;
            continue;
        }

        QString albumRootPath = CollectionManager::instance()->albumRootPath(info.albumRootId);

        // Create the new album
        PAlbum* album       = new PAlbum(albumRootPath, name, info.id);
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
    TagInfo::List tList = DatabaseAccess().db()->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        Q3IntDict<TAlbum> tagDict;
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
        Q3IntDictIterator<TAlbum> iter(tagDict);
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
                DWarning() << "Failed to find parent tag for tag "
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
            DWarning() << "Failed to find parent tag for tag "
                        << info.name 
                        << " with pid "
                        << info.pid << endl;
            continue;
        }

        TAlbum* parent = iter.value();

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
    SearchInfo::List sList = DatabaseAccess().db()->scanSearches();

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
    // List dates using kioslave:
    // The kioslave has a special mode listing the dates
    // for which there are images in the DB.

    if (d->dateListJob)
    {
        d->dateListJob->kill();
        d->dateListJob = 0;
    }

    DatabaseUrl u = DatabaseUrl::fromDate(QDate());

    d->dateListJob = ImageLister::startListJob(u);
                                               //AlbumSettings::instance()->getAllFileFilter(),
                                               //0);
    d->dateListJob->addMetaData("folders", "yes");

    connect(d->dateListJob, SIGNAL(result(KJob*)),
            SLOT(slotResult(KJob*)));
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

PAlbum* AlbumManager::findPAlbum(const KUrl& url) const
{
#warning Reimplement this, not safe for multiple roots!
    return d->pAlbumDict.find(CollectionManager::instance()->album(url));
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

TAlbum* AlbumManager::findTAlbum(const QString &tagPath) const
{
    // handle gracefully with or without leading slash
    bool withLeadingSlash = tagPath.startsWith("/");
    AlbumIterator it(d->rootTAlbum);
    while (it.current())
    {
        TAlbum *talbum = static_cast<TAlbum *>(*it);
        if (talbum->tagPath(withLeadingSlash) == tagPath)
            return talbum;
        ++it;
    }
    return 0;

}


PAlbum* AlbumManager::createPAlbum(PAlbum* parent,
                                   const QString& albumRoot,
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

    QString albumRootPath;
    if (parent->isRoot())
    {
        if (albumRoot.isNull())
        {
            errMsg = i18n("No album root path supplied");
            return 0;
        }
        albumRootPath = albumRoot;
    }
    else
    {
        albumRootPath = parent->albumRootPath();
    }

    // first check if we have another album with the same name
    Album *child = parent->m_firstChild;
    while (child)
    {
        if (child->title() == name)
        {
            errMsg = i18n("An existing album has the same name.");
            return 0;
        }
        child = child->m_next;
    }

    DatabaseUrl url = parent->databaseUrl();
    url.addPath(name);
    KUrl fileUrl = url.fileUrl();

    //TODO: Use KIO::NetAccess?
    // make the directory synchronously, so that we can add the
    // album info to the database directly
    if (::mkdir(QFile::encodeName(fileUrl.path()), 0777) != 0)
    {
        if (errno == EEXIST)
            errMsg = i18n("Another file or folder with same name exists");
        else if (errno == EACCES)
            errMsg = i18n("Access denied to path");
        else if (errno == ENOSPC)
            errMsg = i18n("Disk is full");
        else
            errMsg = i18n("Unknown error"); // being lazy

        return 0;
    }

    int id = DatabaseAccess().db()->addAlbum(albumRootPath, url.album(), caption, date, collection);

    if (id == -1)
    {
        errMsg = i18n("Failed to add album to database");
        return 0;
    }

    PAlbum *album = new PAlbum(albumRootPath, name, id);
    album->m_caption    = caption;
    album->m_collection = collection;
    album->m_date       = date;

    album->setParent(parent);

    d->dirWatch->addDir(fileUrl.path());

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
            errMsg = i18n("Another album with same name exists\n"
                          "Please choose another name");
            return false;
        }
        sibling = sibling->m_next;
    }

    QString oldAlbumPath = album->albumPath();

    KUrl u = album->fileUrl();
    u      = u.upUrl();
    u.addPath(newName);

    if (::rename(QFile::encodeName(album->folderPath()),
                 QFile::encodeName(u.path(KUrl::RemoveTrailingSlash))) != 0)
    {
        errMsg = i18n("Failed to rename Album");
        return false;
    }

    // now rename the album and subalbums in the database

    // all we need to do is set the title of the album which is being
    // renamed correctly and all the sub albums will automatically get
    // their url set correctly

    album->setTitle(newName);
    {
        DatabaseAccess access;
        access.db()->renameAlbum(album->id(), album->albumPath(), false);

        Album* subAlbum = 0;
        AlbumIterator it(album);
        while ((subAlbum = it.current()) != 0)
        {
            access.db()->renameAlbum(subAlbum->id(), ((PAlbum*)subAlbum)->albumPath(), false);
            ++it;
        }
    }

    // Update AlbumDict. basically clear it and rebuild from scratch
    {
        d->pAlbumDict.clear();
        d->pAlbumDict.insert(d->rootPAlbum->albumPath(), d->rootPAlbum);
        AlbumIterator it(d->rootPAlbum);
        PAlbum* subAlbum = 0;
        while ((subAlbum = (PAlbum*)it.current()) != 0)
        {
            d->pAlbumDict.insert(subAlbum->albumPath(), subAlbum);
            ++it;
        }
    }

    emit signalAlbumRenamed(album);

    return true;
}

bool AlbumManager::updatePAlbumIcon(PAlbum *album, qlonglong iconID, QString& errMsg)
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

    {
        DatabaseAccess access;
        access.db()->setAlbumIcon(album->id(), iconID);
        album->m_icon = access.db()->getAlbumIcon(album->id());
    }

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
            errMsg = i18n("Tag name already exists");
            return 0;
        }
        child = child->m_next;
    }

    int id = DatabaseAccess().db()->addTag(parent->id(), name, iconkde, 0);
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

AlbumList AlbumManager::findOrCreateTAlbums(const QStringList &tagPaths)
{
    // find tag ids for tag paths in list, create if they don't exist
    QList<int> tagIDs = DatabaseAccess().db()->getTagsFromTagPaths(tagPaths, true);

    // create TAlbum objects for the newly created tags
    scanTAlbums();

    AlbumList resultList;

    for (QList<int>::iterator it = tagIDs.begin() ; it != tagIDs.end() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
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

    {
        DatabaseAccess access;
        access.db()->deleteTag(album->id());

        Album* subAlbum = 0;
        AlbumIterator it(album);
        while ((subAlbum = it.current()) != 0)
        {
            access.db()->deleteTag(subAlbum->id());
            ++it;
        }
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

    DatabaseAccess().db()->setTagName(album->id(), name);
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

    DatabaseAccess().db()->setTagParentID(album->id(), newParent->id());
    album->parent()->removeChild(album);
    album->setParent(newParent);

    emit signalTAlbumMoved(album, newParent);

    return true;
}

bool AlbumManager::updateTAlbumIcon(TAlbum* album, const QString& iconKDE,
                                    qlonglong iconID, QString& errMsg)
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

    {
        DatabaseAccess access;
        access.db()->setTagIcon(album->id(), iconKDE, iconID);
        album->m_icon = access.db()->getTagIcon(album->id());
    }

    emit signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags() const
{
    QList<int> tagIDs = DatabaseAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::iterator it = tagIDs.begin() ; it != tagIDs.end() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
}

QStringList AlbumManager::tagPaths(const Q3ValueList<int> &tagIDs, bool leadingSlash) const
{
    QStringList tagPaths;

    for (Q3ValueList<int>::const_iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
    {
        TAlbum *album = findTAlbum(*it);
        if (album)
        {
            tagPaths.append(album->tagPath(leadingSlash));
        }
    }

    return tagPaths;
}

SAlbum* AlbumManager::createSAlbum(const KUrl& url, bool simple)
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
            DatabaseAccess access;
            access.db()->updateSearch(sa->id(), url.queryItem("name"), url);
            return sa;
        }
    }

    int id = DatabaseAccess().db()->addSearch(url.queryItem("name"), url);;

    if (id == -1)
        return 0;

    SAlbum* album = new SAlbum(id, url, simple, false);
    album->setTitle(url.queryItem("name"));
    album->setParent(d->rootSAlbum);

    d->albumIntDict.insert(album->globalID(), album);
    emit signalAlbumAdded(album);

    return album;
}

bool AlbumManager::updateSAlbum(SAlbum* album, const KUrl& newURL)
{
    if (!album)
        return false;

    DatabaseAccess().db()->updateSearch(album->id(), newURL.queryItem("name"), newURL);

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

    DatabaseAccess().db()->deleteSearch(album->id());

    d->albumIntDict.remove(album->globalID());
    delete album;

    return true;
}

void AlbumManager::insertPAlbum(PAlbum *album)
{
    if (!album)
        return;

    d->pAlbumDict.insert(album->albumPath(), album);
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
    
    d->pAlbumDict.remove(album->albumPath());
    d->albumIntDict.remove(album->globalID());

    DatabaseUrl url = album->databaseUrl();
    d->dirtyAlbums.removeAll(url.url());
    d->dirWatch->removeDir(url.fileUrl().path());

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

void AlbumManager::refreshItemHandler(const KUrl::List& itemList)
{
    if (itemList.empty())
        d->itemHandler->refresh();
    else
        d->itemHandler->refreshItems(itemList);
}

void AlbumManager::slotResult(KJob* job)
{
    d->dateListJob = 0;

    if (job->error())
    {
        DWarning() << "Failed to list dates" << endl;
        return;
    }
    
    emit signalAllDAlbumsLoaded();
}


void AlbumManager::slotData(KIO::Job*, const QByteArray& data)
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
    
    QByteArray di(data);
    QDataStream ds(&di, QIODevice::ReadOnly);
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
        DAlbum* album = it.value();
        emit signalAlbumDeleted(album);
        d->albumIntDict.remove(album->globalID());
        delete album;
    }
}

void AlbumManager::slotDirty(const QString& path)
{
    KUrl fileUrl;
    // we need to provide a trailing slash to DatabaseUrl to mark it as a directory
    fileUrl.setPath(QDir::cleanPath(path) + '/');
    DatabaseUrl url = DatabaseUrl::fromFileUrl(fileUrl, CollectionManager::instance()->albumRoot(fileUrl));

    if (d->dirtyAlbums.contains(url.url()))
        return;

    DDebug() << "Dirty: " << url.fileUrl().path() << endl;
    d->dirtyAlbums.append(url.url());

    if (DIO::running())
        return;

    KUrl u(d->dirtyAlbums.first());
    d->dirtyAlbums.pop_front();

    DIO::scan(u);
}

}  // namespace Digikam
