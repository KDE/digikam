/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export images to VKontakte web service
 *
 * Copyright (C) 2011      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2011-2015 by Alexander Potashev <aspotashev at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef VK_NEW_ALBUM_DLG_H
#define VK_NEW_ALBUM_DLG_H

// Qt includes

#include <QDialog>
#include <QWidget>
#include <QString>

namespace Digikam
{

class VKNewAlbumDlg : public QDialog
{
    Q_OBJECT

public:

    struct AlbumProperties
    {
        QString title;
        QString description;
        int     privacy;
        int     commentPrivacy;
    };

public:

    /**
     * @brief Album creation dialog
     *
     * @param parent Parent widget
     */
    VKNewAlbumDlg(QWidget* const parent);

    /**
     * @brief Album editing dialog
     *
     * @param parent Parent widget
     * @param album Initial album properties
     */
    VKNewAlbumDlg(QWidget* const parent, const AlbumProperties& album);

    ~VKNewAlbumDlg();

    const AlbumProperties& album() const;

private Q_SLOTS:

    void accept() Q_DECL_OVERRIDE;

private:

    void initDialog(bool editing);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // VK_NEW_ALBUM_DLG_H
