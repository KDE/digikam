/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_FB_NEW_ALBUM_DLG_H
#define DIGIKAM_FB_NEW_ALBUM_DLG_H

// Qt includes

#include <QDialog>

// Local includes

#include "wsnewalbumdialog.h"

class QComboBox;

using namespace Digikam;

namespace GenericDigikamFaceBookPlugin
{

class FbAlbum;

class FbNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit FbNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~FbNewAlbumDlg();

    void getAlbumProperties();
    void getAlbumProperties(FbAlbum& album);

private:

    QComboBox* m_privacyCoB;

    friend class FbWindow;
};

} // namespace GenericDigikamFaceBookPlugin

#endif // DIGIKAM_FB_NEW_ALBUM_DLG_H
