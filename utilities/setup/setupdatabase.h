/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class SetupDatabase : public QScrollArea
{
    Q_OBJECT

public:
    enum AlbumTab
    {
        DbSettings = 0,
        IgnoreDirs
    };

public:

    explicit SetupDatabase(QWidget* const parent = 0);
    virtual ~SetupDatabase();

    void applySettings();

private:

    void readSettings();
    void createUpdateBox();

private Q_SLOTS:

    void upgradeUniqueHashes();
    void showHashInformation();
    void slotShowCurrentIgnoredDirectoriesSettings() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SETUPDATABASE_H
