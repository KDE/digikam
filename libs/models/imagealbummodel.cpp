/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with ioslave
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagealbummodel.moc"

// Qt includes

#include <QTimer>

// KDE includes

#include <kio/job.h>
#include <kdebug.h>
#include <kapplication.h>

// Local includes

#include "albummanager.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databaseface.h"
#include "databasewatch.h"
#include "databaseurl.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imagelister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"

namespace Digikam
{

class ImageAlbumModel::ImageAlbumModelPriv
{
public:

    ImageAlbumModelPriv()
    {
        job              = 0;
        refreshTimer     = 0;
        incrementalTimer = 0;
        recurseAlbums    = false;
        recurseTags      = false;
        listOnlyAvailableImages = false;
        extraValueJob    = false;
    }

    QList<Album*>     currentAlbums;
    KIO::TransferJob* job;
    QTimer*           refreshTimer;
    QTimer*           incrementalTimer;

    bool              recurseAlbums;
    bool              recurseTags;
    bool              listOnlyAvailableImages;
    QString           specialListing;

    bool              extraValueJob;
};

ImageAlbumModel::ImageAlbumModel(QObject* parent)
    : ImageThumbnailModel(parent),
      d(new ImageAlbumModelPriv)
{
    d->refreshTimer     = new QTimer(this);
    d->refreshTimer->setSingleShot(true);

    d->incrementalTimer = new QTimer(this);
    d->incrementalTimer->setSingleShot(true);

    connect(d->refreshTimer, SIGNAL(timeout()),
            this, SLOT(slotNextRefresh()));

    connect(d->incrementalTimer, SIGNAL(timeout()),
            this, SLOT(slotNextIncrementalRefresh()));

    connect(this, SIGNAL(readyForIncrementalRefresh()),
            this, SLOT(incrementalRefresh()));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
            this, SLOT(slotSearchChange(SearchChangeset)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalShowOnlyAvailableAlbumsChanged(bool)),
            this, SLOT(setListOnlyAvailableImages(bool)));
}

ImageAlbumModel::~ImageAlbumModel()
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    delete d;
}

QList<Album*> ImageAlbumModel::currentAlbums() const
{
    return d->currentAlbums;
}

void ImageAlbumModel::setRecurseAlbums(bool recursiveListing)
{
    if (d->recurseAlbums != recursiveListing)
    {
        d->recurseAlbums = recursiveListing;
        refresh();
    }
}

void ImageAlbumModel::setRecurseTags(bool recursiveListing)
{
    if (d->recurseTags != recursiveListing)
    {
        d->recurseTags = recursiveListing;
        refresh();
    }
}

void ImageAlbumModel::setListOnlyAvailableImages(bool onlyAvailable)
{
    if (d->listOnlyAvailableImages!= onlyAvailable)
    {
        d->listOnlyAvailableImages = onlyAvailable;
        refresh();
    }
}

bool ImageAlbumModel::isRecursingAlbums() const
{
    return d->recurseAlbums;
}

bool ImageAlbumModel::isRecursingTags() const
{
    return d->recurseTags;
}

bool ImageAlbumModel::isListingOnlyAvailableImages() const
{
    return d->listOnlyAvailableImages;
}

void ImageAlbumModel::setSpecialTagListing(const QString& specialListing)
{
    if (d->specialListing != specialListing)
    {
        d->specialListing = specialListing;
        refresh();
    }
}

void ImageAlbumModel::openAlbum(QList<Album*> albums)
{
    if (d->currentAlbums == albums)
    {
        return;
    }

    d->currentAlbums.clear();

    /**
     * Extra safety, ensure that no null pointers are added
     */
    foreach (Album* a, albums)
    {
        if (a)
        {
            d->currentAlbums << a;
        }
    }
    //emit listedAlbumChanged(d->currentAlbums);
    refresh();
}

void ImageAlbumModel::refresh()
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    clearImageInfos();

    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    /** TODO: Figure out how to deal with root album
    if (d->currentAlbum->isRoot())
    {
        return;
    }
    */

    startRefresh();

    startListJob(d->currentAlbums);
}

void ImageAlbumModel::incrementalRefresh()
{
    // The path to this method is:
    // scheduleIncrementalRefresh -> incrementalTimer waits 100ms -> slotNextIncrementalRefresh
    // -> ImageModel::requestIncrementalRefresh -> waits until model is ready, maybe immediately
    // -> to this method via SIGNAL(readyForIncrementalRefresh())

    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    startIncrementalRefresh();

    startListJob(d->currentAlbums);
}

bool ImageAlbumModel::hasScheduledRefresh() const
{
    return d->refreshTimer->isActive() || d->incrementalTimer->isActive() || hasIncrementalRefreshPending();
}

void ImageAlbumModel::scheduleRefresh()
{
    if (!d->refreshTimer->isActive())
    {
        d->incrementalTimer->stop();
        d->refreshTimer->start(100);
    }
}

void ImageAlbumModel::scheduleIncrementalRefresh()
{
    if (!hasScheduledRefresh())
    {
        d->incrementalTimer->start(100);
    }
}

void ImageAlbumModel::slotNextRefresh()
{
    // Refresh, unless job is running, then postpone restart until job is finished
    // Rationale: Let the job run, don't stop it possibly several times
    if (d->job)
    {
        d->refreshTimer->start(50);
    }
    else
    {
        refresh();
    }
}

void ImageAlbumModel::slotNextIncrementalRefresh()
{
    if (d->job)
    {
        d->incrementalTimer->start(50);
    }
    else
    {
        requestIncrementalRefresh();
    }
}

void ImageAlbumModel::startListJob(QList<Album*> albums)
{
    if(albums.isEmpty())
    {
        return;
    }

    KUrl url;
    if(albums.first()->type() == Album::TAG)
    {
        QList<int> tagIds;

        for(QList<Album*>::iterator it = albums.begin(); it != albums.end(); ++it)
        {
            tagIds << (*it)->id();
        }
        url = DatabaseUrl::fromTagIds(tagIds);
    }
    else
        url = albums.first()->databaseUrl();

    d->extraValueJob = false;
    d->job   = ImageLister::startListJob(url);
    d->job->addMetaData("listAlbumsRecursively", d->recurseAlbums ? "true" : "false");
    d->job->addMetaData("listTagsRecursively", d->recurseTags ? "true" : "false");
    d->job->addMetaData("listOnlyAvailableImages", d->listOnlyAvailableImages ? "true" : "false");

    if (albums.first()->type() == Album::TAG && !d->specialListing.isNull())
    {
        d->job->addMetaData("specialTagListing", d->specialListing);
        d->extraValueJob = true;
    }

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(d->job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotData(KIO::Job*,QByteArray)));
}

void ImageAlbumModel::slotResult(KJob* job)
{
    if (job != d->job)
    {
        return;
    }

    d->job = 0;

    // either of the two
    finishRefresh();
    finishIncrementalRefresh();

    if (job->error())
    {
        kWarning() << "Failed to list url: " << job->errorString();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), job->errorString(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }
}

void ImageAlbumModel::slotData(KIO::Job* job, const QByteArray& data)
{
    if (data.isEmpty() || job != d->job)
    {
        return;
    }

    ImageInfoList newItemsList;
    QByteArray    tmp(data);
    QDataStream   ds(&tmp, QIODevice::ReadOnly);

    if (d->extraValueJob)
    {
        QList<QVariant> extraValues;

        if (!ImageListerRecord::checkStream(ImageListerRecord::ExtraValueFormat, ds))
        {
            kError() << "Binary stream from ioslave is not valid, rejecting";
            return;
        }

        while (!ds.atEnd())
        {
            ImageListerRecord record(ImageListerRecord::ExtraValueFormat);
            ds >> record;

            ImageInfo info(record);
            newItemsList << info;

            if (d->specialListing == "faces")
            {
                DatabaseFace face = DatabaseFace::fromListing(info.id(), record.extraValues);
                extraValues << face.toVariant();
            }
            else
            {
                // default handling: just pass extraValue
                if (record.extraValues.isEmpty())
                {
                    extraValues  << QVariant();
                }
                else if (record.extraValues.size() == 1)
                {
                    extraValues  << record.extraValues.first();
                }
                else
                {
                    extraValues  << QVariant(record.extraValues);    // uh-uh. List in List.
                }
            }
        }

        addImageInfos(newItemsList, extraValues);
    }
    else
    {
        while (!ds.atEnd())
        {
            ImageListerRecord record;
            ds >> record;

            ImageInfo info(record);
            newItemsList << info;
        }

        addImageInfos(newItemsList);
    }
}

void ImageAlbumModel::slotImageChange(const ImageChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    // already scheduled to refresh?
    if (hasScheduledRefresh())
    {
        return;
    }

    // this is for the case that _only_ the status changes, i.e., explicit setVisible()
    if ((DatabaseFields::Images)changeset.changes() == DatabaseFields::Status)
    {
        scheduleIncrementalRefresh();
    }

    // If we list a search, a change to a property may alter the search result

    QList<Album*>::iterator it;
    /**
     * QList is designed for multiple selection, for now, only tags are supported
     * for SAlbum it will be a list with one element
     */
    for(it = d->currentAlbums.begin(); it != d->currentAlbums.end(); ++it)
    {
        if ((*it)->type() == Album::SEARCH)
        {
            SAlbum* salbum = static_cast<SAlbum*>(*it);

            bool needCheckRefresh = false;
            if (salbum->isNormalSearch())
            {
                // For searches any touched field can require a refresh.
                // We cannot easily find out which fields are searched for, so we refresh for any change.
                needCheckRefresh = true;
            }
            else if (salbum->isTimelineSearch())
            {
                if (changeset.changes() & DatabaseFields::CreationDate)
                {
                    needCheckRefresh = true;
                }
            }
            else if (salbum->isMapSearch())
            {
                if (changeset.changes() & DatabaseFields::ImagePositionsAll)
                {
                    needCheckRefresh = true;
                }
            }

            if (needCheckRefresh)
            {
                foreach(const qlonglong& id, changeset.ids())
                {
                    // if one matching image id is found, trigger a refresh
                    if (hasImage(id))
                    {
                        scheduleIncrementalRefresh();
                        break;
                    }
                }
            }
        }
    }

    ImageModel::slotImageChange(changeset);
}

void ImageAlbumModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    bool doRefresh = false;
    QList<Album*>::iterator it;

    for(it = d->currentAlbums.begin(); it != d->currentAlbums.end(); ++it)
    {
        if ((*it)->type() == Album::TAG)
        {
            doRefresh = changeset.containsTag((*it)->id());

            if (!doRefresh && d->recurseTags)
            {
                foreach(int tagId, changeset.tags())
                {
                    Album* a = AlbumManager::instance()->findTAlbum(tagId);

                    if (a && (*it)->isAncestorOf(a))
                    {
                        doRefresh = true;
                        break;
                    }
                }
            }
        }

        if(doRefresh)
        {
            break;
        }
    }

    if (doRefresh)
    {
        scheduleIncrementalRefresh();
    }

    ImageModel::slotImageTagChange(changeset);
}

void ImageAlbumModel::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    // already scheduled to refresh?
    if (d->refreshTimer->isActive())
    {
        return;
    }

    bool doRefresh = false;

    QList<Album*>::iterator it;

    for(it = d->currentAlbums.begin(); it != d->currentAlbums.end(); ++it)
    {
        switch (changeset.operation())
        {
            case CollectionImageChangeset::Added:
            {
                switch ((*it)->type())
                {
                    case Album::PHYSICAL:
                        // that's easy: try if our album is affected
                        doRefresh = changeset.containsAlbum((*it)->id());

                        if (!doRefresh && d->recurseAlbums)
                        {
                            foreach(int albumId, changeset.albums())
                            {
                                Album* a = AlbumManager::instance()->findPAlbum(albumId);

                                if (a && (*it)->isAncestorOf(a))
                                {
                                    doRefresh = true;
                                    break;
                                }
                            }
                        }

                        break;
                    default:
                        // we cannot easily know if we are affected
                        doRefresh = true;
                        break;
                }
                break;
            }
            case CollectionImageChangeset::Removed:
            case CollectionImageChangeset::RemovedAll:
                // is one of our images affected?
                foreach(const qlonglong& id, changeset.ids())
                {
                    // if one matching image id is found, trigger a refresh
                    if (hasImage(id))
                    {
                        doRefresh = true;
                        break;
                    }
                }
                break;

            default:
                break;
        }

        if(doRefresh)
            break;
    }

    if (doRefresh)
    {
        scheduleIncrementalRefresh();
    }
}

void ImageAlbumModel::slotSearchChange(const SearchChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    if (changeset.operation() != SearchChangeset::Changed)
    {
        return;
    }

    SAlbum* album = AlbumManager::instance()->findSAlbum(changeset.searchId());

    QList<Album*>::iterator it;

    for(it = d->currentAlbums.begin(); it != d->currentAlbums.end(); ++it)
    {
        if (album && (*it) == album)
        {
            scheduleIncrementalRefresh();
        }
    }
}

void ImageAlbumModel::slotAlbumAdded(Album* /*album*/)
{
}

void ImageAlbumModel::slotAlbumDeleted(Album* album)
{
    if (d->currentAlbums.contains(album))
    {
        d->currentAlbums.removeOne(album);
        clearImageInfos();
        return;
    }

    // display changed tags
    if (album->type() == Album::TAG)
    {
        emitDataChangedForAll();
    }
}

void ImageAlbumModel::slotAlbumRenamed(Album* album)
{
    // display changed names
    if(d->currentAlbums.contains(album))
    {
        emitDataChangedForAll();
    }
}

void ImageAlbumModel::slotAlbumsCleared()
{
    d->currentAlbums.clear();
    clearImageInfos();
}

} // namespace Digikam
