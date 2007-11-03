/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : general configuration setup tab
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

#ifndef SETUPGENERAL_H
#define SETUPGENERAL_H

// Qt includes.

#include <QWidget>

class KPageDialog;

namespace Digikam
{

class SetupGeneralPriv;

class SetupGeneral : public QWidget
{
    Q_OBJECT

public:

    SetupGeneral(KPageDialog* dialog, QWidget* parent=0);
    ~SetupGeneral();

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

    SetupGeneralPriv* d;
};

}  // namespace Digikam

#endif // SETUPGENERAL_H
