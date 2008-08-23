/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Keeping image properties in sync.
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

#ifndef DATABASEWATCH_H
#define DATABASEWATCH_H

// Qt includes.

#include <QObject>

// KDE includes.

// Local includes.

#include "digikam_export.h"
#include "databasechangesets.h"

namespace Digikam
{

class DatabaseWatchPriv;

class DIGIKAM_EXPORT DatabaseWatch : public QObject
{

    Q_OBJECT

signals:

    /** Retrieve the DatabaseWatch object from DatabaseAccess::databaseWatch(). */

    /**
     * Notifies of changes in the database.
     * Connect to the set of signals that you are interested in.
     */
    void imageChange(const ImageChangeset &changeset);
    void imageTagChange(const ImageTagChangeset &changeset);
    void collectionImageChange(const CollectionImageChangeset &changeset);
    void albumChange(const AlbumChangeset &changeset);
    void tagChange(const TagChangeset &changeset);
    void albumRootChange(const AlbumRootChangeset &changeset);
    void searchChange(const SearchChangeset &changeset);

    // --------------- //

protected:

    ~DatabaseWatch();

protected slots:

    // DBus slots, for internal use
    void slotDBusChangeset(const QString &databaseIdentifier,
                           const QString &applicationIdentifier,
                           const QDBusVariant &changeset);

signals:

    // DBus signals, for internal use
    void changeset(const QString &databaseIdentifier,
                   const QString &applicationIdentifier,
                   const QDBusVariant &changeset);

public:

    // library-internal initialization API

    DatabaseWatch();

    enum DatabaseMode
    {
        DatabaseMaster,
        DatabaseSlave
    };

    void initializeRemote(DatabaseMode mode);
    void setDatabaseIdentifier(const QString &identifier);
    void setApplicationIdentifier(const QString &identifier);

    // library-internal signal-trigger methods

    void sendImageChange(const ImageChangeset &changeset);
    void sendImageTagChange(const ImageTagChangeset &changeset);
    void sendCollectionImageChange(const CollectionImageChangeset &changeset);
    void sendAlbumChange(const AlbumChangeset &changeset);
    void sendTagChange(const TagChangeset &changeset);
    void sendAlbumRootChange(const AlbumRootChangeset &changeset);
    void sendSearchChange(const SearchChangeset &changeset);

private:

    DatabaseWatchPriv *d;
};

} // namespace Digikam

#endif // DATABASEATTRIBUTESWATCH_H
