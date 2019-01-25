/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
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

#ifndef DIGIKAM_OD_NEW_ALBUM_DLG_H
#define DIGIKAM_OD_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace GenericDigikamOneDrivePlugin
{

class ODFolder;

class ODNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit ODNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~ODNewAlbumDlg();

    void getFolderTitle(ODFolder& folder);
};

} // namespace GenericDigikamOneDrivePlugin

#endif // DIGIKAM_OD_NEW_ALBUM_DLG_H
