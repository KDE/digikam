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
    connect(ItemAttributesWatch::instance(), SIGNAL(signalImageDateChanged(qlonglong)),
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
    qCDebug(DIGIKAM_GENERAL_LOG) << "Got image deletion notification from ItemViewUtilities for " << imageIds.size() << " images.";

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

} // namespace Digikam
