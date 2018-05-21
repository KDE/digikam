/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs for file systems jobs
 *
 * Copyright (C) 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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
#include "imageinfo.h"
#include "dtrash.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "albummanager.h"
#include "dfileoperations.h"

namespace Digikam
{

IOJob::IOJob()
{
}

// --------------------------------------------

CopyOrMoveJob::CopyOrMoveJob(IOJobData* const data)
{
    m_data = data;
}

void CopyOrMoveJob::run()
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
            emit signalError(i18n("File/Folder %1 does not exist anymore", srcInfo.baseName()));
            continue;
        }

        if (!dstDir.exists())
        {
            emit signalError(i18n("Album %1 does not exist anymore", dstDir.dirName()));
            continue;
        }

        // Checking if there is a file with the same name in destination folder
        QString destenationName = srcInfo.isFile() ? srcInfo.fileName() : srcInfo.dir().dirName();
        QString destenation     = dstDir.path() + QLatin1Char('/') + destenationName;
        QFileInfo fileInfoForDestination(destenation);

        if (fileInfoForDestination.exists())
        {
            emit signalError(i18n("A file or folder named %1 already exists in %2",
                                  srcInfo.baseName(), QDir::toNativeSeparators(dstDir.path())));
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
                    if (!DFileOperations::copyFolderRecursively(srcDir.path(), dstDir.path(), &m_cancel))
                    {
                        if (m_cancel)
                        {
                            break;
                        }

                        emit signalError(i18n("Could not move folder %1 to album %2",
                                              QDir::toNativeSeparators(srcDir.path()),
                                              QDir::toNativeSeparators(dstDir.path())));

                        continue;
                    }
                    else if (!srcDir.removeRecursively())
                    {
                        emit signalError(i18n("Could not move folder %1 to album %2. "
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
                    emit signalError(i18n("Could not move file %1 to album %2",
                                          srcInfo.filePath(),
                                          QDir::toNativeSeparators(dstDir.path())));

                    continue;
                }
           }
        }
        else
        {
            if (srcInfo.isDir())
            {
                QDir srcDir(srcInfo.filePath());

                if (!DFileOperations::copyFolderRecursively(srcDir.path(), dstDir.path(), &m_cancel))
                {
                    if (m_cancel)
                    {
                        break;
                    }

                    emit signalError(i18n("Could not copy folder %1 to album %2",
                                          QDir::toNativeSeparators(srcDir.path()),
                                          QDir::toNativeSeparators(dstDir.path())));

                    continue;
                }
            }
            else
            {
                if (!QFile::copy(srcInfo.filePath(), destenation))
                {
                    emit signalError(i18n("Could not copy file %1 to album %2",
                                          QDir::toNativeSeparators(srcInfo.path()),
                                          QDir::toNativeSeparators(dstDir.path())));

                    continue;
                }

            }
        }

        emit signalOneProccessed(srcUrl);
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
            emit signalError(i18n("File/Folder %1 does not exist",
                                  QDir::toNativeSeparators(fileInfo.filePath())));

            continue;
        }

        if (useTrash)
        {
            if (fileInfo.isDir())
            {
                if (!DTrash::deleteDirRecursivley(deleteUrl.toLocalFile()))
                {
                    emit signalError(i18n("Could not move folder %1 to collection trash",
                                          QDir::toNativeSeparators(fileInfo.path())));

                    continue;
                }
            }
            else
            {
                if (!DTrash::deleteImage(deleteUrl.toLocalFile()))
                {
                    emit signalError(i18n("Could not move image %1 to collection trash",
                                          QDir::toNativeSeparators(fileInfo.filePath())));

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
                    emit signalError(i18n("Album %1 could not be removed",
                                          QDir::toNativeSeparators(fileInfo.path())));

                    continue;
                }
            }
            else
            {
                QFile file(fileInfo.filePath());

                if (!file.remove())
                {
                    emit signalError(i18n("Image %1 could not be removed",
                                          QDir::toNativeSeparators(fileInfo.filePath())));

                    continue;
                }
            }
        }

        emit signalOneProccessed(deleteUrl);
    }

    emit signalDone();
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
            if (m_data->overwrite())
            {
                QFile::remove(destUrl.toLocalFile());
            }
            else
            {
                qCDebug(DIGIKAM_IOJOB_LOG) << "File with the same name exists!";
                emit signalError(i18n("Image with the same name %1 already there",
                                      QDir::toNativeSeparators(destUrl.toLocalFile())));

                emit signalRenameFailed(renameUrl);
                continue;
            }
        }

        QFile file(renameUrl.toLocalFile());

        qCDebug(DIGIKAM_IOJOB_LOG) << "Trying to rename"
                                   << renameUrl.toLocalFile() << "to"
                                   << destUrl.toLocalFile();

        if (!file.rename(destUrl.toLocalFile()))
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "File could not be renamed!";
            emit signalError(i18n("Image %1 could not be renamed",
                                  QDir::toNativeSeparators(renameUrl.toLocalFile())));

            emit signalRenameFailed(renameUrl);
            continue;
        }

        emit signalOneProccessed(renameUrl);
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
            qCDebug(DIGIKAM_IOJOB_LOG) << "Trash file could not be renamed!";
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
    QList<int> albumsFromImages;
    QList<qlonglong> imagesToRemove;

    foreach (const DTrashItemInfo& item, m_dtrashItemInfoList)
    {
        QFile::remove(item.trashPath);
        QFile::remove(item.jsonFilePath);

        imagesToRemove   << item.imageId;
        albumsFromImages << ImageInfo(item.imageId).albumId();

        access.db()->removeAllImageRelationsFrom(item.imageId, DatabaseRelation::Grouped);
    }

    access.db()->removeItemsPermanently(imagesToRemove, albumsFromImages);

    emit signalDone();
}

} // namespace Digikam
