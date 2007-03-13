/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-22-01
 * Description : digikamalbum KIO slave interface to get image 
 *               info from database.
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

// Qt includes.

#include <qstring.h>
#include <qdatastream.h>

// KDE includes.

#include <kio/job.h>
#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "imageinfojob.h"
#include "imageinfojob.moc"

namespace Digikam
{

class ImageInfoJobPriv
{
public:

    ImageInfoJobPriv()
    {
        job         = 0;

        AlbumSettings *settings = AlbumSettings::instance();
        imagefilter = settings->getImageFileFilter().lower() +
                      settings->getImageFileFilter().upper() +
                      settings->getRawFileFilter().lower()   +
                      settings->getRawFileFilter().upper();
    }

    QString            imagefilter; 

    KIO::TransferJob  *job;
};

ImageInfoJob::ImageInfoJob()
{
    d = new ImageInfoJobPriv;
}

ImageInfoJob::~ImageInfoJob()
{
    delete d;
}

void ImageInfoJob::allItemsFromAlbum(Album *album)
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    if (!album)
        return;
    
    QByteArray  ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << AlbumManager::instance()->getLibraryPath();
    ds << album->kurl();
    ds << d->imagefilter;
    ds << false;            // No need image resolution info.

    // Protocol = digikamalbums -> kio_digikamalbums
    d->job = new KIO::TransferJob(album->kurl(), KIO::CMD_SPECIAL,
                                  ba, QByteArray(), false);

    connect(d->job, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotResult(KIO::Job*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void ImageInfoJob::stop()
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }
}

void ImageInfoJob::slotResult(KIO::Job* job)
{
    d->job = 0;

    if (job->error())
    {
        DWarning() << "Failed to list url: " << job->errorString() << endl;
        return;
    }

    emit signalCompleted();
}

void ImageInfoJob::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    Q_LLONG       imageID;
    int           albumID;
    QString       name;
    QString       date;
    size_t        size;
    QSize         dims;
    ImageInfoList itemsList;
    QDataStream   ds(data, IO_ReadOnly);

    while (!ds.atEnd())
    {
        ds >> imageID;
        ds >> albumID;
        ds >> name;
        ds >> date;
        ds >> size;
        ds >> dims;

        ImageInfo* info = new ImageInfo(imageID, albumID, name,
                                        QDateTime::fromString(date, Qt::ISODate),
                                        size, dims);

        itemsList.append(info);
    }

    emit signalItemsInfo(itemsList);
}

}  // namespace Digikam
