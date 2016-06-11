/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Core database image properties synchronizer
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "coredbwatch.h"
#include "coredbwatchadaptor.h"

// C Ansi includes

#include <unistd.h>

// Qt includes

#include <QMetaType>
#include <QtDBus>

// Local includes

#include "collectionmanager.h"
#include "coredbwatchadaptor.h"
#include "digikam_config.h"

namespace Digikam
{

class DBusSignalListenerThread;

class CoreDbWatch::Private
{
public:

    Private() :
        mode(CoreDbWatch::DatabaseSlave),
        adaptor(0),
        slaveThread(0)
    {
    }

    void connectWithDBus(const char* dbusSignal, QObject* obj, const char* slot,
                         QDBusConnection connection = QDBusConnection::sessionBus())
    {
        // connect to slave signals
        connection.connect(QString(), QLatin1String("/ChangesetRelay"),
                           QLatin1String("org.kde.digikam.DatabaseChangesetRelay"),
                           QString::fromUtf8(dbusSignal),
                           obj, slot);
        // connect to master signals
        connection.connect(QString(), QLatin1String("/ChangesetRelayForPeers"),
                           QLatin1String("org.kde.digikam.DatabaseChangesetRelay"),
                           QString::fromUtf8(dbusSignal),
                           obj, slot);
    }

public:


    CoreDbWatch::DatabaseMode   mode;
    QString                     databaseId;
    QString                     applicationId;

    CoreDbWatchAdaptor* adaptor;

    DBusSignalListenerThread*   slaveThread;
};

// ---------------------------------------------------------------------------------

DBusSignalListenerThread::DBusSignalListenerThread(CoreDbWatch* const q, CoreDbWatch::Private* const d)
    : q(q),
      d(d)
{
    start();
}

DBusSignalListenerThread::~DBusSignalListenerThread()
{
    quit();
    wait();
}

void DBusSignalListenerThread::run()
{
    // We cannot use sessionBus() here but need to connect on our own
    QDBusConnection threadConnection = QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString::fromUtf8("DigikamDatabaseSlaveConnection-%1").arg(getpid()));

    // DBus signals are received from within this thread and then sent with queued signals to the main thread
    d->connectWithDBus("imageTagChange", q,
                        SLOT(slotImageTagChangeDBus(QString,QString,Digikam::ImageTagChangeset)),
                        threadConnection);
    d->connectWithDBus("albumRootChange", q,
                        SLOT(slotAlbumRootChangeDBus(QString,QString,Digikam::AlbumRootChangeset)),
                        threadConnection);

    // enter thread event loop
    exec();
}

// ---------------------------------------------------------------------------------

CoreDbWatch::CoreDbWatch()
    : d(new Private)
{
}

CoreDbWatch::~CoreDbWatch()
{
    delete d->adaptor;
    delete d->slaveThread;
    delete d;
}

void CoreDbWatch::initializeRemote(DatabaseMode mode)
{
    d->mode = mode;

    qRegisterMetaType<ImageChangeset>("ImageChangeset");
    qRegisterMetaType<ImageTagChangeset>("ImageTagChangeset");
    qRegisterMetaType<CollectionImageChangeset>("CollectionImageChangeset");
    qRegisterMetaType<AlbumChangeset>("AlbumChangeset");
    qRegisterMetaType<TagChangeset>("TagChangeset");
    qRegisterMetaType<AlbumRootChangeset>("AlbumRootChangeset");
    qRegisterMetaType<SearchChangeset>("SearchChangeset");

    //NOTE: The literal for registration with DBus here will include namespace qualifier.
    //      Therefore, the header file declaration for DBus signals and slots
    //      must contain the full qualifier as well, so that moc picks them up.
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
        d->adaptor = new CoreDbWatchAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QLatin1String("/ChangesetRelay"), this);

        // KIOSlave do not have an event loop which is needed for receiving DBus signals.
        // See also the event loop in CoreDbAccess::setParameters.
        d->slaveThread = new DBusSignalListenerThread(this, d);
    }
    else
    {
        d->adaptor = new CoreDbWatchAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QLatin1String("/ChangesetRelayForPeers"), this);

        // connect DBus signals from slave or peer to our application
        d->connectWithDBus("imageChange", this,
                           SLOT(slotImageChangeDBus(QString,QString,Digikam::ImageChangeset)));
        d->connectWithDBus("imageTagChange", this,
                           SLOT(slotImageTagChangeDBus(QString,QString,Digikam::ImageTagChangeset)));
        d->connectWithDBus("collectionImageChange", this,
                           SLOT(slotCollectionImageChangeDBus(QString,QString,Digikam::CollectionImageChangeset)));
        d->connectWithDBus("albumChange", this,
                           SLOT(slotAlbumChangeDBus(QString,QString,Digikam::AlbumChangeset)));
        d->connectWithDBus("tagChange", this,
                           SLOT(slotTagChangeDBus(QString,QString,Digikam::TagChangeset)));
        d->connectWithDBus("albumRootChange", this,
                           SLOT(slotAlbumRootChangeDBus(QString,QString,Digikam::AlbumRootChangeset)));
        d->connectWithDBus("searchChange", this,
                           SLOT(slotSearchChangeDBus(QString,QString,Digikam::SearchChangeset)));
    }

    // Do this as a favor for CollectionManager, we may not exist at time of its creation
    connect(this, SIGNAL(albumRootChange(AlbumRootChangeset)),
            CollectionManager::instance(), SLOT(slotAlbumRootChange(AlbumRootChangeset)));
}

void CoreDbWatch::doAnyProcessing()
{
    // In a slave we have no event loop.
    // This method is called when a slave begins a new operation
    // (it calls CoreDbAccess::setParameters then).
    // Allow here queued signals to proceed that may be caused by CoreDbWatch signals
    // that were send from within the DBus listener thread (see above).
    QEventLoop loop;
    loop.processEvents();
}

void CoreDbWatch::setDatabaseIdentifier(const QString& identifier)
{
    d->databaseId = identifier;
}

void CoreDbWatch::setApplicationIdentifier(const QString& identifier)
{
    d->applicationId = identifier;
}

void CoreDbWatch::sendDatabaseChanged()
{
    // Note: This is not dispatched by DBus!
    emit databaseChanged();
}

// --- methods to dispatch changes from database to listeners (local and remote) ---

void CoreDbWatch::sendImageChange(const ImageChangeset& cset)
{
    // send local signal
    emit imageChange(cset);
    // send DBUS signal
    emit imageChange(d->databaseId, d->applicationId, cset);
}

void CoreDbWatch::sendImageTagChange(const ImageTagChangeset& cset)
{
    emit imageTagChange(cset);
    emit imageTagChange(d->databaseId, d->applicationId, cset);
}

void CoreDbWatch::sendCollectionImageChange(const CollectionImageChangeset& cset)
{
    emit collectionImageChange(cset);
    emit collectionImageChange(d->databaseId, d->applicationId, cset);
}

void CoreDbWatch::sendAlbumChange(const AlbumChangeset& cset)
{
    emit albumChange(cset);
    emit albumChange(d->databaseId, d->applicationId, cset);
}

void CoreDbWatch::sendTagChange(const TagChangeset& cset)
{
    emit tagChange(cset);
    emit tagChange(d->databaseId, d->applicationId, cset);
}

void CoreDbWatch::sendAlbumRootChange(const AlbumRootChangeset& cset)
{
    emit albumRootChange(cset);
    emit albumRootChange(d->databaseId, d->applicationId, cset);
}

void CoreDbWatch::sendSearchChange(const SearchChangeset& cset)
{
    emit searchChange(cset);
    emit searchChange(d->databaseId, d->applicationId, cset);
}

// --- methods to dispatch from slave or peer to local listeners ---

void CoreDbWatch::slotImageChangeDBus(const QString& databaseIdentifier,
                                        const QString& applicationIdentifier,
                                        const ImageChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit imageChange(changeset);
    }
}

void CoreDbWatch::slotImageTagChangeDBus(const QString& databaseIdentifier,
                                           const QString& applicationIdentifier,
                                           const ImageTagChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit imageTagChange(changeset);
    }
}

void CoreDbWatch::slotCollectionImageChangeDBus(const QString& databaseIdentifier,
                                                  const QString& applicationIdentifier,
                                                  const CollectionImageChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit collectionImageChange(changeset);
    }
}

void CoreDbWatch::slotAlbumChangeDBus(const QString& databaseIdentifier,
                                        const QString& applicationIdentifier,
                                        const AlbumChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit albumChange(changeset);
    }
}

void CoreDbWatch::slotTagChangeDBus(const QString& databaseIdentifier,
                                      const QString& applicationIdentifier,
                                      const TagChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit tagChange(changeset);
    }
}

void CoreDbWatch::slotAlbumRootChangeDBus(const QString& databaseIdentifier,
        const QString& applicationIdentifier,
        const AlbumRootChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit albumRootChange(changeset);
    }
}

void CoreDbWatch::slotSearchChangeDBus(const QString& databaseIdentifier,
                                         const QString& applicationIdentifier,
                                         const SearchChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier    == d->databaseId)
    {
        emit searchChange(changeset);
    }
}

} // namespace Digikam
