/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata from all Albums
 *               with digiKam database
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchalbumssyncmetadata.h"
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
#include <kpassivepopup.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imageinfojob.h"
#include "metadatahub.h"

namespace Digikam
{

class BatchAlbumsSyncMetadataPriv
{
public:

    BatchAlbumsSyncMetadataPriv()
    {
        cancel       = false;
        imageInfoJob = 0;
        palbumList   = AlbumManager::instance()->allPAlbums();
        duration.start();
    }

    bool                 cancel;

    QTime                duration;

    ImageInfoJob        *imageInfoJob;

    AlbumList            palbumList;
    AlbumList::Iterator  albumsIt;
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
    resize(600, 300);
    QTimer::singleShot(500, this, SLOT(slotStart()));
}

BatchAlbumsSyncMetadata::~BatchAlbumsSyncMetadata()
{
    delete d;
}

void BatchAlbumsSyncMetadata::slotStart()
{
    setTitle(i18n("Parsing all albums"));
    setMaximum(d->palbumList.count());

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(const ImageInfoList&)),
            this, SLOT(slotAlbumParsed(const ImageInfoList&)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));

    d->albumsIt = d->palbumList.begin();
    parseAlbum();
}

void BatchAlbumsSyncMetadata::parseAlbum()
{
    if (d->albumsIt == d->palbumList.end())     // All is done.
    {
        QTime t;
        t = t.addMSecs(d->duration.elapsed());
        setLabel(i18n("<b>The metadata of all images has been synchronized with the digiKam database.</b>"));
        setTitle(i18n("Duration: %1",t.toString()));
        setButtonText(i18n("&Close"));
        // Pop-up a message to bring user when all is done.
        KPassivePopup::message(windowTitle(), i18n("Images' metadata synchronization with database is done."), this);
        advance(1);
        abort();
    }
    else if (!(*d->albumsIt)->isRoot())
    {
        d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
        kDebug() << "Sync Items from Album :" << (*d->albumsIt)->databaseUrl().directory();
    }
    else
    {
        d->albumsIt++;
        parseAlbum();
    }
}

void BatchAlbumsSyncMetadata::slotAlbumParsed(const ImageInfoList& list)
{
    QPixmap pix = KIconLoader::global()->loadIcon("folder-image", KIconLoader::NoGroup, 32);

    ImageInfoList imageInfoList = list;

    if (!imageInfoList.isEmpty())
    {
        addedAction(pix, imageInfoList.first().fileUrl().directory());

        foreach(const ImageInfo& info, imageInfoList)
        {
            MetadataHub fileHub;
            // read in from database
            fileHub.load(info);
            // write out to file DMetadata
            fileHub.write(info.filePath());
        }
    }

    advance(1);
    d->albumsIt++;
    parseAlbum();
}

void BatchAlbumsSyncMetadata::slotComplete()
{
    advance(1);
    d->albumsIt++;
    parseAlbum();
}

void BatchAlbumsSyncMetadata::slotCancel()
{
    abort();
    done(Cancel);
}

void BatchAlbumsSyncMetadata::closeEvent(QCloseEvent *e)
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
