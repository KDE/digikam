/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#ifndef DIGIKAM_TW_NEW_ALBUM_DLG_H
#define DIGIKAM_TW_NEW_ALBUM_DLG_H

// Qt includes

#include <QDialog>

// Local includes

#include "wsnewalbumdialog.h"

class QComboBox;

namespace Digikam
{

class TwAlbum;

class TwNewAlbumDlg : public WSNewAlbumDialog
{
     Q_OBJECT

public:

     explicit TwNewAlbumDlg(QWidget* const parent, const QString& toolName);
     ~TwNewAlbumDlg();

     void getAlbumProperties(TwAlbum& album);

private:

     friend class TwWindow;
};

} // namespace Digikam

#endif // DIGIKAM_TW_NEW_ALBUM_DLG_H
