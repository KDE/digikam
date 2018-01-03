/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Database engine abstract database backend
 *
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASE_ENGINE_BACKEND_P_H
#define DATABASE_ENGINE_BACKEND_P_H

// Qt includes

#include <QHash>
#include <QSqlDatabase>
#include <QThread>
#include <QThreadStorage>
#include <QWaitCondition>

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class DbEngineThreadData
{
public:

    DbEngineThreadData();
    ~DbEngineThreadData();

    void closeDatabase();

    QSqlDatabase database;
    int          valid;
    int          transactionCount;
    QSqlError    lastError;
};

class DIGIKAM_EXPORT BdEngineBackendPrivate : public DbEngineErrorAnswer
{
public:

    explicit BdEngineBackendPrivate(BdEngineBackend* const backend);
    virtual ~BdEngineBackendPrivate();

    void init(const QString& connectionName, DbEngineLocking* const locking);

    QString connectionName();

    QSqlDatabase databaseForThread();
    QSqlError    databaseErrorForThread();
    void         setDatabaseErrorForThread(const QSqlError& lastError);

    QSqlDatabase createDatabaseConnection();
    void closeDatabaseForThread();
    bool incrementTransactionCount();
    bool decrementTransactionCount();

    bool isInMainThread() const;
    bool isInUIThread()   const;

    bool reconnectOnError()                                          const;
    bool isSQLiteLockError(const DbEngineSqlQuery& query)            const;
    bool isSQLiteLockTransactionError(const QSqlError& lastError)    const;
    bool isConnectionError(const DbEngineSqlQuery& query)            const;
    bool needToConsultUserForError(const DbEngineSqlQuery& query)    const;
    bool needToHandleWithErrorHandler(const DbEngineSqlQuery& query) const;
    void debugOutputFailedQuery(const QSqlQuery& query)              const;
    void debugOutputFailedTransaction(const QSqlError& error)        const;

    bool checkRetrySQLiteLockError(int retries);
    bool checkOperationStatus();
    bool handleWithErrorHandler(const DbEngineSqlQuery* const query);
    void setQueryOperationFlag(BdEngineBackend::QueryOperationStatus status);
    void queryOperationWakeAll(BdEngineBackend::QueryOperationStatus status);

    // called by DbEngineErrorHandler, implementing DbEngineErrorAnswer
    virtual void connectionErrorContinueQueries();
    virtual void connectionErrorAbortQueries();
    virtual void transactionFinished();

public:

    QThreadStorage<DbEngineThreadData*>       threadDataStorage;

    // This compares to DbEngineThreadData's valid. If currentValidity is increased and > valid, the db is marked as invalid
    int                                       currentValidity;

    bool                                      isInTransaction;

    QString                                   backendName;

    DbEngineParameters                        parameters;

    BdEngineBackend::Status                   status;

    DbEngineLocking*                          lock;

    BdEngineBackend::QueryOperationStatus     operationStatus;

    QMutex                                    errorLockMutex;
    QWaitCondition                            errorLockCondVar;
    BdEngineBackend::QueryOperationStatus     errorLockOperationStatus;

    QMutex                                    busyWaitMutex;
    QWaitCondition                            busyWaitCondVar;

    DbEngineErrorHandler*                     errorHandler;

public:

    class AbstractUnlocker
    {
    public:

        explicit AbstractUnlocker(BdEngineBackendPrivate* const d);
        ~AbstractUnlocker();

        void finishAcquire();

    protected:

        int                           count;
        BdEngineBackendPrivate* const d;
    };

    friend class AbstractUnlocker;

    // ------------------------------------------------------------------

    class AbstractWaitingUnlocker : public AbstractUnlocker
    {
    public:

        AbstractWaitingUnlocker(BdEngineBackendPrivate* const d, QMutex* const mutex, QWaitCondition* const condVar);
        ~AbstractWaitingUnlocker();

        bool wait(unsigned long time = ULONG_MAX);

    protected:

        QMutex*         const mutex;
        QWaitCondition* const condVar;
    };

    // ------------------------------------------------------------------

    class ErrorLocker : public AbstractWaitingUnlocker
    {
    public:

        explicit ErrorLocker(BdEngineBackendPrivate* const d);
        void wait();
    };

    // ------------------------------------------------------------------

    class BusyWaiter : public AbstractWaitingUnlocker
    {
    public:

        explicit BusyWaiter(BdEngineBackendPrivate* const d);
    };

public :

    BdEngineBackend* const q;
};

}  // namespace Digikam

#endif // DATABASE_ENGINE_BACKEND_P_H
