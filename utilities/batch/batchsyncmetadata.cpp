/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-22-01
 * Description : batch sync picture metadata with digiKam database
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// QT includes.

#include <qstring.h>
#include <qtimer.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qdatetime.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "imageinfojob.h"
#include "metadatahub.h"
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

BatchSyncMetadata::BatchSyncMetadata(QWidget* parent)
                 : DProgressDlg(parent)
{
    d = new BatchSyncMetadataPriv;
    d->imageInfoJob = new ImageInfoJob();
    setValue(0);
    setCaption(i18n("Sync Pictures Metadata"));
    setLabel(i18n("<b>Sync pictures metadata with digiKam database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    QTimer::singleShot(500, this, SLOT(slotStart()));
    resize(600, 300);
}

BatchSyncMetadata::~BatchSyncMetadata()
{
    delete d;
}

void BatchSyncMetadata::slotStart()
{
    setTitle(i18n("Parsing all albums"));
    setTotalSteps(d->palbumList.count());

    connect(d->imageInfoJob, SIGNAL(signalAllItemsFromAlbum(const ImageInfoList&)),
            this, SLOT(slotAlbumParsed(const ImageInfoList&)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));

    // Get all digiKam albums collection pictures path.
    
    d->albumsIt = d->palbumList.begin();
    parseAlbum();
}

void BatchSyncMetadata::parseAlbum()
{
    if (d->albumsIt == d->palbumList.end())     // All is done.
    {
        QTime t;
        t = t.addMSecs(d->duration.elapsed());
        setLabel(i18n("<b>Sync pictures metadata with digiKam database done</b>"));
        setTitle(i18n("Duration: %1").arg(t.toString()));
        setButtonText(i18n("&Close"));
        advance(1);
        abort();
    }
    else if (!(*d->albumsIt)->isRoot())
    {
        d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
        DDebug() << "Sync Items from Album :" << (*d->albumsIt)->kurl().directory() << endl;
    }
    else
    {
        d->albumsIt++;
        parseAlbum();
    }
}

void BatchSyncMetadata::slotAlbumParsed(const ImageInfoList& list)
{
    QPixmap pix = KApplication::kApplication()->iconLoader()->loadIcon(
                  "folder_image", KIcon::NoGroup, 32);

    ImageInfoList imageInfoList = list;

    addedAction(pix, imageInfoList.first()->kurl().directory());

    for (ImageInfo *info = imageInfoList.first(); info; info = imageInfoList.next())
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info->filePath());
    }

    advance(1);
    d->albumsIt++;
    parseAlbum();
}

void BatchSyncMetadata::slotComplete()
{
    advance(1);
    d->albumsIt++;
    parseAlbum();
}

void BatchSyncMetadata::slotCancel()
{
    abort();
    done(Cancel);
}

void BatchSyncMetadata::closeEvent(QCloseEvent *e)
{
    abort();
    e->accept();
}

void BatchSyncMetadata::abort()
{
    d->cancel = true;
    d->imageInfoJob->stop();
    emit signalComplete();
}

}  // namespace Digikam


