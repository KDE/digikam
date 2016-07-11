/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : misc file operation methods
 *
 * Copyright (C) 2014-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

// Qt includes

#include <QString>
#include <QUrl>

// KDE includes

#include <kservice.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

namespace FileOperation
{

    /** This method rename a local file 'orgPath' to 'destPath' with all ACL properties
     *  restoration taken from 'source' file.
     *  Return true if operation is completed.
     */
    DIGIKAM_EXPORT bool localFileRename(const QString& source, const QString& orgPath, const QString& destPath);

    /** Open file urls to default application relevant of file type-mimes desktop configration.
     */
    DIGIKAM_EXPORT void openFilesWithDefaultApplication(const QList<QUrl>& urls);

    /** Get unique file url if file exist by appending a counter suffix or return original url.
     */
    DIGIKAM_EXPORT QUrl getUniqueFileUrl(const QUrl& orgUrl, bool* const newurl = 0);

    /** Open file urls with the service.
     */
    DIGIKAM_EXPORT bool runFiles(const KService& service, const QList<QUrl>& urls);

    /** Open file urls with the application command.
     */
    DIGIKAM_EXPORT bool runFiles(const QString& appCmd, const QList<QUrl>& urls, const QString& name = QString(),
                                                                                 const QString& icon = QString());

    /** Return list of service available on desktop to open files.
     */
    DIGIKAM_EXPORT KService::List servicesForOpenWith(const QList<QUrl>& urls);

} // namespace FileOperation

} // namespace Digikam

#endif /* FILE_OPERATION_H */
