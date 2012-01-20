/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "facedetector.moc"

// Qt includes

#include <QClipboard>
#include <QVBoxLayout>

// KDE includes

#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <ktextedit.h>
#include <kapplication.h>

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

class BenchmarkMessageDisplay : public QWidget
{
public:

    BenchmarkMessageDisplay(const QString& richText)
        : QWidget(0)
    {
        setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout* vbox     = new QVBoxLayout;
        KTextEdit* edit       = new KTextEdit;
        vbox->addWidget(edit, 1);
        KPushButton* okButton = new KPushButton(KStandardGuiItem::ok());
        vbox->addWidget(okButton, 0, Qt::AlignRight);

        setLayout(vbox);

        connect(okButton, SIGNAL(clicked()),
                this, SLOT(close()));

        edit->setHtml(richText);
        QApplication::clipboard()->setText(edit->toPlainText());

        resize(500, 400);
        show();
        raise();
    }
};

// --------------------------------------------------------------------------

class FaceDetector::FaceDetectorPriv
{
public:

    FaceDetectorPriv()
    {
        benchmark  = false;
        total      = 0;

        duration.start();
    }

    bool               benchmark;

    QTime              duration;
    int                total;

    AlbumPointerList<> albumTodoList;
    ImageInfoJob       albumListing;
    FacePipeline       pipeline;
};

FaceDetector::FaceDetector(const FaceScanSettings& settings)
    : ProgressItem(0,
                   "FaceDetector",
                   QString(),
                   QString(),
                   true,
                   true),
      d(new FaceDetectorPriv)
{
    if (settings.task == FaceScanSettings::RetrainAll)
    {
        KFaceIface::RecognitionDatabase::addDatabase();
        d->pipeline.plugRetrainingDatabaseFilter();
        d->pipeline.plugTrainer();
        d->pipeline.construct();
    }
    else if (settings.task == FaceScanSettings::Benchmark)
    {
        d->benchmark = true;
        d->pipeline.plugDatabaseFilter(FacePipeline::ScanAll);
        d->pipeline.plugPreviewLoader();

        if (settings.useFullCpu)
        {
            d->pipeline.plugParallelFaceDetectors();
        }
        else
        {
            d->pipeline.plugFaceDetector();
        }

        d->pipeline.plugBenchmarker();
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

    connect(&d->albumListing, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotItemsInfo(ImageInfoList)));

    connect(&d->albumListing, SIGNAL(signalCompleted()),
            this, SLOT(continueAlbumListing()));

    connect(&d->pipeline, SIGNAL(finished()),
            this, SLOT(continueAlbumListing()));

    connect(&d->pipeline, SIGNAL(processed(FacePipelinePackage)),
            this, SLOT(slotShowOneDetected(FacePipelinePackage)));

    connect(&d->pipeline, SIGNAL(skipped(QList<ImageInfo>)),
            this, SLOT(slotImagesSkipped(QList<ImageInfo>)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    if (settings.albums.isEmpty() || settings.task == FaceScanSettings::RetrainAll)
    {
        d->albumTodoList = AlbumManager::instance()->allPAlbums();
    }
    else
    {
        d->albumTodoList = settings.albums;
    }

    if (ProgressManager::addProgressItem(this))
        startAlbumListing();
}

FaceDetector::~FaceDetector()
{
    delete d;
}

void FaceDetector::startAlbumListing()
{
    setThumbnail(KIcon("edit-image-face-show").pixmap(22));
    setUsesBusyIndicator(true);

    // get total count, cached by AlbumManager
    QMap<int, int> palbumCounts, talbumCounts;
    bool hasPAlbums = false;
    bool hasTAlbums = false;

    foreach(Album* album, d->albumTodoList)
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

    foreach(Album* album, d->albumTodoList)
    {
        if (album->type() == Album::PHYSICAL)
        {
            d->total += palbumCounts.value(album->id());
        }
        else
            // this is possibly broken of course because we do not know if images have multiple tags,
            // but there's no better solution without expensive operation
        {
            d->total += talbumCounts.value(album->id());
        }
    }

    kDebug() << "Total is" << d->total;

    d->total = qMax(1, d->total);

    setUsesBusyIndicator(false);
    setLabel(i18n("Updating faces database."));
    setTotalItems(d->total);

    continueAlbumListing();
}

void FaceDetector::continueAlbumListing()
{
    kDebug() << d->albumListing.isRunning() << !d->pipeline.hasFinished();

    // we get here by the finished signal from both, and want both to have finished to continue
    if (d->albumListing.isRunning() || !d->pipeline.hasFinished())
    {
        return;
    }

    // list can have null pointer if album was deleted recently
    Album* album = 0;

    do
    {
        if (d->albumTodoList.isEmpty())
        {
            return complete();
        }

        album = d->albumTodoList.takeFirst();
    }
    while (!album);

    d->albumListing.allItemsFromAlbum(album);
}

void FaceDetector::slotItemsInfo(const ImageInfoList& items)
{
    d->pipeline.process(items);
}

void FaceDetector::complete()
{
    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Face Detection is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());
    emit signalComplete();

    if (d->benchmark)
    {
        new BenchmarkMessageDisplay(d->pipeline.benchmarkResult());
    }

    setComplete();
}

void FaceDetector::slotCancel()
{
    d->pipeline.cancel();
    setComplete();
}

void FaceDetector::slotImagesSkipped(const QList<ImageInfo>& infos)
{
    advance(infos.size());
}

void FaceDetector::slotShowOneDetected(const FacePipelinePackage& package)
{
    QPixmap pix;

    if (!package.image.isNull())
    {
        pix = package.image.smoothScale(22, 22, Qt::KeepAspectRatio).convertToPixmap();
    }
    else if (!package.faces.isEmpty())
    {
        pix = QPixmap::fromImage(package.faces.first().image().toQImage().scaled(22, 22, Qt::KeepAspectRatio));
    }

    setThumbnail(pix);
    advance(1);
}

} // namespace Digikam
