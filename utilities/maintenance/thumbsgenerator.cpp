/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbsgenerator.moc"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPixmap>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <klocale.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albuminfo.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "knotificationwrapper.h"
#include "config-digikam.h"

namespace Digikam
{

class ThumbsGenerator::ThumbsGeneratorPriv
{
public:

    ThumbsGeneratorPriv() :
        cancel(false),
        rebuildAll(true),
        thumbLoadThread(0)
    {
        duration.start();
    }

    bool                 cancel;
    bool                 rebuildAll;

    int                  albumId;

    QTime                duration;

    QStringList          allPicturesPath;

    ThumbnailLoadThread* thumbLoadThread;
};

ThumbsGenerator::ThumbsGenerator(bool rebuildAll, int albumId)
    : ProgressItem(0, "ThumbsGenerator", QString(), QString(), true, true),
      d(new ThumbsGeneratorPriv)
{
    ProgressManager::addProgressItem(this);

    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->rebuildAll      = rebuildAll;
    d->albumId         = albumId;

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription, QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription, QPixmap)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    setLabel(i18n("Thumbs"));

    QTimer::singleShot(500, this, SLOT(slotRebuildThumbs()));
}

ThumbsGenerator::~ThumbsGenerator()
{
    delete d;
}

void ThumbsGenerator::slotRebuildThumbs()
{
    // Get all digiKam albums collection pictures path.
    AlbumList palbumList;

    if (d->albumId == -1)
    {
        palbumList  = AlbumManager::instance()->allPAlbums();
    }
    else
    {
        palbumList.append(AlbumManager::instance()->findPAlbum(d->albumId));
    }

    for (AlbumList::const_iterator it = palbumList.constBegin();
         !d->cancel && (it != palbumList.constEnd()); ++it )
    {
        if (!(*it))
        {
            continue;
        }

        d->allPicturesPath += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
    }

#ifdef USE_THUMBS_DB

    if (!d->rebuildAll)
    {
        QHash<QString, int> filePaths = ThumbnailDatabaseAccess().db()->getFilePathsWithThumbnail();

        QStringList::iterator it = d->allPicturesPath.begin();

        while (it != d->allPicturesPath.end())
        {
            if (filePaths.contains(*it))
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

    // remove non-image files from the list
    QStringList::iterator it = d->allPicturesPath.begin();

    while (it != d->allPicturesPath.end())
    {
        ImageInfo info((*it));

        if (info.category() != DatabaseItem::Image)
        {
            it = d->allPicturesPath.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (d->allPicturesPath.isEmpty())
    {
        slotCancel();
        return;
    }

    setTotalItems(d->allPicturesPath.count());
    processOne();
}

void ThumbsGenerator::processOne()
{
    if (d->cancel || d->allPicturesPath.isEmpty())
    {
        return;
    }

    QString path = d->allPicturesPath.first();
    d->thumbLoadThread->deleteThumbnail(path);
    d->thumbLoadThread->find(path);
}

void ThumbsGenerator::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->cancel || d->allPicturesPath.isEmpty())
    {
        return;
    }

    if (d->allPicturesPath.first() != desc.filePath)
    {
        return;
    }

    setThumbnail(pix);
    advance(1);

    if (!d->allPicturesPath.isEmpty())
    {
        d->allPicturesPath.removeFirst();
    }

    if (d->allPicturesPath.isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

void ThumbsGenerator::complete()
{
    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Thumbs generation is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());

    emit signalComplete();

    setComplete();
}

void ThumbsGenerator::slotCancel()
{
    d->cancel = true;
    setComplete();
}

}  // namespace Digikam
