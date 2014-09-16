/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-22-01
 * Description : interface to get image info from database.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageinfojob.moc"

// Qt includes

#include <QString>
#include <QDataStream>

// KDE includes

#include <kio/job.h>
#include <kurl.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "imagelister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"

namespace Digikam
{

class ImageInfoJob::Private
{
public:

    Private() :
        job(0)
    {
    }

    KIO::TransferJob* job;
};

ImageInfoJob::ImageInfoJob()
    : d(new Private)
{
}

ImageInfoJob::~ImageInfoJob()
{
    if (d->job)
    {
        d->job->kill();
    }

    delete d;
}

void ImageInfoJob::allItemsFromAlbum(Album* const album)
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    if (!album)
    {
        return;
    }

    ImageLister lister;
    d->job = lister.startListJob(album->databaseUrl());

    connect(d->job, SIGNAL(finished(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(d->job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotData(KIO::Job*,QByteArray)));
}

void ImageInfoJob::stop()
{
    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }
}

bool ImageInfoJob::isRunning() const
{
    return d->job;
}

void ImageInfoJob::slotResult(KJob* job)
{
    if (job->error())
    {
        kWarning() << "Failed to list url: " << job->errorString();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(), d->job->errorString(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    d->job = 0;

    emit signalCompleted();
}

void ImageInfoJob::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }

    ImageInfoList itemsList;
    QDataStream   ds(data);

    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        ImageInfo info(record);

        itemsList.append(info);
    }

    // Sort the itemList based on name
    qSort(itemsList.begin(), itemsList.end(), ImageInfoList::namefileLessThan);

    emit signalItemsInfo(itemsList);
}

}  // namespace Digikam
