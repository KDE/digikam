/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2011 by Roman Tsisyk <roman at tsisyk dot com>
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

#ifndef YF_NEW_ALBUM_DLG_H
#define YF_NEW_ALBUM_DLG_H

// Qt includes

#include <QDialog>
#include <QLineEdit>

// Local includes

#include "yfalbum.h"
#include "wsnewalbumdialog.h"

namespace Digikam
{

class YFNewAlbumDlg: public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit YFNewAlbumDlg(QWidget* const parent, YandexFotkiAlbum& album);
    ~YFNewAlbumDlg();

    YandexFotkiAlbum& album() const
    {
        return m_album;
    }

protected Q_SLOTS:

    void slotOkClicked();

protected:

    QLineEdit*        m_passwordEdit;

    YandexFotkiAlbum& m_album;
};

} // namespace Digikam

#endif // YF_NEW_ALBUM_DLG_H
