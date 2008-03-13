/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPointer>
#include <QPixmap>
#include <QCloseEvent>

// KDE includes.

#include <kcodecs.h>
#include <klocale.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "batchthumbsgenerator.h"
#include "batchthumbsgenerator.moc"

namespace Digikam
{

class BatchThumbsGeneratorPriv
{
public:

    BatchThumbsGeneratorPriv()
    {
        cancel          = false;
        thumbLoadThread = 0;
        duration.start();
    }

    bool                 cancel;

    QTime                duration;

    QStringList          allPicturesPath;

    ThumbnailLoadThread *thumbLoadThread;
};

BatchThumbsGenerator::BatchThumbsGenerator(QWidget* parent)
                    : DProgressDlg(parent)
{
    d = new BatchThumbsGeneratorPriv;
    d->thumbLoadThread = new ThumbnailLoadThread();

    // Set cache size to 256 to have the max quality thumb.
    d->thumbLoadThread->setThumbnailSize(ThumbnailSize::Huge);
    d->thumbLoadThread->setSendSurrogatePixmap(true);
    d->thumbLoadThread->setExifRotate(AlbumSettings::instance()->getExifRotate());

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotGotThumbnail(const LoadingDescription&, const QPixmap&)));

    setValue(0);
    setCaption(i18n("Rebuild All Thumbnails"));
    setLabel(i18n("<b>Updating thumbnails database in progress. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    resize(600, 300);

    QTimer::singleShot(500, this, SLOT(slotRebuildThumbs()));
}

BatchThumbsGenerator::~BatchThumbsGenerator()
{
    delete d->thumbLoadThread;
    delete d;
}

void BatchThumbsGenerator::slotRebuildThumbs()
{
    setTitle(i18n("Processing..."));
    QString filesFilter   = AlbumSettings::instance()->getAllFileFilter();
    AlbumList palbumList  = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path.

    for (AlbumList::Iterator it = palbumList.begin();
         !d->cancel && (it != palbumList.end()); ++it )
    {
        // Don't use the root album
        if ((*it)->isRoot())
            continue;

        QStringList albumItemsPath;
        {
            DatabaseAccess access;
            albumItemsPath = access.db()->getItemURLsInAlbum((*it)->id());
        }

        QStringList pathSorted;
        for (QStringList::iterator it2 = albumItemsPath.begin();
             !d->cancel && (it2 != albumItemsPath.end()); ++it2)
        {
            QFileInfo fi(*it2);
            if (filesFilter.contains(fi.suffix()))
                pathSorted.append(*it2);
        }

        d->allPicturesPath += pathSorted;
    }

    setMaximum(d->allPicturesPath.count());

    if(d->allPicturesPath.isEmpty())
    {
       slotCancel();
       return;
    }

    processOne();
}

void BatchThumbsGenerator::processOne()
{
    if (d->cancel) return;
    QString path = d->allPicturesPath.first();
    d->thumbLoadThread->deleteThumbnail(path);
    d->thumbLoadThread->find(path);
}

void BatchThumbsGenerator::complete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Update of thumbnails database done</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    setButtonText(i18n("&Close"));
    emit signalRebuildAllThumbsDone();
}

void BatchThumbsGenerator::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    addedAction(pix, desc.filePath);
    advance(1);
    d->allPicturesPath.removeFirst();
    if (d->allPicturesPath.isEmpty())
        complete();
    else
        processOne();
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
    emit signalRebuildAllThumbsDone();
}

}  // namespace Digikam
