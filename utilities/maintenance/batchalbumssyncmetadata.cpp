/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata from all Albums
 *               with digiKam database
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchalbumssyncmetadata.moc"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QPixmap>
#include <QCloseEvent>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imageinfojob.h"
#include "metadatahub.h"
#include "knotificationwrapper.h"

namespace Digikam
{

enum BatchAlbumsStatus
{
    NotRunning,
    LoadingAlbum,
    ReceivingImageInfos,
    CompletedAlbum
};

class BatchAlbumsSyncMetadata::BatchAlbumsSyncMetadataPriv
{
public:

    BatchAlbumsSyncMetadataPriv() :
        cancel(false),
        imageInfoJob(0),
        status(NotRunning)
    {
    }

    bool                 cancel;

    QTime                duration;

    ImageInfoJob*        imageInfoJob;

    AlbumList            palbumList;
    AlbumList::Iterator  albumsIt;
    BatchAlbumsStatus    status;
};

BatchAlbumsSyncMetadata::BatchAlbumsSyncMetadata(QWidget* /*parent*/)
    : DProgressDlg(0), d(new BatchAlbumsSyncMetadataPriv)
{
    d->imageInfoJob = new ImageInfoJob();

    setModal(false);
    setValue(0);
    setCaption(i18n("Sync All Images' Metadata"));
    setLabel(i18n("<b>Syncing the metadata of all images with the digiKam database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    QTimer::singleShot(0, this, SLOT(slotStart()));

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotAlbumItemsInfo(ImageInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

BatchAlbumsSyncMetadata::~BatchAlbumsSyncMetadata()
{
    delete d;
}

void BatchAlbumsSyncMetadata::slotStart()
{
    d->palbumList   = AlbumManager::instance()->allPAlbums();
    d->duration.start();

    setTitle(i18n("Parsing all albums"));
    setMaximum(d->palbumList.count());

    d->albumsIt = d->palbumList.begin();
    parseAlbum();
}

void BatchAlbumsSyncMetadata::parseAlbum()
{
    if (d->status == CompletedAlbum)
    {
        advance(1);
        d->albumsIt++;
    }

    if (d->albumsIt == d->palbumList.end())     // All is done.
    {
        QTime t;
        t = t.addMSecs(d->duration.elapsed());
        setLabel(i18n("<b>The metadata of all images has been synchronized with the digiKam database.</b>"));
        setTitle(i18n("Duration: %1",t.toString()));
        setButtonText(i18n("&Close"));
        // Pop-up a message to bring user when all is done.
        KNotificationWrapper("batchalbumssyncmetadatacompleted",
                             i18n("Images' metadata synchronization with database is done."),
                             this, windowTitle());
        advance(1);
        abort();
        return;
    }

    if ((*d->albumsIt)->isRoot())
    {
        d->status = CompletedAlbum;
        parseAlbum();
        return;
    }

    d->status = LoadingAlbum;
    d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
    kDebug() << "Sync Items from Album :" << (*d->albumsIt)->title();
}

void BatchAlbumsSyncMetadata::slotAlbumItemsInfo(const ImageInfoList& list)
{
    if (d->status == LoadingAlbum)
    {
        d->status = ReceivingImageInfos;

        if (!list.isEmpty())
        {
            QPixmap pix = KIconLoader::global()->loadIcon("folder-image", KIconLoader::NoGroup, 32);
            addedAction(pix, list.first().fileUrl().directory());
        }
    }

    foreach(const ImageInfo& info, list)
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info.filePath());
    }
}

void BatchAlbumsSyncMetadata::slotComplete()
{
    d->status = CompletedAlbum;
    parseAlbum();
}

void BatchAlbumsSyncMetadata::slotCancel()
{
    abort();
    done(Cancel);
}

void BatchAlbumsSyncMetadata::closeEvent(QCloseEvent* e)
{
    abort();
    e->accept();
}

void BatchAlbumsSyncMetadata::abort()
{
    d->cancel = true;
    d->imageInfoJob->stop();
    emit signalComplete();
}

}  // namespace Digikam
