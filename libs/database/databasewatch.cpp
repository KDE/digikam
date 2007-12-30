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

// Qt includes

#include <QMetaType>
#include <QtDBus>
#include <QThread>

// Local includes

#include "ddebug.h"
#include "databasewatch.h"
#include "databasewatchadaptor.h"

#include "databasewatchadaptor.moc"
#include "databasewatch.moc"

namespace Digikam
{

DatabaseWatchAdaptor::DatabaseWatchAdaptor(DatabaseWatch *watch)
    : QDBusAbstractAdaptor(watch)
{
    setAutoRelaySignals(true);
}

// ------------------- //

class DatabaseWatchPriv
{
public:
    DatabaseWatchPriv()
    {
        mode = DatabaseWatch::DatabaseSlave;
    }

    DatabaseWatch::DatabaseMode mode;
    QString                     databaseId;
    QString                     applicationId;
};

// ------------------- //

DatabaseWatch::DatabaseWatch()
{
    d = new DatabaseWatchPriv;
}

DatabaseWatch::~DatabaseWatch()
{
    delete d;
}

void DatabaseWatch::initializeRemote(DatabaseMode mode)
{
    d->mode = mode;

    qRegisterMetaType<ImageChangeset>("ImageChangeset");
    qRegisterMetaType<ImageTagChangeset>("ImageTagChangeset");
    qRegisterMetaType<CollectionImageChangeset>("CollectionImageChangeset");
    qRegisterMetaType<AlbumChangeset>("AlbumChangeset");
    qRegisterMetaType<TagChangeset>("TagChangeset");
    qRegisterMetaType<AlbumRootChangeset>("AlbumRootChangeset");
    qRegisterMetaType<SearchChangeset>("SearchChangeset");

    qDBusRegisterMetaType<ImageChangeset>();
    qDBusRegisterMetaType<ImageTagChangeset>();
    qDBusRegisterMetaType<CollectionImageChangeset>();
    qDBusRegisterMetaType<AlbumChangeset>();
    qDBusRegisterMetaType<TagChangeset>();
    qDBusRegisterMetaType<AlbumRootChangeset>();
    qDBusRegisterMetaType<SearchChangeset>();

    qDBusRegisterMetaType<DatabaseFields::Set>();
    qDBusRegisterMetaType< QList<qlonglong> >();
    qDBusRegisterMetaType< QList<int> >();

    if (d->mode == DatabaseSlave)
    {
        new DatabaseWatchAdaptor(this);
        QDBusConnection::sessionBus().registerObject("/ChangesetRelay", this);
    }
    else
    {
        new DatabaseWatchAdaptor(this);
        QDBusConnection::sessionBus().registerObject("/ChangesetRelayForPeers", this);

        // connect to slave signals
        QDBusConnection::sessionBus().connect(QString(), "/ChangesetRelay",
                                              "org.digikam.DatabaseChangesetRelay",
                                              "changeset",
                                              this, SLOT(slotDBusChangeset(const QString &,
                                                                             const QString &,
                                                                             const QDBusVariant &)));

        // connect to master signals
        QDBusConnection::sessionBus().connect(QString(), "/ChangesetRelayForPeers",
                                              "org.digikam.DatabaseChangesetRelay",
                                              "changeset",
                                              this, SLOT(slotDBusChangeset(const QString &,
                                                                             const QString &,
                                                                             const QDBusVariant &)));

    }
}

void DatabaseWatch::setDatabaseIdentifier(const QString &identifier)
{
    d->databaseId = identifier;
}

void DatabaseWatch::setApplicationIdentifier(const QString &identifier)
{
    d->applicationId = identifier;
}

// --- methods to dispatch from slave or peer to local listeners ---

void DatabaseWatch::slotDBusChangeset(const QString &databaseIdentifier,
                                      const QString &applicationIdentifier,
                                      const QDBusVariant &v)
{
    // We only want to process messages that 1) were not sent by ourselves, 2) are sent for our database
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        QVariant var = v.variant();
        if (var.canConvert<ImageChangeset>())
        {
            emit imageChange(var.value<ImageChangeset>());
        }
        else if (var.canConvert<ImageTagChangeset>())
        {
            emit imageTagChange(var.value<ImageTagChangeset>());
        }
        else if (var.canConvert<CollectionImageChangeset>())
        {
            emit collectionImageChange(var.value<CollectionImageChangeset>());
        }
        else if (var.canConvert<AlbumChangeset>())
        {
            emit albumChange(var.value<AlbumChangeset>());
        }
        else if (var.canConvert<TagChangeset>())
        {
            emit tagChange(var.value<TagChangeset>());
        }
        else if (var.canConvert<AlbumRootChangeset>())
        {
            emit albumRootChange(var.value<AlbumRootChangeset>());
        }
        else if (var.canConvert<SearchChangeset>())
        {
            emit searchChange(var.value<SearchChangeset>());
        }
    }
}

// --- methods to dispatch changes from database to listeners (local and remote) ---

void DatabaseWatch::sendImageChange(ImageChangeset cset)
{
    // send local signal
    emit imageChange(cset);
    // send DBUS signal
    emit changeset(d->databaseId, d->applicationId,
                   QDBusVariant(QVariant::fromValue<ImageChangeset>(cset)));
}

void DatabaseWatch::sendImageTagChange(ImageTagChangeset cset)
{
    emit imageTagChange(cset);
    emit changeset(d->databaseId, d->applicationId,
                   QDBusVariant(QVariant::fromValue<ImageTagChangeset>(cset)));
}

void DatabaseWatch::sendCollectionImageChange(CollectionImageChangeset cset)
{
    emit collectionImageChange(cset);
    emit changeset(d->databaseId, d->applicationId,
                     QDBusVariant(QVariant::fromValue<CollectionImageChangeset>(cset)));
}

void DatabaseWatch::sendAlbumChange(AlbumChangeset cset)
{
    emit albumChange(cset);
    emit changeset(d->databaseId, d->applicationId,
                     QDBusVariant(QVariant::fromValue<AlbumChangeset>(cset)));
}

void DatabaseWatch::sendTagChange(TagChangeset cset)
{
    emit tagChange(cset);
    emit changeset(d->databaseId, d->applicationId,
                     QDBusVariant(QVariant::fromValue<TagChangeset>(cset)));
}

void DatabaseWatch::sendAlbumRootChange(AlbumRootChangeset cset)
{
    emit albumRootChange(cset);
    emit changeset(d->databaseId, d->applicationId,
                     QDBusVariant(QVariant::fromValue<AlbumRootChangeset>(cset)));
}

void DatabaseWatch::sendSearchChange(SearchChangeset cset)
{
    emit searchChange(cset);
    emit changeset(d->databaseId, d->applicationId,
                     QDBusVariant(QVariant::fromValue<SearchChangeset>(cset)));
}




} // namespace Digikam
