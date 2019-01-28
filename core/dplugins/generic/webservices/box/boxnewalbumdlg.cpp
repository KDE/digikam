/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#include "boxnewalbumdlg.h"

// Local includes

#include "digikam_debug.h"
#include "boxitem.h"

namespace DigikamGenericBoxPlugin
{

BOXNewAlbumDlg::BOXNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

BOXNewAlbumDlg::~BOXNewAlbumDlg()
{
}

void BOXNewAlbumDlg::getFolderTitle(BOXFolder& folder)
{
    folder.title = QLatin1Char('/') + getTitleEdit()->text();
}

} // namespace DigikamGenericBoxPlugin
