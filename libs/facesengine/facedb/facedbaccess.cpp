/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Face database access wrapper.
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

#include "facedbaccess.h"

// Qt includes

#include <QMutex>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "facedbbackend.h"
#include "facedb.h"
#include "facedbschemaupdater.h"
#include "dbengineparameters.h"
#include "dbengineaccess.h"

namespace Digikam
{

class FaceDbAccessStaticPriv
{
public:

    FaceDbAccessStaticPriv()
        : backend(0),
          db(0),
          initializing(false)
    {
    }

    ~FaceDbAccessStaticPriv()
    {
    }

public:

    FaceDbBackend*     backend;
    FaceDb*            db;
    DbEngineParameters parameters;
    DbEngineLocking    lock;
    QString            lastError;
    bool               initializing;
};

FaceDbAccessStaticPriv* FaceDbAccess::d = 0;

// ----------------------------------------------------------------

class FaceDbAccessMutexLocker : public QMutexLocker
{
public:

    FaceDbAccessMutexLocker(FaceDbAccessStaticPriv* const d)
        : QMutexLocker(&d->lock.mutex),
          d(d)
    {
        d->lock.lockCount++;
    }

    ~FaceDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

    FaceDbAccessStaticPriv* const d;
};

// ----------------------------------------------------------------

FaceDbAccess::FaceDbAccess()
{
    // You will want to call setParameters before constructing FaceDbAccess.
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

FaceDbAccess::~FaceDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

FaceDbAccess::FaceDbAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->lock.mutex.lock();
    d->lock.lockCount++;
}

FaceDb* FaceDbAccess::db() const
{
    return d->db;
}

FaceDbBackend* FaceDbAccess::backend() const
{
    return d->backend;
}

DbEngineParameters FaceDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

bool FaceDbAccess::isInitialized()
{
    return d;
}

void FaceDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d)
    {
        d = new FaceDbAccessStaticPriv();
    }

    //DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void FaceDbAccess::setParameters(const DbEngineParameters& parameters)
{
    if (!d)
    {
        d = new FaceDbAccessStaticPriv();
    }

    FaceDbAccessMutexLocker lock(d);

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
        d->backend = new FaceDbBackend(&d->lock);
        d->db      = new FaceDb(d->backend);
    }
}

bool FaceDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
        return false;

    // Create an object with private shortcut constructor

    FaceDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "Face database: no database backend available in checkReadyForUse. "
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

    // Avoid endless loops (if called methods create new FaceDbAccess objects)

    d->initializing = true;

    // Update schema

    FaceDbSchemaUpdater updater(&access);
    updater.setObserver(observer);

    if (!d->backend->initSchema(&updater))
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "Face database: cannot process schema initialization";

        d->initializing = false;
        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString FaceDbAccess::lastError() const
{
    return d->lastError;
}

void FaceDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void FaceDbAccess::cleanUpDatabase()
{
    if (d)
    {
        FaceDbAccessMutexLocker locker(d);

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

// ---------------------------------------------------------------------------------

FaceDbAccessUnlock::FaceDbAccessUnlock()
{
    // acquire lock
    FaceDbAccess::d->lock.mutex.lock();
    // store lock count
    count = FaceDbAccess::d->lock.lockCount;
    // set lock count to 0
    FaceDbAccess::d->lock.lockCount = 0;

    // unlock
    for (int i = 0; i < count; ++i)
    {
        FaceDbAccess::d->lock.mutex.unlock();
    }

    // drop lock acquired in first line. Mutex is now free.
    FaceDbAccess::d->lock.mutex.unlock();
}

FaceDbAccessUnlock::FaceDbAccessUnlock(FaceDbAccess* const)
{
    // With the passed pointer, we have assured that the mutex is acquired
    // Store lock count
    count = FaceDbAccess::d->lock.lockCount;
    // set lock count to 0
    FaceDbAccess::d->lock.lockCount = 0;

    // unlock
    for (int i = 0; i < count; ++i)
    {
        FaceDbAccess::d->lock.mutex.unlock();
    }

    // Mutex is now free
}

FaceDbAccessUnlock::~FaceDbAccessUnlock()
{
    // lock as often as it was locked before
    for (int i = 0; i < count; ++i)
    {
        FaceDbAccess::d->lock.mutex.lock();
    }

    // update lock count
    FaceDbAccess::d->lock.lockCount += count;
}

} // namespace Digikam
