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

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QCloseEvent>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <klocale.h>
#include <kstandardguiitem.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Libkface includes

#include <libkface/kface.h>

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
#include "searchxml.h"
#include "tagproperties.h"
#include "tagscache.h"
#include "imagetagpair.h"
#include "faceiface.h"

/*
#include <libs/database/tagproperties.h>
#include <libs/database/imagetagpair.h>
#include <libs/dimg/dimg.h>
#include <libs/threadimageio/loadingdescription.h>
#include <libs/database/imageinfo.h>*/
// #include <libs/database/imagetagpair.h>
// #include <libs/database/searchxml.h>
// #include <libs/database/tagscache.h>

namespace Digikam
{

class BatchFaceDetector::BatchFaceDetectorPriv
{
public:

    BatchFaceDetectorPriv()
    {
        cancel              = false;
        rebuildAll          = true;
        previewLoadThread   = 0;
        thumbnailLoadThread = 0;
        
        faceIface           = new FaceIface();
        
        duration.start();
    }

    ~BatchFaceDetectorPriv()
    {
        delete faceIface;
    }

    bool                  cancel;
    bool                  rebuildAll;

    QTime                 duration;

    QStringList           allPicturesPath;
    PreviewLoadThread*    previewLoadThread;
    ThumbnailLoadThread*  thumbnailLoadThread;
    
    FaceIface* faceIface;
};

BatchFaceDetector::BatchFaceDetector(QWidget* /*parent*/, bool rebuildAll)
                 : DProgressDlg(0), d(new BatchFaceDetectorPriv)
{
    d->rebuildAll          = rebuildAll;
    d->previewLoadThread   = new PreviewLoadThread();
    d->thumbnailLoadThread = ThumbnailLoadThread::defaultThread();
    d->thumbnailLoadThread->setThumbnailSize(256, true);

    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotGotImagePreview(const LoadingDescription&, const DImg&)), Qt::DirectConnection);

    connect(this, SIGNAL(signalOneDetected(const LoadingDescription&, const DImg&)),
            this, SLOT(slotShowOneDetected(const LoadingDescription&, const DImg&)));

    setModal(false);
    setValue(0);
    setCaption(d->rebuildAll ? i18n("Rebuild All Faces") : i18n("Build Missing Faces"));
    setLabel(i18n("<b>Updating faces database. Please wait...</b>"));
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
    
    QStringList pathList;
    
    for (AlbumList::ConstIterator it = palbumList.constBegin();
            !d->cancel && (it != palbumList.constEnd()); ++it)
    {
        pathList += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
    }

    if (d->rebuildAll)
    {
        d->allPicturesPath = pathList;
    }
    else
    {
        for (QStringList::ConstIterator i = pathList.constBegin();
                !d->cancel && (i != pathList.constEnd()); ++i)
        {
            ImageInfo info(*i);
            
            if(d->faceIface->hasBeenScanned(info.id()))
            {
                kDebug()<< "Image " << info.filePath() << "has already been scanned.";
                continue;
            }
            d->allPicturesPath << (*i);
        }
    }

     setMaximum(pathList.count());
     setValue(pathList.count() - d->allPicturesPath.count());
     
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
    setLabel(i18n("<b>Scanning for people completed.</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    setButtonGuiItem(KStandardGuiItem::ok());
    setButtonText(i18n("&Close"));
// Pop-up a message to bring user when all is done.
    
    emit signalDetectAllFacesDone();
}

void BatchFaceDetector::slotGotImagePreview(const LoadingDescription& desc, const DImg& img)
{
    if (d->allPicturesPath.isEmpty())
        return;

    if (d->allPicturesPath.first() != desc.filePath)
        return;

    DImg dimg(img);
    
    // FIXME: Ignore xcf images for now, till the problem with xcf is solved.
    if (!dimg.isNull() && !desc.filePath.endsWith("xcf", Qt::CaseInsensitive))
    {
        kDebug() << "Will detect faces in " << desc.filePath << " => " << "Height= " << img.height() << ", Width= " << img.width();

        // If we're to rebuild everything, delete old thumbnails. 
        // FIXME: We have multiple thumbs per image, but deletethumbnail deletes all.
        if (d->rebuildAll)
            d->thumbnailLoadThread->deleteThumbnail(desc.filePath);
        
        // Find all faces, and create and associate their face thumbnails with the image.
        QList<KFaceIface::Face> faceList = d->faceIface->findAndTagFaces(dimg, ImageInfo(desc.filePath).id() );
        
        QListIterator<KFaceIface::Face> it(faceList);
        while(it.hasNext())
        {
            KFaceIface::Face face = it.next();
            d->thumbnailLoadThread->storeDetailThumbnail(desc.filePath, face.toRect(), face.getImage(), true);
        }
    }

    emit signalOneDetected(desc, dimg);

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

void BatchFaceDetector::slotShowOneDetected(const Digikam::LoadingDescription& desc, const Digikam::DImg& dimg)
{
    QPixmap pix = DImg(dimg).smoothScale(128, 128, Qt::KeepAspectRatio).convertToPixmap();
    addedAction(pix, desc.filePath);
    advance(1);
}

} // namespace Digikam
