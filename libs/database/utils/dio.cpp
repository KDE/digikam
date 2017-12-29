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
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "imageinfo.h"
#include "albummanager.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "album.h"
#include "dmetadata.h"
#include "imagelister.h"
#include "loadingcacheinterface.h"
#include "metadatasettings.h"
#include "scancontroller.h"
#include "thumbnailloadthread.h"
#include "iojobsmanager.h"
#include "collectionmanager.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"

namespace Digikam
{

namespace
{
    const QString renameFileProperty(QLatin1String("DIO Rename source file"));
    const QString noErrorMessageProperty(QLatin1String("DIO Ignore Error Message"));
}

// ------------------------------------------------------------------------------------------------

SidecarFinder::SidecarFinder(const QList<QUrl>& files)
{
    process(files);
}

SidecarFinder::SidecarFinder(const QUrl& file)
{
    process(QList<QUrl>() << file);
}

void SidecarFinder::process(const QList<QUrl>& files)
{
    foreach(const QUrl& url, files)
    {
        if (url.isLocalFile())
        {
            if (DMetadata::hasSidecar(url.toLocalFile()))
            {
                localFiles << DMetadata::sidecarUrl(url);
                localFileSuffixes << QLatin1String(".xmp");
                qCDebug(DIGIKAM_DATABASE_LOG) << "Detected a sidecar" << localFiles.last();
            }

            foreach(QString suffix, MetadataSettings::instance()->settings().sidecarExtensions)
            {
                suffix = QLatin1String(".") + suffix;
                QString sidecarName = url.toLocalFile() + suffix;

                if (QFileInfo(sidecarName).exists() && !localFiles.contains(QUrl::fromLocalFile(sidecarName)))
                {
                    localFiles << QUrl::fromLocalFile(sidecarName);
                    localFileSuffixes << suffix;
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Detected a sidecar" << localFiles.last();
                }
            }

            localFiles << url;
            localFileSuffixes << QString();
        }
        else
        {
            possibleRemoteSidecars << DMetadata::sidecarUrl(url);
            possibleRemoteSidecarSuffixes << QLatin1String(".xmp");

            foreach(QString suffix,  MetadataSettings::instance()->settings().sidecarExtensions)
            {
                suffix = QLatin1String(".") + suffix;
                QString sidecarName = url.toLocalFile() + suffix;

                if (!possibleRemoteSidecars.contains(QUrl::fromLocalFile(sidecarName)))
                {
                    possibleRemoteSidecars << QUrl::fromLocalFile(sidecarName);
                    possibleRemoteSidecarSuffixes << suffix;
                }
            }

            remoteFiles << url;
            remoteFileSuffixes << QString();
        }
    }
}

// ------------------------------------------------------------------------------------------------

// TODO
// Groups should not be resolved in dio, it should be handled in views.
// This is already done for most things except for drag&drop, which is hard :)
GroupedImagesFinder::GroupedImagesFinder(const QList<ImageInfo>& source)
{
    process(source);
}

void GroupedImagesFinder::process(const QList<ImageInfo>& source)
{
    QSet<qlonglong> ids;

    foreach(const ImageInfo& info, source)
    {
        ids << info.id();
    }

    infos.reserve(source.size());

    foreach(const ImageInfo& info, source)
    {
        infos << info;

        if (info.hasGroupedImages())
        {
            foreach(const ImageInfo& groupedImage, info.groupedImages())
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

DIO::Private::Private(DIO* const qq)
    : q(qq)
{
    connectAndSchedule(this, SIGNAL(jobToProcess(int,QList<QUrl>,QUrl)),
                       this, SLOT(processJob(int,QList<QUrl>,QUrl)));

    connectAndSchedule(this, SIGNAL(renameToProcess(QUrl,QUrl)),
                       this, SLOT(processRename(QUrl,QUrl)));

    connect(this, SIGNAL(jobToCreate(int,QList<QUrl>,QUrl)),
            q, SLOT(createJob(int,QList<QUrl>,QUrl)));
}

void DIO::Private::processJob(int operation, const QList<QUrl>& srcList, const QUrl& dest)
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

void DIO::Private::processRename(const QUrl& src, const QUrl& dest)
{
    SidecarFinder finder(src);

    if (src.isLocalFile())
    {
        for (int i = 0 ; i < finder.localFiles.length() ; ++i)
        {
            emit jobToCreate(Rename, QList<QUrl>() << finder.localFiles.at(i),
                             QUrl::fromLocalFile(dest.toLocalFile() + finder.localFileSuffixes.at(i)));
        }

        return;
    }

    for (int i = 0 ; i < finder.remoteFileSuffixes.length() ; ++i)
    {
        emit jobToCreate(Rename | SourceStatusUnknown,
                         QList<QUrl>() << finder.possibleRemoteSidecars.at(i),
                         QUrl::fromLocalFile(dest.toLocalFile() + finder.possibleRemoteSidecarSuffixes.at(i)));
    }
    emit jobToCreate(Rename, QList<QUrl>() << src, dest);
}

void DIO::Private::albumToAlbum(int operation, const PAlbum* const src, const PAlbum* const dest)
{
    ScanController::instance()->hintAtMoveOrCopyOfAlbum(src, dest);
    emit jobToCreate(operation, QList<QUrl>() << src->fileUrl(), dest->fileUrl());
}

void DIO::Private::imagesToAlbum(int operation, const QList<ImageInfo>& infos, const PAlbum* const dest)
{
    // this is a fast db operation, do here
    GroupedImagesFinder finder(infos);

    QStringList      filenames;
    QList<qlonglong> ids;
    QList<QUrl>      urls;

    if (operation == Move)
    {
        // update the image infos
        CoreDbAccess access;
        foreach(const ImageInfo& info, finder.infos)
        {
            access.db()->moveItem(info.albumId(), info.name(), dest->id(), info.name());
        }
    }

    foreach(const ImageInfo& info, finder.infos)
    {
        filenames << info.name();
        ids << info.id();
        urls << info.fileUrl();
    }

    ScanController::instance()->hintAtMoveOrCopyOfItems(ids, dest, filenames);

    emit jobToProcess(operation, urls, dest->fileUrl());
}

void DIO::Private::filesToAlbum(int operation, const QList<QUrl>& srcList, const PAlbum* const dest)
{
    emit jobToProcess(operation, srcList, dest->fileUrl());
}

void DIO::Private::renameFile(const ImageInfo& info, const QString& newName)
{
    QUrl oldUrl = info.fileUrl();
    QUrl newUrl = oldUrl;
    newUrl      = newUrl.adjusted(QUrl::RemoveFilename);
    newUrl.setPath(newUrl.path() + newName);

    PAlbum* const album = AlbumManager::instance()->findPAlbum(info.albumId());

    if (album)
    {
        ScanController::instance()->hintAtMoveOrCopyOfItem(info.id(), album, newName);
    }

    // If we rename a file, the name changes. This is equivalent to a move.
    // Do this in database, too.
    CoreDbAccess().db()->moveItem(info.albumId(), oldUrl.fileName(), info.albumId(), newName);

    emit renameToProcess(oldUrl, newUrl);
}

void DIO::Private::deleteFiles(const QList<ImageInfo>& infos, bool useTrash)
{
    QList<QUrl> urls;

    foreach(const ImageInfo& info, infos)
    {
        urls << info.fileUrl();
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Deleting files:" << urls;

    emit jobToProcess(useTrash ? Trash : Delete, urls, QUrl());
}

// ------------------------------------------------------------------------------------------------

class DIOCreator
{
public:

    DIO object;
};

Q_GLOBAL_STATIC(DIOCreator, creator)

// ------------------------------------------------------------------------------------------------

DIO* DIO::instance()
{
    return &creator->object;
}

DIO::DIO()
    : d(new Private(this))
{
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
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

void DIO::createJob(int operation, const QList<QUrl>& src, const QUrl& dest)
{
    if (src.isEmpty())
    {
        return;
    }

    IOJobsThread* jobThread = 0;
    int flags               = operation & FlagMask;
    operation              &= OperationMask;

    if (operation == Copy)
    {
        jobThread = IOJobsManager::instance()->startCopy(src, dest);
    }
    else if (operation == Move)
    {
        jobThread = IOJobsManager::instance()->startMove(src, dest);
    }
    else if (operation == Rename)
    {
        if (src.size() != 1)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Invalid operation: renaming is not 1:1";
            return;
        }

        jobThread = IOJobsManager::instance()->startRenameFile(src.first(), dest);

        connect(jobThread, SIGNAL(renamed(QUrl,QUrl)),
                this, SLOT(slotRenamed(QUrl,QUrl)));

        connect(jobThread, SIGNAL(renameFailed(QUrl)),
                this, SIGNAL(imageRenameFailed(QUrl)));
    }
    else if (operation == Trash)
    {
        jobThread = IOJobsManager::instance()->startDelete(src);
    }
    else // operation == Del
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "SRCS " << src;
        jobThread = IOJobsManager::instance()->startDelete(src, false);
    }

    if (flags & SourceStatusUnknown)
    {
        jobThread->setKeepErrors(false);
    }

    connect(jobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));
}

void DIO::slotResult()
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread)
    {
        return;
    }

    if (jobThread->hasErrors() && jobThread->isKeepingErrors())
    {
        // Pop-up a message about the error.
        QString errors = QStringList(jobThread->errorsList()).join(QLatin1String("\n"));
        DNotificationWrapper(QString(), errors, DigikamApp::instance(),
                             DigikamApp::instance()->windowTitle());
    }
}

void DIO::slotRenamed(const QUrl& oldUrl, const QUrl& newUrl)
{
    // delete thumbnail
    ThumbnailLoadThread::deleteThumbnail(oldUrl.toLocalFile());
    // clean LoadingCache as well - be pragmatic, do it here.
    LoadingCacheInterface::fileChanged(newUrl.toLocalFile());

    emit imageRenameSucceeded(oldUrl);
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

#ifdef Q_OS_WIN
    AlbumManager::instance()->removeWatchedPAlbums(src);
#endif

    instance()->d->albumToAlbum(Move, src, dest);
}

// Images -> Album ----------------------------------------------------

void DIO::copy(const QList<ImageInfo>& infos, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->imagesToAlbum(Copy, infos, dest);
}

void DIO::move(const QList<ImageInfo>& infos, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->imagesToAlbum(Move, infos, dest);
}

// External files -> album --------------------------------------------

void DIO::copy(const QUrl& src, const PAlbum* const dest)
{
    copy(QList<QUrl>() << src, dest);
}

void DIO::copy(const QList<QUrl>& srcList, const PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->d->filesToAlbum(Copy, srcList, dest);
}

void DIO::move(const QUrl& src, const PAlbum* const dest)
{
    move(QList<QUrl>() << src, dest);
}

void DIO::move(const QList<QUrl>& srcList, const PAlbum* const dest)
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

#ifdef Q_OS_WIN
    AlbumManager::instance()->removeWatchedPAlbums(album);
#endif

    instance()->createJob(useTrash ? Trash : Delete, QList<QUrl>() << album->fileUrl(), QUrl());
}

} // namespace Digikam
