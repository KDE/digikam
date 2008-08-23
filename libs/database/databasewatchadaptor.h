/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-23
 * Description : DBus interface description
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEWATCHADAPTOR_H
#define DATABASEWATCHADAPTOR_H

// Qt includes.

#include <QObject>
#include <QDBusVariant>

namespace Digikam
{

// qdbuscpp2xml -S -M databasewatchadaptor.h -o org.digikam.DatabaseChangesetRelay.xml

class DatabaseWatchAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.digikam.DatabaseChangesetRelay")

public:

    DatabaseWatchAdaptor(DatabaseWatch *watch);

signals:

    void changeset(const QString &databaseIdentifier,
                   const QString &applicationIdentifier,
                   const QDBusVariant &changeset);
};

}

#endif


