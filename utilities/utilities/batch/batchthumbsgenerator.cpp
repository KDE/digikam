/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright 2006-2007 by Gilles Caulier
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

#include <kmdcodec.h>
#include <klocale.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "thumbnailjob.h"
#include "batchthumbsgenerator.h"
#include "batchthumbsgenerator.moc"

namespace Digikam
{

class BatchThumbsGeneratorPriv
{
public:

    BatchThumbsGeneratorPriv()
    {
        cancel   = false;
        thumbJob = 0;
        duration.start();
    }

    bool                       cancel;
    
    QTime                      duration;
    
    QGuardedPtr<ThumbnailJob>  thumbJob;
};

BatchThumbsGenerator::BatchThumbsGenerator(QWidget* parent)
                    : DProgressDlg(parent)
{
    d = new BatchThumbsGeneratorPriv;
    setValue(0);
    setCaption(i18n("Thumbnails processing"));
    setLabel(i18n("<b>Updating thumbnails database in progress. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    QTimer::singleShot(500, this, SLOT(slotRebuildThumbs128()));
    resize(600, 300);
}

BatchThumbsGenerator::~BatchThumbsGenerator()
{
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }

    delete d;
}

void BatchThumbsGenerator::slotRebuildThumbs128()
{
    setTitle(i18n("Processing small thumbs"));
    rebuildAllThumbs(128);

    connect(this, SIGNAL(signalRebuildThumbsDone()),
            this, SLOT(slotRebuildThumbs256()));
}

void BatchThumbsGenerator::slotRebuildThumbs256()
{
    setTitle(i18n("Processing large thumbs"));
    rebuildAllThumbs(256);

    disconnect(this, SIGNAL(signalRebuildThumbsDone()),
               this, SLOT(slotRebuildThumbs256()));

    connect(this, SIGNAL(signalRebuildThumbsDone()),
            this, SLOT(slotRebuildAllThumbComplete()));
}

void BatchThumbsGenerator::slotRebuildAllThumbComplete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Update of thumbnails database done</b>"));
    setTitle(i18n("Duration: %1").arg(t.toString()));
    setButtonText(i18n("&Close"));
}

void BatchThumbsGenerator::rebuildAllThumbs(int size)
{
    QStringList allPicturesPath;
    QString thumbCacheDir = QDir::homeDirPath() + "/.thumbnails/";
    QString filesFilter   = AlbumSettings::instance()->getAllFileFilter();
    bool exifRotate       = AlbumSettings::instance()->getExifRotate();
    AlbumDB *db           = AlbumManager::instance()->albumDB();
    AlbumList palbumList  = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path.
    
    for (AlbumList::Iterator it = palbumList.begin();
         !d->cancel && (it != palbumList.end()); ++it )
    {
        // Don't use the root album
        if ((*it)->isRoot())
            continue;

        db->beginTransaction();
        QStringList albumItemsPath = db->getItemURLsInAlbum((*it)->id());
        db->commitTransaction();

        QStringList pathSorted;
        for (QStringList::iterator it2 = albumItemsPath.begin();
             !d->cancel && (it2 != albumItemsPath.end()); ++it2)
        {
            QFileInfo fi(*it2);
            if (filesFilter.contains(fi.extension(false)))
                pathSorted.append(*it2);
        }
        
        allPicturesPath += pathSorted;
    }

    setTotalSteps(allPicturesPath.count()*2);

    // Remove all current album item thumbs from disk cache.

    for (QStringList::iterator it = allPicturesPath.begin(); 
         !d->cancel && (it != allPicturesPath.end()); ++it)
    {
        QString uri = "file://" + QDir::cleanDirPath(*it);
        KMD5 md5(QFile::encodeName(uri));
        uri = md5.hexDigest();
    
        QString smallThumbPath = thumbCacheDir + "normal/" + uri + ".png";
        QString bigThumbPath   = thumbCacheDir + "large/"  + uri + ".png";

        if (size <= 128)
            ::unlink(QFile::encodeName(smallThumbPath));
        else
            ::unlink(QFile::encodeName(bigThumbPath));
    }

    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }
    
    d->thumbJob = new ThumbnailJob(KURL::List(allPicturesPath), size, true, exifRotate);
       
    connect(d->thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
            this, SLOT(slotRebuildThumbDone(const KURL&, const QPixmap&)));
    
    connect(d->thumbJob, SIGNAL(signalFailed(const KURL&)),
            this, SLOT(slotRebuildThumbDone(const KURL&)));

    connect(d->thumbJob, SIGNAL(signalCompleted()),
            this, SIGNAL(signalRebuildThumbsDone()));
}

void BatchThumbsGenerator::slotRebuildThumbDone(const KURL& url, const QPixmap& pix)
{
    addedAction(pix, url.path());
    advance(1);
}

void BatchThumbsGenerator::slotCancel()
{
    abort();
    done(Cancel);
}

void BatchThumbsGenerator::closeEvent(QCloseEvent *e)
{
    abort();
    e->accept();
}

void BatchThumbsGenerator::abort()
{
    d->cancel = true;

    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }

    emit signalRebuildAllThumbsDone();
}

}  // namespace Digikam


