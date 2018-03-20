/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Database engine abstract database backend
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dbenginebackend.h"
#include "dbenginebackend_p.h"

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QRegExp>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlRecord>
#include <QThread>
#include <QTime>

// Local includes

#include "digikam_debug.h"
#include "dbengineactiontype.h"

namespace Digikam
{

DbEngineLocking::DbEngineLocking()
    : mutex(QMutex::Recursive),
      lockCount(0) // create a recursive mutex
{
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::BusyWaiter::BusyWaiter(BdEngineBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->busyWaitMutex, &d->busyWaitCondVar)
{
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::ErrorLocker::ErrorLocker(BdEngineBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->errorLockMutex, &d->errorLockCondVar)
{
}

// -----------------------------------------------------------------------------------------

DbEngineThreadData::DbEngineThreadData()
    : valid(0),
      transactionCount(0)
{
}

DbEngineThreadData::~DbEngineThreadData()
{
    if (transactionCount)
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "WARNING !!! Transaction count is" << transactionCount << "when destroying database!!!";
    }

    closeDatabase();
}

void DbEngineThreadData::closeDatabase()
{
    QString connectionToRemove;

    if (database.isOpen())
    {
        connectionToRemove = database.connectionName();
    }

    // Destroy object
    database         = QSqlDatabase();

    valid            = 0;
    transactionCount = 0;
    lastError        = QSqlError();

    // Remove connection
    if (!connectionToRemove.isNull())
    {
        QSqlDatabase::removeDatabase(connectionToRemove);
    }
}

BdEngineBackendPrivate::BdEngineBackendPrivate(BdEngineBackend* const backend)
    : currentValidity(0),
      isInTransaction(false),
      status(BdEngineBackend::Unavailable),
      lock(0),
      operationStatus(BdEngineBackend::ExecuteNormal),
      errorLockOperationStatus(BdEngineBackend::ExecuteNormal),
      errorHandler(0),
      q(backend)
{
}

BdEngineBackendPrivate::~BdEngineBackendPrivate()
{
    // Must be shut down from the main thread.
    // Clean up the QThreadStorage. It deletes any stored data.
    threadDataStorage.setLocalData(0);
}

void BdEngineBackendPrivate::init(const QString& name, DbEngineLocking* const l)
{
    backendName = name;
    lock        = l;

    qRegisterMetaType<DbEngineErrorAnswer*>("DbEngineErrorAnswer*");
    qRegisterMetaType<QSqlError>();
}

// "A connection can only be used from within the thread that created it.
//  Moving connections between threads or creating queries from a different thread is not supported."
// => one QSqlDatabase object per thread.
// The main class' open/close methods only interact with the "firstDatabase" object.
// When another thread requests a DB, a new connection is opened and closed at
// finishing of the thread.
QSqlDatabase BdEngineBackendPrivate::databaseForThread()
{
    DbEngineThreadData* threadData = 0;

    if (!threadDataStorage.hasLocalData())
    {
        threadData = new DbEngineThreadData;
        threadDataStorage.setLocalData(threadData);
    }
    else
    {
        threadData = threadDataStorage.localData();
    }

    // do we need to reopen the database because parameter changed and validity was increased?
    if (threadData->valid && threadData->valid < currentValidity)
    {
        threadData->closeDatabase();
    }

    if (!threadData->valid || !threadData->database.isOpen())
    {
        threadData->database = createDatabaseConnection();

        if (threadData->database.open())
        {
            threadData->valid = currentValidity;
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while opening the database. Error was" << threadData->database.lastError();
        }
    }

    return threadData->database;
}

QSqlDatabase BdEngineBackendPrivate::createDatabaseConnection()
{
    QSqlDatabase db        = QSqlDatabase::addDatabase(parameters.databaseType, connectionName());
    QString connectOptions = parameters.connectOptions;

    if (parameters.isSQLite())
    {
        QStringList toAdd;
        // enable shared cache, especially useful with SQLite >= 3.5.0
        toAdd << QLatin1String("QSQLITE_ENABLE_SHARED_CACHE");
        // We do our own waiting.
        toAdd << QLatin1String("QSQLITE_BUSY_TIMEOUT=0");

        if (!connectOptions.isEmpty())
        {
            connectOptions += QLatin1Char(';');
        }

        connectOptions += toAdd.join(QLatin1String(";"));
    }

    db.setDatabaseName(parameters.databaseNameCore);
    db.setConnectOptions(connectOptions);
    db.setHostName(parameters.hostName);
    db.setPort(parameters.port);
    db.setUserName(parameters.userName);
    db.setPassword(parameters.password);

    return db;
}

void BdEngineBackendPrivate::closeDatabaseForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->closeDatabase();
    }
}

QSqlError BdEngineBackendPrivate::databaseErrorForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        return threadDataStorage.localData()->lastError;
    }

    return QSqlError();
}

void BdEngineBackendPrivate::setDatabaseErrorForThread(const QSqlError& lastError)
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->lastError = lastError;
    }
}

QString BdEngineBackendPrivate::connectionName()
{
    return backendName + QString::number((quintptr)QThread::currentThread());
}

bool BdEngineBackendPrivate::incrementTransactionCount()
{
    return (!threadDataStorage.localData()->transactionCount++);
}

bool BdEngineBackendPrivate::decrementTransactionCount()
{
    return (!--threadDataStorage.localData()->transactionCount);
}

bool BdEngineBackendPrivate::isInMainThread() const
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

bool BdEngineBackendPrivate::isInUIThread() const
{
    QApplication* const app = qobject_cast<QApplication*>(QCoreApplication::instance());

    if (!app)
    {
        return false;
    }

    return (QThread::currentThread() == app->thread());
}

bool BdEngineBackendPrivate::reconnectOnError() const
{
    return parameters.isMySQL();
}

bool BdEngineBackendPrivate::isSQLiteLockError(const DbEngineSqlQuery& query) const
{
    return parameters.isSQLite() &&
           (query.lastError().number() == 5 /*SQLITE_BUSY*/ || query.lastError().number() == 6/*SQLITE_LOCKED*/);
}

bool BdEngineBackendPrivate::isSQLiteLockTransactionError(const QSqlError& lastError) const
{
    return (parameters.isSQLite()                                   &&
            lastError.type()         == QSqlError::TransactionError &&
            lastError.databaseText() == QLatin1String("database is locked")
           );

    // wouldnt it be great if they gave us the database error number...
}

bool BdEngineBackendPrivate::isConnectionError(const DbEngineSqlQuery& query) const
{
    // the backend returns connection error e.g. for Constraint Failed errors.
    if (parameters.isSQLite())
    {
        return false;
    }

    return (query.lastError().type()   == QSqlError::ConnectionError ||
            query.lastError().number() == 2006
           );
}

bool BdEngineBackendPrivate::needToConsultUserForError(const DbEngineSqlQuery&) const
{
    // no such conditions found and defined as yet
    return false;
}

bool BdEngineBackendPrivate::needToHandleWithErrorHandler(const DbEngineSqlQuery& query) const
{
    return (isConnectionError(query) || needToConsultUserForError(query));
}

bool BdEngineBackendPrivate::checkRetrySQLiteLockError(int retries)
{
    if (!(retries % 25))
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Database is locked. Waited" << retries*10;
    }

    const int uiMaxRetries = 50;
    const int maxRetries   = 1000;

    if (retries > qMax(uiMaxRetries, maxRetries))
    {
        if (retries > (isInUIThread() ? uiMaxRetries : maxRetries))
        {
            qCWarning(DIGIKAM_DBENGINE_LOG) << "Detected locked database file. There is an active transaction. Waited but giving up now.";
            return false;
        }
    }

    BusyWaiter waiter(this);
    waiter.wait(10);
    return true;
}

void BdEngineBackendPrivate::debugOutputFailedQuery(const QSqlQuery& query) const
{
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Failure executing query:\n"
                                  << query.executedQuery()
                                  << "\nError messages:" << query.lastError().driverText() << query.lastError().databaseText()
                                  << query.lastError().number() << query.lastError().type()
                                  << "\nBound values: " << query.boundValues().values();
}

void BdEngineBackendPrivate::debugOutputFailedTransaction(const QSqlError& error) const
{
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Failure executing transaction. Error messages:\n"
                                  << error.driverText() << error.databaseText()
                                  << error.number() << error.type();
}

void BdEngineBackendPrivate::transactionFinished()
{
    // wakes up any BusyWaiter waiting on the busyWaitCondVar.
    // Possibly called under d->lock->mutex lock, so we do not lock the busyWaitMutex
    busyWaitCondVar.wakeOne();
}

/** Set the wait flag to queryStatus. Typically, call this with Wait. */
void BdEngineBackendPrivate::setQueryOperationFlag(BdEngineBackend::QueryOperationStatus status)
{
    // Enforce lock order (first main mutex, second error lock mutex)
    QMutexLocker l(&errorLockMutex);
    // this change must be done under errorLockMutex lock
    errorLockOperationStatus = status;
    operationStatus          = status;
}

/** Set the wait flag to queryStatus and wake all waiting threads.
 *  Typically, call wakeAll with status ExecuteNormal or AbortQueries.
 */
void BdEngineBackendPrivate::queryOperationWakeAll(BdEngineBackend::QueryOperationStatus status)
{
    QMutexLocker l(&errorLockMutex);
    operationStatus          = status;
    errorLockOperationStatus = status;
    errorLockCondVar.wakeAll();
}

bool BdEngineBackendPrivate::checkOperationStatus()
{
    while (operationStatus == BdEngineBackend::Wait)
    {
        ErrorLocker locker(this);
        locker.wait();
    }

    if (operationStatus == BdEngineBackend::ExecuteNormal)
    {
        return true;
    }
    else if (operationStatus == BdEngineBackend::AbortQueries)
    {
        return false;
    }

    return false;
}

/// Returns true if the query shall be retried
bool BdEngineBackendPrivate::handleWithErrorHandler(const DbEngineSqlQuery* const query)
{
    if (errorHandler)
    {
        setQueryOperationFlag(BdEngineBackend::Wait);

        ErrorLocker locker(this);
        bool called         = false;
        QSqlError lastError = query ? query->lastError() : databaseForThread().lastError();
        QString lastQuery   = query ? query->lastQuery() : QString();

        if (!query || isConnectionError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "connectionError",
                                               Qt::AutoConnection,
                                               Q_ARG(DbEngineErrorAnswer*, this),
                                               Q_ARG(const QSqlError, lastError),
                                               Q_ARG(const QString, lastQuery));
        }
        else if (needToConsultUserForError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "consultUserForError",
                                               Qt::AutoConnection,
                                               Q_ARG(DbEngineErrorAnswer*, this),
                                               Q_ARG(const QSqlError, lastError),
                                               Q_ARG(const QString, lastQuery));
        }
        else
        {
            // unclear what to do.
            errorLockOperationStatus = BdEngineBackend::ExecuteNormal;
            operationStatus          = BdEngineBackend::ExecuteNormal;
            return true;
        }

        if (called)
        {
            locker.wait();
        }
        else
        {
            qCWarning(DIGIKAM_DBENGINE_LOG) << "Failed to invoke DbEngineErrorHandler. Aborting all queries.";
            operationStatus = BdEngineBackend::AbortQueries;
        }

        switch (operationStatus)
        {
            case BdEngineBackend::ExecuteNormal:
            case BdEngineBackend::Wait:
                return true;
            case BdEngineBackend::AbortQueries:
                return false;
        }
    }
    else
    {
        // TODO check if it's better to use an own error handler for kio slaves.
        // But for now, close only the database in the hope, that the next
        // access will be successful.
        closeDatabaseForThread();
    }

    return false;
}

void BdEngineBackendPrivate::connectionErrorContinueQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(BdEngineBackend::ExecuteNormal);
}

void BdEngineBackendPrivate::connectionErrorAbortQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(BdEngineBackend::AbortQueries);
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::AbstractUnlocker::AbstractUnlocker(BdEngineBackendPrivate* const d)
    : count(0),
      d(d)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.

    // acquire lock
    d->lock->mutex.lock();
    // store lock count
    count = d->lock->lockCount;
    // set lock count to 0
    d->lock->lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        d->lock->mutex.unlock();
    }
}

void BdEngineBackendPrivate::AbstractUnlocker::finishAcquire()
{
    // drop lock acquired in first line. Main mutex is now free.
    // We maintain lock order (first main mutex, second error lock mutex)
    // but we drop main mutex lock for waiting on the cond var.
    d->lock->mutex.unlock();
}

BdEngineBackendPrivate::AbstractUnlocker::~AbstractUnlocker()
{
    // lock main mutex as often as it was locked before
    for (int i=0; i<count; ++i)
    {
        d->lock->mutex.lock();
    }

    // update lock count
    d->lock->lockCount += count;
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::AbstractWaitingUnlocker::AbstractWaitingUnlocker(BdEngineBackendPrivate* const d,
                                                                         QMutex* const mutex,
                                                                         QWaitCondition* const condVar)
    : AbstractUnlocker(d),
      mutex(mutex),
      condVar(condVar)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.
    // lock condvar mutex (lock only if main mutex is locked)
    mutex->lock();

    finishAcquire();
}

BdEngineBackendPrivate::AbstractWaitingUnlocker::~AbstractWaitingUnlocker()
{
    // unlock condvar mutex. Both mutexes are now free.
    mutex->unlock();
    // now base class destructor is executed, reallocating main mutex
}

bool BdEngineBackendPrivate::AbstractWaitingUnlocker::wait(unsigned long time)
{
    return condVar->wait(mutex, time);
}

// -----------------------------------------------------------------------------------------

/** This suspends the current thread if the query status as
 *  set by setFlag() is Wait and until the thread is woken with wakeAll().
 *  The CoreDbAccess mutex will be unlocked while waiting.
 */
void BdEngineBackendPrivate::ErrorLocker::wait()
{
    // we use a copy of the flag under lock of the errorLockMutex to be able to check it here
    while (d->errorLockOperationStatus == BdEngineBackend::Wait)
    {
        wait();
    }
}

// -----------------------------------------------------------------------------------------

BdEngineBackend::BdEngineBackend(const QString& backendName, DbEngineLocking* const locking)
    : d_ptr(new BdEngineBackendPrivate(this))
{
    d_ptr->init(backendName, locking);
}

BdEngineBackend::BdEngineBackend(const QString& backendName, DbEngineLocking* const locking, BdEngineBackendPrivate& dd)
    : d_ptr(&dd)
{
    d_ptr->init(backendName, locking);
}

BdEngineBackend::~BdEngineBackend()
{
    Q_D(BdEngineBackend);
    close();

    delete d;
}

DbEngineConfigSettings BdEngineBackend::configElement() const
{
    Q_D(const BdEngineBackend);
    return DbEngineConfig::element(d->parameters.databaseType);
}

DbEngineAction BdEngineBackend::getDBAction(const QString& actionName) const
{
    Q_D(const BdEngineBackend);
    DbEngineAction action = configElement().sqlStatements.value(actionName);

    if (action.name.isNull())
    {
        qCWarning(DIGIKAM_DBENGINE_LOG) << "No DB action defined for" << actionName
                                        << "! Implementation missing for this database type ("
                                        << d->parameters.databaseType << ").";
    }

    return action;
}

BdEngineBackend::DbType BdEngineBackend::databaseType() const
{
    Q_D(const BdEngineBackend);
    return d->parameters.isSQLite() ? DbType::SQLite : DbType::MySQL;
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const DbEngineAction& action, QList<QVariant>* const values,
                                                          QVariant* const lastInsertId)
{
    return execDBAction(action, QMap<QString, QVariant>(), values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const QString& action, QList<QVariant>* const values,
                                                          QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), QMap<QString, QVariant>(), values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const QString& action, const QMap<QString, QVariant>& bindingMap,
                                                          QList<QVariant>* const values, QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), bindingMap, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const DbEngineAction& action, const QMap<QString, QVariant>& bindingMap,
                                                          QList<QVariant>* const values, QVariant* const lastInsertId)
{
    Q_D(BdEngineBackend);

    BdEngineBackend::QueryState returnResult = BdEngineBackend::NoErrors;
    QSqlDatabase db                          = d->databaseForThread();

    if (action.name.isNull())
    {
        qCWarning(DIGIKAM_DBENGINE_LOG) << "Attempt to execute null action";
        return BdEngineBackend::SQLError;
    }

    //qCDebug(DIGIKAM_DBENGINE_LOG) << "Executing DBAction ["<<  action.name  <<"]";

    bool wrapInTransaction = (action.mode == QLatin1String("transaction"));

    if (wrapInTransaction)
    {
        beginTransaction();
    }

    foreach(const DbEngineActionElement& actionElement, action.dbActionElements)
    {
        BdEngineBackend::QueryState result;

        if (actionElement.mode == QLatin1String("query"))
        {
            result = execSql(actionElement.statement, bindingMap, values, lastInsertId);
        }
        else if (actionElement.mode == QLatin1String("unprepared"))
        {
            result = execDirectSqlWithResult(actionElement.statement, values, lastInsertId);
        }
        else
        {
            result = execDirectSql(actionElement.statement);
        }

        if (result != BdEngineBackend::NoErrors)
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while executing DBAction ["
                                          << action.name << "] Statement ["
                                          << actionElement.statement << "]";
            returnResult = result;

/*
            if (wrapInTransaction && !db.rollback())
            {
                qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while rollback changes of previous DBAction.";
            }
*/

            break;
        }
    }

    if (wrapInTransaction)
    {
        commitTransaction();
    }

/*
    if (returnResult == BdEngineBackend::NoErrors && wrapInTransaction && !db.commit())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while committing changes of previous DBAction.";
    }
*/

    return returnResult;
}

QSqlQuery BdEngineBackend::execDBActionQuery(const QString& action, const QMap<QString, QVariant>& bindingMap)
{
    return execDBActionQuery(getDBAction(action), bindingMap);
}

QSqlQuery BdEngineBackend::execDBActionQuery(const DbEngineAction& action, const QMap<QString, QVariant>& bindingMap)
{
    Q_D(BdEngineBackend);

    QSqlDatabase db = d->databaseForThread();

//    qCDebug(DIGIKAM_DBENGINE_LOG) << "Executing DBAction ["<<  action.name  <<"]";

    QSqlQuery result;

    foreach(const DbEngineActionElement& actionElement, action.dbActionElements)
    {
        if (actionElement.mode == QLatin1String("query"))
        {
            result = execQuery(actionElement.statement, bindingMap);
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error, only DBActions with mode 'query' are allowed at this call!";
        }

        if (result.lastError().isValid() && result.lastError().number())
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while executing DBAction [" <<  action.name
                                          << "] Statement [" << actionElement.statement
                                          << "] Errornr. [" << result.lastError() << "]";
            break;
        }
    }

    return result;
}

void BdEngineBackend::setDbEngineErrorHandler(DbEngineErrorHandler* const handler)
{
    Q_D(BdEngineBackend);

    delete d->errorHandler;

    d->errorHandler = handler;
}

bool BdEngineBackend::isCompatible(const DbEngineParameters& parameters)
{
    return QSqlDatabase::drivers().contains(parameters.databaseType);
}

bool BdEngineBackend::open(const DbEngineParameters& parameters)
{
    Q_D(BdEngineBackend);
    d->parameters = parameters;
    // This will make possibly opened thread dbs reload at next access
    d->currentValidity++;

    int retries = 0;

    forever
    {
        QSqlDatabase database = d->databaseForThread();

        if (!database.isOpen())
        {
            //qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while opening the database. Trying again.";

            if (connectionErrorHandling(retries++))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        else
        {
            break;
        }
    }

    d->status = Open;
    return true;
}

void BdEngineBackend::close()
{
    Q_D(BdEngineBackend);
    d->closeDatabaseForThread();
    d->status = Unavailable;
}

BdEngineBackend::Status BdEngineBackend::status() const
{
    Q_D(const BdEngineBackend);
    return d->status;
}

/*
bool BdEngineBackend::execSql(const QString& sql, QStringList* const values)
{
    QSqlQuery query = execQuery(sql);

    if (!query.isActive())
        return false;

    if (!values)
        return true;

    int count = query.record().count();

    while (query.next())
    {
        for (int i=0; i<count; ++i)
            (*values) << query.value(i).toString();
    }

    return true;
}
*/

QList<QVariant> BdEngineBackend::readToList(DbEngineSqlQuery& query)
{
    QList<QVariant> list;

    QSqlRecord record = query.record();
    int count         = record.count();

    while (query.next())
    {
        for (int i=0; i<count; ++i)
        {
            list << query.value(i);
        }
    }

//    qCDebug(DIGIKAM_DBENGINE_LOG) << "Setting result value list ["<< list <<"]";

    return list;
}

BdEngineBackend::QueryState BdEngineBackend::handleQueryResult(DbEngineSqlQuery& query, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    if (!query.isActive())
    {
        if (query.lastError().type() == QSqlError::ConnectionError)
        {
            return BdEngineBackend::ConnectionError;
        }
        else
        {
            return BdEngineBackend::SQLError;
        }
    }

    if (lastInsertId)
    {
        (*lastInsertId) = query.lastInsertId();
    }

    if (values)
    {
        (*values) = readToList(query);
    }

    return BdEngineBackend::NoErrors;
}

// -------------------------------------------------------------------------------------

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql);
    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql, const QVariant& boundValue1,
                                                     QList<QVariant>* const values, QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql, boundValue1);
    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1, const QVariant& boundValue2,
                                                     QList<QVariant>* const values, QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql, boundValue1, boundValue2);
    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1, const QVariant& boundValue2,
                                                     const QVariant& boundValue3, QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1, const QVariant& boundValue2,
                                                     const QVariant& boundValue3, const QVariant& boundValue4,
                                                     QList<QVariant>* const values, QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql, const QList<QVariant>& boundValues,
                                                     QList<QVariant>* const values, QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql, boundValues);
    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql, const QMap<QString, QVariant>& bindingMap,
                                                     QList<QVariant>* const values, QVariant* const lastInsertId)
{
    DbEngineSqlQuery query = execQuery(sql, bindingMap);
    return handleQueryResult(query, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

BdEngineBackend::QueryState BdEngineBackend::execSql(DbEngineSqlQuery& preparedQuery, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    exec(preparedQuery);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(DbEngineSqlQuery& preparedQuery, const QVariant& boundValue1,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(DbEngineSqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(DbEngineSqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        const QVariant& boundValue3, QList<QVariant>* const values,
        QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(DbEngineSqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        const QVariant& boundValue3, const QVariant& boundValue4,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(DbEngineSqlQuery& preparedQuery, const QList<QVariant>& boundValues,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValues);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql, const QVariant& boundValue1)
{
    DbEngineSqlQuery query = prepareQuery(sql);

//    qCDebug(DIGIKAM_DBENGINE_LOG) << "Trying to sql ["<< sql <<"] query ["<<query.lastQuery()<<"]";

    execQuery(query, boundValue1);
    return query;
}

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2)
{
    DbEngineSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2);
    return query;
}

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3)
{
    DbEngineSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3);
    return query;
}

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2,
                                        const QVariant& boundValue3, const QVariant& boundValue4)
{
    DbEngineSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3, boundValue4);
    return query;
}

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql, const QList<QVariant>& boundValues)
{
    DbEngineSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValues);
    return query;
}

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql)
{
    DbEngineSqlQuery query = prepareQuery(sql);

//    qCDebug(DIGIKAM_DBENGINE_LOG)<<"execQuery: Using statement ["<< query.lastQuery() <<"]";

    exec(query);
    return query;
}

// -------------------------------------------------------------------------------------

void BdEngineBackend::execQuery(DbEngineSqlQuery& query, const QVariant& boundValue1)
{
    query.bindValue(0, boundValue1);
    exec(query);
}

void BdEngineBackend::execQuery(DbEngineSqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    exec(query);
}

void BdEngineBackend::execQuery(DbEngineSqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2,
                                    const QVariant& boundValue3)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    exec(query);
}

void BdEngineBackend::execQuery(DbEngineSqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2,
                                    const QVariant& boundValue3, const QVariant& boundValue4)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    exec(query);
}

void BdEngineBackend::execQuery(DbEngineSqlQuery& query, const QList<QVariant>& boundValues)
{
    for (int i=0; i<boundValues.size(); ++i)
    {
        query.bindValue(i, boundValues.at(i));
    }

    exec(query);
}

// -------------------------------------------------------------------------------------

DbEngineSqlQuery BdEngineBackend::execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap)
{
    QString preparedString = sql;
    QList<QVariant> valuesToBind;

    if (!bindingMap.isEmpty())
    {
//        qCDebug(DIGIKAM_DBENGINE_LOG) << "Prepare statement [" << preparedString << "] with binding map [" << bindingMap << "]";

        QRegExp identifierRegExp(QLatin1String(":[A-Za-z0-9]+"));
        int pos = 0;

        while ( (pos=identifierRegExp.indexIn(preparedString, pos)) != -1)
        {
            QString namedPlaceholder = identifierRegExp.cap(0);

            if (!bindingMap.contains(namedPlaceholder))
            {
                qCWarning(DIGIKAM_DBENGINE_LOG) << "Missing place holder" << namedPlaceholder
                                                << "in binding map. The following values are defined for this action:"
                                                << bindingMap.keys() <<". This is a setup error!";

                //TODO What should we do here? How can we cancel that action?
            }

            QVariant placeHolderValue = bindingMap.value(namedPlaceholder);
            QString replaceStr;

            if (placeHolderValue.userType() == qMetaTypeId<DbEngineActionType>())
            {
                DbEngineActionType actionType = placeHolderValue.value<DbEngineActionType>();
                bool isValue                  = actionType.isValue();
                QVariant value                = actionType.getActionValue();

                if ( value.type() == QVariant::Map )
                {
                    QMap<QString, QVariant> placeHolderMap = value.toMap();
                    QMap<QString, QVariant>::const_iterator iterator;

                    for (iterator = placeHolderMap.constBegin(); iterator != placeHolderMap.constEnd(); ++iterator)
                    {
                        const QString& key    = iterator.key();
                        const QVariant& value = iterator.value();
                        replaceStr.append(key);
                        replaceStr.append(QLatin1String("= ?"));
                        valuesToBind.append(value);

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderMap.constEnd())
                        {
                            replaceStr.append(QLatin1String(", "));
                        }
                    }
                }
                else if ( value.type() == QVariant::List )
                {
                    QList<QVariant> placeHolderList = value.toList();
                    QList<QVariant>::const_iterator iterator;

                    for (iterator = placeHolderList.constBegin(); iterator != placeHolderList.constEnd(); ++iterator)
                    {
                        const QVariant& entry = *iterator;

                        if (isValue)
                        {
                            replaceStr.append(QLatin1String("?"));
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry.value<QString>());
                        }

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(QLatin1String(", "));
                        }
                    }
                }
                else if (value.type() == QVariant::StringList )
                {
                    QStringList placeHolderList = value.toStringList();
                    QStringList::const_iterator iterator;

                    for (iterator = placeHolderList.constBegin(); iterator != placeHolderList.constEnd(); ++iterator)
                    {
                        const QString& entry = *iterator;

                        if (isValue)
                        {
                            replaceStr.append(QLatin1String("?"));
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry);
                        }

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(QLatin1String(", "));
                        }
                    }
                }
                else
                {
                    if (isValue)
                    {
                        replaceStr = QLatin1Char('?');
                        valuesToBind.append(value);
                    }
                    else
                    {
                        replaceStr = value.toString();
                    }
                }
            }
            else
            {

//                qCDebug(DIGIKAM_DBENGINE_LOG) << "Bind key ["<< namedPlaceholder << "] to value [" << bindingMap[namedPlaceholder] << "]";

                valuesToBind.append(placeHolderValue);
                replaceStr = QLatin1Char('?');
            }

            preparedString = preparedString.replace(pos, identifierRegExp.matchedLength(), replaceStr);
            pos=0; // reset pos
        }
    }

//    qCDebug(DIGIKAM_DBENGINE_LOG) << "Prepared statement [" << preparedString << "] values [" << valuesToBind << "]";

    DbEngineSqlQuery query = prepareQuery(preparedString);

    for (int i=0; i < valuesToBind.size(); ++i)
    {
        query.bindValue(i, valuesToBind.at(i));
    }

    exec(query);
    return query;
}

BdEngineBackend::QueryState BdEngineBackend::execUpsertDBAction(const DbEngineAction& action, const QVariant& id,
                                                                        const QStringList fieldNames, const QList<QVariant>& values)
{
    QMap<QString, QVariant> parameters;
    QMap<QString, QVariant> fieldValueMap;

    for (int i = 0; i < fieldNames.size(); ++i)
    {
        fieldValueMap.insert(fieldNames.at(i), values.at(i));
    }

    DbEngineActionType fieldValueList = DbEngineActionType::value(fieldValueMap);
    DbEngineActionType fieldList      = DbEngineActionType::fieldEntry(fieldNames);
    DbEngineActionType valueList      = DbEngineActionType::value(values);

    parameters.insert(QLatin1String(":id"),             id);
    parameters.insert(QLatin1String(":fieldValueList"), qVariantFromValue(fieldValueList));
    parameters.insert(QLatin1String(":fieldList"),      qVariantFromValue(fieldList));
    parameters.insert(QLatin1String(":valueList"),      qVariantFromValue(valueList));

    return execDBAction(action, parameters);
}

BdEngineBackend::QueryState BdEngineBackend::execUpsertDBAction(const QString& action, const QVariant& id,
                                                                        const QStringList fieldNames, const QList<QVariant>& values)
{
    return execUpsertDBAction(getDBAction(action), id, fieldNames, values);
}

bool BdEngineBackend::connectionErrorHandling(int /*retries*/)
{
    Q_D(BdEngineBackend);

    if (d->reconnectOnError())
    {
        if (d->handleWithErrorHandler(0))
        {
            d->closeDatabaseForThread();
            return true;
        }
    }

    return false;
}

bool BdEngineBackend::queryErrorHandling(DbEngineSqlQuery& query, int retries)
{
    Q_D(BdEngineBackend);

    if (d->isSQLiteLockError(query))
    {
        if (d->checkRetrySQLiteLockError(retries))
        {
            return true;
        }
    }

    d->debugOutputFailedQuery(query);

    /*
     * Check if the error is query or database related.
     * It seems, that insufficient privileges results only in query errors,
     * the database gives an invalid lastError() value.
     */
    if (query.lastError().isValid())
    {
        d->setDatabaseErrorForThread(query.lastError());
    }
    else
    {
        d->setDatabaseErrorForThread(d->databaseForThread().lastError());
    }

    if (d->isConnectionError(query) && d->reconnectOnError())
    {
        // after connection errors, it can be required
        // to start with a new connection and a fresh, copied query
        d->closeDatabaseForThread();
        query = copyQuery(query);
    }

    if (d->needToHandleWithErrorHandler(query))
    {
        if (d->handleWithErrorHandler(&query))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool BdEngineBackend::transactionErrorHandling(const QSqlError& lastError, int retries)
{
    Q_D(BdEngineBackend);

    if (d->isSQLiteLockTransactionError(lastError))
    {
        if (d->checkRetrySQLiteLockError(retries))
        {
            return true;
        }
    }

    d->debugOutputFailedTransaction(lastError);

    // no experience with other forms of failure

    return false;
}

BdEngineBackend::QueryState BdEngineBackend::execDirectSql(const QString& sql)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return BdEngineBackend::SQLError;
    }

    DbEngineSqlQuery query = getQuery();
    int retries    = 0;

    forever
    {
        if (query.exec(sql))
        {
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                continue;
            }
            else
            {
                return BdEngineBackend::SQLError;
            }
        }
    }

    return BdEngineBackend::NoErrors;
}

BdEngineBackend::QueryState BdEngineBackend::execDirectSqlWithResult(const QString& sql, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return BdEngineBackend::SQLError;
    }

    DbEngineSqlQuery query = getQuery();
    int retries    = 0;

    forever
    {
        if (query.exec(sql))
        {
            handleQueryResult(query, values, lastInsertId);
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                continue;
            }
            else
            {
                return BdEngineBackend::SQLError;
            }
        }
    }

    return BdEngineBackend::NoErrors;
}

bool BdEngineBackend::exec(DbEngineSqlQuery& query)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return false;
    }

    int retries = 0;

    forever
    {
//        qCDebug(DIGIKAM_DBENGINE_LOG) << "Trying to query [" << query.lastQuery() << "] values [" << query.boundValues() << "]";

        if (query.exec())
        {
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

bool BdEngineBackend::execBatch(DbEngineSqlQuery& query)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return false;
    }

    int retries = 0;

    forever
    {
        if (query.execBatch())
        {
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

DbEngineSqlQuery BdEngineBackend::prepareQuery(const QString& sql)
{
    int retries = 0;

    forever
    {
        DbEngineSqlQuery query = getQuery();

        if (query.prepare(sql))
        {
            return query;
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Prepare failed!";

            if (queryErrorHandling(query, retries++))
            {
                continue;
            }
            else
            {
                return query;
            }
        }
    }
}

DbEngineSqlQuery BdEngineBackend::copyQuery(const DbEngineSqlQuery& old)
{
    DbEngineSqlQuery query = getQuery();

//    qCDebug(DIGIKAM_DBENGINE_LOG) << "Last query was [" << old.lastQuery() << "]";

    query.prepare(old.lastQuery());
    query.setForwardOnly(old.isForwardOnly());

    // only for positional binding
    QList<QVariant> boundValues = old.boundValues().values();

    foreach(const QVariant& value, boundValues)
    {
//        qCDebug(DIGIKAM_DBENGINE_LOG) << "Bind value to query ["<<value<<"]";

        query.addBindValue(value);
    }

    return query;
}

DbEngineSqlQuery BdEngineBackend::getQuery()
{
    Q_D(BdEngineBackend);
    QSqlDatabase db = d->databaseForThread();

    DbEngineSqlQuery query(db);
    query.setForwardOnly(true);
    return query;
}

BdEngineBackend::QueryState BdEngineBackend::beginTransaction()
{
    Q_D(BdEngineBackend);

    // Call databaseForThread before touching transaction count - open() will reset the count!
    QSqlDatabase db = d->databaseForThread();

    if (d->incrementTransactionCount())
    {
        int retries = 0;

        forever
        {
            if (db.transaction())
            {
                break;
            }
            else
            {
                if (transactionErrorHandling(db.lastError(), retries++))
                {
                    continue;
                }
                else
                {
                    d->decrementTransactionCount();

                    if (db.lastError().type() == QSqlError::ConnectionError)
                    {
                        return BdEngineBackend::ConnectionError;
                    }
                    else
                    {
                        return BdEngineBackend::SQLError;
                    }
                }
            }
        }

        d->isInTransaction = true;
    }

    return BdEngineBackend::NoErrors;
}

BdEngineBackend::QueryState BdEngineBackend::commitTransaction()
{
    Q_D(BdEngineBackend);

    if (d->decrementTransactionCount())
    {
        QSqlDatabase db = d->databaseForThread();
        int retries     = 0;

        forever
        {
            if (db.commit())
            {
                break;
            }
            else
            {
                QSqlError lastError = db.lastError();

                if (transactionErrorHandling(lastError, retries++))
                {
                    continue;
                }
                else
                {
                    qCDebug(DIGIKAM_DBENGINE_LOG) << "Failed to commit transaction. Starting rollback.";
                    db.rollback();

                    if (lastError.type() == QSqlError::ConnectionError)
                    {
                        return BdEngineBackend::ConnectionError;
                    }
                    else
                    {
                        return BdEngineBackend::SQLError;
                    }
                }
            }
        }

        d->isInTransaction = false;
        d->transactionFinished();
    }

    return BdEngineBackend::NoErrors;
}

bool BdEngineBackend::isInTransaction() const
{
    Q_D(const BdEngineBackend);
    return d->isInTransaction;
}

void BdEngineBackend::rollbackTransaction()
{
    Q_D(BdEngineBackend);
    // we leave that out for transaction counting. It's an exceptional condition.
    d->databaseForThread().rollback();
}

QStringList BdEngineBackend::tables()
{
    Q_D(BdEngineBackend);
    return d->databaseForThread().tables();
}

QSqlError BdEngineBackend::lastSQLError()
{
    Q_D(BdEngineBackend);
    return d->databaseErrorForThread();
}

QString BdEngineBackend::lastError()
{
    Q_D(BdEngineBackend);
    return d->databaseForThread().lastError().text();
}

int BdEngineBackend::maximumBoundValues() const
{
    Q_D(const BdEngineBackend);

    if (d->parameters.isSQLite())
    {
        return 999;   // SQLITE_MAX_VARIABLE_NUMBER
    }
    else
    {
        return 65535; // MySQL
    }
}

void BdEngineBackend::setForeignKeyChecks(bool check)
{
    Q_D(BdEngineBackend);

    if (d->parameters.isMySQL())
    {
        if (check)
        {
            execSql(QLatin1String("SET FOREIGN_KEY_CHECKS=1;"));
        }
        else
        {
            execSql(QLatin1String("SET FOREIGN_KEY_CHECKS=0;"));
        }
    }
}

}  // namespace Digikam
