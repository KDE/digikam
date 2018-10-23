/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "albummanager_p.h"

namespace Digikam
{

Q_GLOBAL_STATIC(AlbumManagerCreator, creator)

// A friend-class shortcut to circumvent accessing this from within the destructor
AlbumManager* AlbumManager::internalInstance = 0;

AlbumManager* AlbumManager::instance()
{
    return &creator->object;
}

// -----------------------------------------------------------------------------------

AlbumManager::AlbumManager()
    : d(new Private)
{
    qRegisterMetaType<QMap<QDateTime,int>>("QMap<QDateTime,int>");
    qRegisterMetaType<QMap<int,int>>("QMap<int,int>");
    qRegisterMetaType<QMap<QString,QMap<int,int> >>("QMap<QString,QMap<int,int> >");

    internalInstance = this;
    d->albumWatch    = new AlbumWatch(this);

    // these operations are pretty fast, no need for long queuing
    d->scanPAlbumsTimer = new QTimer(this);
    d->scanPAlbumsTimer->setInterval(50);
    d->scanPAlbumsTimer->setSingleShot(true);

    connect(d->scanPAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanPAlbums()));

    d->scanTAlbumsTimer = new QTimer(this);
    d->scanTAlbumsTimer->setInterval(50);
    d->scanTAlbumsTimer->setSingleShot(true);

    connect(d->scanTAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanTAlbums()));

    d->scanSAlbumsTimer = new QTimer(this);
    d->scanSAlbumsTimer->setInterval(50);
    d->scanSAlbumsTimer->setSingleShot(true);

    connect(d->scanSAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanSAlbums()));

    d->updatePAlbumsTimer = new QTimer(this);
    d->updatePAlbumsTimer->setInterval(50);
    d->updatePAlbumsTimer->setSingleShot(true);

    connect(d->updatePAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(updateChangedPAlbums()));

    // this operation is much more expensive than the other scan methods
    d->scanDAlbumsTimer = new QTimer(this);
    d->scanDAlbumsTimer->setInterval(30 * 1000);
    d->scanDAlbumsTimer->setSingleShot(true);

    connect(d->scanDAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanDAlbumsScheduled()));

    // moderately expensive
    d->albumItemCountTimer = new QTimer(this);
    d->albumItemCountTimer->setInterval(1000);
    d->albumItemCountTimer->setSingleShot(true);

    connect(d->albumItemCountTimer, SIGNAL(timeout()),
            this, SLOT(getAlbumItemsCount()));

    // more expensive
    d->tagItemCountTimer = new QTimer(this);
    d->tagItemCountTimer->setInterval(2500);
    d->tagItemCountTimer->setSingleShot(true);

    connect(d->tagItemCountTimer, SIGNAL(timeout()),
            this, SLOT(getTagItemsCount()));
}

AlbumManager::~AlbumManager()
{
    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;
    delete d->rootSAlbum;

    internalInstance = 0;
    delete d;
}

void AlbumManager::cleanUp()
{
    // This is what we prefer to do before Application destruction

    if (d->dateListJob)
    {
        d->dateListJob->cancel();
        d->dateListJob = 0;
    }

    if (d->albumListJob)
    {
        d->albumListJob->cancel();
        d->albumListJob = 0;
    }

    if (d->tagListJob)
    {
        d->tagListJob->cancel();
        d->tagListJob = 0;
    }

    if (d->personListJob)
    {
        d->personListJob->cancel();
        d->personListJob = 0;
    }
}

void AlbumManager::startScan()
{
    if (!d->changed)
    {
        return;
    }

    d->changed = false;

    // create root albums
    d->rootPAlbum = new PAlbum(i18n("Albums"));
    insertPAlbum(d->rootPAlbum, 0);

    d->rootTAlbum = new TAlbum(i18n("Tags"), 0, true);
    insertTAlbum(d->rootTAlbum, 0);

    d->rootSAlbum = new SAlbum(i18n("Searches"), 0, true);
    emit signalAlbumAboutToBeAdded(d->rootSAlbum, 0, 0);
    d->allAlbumsIdHash[d->rootSAlbum->globalID()] = d->rootSAlbum;
    emit signalAlbumAdded(d->rootSAlbum);

    d->rootDAlbum = new DAlbum(QDate(), true);
    emit signalAlbumAboutToBeAdded(d->rootDAlbum, 0, 0);
    d->allAlbumsIdHash[d->rootDAlbum->globalID()] = d->rootDAlbum;
    emit signalAlbumAdded(d->rootDAlbum);

    // Create albums for album roots. Reuse logic implemented in the method
    foreach (const CollectionLocation& location, CollectionManager::instance()->allLocations())
    {
        handleCollectionStatusChange(location, CollectionLocation::LocationNull);
    }

    // listen to location status changes
    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(CollectionLocation,int)),
            this, SLOT(slotCollectionLocationStatusChanged(CollectionLocation,int)));

    connect(CollectionManager::instance(), SIGNAL(locationPropertiesChanged(CollectionLocation)),
            this, SLOT(slotCollectionLocationPropertiesChanged(CollectionLocation)));

    // reload albums
    refresh();

    // listen to album database changes
    connect(CoreDbAccess::databaseWatch(), SIGNAL(albumChange(AlbumChangeset)),
            this, SLOT(slotAlbumChange(AlbumChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
            this, SLOT(slotTagChange(TagChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
            this, SLOT(slotSearchChange(SearchChangeset)));

    // listen to collection image changes
    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));

    // listen to image attribute changes
    connect(ImageAttributesWatch::instance(), SIGNAL(signalImageDateChanged(qlonglong)),
            d->scanDAlbumsTimer, SLOT(start()));

    emit signalAllAlbumsLoaded();
}

bool AlbumManager::isShowingOnlyAvailableAlbums() const
{
    return d->showOnlyAvailableAlbums;
}

void AlbumManager::setShowOnlyAvailableAlbums(bool onlyAvailable)
{
    if (d->showOnlyAvailableAlbums == onlyAvailable)
    {
        return;
    }

    d->showOnlyAvailableAlbums = onlyAvailable;
    emit signalShowOnlyAvailableAlbumsChanged(d->showOnlyAvailableAlbums);

    // We need to update the unavailable locations.
    // We assume the handleCollectionStatusChange does the right thing (even though old status == current status)
    foreach (const CollectionLocation& location, CollectionManager::instance()->allLocations())
    {
        if (location.status() == CollectionLocation::LocationUnavailable)
        {
            handleCollectionStatusChange(location, CollectionLocation::LocationUnavailable);
        }
    }
}

void AlbumManager::refresh()
{
    scanPAlbums();
    scanTAlbums();
    scanSAlbums();
    scanDAlbums();
}

void AlbumManager::prepareItemCounts()
{
    // There is no way to find out if any data we had collected
    // previously is still valid - recompute
    scanDAlbums();
    getAlbumItemsCount();
    getTagItemsCount();
}

void AlbumManager::slotImagesDeleted(const QList<qlonglong>& imageIds)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Got image deletion notification from ImageViewUtilities for " << imageIds.size() << " images.";

    QSet<SAlbum*> sAlbumsToUpdate;
    QSet<qlonglong> deletedImages = imageIds.toSet();

    QList<SAlbum*> sAlbums = findSAlbumsBySearchType(DatabaseSearch::DuplicatesSearch);

    foreach (SAlbum* const sAlbum, sAlbums)
    {
        // Read the search query XML and save the image ids
        SearchXmlReader reader(sAlbum->query());
        SearchXml::Element element;
        QSet<qlonglong> images;

        while ((element = reader.readNext()) != SearchXml::End)
        {
            if ((element == SearchXml::Field) && (reader.fieldName().compare(QLatin1String("imageid")) == 0))
            {
                images = reader.valueToLongLongList().toSet();
            }
        }

        // If the deleted images are part of the SAlbum,
        // mark the album as ready for deletion and the images as ready for rescan.
#if QT_VERSION >= 0x050600
        if (images.intersects(deletedImages))
#else
        if (images.intersect(deletedImages).isEmpty())
#endif
        {
            sAlbumsToUpdate.insert(sAlbum);
        }
    }

    if (!sAlbumsToUpdate.isEmpty())
    {
        emit signalUpdateDuplicatesAlbums(sAlbumsToUpdate.toList(), deletedImages.toList());
    }
}

void AlbumManager::scanPAlbums()
{
    d->scanPAlbumsTimer->stop();

    // first insert all the current normal PAlbums into a map for quick lookup
    QHash<int, PAlbum*> oldAlbums;
    AlbumIterator it(d->rootPAlbum);

    while (it.current())
    {
        PAlbum* const a    = (PAlbum*)(*it);
        oldAlbums[a->id()] = a;
        ++it;
    }

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = CoreDbAccess().db()->scanAlbums();

    // sort by relative path so that parents are created before children
    std::sort(currentAlbums.begin(), currentAlbums.end());

    QList<AlbumInfo> newAlbums;

    // go through all the Albums and see which ones are already present
    foreach (const AlbumInfo& info, currentAlbums)
    {
        // check that location of album is available
        if (d->showOnlyAvailableAlbums && !CollectionManager::instance()->locationForAlbumRootId(info.albumRootId).isAvailable())
        {
            continue;
        }

        if (oldAlbums.contains(info.id))
        {
            oldAlbums.remove(info.id);
        }
        else
        {
            newAlbums << info;
        }
    }

    // now oldAlbums contains all the deleted albums and
    // newAlbums contains all the new albums

    // delete old albums, informing all frontends

    // The albums have to be removed with children being removed first,
    // removePAlbum takes care of that.
    // So we only feed it the albums from oldAlbums topmost in hierarchy.
    QSet<PAlbum*> topMostOldAlbums;

    foreach (PAlbum* const album, oldAlbums)
    {
        if (album->isTrashAlbum())
        {
            continue;
        }

        if (!album->parent() || !oldAlbums.contains(album->parent()->id()))
        {
            topMostOldAlbums << album;
        }
    }

    foreach (PAlbum* const album, topMostOldAlbums)
    {
        // recursively removes all children and the album
        removePAlbum(album);
    }

    // sort by relative path so that parents are created before children
    std::sort(newAlbums.begin(), newAlbums.end());

    // create all new albums
    foreach (const AlbumInfo& info, newAlbums)
    {
        if (info.relativePath.isEmpty())
        {
            continue;
        }

        PAlbum* album  = 0;
        PAlbum* parent = 0;

        if (info.relativePath == QLatin1String("/"))
        {
            // Albums that represent the root directory of an album root
            // We have them as here new albums first time after their creation

            parent = d->rootPAlbum;
            album  = d->albumRootAlbumHash.value(info.albumRootId);

            if (!album)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Did not find album root album in hash";
                continue;
            }

            // it has been created from the collection location
            // with album root id, parentPath "/" and a name, but no album id yet.
            album->m_id = info.id;
        }
        else
        {
            // last section, no slash
            QString name = info.relativePath.section(QLatin1Char('/'), -1, -1);
            // all but last sections, leading slash, no trailing slash
            QString parentPath = info.relativePath.section(QLatin1Char('/'), 0, -2);

            if (parentPath.isEmpty())
            {
                parent = d->albumRootAlbumHash.value(info.albumRootId);
            }
            else
            {
                parent = d->albumPathHash.value(PAlbumPath(info.albumRootId, parentPath));
            }

            if (!parent)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Could not find parent with url: "
                                             << parentPath << " for: "
                                             << info.relativePath;
                continue;
            }

            // Create the new album
            album = new PAlbum(info.albumRootId, parentPath, name, info.id);
        }

        album->m_caption  = info.caption;
        album->m_category = info.category;
        album->m_date     = info.date;
        album->m_iconId   = info.iconId;

        insertPAlbum(album, parent);

        if (album->isAlbumRoot())
        {
            // Inserting virtual Trash PAlbum for AlbumsRootAlbum using special constructor
            PAlbum* trashAlbum = new PAlbum(album->title(), album->id());
            insertPAlbum(trashAlbum, album);
        }
    }

    if (!topMostOldAlbums.isEmpty() || !newAlbums.isEmpty())
    {
        emit signalAlbumsUpdated(Album::PHYSICAL);
    }

    getAlbumItemsCount();
}

void AlbumManager::updateChangedPAlbums()
{
    d->updatePAlbumsTimer->stop();

    // scan db and get a list of all albums
    QList<AlbumInfo> currentAlbums = CoreDbAccess().db()->scanAlbums();
    bool needScanPAlbums           = false;

    // Find the AlbumInfo for each id in changedPAlbums
    foreach (int id, d->changedPAlbums)
    {
        foreach (const AlbumInfo& info, currentAlbums)
        {
            if (info.id == id)
            {
                d->changedPAlbums.remove(info.id);

                PAlbum* album = findPAlbum(info.id);

                if (album)
                {
                    // Renamed?
                    if (info.relativePath != QLatin1String("/"))
                    {
                        // Handle rename of album name
                        // last section, no slash
                        QString name       = info.relativePath.section(QLatin1Char('/'), -1, -1);
                        QString parentPath = info.relativePath;
                        parentPath.chop(name.length());

                        if (parentPath != album->m_parentPath || info.albumRootId != album->albumRootId())
                        {
                            // Handle actual move operations: trigger ScanPAlbums
                            needScanPAlbums = true;
                            removePAlbum(album);
                            break;
                        }
                        else if (name != album->title())
                        {
                            album->setTitle(name);
                            updateAlbumPathHash();
                            emit signalAlbumRenamed(album);
                        }
                    }

                    // Update caption, collection, date
                    album->m_caption  = info.caption;
                    album->m_category = info.category;
                    album->m_date     = info.date;

                    // Icon changed?
                    if (album->m_iconId != info.iconId)
                    {
                        album->m_iconId = info.iconId;
                        emit signalAlbumIconChanged(album);
                    }
                }
            }
        }
    }

    if (needScanPAlbums)
    {
        scanPAlbums();
    }
}

void AlbumManager::scanTAlbums()
{
    d->scanTAlbumsTimer->stop();

    // list TAlbums directly from the db
    // first insert all the current TAlbums into a map for quick lookup
    typedef QMap<int, TAlbum*> TagMap;
    TagMap                     tmap;

    tmap.insert(0, d->rootTAlbum);

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* t = (TAlbum*)(*it);
        tmap.insert(t->id(), t);
        ++it;
    }

    // Retrieve the list of tags from the database
    TagInfo::List tList = CoreDbAccess().db()->scanTags();

    // sort the list. needed because we want the tags can be read in any order,
    // but we want to make sure that we are ensure to find the parent TAlbum
    // for a new TAlbum

    {
        QHash<int, TAlbum*> tagHash;

        // insert items into a dict for quick lookup
        for (TagInfo::List::const_iterator iter = tList.constBegin() ; iter != tList.constEnd() ; ++iter)
        {
            TagInfo info        = *iter;
            TAlbum* const album = new TAlbum(info.name, info.id);
            album->m_icon       = info.icon;
            album->m_iconId     = info.iconId;
            album->m_pid        = info.pid;
            tagHash.insert(info.id, album);
        }

        tList.clear();

        // also add root tag
        TAlbum* const rootTag = new TAlbum(QLatin1String("root"), 0, true);
        tagHash.insert(0, rootTag);

        // build tree
        for (QHash<int, TAlbum*>::const_iterator iter = tagHash.constBegin() ; iter != tagHash.constEnd() ; ++iter)
        {
            TAlbum* album = *iter;

            if (album->m_id == 0)
            {
                continue;
            }

            TAlbum* const parent = tagHash.value(album->m_pid);

            if (parent)
            {
                album->setParent(parent);
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                           << album->m_title
                           << " with pid "
                           << album->m_pid;
            }
        }

        tagHash.clear();

        // now insert the items into the list. becomes sorted
        AlbumIterator it(rootTag);

        while (it.current())
        {
            TagInfo info;
            TAlbum* const album = static_cast<TAlbum*>(it.current());

            if (album)
            {
                info.id     = album->m_id;
                info.pid    = album->m_pid;
                info.name   = album->m_title;
                info.icon   = album->m_icon;
                info.iconId = album->m_iconId;
            }

            tList.append(info);
            ++it;
        }

        // this will also delete all child albums
        delete rootTag;
    }

    for (TagInfo::List::const_iterator it = tList.constBegin() ; it != tList.constEnd() ; ++it)
    {
        TagInfo info = *it;

        // check if we have already added this tag
        if (tmap.contains(info.id))
        {
            continue;
        }

        // Its a new album. Find the parent of the album
        TagMap::const_iterator iter = tmap.constFind(info.pid);

        if (iter == tmap.constEnd())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to find parent tag for tag "
                                           << info.name
                                           << " with pid "
                                           << info.pid;
            continue;
        }

        TAlbum* const parent = iter.value();

        // Create the new TAlbum
        TAlbum* const album = new TAlbum(info.name, info.id, false);
        album->m_icon       = info.icon;
        album->m_iconId     = info.iconId;
        insertTAlbum(album, parent);

        // also insert it in the map we are doing lookup of parent tags
        tmap.insert(info.id, album);
    }

    if (!tList.isEmpty())
    {
        emit signalAlbumsUpdated(Album::TAG);
    }

    getTagItemsCount();
}

void AlbumManager::getTagItemsCount()
{
    d->tagItemCountTimer->stop();

    if (!ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        return;
    }

    tagItemsCount();
    personItemsCount();
}

void AlbumManager::tagItemsCount()
{
    if (d->tagListJob)
    {
        d->tagListJob->cancel();
        d->tagListJob = 0;
    }

    TagsDBJobInfo jInfo;
    jInfo.setFoldersJob();

    d->tagListJob = DBJobsManager::instance()->startTagsJobThread(jInfo);

    connect(d->tagListJob, SIGNAL(finished()),
            this, SLOT(slotTagsJobResult()));

    connect(d->tagListJob, SIGNAL(foldersData(QMap<int,int>)),
            this, SLOT(slotTagsJobData(QMap<int,int>)));
}

AlbumList AlbumManager::allPAlbums() const
{
    AlbumList list;

    if (d->rootPAlbum)
    {
        list.append(d->rootPAlbum);
    }

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
    {
        list.append(d->rootTAlbum);
    }

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

PAlbum* AlbumManager::currentPAlbum() const
{
    /**
     * Temporary fix, to return multiple items,
     * iterate and cast each element
     */
    if (!d->currentAlbums.isEmpty())
        return dynamic_cast<PAlbum*>(d->currentAlbums.first());
    else
        return 0;
}

QList<TAlbum*> AlbumManager::currentTAlbums() const
{
    /**
     * This method is not yet used
     */
    QList<TAlbum*> talbums;
    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        TAlbum* const temp = dynamic_cast<TAlbum*>(*it);

        if (temp)
            talbums.append(temp);
    }

    return talbums;
}

PAlbum* AlbumManager::findPAlbum(const QUrl& url) const
{
    CollectionLocation location = CollectionManager::instance()->locationForUrl(url);

    if (location.isNull())
    {
        return 0;
    }

    return d->albumPathHash.value(PAlbumPath(location.id(), CollectionManager::instance()->album(location, url)));
}

PAlbum* AlbumManager::findPAlbum(int id) const
{
    if (!d->rootPAlbum)
    {
        return 0;
    }

    int gid = d->rootPAlbum->globalID() + id;

    return static_cast<PAlbum*>((d->allAlbumsIdHash.value(gid)));
}

TAlbum* AlbumManager::findTAlbum(int id) const
{
    if (!d->rootTAlbum)
    {
        return 0;
    }

    int gid = d->rootTAlbum->globalID() + id;

    return static_cast<TAlbum*>((d->allAlbumsIdHash.value(gid)));
}

TAlbum* AlbumManager::findTAlbum(const QString& tagPath) const
{
    // handle gracefully with or without leading slash
    bool withLeadingSlash = tagPath.startsWith(QLatin1Char('/'));
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const talbum = static_cast<TAlbum*>(*it);

        if (talbum->tagPath(withLeadingSlash) == tagPath)
        {
            return talbum;
        }

        ++it;
    }

    return 0;

}

PAlbum* AlbumManager::createPAlbum(const QString& albumRootPath, const QString& name,
                                   const QString& caption, const QDate& date,
                                   const QString& category,
                                   QString& errMsg)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRootPath);

    return createPAlbum(location, name, caption, date, category, errMsg);
}

PAlbum* AlbumManager::createPAlbum(const CollectionLocation& location, const QString& name,
                                   const QString& caption, const QDate& date,
                                   const QString& category,
                                   QString& errMsg)
{
    if (location.isNull() || !location.isAvailable())
    {
        errMsg = i18n("The collection location supplied is invalid or currently not available.");
        return 0;
    }

    PAlbum* album = d->albumRootAlbumHash.value(location.id());

    if (!album)
    {
        errMsg = i18n("No album for collection location: Internal error");
        return 0;
    }

    return createPAlbum(album, name, caption, date, category, errMsg);
}

PAlbum* AlbumManager::createPAlbum(PAlbum*        parent,
                                   const QString& name,
                                   const QString& caption,
                                   const QDate&   date,
                                   const QString& category,
                                   QString&       errMsg)
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

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Album name cannot contain '/'.");
        return 0;
    }

    if (parent->isRoot())
    {
        errMsg = i18n("createPAlbum does not accept the root album as parent.");
        return 0;
    }

    QString albumPath = parent->isAlbumRoot() ? QString(QLatin1Char('/') + name) : QString(parent->albumPath() + QLatin1Char('/') + name);
    int albumRootId   = parent->albumRootId();

    // first check if we have a sibling album with the same name
    PAlbum* child = static_cast<PAlbum*>(parent->firstChild());

    while (child)
    {
        if (child->albumRootId() == albumRootId && child->albumPath() == albumPath)
        {
            errMsg = i18n("An existing album has the same name.");
            return 0;
        }

        child = static_cast<PAlbum*>(child->next());
    }

    CoreDbUrl url   = parent->databaseUrl();
    url             = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + name);
    QUrl fileUrl    = url.fileUrl();

    bool ret = QDir().mkdir(fileUrl.toLocalFile());

    if (!ret)
    {
        errMsg = i18n("Failed to create directory '%1'", fileUrl.toString()); // TODO add tags?
        return 0;
    }

    ChangingDB changing(d);
    int        id = CoreDbAccess().db()->addAlbum(albumRootId, albumPath, caption, date, category);

    if (id == -1)
    {
        errMsg = i18n("Failed to add album to database");
        return 0;
    }

    QString parentPath;

    if (!parent->isAlbumRoot())
    {
        parentPath = parent->albumPath();
    }

    PAlbum* const album = new PAlbum(albumRootId, parentPath, name, id);
    album->m_caption    = caption;
    album->m_category   = category;
    album->m_date       = date;

    insertPAlbum(album, parent);
    emit signalAlbumsUpdated(Album::PHYSICAL);

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

    if (album->isAlbumRoot())
    {
        errMsg = i18n("Cannot rename album root album");
        return false;
    }

    if (newName.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Album name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name
    if (hasDirectChildAlbumWithTitle(album->m_parent, newName))
    {
        errMsg = i18n("Another album with the same name already exists.\n"
                      "Please choose another name.");
        return false;
    }

    d->albumWatch->removeWatchedPAlbums(album);

    QString oldAlbumPath = album->albumPath();
    QUrl oldUrl          = album->fileUrl();
    album->setTitle(newName);
    album->m_path        = newName;
    QUrl newUrl          = album->fileUrl();
    QString newAlbumPath = album->albumPath();

    // We use a private shortcut around collection scanner noticing our changes,
    // we rename them directly. Faster.
    ScanController::instance()->suspendCollectionScan();

    bool ret = QDir().rename(oldUrl.toLocalFile(), newUrl.toLocalFile());

    if (!ret)
    {
        ScanController::instance()->resumeCollectionScan();

        errMsg = i18n("Failed to rename Album");
        return false;
    }

    // now rename the album and subalbums in the database
    {
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->renameAlbum(album->id(), album->albumRootId(), album->albumPath());

        PAlbum* subAlbum = 0;
        AlbumIterator it(album);

        while ((subAlbum = static_cast<PAlbum*>(it.current())) != 0)
        {
            subAlbum->m_parentPath = newAlbumPath + subAlbum->m_parentPath.mid(oldAlbumPath.length());
            access.db()->renameAlbum(subAlbum->id(), album->albumRootId(), subAlbum->albumPath());
            emit signalAlbumNewPath(subAlbum);
            ++it;
        }
    }

    updateAlbumPathHash();
    emit signalAlbumRenamed(album);

    ScanController::instance()->resumeCollectionScan();

    return true;
}

bool AlbumManager::updatePAlbumIcon(PAlbum* album, qlonglong iconID, QString& errMsg)
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
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setAlbumIcon(album->id(), iconID);
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

qlonglong AlbumManager::getItemFromAlbum(PAlbum* album, const QString& fileName)
{
    return CoreDbAccess().db()->getItemFromAlbum(album->id(), fileName);
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

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return 0;
    }

    // first check if we have another album with the same name
    if (hasDirectChildAlbumWithTitle(parent, name))
    {
        errMsg = i18n("Tag name already exists");
        return 0;
    }

    ChangingDB changing(d);
    int id = CoreDbAccess().db()->addTag(parent->id(), name, iconkde, 0);

    if (id == -1)
    {
        errMsg = i18n("Failed to add tag to database");
        return 0;
    }

    TAlbum* const album = new TAlbum(name, id, false);
    album->m_icon       = iconkde;

    insertTAlbum(album, parent);

    TAlbum* personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    emit signalAlbumsUpdated(Album::TAG);

    return album;
}

AlbumList AlbumManager::findOrCreateTAlbums(const QStringList& tagPaths)
{
    // find tag ids for tag paths in list, create if they don't exist
    QList<int> tagIDs = TagsCache::instance()->getOrCreateTags(tagPaths);

    // create TAlbum objects for the newly created tags
    scanTAlbums();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        resultList.append(findTAlbum(*it));
    }

    return resultList;
}

bool AlbumManager::deleteTAlbum(TAlbum* album, QString& errMsg, bool askUser)
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

    QList<qlonglong> imageIds;

    if (askUser)
    {
        imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
    }

    {
        CoreDbAccess access;
        ChangingDB changing(d);
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
    emit signalAlbumsUpdated(Album::TAG);

    if (askUser)
    {
        askUserForWriteChangedTAlbumToFiles(imageIds);
    }

    return true;
}

bool AlbumManager::renameTAlbum(TAlbum* album,
                                const QString& name,
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

    if (name.contains(QLatin1Char('/')))
    {
        errMsg = i18n("Tag name cannot contain '/'");
        return false;
    }

    // first check if we have another sibling with the same name
    if (hasDirectChildAlbumWithTitle(album->m_parent, name))
    {
        errMsg = i18n("Another tag with the same name already exists.\n"
                      "Please choose another name.");
        return false;
    }

    ChangingDB changing(d);
    CoreDbAccess().db()->setTagName(album->id(), name);
    album->setTitle(name);
    emit signalAlbumRenamed(album);

    askUserForWriteChangedTAlbumToFiles(album);

    return true;
}

bool AlbumManager::moveTAlbum(TAlbum* album, TAlbum* newParent, QString& errMsg)
{
    if (!album)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (!newParent)
    {
        errMsg = i18n("Attempt to move TAlbum to nowhere");
        return false;
    }

    if (album == d->rootTAlbum)
    {
        errMsg = i18n("Cannot move root tag");
        return false;
    }

    if (hasDirectChildAlbumWithTitle(newParent, album->title()))
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18n("Another tag with the same name already exists.\n"
                      "Do you want to merge the tags?"),
                 QMessageBox::Yes | QMessageBox::No,
                 qApp->activeWindow());

        int result = msgBox->exec();
        delete msgBox;

        if (result == QMessageBox::Yes)
        {
            TAlbum* const destAlbum = findTAlbum(newParent->tagPath() +
                                                 QLatin1Char('/')     +
                                                 album->title());

            return mergeTAlbum(album, destAlbum, false, errMsg);
        }
        else
        {
            return true;
        }
    }

    d->currentlyMovingAlbum = album;
    emit signalAlbumAboutToBeMoved(album);

    emit signalAlbumAboutToBeDeleted(album);

    if (album->parent())
    {
        album->parent()->removeChild(album);
    }

    album->setParent(0);
    emit signalAlbumDeleted(album);
    emit signalAlbumHasBeenDeleted(reinterpret_cast<quintptr>(album));

    emit signalAlbumAboutToBeAdded(album, newParent, newParent->lastChild());
    ChangingDB changing(d);
    CoreDbAccess().db()->setTagParentID(album->id(), newParent->id());
    album->setParent(newParent);
    emit signalAlbumAdded(album);

    emit signalAlbumMoved(album);
    emit signalAlbumsUpdated(Album::TAG);
    d->currentlyMovingAlbum = 0;

    TAlbum* personParentTag = findTAlbum(FaceTags::personParentTag());

    if (personParentTag && personParentTag->isAncestorOf(album))
    {
        FaceTags::ensureIsPerson(album->id());
    }

    askUserForWriteChangedTAlbumToFiles(album);

    return true;
}

bool AlbumManager::mergeTAlbum(TAlbum* album, TAlbum* destAlbum, bool dialog, QString& errMsg)
{
    if (!album || !destAlbum)
    {
        errMsg = i18n("No such album");
        return false;
    }

    if (album == d->rootTAlbum || destAlbum == d->rootTAlbum)
    {
        errMsg = i18n("Cannot merge root tag");
        return false;
    }

    if (album->firstChild())
    {
        errMsg = i18n("Only a tag without children can be merged!");
        return false;
    }

    if (dialog)
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18n("Do you want to merge tag '%1' into tag '%2'?",
                      album->title(), destAlbum->title()),
                 QMessageBox::Yes | QMessageBox::No,
                 qApp->activeWindow());

        int result = msgBox->exec();
        delete msgBox;

        if (result == QMessageBox::No)
        {
            return true;
        }
    }

    int oldId   = album->id();
    int mergeId = destAlbum->id();

    if (oldId == mergeId)
    {
        return true;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(oldId);

    CoreDbOperationGroup group;
    group.setMaximumTime(200);

    foreach (const qlonglong& imageId, imageIds)
    {
        QList<FaceTagsIface> facesList = FaceTagsEditor().databaseFaces(imageId);
        bool foundFace                 = false;

        foreach (const FaceTagsIface& face, facesList)
        {
            if (face.tagId() == oldId)
            {
                foundFace = true;
                FaceTagsEditor().removeFace(face);
                FaceTagsEditor().add(imageId, mergeId, face.region(), false);
            }
        }

        if (!foundFace)
        {
            ImageInfo info(imageId);
            info.removeTag(oldId);
            info.setTag(mergeId);
            group.allowLift();
        }
    }

    QApplication::restoreOverrideCursor();

    if (!deleteTAlbum(album, errMsg, false))
    {
        return false;
    }

    askUserForWriteChangedTAlbumToFiles(imageIds);

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
        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setTagIcon(album->id(), iconKDE, iconID);
        album->m_icon   = iconKDE;
        album->m_iconId = iconID;
    }

    emit signalAlbumIconChanged(album);

    return true;
}

AlbumList AlbumManager::getRecentlyAssignedTags(bool includeInternal) const
{
    QList<int> tagIDs = CoreDbAccess().db()->getRecentlyAssignedTags();

    AlbumList resultList;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* const album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            resultList.append(album);
        }
    }

    return resultList;
}

QStringList AlbumManager::tagPaths(const QList<int>& tagIDs,
                                   bool leadingSlash,
                                   bool includeInternal) const
{
    QStringList tagPaths;

    for (QList<int>::const_iterator it = tagIDs.constBegin() ; it != tagIDs.constEnd() ; ++it)
    {
        TAlbum* album = findTAlbum(*it);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagPaths.append(album->tagPath(leadingSlash));
        }
    }

    return tagPaths;
}

QStringList AlbumManager::tagNames(const QList<int>& tagIDs, bool includeInternal) const
{
    QStringList tagNames;

    foreach (int id, tagIDs)
    {
        TAlbum* const album = findTAlbum(id);

        if (album)
        {
            if (!includeInternal && album->isInternalTag())
            {
                continue;
            }

            tagNames << album->title();
        }
    }

    return tagNames;
}

QHash<int, QString> AlbumManager::tagPaths(bool leadingSlash, bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->tagPath(leadingSlash));
        }

        ++it;
    }

    return hash;
}

QHash<int, QString> AlbumManager::tagNames(bool includeInternal) const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        TAlbum* const t = (TAlbum*)(*it);

        if (includeInternal || !t->isInternalTag())
        {
            hash.insert(t->id(), t->title());
        }

        ++it;
    }

    return hash;
}

QList< int > AlbumManager::subTags(int tagId, bool recursive)
{
    TAlbum* const album = this->findTAlbum(tagId);

    return album->childAlbumIds(recursive);
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property)
{
    AlbumList list;

    QList<int> ids = TagsCache::instance()->tagsWithProperty(property);

    foreach (int id, ids)
    {
        TAlbum* const album = findTAlbum(id);

        if (album)
        {
            list << album;
        }
    }

    return list;
}

AlbumList AlbumManager::findTagsWithProperty(const QString& property, const QString& value)
{
    AlbumList list;

    AlbumIterator it(d->rootTAlbum);

    while (it.current())
    {
        if (static_cast<TAlbum*>(*it)->property(property) == value)
        {
            list << *it;
        }

        ++it;
    }

    return list;
}

QMap<int, int> AlbumManager::getPAlbumsCount() const
{
    return d->pAlbumsCount;
}

QMap<int, int> AlbumManager::getTAlbumsCount() const
{
    return d->tAlbumsCount;
}

void AlbumManager::insertPAlbum(PAlbum* album, PAlbum* parent)
{
    if (!album)
    {
        return;
    }

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
    {
        album->setParent(parent);
    }

    d->albumPathHash[PAlbumPath(album)]   = album;
    d->allAlbumsIdHash[album->globalID()] = album;

    emit signalAlbumAdded(album);
}

void AlbumManager::removePAlbum(PAlbum* album)
{
    if (!album)
    {
        return;
    }

    // remove all children of this album
    Album* child        = album->firstChild();
    PAlbum* toBeRemoved = 0;

    while (child)
    {
        Album* next = child->next();
        toBeRemoved = static_cast<PAlbum*>(child);

        if (toBeRemoved)
        {
            removePAlbum(toBeRemoved);
            toBeRemoved = 0;
        }

        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->albumPathHash.remove(PAlbumPath(album));
    d->allAlbumsIdHash.remove(album->globalID());

    CoreDbUrl url = album->databaseUrl();

    if (!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    if (album->isAlbumRoot())
    {
        d->albumRootAlbumHash.remove(album->albumRootId());
    }

    emit signalAlbumDeleted(album);
    quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
    delete album;

    emit signalAlbumHasBeenDeleted(deletedAlbum);
}

void AlbumManager::insertTAlbum(TAlbum* album, TAlbum* parent)
{
    if (!album)
    {
        return;
    }

    emit signalAlbumAboutToBeAdded(album, parent, parent ? parent->lastChild() : 0);

    if (parent)
    {
        album->setParent(parent);
    }

    d->allAlbumsIdHash.insert(album->globalID(), album);

    emit signalAlbumAdded(album);
}

void AlbumManager::removeTAlbum(TAlbum* album)
{
    if (!album)
    {
        return;
    }

    // remove all children of this album
    Album* child        = album->firstChild();
    TAlbum* toBeRemoved = 0;

    while (child)
    {
        Album* next = child->next();
        toBeRemoved = static_cast<TAlbum*>(child);

        if (toBeRemoved)
        {
            removeTAlbum(toBeRemoved);
            toBeRemoved = 0;
        }

        child = next;
    }

    emit signalAlbumAboutToBeDeleted(album);
    d->allAlbumsIdHash.remove(album->globalID());

    if (!d->currentAlbums.isEmpty())
    {
        if (album == d->currentAlbums.first())
        {
            d->currentAlbums.clear();
            emit signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    emit signalAlbumDeleted(album);

    quintptr deletedAlbum = reinterpret_cast<quintptr>(album);
    delete album;

    emit signalAlbumHasBeenDeleted(deletedAlbum);
}

void AlbumManager::slotTagsJobResult()
{
    if (!d->tagListJob)
    {
        return;
    }

    if (d->tagListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list face tags";

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->personListJob->errorsList().first(),
                             0, i18n("digiKam"));
    }

    d->tagListJob = 0;
}

void AlbumManager::slotTagsJobData(const QMap<int, int>& tagsStatMap)
{
    if (tagsStatMap.isEmpty())
    {
        return;
    }

    d->tAlbumsCount = tagsStatMap;
    emit signalTAlbumsDirty(tagsStatMap);
}

void AlbumManager::slotTagChange(const TagChangeset& changeset)
{
    if (d->changingDB || !d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case TagChangeset::Added:
        case TagChangeset::Moved:
        case TagChangeset::Deleted:
        case TagChangeset::Reparented:

            if (!d->scanTAlbumsTimer->isActive())
            {
                d->scanTAlbumsTimer->start();
            }

            break;

        case TagChangeset::Renamed:
        case TagChangeset::IconChanged:
            /**
             * @todo what happens here?
             */
            break;

        case TagChangeset::PropertiesChanged:
        {
            TAlbum* tag = findTAlbum(changeset.tagId());

            if (tag)
            {
                emit signalTagPropertiesChanged(tag);
            }

            break;
        }

        case TagChangeset::Unknown:
            break;
    }
}

void AlbumManager::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (!d->rootTAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case ImageTagChangeset::Added:
        case ImageTagChangeset::Removed:
        case ImageTagChangeset::RemovedAll:
        // Add properties changed.
        // Reason: in people sidebar, the images are not
        // connected with the ImageTag table but by
        // ImageTagProperties entries.
        // Thus, the count of entries in face tags are not
        // updated. This adoption should fix the problem.
        case ImageTagChangeset::PropertiesChanged:

            if (!d->tagItemCountTimer->isActive())
            {
                d->tagItemCountTimer->start();
            }

            break;

        default:
            break;
    }
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(TAlbum* const album)
{
    QList<qlonglong> imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
    askUserForWriteChangedTAlbumToFiles(imageIds);
}

void AlbumManager::askUserForWriteChangedTAlbumToFiles(const QList<qlonglong>& imageIds)
{
    MetaEngineSettings* const settings = MetaEngineSettings::instance();

    if ((!settings->settings().saveTags &&
         !settings->settings().saveFaceTags) || imageIds.isEmpty())
    {
        return;
    }

    if (imageIds.count() > 100)
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18n("This operation can take a long time in the background.\n"
                      "Do you want to write the metadata to %1 files now?",
                      imageIds.count()),
                 QMessageBox::Yes | QMessageBox::No,
                 qApp->activeWindow());

        int result = msgBox->exec();
        delete msgBox;

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    ImageInfoList infos(imageIds);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void AlbumManager::removeWatchedPAlbums(const PAlbum* const album)
{
    d->albumWatch->removeWatchedPAlbums(album);
}

} // namespace Digikam
