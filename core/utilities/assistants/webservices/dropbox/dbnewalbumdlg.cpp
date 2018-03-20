/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dbnewalbumdlg.h"

// Local includes

#include "digikam_debug.h"
#include "dbitem.h"

namespace Digikam
{

DBNewAlbumDlg::DBNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

DBNewAlbumDlg::~DBNewAlbumDlg()
{
}

void DBNewAlbumDlg::getFolderTitle(DBFolder& folder)
{
    folder.title = QLatin1String("/") + getTitleEdit()->text();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getFolderTitle:" << folder.title;
}

} // namespace Digikam
