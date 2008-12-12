/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata from all Albums
 *               with digiKam database
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchsyncmetadata.h"
#include "batchsyncmetadata.moc"

// Qt includes.

#include <QString>

// KDE includes.

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "album.h"
#include "imageinfojob.h"
#include "metadatahub.h"
#include "statusprogressbar.h"

namespace Digikam
{

class BatchSyncMetadataPriv
{
public:

    BatchSyncMetadataPriv()
    {
        cancel         = false;
        imageInfoJob   = new ImageInfoJob();
        album          = 0;
        count          = 0;
        imageInfoIndex = 0;
    }

    bool                   cancel;

    int                    count;

    Album                 *album;

    ImageInfoJob          *imageInfoJob;

    ImageInfoList          imageInfoList;

    int                    imageInfoIndex;
};

BatchSyncMetadata::BatchSyncMetadata(QObject* parent, Album *album)
                 : QObject(parent), d(new BatchSyncMetadataPriv)
{
    d->album = album;
}

BatchSyncMetadata::BatchSyncMetadata(QObject* parent, const ImageInfoList& list)
                 : QObject(parent), d(new BatchSyncMetadataPriv)
{
    d->imageInfoList = list;
}

BatchSyncMetadata::~BatchSyncMetadata()
{
    delete d;
}

void BatchSyncMetadata::parseAlbum()
{
    d->imageInfoJob->allItemsFromAlbum(d->album);

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(const ImageInfoList&)),
            this, SLOT(slotAlbumParsed(const ImageInfoList&)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

void BatchSyncMetadata::slotComplete()
{
    if (d->imageInfoList.isEmpty())
        complete();
}

void BatchSyncMetadata::slotAlbumParsed(const ImageInfoList& list)
{
    d->imageInfoList = list;
    parseList();
}

void BatchSyncMetadata::parseList()
{
    emit signalProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                               i18n("Synchronizing images Metadata with database. Please wait..."));

    d->imageInfoIndex = 0;
    parsePicture();
}

void BatchSyncMetadata::parsePicture()
{
    if (d->imageInfoIndex >= d->imageInfoList.count())     // All is done.
    {
        complete();
        slotAbort();
    }
    else if (d->cancel)
    {
        complete();
    }
    else
    {
        ImageInfo info = d->imageInfoList[d->imageInfoIndex];
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info.filePath());

        emit signalProgressValue((int)((d->count++/(float)d->imageInfoList.count())*100.0));

        d->imageInfoIndex++;

        kapp->processEvents();
        parsePicture();
    }
}

void BatchSyncMetadata::slotAbort()
{
    d->cancel = true;
    d->imageInfoJob->stop();
}

void BatchSyncMetadata::complete()
{
    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
    emit signalComplete();
}

}  // namespace Digikam
