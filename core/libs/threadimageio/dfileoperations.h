/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : misc file operation methods
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DFILE_OPERATIONS_H
#define DFILE_OPERATIONS_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QUrl>

// KDE includes

#include <kservice.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFileOperations
{
public:

    /** This method rename a local file 'orgPath' to 'destPath' with all ACL properties
     *  restoration taken from 'source' file.
     *  Return true if operation is completed.
     */
    static bool localFileRename(const QString& source,
                                const QString& orgPath,
                                const QString& destPath,
                                bool ignoreSettings = false);

    /** Open file urls to default application relevant of file type-mimes desktop configuration.
     */
    static void openFilesWithDefaultApplication(const QList<QUrl>& urls);

    /** Get unique file url if file exist by appending a counter suffix or return original url.
     */
    static QUrl getUniqueFileUrl(const QUrl& orgUrl, bool* const newurl = 0);

    /** Open file urls with the service.
     */
    static bool runFiles(KService* const service, const QList<QUrl>& urls);

    /** Open file urls with the application command.
     */
    static bool runFiles(const QString& appCmd,
                         const QList<QUrl>& urls,
                         KService* const service = 0);

    /** Return list of service available on desktop to open files.
     */
    static KService::List servicesForOpenWith(const QList<QUrl>& urls);

    /** Copy recursively a directory contents to another one.
     */
    static bool copyFolderRecursively(const QString& srcPath,
                                      const QString& dstPath);

    /** Copy a list of files to another place.
     */
    static bool copyFiles(const QStringList& srcPaths,
                          const QString& dstPath);
};

} // namespace Digikam

#endif // DFILE_OPERATIONS_H
