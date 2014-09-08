/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dio.moc"
#include "dio_p.moc"

// Qt includes

#include <QApplication>
#include <QFileInfo>

// KDE includes

#include <kde_file.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "imageinfo.h"
#include "applicationsettings.h"
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
    const QString renameFileProperty("DIO Rename source file");
    const QString noErrorMessageProperty("DIO Ignore Error Message");
}

// ------------------------------------------------------------------------------------------------

SidecarFinder::SidecarFinder(const KUrl::List& files)
{
    process(files);
}

SidecarFinder::SidecarFinder(const KUrl& file)
{
    process(KUrl::List() << file);
}

void SidecarFinder::process(const KUrl::List& files)
{
    foreach (const KUrl& url, files)
    {
        if (url.isLocalFile())
        {
            if (DMetadata::hasSidecar(url.toLocalFile()))
            {
                localFiles << DMetadata::sidecarUrl(url);
                kDebug()   << "Detected a sidecar" << localFiles.last();
            }

            localFiles << url;
        }
        else
        {
            possibleRemoteSidecars << DMetadata::sidecarUrl(url);
            remoteFiles            << url;
        }
    }
}

// ------------------------------------------------------------------------------------------------

GroupedImagesFinder::GroupedImagesFinder(const QList<ImageInfo> source)
{
    process(source);
}

void GroupedImagesFinder::process(const QList<ImageInfo> source)
{
    QSet<qlonglong> ids;

    foreach (const ImageInfo& info, source)
    {
        ids << info.id();
    }

    infos.reserve(source.size());

    foreach (const ImageInfo& info, source)
    {
        infos << info;

        if (info.hasGroupedImages())
        {
            foreach (const ImageInfo& groupedImage, info.groupedImages())
            {
                if (ids.contains(groupedImage.id()))
                {
                    continue;
                }

                infos << groupedImage;
                ids << groupedImage.id();
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------

DIO::Private::Private(DIO* const q)
    : q(q)
{
    connectAndSchedule(this, SIGNAL(jobToProcess(int,KUrl::List,KUrl)),
                       this, SLOT(processJob(int,KUrl::List,KUrl)));

    connectAndSchedule(this, SIGNAL(renameToProcess(KUrl,KUrl)),
                       this, SLOT(processRename(KUrl,KUrl)));

    connect(this, SIGNAL(jobToCreate(int,KUrl::List,KUrl)),
            q, SLOT(createJob(int,KUrl::List,KUrl)));
}

void DIO::Private::processJob(int operation, const KUrl::List& srcList, const KUrl& dest)
{
    SidecarFinder finder(srcList);

    emit jobToCreate(operation, finder.localFiles, dest);

    if (!finder.remoteFiles.isEmpty())
    {
        emit jobToCreate(operation, finder.remoteFiles, dest);
        // stat'ing is unreliable; just try to copy and suppress error message
        emit jobToCreate(operation | SourceStatusUnknown, finder.possibleRemoteSidecars, dest);
    }
}

void DIO::Private::processRename(const KUrl& src, const KUrl& dest)
{
    QString sidecar = DMetadata::sidecarFilePathForFile(src.toLocalFile());

    if (QFileInfo(sidecar).exists())
    {
        QString destSidecar = DMetadata::sidecarFilePathForFile(dest.toLocalFile());
        emit jobToCreate(Rename, KUrl::List() << KUrl::fromPath(sidecar), KUrl::fromPath(destSidecar));
    }

    emit jobToCreate(Rename, KUrl::List() << src, dest);
}

void DIO::Private::albumToAlbum(int operation, const PAlbum* const src, const PAlbum* const dest)
{
    ScanController::instance()->hintAtMoveOrCopyOfAlbum(src, dest);
    emit jobToCreate(operation, src->fileUrl(), dest->fileUrl());
}

void DIO::Private::imagesToAlbum(int operation, const QList<ImageInfo> infos, const PAlbum* const dest)
{
    // this is a fast db operation, do here
    GroupedImagesFinder finder(infos);

    QStringList      filenames;
    QList<qlonglong> ids;
    KUrl::List       urls;

    foreach(const ImageInfo& info, finder.infos)
    {
        filenames << info.name();
        ids << info.id();
        urls << info.fileUrl();
    }

    ScanController::instance()->hintAtMoveOrCopyOfItems(ids, dest, filenames);

    emit jobToProcess(operation, urls, dest->fileUrl());
}

void DIO::Private::filesToAlbum(int operation, const KUrl::List& srcList, const PAlbum* const dest)
{
    emit jobToProcess(operation, srcList, dest->fileUrl());
}

void DIO::Private::renameFile(const ImageInfo& info, const QString& newName)
{
    KUrl oldUrl = info.fileUrl();
    KUrl newUrl = oldUrl;
    newUrl.setFileName(newName);

    PAlbum* const album = AlbumManager::instance()->findPAlbum(info.albumId());

    if (album)
    {
        ScanController::instance()->hintAtMoveOrCopyOfItem(info.id(), album, newName);
    }

    emit renameToProcess(oldUrl, newUrl);
}

void DIO::Private::deleteFiles(const QList<ImageInfo>& infos, bool useTrash)
{
    KUrl::List urls;

    foreach(const ImageInfo& info, infos)
    {
        urls << info.fileUrl();
    }

    emit jobToProcess(useTrash ? Trash : Delete, urls, KUrl());
}

// ------------------------------------------------------------------------------------------------

class DIOCreator
{
public:

    DIO object;
};

K_GLOBAL_STATIC(DIOCreator, creator)

// ------------------------------------------------------------------------------------------------

DIO* DIO::instance()
{
    return &creator->object;
}

DIO::DIO()
    : d(new Private(this))
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
    if (src.isEmpty())
    {
        return 0;
    }

    KIO::Job* job  = 0;
    int flags      = operation & FlagMask;
    operation     &= OperationMask;

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

    if (flags & SourceStatusUnknown)
    {
        job->setProperty(noErrorMessageProperty.toAscii(), true);
    }

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    return job;
}

void DIO::slotResult(KJob* kjob)
{
    KIO::Job* const job = static_cast<KIO::Job*>(kjob);

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

        if (job->property(noErrorMessageProperty.toAscii()).isValid())
        {
            return;
        }

        QWidget* const w = QApplication::activeWindow();

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

// Album -> Album -----------------------------------------------------

void DIO::copy(const PAlbum* const src, const PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

    instance()->d->albumToAlbum(Copy, src, dest);
}

void DIO::move(const PAlbum* src, const PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

    instance()->d->albumToAlbum(Move, src, dest);
}

// Images -> Album ----------------------------------------------------

void DIO::copy(const QList<ImageInfo> infos, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->imagesToAlbum(Copy, infos, dest);
}

void DIO::move(const QList<ImageInfo> infos, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->imagesToAlbum(Move, infos, dest);
}

// External files -> album --------------------------------------------

void DIO::copy(const KUrl& src, const PAlbum* const dest)
{
    copy(KUrl::List() << src, dest);
}

void DIO::copy(const KUrl::List& srcList, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->filesToAlbum(Copy, srcList, dest);
}

void DIO::move(const KUrl& src, const PAlbum* const dest)
{
    move(KUrl::List() << src, dest);
}

void DIO::move(const KUrl::List& srcList, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->filesToAlbum(Move, srcList, dest);
}

// Rename --------------------------------------------------------------

void DIO::rename(const ImageInfo& info, const QString& newName)
{
    instance()->d->renameFile(info, newName);
}

// Delete --------------------------------------------------------------

void DIO::del(const QList<ImageInfo>& infos, bool useTrash)
{
    instance()->d->deleteFiles(infos, useTrash);
}

void DIO::del(const ImageInfo& info, bool useTrash)
{
    del(QList<ImageInfo>() << info, useTrash);
}

void DIO::del(const PAlbum* const album, bool useTrash)
{
    if (!album)
    {
        return;
    }

    instance()->createJob(useTrash ? Trash : Delete, KUrl::List() << album->fileUrl(), KUrl());
}

} // namespace Digikam
