/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-23
 * Description : Core database DBus interface description
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COREDATABASEWATCHADAPTOR_H
#define COREDATABASEWATCHADAPTOR_H

// Qt includes

#include <QObject>
#include <QDBusVariant>
#include <QDBusAbstractAdaptor>

// Local includes

#include "coredbwatch.h"

// qdbuscpp2xml -S -M databasewatchadaptor.h -o org.kde.digikam.DatabaseChangesetRelay.xml

class CoreDbWatchAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.digikam.DatabaseChangesetRelay")

public:

    explicit CoreDbWatchAdaptor(Digikam::CoreDbWatch* const watch);

Q_SIGNALS:

    // These signals are the same as declared in CoreDbWatch, setAutoRelaySignals will
    // automatically connect the CoreDbWatch signals to these, which are then sent over DBus.

    //NOTE:
    // The full qualification with "Digikam::" for the changeset types in the following
    // signals and slots are required to make moc pick them up.
    // If moc does not get the namespace in its literal, DBus connections will silently break.

    void imageChange(const QString& databaseIdentifier,
                     const QString& applicationIdentifier,
                     const Digikam::ImageChangeset& changeset);

    void imageTagChange(const QString& databaseIdentifier,
                        const QString& applicationIdentifier,
                        const Digikam::ImageTagChangeset& changeset);

    void collectionImageChange(const QString& databaseIdentifier,
                               const QString& applicationIdentifier,
                               const Digikam::CollectionImageChangeset& changeset);

    void albumChange(const QString& databaseIdentifier,
                     const QString& applicationIdentifier,
                     const Digikam::AlbumChangeset& changeset);

    void tagChange(const QString& databaseIdentifier,
                   const QString& applicationIdentifier,
                   const Digikam::TagChangeset& changeset);

    void albumRootChange(const QString& databaseIdentifier,
                         const QString& applicationIdentifier,
                         const Digikam::AlbumRootChangeset& changeset);

    void searchChange(const QString& databaseIdentifier,
                      const QString& applicationIdentifier,
                      const Digikam::SearchChangeset& changeset);
};

#endif // COREDATABASEWATCHADAPTOR_H
