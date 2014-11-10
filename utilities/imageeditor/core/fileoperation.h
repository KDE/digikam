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

#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H

// Qt includes

#include <QString>

// KDE includes

#include <kurl.h>
#include <kservice.h>

// Local includes

#include "digikam_export.h"

class QWidget;

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
    DIGIKAM_EXPORT void openFilesWithDefaultApplication(const KUrl::List& urls, QWidget* const parentWidget);

    /** Return list of service available on desktop to open files.
     */
    DIGIKAM_EXPORT KService::List servicesForOpenWith(const KUrl::List& urls);

} // namespace FileOperation

} // namespace Digikam

#endif /* FILEMANAGEMENT_H */
