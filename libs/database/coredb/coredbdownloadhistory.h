/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-01
 * Description : Core database interface to manage camera item download history.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CORE_DATABASE_DOWNLOAD_HISTORY_H
#define CORE_DATABASE_DOWNLOAD_HISTORY_H

// Qt includes

#include <QString>
#include <QDateTime>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbDownloadHistory
{
public:

    enum Status
    {
        StatusUnknown = -1,
        NotDownloaded = 0,
        Downloaded    = 1
    };

public:

    /**
     * Queries the status of a download item that is uniquely described by the four parameters.
     * The identifier is recommended to be an MD5 hash of properties describing the camera,
     * if available, and the directory path (though you are free to use all four parameters as you want)
     */
    static Status status(const QString& identifier, const QString& name,
                         qlonglong fileSize, const QDateTime& date);

    /**
     * Sets the status of the item to Downloaded
     */
    static void setDownloaded(const QString& identifier, const QString& name,
                              qlonglong fileSize, const QDateTime& date);
};

} // namespace Digikam

#endif // CORE_DATABASE_DOWNLOAD_HISTORY_H
