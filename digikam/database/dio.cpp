/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include "dio.h"
#include "dio_p.h"

// Qt includes

#include <QApplication>
#include <QFileInfo>

// KDE includes

#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "imageinfo.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "dmetadata.h"
#include "imagelister.h"
#include "loadingcacheinterface.h"
#include "scancontroller.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace
{
    enum Operation
    {
        Copy,
        Move,
        Rename,
        Trash,
        Delete
    };

    const QString renameFileProperty("DIO Rename source file");
}

DIO::DIOPriv::DIOPriv(DIO* q)
    : q(q)
{
    connectAndSchedule(this, SIGNAL(jobToProcess(int, KUrl::List, KUrl)),
                       this, SLOT(processJob(int, KUrl::List, KUrl)));

    connectAndSchedule(this, SIGNAL(renameToProcess(KUrl, KUrl)),
                       this, SLOT(processRename(KUrl, KUrl)));

    connect(this, SIGNAL(jobToCreate(int, KUrl::List, KUrl)),
            q, SLOT(createJob(int, KUrl::List, KUrl)));
}

void DIO::DIOPriv::processJob(int operation, const KUrl::List& srcList, const KUrl& dest)
{
    KUrl::List list = srcList, remoteList;
    for (KUrl::List::iterator it = list.begin(); it != list.end(); ++it)
    {
        if (it->isLocalFile())
        {
            QString sidecar = DMetadata::sidecarFilePathForFile(it->toLocalFile());
            if (QFileInfo(sidecar).exists())
            {
                it = list.insert(it, KUrl::fromPath(sidecar))+1;
                //kDebug() << "Detected a sidecar" << sidecar;
            }
        }
        else
        {
            QString path = it->path();
            QString sidecarPath = DMetadata::sidecarFilePathForFile(path);
            KUrl sidecarUrl = *it;
            sidecarUrl.setPath(sidecarPath);
            remoteList << sidecarUrl;
        }
    }

    emit jobToCreate(operation, list, dest);

    if (!remoteList.isEmpty())
    {
        kWarning() << "Copying sidecar files from remote destination is not implemented";
        //emit remoteFilesToStat(operation, srcList, dest
    }
}

void DIO::DIOPriv::processRename(const KUrl& src, const KUrl& dest)
{
    QString sidecar = DMetadata::sidecarFilePathForFile(src.toLocalFile());
    if (QFileInfo(sidecar).exists())
    {
        QString destSidecar = DMetadata::sidecarFilePathForFile(dest.toLocalFile());
        emit jobToCreate(Rename, KUrl::List() << KUrl::fromPath(sidecar), KUrl::fromPath(destSidecar));
    }
    emit jobToCreate(Rename, KUrl::List() << src, dest);
}

void DIO::DIOPriv::albumToAlbum(int operation, const PAlbum* src, const PAlbum* dest)
{
    ScanController::instance()->hintAtMoveOrCopyOfAlbum(src, dest);
    emit jobToCreate(operation, src->fileUrl(), dest->fileUrl());
}

void DIO::DIOPriv::imagesToAlbum(int operation, const QList<ImageInfo> infos, const PAlbum* dest)
{
    QStringList filenames;
    QList<qlonglong> ids;
    KUrl::List urls;
    foreach(const ImageInfo& info, infos)
    {
        filenames << info.name();
        ids << info.id();
        urls << info.fileUrl();
    }
    ScanController::instance()->hintAtMoveOrCopyOfItems(ids, dest, filenames);

    emit jobToProcess(operation, urls, dest->fileUrl());
}

void DIO::DIOPriv::filesToAlbum(int operation, const KUrl::List& srcList, const PAlbum* dest)
{
    emit jobToProcess(operation, srcList, dest->fileUrl());
}

void DIO::DIOPriv::renameFile(const ImageInfo& info, const QString& newName)
{
    KUrl oldUrl = info.fileUrl();
    KUrl newUrl = oldUrl;
    newUrl.setFileName(newName);

    PAlbum* album = AlbumManager::instance()->findPAlbum(info.albumId());

    if (album)
    {
        ScanController::instance()->hintAtMoveOrCopyOfItem(info.id(), album, newName);
    }

    emit renameToProcess(oldUrl, newUrl);
}

void DIO::DIOPriv::deleteFiles(const QList<ImageInfo>& infos, bool useTrash)
{
    KUrl::List urls;
    foreach(const ImageInfo& info, infos)
    {
        urls << info.fileUrl();
    }
    emit jobToProcess(useTrash ? Trash : Delete, urls, KUrl());
}

class DIOCreator
{
public:

    DIO object;
};

K_GLOBAL_STATIC(DIOCreator, creator)

DIO* DIO::instance()
{
    return &creator->object;
}

DIO::DIO()
    : d(new DIOPriv(this))
{
}

DIO::~DIO()
{
    delete d;
}

void DIO::cleanUp()
{
    instance()->d->deactivate();
    instance()->d->wait();
}

KIO::Job* DIO::createJob(int operation, const KUrl::List& src, const KUrl& dest)
{
    KIO::Job* job;
    if (operation == Copy)
    {
        job = KIO::copy(src, dest);
    }
    else if (operation == Move)
    {
        job = KIO::move(src, dest);
    }
    else if (operation == Rename)
    {
        if (src.size() != 1)
        {
            kError() << "Invalid operation: renaming is not 1:1";
            return 0;
        }
        job = KIO::move(src.first(), dest, KIO::HideProgressInfo);
        job->setProperty(renameFileProperty.toAscii(), src.first().toLocalFile());

        connect(job, SIGNAL(copyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)),
                this, SLOT(slotRenamed(KIO::Job*,KUrl,KUrl)));
    }
    else if (operation == Trash)
    {
        job = KIO::trash(src);
    }
    else // operation == Del
    {
        job = KIO::del(src);
    }

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    return job;
}

void DIO::slotResult(KJob* kjob)
{
    KIO::Job* job = static_cast<KIO::Job*>(kjob);

    if (job->error())
    {
        // this slot can be used by others, too.
        // check if image renaming property is set.
        QVariant v = job->property(renameFileProperty.toAscii());

        if (!v.isNull())
        {
            KUrl url(v.toString());
            if (job->error() == KIO::Job::KilledJobError)
            {
                emit renamingAborted(url);
            }
            else
            {
                emit imageRenameFailed(url);
            }
        }

        QWidget* w = QApplication::activeWindow();
        if (w)
        {
            job->ui()->setWindow(w);
        }
        job->ui()->showErrorMessage();
    }
}

void DIO::slotRenamed(KIO::Job* job, const KUrl&, const KUrl& newURL)
{
    // reconstruct file path from digikamalbums:// URL
    KUrl fileURL;
    fileURL.setPath(newURL.user());
    fileURL.addPath(newURL.path());

    // refresh thumbnail
    ThumbnailLoadThread::deleteThumbnail(fileURL.toLocalFile());
    // clean LoadingCache as well - be pragmatic, do it here.
    LoadingCacheInterface::fileChanged(fileURL.toLocalFile());

    KUrl url(job->property(renameFileProperty.toAscii()).toString());
    emit imageRenameSucceeded(url);
}

// Album -> Album

void DIO::copy(const PAlbum* src, const PAlbum* dest)
{
    if (!src || !dest)
    {
        return;
    }
    instance()->d->albumToAlbum(Copy, src, dest);
}

void DIO::move(const PAlbum* src, const PAlbum* dest)
{
    if (!src || !dest)
    {
        return;
    }
    instance()->d->albumToAlbum(Move, src, dest);
}

// Images -> Album

void DIO::copy(const QList<ImageInfo> infos, const PAlbum* dest)
{
    if (!dest)
    {
        return;
    }
    instance()->d->imagesToAlbum(Copy, infos, dest);
}

void DIO::move(const QList<ImageInfo> infos, const PAlbum* dest)
{
    if (!dest)
    {
        return;
    }
    instance()->d->imagesToAlbum(Move, infos, dest);
}

// External files -> album

void DIO::copy(const KUrl& src, const PAlbum* dest)
{
    copy(KUrl::List() << src, dest);
}

void DIO::copy(const KUrl::List& srcList, const PAlbum* dest)
{
    if (!dest)
    {
        return;
    }
    instance()->d->filesToAlbum(Copy, srcList, dest);
}

void DIO::move(const KUrl& src, const PAlbum* dest)
{
    move(KUrl::List() << src, dest);
}

void DIO::move(const KUrl::List& srcList, const PAlbum* dest)
{
    if (!dest)
    {
        return;
    }
    instance()->d->filesToAlbum(Move, srcList, dest);
}

// Rename

void DIO::rename(const ImageInfo& info, const QString& newName)
{
    instance()->d->renameFile(info, newName);
}

// Delete

void DIO::del(const QList<ImageInfo>& infos, bool useTrash)
{
    instance()->d->deleteFiles(infos, useTrash);
}

void DIO::del(const ImageInfo& info, bool useTrash)
{
    del(QList<ImageInfo>() << info, useTrash);
}

void DIO::del(PAlbum* album, bool useTrash)
{
    if (!album)
    {
        return;
    }
    instance()->createJob(useTrash ? Trash : Delete, KUrl::List() << album->fileUrl(), KUrl());
}

} // namespace Digikam
