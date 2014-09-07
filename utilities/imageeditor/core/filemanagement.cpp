/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : misc file operation methods
 *
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "filemanagement.h"

// C ANSI includes

#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

// Qt includes

#include <QFileInfo>
#include <QByteArray>
#include <QDir>

// KDE includes

#include <kdebug.h>
#include <kde_file.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kmimetypetrader.h>

// Local includes

#include "metadatasettings.h"

namespace Digikam
{

bool FileManagement::localFileRename(const QString& source, const QString& orgPath, const QString& destPath)
{
    QString dest = destPath;
    // check that we're not replacing a symlink
    QFileInfo info(dest);

    if (info.isSymLink())
    {
        dest = info.symLinkTarget();
        kDebug() << "Target filePath" << QDir::toNativeSeparators(dest) << "is a symlink pointing to"
                 << QDir::toNativeSeparators(dest) << ". Storing image there.";
    }

#ifndef Q_OS_WIN
    QByteArray dstFileName = QFile::encodeName(dest);

    // Store old permissions:
    // Just get the current umask.
    mode_t curr_umask = umask(S_IREAD | S_IWRITE);
    // Restore the umask.
    umask(curr_umask);

    // For new files respect the umask setting.
    mode_t filePermissions = (S_IREAD | S_IWRITE | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP) & ~curr_umask;

    // For existing files, use the mode of the original file.
    struct stat stbuf;

    if (::stat(dstFileName, &stbuf) == 0)
    {
        filePermissions = stbuf.st_mode;
    }
#endif // Q_OS_WIN

    struct stat st;

    if (::stat(QFile::encodeName(source), &st) == 0)
    {
        // See B.K.O #329608: Restore file modification time from original file only if updateFileTimeStamp for Setup/Metadata is turned off.

        if (!MetadataSettings::instance()->settings().updateFileTimeStamp)
        {
            struct utimbuf ut;
            ut.modtime = st.st_mtime;
            ut.actime  = st.st_atime;

            if (::utime(QFile::encodeName(orgPath), &ut) != 0)
            {
                kWarning() << "Failed to restore modification time for file " << dest;
            }
        }
    }

    // rename tmp file to dest
    // KDE::rename() takes care of QString -> bytestring encoding
    if (KDE::rename(orgPath, dest) != 0)
    {
        return false;
    }

#ifndef Q_OS_WIN
    // restore permissions
    if (::chmod(dstFileName, filePermissions) != 0)
    {
        kWarning() << "Failed to restore file permissions for file " << dstFileName;
    }
#endif // Q_OS_WIN

    return true;
}

void FileManagement::openFilesWithDefaultApplication(const KUrl::List& urls, QWidget* const parentWidget)
{
    if (urls.isEmpty())
    {
        return;
    }

    // Create a map of service depending of type mimes to route and start only one instance of relevant application with all same type mime files.

    QMap<KService::Ptr, KUrl::List> servicesMap;

    foreach (const KUrl& url, urls)
    {
        const QString mimeType = KMimeType::findByUrl(url, 0, true, true)->name();
        KService::List offers  = KMimeTypeTrader::self()->query(mimeType, "Application");

        if (offers.isEmpty())
        {
            return;
        }

        KService::Ptr ptr                                  = offers.first();
        QMap<KService::Ptr, KUrl::List>::const_iterator it = servicesMap.find(ptr);

        if (it != servicesMap.end())
        {
            servicesMap[ptr] << url;
        }
        else
        {
            servicesMap.insert(ptr, KUrl::List() << url);
        }
    }

    for (QMap<KService::Ptr, KUrl::List>::const_iterator it = servicesMap.constBegin();
         it != servicesMap.constEnd(); ++it)
    {
        // Run the dedicated app to open the item.
        KRun::run(*it.key(), it.value(), parentWidget);
    }
}

}  // namespace Digikam
