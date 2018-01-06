/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with ioslave
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "imagealbummodel.h"

// Qt includes

#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "facetagsiface.h"
#include "coredbwatch.h"
#include "coredburl.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imagelister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "dbjobsmanager.h"
#include "dbjobsthread.h"

namespace Digikam
{

class ImageAlbumModel::Private
{
public:

    Private()
    {
        jobThread               = 0;
        refreshTimer            = 0;
        incrementalTimer        = 0;
        recurseAlbums           = false;
        recurseTags             = false;
        listOnlyAvailableImages = false;
        extraValueJob           = false;
    }

    QList<Album*>     currentAlbums;
    DBJobsThread*     jobThread;
    QTimer*           refreshTimer;
    QTimer*           incrementalTimer;

    bool              recurseAlbums;
    bool              recurseTags;
    bool              listOnlyAvailableImages;
    QString           specialListing;

    bool              extraValueJob;
};

ImageAlbumModel::ImageAlbumModel(QObject* const parent)
    : ImageThumbnailModel(parent),
      d(new Private)
{
    qRegisterMetaType<QList<ImageListerRecord>>("QList<ImageListerRecord>");

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

    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
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
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = 0;
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
    foreach(Album* const a, albums)
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
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = 0;
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

    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = 0;
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
    if (d->jobThread)
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
    if (d->jobThread)
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
    if (albums.isEmpty())
    {
        return;
    }

    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = 0;
    }

    CoreDbUrl url;
    QList<int> ids;

    if (albums.first()->isTrashAlbum())
    {
        return;
    }

    if (albums.first()->type() == Album::TAG || albums.first()->type() == Album::SEARCH)
    {
        for (QList<Album*>::iterator it = albums.begin() ; it != albums.end() ; ++it)
        {
            ids << (*it)->id();
        }

        if (albums.first()->type() == Album::TAG)
        {
            url = CoreDbUrl::fromTagIds(ids);
        }
    }
    else
    {
        url = albums.first()->databaseUrl();
    }

    if (albums.first()->type() == Album::DATE)
    {
        d->extraValueJob = false;

        DatesDBJobInfo jobInfo;

        jobInfo.setStartDate( url.startDate() );
        jobInfo.setEndDate( url.endDate() );

        if (d->recurseAlbums)
            jobInfo.setRecursive();

        if (d->listOnlyAvailableImages)
            jobInfo.setListAvailableImagesOnly();

        d->jobThread = DBJobsManager::instance()->startDatesJobThread(jobInfo);
    }
    else if (albums.first()->type() == Album::TAG)
    {
        d->extraValueJob = false;

        TagsDBJobInfo jobInfo;

        if (d->listOnlyAvailableImages)
            jobInfo.setListAvailableImagesOnly();

        if (d->recurseTags)
            jobInfo.setRecursive();

        jobInfo.setTagsIds(ids);

        if (!d->specialListing.isNull())
        {
            jobInfo.setSpecialTag(d->specialListing);
            d->extraValueJob = true;
        }

        d->jobThread = DBJobsManager::instance()->startTagsJobThread(jobInfo);
    }
    else if (albums.first()->type() == Album::PHYSICAL)
    {
        d->extraValueJob = false;
        AlbumsDBJobInfo jobInfo;

        if (d->recurseAlbums)
            jobInfo.setRecursive();

        if (d->listOnlyAvailableImages)
            jobInfo.setListAvailableImagesOnly();

        jobInfo.setAlbumRootId( url.albumRootId() );
        jobInfo.setAlbum( url.album() );

        d->jobThread = DBJobsManager::instance()->startAlbumsJobThread(jobInfo);
    }
    else if (albums.first()->type() == Album::SEARCH)
    {
        d->extraValueJob = false;

        SearchesDBJobInfo jobInfo;

        if (d->listOnlyAvailableImages)
            jobInfo.setListAvailableImagesOnly();

        jobInfo.setSearchIds(ids);

        d->jobThread = DBJobsManager::instance()->startSearchesJobThread(jobInfo);
    }

    connect(d->jobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));

    connect(d->jobThread, SIGNAL(data(QList<ImageListerRecord>)),
            this, SLOT(slotData(QList<ImageListerRecord>)));
}

void ImageAlbumModel::slotResult()
{
    if (d->jobThread != sender())
    {
        return;
    }

    if (d->jobThread->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list url: "
                                       << d->jobThread->errorsList().first();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->jobThread->errorsList().first(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    d->jobThread->cancel();
    d->jobThread = 0;

    // either of the two
    finishRefresh();
    finishIncrementalRefresh();
}

void ImageAlbumModel::slotData(const QList<ImageListerRecord> &records)
{
    if (d->jobThread != sender())
        return;

    if (records.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Data From DBJobsThread is null: " << records.isEmpty();
        return;
    }

    ImageInfoList newItemsList;

    if (d->extraValueJob)
    {
        QList<QVariant> extraValues;

        foreach (const ImageListerRecord &record, records)
        {
            ImageInfo info(record);
            newItemsList << info;

            if (d->specialListing == QLatin1String("faces"))
            {
                FaceTagsIface face = FaceTagsIface::fromListing(info.id(), record.extraValues);
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
        foreach (const ImageListerRecord &record, records)
        {
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
    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        if ((*it)->type() == Album::SEARCH)
        {
            SAlbum* const salbum = static_cast<SAlbum*>(*it);

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

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        if ((*it)->type() == Album::TAG)
        {
            doRefresh = changeset.containsTag((*it)->id());

            if (!doRefresh && d->recurseTags)
            {
                foreach(int tagId, changeset.tags())
                {
                    Album* const a = AlbumManager::instance()->findTAlbum(tagId);

                    if (a && (*it)->isAncestorOf(a))
                    {
                        doRefresh = true;
                        break;
                    }
                }
            }
        }

        if (doRefresh)
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
                                Album* const a = AlbumManager::instance()->findPAlbum(albumId);

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

    SAlbum* const album = AlbumManager::instance()->findSAlbum(changeset.searchId());

    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
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
    if (d->currentAlbums.contains(album))
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
