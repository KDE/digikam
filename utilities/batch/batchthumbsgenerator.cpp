/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchthumbsgenerator.h"
#include "batchthumbsgenerator.moc"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPointer>
#include <QPixmap>
#include <QCloseEvent>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpassivepopup.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "config-digikam.h"

namespace Digikam
{

class BatchThumbsGeneratorPriv
{
public:

    BatchThumbsGeneratorPriv()
    {
        cancel          = false;
        rebuildAll      = true;
        thumbLoadThread = 0;
        duration.start();
    }

    bool                 cancel;
    bool                 rebuildAll;

    QTime                duration;

    QStringList          allPicturesPath;

    ThumbnailLoadThread *thumbLoadThread;
};

BatchThumbsGenerator::BatchThumbsGenerator(QWidget* /*parent*/, bool rebuildAll)
                    : DProgressDlg(0), d(new BatchThumbsGeneratorPriv)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->rebuildAll      = rebuildAll;

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotGotThumbnail(const LoadingDescription&, const QPixmap&)));

    setModal(false);
    setValue(0);
    setCaption(i18n("Rebuild All Thumbnails"));
    setLabel(i18n("<b>Updating thumbnails database. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    resize(600, 300);

    QTimer::singleShot(500, this, SLOT(slotRebuildThumbs()));
}

BatchThumbsGenerator::~BatchThumbsGenerator()
{
    delete d;
}

void BatchThumbsGenerator::slotRebuildThumbs()
{
    setTitle(i18n("Processing..."));

    // Get all digiKam albums collection pictures path.

    AlbumList palbumList  = AlbumManager::instance()->allPAlbums();

    for (AlbumList::Iterator it = palbumList.begin();
         !d->cancel && (it != palbumList.end()); ++it )
    {
        d->allPicturesPath += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
    }

#ifdef USE_THUMBS_DB

    if (!d->rebuildAll)
    {
        QStringList thumbPaths = ThumbnailDatabaseAccess().db()->getValidFilePaths();
        for (QStringList::iterator it = d->allPicturesPath.begin(); it != d->allPicturesPath.end();)
        {
            if (thumbPaths.contains(*it))
            {
                it = d->allPicturesPath.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

#endif

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
    if (d->cancel || d->allPicturesPath.isEmpty()) return;

    QString path = d->allPicturesPath.first();

    if (d->rebuildAll)
        d->thumbLoadThread->deleteThumbnail(path);

    d->thumbLoadThread->find(path);
}

void BatchThumbsGenerator::complete()
{
    QTime t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>The thumbnails database has been updated.</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    setButtonText(i18n("&Close"));
    // Pop-up a message to bring user when all is done.
    KPassivePopup::message(windowTitle(), i18n("The thumbnails database has been updated."), this);
    emit signalRebuildAllThumbsDone();
}

void BatchThumbsGenerator::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->cancel || d->allPicturesPath.isEmpty()) return;

    if (d->allPicturesPath.first() != desc.filePath) return;

    addedAction(pix, desc.filePath);
    advance(1);
    if (!d->allPicturesPath.isEmpty())
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
