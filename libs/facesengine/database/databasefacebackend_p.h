/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Face database backend
 *
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef _DATABASE_FACE_BACKEND_P_H_
#define _DATABASE_FACE_BACKEND_P_H_

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

class DatabaseFaceBackendPrivate : public DatabaseErrorAnswer
{
public:

    explicit DatabaseFaceBackendPrivate(DatabaseFaceBackend* const backend);
    virtual ~DatabaseFaceBackendPrivate();

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
    void setQueryOperationFlag(DatabaseFaceBackend::QueryOperationStatus status);
    void queryOperationWakeAll(DatabaseFaceBackend::QueryOperationStatus status);

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

    DatabaseFaceBackend::Status               status;

    DatabaseLocking*                          lock;

    DatabaseFaceBackend::QueryOperationStatus operationStatus;

    QMutex                                    errorLockMutex;
    QWaitCondition                            errorLockCondVar;
    DatabaseFaceBackend::QueryOperationStatus errorLockOperationStatus;

    QMutex                                    busyWaitMutex;
    QWaitCondition                            busyWaitCondVar;

    DatabaseErrorHandler*                     errorHandler;

public :

    class AbstractUnlocker
    {
    public:

        explicit AbstractUnlocker(DatabaseFaceBackendPrivate* const d);
        ~AbstractUnlocker();

        void finishAcquire();

    protected:

        int                               count;
        DatabaseFaceBackendPrivate* const d;
    };

    friend class AbstractUnlocker;

    // ------------------------------------------------------------------

    class AbstractWaitingUnlocker : public AbstractUnlocker
    {
    public:

        AbstractWaitingUnlocker(DatabaseFaceBackendPrivate* const d, QMutex* const mutex, QWaitCondition* const condVar);
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

        explicit ErrorLocker(DatabaseFaceBackendPrivate* const d);
        void wait();
    };

    // ------------------------------------------------------------------

    class BusyWaiter : public AbstractWaitingUnlocker
    {
    public:

        explicit BusyWaiter(DatabaseFaceBackendPrivate* const d);
    };

public :

    DatabaseFaceBackend* const q;
};

} // namespace FacesEngine

#endif // _DATABASE_FACE_BACKEND_P_H_
