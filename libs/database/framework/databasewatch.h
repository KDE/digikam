/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Keeping image properties in sync.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "databasechangesets.h"

namespace Digikam
{

class DatabaseWatchPriv;

class DIGIKAM_DATABASE_EXPORT DatabaseWatch : public QObject
{

    Q_OBJECT

Q_SIGNALS:

    /** Retrieve the DatabaseWatch object from DatabaseAccess::databaseWatch(). */

    /**
     * This does not describe a change of the contents of a table;
     * rather, it signals that a new database has been loaded.
     * That means all cached content has to be discarded.
     */
    void databaseChanged();

    /**
     * Notifies of changes in the database.
     * Connect to the set of signals that you are interested in.
     */
    void imageChange(const ImageChangeset& changeset);
    void imageTagChange(const ImageTagChangeset& changeset);
    void collectionImageChange(const CollectionImageChangeset& changeset);
    void albumChange(const AlbumChangeset& changeset);
    void tagChange(const TagChangeset& changeset);
    void albumRootChange(const AlbumRootChangeset& changeset);
    void searchChange(const SearchChangeset& changeset);

protected:

    ~DatabaseWatch();

protected Q_SLOTS:

    //NOTE:
    // The full qualification with "Digikam::" for the changeset types in the following
    // signals and slots are required to make moc pick them up.
    // If moc does not get the namespace in its literal, DBus connections will silently break.

    // DBus slots, for internal use
    void slotImageChangeDBus(const QString& databaseIdentifier,
                             const QString& applicationIdentifier,
                             const Digikam::ImageChangeset& changeset);
    void slotImageTagChangeDBus(const QString& databaseIdentifier,
                                const QString& applicationIdentifier,
                                const Digikam::ImageTagChangeset& changeset);
    void slotCollectionImageChangeDBus(const QString& databaseIdentifier,
                                       const QString& applicationIdentifier,
                                       const Digikam::CollectionImageChangeset& changeset);
    void slotAlbumChangeDBus(const QString& databaseIdentifier,
                             const QString& applicationIdentifier,
                             const Digikam::AlbumChangeset& changeset);
    void slotTagChangeDBus(const QString& databaseIdentifier,
                           const QString& applicationIdentifier,
                           const Digikam::TagChangeset& changeset);
    void slotAlbumRootChangeDBus(const QString& databaseIdentifier,
                                 const QString& applicationIdentifier,
                                 const Digikam::AlbumRootChangeset& changeset);
    void slotSearchChangeDBus(const QString& databaseIdentifier,
                              const QString& applicationIdentifier,
                              const Digikam::SearchChangeset& changeset);

Q_SIGNALS:

    // DBus signals, for internal use
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

public:

    // library-internal initialization API

    DatabaseWatch();

    enum DatabaseMode
    {
        DatabaseMaster,
        DatabaseSlave
    };

    void initializeRemote(DatabaseMode mode);
    void doAnyProcessing();
    void setDatabaseIdentifier(const QString& identifier);
    void setApplicationIdentifier(const QString& identifier);

    // library-internal signal-trigger methods

    void sendDatabaseChanged();

    void sendImageChange(const ImageChangeset& changeset);
    void sendImageTagChange(const ImageTagChangeset& changeset);
    void sendCollectionImageChange(const CollectionImageChangeset& changeset);
    void sendAlbumChange(const AlbumChangeset& changeset);
    void sendTagChange(const TagChangeset& changeset);
    void sendAlbumRootChange(const AlbumRootChangeset& changeset);
    void sendSearchChange(const SearchChangeset& changeset);

private:

    DatabaseWatchPriv* const d;
};

} // namespace Digikam

#endif // DATABASEATTRIBUTESWATCH_H
