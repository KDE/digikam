/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPixmap>
#include <QCloseEvent>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <klocale.h>
#include <kstandardguiitem.h>
#include <kdebug.h>

// Libkface includes

#include <libkface/database.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albuminfo.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "imageinfo.h"
#include "previewloadthread.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "knotificationwrapper.h"
#include "config-digikam.h"

namespace Digikam
{

class BatchFaceDetectorPriv
{
public:

    BatchFaceDetectorPriv()
    {
        cancel            = false;
        rebuildAll        = true;
        previewLoadThread = 0;
        faceIface         = 0;
        duration.start();
    }

    bool                  cancel;
    bool                  rebuildAll;

    QTime                 duration;

    QStringList           allPicturesPath;

    PreviewLoadThread*    previewLoadThread;

    KFaceIface::Database* faceIface;
};

BatchFaceDetector::BatchFaceDetector(QWidget* /*parent*/, bool rebuildAll)
                 : DProgressDlg(0), d(new BatchFaceDetectorPriv)
{
    d->rebuildAll        = rebuildAll;
    d->previewLoadThread = new PreviewLoadThread();
    
    // For now, start the faceIface in detection mode
    d->faceIface = new KFaceIface::Database(KFaceIface::Database::InitDetection);

    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotGotImagePreview(const LoadingDescription&, const DImg&)), Qt::DirectConnection);

    setModal(false);
    setValue(0);
    setCaption(d->rebuildAll ? i18n("Rescan all images for faces") : i18n("Rescan all changed images for faces"));
    setLabel(i18n("<b>Updating face database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));

    QTimer::singleShot(500, this, SLOT(slotDetectFaces()));
}

BatchFaceDetector::~BatchFaceDetector()
{
    delete d;
}

void BatchFaceDetector::slotDetectFaces()
{
    setTitle(i18n("Processing..."));
    const AlbumList palbumList = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    if (d->rebuildAll)
    {
        for (AlbumList::ConstIterator it = palbumList.constBegin();
             !d->cancel && (it != palbumList.constEnd()); ++it)
        {
            d->allPicturesPath += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
        }
    }
    else
    {
        // FIXME: Currently the table query is only implemented in AlbumDB. Need to implement it in other files too
        d->allPicturesPath = DatabaseAccess().db()->getDirtyOrMissingFaceImageUrls();
    }

    setMaximum(d->allPicturesPath.count());

    if (d->allPicturesPath.isEmpty())
    {
        slotCancel();
        return;
    }

    processOne();
}

void BatchFaceDetector::processOne()
{
    if (d->cancel) return;
    QString path = d->allPicturesPath.first();
    d->previewLoadThread->loadHighQuality(path, AlbumSettings::instance()->getExifRotate());
}

void BatchFaceDetector::complete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Face scanning complete.</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    setButtonGuiItem(KStandardGuiItem::ok());
    setButtonText(i18n("&Close"));
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper("facescanningcomplete", i18n("Face scanning complete."),
                         this, windowTitle());
    emit signalDetectAllFacesDone();
}

void BatchFaceDetector::slotGotImagePreview(const LoadingDescription& desc, const DImg& img)
{
    if (d->allPicturesPath.isEmpty())
        return;

    if (d->allPicturesPath.first() != desc.filePath)
        return;

    if (!img.isNull())
    {
        // FIXME: Detect faces from the DImg here ?
        QList<KFaceIface::Face> faceList = d->faceIface->detectFaces(desc.filePath);
        QListIterator<KFaceIface::Face> it(faceList);
        kDebug()<<"Faces detected in "<<desc.filePath;
        while(it.hasNext())
            kDebug()<<it.next();
    }
    
    
//     QPixmap pix = DImg(img).smoothScale(128, 128, Qt::KeepAspectRatio).convertToPixmap();
//     addedAction(pix, desc.filePath);
    advance(1);
    if (!d->allPicturesPath.isEmpty())
        d->allPicturesPath.removeFirst();
    if (d->allPicturesPath.isEmpty())
        complete();
    else
        processOne();
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

} // namespace Digikam
