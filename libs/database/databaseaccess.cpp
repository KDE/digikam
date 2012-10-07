/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database access wrapper.
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

#include "databaseaccess.h"

// Qt includes

#include <QEventLoop>
#include <QMutex>
#include <QSqlDatabase>
#include <QUuid>

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "collectionscannerobserver.h"
#include "imageinfodata.h"
#include "imageinfocache.h"
#include "schemaupdater.h"
#include "collectionmanager.h"
#include "databasewatch.h"
#include "databasebackend.h"
#include "databaseerrorhandler.h"
#include "tagscache.h"

namespace Digikam
{

class DatabaseAccessStaticPriv
{
public:

    DatabaseAccessStaticPriv()
        : backend(0), db(0), databaseWatch(0),
          initializing(false)
    {
        // create a unique identifier for this application (as an application accessing a database
        applicationIdentifier = QUuid::createUuid();
    };
    ~DatabaseAccessStaticPriv() {};

    DatabaseBackend*    backend;
    AlbumDB*            db;
    DatabaseWatch*      databaseWatch;
    DatabaseParameters  parameters;
    DatabaseLocking     lock;
    QString             lastError;
    QUuid               applicationIdentifier;

    bool                initializing;
};

class DatabaseAccessMutexLocker : public QMutexLocker
{
public:

    explicit DatabaseAccessMutexLocker(DatabaseAccessStaticPriv* d)
        : QMutexLocker(&d->lock.mutex), d(d)
    {
        d->lock.lockCount++;
    }

    ~DatabaseAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

    DatabaseAccessStaticPriv* const d;
};

DatabaseAccessStaticPriv* DatabaseAccess::d = 0;

DatabaseAccess::DatabaseAccess()
{
    Q_ASSERT(d/*You will want to call setParameters before constructing DatabaseAccess*/);
    d->lock.mutex.lock();
    d->lock.lockCount++;

    if (!d->backend->isOpen() && !d->initializing)
    {
        // avoid endless loops (e.g. recursing from CollectionManager)
        d->initializing = true;

        d->backend->open(d->parameters);
        d->databaseWatch->setDatabaseIdentifier(d->db->databaseUuid());
        CollectionManager::instance()->refresh();

        d->initializing = false;
    }
}

DatabaseAccess::~DatabaseAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

DatabaseAccess::DatabaseAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->lock.mutex.lock();
    d->lock.lockCount++;
}

AlbumDB* DatabaseAccess::db() const
{
    return d->db;
}

DatabaseBackend* DatabaseAccess::backend() const
{
    return d->backend;
}

DatabaseWatch* DatabaseAccess::databaseWatch()
{
    if (d)
    {
        return d->databaseWatch;
    }

    return 0;
}

void DatabaseAccess::initDatabaseErrorHandler(DatabaseErrorHandler* errorhandler)
{
    if (!d || !d->backend)
    {
        kError() << "Please set parameters before setting a database error handler";
        return;
    }

    d->backend->setDatabaseErrorHandler(errorhandler);
}

DatabaseParameters DatabaseAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DatabaseParameters();
}

void DatabaseAccess::setParameters(const DatabaseParameters& parameters)
{
    //TODO 0.11: Refine API
    setParameters(parameters, DatabaseSlave);

    if (d->databaseWatch)
    {
        d->databaseWatch->doAnyProcessing();
    }
}

void DatabaseAccess::setParameters(const DatabaseParameters& parameters, ApplicationStatus status)
{
    if (!d)
    {
        d = new DatabaseAccessStaticPriv();
    }

    DatabaseAccessMutexLocker lock(d);

    if (d->parameters == parameters)
    {
        return;
    }

    if (d->backend && d->backend->isOpen())
    {
        d->backend->close();
    }

    // Kill the old database error handler
    if (d->backend)
    {
        d->backend->setDatabaseErrorHandler(0);
    }

    d->parameters = parameters;

    if (!d->databaseWatch)
    {
        d->databaseWatch = new DatabaseWatch();
        d->databaseWatch->setApplicationIdentifier(d->applicationIdentifier);

        if (status == MainApplication)
        {
            d->databaseWatch->initializeRemote(DatabaseWatch::DatabaseMaster);
        }
        else
        {
            d->databaseWatch->initializeRemote(DatabaseWatch::DatabaseSlave);
        }
    }

    ImageInfoStatic::create();

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = new DatabaseBackend(&d->lock);
        d->backend->setDatabaseWatch(d->databaseWatch);
        d->db = new AlbumDB(d->backend);
        TagsCache::instance()->initialize();
    }

    d->databaseWatch->sendDatabaseChanged();
    ImageInfoStatic::cache()->invalidate();
    TagsCache::instance()->invalidate();
    d->databaseWatch->setDatabaseIdentifier(QString());
    CollectionManager::instance()->clear_locked();
}

bool DatabaseAccess::checkReadyForUse(InitializationObserver* observer)
{
    QStringList drivers = QSqlDatabase::drivers();

    if (!drivers.contains("QSQLITE"))
    {
        kError() << "No SQLite3 driver available. List of QSqlDatabase drivers: " << drivers;
        d->lastError = i18n("The driver \"SQLITE\" for SQLite3 databases is not available.\n"
                            "digiKam depends on the drivers provided by the SQL module of Qt4.");
        return false;
    }

    if (!DatabaseConfigElement::checkReadyForUse())
    {
        d->lastError = DatabaseConfigElement::errorMessage();

        // Make sure the application does not continue to run
        if (observer)
        {
            observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }
        return false;
    }

    // create an object with private shortcut constructor
    DatabaseAccess access(false);

    if (!d->backend)
    {
        kWarning() << "No database backend available in checkReadyForUse. "
                   "Did you call setParameters before?";
        return false;
    }

    if (d->backend->isReady())
    {
        return true;
    }

    //TODO: Implement a method to wait until the database is open
    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
        {
            access.setLastError(i18n("Error opening database backend.\n ")
                                + d->backend->lastError());
            return false;
        }
    }

    // avoid endless loops (if called methods create new DatabaseAccess objects)
    d->initializing = true;

    // update schema
    SchemaUpdater updater(access.db(), access.backend(), access.parameters());
    updater.setDatabaseAccess(&access);

    updater.setObserver(observer);

    if (!d->backend->initSchema(&updater))
    {
        access.setLastError(updater.getLastErrorMessage());
        d->initializing = false;
        return false;
    }

    // set identifier again
    d->databaseWatch->setDatabaseIdentifier(d->db->databaseUuid());

    // initialize CollectionManager
    CollectionManager::instance()->refresh();

    d->initializing = false;

    return d->backend->isReady();
}

QString DatabaseAccess::lastError()
{
    return d->lastError;
}

void DatabaseAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void DatabaseAccess::cleanUpDatabase()
{
    if (d)
    {
        DatabaseAccessMutexLocker locker(d);
        d->backend->close();
        delete d->db;
        delete d->backend;
    }

    ImageInfoStatic::destroy();
    delete d;
    d = 0;
}

// --------

DatabaseAccessUnlock::DatabaseAccessUnlock()
{
    // acquire lock
    DatabaseAccess::d->lock.mutex.lock();
    // store lock count
    count = DatabaseAccess::d->lock.lockCount;
    // set lock count to 0
    DatabaseAccess::d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        DatabaseAccess::d->lock.mutex.unlock();
    }

    // drop lock acquired in first line. Mutex is now free.
    DatabaseAccess::d->lock.mutex.unlock();
}

DatabaseAccessUnlock::DatabaseAccessUnlock(DatabaseAccess*)
{
    // With the passed pointer, we have assured that the mutex is acquired
    // Store lock count
    count = DatabaseAccess::d->lock.lockCount;
    // set lock count to 0
    DatabaseAccess::d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        DatabaseAccess::d->lock.mutex.unlock();
    }

    // Mutex is now free
}

DatabaseAccessUnlock::~DatabaseAccessUnlock()
{
    // lock as often as it was locked before
    for (int i=0; i<count; ++i)
    {
        DatabaseAccess::d->lock.mutex.lock();
    }

    // update lock count
    DatabaseAccess::d->lock.lockCount += count;
}

}  // namespace Digikam
