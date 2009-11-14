/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPDATABASE_H
#define SETUPDATABASE_H

// Qt includes

#include <QScrollArea>

class QString;

class KPageDialog;
class KUrl;

namespace Digikam
{

class SetupDatabasePriv;

class SetupDatabase : public QScrollArea
{
    Q_OBJECT

public:

    SetupDatabase(KPageDialog* dialog, QWidget* parent=0);
    ~SetupDatabase();

    void applySettings();

private:

    void readSettings();
    void checkDBPath();

private Q_SLOTS:

    void slotChangeDatabasePath(const KUrl&);
    void slotDatabasePathEdited(const QString&);
    void setDatabaseInputFields(const QString&);
    void checkDatabaseConnection();

private:

    SetupDatabasePriv* const d;
};

}  // namespace Digikam

#endif // SETUPDATABASE_H
