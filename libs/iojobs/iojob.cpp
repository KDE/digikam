/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs for file systems jobs
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

CopyJob::CopyJob(const QUrl& src, const QUrl& dest, bool isMove)
{
    m_src    = src;
    m_dest   = dest;
    m_isMove = isMove;
}

void CopyJob::run()
{
    QFileInfo srcInfo(m_src.toLocalFile());
    QDir dstDir(m_dest.toLocalFile());

    if (!srcInfo.exists())
    {
        emit error(i18n("File/Folder %1 does not exist anymore", srcInfo.baseName()));
        emit signalDone();
        return;
    }

    if (!dstDir.exists())
    {
        emit error(i18n("Album %1 does not exist anymore", dstDir.dirName()));
        emit signalDone();
        return;
    }

    // Checking if there is a file with the same name in destination folder
    QString destenationName = srcInfo.isFile() ? srcInfo.fileName() : srcInfo.dir().dirName();
    QString destenation     = dstDir.path() + QLatin1Char('/') + destenationName;
    QFileInfo fileInfoForDestination(destenation);

    if (fileInfoForDestination.exists())
    {
        emit error(i18n("A file or folder named %1 already exists in %2",
                        srcInfo.baseName(), QDir::toNativeSeparators(dstDir.path())));

        emit signalDone();
        return;
    }

    if (m_isMove)
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
            }
        }
        else
        {
            if (!QFile::copy(srcInfo.filePath(), destenation))
            {
                emit error(i18n("Could not copy file %1 to album %2",
                                QDir::toNativeSeparators(srcInfo.path()),
                                QDir::toNativeSeparators(dstDir.path())));
            }
        }
    }

    emit signalDone();
}

// --------------------------------------------

DeleteJob::DeleteJob(const QUrl& srcToDelete, bool useTrash, bool markAsObsolete)
{
    m_srcToDelete    = srcToDelete;
    m_useTrash       = useTrash;
    m_markAsObsolete = markAsObsolete;
}

void DeleteJob::run()
{
    QFileInfo fileInfo(m_srcToDelete.toLocalFile());
    qCDebug(DIGIKAM_IOJOB_LOG) << "DELETING:    " << fileInfo.filePath();
    qCDebug(DIGIKAM_IOJOB_LOG) << "FILE EXISTS? " << fileInfo.exists();
    qCDebug(DIGIKAM_IOJOB_LOG) << "IS TO TRASH? " << m_useTrash;

    if (!fileInfo.exists())
    {
        emit error(i18n("File/Folder %1 does not exist",
                        QDir::toNativeSeparators(fileInfo.filePath())));
        emit signalDone();
        return;
    }

    if (m_useTrash)
    {
        if (fileInfo.isDir())
        {
            if (!DTrash::deleteDirRecursivley(m_srcToDelete.toLocalFile()))
            {
                emit error(i18n("Couldn't move folder %1 to collection trash",
                                QDir::toNativeSeparators(fileInfo.path())));
            }
        }
        else
        {
            if (!DTrash::deleteImage(m_srcToDelete.toLocalFile()))
            {
                emit error(i18n("Couldn't move image %1 to collection trash",
                                QDir::toNativeSeparators(fileInfo.filePath())));
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
            }
            else if (m_markAsObsolete)
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
            }
            else if (m_markAsObsolete)
            {
                // Mark the image info of the removed file as obsolete
                CoreDbAccess access;
                qlonglong imageId = getItemFromUrl(QUrl::fromLocalFile(fileInfo.filePath()));

                if (imageId != -1)
                {
                    access.db()->setItemStatus(imageId, DatabaseItem::Status::Obsolete);
                }
            }
        }
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
        imageId = CoreDbAccess().db()->getItemFromAlbum(pAlbum->id(),fileName);
    }

    return imageId;
}

// --------------------------------------------

RenameFileJob::RenameFileJob(const QUrl& srcToRename, const QUrl& newUrl)
{
    m_srcToRename = srcToRename;
    m_newUrl      = newUrl;
}

void RenameFileJob::run()
{
    if (m_newUrl.isEmpty())
    {
        emit signalRenameFailed(m_srcToRename);
        emit signalDone();
        return;
    }

    qCDebug(DIGIKAM_IOJOB_LOG) << "Destination Url: " << m_newUrl;

    if (QFileInfo(m_newUrl.toLocalFile()).exists())
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File with the same name exists!";
        emit error(i18n("Image with the same name %1 already there",
                        QDir::toNativeSeparators(m_newUrl.toLocalFile())));
        emit signalRenameFailed(m_srcToRename);
        emit signalDone();
        return;
    }

    QFile file(m_srcToRename.toLocalFile());

    qCDebug(DIGIKAM_IOJOB_LOG) << "Trying to rename"
                               << m_srcToRename.toLocalFile() << " to "
                               << m_newUrl.toLocalFile();

    if (!file.rename(m_newUrl.toLocalFile()))
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File couldn't be renamed!";
        emit error(i18n("Image %1 could not be renamed",
                        QDir::toNativeSeparators(m_srcToRename.toLocalFile())));
        emit signalRenameFailed(m_srcToRename);
        emit signalDone();
        return;
    }

    emit signalRenamed(m_srcToRename, m_newUrl);
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

    qCDebug(DIGIKAM_IOJOB_LOG) << "collectionTrashFilesPath: " << collectionTrashFilesPath;

    QDir filesDir(collectionTrashFilesPath);

    foreach (const QFileInfo& fileInfo, filesDir.entryInfoList(QDir::Files))
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "file in trash: " << fileInfo.filePath();
        itemInfo.trashPath = fileInfo.filePath();

        DTrash::extractJsonForItem(m_collectionPath, fileInfo.baseName(), itemInfo);

        emit trashItemInfo(itemInfo);
    }

    emit signalDone();
}

} // namespace Digikam
