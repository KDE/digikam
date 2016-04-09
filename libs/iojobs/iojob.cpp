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

#include <QFile>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dtrash.h"

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

bool CopyJob::copyFolderRecursively(const QString& srcPath, const QString& dstPath)
{
    QDir srcDir(srcPath);
    QString newCopyPath = dstPath + QDir::separator() + srcDir.dirName();

    if (!srcDir.mkpath(newCopyPath))
    {
        return false;
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Files))
    {
        QString copyPath = newCopyPath + QDir::separator() + fileInfo.fileName();

        if (!QFile::copy(fileInfo.filePath(), copyPath))
            return false;
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        copyFolderRecursively(fileInfo.filePath(), newCopyPath);
    }

    return true;
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
    QString destenation     = dstDir.path() + QDir::separator() + destenationName;
    QFileInfo fileInfoForDestination(destenation);

    if (fileInfoForDestination.exists())
    {
        emit error(i18n("A file or folder named %1 already exists in %2",
                        srcInfo.baseName(), dstDir.path()));

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
                if (!copyFolderRecursively(srcDir.path(), dstDir.path()))
                {
                    emit error(i18n("Could not move folder %1 to album %2",
                                    srcDir.path(), dstDir.path()));
                }
                else if (!srcDir.removeRecursively())
                {
                    emit error(i18n("Could not move folder %1 to album %2. "
                                    "The folder %1 was copied as well to album %2",
                                    srcDir.path(), dstDir.path()));
                }
            }
        }
        else
        {
            QFile srcFile(srcInfo.filePath());

            if (!srcFile.rename(destenation))
            {
                emit error(i18n("Could not move file %1 to album %2",
                                srcInfo.filePath(), dstDir.path()));
            }
        }
    }
    else
    {
        if (srcInfo.isDir())
        {
            QDir srcDir(srcInfo.filePath());

            if (!copyFolderRecursively(srcDir.path(), dstDir.path()))
            {
                emit error(i18n("Could not copy folder %1 to album %2",
                                srcDir.path(), dstDir.path()));
            }
        }
        else
        {
            if (!QFile::copy(srcInfo.filePath(), destenation))
            {
                emit error(i18n("Could not copy file %1 to album %2",
                                srcInfo.path(), dstDir.path()));
            }
        }
    }

    emit signalDone();
}

// --------------------------------------------

DeleteJob::DeleteJob(const QUrl& srcToDelete, bool useTrash)
{
    m_srcToDelete = srcToDelete;
    m_useTrash    = useTrash;
}

void DeleteJob::run()
{
    QFileInfo fileInfo(m_srcToDelete.path());
    qCDebug(DIGIKAM_IOJOB_LOG) << "DELETING: "    << fileInfo.filePath() << "\n"
                               << "FILE EXISTS? " << fileInfo.exists()   << "\n"
                               << "IS TO TRASH? " << m_useTrash;

    if (!fileInfo.exists())
    {
        emit error(i18n("File/Folder %1 does not exist", fileInfo.filePath()));
        emit signalDone();
        return;
    }

    if (m_useTrash)
    {
        if (fileInfo.isDir())
        {
            if (!DTrash::deleteDirRecursivley(m_srcToDelete.path()))
            {
                emit error(i18n("Couldn't move Dir %1 to collection trash", fileInfo.path()));
            }
        }
        else
        {
            if (!DTrash::deleteImage(m_srcToDelete.path()))
            {
                emit error(i18n("Couldn't move image %1 to collection trash", fileInfo.filePath()));
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
                emit error(i18n("Album %1 could not be removed", fileInfo.path()));
            }
        }
        else
        {
            QFile file(fileInfo.filePath());

            if (!file.remove())
            {
                emit error(i18n("Image %1 could not be removed", fileInfo.filePath()));
            }
        }
    }

    emit signalDone();
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
        emit signalDone();
        return;
    }

    qCDebug(DIGIKAM_IOJOB_LOG) << "Destination Url: "      << m_newUrl << "\n"
                               << "Destination Url path: " << m_newUrl.path();

    if (QFileInfo(m_newUrl.path()).exists())
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File with the same name exists!";
        emit error(i18n("Image with the same name %1 already there", m_newUrl.path()));
        emit signalDone();
        return;
    }

    QFile file(m_srcToRename.path());

    qCDebug(DIGIKAM_IOJOB_LOG) << "Trying to rename"
                               << m_srcToRename.toLocalFile() << "\nto "
                               << m_newUrl.path();

    if (!file.rename(m_newUrl.path()))
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File couldn't be renamed!";
        emit error(i18n("Image %1 could not be renamed", m_srcToRename.path()));
        emit signalDone();
        return;
    }

    emit signalRenamed(m_srcToRename, m_newUrl);
    emit signalDone();
}

// ----------------------------------------------

DTrashItemsListingJob::DTrashItemsListingJob(const QString &collectionPath)
{
    m_collectionPath = collectionPath;
}

void DTrashItemsListingJob::run()
{
    DTrashItemInfo itemInfo;

    QString collectionTrashFilesPath = m_collectionPath + QDir::separator() + DTrash::TRASH_FOLDER +
                                       QDir::separator() + DTrash::FILES_FOLDER;

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
