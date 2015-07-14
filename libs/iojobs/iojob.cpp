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
#include "digikam_export.h"
#include "albumdb.h"
#include "album.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "imageinfo.h"

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
        error(i18n("File/Folder %1 does not exist anymore", srcInfo.baseName()));
        emit signalDone();
        return;
    }

    if (!dstDir.exists())
    {
        error(i18n("Album %1 does not exist anymore", dstDir.dirName()));
        emit signalDone();
        return;
    }

    // Checking if there is a file with the same name in destination folder
    QString destenationName = srcInfo.isFile() ? srcInfo.fileName() : srcInfo.dir().dirName();
    QString destenation = dstDir.path() + QDir::separator() + destenationName;
    QFileInfo fileInfoForDestination(destenation);

    if (fileInfoForDestination.exists())
    {
        error(i18n("A file or folder named %1 already exists in %2",
                   srcInfo.baseName(), dstDir.path()));

        emit signalDone();
        return;
    }

    if (m_isMove)
    {
        if(srcInfo.isDir())
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "IT IS DIR";
            QDir srcDir(srcInfo.filePath());

            if (!srcDir.rename(srcDir.path(), destenation))
            {
                error(i18n("Could not move folder %1 to album %2",
                           srcDir.path(), dstDir.path()));
            }
        }
        else
        {
            QFile srcFile(srcInfo.filePath());

            if (!srcFile.rename(destenation))
            {
                error(i18n("Could not move file %1 to album %2",
                           srcInfo.filePath(), dstDir.path()));
            }
        }
    }
    else
    {
        if (srcInfo.isDir())
        {
            QDir srcDir(srcInfo.filePath());

            if(!copyFolderRecursively(srcDir.path(), dstDir.path()))
            {
                error(i18n("Could not copy folder %1 to album %2",
                           srcDir.path(), dstDir.path()));
            }
        }
        else
        {
            if (!QFile::copy(srcInfo.filePath(), destenation))
            {
                error(i18n("Could not copy file %1 to album %2",
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
    qCDebug(DIGIKAM_IOJOB_LOG) << "DELETING: " << fileInfo.filePath() << "\n"
                               << "FILE EXISTS? " << fileInfo.exists() << "\n"
                               << "IS TO TRASH? " << m_useTrash;

    if (!fileInfo.exists())
    {
        error(i18n("File/Folder %1 does not exist", fileInfo.filePath()));
        emit signalDone();
        return;
    }

    if (m_useTrash)
    {
        // TODO : This part must depend of desktop trash implementation, as Gnome, KDE, Windows, OSX, etc...
    }
    else
    {
        if (fileInfo.isDir())
        {
            QDir dir(fileInfo.filePath());

            if (!dir.removeRecursively())
            {
                error(i18n("Album %1 could not be removed", fileInfo.path()));
            }
        }
        else
        {
            QFile file(fileInfo.filePath());

            if (!file.remove())
            {
                error(i18n("Image %1 could not be removed", fileInfo.filePath()));
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

    qCDebug(DIGIKAM_IOJOB_LOG) << "Destination Url: " << m_newUrl << "\n"
                               << "Destination Url path: " << m_newUrl.path();

    if (QFileInfo(m_newUrl.path()).exists())
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File with the same name exists!";
        error(i18n("Image with the same name %1 already there", m_newUrl.path()));
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
        error(i18n("Image %1 could not be renamed", m_srcToRename.path()));
        emit signalDone();
        return;
    }

    emit signalRenamed(m_srcToRename, m_newUrl);
    emit signalDone();
}

} // namespace Digikam
