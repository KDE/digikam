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
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
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
#include "metadatasettings.h"
#include "scancontroller.h"
#include "thumbsdb.h"
#include "thumbsdbaccess.h"
#include "iojobsmanager.h"
#include "collectionmanager.h"
#include "dnotificationwrapper.h"
#include "progressmanager.h"
#include "digikamapp.h"
#include "iojobdata.h"

namespace Digikam
{

SidecarFinder::SidecarFinder(const QList<QUrl>& files)
{
    foreach(const QUrl& url, files)
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

            if (QFileInfo::exists(sidecarName) && !localFiles.contains(QUrl::fromLocalFile(sidecarName)))
            {
                localFiles << QUrl::fromLocalFile(sidecarName);
                localFileSuffixes << suffix;
                qCDebug(DIGIKAM_DATABASE_LOG) << "Detected a sidecar" << localFiles.last();
            }
        }

        localFiles << url;
        localFileSuffixes << QString();
    }
}

// ------------------------------------------------------------------------------------------------

// TODO
// Groups should not be resolved in dio, it should be handled in views.
// This is already done for most things except for drag&drop, which is hard :)
GroupedImagesFinder::GroupedImagesFinder(const QList<ImageInfo>& source)
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
{
}

DIO::~DIO()
{
}

void DIO::cleanUp()
{
}

// Album -> Album -----------------------------------------------------

void DIO::copy(PAlbum* const src, PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyAlbum, src, dest));
}

void DIO::move(PAlbum* const src, PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

#ifdef Q_OS_WIN
    AlbumManager::instance()->removeWatchedPAlbums(src);
#endif

    instance()->processJob(new IOJobData(IOJobData::MoveAlbum, src, dest));
}

// Images -> Album ----------------------------------------------------

void DIO::copy(const QList<ImageInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyImage, infos, dest));
}

void DIO::move(const QList<ImageInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::MoveImage, infos, dest));
}

// External files -> album --------------------------------------------

void DIO::copy(const QUrl& src, PAlbum* const dest)
{
    copy(QList<QUrl>() << src, dest);
}

void DIO::copy(const QList<QUrl>& srcList, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyFiles, srcList, dest));
}

void DIO::move(const QUrl& src, PAlbum* const dest)
{
    move(QList<QUrl>() << src, dest);
}

void DIO::move(const QList<QUrl>& srcList, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::MoveFiles, srcList, dest));
}

// Rename --------------------------------------------------------------

void DIO::rename(const ImageInfo& info, const QString& newName, bool overwrite)
{
    instance()->processJob(new IOJobData(IOJobData::Rename, info, newName, overwrite));
}

// Delete --------------------------------------------------------------

void DIO::del(const QList<ImageInfo>& infos, bool useTrash)
{
    instance()->processJob(new IOJobData(useTrash ? IOJobData::Trash 
                                                  : IOJobData::Delete, infos));
}

void DIO::del(const ImageInfo& info, bool useTrash)
{
    del(QList<ImageInfo>() << info, useTrash);
}

void DIO::del(PAlbum* const album, bool useTrash)
{
    if (!album)
    {
        return;
    }

#ifdef Q_OS_WIN
    AlbumManager::instance()->removeWatchedPAlbums(album);
#endif

    instance()->createJob(new IOJobData(useTrash ? IOJobData::Trash 
                                                 : IOJobData::Delete, album));
}

// ------------------------------------------------------------------------------------------------

void DIO::processJob(IOJobData* const data)
{
    const int operation = data->operation();

    if (operation == IOJobData::CopyImage || operation == IOJobData::MoveImage)
    {
        // this is a fast db operation, do here
        GroupedImagesFinder finder(data->imageInfos());
        data->setImageInfos(finder.infos);

        QStringList      filenames;
        QList<qlonglong> ids;

        foreach(const ImageInfo& info, data->imageInfos())
        {
            filenames << info.name();
            ids << info.id();
        }

        ScanController::instance()->hintAtMoveOrCopyOfItems(ids, data->destAlbum(), filenames);
    }
    else if (operation == IOJobData::CopyAlbum || operation == IOJobData::MoveAlbum)
    {
        ScanController::instance()->hintAtMoveOrCopyOfAlbum(data->srcAlbum(), data->destAlbum());
        createJob(data);
        return;
    }
    else if (operation == IOJobData::Delete || operation == IOJobData::Trash)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Number of files to be deleted:" << data->sourceUrls().count();
    }

    SidecarFinder finder(data->sourceUrls());
    data->setSourceUrls(finder.localFiles);

    if (operation == IOJobData::Rename)
    {
        if (!data->imageInfos().isEmpty())
        {
            ImageInfo info      = data->imageInfos().first();
            PAlbum* const album = AlbumManager::instance()->findPAlbum(info.albumId());

            if (album)
            {
                ScanController::instance()->hintAtMoveOrCopyOfItem(info.id(), album,
                                                                   data->destUrl().fileName());
            }

            for (int i = 0 ; i < finder.localFiles.length() ; ++i)
            {
                data->setDestUrl(finder.localFiles.at(i),
                                 QUrl::fromLocalFile(data->destUrl().toLocalFile() +
                                                     finder.localFileSuffixes.at(i)));
            }
        }
    }

    createJob(data);
}

void DIO::createJob(IOJobData* const data)
{
    if (data->sourceUrls().isEmpty())
    {
        delete data;
        return;
    }

    ProgressItem* item = 0;
    QString itemString = getItemString(data);

    if (!itemString.isEmpty())
    {
        item = ProgressManager::instance()->createProgressItem(itemString,
                                                               QString(), true, false);
        item->setTotalItems(data->sourceUrls().count());
        data->setProgressId(item->id());
    }

    IOJobsThread* const jobThread = IOJobsManager::instance()->startIOJobs(data);

    connect(jobThread, SIGNAL(signalOneProccessed(QUrl)),
            this, SLOT(slotOneProccessed(QUrl)),
            Qt::QueuedConnection);

    connect(jobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));

    if (data->operation() == IOJobData::Rename)
    {
        connect(jobThread, SIGNAL(signalRenameFailed(QUrl)),
                this, SIGNAL(signalRenameFailed(QUrl)));
    }

    if (item)
    {
        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                jobThread, SLOT(slotCancel()));

        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                this, SLOT(slotCancel(ProgressItem*)));
    }
}

void DIO::slotResult()
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread || !jobThread->jobData())
    {
        return;
    }

    IOJobData* const data = jobThread->jobData();

    if (jobThread->hasErrors() && data->operation() != IOJobData::Rename)
    {
        // Pop-up a message about the error.
        QString errors = jobThread->errorsList().join(QLatin1String("\n"));
        DNotificationWrapper(QString(), errors, DigikamApp::instance(),
                             DigikamApp::instance()->windowTitle());
    }

    slotCancel(getProgressItem(data));
}

void DIO::slotOneProccessed(const QUrl& url)
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread || !jobThread->jobData())
    {
        return;
    }

    IOJobData* const data = jobThread->jobData();
    const int operation   = data->operation();

    if (operation == IOJobData::MoveImage)
    {
        ImageInfo info = data->findImageInfo(url);

        if (!info.isNull() && data->destAlbum())
        {
            CoreDbAccess().db()->moveItem(info.albumId(), info.name(),
                                          data->destAlbum()->id(), info.name());
        }
    }
    else if (operation == IOJobData::Delete)
    {
        // Mark the images as obsolete and remove them
        // from their album and from the grouped
        PAlbum* const album = data->srcAlbum();
        CoreDbAccess access;

        if (album && album->fileUrl() == url)
        {
            // get all deleted albums
            QList<int> albumsToDelete;
            QList<qlonglong> imagesToRemove;

            addAlbumChildrenToList(albumsToDelete, album);

            foreach(int albumId, albumsToDelete)
            {
                imagesToRemove << access.db()->getItemIDsInAlbum(albumId);
            }

            foreach(const qlonglong& imageId, imagesToRemove)
            {
                access.db()->removeAllImageRelationsFrom(imageId,
                                                         DatabaseRelation::Grouped);
            }

            access.db()->removeItemsPermanently(imagesToRemove, albumsToDelete);
        }
        else
        {
            ImageInfo info = data->findImageInfo(url);

            if (!info.isNull())
            {
                CoreDbAccess access;
                access.db()->removeAllImageRelationsFrom(info.id(),
                                                         DatabaseRelation::Grouped);

                access.db()->removeItemsPermanently(QList<qlonglong>() << info.id(),
                                                    QList<int>() << info.albumId());
            }
        }
    }
    else if (operation == IOJobData::Trash)
    {
        ImageInfo info = data->findImageInfo(url);

        if (!info.isNull())
        {
            CoreDbAccess().db()->removeItems(QList<qlonglong>() << info.id(),
                                             QList<int>() << info.albumId());
        }
    }
    else if (operation == IOJobData::Rename)
    {
        ImageInfo info = data->findImageInfo(url);

        if (!info.isNull())
        {
            QString oldPath = url.toLocalFile();
            QString newName = data->destUrl(url).fileName();
            QString newPath = data->destUrl(url).toLocalFile();

            if (data->overwrite())
            {
                ThumbsDbAccess().db()->removeByFilePath(newPath);
                CoreDbAccess().db()->deleteItem(info.albumId(), newName);
            }

            ThumbsDbAccess().db()->renameByFilePath(oldPath, newPath);
            info.setName(newName);

            // Remove old thumbnails and images from the cache
            {
                LoadingCache* const cache = LoadingCache::cache();
                LoadingCache::CacheLock lock(cache);
                QStringList possibleKeys  = LoadingDescription::possibleThumbnailCacheKeys(oldPath);

                if (data->overwrite())
                {
                    possibleKeys         << LoadingDescription::possibleThumbnailCacheKeys(newPath);
                }

                foreach(const QString& cacheKey, possibleKeys)
                {
                    cache->removeThumbnail(cacheKey);
                }

                possibleKeys              = LoadingDescription::possibleCacheKeys(oldPath);

                if (data->overwrite())
                {
                    possibleKeys         << LoadingDescription::possibleCacheKeys(newPath);
                }

                foreach(const QString& cacheKey, possibleKeys)
                {
                    cache->removeImage(cacheKey);
                }
            }
        }

        emit signalRenameSucceeded(url);
    }

    // Scan folders for changes

    QString scanPath;

    if (operation == IOJobData::CopyImage || operation == IOJobData::CopyAlbum ||
        operation == IOJobData::CopyFiles || operation == IOJobData::MoveImage ||
        operation == IOJobData::MoveAlbum || operation == IOJobData::MoveFiles)
    {
        scanPath = data->destUrl().toLocalFile();
    }
    else if (operation == IOJobData::Delete || operation == IOJobData::Trash)
    {
        PAlbum* const album = data->srcAlbum();

        if (album)
        {
            PAlbum* const parent = dynamic_cast<PAlbum*>(album->parent());

            if (parent)
            {
                scanPath = parent->fileUrl().toLocalFile();
            }
        }
        else
        {
            scanPath = url.adjusted(QUrl::RemoveFilename).toLocalFile();
        }
    }

    if (!scanPath.isEmpty())
    {
        ScanController::instance()->scheduleCollectionScanRelaxed(scanPath);
    }

    ProgressItem* const item = getProgressItem(data);

    if (item)
    {
        item->advance(1);
    }
}

QString DIO::getItemString(IOJobData* const data) const
{
    switch (data->operation())
    {
        case IOJobData::CopyAlbum:
            return i18n("Copy Album");

        case IOJobData::CopyImage:
            return i18n("Copy Images");

        case IOJobData::CopyFiles:
            return i18n("Copy Files");

        case IOJobData::MoveAlbum:
            return i18n("Move Album");

        case IOJobData::MoveImage:
            return i18n("Move Images");

        case IOJobData::MoveFiles:
            return i18n("Move Files");

        case IOJobData::Trash:
            return i18n("Trash");

        case IOJobData::Delete:
            return i18n("Delete");

        default:
            break;
    }

    return QString();
}

ProgressItem* DIO::getProgressItem(IOJobData* const data) const
{
    QString itemId = data->getProgressId();

    if (itemId.isEmpty())
    {
        return 0;
    }

    return ProgressManager::instance()->findItembyId(itemId);
}

void DIO::slotCancel(ProgressItem* item)
{
    if (item)
    {
        item->setComplete();
    }
}

void DIO::addAlbumChildrenToList(QList<int>& list, Album* const album)
{
    // simple recursive helper function
    if (album)
    {
        if (!list.contains(album->id()))
        {
            list.append(album->id());
        }

        AlbumIterator it(album);

        while (it.current())
        {
            addAlbumChildrenToList(list, *it);
            ++it;
        }
    }
}

} // namespace Digikam
