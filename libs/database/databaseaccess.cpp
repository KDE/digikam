/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-03-18
 * Description : database interface.
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <qmutex.h>

#include "albumdb.h"
#include "databasebackend.h"
#include "databaseaccess.h"

namespace Digikam
{

class DatabaseAccessStaticPriv
{
public:
    DatabaseAccessStaticPriv()
    : backend(0), db(0), mutex(true) // create a recursive mutex
    {
    };
    ~DatabaseAccessStaticPriv() {};

    DatabaseBackend *backend;
    AlbumDB *db;
    DatabaseParameters parameters;
    QMutex mutex;
    QString albumRoot;
};

DatabaseAccessStaticPriv *DatabaseAccess::d = 0;

DatabaseAccess::DatabaseAccess()
{
    d->mutex.lock();
    if (!d->backend->isReady())
    {
        if (!d->backend->isOpen())
            d->backend->open(d->parameters);
        d->backend->initSchema();
    }
}

DatabaseAccess::~DatabaseAccess()
{
    d->mutex.unlock();
}

AlbumDB *DatabaseAccess::db() const
{
    return d->db;
}

DatabaseBackend *DatabaseAccess::backend() const
{
    return d->backend;
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

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = DatabaseBackend::createBackend(parameters);
        d->db = new AlbumDB(d->backend);
    }

    //TODO: remove when albumRoot is removed
    if (d->parameters.databaseType == "QSQLITE")
    {
        KURL url;
        url.setPath(d->parameters.databaseName);
        d->albumRoot = url.directory(true);
    }
}

bool DatabaseAccess::checkReadyForUse()
{
    // this code is similar to the constructor
    QMutexLocker locker(&d->mutex);

    if (!d->backend)
        return false;
    if (d->backend->isReady())
        return true;
    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
            return false;
    }
    return d->backend->initSchema();
}

QString DatabaseAccess::albumRoot()
{
    return d->albumRoot;
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



}
