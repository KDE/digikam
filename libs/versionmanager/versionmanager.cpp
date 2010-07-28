/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Qt includes

#include <QFileInfo>
#include <QDir>
#include <QFile>

// KDE includes

#include <KDiskFreeSpaceInfo>
#include <KDebug>

// Local includes

#include "versionmanager.h"
#include "dmetadata.h"

namespace Digikam
{

QString VersionManager::getVersionedFilename(const QString& originalPath, const QString& originalName, 
                                             qint64 fileSize, bool editingOriginal, bool fork)
{
    kDebug() << "Original path: " << originalPath << " | Original name: " << originalName
             << " | Editing original: " << editingOriginal;

    //check if we have enough free space for another file
    KDiskFreeSpaceInfo diskInfo = KDiskFreeSpaceInfo::freeSpaceInfo(originalPath);

    if(diskInfo.isValid() && diskInfo.available() > (uint)fileSize * 2)
    {
        //DMetadata metadata(originalPath + "/" + originalName);
        QString newFileName;

        if(!editingOriginal)
        {
            //the user is editing some subversion of the original image
            if(fork)
            {
                //user wants to create a fork of current version
            }
            else
            {
                kDebug() << "Subversion image, will use this version";
                kDebug() << originalName;
                return originalName;
            }
        }
        else
        {
            //the user is editing the original image itself, determine last saved version number and create a new one
            kDebug() << "Original image, will create a new version";
            QFileInfo fileInfo(originalPath + QString("/") + originalName);
            QDir dirInfo(originalPath);

            // To find the right number for the new version, go through all the items in the given dir, 
            // the version number won't be bigger than count()
            for(uint i = 1; i < dirInfo.count(); i++)
            {
                newFileName.clear();
                newFileName.append(fileInfo.completeBaseName());
                newFileName.append("_v");
                newFileName.append(QString::number(i));
                newFileName.append(".");
                newFileName.append(fileInfo.suffix());
                kDebug() << newFileName;
                QFile newFile(originalPath + QString("/") + newFileName);
                if(!newFile.exists()) 
                {
                    kDebug() << newFileName;
                    return newFileName;
                }
            }
        }
    }

    // FIXME: if space is not enough, do not return a null string there.
    return QString();
}

VersionManager* VersionManager::m_instance = 0;

VersionManager* VersionManager::instance()
{
    if (!m_instance)
        m_instance = new VersionManager;

    return m_instance;
}

} // namespace Digikam
