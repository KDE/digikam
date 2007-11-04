/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPALBUM_H
#define SETUPALBUM_H

// Qt includes.

#include <QWidget>

class KPageDialog;

namespace Digikam
{

class SetupAlbumPriv;

class SetupAlbum : public QWidget
{
    Q_OBJECT

public:

    SetupAlbum(KPageDialog* dialog, QWidget* parent=0);
    ~SetupAlbum();

    void applySettings();

private:

    void readSettings();
    void checkforOkButton();

private slots:

    void slotChangeAlbumPath(const KUrl&);
    void slotAlbumPathEdited(const QString&);

    void slotChangeDatabasePath(const KUrl&);
    void slotDatabasePathEdited(const QString&);

private:

    SetupAlbumPriv* d;
};

}  // namespace Digikam

#endif // SETUPALBUM_H
