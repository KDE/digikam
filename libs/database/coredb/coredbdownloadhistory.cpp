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

#include "coredbdownloadhistory.h"

// Local includes

#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

CoreDbDownloadHistory::Status CoreDbDownloadHistory::status(const QString& identifier, const QString& name,
                                                qlonglong fileSize, const QDateTime& date)
{
    int id = CoreDbAccess().db()->findInDownloadHistory(identifier, name, fileSize, date);

    if (id != -1)
    {
        return Downloaded;
    }
    else
    {
        return NotDownloaded;
    }
}

void CoreDbDownloadHistory::setDownloaded(const QString& identifier, const QString& name,
                                    qlonglong fileSize, const QDateTime& date)
{
    CoreDbAccess().db()->addToDownloadHistory(identifier, name, fileSize, date);
}

} // namespace Digikam
