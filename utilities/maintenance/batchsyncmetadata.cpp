/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata from all Albums
 *               with digiKam database
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchsyncmetadata.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "album.h"
#include "collectionscanner.h"
#include "imageinfojob.h"
#include "metadatahub.h"

namespace Digikam
{

class BatchSyncMetadata::BatchSyncMetadataPriv
{
public:

    BatchSyncMetadataPriv() :
        cancel(false),
        everStarted(false),
        running(false),
        count(0),
        imageInfoIndex(0),
        album(0),
        imageInfoJob(0),
        direction(BatchSyncMetadata::WriteFromDatabaseToFile)
    {
    }

    bool                             cancel;
    bool                             everStarted;
    bool                             running;

    int                              count;
    int                              imageInfoIndex;

    Album*                           album;

    ImageInfoJob*                    imageInfoJob;

    ImageInfoList                    imageInfoList;

    CollectionScanner                scanner;

    BatchSyncMetadata::SyncDirection direction;
};

BatchSyncMetadata::BatchSyncMetadata(Album* album, SyncDirection direction, QObject* parent)
    : QObject(parent), d(new BatchSyncMetadataPriv)
{
    d->album     = album;
    d->direction = direction;

    connect(this, SIGNAL(startParsingList()),
            this, SLOT(parseList()),
            Qt::QueuedConnection);
}

BatchSyncMetadata::BatchSyncMetadata(const ImageInfoList& list, SyncDirection direction, QObject* parent)
    : QObject(parent), d(new BatchSyncMetadataPriv)
{
    d->imageInfoList = list;
    d->direction     = direction;
}

BatchSyncMetadata::~BatchSyncMetadata()
{
    delete d->imageInfoJob;
    delete d;
}

void BatchSyncMetadata::parseAlbum()
{
    d->imageInfoJob = new ImageInfoJob;
    d->imageInfoJob->allItemsFromAlbum(d->album);

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotAlbumParsed(ImageInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

void BatchSyncMetadata::slotComplete()
{
    if (!d->running)
    {
        emit startParsingList();
    }
}

void BatchSyncMetadata::slotAlbumParsed(const ImageInfoList& list)
{
    d->imageInfoList << list;

    if (!d->running && !d->cancel)
    {
        emit startParsingList();
    }
}

void BatchSyncMetadata::parseList()
{
    if (!d->everStarted)
    {
        QString message;

        if (d->direction == WriteFromDatabaseToFile)
        {
            message = i18n("Synchronizing image metadata with database. Please wait...");
        }
        else
        {
            message = i18n("Updating database from image metadata. Please wait...");
        }

        emit signalBegin(message);

        d->everStarted = true;
    }

    d->running = true;

    while (d->imageInfoIndex != d->imageInfoList.size() && !d->cancel)
    {
        parsePicture();
        kapp->processEvents();
    }

    d->running = false;

    if (d->cancel                                          ||
        (d->imageInfoJob && !d->imageInfoJob->isRunning()) ||
        !d->imageInfoJob
       )
    {
        complete();
    }
}

void BatchSyncMetadata::parsePicture()
{
    ImageInfo info = d->imageInfoList.at(d->imageInfoIndex);

    if (d->direction == WriteFromDatabaseToFile)
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info.filePath());
    }
    else
    {
        d->scanner.scanFile(info, CollectionScanner::Rescan);
    }

    emit signalProgressValue(d->count++/(float)d->imageInfoList.count());

    d->imageInfoIndex++;
}

void BatchSyncMetadata::slotAbort()
{
    d->cancel = true;

    if (d->imageInfoJob)
    {
        d->imageInfoJob->stop();
    }
}

void BatchSyncMetadata::complete()
{
    emit signalComplete();
}

}  // namespace Digikam
