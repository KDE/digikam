/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-28
 * Description : Similarity Database access wrapper.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#include "similaritydbaccess.h"

// Qt includes

#include <QMutex>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "similaritydbbackend.h"
#include "similaritydb.h"
#include "similaritydbschemaupdater.h"
#include "dbengineparameters.h"
#include "dbengineaccess.h"

namespace Digikam
{

class SimilarityDbAccessStaticPriv
{
public:

    SimilarityDbAccessStaticPriv()
        : backend(0),
          db(0),
          initializing(false)
    {
    }

    ~SimilarityDbAccessStaticPriv()
    {
    };

    SimilarityDbBackend* backend;
    SimilarityDb*        db;
    DbEngineParameters   parameters;
    DbEngineLocking      lock;
    QString              lastError;

    bool                 initializing;
};

SimilarityDbAccessStaticPriv* SimilarityDbAccess::d = 0;

// -----------------------------------------------------------------------------

class SimilarityDbAccessMutexLocker : public QMutexLocker
{
public:

    explicit SimilarityDbAccessMutexLocker(SimilarityDbAccessStaticPriv* const d)
        : QMutexLocker(&d->lock.mutex),
          d(d)
    {
        d->lock.lockCount++;
    }

    ~SimilarityDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

public:

    SimilarityDbAccessStaticPriv* const d;
};

// -----------------------------------------------------------------------------

SimilarityDbAccess::SimilarityDbAccess()
{
    // You will want to call setParameters before constructing SimilarityDbAccess.
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

SimilarityDbAccess::~SimilarityDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

SimilarityDbAccess::SimilarityDbAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->lock.mutex.lock();
    d->lock.lockCount++;
}

SimilarityDb* SimilarityDbAccess::db() const
{
    return d->db;
}

SimilarityDbBackend* SimilarityDbAccess::backend() const
{
    return d->backend;
}

DbEngineParameters SimilarityDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

bool SimilarityDbAccess::isInitialized()
{
    return d;
}

void SimilarityDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d)
    {
        d = new SimilarityDbAccessStaticPriv();
    }

    //DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void SimilarityDbAccess::setParameters(const DbEngineParameters& parameters)
{
    if (!d)
    {
        d = new SimilarityDbAccessStaticPriv();
    }

    SimilarityDbAccessMutexLocker lock(d);

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
        d->backend = new SimilarityDbBackend(&d->lock);
        d->db      = new SimilarityDb(d->backend);
    }
}

bool SimilarityDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
        return false;

    // create an object with private shortcut constructor
    SimilarityDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: no database backend available in checkReadyForUse. "
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

    // avoid endless loops (if called methods create new SimilarityDbAccess objects)
    d->initializing = true;

    // update schema
    SimilarityDbSchemaUpdater updater(&access);
    updater.setObserver(observer);

    if (!d->backend->initSchema(&updater))
    {
        qCWarning(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: cannot process schema initialization";

        d->initializing = false;
        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString SimilarityDbAccess::lastError() const
{
    return d->lastError;
}

void SimilarityDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void SimilarityDbAccess::cleanUpDatabase()
{
    if (d)
    {
        SimilarityDbAccessMutexLocker locker(d);

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

} // namespace Digikam
