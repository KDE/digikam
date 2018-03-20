/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-22-01
 * Description : interface to get image info from database.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageinfojob.h"

// Qt includes

#include <QString>
#include <QDataStream>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "imagelister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "dbjobsmanager.h"

namespace Digikam
{

class ImageInfoJob::Private
{
public:

    Private() :
        jobThread(0)
    {
    }

    DBJobsThread* jobThread;
};

ImageInfoJob::ImageInfoJob()
    : d(new Private)
{
}

ImageInfoJob::~ImageInfoJob()
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
    }

    delete d;
}

void ImageInfoJob::allItemsFromAlbum(Album* const album)
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = 0;
    }

    if (!album)
    {
        return;
    }

    // TODO: Drop Database Url usage
    CoreDbUrl url = album->databaseUrl();

    if (album->type() == Album::DATE)
    {
        DatesDBJobInfo jobInfo;
        jobInfo.setStartDate(url.startDate());
        jobInfo.setEndDate(url.endDate());

        d->jobThread = DBJobsManager::instance()->startDatesJobThread(jobInfo);
    }
    else if (album->type() == Album::TAG)
    {
        TagsDBJobInfo jobInfo;
        // If we want to search for images with this tag, we only want the tag and not
        // all images in the tag path.
        jobInfo.setTagsIds(QList<int>() << url.tagId());

        d->jobThread = DBJobsManager::instance()->startTagsJobThread(jobInfo);
    }
    else if (album->type() == Album::PHYSICAL)
    {
        AlbumsDBJobInfo jobInfo;
        jobInfo.setAlbumRootId(url.albumRootId());
        jobInfo.setAlbum(url.album());

        d->jobThread = DBJobsManager::instance()->startAlbumsJobThread(jobInfo);
    }
    else if (album->type() == Album::SEARCH)
    {
        SearchesDBJobInfo jobInfo;
        jobInfo.setSearchId(url.searchId());

        d->jobThread = DBJobsManager::instance()->startSearchesJobThread(jobInfo);
    }

    connect(d->jobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));

    connect(d->jobThread, SIGNAL(data(QList<ImageListerRecord>)),
            this, SLOT(slotData(QList<ImageListerRecord>)));
}

void ImageInfoJob::stop()
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = 0;
    }
}

bool ImageInfoJob::isRunning() const
{
    return d->jobThread;
}

void ImageInfoJob::slotResult()
{
    if (d->jobThread != sender())
    {
        return;
    }

    if (d->jobThread->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list url: " << d->jobThread->errorsList().first();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->jobThread->errorsList().first(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    d->jobThread = 0;

    emit signalCompleted();
}

void ImageInfoJob::slotData(const QList<ImageListerRecord>& records)
{
    if (records.isEmpty())
    {
        return;
    }

    ImageInfoList itemsList;

    foreach(const ImageListerRecord& record, records)
    {
        ImageInfo info(record);
        itemsList.append(info);
    }

    // Sort the itemList based on name
    std::sort(itemsList.begin(), itemsList.end(), ImageInfoList::namefileLessThan);

    emit signalItemsInfo(itemsList);
}

}  // namespace Digikam
