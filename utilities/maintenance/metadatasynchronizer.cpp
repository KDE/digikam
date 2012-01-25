/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
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

#include "metadatasynchronizer.moc"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDateTime>

// KDE includes

#include <kicon.h>
#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "collectionscanner.h"
#include "imageinfojob.h"
#include "metadatahub.h"
#include "knotificationwrapper.h"

namespace Digikam
{

class MetadataSynchronizer::MetadataSynchronizerPriv
{

public:

    MetadataSynchronizerPriv() :
        cancel(false),
        imageInfoIndex(0),
        imageInfoJob(0),
        direction(MetadataSynchronizer::WriteFromDatabaseToFile)
    {
        duration.start();
    }

    bool                                cancel;

    int                                 imageInfoIndex;

    QTime                               duration;

    AlbumList                           palbumList;
    AlbumList::Iterator                 albumsIt;

    ImageInfoJob*                       imageInfoJob;

    ImageInfoList                       imageInfoList;

    CollectionScanner                   scanner;

    MetadataSynchronizer::SyncDirection direction;
};

MetadataSynchronizer::MetadataSynchronizer(SyncDirection direction)
    : ProgressItem(0,
                   "MetadataSynchronizer",
                   QString(),
                   QString(),
                   true,
                   true),
      d(new MetadataSynchronizerPriv)
{
    d->palbumList = AlbumManager::instance()->allPAlbums();
    d->direction  = direction;

    init();
}

MetadataSynchronizer::MetadataSynchronizer(Album* album, SyncDirection direction)
    : ProgressItem(0,
                   "MetadataSynchronizer",
                   QString(),
                   QString(),
                   true,
                   true),
      d(new MetadataSynchronizerPriv)
{
    d->palbumList.append(album);
    d->direction = direction;

    init();
}

MetadataSynchronizer::MetadataSynchronizer(const ImageInfoList& list, SyncDirection direction)
    : ProgressItem(0,
                   "MetadataSynchronizer",
                    QString(),
                    QString(),
                    true,
                    true),
      d(new MetadataSynchronizerPriv)
{
    d->imageInfoList = list;
    d->direction     = direction;

    init();
}

// Common methods ----------------------------------------------------------------------------

void MetadataSynchronizer::init()
{
    d->imageInfoJob = new ImageInfoJob;

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotAlbumParsed(ImageInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotOneAlbumIsComplete()));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    if (ProgressManager::addProgressItem(this))
        QTimer::singleShot(500, this, SLOT(slotParseAlbums()));
}

MetadataSynchronizer::~MetadataSynchronizer()
{
    delete d->imageInfoJob;
    delete d;
}

void MetadataSynchronizer::slotCancel()
{
    d->cancel = true;
    d->imageInfoJob->stop();
    setComplete();
}

// Parse Albums methods ------------------------------------------------------------------

void MetadataSynchronizer::slotParseAlbums()
{
    setUsesBusyIndicator(true);
    d->albumsIt = d->palbumList.begin();
    processOneAlbum();
}

void MetadataSynchronizer::processOneAlbum()
{
    if (d->cancel) return;

    if (d->albumsIt == d->palbumList.end())     // All albums are parsed.
    {
        parseList();
        return;
    }

    d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
}

void MetadataSynchronizer::slotAlbumParsed(const ImageInfoList& list)
{
    d->imageInfoList << list;
}

void MetadataSynchronizer::slotOneAlbumIsComplete()
{
    d->albumsIt++;
    processOneAlbum();
}

// Parse info list methods -----------------------------------------------------------------------

void MetadataSynchronizer::parseList()
{
    setUsesBusyIndicator(false);

    if (d->direction == WriteFromDatabaseToFile)
    {
        setLabel(i18n("Synchronizing image metadata with database"));
        setThumbnail(KIcon("document-edit").pixmap(22));
    }
    else
    {
        setLabel(i18n("Updating database from image metadata"));
        setThumbnail(KIcon("edit-redo").pixmap(22));
    }

    setTotalItems(d->imageInfoList.count());

    while (d->imageInfoIndex != d->imageInfoList.size() && !d->cancel)
    {
        parsePicture();
        kapp->processEvents();
    }

    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Process is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());
    emit signalComplete();
    setComplete();
}

// TODO : use multithreading to process this method.
void MetadataSynchronizer::parsePicture()
{
    ImageInfo   info = d->imageInfoList.at(d->imageInfoIndex);
    MetadataHub fileHub;

    if (d->direction == WriteFromDatabaseToFile)
    {
        // read in from database
        fileHub.load(info);

        // write out to file DMetadata
        fileHub.write(info.filePath());
    }
    else
    {
        d->scanner.scanFile(info, CollectionScanner::Rescan);
    }

    advance(1);
    d->imageInfoIndex++;
}

}  // namespace Digikam
