/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-22-01
 * Description : batch sync pictures metadata from all Albums 
 *               with digiKam database
 *
 * Copyright 2007 by Gilles Caulier
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

// QT includes.

#include <qstring.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "imageinfojob.h"
#include "metadatahub.h"
#include "statusprogressbar.h"
#include "batchsyncmetadata.h"
#include "batchsyncmetadata.moc"

namespace Digikam
{

class BatchSyncMetadataPriv
{
public:

    BatchSyncMetadataPriv()
    {
        cancel       = false;
        imageInfoJob = new ImageInfoJob();
        album        = 0;
        count        = 0;
        imageInfo    = 0; 
    }

    bool                   cancel;

    int                    count;

    Album                 *album;
    
    ImageInfoJob          *imageInfoJob;

    ImageInfoList          imageInfoList;
 
    ImageInfo             *imageInfo;
};

BatchSyncMetadata::BatchSyncMetadata(QObject* parent, Album *album)
                 : QObject(parent)
{
    d = new BatchSyncMetadataPriv;
    d->album = album;
}

BatchSyncMetadata::BatchSyncMetadata(QObject* parent, const ImageInfoList& list)
                 : QObject(parent)
{
    d = new BatchSyncMetadataPriv;
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
                               i18n("Sync pictures Metadata with database. Please wait..."));

    d->imageInfo = d->imageInfoList.first();
    parsePicture();
}

void BatchSyncMetadata::parsePicture()
{
    if (!d->imageInfo)     // All is done.
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
        MetadataHub fileHub;
        // read in from database
        fileHub.load(d->imageInfo);
        // write out to file DMetadata
        fileHub.write(d->imageInfo->filePath());
   
        emit signalProgressValue((int)((d->count++/(float)d->imageInfoList.count())*100.0));

        d->imageInfo = d->imageInfoList.next();

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
    emit signalProgressBarMode(StatusProgressBar::TextMode, QString::null);
    emit signalComplete();
}

}  // namespace Digikam


