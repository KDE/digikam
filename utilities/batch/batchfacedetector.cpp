/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchfacedetector.moc"

// Qt includes

#include <QCloseEvent>

// KDE includes

#include <klocale.h>
#include <kstandardguiitem.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "facepipeline.h"
#include "imageinfo.h"
#include "imageinfojob.h"
#include "knotificationwrapper.h"

namespace Digikam
{

class BatchFaceDetector::BatchFaceDetectorPriv
{
public:

    BatchFaceDetectorPriv()
    {
        cancel              = false;
        rebuildAll          = true;

        duration.start();
    }

    bool                  cancel;
    bool                  rebuildAll;

    QTime                 duration;

    AlbumList             albumTodoList;
    ImageInfoJob          albumListing;
    FacePipeline          pipeline;
};

BatchFaceDetector::BatchFaceDetector(QWidget* /*parent*/, bool rebuildAll)
                 : DProgressDlg(0), d(new BatchFaceDetectorPriv)
{
    setModal(false);
    setValue(0);
    setCaption(d->rebuildAll ? i18n("Rebuild All Faces") : i18n("Build Missing Faces"));
    setLabel(i18n("<b>Updating faces database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));

    d->pipeline.plugDatabaseFilter(rebuildAll ? FacePipeline::RescanAll : FacePipeline::SkipAlreadyScanned);
    d->pipeline.plugPreviewLoader();
    d->pipeline.plugParallelFaceDetectors();
    d->pipeline.plugFaceRecognizer();
    d->pipeline.plugDatabaseWriter();
    d->pipeline.construct();

    connect(&d->albumListing, SIGNAL(signalItemsInfo(const ImageInfoList&)),
            this, SLOT(slotItemsInfo(const ImageInfoList&)));

    connect(&d->albumListing, SIGNAL(signalCompleted()),
            this, SLOT(continueAlbumListing()));

    connect(&d->pipeline, SIGNAL(finished()),
            this, SLOT(continueAlbumListing()));

    connect(&d->pipeline, SIGNAL(processed(const FacePipelinePackage&)),
            this, SLOT(slotShowOneDetected(const FacePipelinePackage&)));

    connect(&d->pipeline, SIGNAL(skipped(const QList<ImageInfo>&)),
            this, SLOT(slotImagesSkipped(const QList<ImageInfo>&)));

    startAlbumListing();
}

BatchFaceDetector::~BatchFaceDetector()
{
    delete d;
}

void BatchFaceDetector::startAlbumListing()
{
    d->albumTodoList << AlbumManager::instance()->findPAlbum(225); //= AlbumManager::instance()->allPAlbums();

    // get total count, cached by AlbumManager
    QMap<int, int> palbumCounts = AlbumManager::instance()->getPAlbumsCount();
    int total = 0;
    foreach (Album *album, d->albumTodoList)
        total += palbumCounts.value(album->id());
    kDebug() << "Total is" << total;
    setMaximum(total);

    continueAlbumListing();
}

void BatchFaceDetector::continueAlbumListing()
{
    kDebug() << d->albumListing.isRunning() << !d->pipeline.hasFinished();
    // we get here by the finished signal from both, and want both to have finished to continue
    if (d->albumListing.isRunning() || !d->pipeline.hasFinished())
        return;

    if (d->albumTodoList.isEmpty())
        return complete();

    Album *album = d->albumTodoList.takeFirst();
    d->albumListing.allItemsFromAlbum(album);
}

void BatchFaceDetector::slotItemsInfo(const ImageInfoList& items)
{
    kDebug() << items.size();
    d->pipeline.process(items);
}

void BatchFaceDetector::complete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Scanning for people completed.</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    setButtonGuiItem(KStandardGuiItem::ok());
    setButtonText(i18n("&Close"));
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper("batchfacedetectioncompleted", i18n("The face detected database has been updated."),
                         this, windowTitle());
    emit signalDetectAllFacesDone();
}

void BatchFaceDetector::slotCancel()
{
    abort();
    done(Cancel);
}

void BatchFaceDetector::closeEvent(QCloseEvent* e)
{
    abort();
    e->accept();
}

void BatchFaceDetector::abort()
{
    d->cancel = true;
    emit signalDetectAllFacesDone();
}

void BatchFaceDetector::slotImagesSkipped(const QList<ImageInfo>& infos)
{
    advance(infos.size());
}

void BatchFaceDetector::slotShowOneDetected(const FacePipelinePackage& package)
{
    QPixmap pix = package.image.smoothScale(128, 128, Qt::KeepAspectRatio).convertToPixmap();
    addedAction(pix, package.info.filePath());
    advance(1);
}

} // namespace Digikam
