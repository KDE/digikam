/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QApplication>
#include <QCloseEvent>

// KDE includes

#include <klocale.h>
#include <kstandardguiitem.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// KFace includes

#include <libkface/recognitiondatabase.h>

// Local includes

#include "albumdb.h"
#include "album.h"
#include "albummanager.h"
#include "facepipeline.h"
#include "facescandialog.h"
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
        rebuildAll = true;
        total      = 0;

        duration.start();
    }

    bool         rebuildAll;

    QTime        duration;
    int          total;

    AlbumList    albumTodoList;
    ImageInfoJob albumListing;
    FacePipeline pipeline;
};

BatchFaceDetector::BatchFaceDetector(QWidget* /*parent*/, const FaceScanSettings& settings)
    : DProgressDlg(0), d(new BatchFaceDetectorPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
    setValue(0);
    setCaption(i18nc("@title:window", "Scanning Faces"));
    setLabel(i18n("<b>Updating faces database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));

    if (settings.task == FaceScanSettings::RetrainAll)
    {
        KFaceIface::RecognitionDatabase::addDatabase();
        d->pipeline.plugRetrainingDatabaseFilter();
        d->pipeline.plugTrainer();
        d->pipeline.construct();
    }
    else
    {
        FacePipeline::FilterMode filterMode;
        FacePipeline::WriteMode  writeMode;

        if (settings.task == FaceScanSettings::DetectAndRecognize)
        {
            if (settings.alreadyScannedHandling == FaceScanSettings::Skip)
            {
                filterMode = FacePipeline::SkipAlreadyScanned;
                writeMode  = FacePipeline::NormalWrite;
            }
            else if (settings.alreadyScannedHandling == FaceScanSettings::Rescan)
            {
                filterMode = FacePipeline::ScanAll;
                writeMode  = FacePipeline::OverwriteUnconfirmed;
            }
            else // if (settings.alreadyScannedHandling == FaceScanSettings::Merge)
            {
                filterMode = FacePipeline::ScanAll;
                writeMode  = FacePipeline::NormalWrite;
            }
        }
        else // if (settings.task == FaceScanSettings::RecognizeMarkedFaces)
        {
            filterMode = FacePipeline::ReadUnconfirmedFaces;
            writeMode  = FacePipeline::NormalWrite;
        }

        d->pipeline.plugDatabaseFilter(filterMode);

        if (settings.task == FaceScanSettings::DetectAndRecognize)
        {
            d->pipeline.plugPreviewLoader();

            if (settings.useFullCpu)
            {
                d->pipeline.plugParallelFaceDetectors();
            }
            else
            {
                d->pipeline.plugFaceDetector();
            }
        }

        d->pipeline.plugFaceRecognizer();
        d->pipeline.plugDatabaseWriter(writeMode);
        d->pipeline.construct();

        d->pipeline.setDetectionAccuracy(settings.accuracy);
    }

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

    if (settings.albums.isEmpty() || settings.task == FaceScanSettings::RetrainAll)
    {
        d->albumTodoList = AlbumManager::instance()->allPAlbums();
    }
    else
    {
        d->albumTodoList = settings.albums;
    }

    startAlbumListing();
}

BatchFaceDetector::~BatchFaceDetector()
{
    delete d;
}

void BatchFaceDetector::startAlbumListing()
{
    // get total count, cached by AlbumManager
    QMap<int, int> palbumCounts, talbumCounts;
    bool hasPAlbums = false, hasTAlbums = false;

    foreach (Album* album, d->albumTodoList)
    {
        if (album->type() == Album::PHYSICAL)
        {
            hasPAlbums = true;
        }
        else
        {
            hasTAlbums = true;
        }
    }

    palbumCounts = AlbumManager::instance()->getPAlbumsCount();
    talbumCounts = AlbumManager::instance()->getTAlbumsCount();

    if (palbumCounts.isEmpty() && hasPAlbums)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        palbumCounts = DatabaseAccess().db()->getNumberOfImagesInAlbums();
        QApplication::restoreOverrideCursor();
    }

    if (talbumCounts.isEmpty() && hasTAlbums)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        talbumCounts = DatabaseAccess().db()->getNumberOfImagesInTags();
        QApplication::restoreOverrideCursor();
    }

    d->total = 0;
    foreach (Album* album, d->albumTodoList)
    {
        if (album->type() == Album::PHYSICAL)
        {
            d->total += palbumCounts.value(album->id());
        }
        else
            // this is possibly broken of course because we dont know if images have multiple tags,
            // but there's no better solution without expensive operation
        {
            d->total += talbumCounts.value(album->id());
        }
    }
    kDebug() << "Total is" << d->total;
    d->total = qMax(1, d->total);
    setMaximum(d->total);

    continueAlbumListing();
}

void BatchFaceDetector::continueAlbumListing()
{
    kDebug() << d->albumListing.isRunning() << !d->pipeline.hasFinished();

    // we get here by the finished signal from both, and want both to have finished to continue
    if (d->albumListing.isRunning() || !d->pipeline.hasFinished())
    {
        return;
    }

    if (d->albumTodoList.isEmpty())
    {
        return complete();
    }

    Album* album = d->albumTodoList.takeFirst();
    kDebug() << "Album" << album->title();
    d->albumListing.allItemsFromAlbum(album);
}

void BatchFaceDetector::slotItemsInfo(const ImageInfoList& items)
{
    d->pipeline.process(items);
}

void BatchFaceDetector::complete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Scanning for people completed.</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    // set value to be sure in case of scanning for tags and total was too large
    setValue(d->total);
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
    d->pipeline.cancel();
}

void BatchFaceDetector::slotImagesSkipped(const QList<ImageInfo>& infos)
{
    advance(infos.size());
}

void BatchFaceDetector::slotShowOneDetected(const FacePipelinePackage& package)
{
    QPixmap pix;

    if (!package.image.isNull())
    {
        pix = package.image.smoothScale(128, 128, Qt::KeepAspectRatio).convertToPixmap();
    }
    else if (!package.faces.isEmpty())
    {
        pix = QPixmap::fromImage(package.faces.first().image().toQImage().scaled(128, 128, Qt::KeepAspectRatio));
    }

    addedAction(pix, package.info.filePath());
    advance(1);
}

} // namespace Digikam
