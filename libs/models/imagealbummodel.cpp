/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with ioslave
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "imagealbummodel.moc"

// Qt includes.

#include <QTimer>

// KDE includes.

#include <kio/job.h>
#include <kdebug.h>

// Local includes.

#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databasewatch.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imagelister.h"

namespace Digikam
{

class ImageAlbumModelPriv
{
public:

    ImageAlbumModelPriv()
    {
        currentAlbum        = 0;
        job                 = 0;
        refreshTimer        = 0;
    }

    Album                   *currentAlbum;
    KIO::TransferJob        *job;
    QTimer                  *refreshTimer;

    bool                     recurseAlbums;
    bool                     recurseTags;
};

ImageAlbumModel::ImageAlbumModel(QObject *parent)
               : ImageModel(parent),
      d(new ImageAlbumModelPriv)
{
    d->refreshTimer = new QTimer(this);
    d->refreshTimer->setSingleShot(true);

    d->recurseAlbums = AlbumSettings::instance()->getRecurseAlbums();
    d->recurseTags   = AlbumSettings::instance()->getRecurseTags();

    connect(d->refreshTimer, SIGNAL(timeout()),
            this, SLOT(slotNextRefresh()));

    connect(AlbumSettings::instance(), SIGNAL(recurseSettingsChanged()),
            this, SLOT(slotRecurseSettingsChanged()));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset &)),
            this, SLOT(slotCollectionImageChange(const CollectionImageChangeset &)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(searchChange(const SearchChangeset &)),
            this, SLOT(slotSearchChange(const SearchChangeset &)));

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

Album *ImageAlbumModel::currentAlbum() const
{
    return d->currentAlbum;
}

void ImageAlbumModel::openAlbum(Album *album)
{
    if (d->currentAlbum == album)
        return;

    startLoadingAlbum(album);
}

void ImageAlbumModel::refresh()
{
    startLoadingAlbum(d->currentAlbum);
}

void ImageAlbumModel::startLoadingAlbum(Album *album)
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    clearImageInfos();
    emit listedAlbumChanged(album);

    if (!album)
        return;

    startListJob(album->databaseUrl());
}

void ImageAlbumModel::startListJob(const KUrl &url)
{
    d->job = ImageLister::startListJob(url);
    d->job->addMetaData("listAlbumsRecursively", d->recurseAlbums ? "true" : "false");
    d->job->addMetaData("listTagsRecursively", d->recurseTags ? "true" : "false");

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void ImageAlbumModel::slotResult(KJob* job)
{
    d->job = 0;

    if (job->error())
    {
        kWarning(50003) << "Failed to list url: " << job->errorString() << endl;
        return;
    }
}

void ImageAlbumModel::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    ImageInfoList newItemsList;

    QByteArray tmp(data);
    QDataStream ds(&tmp, QIODevice::ReadOnly);

    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        ImageInfo info(record);
        newItemsList << info;
    }

    addImageInfos(newItemsList);
}

void ImageAlbumModel::slotRecurseSettingsChanged()
{
    d->recurseAlbums = AlbumSettings::instance()->getRecurseAlbums();
    d->recurseTags   = AlbumSettings::instance()->getRecurseTags();
    refresh();
}

void ImageAlbumModel::slotNextRefresh()
{
    // Refresh, unless job is running, then postpone restart until job is finished
    // Rationale: Let the job run, don't stop it possibly several times
    if (d->job)
        d->refreshTimer->start(50);
    else
        refresh();
}

void ImageAlbumModel::slotCollectionImageChange(const CollectionImageChangeset &changeset)
{
    if (!d->currentAlbum)
        return;

    bool doRefresh = false;

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
            switch(d->currentAlbum->type())
            {
                case Album::PHYSICAL:
                    // that's easy: try if our album is affected
                    doRefresh = changeset.containsAlbum(d->currentAlbum->id());
                    break;
                default:
                    // we cannot easily know if we are affected
                    doRefresh = true;
                    break;
            }

        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
            // is one of our images affected?
            foreach(qlonglong id, changeset.ids())
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

    if (doRefresh)
    {
        // use timer: there may be several signals in a row
        if (!d->refreshTimer->isActive())
            d->refreshTimer->start(100);
    }
}

void ImageAlbumModel::slotSearchChange(const SearchChangeset &changeset)
{
    if (!d->currentAlbum)
        return;

    if (changeset.operation() != SearchChangeset::Changed)
        return;

    SAlbum *album = AlbumManager::instance()->findSAlbum(changeset.searchId());

    if (album && d->currentAlbum == album)
    {
        if (!d->refreshTimer->isActive())
            d->refreshTimer->start(100);
    }
}

} // namespace Digikam
