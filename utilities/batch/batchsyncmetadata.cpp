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
        imageInfoJob = new ImageInfoJob();
        album        = 0;
        imageInfo    = 0; 
        duration.start();

        okPix        = KApplication::kApplication()->iconLoader()->loadIcon(
                       "button_ok", KIcon::NoGroup, 32);

        warnPix      = KApplication::kApplication()->iconLoader()->loadIcon(
                       "messagebox_info", KIcon::NoGroup, 16);
    }

    bool                   cancel;

    QTime                  duration;

    QPixmap                okPix;
    QPixmap                warnPix;

    Album                 *album;
    
    ImageInfoJob          *imageInfoJob;

    ImageInfoList          imageInfoList;
 
    ImageInfo             *imageInfo;
};

BatchSyncMetadata::BatchSyncMetadata(QWidget* parent, Album *album)
                 : DProgressDlg(parent)
{
    d = new BatchSyncMetadataPriv;
    d->album = album;
    setValue(0);
    setCaption(i18n("Sync Pictures Metadata"));
    setTitle(i18n("Parsing pictures"));
    setLabel(i18n("<b>Sync pictures metadata with digiKam database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    resize(600, 300);
    QTimer::singleShot(500, this, SLOT(slotParseAlbum()));
}

BatchSyncMetadata::BatchSyncMetadata(QWidget* parent, const ImageInfoList& list)
                 : DProgressDlg(parent)
{
    d = new BatchSyncMetadataPriv;
    d->imageInfoList = list;
    setValue(0);
    setCaption(i18n("Sync Pictures Metadata"));
    setTitle(i18n("Parsing pictures"));
    setLabel(i18n("<b>Sync pictures metadata with digiKam database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    resize(600, 300);
    QTimer::singleShot(500, this, SLOT(slotParseList()));
}

BatchSyncMetadata::~BatchSyncMetadata()
{
    delete d;
}

void BatchSyncMetadata::slotParseAlbum()
{
    d->imageInfoJob->allItemsFromAlbum(d->album);

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(const ImageInfoList&)),
            this, SLOT(slotAlbumParsed(const ImageInfoList&)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

void BatchSyncMetadata::slotAlbumParsed(const ImageInfoList& list)
{
    d->imageInfoList = list;
    slotParseList();
}

void BatchSyncMetadata::slotParseList()
{
    setTotalSteps(d->imageInfoList.count());
    d->imageInfo = d->imageInfoList.first();
    parsePicture();
}

void BatchSyncMetadata::parsePicture()
{
    if (!d->imageInfo)     // All is done.
    {
        complete();
    }
    else if (d->cancel)
    {
        abort();
    }
    else 
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(d->imageInfo);
        // write out to file DMetadata
        bool result = fileHub.write(d->imageInfo->filePath());
   
        addedAction(result ? d->okPix : d->warnPix, d->imageInfo->kurl().filename());
        advance(1);
        d->imageInfo = d->imageInfoList.next();

        kapp->processEvents();
        parsePicture();
    }
}

void BatchSyncMetadata::slotComplete()
{
    if (d->imageInfoList.isEmpty())
        complete();
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

void BatchSyncMetadata::complete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Sync pictures metadata with digiKam database done</b>"));
    setTitle(i18n("Duration: %1").arg(t.toString()));
    setButtonText(i18n("&Close"));
    setValue(100);
    abort();
}

void BatchSyncMetadata::abort()
{
    d->cancel = true;
    d->imageInfoJob->stop();
    emit signalComplete();
}

}  // namespace Digikam


