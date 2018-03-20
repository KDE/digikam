/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : a dialog to create a new remote album for export tools
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
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

#include "flickrnewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QApplication>
#include <QStyle>

// Local includes

#include "digikam_debug.h"
#include "flickritem.h"

namespace Digikam
{

FlickrNewAlbumDlg::FlickrNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideLocation();
    getMainWidget()->setMinimumSize(350,0);
}

FlickrNewAlbumDlg::~FlickrNewAlbumDlg()
{
}

void FlickrNewAlbumDlg::getFolderProperties(FPhotoSet& folder)
{
    folder.title       = getTitleEdit()->text();
    folder.description = getDescEdit()->toPlainText();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder Title " << folder.title<<" Folder Description "<<folder.description;
}

} // namespace Digikam
