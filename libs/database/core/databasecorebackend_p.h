/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Abstract database backend
 *
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASECOREBACKEND_P_H
#define DATABASECOREBACKEND_P_H

// Qt includes

#include <QHash>
#include <QSqlDatabase>
#include <QThread>
#include <QWaitCondition>

// Local includes

#include "digikam_export.h"
#include "databaseparameters.h"
#include "databaseerrorhandler.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseCoreBackendPrivate : public DatabaseErrorAnswer
{
public:

    explicit DatabaseCoreBackendPrivate(DatabaseCoreBackend* backend);
    virtual ~DatabaseCoreBackendPrivate() {}

    void init(const QString& connectionName, DatabaseLocking* locking);

    QString connectionName(QThread* thread);

    QSqlDatabase databaseForThread();
    QSqlError    databaseErrorForThread();
    void         setDatabaseErrorForThread(QSqlError lastError);

    void closeDatabaseForThread();
    bool open(QSqlDatabase& db);
    bool incrementTransactionCount();
    bool decrementTransactionCount();
    bool isInTransactionInOtherThread() const;

    bool isInMainThread() const;
    bool isInUIThread() const;

    bool reconnectOnError() const;
    bool isSQLiteLockError(const SqlQuery& query) const;
    bool isSQLiteLockTransactionError(const QSqlError& lastError) const;
    bool checkRetrySQLiteLockError(int retries);
    bool isConnectionError(const SqlQuery& query) const;
    bool needToConsultUserForError(const SqlQuery& query) const;
    bool needToHandleWithErrorHandler(const SqlQuery& query) const;
    void debugOutputFailedQuery(const QSqlQuery& query) const;
    void debugOutputFailedTransaction(const QSqlError& error) const;

    bool checkOperationStatus();
    bool handleWithErrorHandler(const SqlQuery* query);
    // called by DatabaseErrorHandler, implementing DatabaseErrorAnswer
    virtual void connectionErrorContinueQueries();
    virtual void connectionErrorAbortQueries();
    void setQueryOperationFlag(DatabaseCoreBackend::QueryOperationStatus status);
    void queryOperationWakeAll(DatabaseCoreBackend::QueryOperationStatus status);

    virtual void transactionFinished();

public:

    // this is always accessed in mutex context, no need for QThreadStorage
    QHash<QThread*, QSqlDatabase>             threadDatabases;
    // this is not only db.isValid(), but also "parameters changed, need to reopen"
    QHash<QThread*, int>                      databasesValid;
    // for recursive transactions
    QHash<QThread*, int>                      transactionCount;

    QHash<QThread*, QSqlError>                databaseErrors;

    bool                                      isInTransaction;

    QString                                   backendName;

    DatabaseParameters                        parameters;

    DatabaseCoreBackend::Status               status;

    DatabaseLocking*                          lock;

    DatabaseCoreBackend::QueryOperationStatus operationStatus;

    QMutex                                    errorLockMutex;
    QWaitCondition                            errorLockCondVar;
    DatabaseCoreBackend::QueryOperationStatus errorLockOperationStatus;

    QMutex                                    busyWaitMutex;
    QWaitCondition                            busyWaitCondVar;

    DatabaseErrorHandler*                     errorHandler;

public :

    class AbstractUnlocker
    {
    public:

        explicit AbstractUnlocker(DatabaseCoreBackendPrivate* d);
        void finishAcquire();
        ~AbstractUnlocker();

    protected:

        int count;
        DatabaseCoreBackendPrivate* const d;
    };
    friend class AbstractUnlocker;

    class AbstractWaitingUnlocker : public AbstractUnlocker
    {
    public:

        AbstractWaitingUnlocker(DatabaseCoreBackendPrivate* d, QMutex* mutex, QWaitCondition* condVar);
        ~AbstractWaitingUnlocker();

        bool wait(unsigned long time = ULONG_MAX);

    protected:

        QMutex*         const mutex;
        QWaitCondition* const condVar;
    };

    class ErrorLocker : public AbstractWaitingUnlocker
    {
    public:

        explicit ErrorLocker(DatabaseCoreBackendPrivate* d);
        void wait();
    };

    class BusyWaiter : public AbstractWaitingUnlocker
    {
    public:
        explicit BusyWaiter(DatabaseCoreBackendPrivate* d);
    };

public :

    DatabaseCoreBackend* const q;
};

}  // namespace Digikam

#endif // DATABASECOREBACKEND_P_H
