/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-08
 * Description : Low level copy files and directories
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dfileoperations.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

bool DFileOperations::copyFolderRecursively(const QString& srcPath, const QString& dstPath)
{
    QDir srcDir(srcPath);
    QString newCopyPath = dstPath + QLatin1Char('/') + srcDir.dirName();

    if (!srcDir.mkpath(newCopyPath))
    {
        return false;
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Files))
    {
        QString copyPath = newCopyPath + QLatin1Char('/') + fileInfo.fileName();

        if (!QFile::copy(fileInfo.filePath(), copyPath))
            return false;
    }

    foreach (const QFileInfo& fileInfo, srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        copyFolderRecursively(fileInfo.filePath(), newCopyPath);
    }

    return true;
}

bool DFileOperations::copyFiles(const QStringList& srcPaths, const QString& dstPath)
{
    foreach (const QString& path, srcPaths)
    {
        QFileInfo fileInfo(path);
        QString copyPath = dstPath + QLatin1Char('/') + fileInfo.fileName();

        if (!QFile::copy(fileInfo.filePath(), copyPath))
            return false;
    }

    return true;
}

} // namespace Digikam
