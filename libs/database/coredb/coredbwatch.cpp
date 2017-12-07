/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Core database image properties synchronizer
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QMetaType>
#include <QApplication>

#ifdef HAVE_DBUS
#   include <QtDBus>
#   include "coredbwatchadaptor.h"
#endif

// Local includes

#include "collectionmanager.h"
#include "digikam_config.h"

namespace Digikam
{

#ifdef HAVE_DBUS
class DBusSignalListenerThread;
#endif

class CoreDbWatch::Private
{
public:

    Private() :
        mode(CoreDbWatch::DatabaseSlave)
#ifdef HAVE_DBUS
        ,adaptor(0)
        ,slaveThread(0)
#endif
    {
    }

#ifdef HAVE_DBUS
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
#endif

public:


    CoreDbWatch::DatabaseMode   mode;
    QString                     databaseId;
    QString                     applicationId;

#ifdef HAVE_DBUS
    CoreDbWatchAdaptor*         adaptor;
    DBusSignalListenerThread*   slaveThread;
#endif
};

// ---------------------------------------------------------------------------------

#ifdef HAVE_DBUS

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
    QDBusConnection threadConnection = QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                                                     QString::fromUtf8("DigikamDatabaseSlaveConnection-%1")
                                                                     .arg(qApp->applicationPid()));

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

#endif

// ---------------------------------------------------------------------------------

CoreDbWatch::CoreDbWatch()
    : d(new Private)
{
}

CoreDbWatch::~CoreDbWatch()
{
#ifdef HAVE_DBUS
    delete d->adaptor;
    delete d->slaveThread;
#endif
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

#ifdef HAVE_DBUS
    // NOTE: The literal for registration with DBus here will include namespace qualifier.
    // Therefore, the header file declaration for DBus signals and slots
    // must contain the full qualifier as well, so that moc picks them up.
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
#endif

    if (d->mode == DatabaseSlave)
    {
#ifdef HAVE_DBUS
        d->adaptor = new CoreDbWatchAdaptor(this);

        QDBusConnection::sessionBus().registerObject(QLatin1String("/ChangesetRelay"), this);

        // Slave do not have an event loop which is needed for receiving DBus signals.
        // See also the event loop in CoreDbAccess::setParameters.
        d->slaveThread = new DBusSignalListenerThread(this, d);
#endif
    }
    else
    {
#ifdef HAVE_DBUS
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
#endif
    }

    // Do this as a favor for CollectionManager, we may not exist at time of its creation
    connect(this, SIGNAL(albumRootChange(AlbumRootChangeset)),
            CollectionManager::instance(), SLOT(slotAlbumRootChange(AlbumRootChangeset)));
}

void CoreDbWatch::doAnyProcessing()
{
#ifdef HAVE_DBUS
    // In a slave we have no event loop.
    // This method is called when a slave begins a new operation
    // (it calls CoreDbAccess::setParameters then).
    // Allow here queued signals to proceed that may be caused by CoreDbWatch signals
    // that were send from within the DBus listener thread (see above).
    QEventLoop loop;
    loop.processEvents();
#endif
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
#ifdef HAVE_DBUS
    // Note: This is not dispatched by DBus!
#endif

    emit databaseChanged();
}

// --- methods to dispatch changes from database to listeners (local and remote) ---

void CoreDbWatch::sendImageChange(const ImageChangeset& cset)
{
    // send local signal
    emit imageChange(cset);
#ifdef HAVE_DBUS
    // send DBUS signal
    emit imageChange(d->databaseId, d->applicationId, cset);
#endif
}

void CoreDbWatch::sendImageTagChange(const ImageTagChangeset& cset)
{
    emit imageTagChange(cset);
#ifdef HAVE_DBUS
    emit imageTagChange(d->databaseId, d->applicationId, cset);
#endif
}

void CoreDbWatch::sendCollectionImageChange(const CollectionImageChangeset& cset)
{
    emit collectionImageChange(cset);
#ifdef HAVE_DBUS
    emit collectionImageChange(d->databaseId, d->applicationId, cset);
#endif
}

void CoreDbWatch::sendAlbumChange(const AlbumChangeset& cset)
{
    emit albumChange(cset);
#ifdef HAVE_DBUS
    emit albumChange(d->databaseId, d->applicationId, cset);
#endif
}

void CoreDbWatch::sendTagChange(const TagChangeset& cset)
{
    emit tagChange(cset);
#ifdef HAVE_DBUS
    emit tagChange(d->databaseId, d->applicationId, cset);
#endif
}

void CoreDbWatch::sendAlbumRootChange(const AlbumRootChangeset& cset)
{
    emit albumRootChange(cset);
#ifdef HAVE_DBUS
    emit albumRootChange(d->databaseId, d->applicationId, cset);
#endif
}

void CoreDbWatch::sendSearchChange(const SearchChangeset& cset)
{
    emit searchChange(cset);
#ifdef HAVE_DBUS
    emit searchChange(d->databaseId, d->applicationId, cset);
#endif
}

#ifdef HAVE_DBUS

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

#endif

} // namespace Digikam
