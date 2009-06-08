/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Abstract database backend
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QHash>
#include <QSqlDatabase>
#include <QThread>

// KDE includes

// Local includes

#include "digikam_export.h"
#include "databaseparameters.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseCoreBackendPrivate
{
public:

    DatabaseCoreBackendPrivate(DatabaseCoreBackend *backend);
    virtual ~DatabaseCoreBackendPrivate() {}
    void init();

    // "A connection can only be used from within the thread that created it.
    //  Moving connections between threads or creating queries from a different thread is not supported."
    // => one QSqlDatabase object per thread.
    // The main class' open/close methods only interact with the "firstDatabase" object.
    // When another thread requests a DB, a new connection is opened and closed at
    // finishing of the thread.

    inline QString connectionName(QThread *thread)
    {
        return QString("digikamDatabase-" + QString::number((quintptr)thread));
    }

    QSqlDatabase databaseForThread();
    void closeDatabaseForThread();
    bool open(QSqlDatabase& db);
    bool incrementTransactionCount();
    bool decrementTransactionCount();
    bool isInTransactionInOtherThread() const;

    virtual void transactionFinished() {}

    // this is always accessed in mutex context, no need for QThreadStorage
    QHash<QThread*, QSqlDatabase> threadDatabases;
    // this is not only db.isValid(), but also "parameters changed, need to reopen"
    QHash<QThread*, int>          databasesValid;
    // for recursive transactions
    QHash<QThread*, int>          transactionCount;

    bool                          isInTransaction;

    DatabaseParameters            parameters;

    DatabaseCoreBackend::Status   status;

    DatabaseCoreBackend* const    q;

};


}  // namespace Digikam
