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

#include <qmutex.h>

// KDE includes

#include <klocale.h>

// Local includes

#include "ddebug.h"
#include "albumdb.h"
#include "imageinfocache.h"
#include "schemaupdater.h"
#include "databaseattributeswatch.h"
#include "databasebackend.h"
#include "databaseaccess.h"

namespace Digikam
{

class DatabaseAccessStaticPriv
{
public:
    DatabaseAccessStaticPriv()
    : backend(0), db(0), infoCache(0), attributesWatch(0), mutex(QMutex::Recursive) // create a recursive mutex
    {
    };
    ~DatabaseAccessStaticPriv() {};

    DatabaseBackend *backend;
    AlbumDB *db;
    ImageInfoCache *infoCache;
    DatabaseAttributesWatch *attributesWatch;
    DatabaseParameters parameters;
    QMutex mutex;
    QString albumRoot;
    QString lastError;
};

DatabaseAccessStaticPriv *DatabaseAccess::d = 0;

DatabaseAccess::DatabaseAccess()
{
    d->mutex.lock();
    if (!d->backend->isOpen())
    {
        d->backend->open(d->parameters);
    }
}

DatabaseAccess::~DatabaseAccess()
{
    d->mutex.unlock();
}

DatabaseAccess::DatabaseAccess(QMutexLocker *)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
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

DatabaseAttributesWatch *DatabaseAccess::attributesWatch()
{
    return d->attributesWatch;
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

    if (!d->attributesWatch)
        d->attributesWatch = new DatabaseAttributesWatch();

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        delete d->infoCache;
        d->backend = DatabaseBackend::createBackend(parameters);
        d->db = new AlbumDB(d->backend);
        d->infoCache = new ImageInfoCache();
    }

    //TODO: remove when albumRoot is removed
    if (d->parameters.databaseType == "QSQLITE")
    {
        KUrl url;
        url.setPath(d->parameters.databaseName);
        d->albumRoot = url.directory();
    }
}

bool DatabaseAccess::checkReadyForUse()
{
    // this code is similar to the constructor
    QMutexLocker locker(&d->mutex);

    // create an object with private shortcut constructor
    DatabaseAccess access(&locker);

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

    SchemaUpdater updater(&access);
    d->backend->initSchema(&updater);

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
