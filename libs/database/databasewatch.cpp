/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Keeping image properties in sync.
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

#include "databasewatch.h"
#include "databasewatchadaptor.h"
#include "moc_databasewatch.cpp"
#include "moc_databasewatchadaptor.cpp"

// C Ansi includes

#include <unistd.h>

// Qt includes

#include <QMetaType>
#include <QtDBus>
#include <QThread>

// Local includes

#include "collectionmanager.h"
#include "config-digikam.h"

Digikam_DatabaseWatchAdaptor::Digikam_DatabaseWatchAdaptor(Digikam::DatabaseWatch* watch)
    : QDBusAbstractAdaptor(watch)
{
    setAutoRelaySignals(true);
}

namespace Digikam
{

class DBusSignalListenerThread;

class DatabaseWatchPriv
{
public:

    DatabaseWatchPriv() :
        mode(DatabaseWatch::DatabaseSlave),
        adaptor(0),
        slaveThread(0)
    {
    }

    DatabaseWatch::DatabaseMode   mode;
    QString                       databaseId;
    QString                       applicationId;

    Digikam_DatabaseWatchAdaptor* adaptor;

    DBusSignalListenerThread*     slaveThread;

    void connectWithDBus(const char* dbusSignal, QObject* obj, const char* slot,
                         QDBusConnection connection = QDBusConnection::sessionBus())
    {
        // connect to slave signals
        connection.connect(QString(), "/ChangesetRelay",
                           "org.kde.digikam.DatabaseChangesetRelay",
                           dbusSignal,
                           obj, slot);
        // connect to master signals
        connection.connect(QString(), "/ChangesetRelayForPeers",
                           "org.kde.digikam.DatabaseChangesetRelay",
                           dbusSignal,
                           obj, slot);
    }
};

class DBusSignalListenerThread : public QThread
{
    Q_OBJECT

public:

    DBusSignalListenerThread(DatabaseWatch* q, DatabaseWatchPriv* d)
        : q(q), d(d)
    {
        start();
    }

    ~DBusSignalListenerThread()
    {
        quit();
        wait();
    }

    virtual void run()
    {
        // We cannot use sessionBus() here but need to connect on our own
        QDBusConnection threadConnection =
            QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString("DigikamDatabaseSlaveConnection-%1").arg(getpid()));

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

private:

    DatabaseWatch*     q;
    DatabaseWatchPriv* d;
};

#include "databasewatch.moc"

// ---------------------------------------------------------------------------------

DatabaseWatch::DatabaseWatch()
    : d(new DatabaseWatchPriv)
{
}

DatabaseWatch::~DatabaseWatch()
{
    delete d->adaptor;
    delete d->slaveThread;
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
        d->adaptor = new Digikam_DatabaseWatchAdaptor(this);
        QDBusConnection::sessionBus().registerObject("/ChangesetRelay", this);

        // KIOSlave do not have an event loop which is needed for receiving DBus signals.
        // See also the event loop in DatabaseAccess::setParameters.
        d->slaveThread = new DBusSignalListenerThread(this, d);
    }
    else
    {
        d->adaptor = new Digikam_DatabaseWatchAdaptor(this);
        QDBusConnection::sessionBus().registerObject("/ChangesetRelayForPeers", this);

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

void DatabaseWatch::doAnyProcessing()
{
    // In a slave we have no event loop.
    // This method is called when a slave begins a new operation
    // (it calls DatabaseAccess::setParameters then).
    // Allow here queued signals to proceed that may be caused by DatabaseWatch signals
    // that were send from within the DBus listener thread (see above).
    QEventLoop loop;
    loop.processEvents();
}

void DatabaseWatch::setDatabaseIdentifier(const QString& identifier)
{
    d->databaseId = identifier;
}

void DatabaseWatch::setApplicationIdentifier(const QString& identifier)
{
    d->applicationId = identifier;
}

void DatabaseWatch::sendDatabaseChanged()
{
    // Note: This is not dispatched by DBus!
    emit databaseChanged();
}

// --- methods to dispatch changes from database to listeners (local and remote) ---

void DatabaseWatch::sendImageChange(const ImageChangeset& cset)
{
    // send local signal
    emit imageChange(cset);
    // send DBUS signal
    emit imageChange(d->databaseId, d->applicationId, cset);
}

void DatabaseWatch::sendImageTagChange(const ImageTagChangeset& cset)
{
    emit imageTagChange(cset);
    emit imageTagChange(d->databaseId, d->applicationId, cset);
}

void DatabaseWatch::sendCollectionImageChange(const CollectionImageChangeset& cset)
{
    emit collectionImageChange(cset);
    emit collectionImageChange(d->databaseId, d->applicationId, cset);
}

void DatabaseWatch::sendAlbumChange(const AlbumChangeset& cset)
{
    emit albumChange(cset);
    emit albumChange(d->databaseId, d->applicationId, cset);
}

void DatabaseWatch::sendTagChange(const TagChangeset& cset)
{
    emit tagChange(cset);
    emit tagChange(d->databaseId, d->applicationId, cset);
}

void DatabaseWatch::sendAlbumRootChange(const AlbumRootChangeset& cset)
{
    emit albumRootChange(cset);
    emit albumRootChange(d->databaseId, d->applicationId, cset);
}

void DatabaseWatch::sendSearchChange(const SearchChangeset& cset)
{
    emit searchChange(cset);
    emit searchChange(d->databaseId, d->applicationId, cset);
}


// --- methods to dispatch from slave or peer to local listeners ---

void DatabaseWatch::slotImageChangeDBus(const QString& databaseIdentifier,
                                        const QString& applicationIdentifier,
                                        const ImageChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit imageChange(changeset);
    }
}

void DatabaseWatch::slotImageTagChangeDBus(const QString& databaseIdentifier,
        const QString& applicationIdentifier,
        const ImageTagChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit imageTagChange(changeset);
    }
}

void DatabaseWatch::slotCollectionImageChangeDBus(const QString& databaseIdentifier,
        const QString& applicationIdentifier,
        const CollectionImageChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit collectionImageChange(changeset);
    }
}

void DatabaseWatch::slotAlbumChangeDBus(const QString& databaseIdentifier,
                                        const QString& applicationIdentifier,
                                        const AlbumChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit albumChange(changeset);
    }
}

void DatabaseWatch::slotTagChangeDBus(const QString& databaseIdentifier,
                                      const QString& applicationIdentifier,
                                      const TagChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit tagChange(changeset);
    }
}

void DatabaseWatch::slotAlbumRootChangeDBus(const QString& databaseIdentifier,
        const QString& applicationIdentifier,
        const AlbumRootChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit albumRootChange(changeset);
    }
}

void DatabaseWatch::slotSearchChangeDBus(const QString& databaseIdentifier,
        const QString& applicationIdentifier,
        const SearchChangeset& changeset)
{
    if (applicationIdentifier != d->applicationId &&
        databaseIdentifier == d->databaseId)
    {
        emit searchChange(changeset);
    }
}

} // namespace Digikam
