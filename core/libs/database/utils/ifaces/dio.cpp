/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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
#include "iteminfo.h"
#include "albummanager.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "album.h"
#include "dmetadata.h"
#include "metaenginesettings.h"
#include "scancontroller.h"
#include "thumbsdb.h"
#include "thumbsdbaccess.h"
#include "iojobsmanager.h"
#include "collectionmanager.h"
#include "dnotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "progressmanager.h"
#include "digikamapp.h"
#include "iojobdata.h"

namespace Digikam
{

SidecarFinder::SidecarFinder(const QList<QUrl>& files)
{
    foreach (const QUrl& url, files)
    {
        if (DMetadata::hasSidecar(url.toLocalFile()))
        {
            localFiles << DMetadata::sidecarUrl(url);
            localFileSuffixes << QLatin1String(".xmp");
            qCDebug(DIGIKAM_DATABASE_LOG) << "Detected a sidecar" << localFiles.last();
        }

        foreach (QString suffix, MetaEngineSettings::instance()->settings().sidecarExtensions)
        {
            suffix = QLatin1Char('.') + suffix;
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
GroupedImagesFinder::GroupedImagesFinder(const QList<ItemInfo>& source)
{
    QSet<qlonglong> ids;

    foreach (const ItemInfo& info, source)
    {
        ids << info.id();
    }

    infos.reserve(source.size());

    foreach (const ItemInfo& info, source)
    {
        infos << info;

        if (info.hasGroupedImages())
        {
            foreach (const ItemInfo& groupedImage, info.groupedImages())
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

class Q_DECL_HIDDEN DIOCreator
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
    m_processingCount = 0;
}

DIO::~DIO()
{
}

void DIO::cleanUp()
{
}

bool DIO::itemsUnderProcessing()
{
    return instance()->m_processingCount;
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

void DIO::copy(const QList<ItemInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyImage, infos, dest));
}

void DIO::move(const QList<ItemInfo>& infos, PAlbum* const dest)
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

void DIO::rename(const QUrl& src, const QString& newName, bool overwrite)
{
    if (src.isEmpty() || newName.isEmpty())
    {
        return;
    }

    ItemInfo info = ItemInfo::fromUrl(src);

    instance()->processJob(new IOJobData(IOJobData::Rename, info, newName, overwrite));
}

// Delete --------------------------------------------------------------

void DIO::del(const QList<ItemInfo>& infos, bool useTrash)
{
    instance()->processJob(new IOJobData(useTrash ? IOJobData::Trash
                                                  : IOJobData::Delete, infos));
}

void DIO::del(const ItemInfo& info, bool useTrash)
{
    del(QList<ItemInfo>() << info, useTrash);
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
        GroupedImagesFinder finder(data->itemInfos());
        data->setItemInfos(finder.infos);

        QStringList      filenames;
        QList<qlonglong> ids;

        foreach (const ItemInfo& info, data->itemInfos())
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
        if (!data->itemInfos().isEmpty())
        {
            ItemInfo info      = data->itemInfos().first();
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

    ProgressItem* item = nullptr;
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
            this, SLOT(slotOneProccessed(QUrl)));

    connect(jobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));

    if (data->operation() == IOJobData::Rename)
    {
        connect(jobThread, SIGNAL(signalRenameFailed(QUrl)),
                this, SIGNAL(signalRenameFailed(QUrl)));

        connect(jobThread, SIGNAL(finished()),
                this, SIGNAL(signalRenameFinished()));
    }

    if (item)
    {
        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                jobThread, SLOT(slotCancel()));

        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                this, SLOT(slotCancel(ProgressItem*)));
    }

    ++m_processingCount;
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
        QString errors = jobThread->errorsList().join(QLatin1Char('\n'));
        DNotificationWrapper(QString(), errors, DigikamApp::instance(),
                             DigikamApp::instance()->windowTitle());
    }

    if (m_processingCount)
    {
        --m_processingCount;
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
        ItemInfo info = data->findItemInfo(url);

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

            foreach (int albumId, albumsToDelete)
            {
                imagesToRemove << access.db()->getItemIDsInAlbum(albumId);
            }

            foreach (const qlonglong& imageId, imagesToRemove)
            {
                access.db()->removeAllImageRelationsFrom(imageId,
                                                         DatabaseRelation::Grouped);
            }

            access.db()->removeItemsPermanently(imagesToRemove, albumsToDelete);
        }
        else
        {
            ItemInfo info = data->findItemInfo(url);

            if (!info.isNull())
            {
                access.db()->removeAllImageRelationsFrom(info.id(),
                                                         DatabaseRelation::Grouped);

                access.db()->removeItemsPermanently(QList<qlonglong>() << info.id(),
                                                    QList<int>() << info.albumId());
            }
        }
    }
    else if (operation == IOJobData::Trash)
    {
        ItemInfo info = data->findItemInfo(url);

        if (!info.isNull())
        {
            CoreDbAccess().db()->removeItems(QList<qlonglong>() << info.id(),
                                             QList<int>() << info.albumId());
        }
    }
    else if (operation == IOJobData::Rename)
    {
        ItemInfo info = data->findItemInfo(url);

        if (!info.isNull())
        {
            QString oldPath = url.toLocalFile();
            QString newName = data->destUrl(url).fileName();
            QString newPath = data->destUrl(url).toLocalFile();

            if (data->overwrite())
            {
                ThumbsDbAccess().db()->removeByFilePath(newPath);
                LoadingCacheInterface::fileChanged(newPath, false);
                CoreDbAccess().db()->deleteItem(info.albumId(), newName);
            }

            ThumbsDbAccess().db()->renameByFilePath(oldPath, newPath);
            // Remove old thumbnails and images from the cache
            LoadingCacheInterface::fileChanged(oldPath, false);
            // Rename in ItemInfo and database
            info.setName(newName);
        }
    }

    // Scan folders for changes

    if (operation == IOJobData::Delete || operation == IOJobData::Trash ||
        operation == IOJobData::MoveAlbum)
    {
        PAlbum* const album = data->srcAlbum();
        QString scanPath;

        if (album)
        {
            PAlbum* const parent = dynamic_cast<PAlbum*>(album->parent());

            if (parent)
            {
                scanPath = parent->fileUrl().toLocalFile();
            }
        }

        if (scanPath.isEmpty())
        {
            scanPath = url.adjusted(QUrl::RemoveFilename).toLocalFile();
        }

        ScanController::instance()->scheduleCollectionScanRelaxed(scanPath);
    }

    if (operation == IOJobData::CopyImage || operation == IOJobData::CopyAlbum ||
        operation == IOJobData::CopyFiles || operation == IOJobData::MoveImage ||
        operation == IOJobData::MoveAlbum || operation == IOJobData::MoveFiles)
    {
        QString scanPath = data->destUrl().toLocalFile();
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
        return nullptr;
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

void DIO::slotDateTimeForUrl(const QUrl& url, const QDateTime& dt, bool updModDate)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        info.setDateTime(dt);

        if (updModDate)
        {
            info.setModDateTime(dt);
        }
    }
}

} // namespace Digikam
