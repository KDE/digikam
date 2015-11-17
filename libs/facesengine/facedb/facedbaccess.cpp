/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Face database access wrapper.
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

#include "facedbaccess.h"

// Qt includes

#include <QMutex>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbenginebackend.h"
#include "trainingdb.h"
#include "facedbschemaupdater.h"

namespace FacesEngine
{

class FaceDbAccessData
{
public:

    FaceDbAccessData()
        : backend(0),
          db(0),
          initializing(false)
    {
    }

    ~FaceDbAccessData()
    {
    }

public:

    FaceDbBackend*     backend;
    TrainingDB*        db;
    DbEngineParameters parameters;
    DbEngineLocking    lock;
    QString            lastError;
    bool               initializing;
};

// ----------------------------------------------------------------

class FaceDbAccessMutexLocker : public QMutexLocker
{
public:

    FaceDbAccessMutexLocker(FaceDbAccessData* const d)
        : QMutexLocker(&d->lock.mutex),
          d(d)
    {
        d->lock.lockCount++;
    }

    ~FaceDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

    FaceDbAccessData* const d;
};

// ----------------------------------------------------------------

FaceDbAccessData* FaceDbAccess::create()
{
    return new FaceDbAccessData;
}

void FaceDbAccess::destroy(FaceDbAccessData* const d)
{
    if (d)
    {
        FaceDbAccessMutexLocker locker(d);
        d->backend->close();
        delete d->db;
        delete d->backend;
    }

    delete d;
}

FaceDbAccess::FaceDbAccess(FaceDbAccessData* const d)
    : d(d)
{
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

FaceDbAccess::FaceDbAccess(bool, FaceDbAccessData* const d)
    : d(d)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->lock.mutex.lock();
    d->lock.lockCount++;
}

TrainingDB* FaceDbAccess::db() const
{
    return d->db;
}

FaceDbBackend* FaceDbAccess::backend() const
{
    return d->backend;
}

DbEngineParameters FaceDbAccess::parameters() const
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

void FaceDbAccess::initDbEngineErrorHandler(FaceDbAccessData* const d, DbEngineErrorHandler* const errorhandler)
{
    //DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void FaceDbAccess::setParameters(FaceDbAccessData* const d, const DbEngineParameters& parameters)
{
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
        d->db      = new TrainingDB(d->backend);
    }
}

bool FaceDbAccess::checkReadyForUse(FaceDbAccessData* const d, InitializationObserver* const observer)
{
    QStringList drivers = QSqlDatabase::drivers();

    if (!drivers.contains(QString::fromLatin1("QSQLITE")))
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "No Sqlite3 driver available. List of QSqlDatabase drivers: " << drivers;

        d->lastError = i18n("The driver \"SQLITE\" for Sqlite3 databases is not available.\n"
                            "digiKam depends on the drivers provided by the Qt::SQL module.");
        return false;
    }

    // create an object with private shortcut constructor
    FaceDbAccess access(false, d);

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

    // avoid endless loops (if called methods create new FaceDbAccess objects)
    d->initializing = true;

    // update schema
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

// ---------------------------------------------------------------------------------

FaceDbAccessUnlock::FaceDbAccessUnlock(FaceDbAccessData* const d)
    : d(d)
{
    // acquire lock
    d->lock.mutex.lock();
    // store lock count
    count = d->lock.lockCount;
    // set lock count to 0
    d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        d->lock.mutex.unlock();
    }

    // drop lock acquired in first line. Mutex is now free.
    d->lock.mutex.unlock();
}

FaceDbAccessUnlock::FaceDbAccessUnlock(FaceDbAccess* const access)
    : d(access->d)
{
    // With the passed pointer, we have assured that the mutex is acquired
    // Store lock count
    count = d->lock.lockCount;
    // set lock count to 0
    d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        d->lock.mutex.unlock();
    }

    // Mutex is now free
}

FaceDbAccessUnlock::~FaceDbAccessUnlock()
{
    // lock as often as it was locked before
    for (int i=0; i<count; ++i)
    {
        d->lock.mutex.lock();
    }

    // update lock count
    d->lock.lockCount += count;
}

} // namespace FacesEngine
