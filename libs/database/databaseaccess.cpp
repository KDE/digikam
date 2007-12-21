/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database access wrapper.
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

#include <QMutex>
#include <QSqlDatabase>

// KDE includes

#include <klocale.h>

// Local includes

#include "ddebug.h"
#include "albumdb.h"
#include "imageinfocache.h"
#include "schemaupdater.h"
#include "collectionmanager.h"
#include "databasewatch.h"
#include "databasebackend.h"
#include "databaseaccess.h"

namespace Digikam
{

class DatabaseAccessStaticPriv
{
public:
    DatabaseAccessStaticPriv()
    : backend(0), db(0), infoCache(0), databaseWatch(0), mutex(QMutex::Recursive) // create a recursive mutex
    {
    };
    ~DatabaseAccessStaticPriv() {};

    DatabaseBackend *backend;
    AlbumDB *db;
    ImageInfoCache *infoCache;
    DatabaseWatch *databaseWatch;
    DatabaseParameters parameters;
    QMutex mutex;
    QString lastError;
};

DatabaseAccessStaticPriv *DatabaseAccess::d = 0;

DatabaseAccess::DatabaseAccess()
{
    d->mutex.lock();
    if (!d->backend->isOpen())
    {
        d->backend->open(d->parameters);
        CollectionManager::instance()->refresh();
    }
}

DatabaseAccess::~DatabaseAccess()
{
    d->mutex.unlock();
}

DatabaseAccess::DatabaseAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->mutex.lock();
}

AlbumDB *DatabaseAccess::db() const
{
    return d->db;
}

DatabaseBackend *DatabaseAccess::backend() const
{
    return d->backend;
}

ImageInfoCache *DatabaseAccess::imageInfoCache() const
{
    return d->infoCache;
}

DatabaseWatch *DatabaseAccess::databaseWatch()
{
    return d->databaseWatch;
}

DatabaseParameters DatabaseAccess::parameters()
{
    return d->parameters;
}

void DatabaseAccess::setParameters(const DatabaseParameters &parameters)
{
    if (!d)
    {
        d = new DatabaseAccessStaticPriv();
    }

    QMutexLocker lock(&d->mutex);

    if (d->parameters == parameters)
        return;

    if (d->backend && d->backend->isOpen())
        d->backend->close();

    d->parameters = parameters;

    if (!d->databaseWatch)
        d->databaseWatch = new DatabaseWatch();

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = new DatabaseBackend();
        d->backend->setDatabaseWatch(d->databaseWatch);
        d->db = new AlbumDB(d->backend);
    }

    delete d->infoCache;
    d->infoCache = new ImageInfoCache();
}

bool DatabaseAccess::checkReadyForUse(InitializationObserver *observer)
{
    QStringList drivers = QSqlDatabase::drivers();
    if (!drivers.contains("QSQLITE"))
    {
        DError() << "No SQLite3 driver available. List of QSqlDatabase drivers: " << drivers << endl;
        d->lastError = i18n("The driver \"SQLITE\" for SQLite3 databases is not available.\n"
                            "digiKam depends on the drivers provided by the SQL module of Qt4.");
        return false;
    }

    // create an object with private shortcut constructor
    DatabaseAccess access(false);

    if (!d->backend)
    {
        DWarning() << "No database backend available in checkReadyForUse. "
                      "Did you call setParameters before?" << endl;
        return false;
    }
    if (d->backend->isReady())
        return true;
    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
        {
            access.setLastError(i18n("Error opening database backend.\n ")
                                + d->backend->lastError());
            return false;
        }
    }

    // update schema
    SchemaUpdater updater(&access);
    updater.setObserver(observer);
    if (!d->backend->initSchema(&updater))
        return false;

    // initialize CollectionManager
    CollectionManager::instance()->refresh();

    return d->backend->isReady();
}

QString DatabaseAccess::lastError()
{
    return d->lastError;
}

void DatabaseAccess::setLastError(const QString &error)
{
    d->lastError = error;
}

void DatabaseAccess::cleanUpDatabase()
{
    if (d)
    {
        QMutexLocker lock(&d->mutex);
        d->backend->close();
        delete d->db;
        delete d->backend;
    }
    delete d;
    d = 0;
}

}  // namespace Digikam
