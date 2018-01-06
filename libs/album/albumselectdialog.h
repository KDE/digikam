/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-16
 * Description : a dialog to select a target album to download
 *               pictures from camera
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMSELECTDIALOG_H
#define ALBUMSELECTDIALOG_H

// Qt includes

#include <QString>
#include <QDialog>

// Local includes

#include "searchtextbar.h"

namespace Digikam
{

class PAlbum;

class AlbumSelectDialog : public QDialog
{
    Q_OBJECT

public:

    AlbumSelectDialog(QWidget* const parent, PAlbum* const albumToSelect, const QString& header=QString());
    ~AlbumSelectDialog();

    static PAlbum* selectAlbum(QWidget* const parent, PAlbum* const albumToSelect, const QString& header=QString());

private Q_SLOTS:

    void slotSelectionChanged();
    void slotHelp();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ALBUMSELECTDIALOG_H */
