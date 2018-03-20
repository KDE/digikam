/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RAJCE_NEW_ALBUM_DLG_H
#define RAJCE_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

class QCheckBox;

namespace Digikam
{

class RajceNewAlbumDlg : public WSNewAlbumDialog
{
public:

    explicit RajceNewAlbumDlg(QWidget* const parent = 0);
    ~RajceNewAlbumDlg();

public:

    QString albumName()        const;
    QString albumDescription() const;
    bool    albumVisible()     const;

private:

    QCheckBox* m_albumVisible;
};

} // namespace Digikam

#endif // RAJCE_NEW_ALBUM_DLG_H
