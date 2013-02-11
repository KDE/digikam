/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

// KDE includes

#include <kicon.h>
#include <klocale.h>
#include <kapplication.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "collectionscanner.h"
#include "imageinfojob.h"
#include "metadatahub.h"

namespace Digikam
{

class MetadataSynchronizer::Private
{

public:

    Private() :
        imageInfoIndex(0),
        imageInfoJob(0),
        direction(MetadataSynchronizer::WriteFromDatabaseToFile)
    {
    }

    int                                 imageInfoIndex;

    AlbumList                           palbumList;
    AlbumList::Iterator                 albumsIt;

    ImageInfoJob*                       imageInfoJob;

    ImageInfoList                       imageInfoList;

    CollectionScanner                   scanner;

    MetadataSynchronizer::SyncDirection direction;
};

MetadataSynchronizer::MetadataSynchronizer(SyncDirection direction, ProgressItem* const parent)
    : MaintenanceTool("MetadataSynchronizer", parent),
      d(new Private)
{
    d->palbumList = AlbumManager::instance()->allPAlbums();
    d->direction  = direction;
}

MetadataSynchronizer::MetadataSynchronizer(Album* const album, SyncDirection direction, ProgressItem* const parent)
    : MaintenanceTool("MetadataSynchronizer", parent),
      d(new Private)
{
    d->palbumList.append(album);
    d->direction = direction;
}

MetadataSynchronizer::MetadataSynchronizer(const ImageInfoList& list, SyncDirection direction, ProgressItem* const parent)
    : MaintenanceTool("MetadataSynchronizer", parent),
      d(new Private)
{
    d->imageInfoList = list;
    d->direction     = direction;
}

// Common methods ----------------------------------------------------------------------------

void MetadataSynchronizer::slotStart()
{
    MaintenanceTool::slotStart();
    d->imageInfoJob = new ImageInfoJob;

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotAlbumParsed(ImageInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotOneAlbumIsComplete()));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    if (ProgressManager::addProgressItem(this))
    {
        QTimer::singleShot(500, this, SLOT(slotParseAlbums()));
    }
}

MetadataSynchronizer::~MetadataSynchronizer()
{
    delete d->imageInfoJob;
    delete d;
}

void MetadataSynchronizer::slotCancel()
{
    d->imageInfoJob->stop();
    MaintenanceTool::slotCancel();
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
    if (canceled())
    {
        return;
    }

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

    while (d->imageInfoIndex != d->imageInfoList.size() && !canceled())
    {
        parsePicture();
        kapp->processEvents();
    }

    MaintenanceTool::slotDone();
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
