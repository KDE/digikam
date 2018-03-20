/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database access wrapper.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dbengineparameters.h"
#include "dbengineaccess.h"

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

    ThumbsDbBackend*   backend;
    ThumbsDb*          db;
    DbEngineParameters parameters;
    DbEngineLocking    lock;
    QString            lastError;

    bool               initializing;
};

ThumbsDbAccessStaticPriv* ThumbsDbAccess::d = 0;

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

ThumbsDbAccess::ThumbsDbAccess()
{
    // You will want to call setParameters before constructing ThumbsDbAccess.
    Q_ASSERT(d);

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

DbEngineParameters ThumbsDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

bool ThumbsDbAccess::isInitialized()
{
    return d;
}

void ThumbsDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d)
    {
        d = new ThumbsDbAccessStaticPriv();
    }

    //DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void ThumbsDbAccess::setParameters(const DbEngineParameters& parameters)
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

bool ThumbsDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
        return false;

    // create an object with private shortcut constructor
    ThumbsDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: no database backend available in checkReadyForUse. "
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
            access.setLastError(i18n("Error opening database backend.\n%1",
                                d->backend->lastError()));
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
        qCWarning(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: cannot process schema initialization";

        d->initializing = false;
        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString ThumbsDbAccess::lastError() const
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

        if (d->backend)
        {
            d->backend->close();
            delete d->db;
            delete d->backend;
        }
    }

    delete d;
    d = 0;
}

}  // namespace Digikam
