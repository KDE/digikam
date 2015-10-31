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

#ifndef DATABASEFACECOREBACKEND_P_H
#define DATABASEFACECOREBACKEND_P_H

// Qt includes

#include <QHash>
#include <QSqlDatabase>
#include <QThread>
#include <QThreadStorage>
#include <QWaitCondition>

// Local includes

#include "databasefaceparameters.h"

namespace FacesEngine
{

class DatabaseThreadData
{
public:

    DatabaseThreadData();
    ~DatabaseThreadData();

    void closeDatabase();

    QSqlDatabase database;
    int          valid;
    int          transactionCount;
    QSqlError    lastError;
};

class DatabaseFaceCoreBackendPrivate : public DatabaseErrorAnswer
{
public:

    explicit DatabaseFaceCoreBackendPrivate(DatabaseFaceCoreBackend* const backend);
    virtual ~DatabaseFaceCoreBackendPrivate();

    void init(const QString& connectionName, DatabaseLocking* const locking);

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

    bool reconnectOnError()                                       const;
    bool isSQLiteLocqCritical(const SqlQuery& query)              const;
    bool isSQLiteLockTransactionError(const QSqlError& lastError) const;
    bool isConnectionError(const SqlQuery& query)                 const;
    bool needToConsultUserForError(const SqlQuery& query)         const;
    bool needToHandleWithErrorHandler(const SqlQuery& query)      const;
    void debugOutputFailedQuery(const QSqlQuery& query)           const;
    void debugOutputFailedTransaction(const QSqlError& error)     const;

    bool checkRetrySQLiteLocqCritical(int retries);
    bool checkOperationStatus();
    bool handleWithErrorHandler(const SqlQuery* const query);
    void setQueryOperationFlag(DatabaseFaceCoreBackend::QueryOperationStatus status);
    void queryOperationWakeAll(DatabaseFaceCoreBackend::QueryOperationStatus status);

    // called by DatabaseErrorHandler, implementing DatabaseErrorAnswer
    virtual void connectionErrorContinueQueries();
    virtual void connectionErrorAbortQueries();
    virtual void transactionFinished();

public:

    QThreadStorage<DatabaseThreadData*>       threadDataStorage;

    // This compares to DatabaseThreadData's valid. If currentValidity is increased and > valid, the db is marked as invalid
    int                                       currentValidity;

    bool                                      isInTransaction;

    QString                                   backendName;

    DatabaseFaceParameters                        parameters;

    DatabaseFaceCoreBackend::Status               status;

    DatabaseLocking*                          lock;

    DatabaseFaceCoreBackend::QueryOperationStatus operationStatus;

    QMutex                                    errorLockMutex;
    QWaitCondition                            errorLockCondVar;
    DatabaseFaceCoreBackend::QueryOperationStatus errorLockOperationStatus;

    QMutex                                    busyWaitMutex;
    QWaitCondition                            busyWaitCondVar;

    DatabaseErrorHandler*                     errorHandler;

public :

    class AbstractUnlocker
    {
    public:

        explicit AbstractUnlocker(DatabaseFaceCoreBackendPrivate* const d);
        ~AbstractUnlocker();

        void finishAcquire();

    protected:

        int                               count;
        DatabaseFaceCoreBackendPrivate* const d;
    };

    friend class AbstractUnlocker;

    // ------------------------------------------------------------------

    class AbstractWaitingUnlocker : public AbstractUnlocker
    {
    public:

        AbstractWaitingUnlocker(DatabaseFaceCoreBackendPrivate* const d, QMutex* const mutex, QWaitCondition* const condVar);
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

        explicit ErrorLocker(DatabaseFaceCoreBackendPrivate* const d);
        void wait();
    };

    // ------------------------------------------------------------------

    class BusyWaiter : public AbstractWaitingUnlocker
    {
    public:

        explicit BusyWaiter(DatabaseFaceCoreBackendPrivate* const d);
    };

public :

    DatabaseFaceCoreBackend* const q;
};

} // namespace FacesEngine

#endif // DATABASEFACECOREBACKEND_P_H
