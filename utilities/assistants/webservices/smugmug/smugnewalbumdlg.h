/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-07
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SMUG_NEW_ALBUM_DLG_H
#define SMUG_NEW_ALBUM_DLG_H

// Qt includes

#include <QDialog>
#include <QGroupBox>
#include <QComboBox>

namespace Digikam
{

class SmugAlbum;

class SmugNewAlbumDlg : public QDialog
{
    Q_OBJECT

public:

    explicit SmugNewAlbumDlg(QWidget* const parent);
    ~SmugNewAlbumDlg();

    void getAlbumProperties(SmugAlbum& album);

    QComboBox* categoryCombo()    const;
    QComboBox* subCategoryCombo() const;
    QComboBox* templateCombo()    const;
    QGroupBox* privateGroupBox()  const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SMUG_NEW_ALBUM_DLG_H
