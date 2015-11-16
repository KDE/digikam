/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database access wrapper.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbsdbaccess.h"

// Qt includes

#include <QMutex>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "thumbsdbbackend.h"
#include "thumbsdb.h"
#include "thumbsdbchemaupdater.h"

namespace Digikam
{

class ThumbsDbAccessStaticPriv
{
public:

    ThumbsDbAccessStaticPriv()
        : backend(0),
          db(0),
          initializing(false)
    {
    }

    ~ThumbsDbAccessStaticPriv()
    {
    };

    ThumbsDbBackend* backend;
    ThumbsDb*              db;
    DatabaseParameters        parameters;
    DatabaseLocking           lock;
    QString                   lastError;

    bool                      initializing;
};

class ThumbsDbAccessMutexLocker : public QMutexLocker
{
public:

    explicit ThumbsDbAccessMutexLocker(ThumbsDbAccessStaticPriv* const d)
        : QMutexLocker(&d->lock.mutex),
          d(d)
    {
        d->lock.lockCount++;
    }

    ~ThumbsDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

public:

    ThumbsDbAccessStaticPriv* const d;
};

ThumbsDbAccessStaticPriv* ThumbsDbAccess::d = 0;

ThumbsDbAccess::ThumbsDbAccess()
{
    Q_ASSERT(d/*You will want to call setParameters before constructing ThumbsDbAccess*/);
    d->lock.mutex.lock();
    d->lock.lockCount++;

    if (!d->backend->isOpen() && !d->initializing)
    {
        // avoid endless loops
        d->initializing = true;

        d->backend->open(d->parameters);

        d->initializing = false;
    }
}

ThumbsDbAccess::~ThumbsDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

ThumbsDbAccess::ThumbsDbAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->lock.mutex.lock();
    d->lock.lockCount++;
}

ThumbsDb* ThumbsDbAccess::db() const
{
    return d->db;
}

ThumbsDbBackend* ThumbsDbAccess::backend() const
{
    return d->backend;
}

DatabaseParameters ThumbsDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DatabaseParameters();
}

bool ThumbsDbAccess::isInitialized()
{
    return d;
}

void ThumbsDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* errorhandler)
{
    if (!d)
    {
        d = new ThumbsDbAccessStaticPriv();
    }

    //DbEngineErrorHandler *errorhandler = new DatabaseGUIErrorHandler(d->parameters);
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void ThumbsDbAccess::setParameters(const DatabaseParameters& parameters)
{
    if (!d)
    {
        d = new ThumbsDbAccessStaticPriv();
    }

    ThumbsDbAccessMutexLocker lock(d);

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
        d->backend->setDbEngineErrorHandler(0);
    }

    d->parameters = parameters;

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = new ThumbsDbBackend(&d->lock);
        d->db      = new ThumbsDb(d->backend);
    }
}

bool ThumbsDbAccess::checkReadyForUse(InitializationObserver* observer)
{
    QStringList drivers = QSqlDatabase::drivers();

    if (!drivers.contains(QLatin1String("QSQLITE")))
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Thumbs database: no SQLite3 driver available. List of QSqlDatabase drivers: " << drivers;

        d->lastError = i18n("The driver \"SQLITE\" for SQLite3 databases is not available.\n"
                            "digiKam depends on the drivers provided by the SQL module of Qt4.");
        return false;
    }

    // create an object with private shortcut constructor
    ThumbsDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Thumbs database: no database backend available in checkReadyForUse. "
                                           "Did you call setParameters before?";
        return false;
    }

    if (d->backend->isReady())
    {
        return true;
    }

    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
        {
            access.setLastError(i18n("Error opening database backend.\n ")
                                + d->backend->lastError());
            return false;
        }
    }

    // avoid endless loops (if called methods create new ThumbsDbAccess objects)
    d->initializing = true;

    // update schema
    ThumbsDbSchemaUpdater updater(&access);
    updater.setObserver(observer);

    if (!d->backend->initSchema(&updater))
    {
        d->initializing = false;
        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString ThumbsDbAccess::lastError()
{
    return d->lastError;
}

void ThumbsDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void ThumbsDbAccess::cleanUpDatabase()
{
    if (d)
    {
        ThumbsDbAccessMutexLocker locker(d);
        d->backend->close();
        delete d->db;
        delete d->backend;
    }

    delete d;
    d = 0;
}

}  // namespace Digikam
