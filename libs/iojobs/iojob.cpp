/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs for file systems jobs
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "iojob.h"

// Qt includes

#include <QDir>
#include <QDirIterator>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dtrash.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "collectionmanager.h"
#include "albummanager.h"
#include "dfileoperations.h"

namespace Digikam
{

IOJob::IOJob()
{
}

// --------------------------------------------

CopyJob::CopyJob(IOJobData* const data)
{
    m_data = data;
}

void CopyJob::run()
{
    while (m_data && !m_cancel)
    {
        QUrl srcUrl = m_data->getNextUrl();

        if (srcUrl.isEmpty())
        {
            break;
        }

        QFileInfo srcInfo(srcUrl.toLocalFile());
        QDir dstDir(m_data->destUrl().toLocalFile());

        if (!srcInfo.exists())
        {
            emit error(i18n("File/Folder %1 does not exist anymore", srcInfo.baseName()));
            emit signalOneProccessed(m_data->operation());
            continue;
        }

        if (!dstDir.exists())
        {
            emit error(i18n("Album %1 does not exist anymore", dstDir.dirName()));
            emit signalOneProccessed(m_data->operation());
            continue;
        }

        // Checking if there is a file with the same name in destination folder
        QString destenationName = srcInfo.isFile() ? srcInfo.fileName() : srcInfo.dir().dirName();
        QString destenation     = dstDir.path() + QLatin1Char('/') + destenationName;
        QFileInfo fileInfoForDestination(destenation);

        if (fileInfoForDestination.exists())
        {
            emit error(i18n("A file or folder named %1 already exists in %2",
                            srcInfo.baseName(), QDir::toNativeSeparators(dstDir.path())));
            emit signalOneProccessed(m_data->operation());
            continue;
        }

        if (m_data->operation() == IOJobData::MoveAlbum ||
            m_data->operation() == IOJobData::MoveImage ||
            m_data->operation() == IOJobData::MoveFiles)
        {
            if (srcInfo.isDir())
            {
                QDir srcDir(srcInfo.filePath());

                if (!srcDir.rename(srcDir.path(), destenation))
                {
                    // If QDir::rename fails, try copy and remove.
                    if (!DFileOperations::copyFolderRecursively(srcDir.path(), dstDir.path()))
                    {
                        emit error(i18n("Could not move folder %1 to album %2",
                                        QDir::toNativeSeparators(srcDir.path()),
                                        QDir::toNativeSeparators(dstDir.path())));

                        emit signalOneProccessed(m_data->operation());
                        continue;
                    }
                    else if (!srcDir.removeRecursively())
                    {
                        emit error(i18n("Could not move folder %1 to album %2. "
                                        "The folder %1 was copied as well to album %2",
                                        QDir::toNativeSeparators(srcDir.path()),
                                        QDir::toNativeSeparators(dstDir.path())));
                    }
                }
            }
            else
            {
                QFile srcFile(srcInfo.filePath());

                if (!srcFile.rename(destenation))
                {
                    emit error(i18n("Could not move file %1 to album %2",
                                    srcInfo.filePath(),
                                    QDir::toNativeSeparators(dstDir.path())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }
           }
        }
        else
        {
            if (srcInfo.isDir())
            {
                QDir srcDir(srcInfo.filePath());

                if (!DFileOperations::copyFolderRecursively(srcDir.path(), dstDir.path()))
                {
                    emit error(i18n("Could not copy folder %1 to album %2",
                                    QDir::toNativeSeparators(srcDir.path()),
                                    QDir::toNativeSeparators(dstDir.path())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }
            }
            else
            {
                if (!QFile::copy(srcInfo.filePath(), destenation))
                {
                    emit error(i18n("Could not copy file %1 to album %2",
                                    QDir::toNativeSeparators(srcInfo.path()),
                                    QDir::toNativeSeparators(dstDir.path())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }

            }
        }

        emit signalOneProccessed(m_data->operation());
        m_data->addProcessedUrl(srcUrl);
    }

    emit signalDone();
}

// --------------------------------------------

DeleteJob::DeleteJob(IOJobData* const data)
{
    m_data = data;
}

void DeleteJob::run()
{
    while (m_data && !m_cancel)
    {
        QUrl deleteUrl = m_data->getNextUrl();

        if (deleteUrl.isEmpty())
        {
            break;
        }

        bool useTrash = (m_data->operation() == IOJobData::Trash);

        QFileInfo fileInfo(deleteUrl.toLocalFile());
        qCDebug(DIGIKAM_IOJOB_LOG) << "Deleting:   " << fileInfo.filePath();
        qCDebug(DIGIKAM_IOJOB_LOG) << "File exists?" << fileInfo.exists();
        qCDebug(DIGIKAM_IOJOB_LOG) << "Is to trash?" << useTrash;

        if (!fileInfo.exists())
        {
            emit error(i18n("File/Folder %1 does not exist",
                            QDir::toNativeSeparators(fileInfo.filePath())));

            emit signalOneProccessed(m_data->operation());
            continue;
        }

        if (useTrash)
        {
            if (fileInfo.isDir())
            {
                if (!DTrash::deleteDirRecursivley(deleteUrl.toLocalFile()))
                {
                    emit error(i18n("Couldn't move folder %1 to collection trash",
                                    QDir::toNativeSeparators(fileInfo.path())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }
            }
            else
            {
                if (!DTrash::deleteImage(deleteUrl.toLocalFile()))
                {
                    emit error(i18n("Couldn't move image %1 to collection trash",
                                QDir::toNativeSeparators(fileInfo.filePath())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }
            }
        }
        else
        {
            if (fileInfo.isDir())
            {
                QDir dir(fileInfo.filePath());

                if (!dir.removeRecursively())
                {
                    emit error(i18n("Album %1 could not be removed",
                                    QDir::toNativeSeparators(fileInfo.path())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }

                if (m_data->operation() == IOJobData::Delete)
                {
                    CoreDbAccess access;
                    // If the images in the directory should be marked as obsolete
                    // get all files recursively and remove all image information
                    // for which the file path leads to an image id.
                    QList<qlonglong> imageIds;
                    QDirIterator iter(dir);

                    while (iter.hasNext())
                    {
                        // get the next path and advance the iterator
                        QString filePath = iter.next();
                        // Use the file info to get the file type
                        QFileInfo info = iter.fileInfo();

                        if (info.isFile())
                        {
                            qlonglong imageId = getItemFromUrl(QUrl::fromLocalFile(filePath));

                            if (imageId != -1)
                            {
                                imageIds << imageId;
                            }
                        }
                    }

                    // Mark all image ids as obsolete.
                    foreach(qlonglong imageId, imageIds)
                    {
                        access.db()->setItemStatus(imageId, DatabaseItem::Status::Obsolete);
                    }
                }
            }
            else
            {
                QFile file(fileInfo.filePath());

                if (!file.remove())
                {
                    emit error(i18n("Image %1 could not be removed",
                                    QDir::toNativeSeparators(fileInfo.filePath())));

                    emit signalOneProccessed(m_data->operation());
                    continue;
                }

                if (m_data->operation() == IOJobData::Delete)
                {
                    CoreDbAccess access;
                    // Mark the image info of the removed file as obsolete
                    qlonglong imageId = getItemFromUrl(QUrl::fromLocalFile(fileInfo.filePath()));

                    if (imageId != -1)
                    {
                        access.db()->setItemStatus(imageId, DatabaseItem::Status::Obsolete);
                    }
                }
            }
        }

        emit signalOneProccessed(m_data->operation());
        m_data->addProcessedUrl(deleteUrl);
    }

    emit signalDone();
}

qlonglong DeleteJob::getItemFromUrl(const QUrl& url)
{
    QString fileName     = url.fileName();
    // Get the album path, i.e. collection + album. For this,
    // get the n leftmost characters where n is the complete path without the size of the filename
    QString completePath = url.toLocalFile();
    QString albumPath    = CollectionManager::instance()->album(completePath);

    qlonglong imageId    = -1;
    // Get the album and with this the image id of the image to trash.
    PAlbum* const pAlbum = AlbumManager::instance()->findPAlbum(QUrl::fromLocalFile(completePath));

    if (pAlbum)
    {
        imageId = CoreDbAccess().db()->getItemFromAlbum(pAlbum->id(), fileName);
    }

    return imageId;
}

// --------------------------------------------

RenameFileJob::RenameFileJob(IOJobData* const data)
{
    m_data = data;
}

void RenameFileJob::run()
{
    while (m_data && !m_cancel)
    {
        QUrl renameUrl = m_data->getNextUrl();

        if (renameUrl.isEmpty())
        {
            break;
        }

        QUrl destUrl = m_data->destUrl(renameUrl);

        qCDebug(DIGIKAM_IOJOB_LOG) << "Destination Url:" << destUrl;

        if (QFileInfo(destUrl.toLocalFile()).exists())
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "File with the same name exists!";
            emit error(i18n("Image with the same name %1 already there",
                            QDir::toNativeSeparators(destUrl.toLocalFile())));

            emit signalOneProccessed(m_data->operation());
            emit signalRenameFailed(renameUrl);
            continue;
        }

        QFile file(renameUrl.toLocalFile());

        qCDebug(DIGIKAM_IOJOB_LOG) << "Trying to rename"
                                   << renameUrl.toLocalFile() << "to"
                                   << destUrl.toLocalFile();

        if (!file.rename(destUrl.toLocalFile()))
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "File couldn't be renamed!";
            emit error(i18n("Image %1 could not be renamed",
                            QDir::toNativeSeparators(renameUrl.toLocalFile())));

            emit signalOneProccessed(m_data->operation());
            emit signalRenameFailed(renameUrl);
            continue;
        }

        emit signalOneProccessed(m_data->operation());
        m_data->addProcessedUrl(renameUrl);
        emit signalRenamed(renameUrl);
    }

    emit signalDone();
}

// ----------------------------------------------

DTrashItemsListingJob::DTrashItemsListingJob(const QString& collectionPath)
{
    m_collectionPath = collectionPath;
}

void DTrashItemsListingJob::run()
{
    DTrashItemInfo itemInfo;

    QString collectionTrashFilesPath = m_collectionPath + QLatin1Char('/') + DTrash::TRASH_FOLDER +
                                       QLatin1Char('/') + DTrash::FILES_FOLDER;

    qCDebug(DIGIKAM_IOJOB_LOG) << "Collection trash files path:" << collectionTrashFilesPath;

    QDir filesDir(collectionTrashFilesPath);

    foreach (const QFileInfo& fileInfo, filesDir.entryInfoList(QDir::Files))
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File in trash:" << fileInfo.filePath();
        itemInfo.trashPath = fileInfo.filePath();

        DTrash::extractJsonForItem(m_collectionPath, fileInfo.baseName(), itemInfo);

        emit trashItemInfo(itemInfo);
    }

    emit signalDone();
}

// ----------------------------------------------

RestoreDTrashItemsJob::RestoreDTrashItemsJob(const DTrashItemInfoList& infos)
{
    m_dtrashItemInfoList = infos;
}

void RestoreDTrashItemsJob::run()
{
    foreach (const DTrashItemInfo& item, m_dtrashItemInfoList)
    {
        QUrl srcToRename = QUrl::fromLocalFile(item.collectionPath);
        QUrl newName     = DFileOperations::getUniqueFileUrl(srcToRename);

        QFileInfo fi(item.collectionPath);

        if (!fi.dir().exists())
        {
            fi.dir().mkpath(fi.dir().path());
        }

        if (!QFile::rename(item.trashPath, newName.toLocalFile()))
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "Trash file couldn't be renamed!";
        }
        else
        {
            QFile::remove(item.jsonFilePath);
        }
    }

    emit signalDone();
}

// ----------------------------------------------

DeleteDTrashItemsJob::DeleteDTrashItemsJob(const DTrashItemInfoList& infos)
{
    m_dtrashItemInfoList = infos;
}

void DeleteDTrashItemsJob::run()
{
    CoreDbAccess access;

    foreach (const DTrashItemInfo& item, m_dtrashItemInfoList)
    {
        QFile::remove(item.trashPath);
        QFile::remove(item.jsonFilePath);
        // Set the status of the image id to obsolete, i.e. to remove.
        access.db()->setItemStatus(item.imageId, DatabaseItem::Status::Obsolete);
    }

    emit signalDone();
}

} // namespace Digikam
