/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "facedetector.moc"

// Qt includes

#include <QClipboard>
#include <QVBoxLayout>
#include <QTimer>

// KDE includes

#include <kicon.h>
#include <kconfig.h>
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
#include "facescansettings.h"
#include "imageinfo.h"
#include "imageinfojob.h"

namespace Digikam
{

class BenchmarkMessageDisplay : public QWidget
{
public:

    explicit BenchmarkMessageDisplay(const QString& richText)
        : QWidget(0)
    {
        setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout* const vbox     = new QVBoxLayout;
        KTextEdit* const edit       = new KTextEdit;
        vbox->addWidget(edit, 1);
        KPushButton* const okButton = new KPushButton(KStandardGuiItem::ok());
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

class FaceDetector::Private
{
public:

    Private()
        : benchmark(false),
          total(0),
          progressValue(0),
          currentProgressChunk(0),
          currentScheduled(0),
          currentFinished(0)
    {
    }

    bool                benchmark;

    int                 total;

    AlbumPointerList<>  albumTodoList;
    ImageInfoJob        albumListing;
    FacePipeline        pipeline;
    QMap<Album*,double> relativeProgressValue;
    double              progressValue;
    double              currentProgressChunk;
    int                 currentScheduled;
    int                 currentFinished;
};

FaceDetector::FaceDetector(const FaceScanSettings& settings, ProgressItem* const parent)
    : MaintenanceTool("FaceDetector", parent),
      d(new Private)
{
    setLabel(i18n("Updating faces database."));
    ProgressManager::addProgressItem(this);

    if (settings.task == FaceScanSettings::RetrainAll)
    {
        // clear all training data in the database
        KFaceIface::RecognitionDatabase::addDatabase().clearAllTraining("digikam");
        d->pipeline.plugRetrainingDatabaseFilter();
        d->pipeline.plugTrainer();
        d->pipeline.construct();
    }
    else if (settings.task == FaceScanSettings::BenchmarkDetection)
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

        d->pipeline.plugDetectionBenchmarker();
        d->pipeline.construct();
    }
    else if (settings.task == FaceScanSettings::BenchmarkRecognition)
    {
        d->benchmark = true;
        d->pipeline.plugRetrainingDatabaseFilter();
        d->pipeline.plugFaceRecognizer();
        d->pipeline.plugRecognitionBenchmarker();
        d->pipeline.construct();
    }
    else if ((settings.task == FaceScanSettings::DetectAndRecognize) || 
             (settings.task == FaceScanSettings::Detect))
    {
        FacePipeline::FilterMode filterMode;
        FacePipeline::WriteMode  writeMode;

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

        d->pipeline.plugDatabaseFilter(filterMode);
        d->pipeline.plugPreviewLoader();

        if (settings.useFullCpu)
        {
            d->pipeline.plugParallelFaceDetectors();
        }
        else
        {
            d->pipeline.plugFaceDetector();
        }

        if(settings.task == FaceScanSettings::DetectAndRecognize)
        {
            d->pipeline.plugRerecognizingDatabaseFilter();
            d->pipeline.plugFaceRecognizer();
        }
        d->pipeline.plugDatabaseWriter(writeMode);
        d->pipeline.setDetectionAccuracy(settings.accuracy);
        d->pipeline.construct();
    }
    else // if (settings.task == FaceScanSettings::RecognizeMarkedFaces)
    {
        d->pipeline.plugRerecognizingDatabaseFilter();
        d->pipeline.plugFaceRecognizer();
        d->pipeline.plugDatabaseWriter(FacePipeline::NormalWrite);
        d->pipeline.setDetectionAccuracy(settings.accuracy);
        d->pipeline.construct();
    }

    connect(&d->albumListing, SIGNAL(signalItemsInfo(ImageInfoList)),
            this, SLOT(slotItemsInfo(ImageInfoList)));

    connect(&d->albumListing, SIGNAL(signalCompleted()),
            this, SLOT(slotContinueAlbumListing()));

    connect(&d->pipeline, SIGNAL(finished()),
            this, SLOT(slotContinueAlbumListing()));

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
}

FaceDetector::~FaceDetector()
{
    delete d;
}

void FaceDetector::slotStart()
{
    MaintenanceTool::slotStart();

    setThumbnail(KIcon("edit-image-face-show").pixmap(22));
    setUsesBusyIndicator(true);

    // get total count, cached by AlbumManager
    QMap<int, int> palbumCounts, talbumCounts;
    bool hasPAlbums = false;
    bool hasTAlbums = false;

    foreach(Album* const album, d->albumTodoList)
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

    // first, we use the relativeProgressValue map to store absolute counts
    foreach(Album* const album, d->albumTodoList)
    {
        if (album->type() == Album::PHYSICAL)
        {
            d->relativeProgressValue[album] = palbumCounts.value(album->id());
        }
        else
            // this is possibly broken of course because we do not know if images have multiple tags,
            // but there's no better solution without expensive operation
        {
            d->relativeProgressValue[album] = talbumCounts.value(album->id());
        }
    }
    // second, calculate (approximate) overall sum
    d->total = 0;
    foreach (double count, d->relativeProgressValue)
    {
        d->total += (int)count;
    }
    d->total = qMax(1, d->total);
    kDebug() << "Total is" << d->total;
    // third, break absolute to relative values
    for (QMap<Album*,double>::iterator it = d->relativeProgressValue.begin(); it != d->relativeProgressValue.end(); ++it)
    {
        it.value() /= double(d->total);
    }

    setUsesBusyIndicator(false);
    setTotalItems(d->total);

    slotContinueAlbumListing();
}

void FaceDetector::slotContinueAlbumListing()
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
            return slotDone();
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

void FaceDetector::slotDone()
{
    if (d->benchmark)
    {
        new BenchmarkMessageDisplay(d->pipeline.benchmarkResult());
    }

    // Switch on scanned for faces flag on digiKam config file.
    KGlobal::config()->group("General Settings").writeEntry("Face Scanner First Run", true);

    MaintenanceTool::slotDone();
}

void FaceDetector::slotCancel()
{
    d->pipeline.shutDown();
    MaintenanceTool::slotCancel();
}

void FaceDetector::slotImagesSkipped(const QList<ImageInfo>& infos)
{
    advance(infos.size());
}

void FaceDetector::slotShowOneDetected(const FacePipelinePackage& /*package*/)
{
    //TODO: Embedded images are gone. Needs to be solved by loading thumbnails
    /*QPixmap pix;

    if (!package.faces.isEmpty())
    {
        pix = QPixmap::fromImage(package.faces.first().image().toQImage().scaled(22, 22, Qt::KeepAspectRatio));
    }
    else if (!package.image.isNull())
    {
        pix = package.image.smoothScale(22, 22, Qt::KeepAspectRatio).convertToPixmap();
    }

    setThumbnail(pix);
    */
    advance(1);
}

} // namespace Digikam
