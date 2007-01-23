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
#include <qtimer.h>
#include <qdatetime.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "imageinfojob.h"
#include "metadatahub.h"
#include "batchalbumssyncmetadata.h"
#include "batchalbumssyncmetadata.moc"

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

BatchAlbumsSyncMetadata::BatchAlbumsSyncMetadata(QWidget* parent)
                       : DProgressDlg(parent)
{
    d = new BatchAlbumsSyncMetadataPriv;
    d->imageInfoJob = new ImageInfoJob();
    setValue(0);
    setCaption(i18n("Sync All Pictures Metadata"));
    setLabel(i18n("<b>Sync all pictures metadata with digiKam database. Please wait...</b>"));
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
    setTotalSteps(d->palbumList.count());

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
        setLabel(i18n("<b>Sync all pictures metadata with digiKam database done</b>"));
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

void BatchAlbumsSyncMetadata::slotAlbumParsed(const ImageInfoList& list)
{
    QPixmap pix = KApplication::kApplication()->iconLoader()->loadIcon(
                  "folder_image", KIcon::NoGroup, 32);

    ImageInfoList imageInfoList = list;

    if (!imageInfoList.isEmpty())
    {
        addedAction(pix, imageInfoList.first()->kurl().directory());
    
        for (ImageInfo *info = imageInfoList.first(); info; info = imageInfoList.next())
        {
            MetadataHub fileHub;
            // read in from database
            fileHub.load(info);
            // write out to file DMetadata
            fileHub.write(info->filePath());
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


