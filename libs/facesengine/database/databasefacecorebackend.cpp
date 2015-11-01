/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Abstract database backend
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databasefacecorebackend.h"
#include "databasefacecorebackend_p.h"

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QRegExp>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QThread>

// Local includes

#include "digikam_debug.h"
#include "dbactiontype.h"
#include "databasefaceschemaupdater.h"

namespace FacesEngine
{

DatabaseLocking::DatabaseLocking()
    : mutex(QMutex::Recursive),
      lockCount(0) // create a recursive mutex
{
}

// -----------------------------------------------------------------------------------------

// For whatever reason, these methods are "static protected"
class sotoSleep : public QThread
{
public:

    static void sleep(unsigned long secs)
    {
        QThread::sleep(secs);
    }

    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }

    static void usleep(unsigned long usecs)
    {
        QThread::usleep(usecs);
    }
};

// -----------------------------------------------------------------------------------------

DatabaseFaceCoreBackendPrivate::BusyWaiter::BusyWaiter(DatabaseFaceCoreBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->busyWaitMutex, &d->busyWaitCondVar)
{
}

// -----------------------------------------------------------------------------------------

DatabaseFaceCoreBackendPrivate::ErrorLocker::ErrorLocker(DatabaseFaceCoreBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->errorLockMutex, &d->errorLockCondVar)
{
}

// -----------------------------------------------------------------------------------------


DatabaseThreadData::DatabaseThreadData()
    : valid(0),
      transactionCount(0)
{
}

DatabaseThreadData::~DatabaseThreadData()
{
    if (transactionCount)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "WARNING !!! Transaction count is" << transactionCount << "when destroying database!!!";
    }
    closeDatabase();
}

void DatabaseThreadData::closeDatabase()
{
    QString connectionToRemove;
    if (database.isOpen())
    {
        connectionToRemove = database.connectionName();
    }

    // Destroy object
    database = QSqlDatabase();

    valid            = 0;
    transactionCount = 0;
    lastError        = QSqlError();

    // Remove connection
    if (!connectionToRemove.isNull())
    {
        QSqlDatabase::removeDatabase(connectionToRemove);
    }
}

DatabaseFaceCoreBackendPrivate::DatabaseFaceCoreBackendPrivate(DatabaseFaceCoreBackend* const backend)
    : currentValidity(0),
      isInTransaction(false),
      status(DatabaseFaceCoreBackend::Unavailable),
      lock(0),
      operationStatus(DatabaseFaceCoreBackend::ExecuteNormal),
      errorLockOperationStatus(DatabaseFaceCoreBackend::ExecuteNormal),
      errorHandler(0),
      q(backend)
{
}

DatabaseFaceCoreBackendPrivate::~DatabaseFaceCoreBackendPrivate()
{
    // Must be shut down from the main thread.
    // Clean up the QThreadStorage. It deletes any stored data.
    threadDataStorage.setLocalData(0);
}

void DatabaseFaceCoreBackendPrivate::init(const QString& name, DatabaseLocking* const l)
{
    backendName = name;
    lock        = l;
}

// "A connection can only be used from within the thread that created it.
//  Moving connections between threads or creating queries from a different thread is not supported."
// => one QSqlDatabase object per thread.
// The main class' open/close methods only interact with the "firstDatabase" object.
// When another thread requests a DB, a new connection is opened and closed at
// finishing of the thread.
QSqlDatabase DatabaseFaceCoreBackendPrivate::databaseForThread()
{
    DatabaseThreadData* threadData = 0;
    if (!threadDataStorage.hasLocalData())
    {
        threadData = new DatabaseThreadData;
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
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error while opening the database. Error was" << threadData->database.lastError();
        }
    }

    return threadData->database;
}

QSqlDatabase DatabaseFaceCoreBackendPrivate::createDatabaseConnection()
{
    QSqlDatabase db        = QSqlDatabase::addDatabase(parameters.databaseType, connectionName());
    QString connectOptions = parameters.connectOptions;

    if (parameters.isSQLite())
    {
        QStringList toAdd;
        // enable shared cache, especially useful with SQLite >= 3.5.0
        toAdd << QString::fromLatin1("QSQLITE_ENABLE_SHARED_CACHE");
        // We do our own waiting.
        toAdd << QString::fromLatin1("QSQLITE_BUSY_TIMEOUT=0");

        if (!connectOptions.isEmpty())
        {
            connectOptions += QString::fromLatin1(";");
        }

        connectOptions += toAdd.join(QString::fromLatin1(";"));
    }

    db.setDatabaseName(parameters.databaseName);
    db.setConnectOptions(connectOptions);

    return db;
}

void DatabaseFaceCoreBackendPrivate::closeDatabaseForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->closeDatabase();
    }
}

QSqlError DatabaseFaceCoreBackendPrivate::databaseErrorForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        return threadDataStorage.localData()->lastError;
    }
    return QSqlError();
}

void DatabaseFaceCoreBackendPrivate::setDatabaseErrorForThread(const QSqlError& lastError)
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->lastError = lastError;
    }
}

QString DatabaseFaceCoreBackendPrivate::connectionName()
{
    return backendName + QString::number((quintptr)QThread::currentThread());
}

bool DatabaseFaceCoreBackendPrivate::incrementTransactionCount()
{
    return (!threadDataStorage.localData()->transactionCount++);
}

bool DatabaseFaceCoreBackendPrivate::decrementTransactionCount()
{
    return (!--threadDataStorage.localData()->transactionCount);
}

bool DatabaseFaceCoreBackendPrivate::isInMainThread() const
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

bool DatabaseFaceCoreBackendPrivate::isInUIThread() const
{
    QApplication* const app = qobject_cast<QApplication*>(QCoreApplication::instance());

    if (!app)
    {
        return false;
    }

    return (QThread::currentThread() == app->thread());
}

bool DatabaseFaceCoreBackendPrivate::reconnectOnError() const
{
    return parameters.isMySQL();
}

bool DatabaseFaceCoreBackendPrivate::isSQLiteLocqCritical(const SqlQuery& query) const
{
    return parameters.isSQLite() &&
           (query.lastError().number() == 5 /*SQLITE_BUSY*/ || query.lastError().number() == 6/*SQLITE_LOCKED*/);
}

bool DatabaseFaceCoreBackendPrivate::isSQLiteLockTransactionError(const QSqlError& lastError) const
{
    return parameters.isSQLite() &&
           lastError.type()         == QSqlError::TransactionError &&
           lastError.databaseText() == QLatin1String("database is locked");
    // wouldnt it be great if they gave us the database error number...
}

bool DatabaseFaceCoreBackendPrivate::isConnectionError(const SqlQuery& query) const
{
    // the backend returns connection error e.g. for Constraint Failed errors.
    if (parameters.isSQLite())
    {
        return false;
    }

    return query.lastError().type()   == QSqlError::ConnectionError ||
           query.lastError().number() == 2006;
}

bool DatabaseFaceCoreBackendPrivate::needToConsultUserForError(const SqlQuery&) const
{
    // no such conditions found and defined as yet
    return false;
}

bool DatabaseFaceCoreBackendPrivate::needToHandleWithErrorHandler(const SqlQuery& query) const
{
    return (isConnectionError(query) || needToConsultUserForError(query));
}

bool DatabaseFaceCoreBackendPrivate::checkRetrySQLiteLocqCritical(int retries)
{
    if (!(retries % 25))
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Database is locked. Waited" << retries*10;
    }

    const int uiMaxRetries = 50;
    const int maxRetries   = 1000;

    if (retries > qMax(uiMaxRetries, maxRetries))
    {
        if (retries > (isInUIThread() ? uiMaxRetries : maxRetries))
        {
            qCWarning(DIGIKAM_FACESENGINE_LOG) << "Detected locked database file. There is an active transaction. Waited but giving up now.";
            return false;
        }
    }

    BusyWaiter waiter(this);
    waiter.wait(10);
    return true;
}

void DatabaseFaceCoreBackendPrivate::debugOutputFailedQuery(const QSqlQuery& query) const
{
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Failure executing query:\n"
             << query.executedQuery()
             << "\nError messages:" << query.lastError().driverText() << query.lastError().databaseText()
             << query.lastError().number() << query.lastError().type()
             << "\nBound values: " << query.boundValues().values();
}

void DatabaseFaceCoreBackendPrivate::debugOutputFailedTransaction(const QSqlError& error) const
{
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Failure executing transaction. Error messages:\n"
             << error.driverText() << error.databaseText()
             << error.number() << error.type();
}

void DatabaseFaceCoreBackendPrivate::transactionFinished()
{
    // wakes up any BusyWaiter waiting on the busyWaitCondVar.
    // Possibly called under d->lock->mutex lock, so we do not lock the busyWaitMutex
    busyWaitCondVar.wakeOne();
}

/** Set the wait flag to queryStatus. Typically, call this with Wait. */
void DatabaseFaceCoreBackendPrivate::setQueryOperationFlag(DatabaseFaceCoreBackend::QueryOperationStatus status)
{
    // Enforce lock order (first main mutex, second error lock mutex)
    QMutexLocker l(&errorLockMutex);
    // this change must be done under errorLockMutex lock
    errorLockOperationStatus = status;
    operationStatus          = status;
}

/** Set the wait flag to queryStatus and wake all waiting threads.
 *  Typically, call wakeAll with status ExecuteNormal or AbortQueries. */
void DatabaseFaceCoreBackendPrivate::queryOperationWakeAll(DatabaseFaceCoreBackend::QueryOperationStatus status)
{
    QMutexLocker l(&errorLockMutex);
    operationStatus          = status;
    errorLockOperationStatus = status;
    errorLockCondVar.wakeAll();
}

bool DatabaseFaceCoreBackendPrivate::checkOperationStatus()
{
    while (operationStatus == DatabaseFaceCoreBackend::Wait)
    {
        ErrorLocker locker(this);
        locker.wait();
    }

    if (operationStatus == DatabaseFaceCoreBackend::ExecuteNormal)
    {
        return true;
    }
    else if (operationStatus == DatabaseFaceCoreBackend::AbortQueries)
    {
        return false;
    }

    return false;
}

/// Returns true if the query shall be retried
bool DatabaseFaceCoreBackendPrivate::handleWithErrorHandler(const SqlQuery* const query)
{
    if (errorHandler)
    {
        setQueryOperationFlag(DatabaseFaceCoreBackend::Wait);

        ErrorLocker locker(this);
        bool called         = false;
        QSqlError lastError = query ? query->lastError() : databaseForThread().lastError();
        QString lastQuery   = query ? query->lastQuery() : QString();

        if (!query || isConnectionError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "connectionError", Qt::AutoConnection,
                                               Q_ARG(Digikam::DatabaseErrorAnswer*, this), Q_ARG(const QSqlError, lastError),
                                               Q_ARG(const QString, lastQuery));
        }
        else if (needToConsultUserForError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "consultUserForError", Qt::AutoConnection,
                                               Q_ARG(Digikam::DatabaseErrorAnswer*, this), Q_ARG(const QSqlError, lastError),
                                               Q_ARG(const QString, lastQuery));
        }
        else
        {
            // unclear what to do.
            errorLockOperationStatus = DatabaseFaceCoreBackend::ExecuteNormal;
            operationStatus          = DatabaseFaceCoreBackend::ExecuteNormal;
            return true;
        }

        if (called)
        {
            locker.wait();
        }
        else
        {
            qCWarning(DIGIKAM_FACESENGINE_LOG) << "Failed to invoke DatabaseErrorHandler. Aborting all queries.";
            operationStatus = DatabaseFaceCoreBackend::AbortQueries;
        }

        switch (operationStatus)
        {
            case DatabaseFaceCoreBackend::ExecuteNormal:
            case DatabaseFaceCoreBackend::Wait:
                return true;
            case DatabaseFaceCoreBackend::AbortQueries:
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

void DatabaseFaceCoreBackendPrivate::connectionErrorContinueQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(DatabaseFaceCoreBackend::ExecuteNormal);
}

void DatabaseFaceCoreBackendPrivate::connectionErrorAbortQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(DatabaseFaceCoreBackend::AbortQueries);
}

// -----------------------------------------------------------------------------------------

DatabaseFaceCoreBackendPrivate::AbstractUnlocker::AbstractUnlocker(DatabaseFaceCoreBackendPrivate* const d)
    : count(0), d(d)
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

void DatabaseFaceCoreBackendPrivate::AbstractUnlocker::finishAcquire()
{
    // drop lock acquired in first line. Main mutex is now free.
    // We maintain lock order (first main mutex, second error lock mutex)
    // but we drop main mutex lock for waiting on the cond var.
    d->lock->mutex.unlock();
}

DatabaseFaceCoreBackendPrivate::AbstractUnlocker::~AbstractUnlocker()
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

DatabaseFaceCoreBackendPrivate::AbstractWaitingUnlocker::AbstractWaitingUnlocker(DatabaseFaceCoreBackendPrivate* const d,
        QMutex* const mutex, QWaitCondition* const condVar)
    : AbstractUnlocker(d), mutex(mutex), condVar(condVar)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.
    // lock condvar mutex (lock only if main mutex is locked)
    mutex->lock();

    finishAcquire();
}

DatabaseFaceCoreBackendPrivate::AbstractWaitingUnlocker::~AbstractWaitingUnlocker()
{
    // unlock condvar mutex. Both mutexes are now free.
    mutex->unlock();
    // now base class destructor is executed, reallocating main mutex
}

bool DatabaseFaceCoreBackendPrivate::AbstractWaitingUnlocker::wait(unsigned long time)
{
    return condVar->wait(mutex, time);
}

// -----------------------------------------------------------------------------------------

/** This suspends the current thread if the query status as
 *  set by setFlag() is Wait and until the thread is woken with wakeAll().
 *  The database access mutex will be unlocked while waiting.
 */
void DatabaseFaceCoreBackendPrivate::ErrorLocker::wait()
{
    // we use a copy of the flag under lock of the errorLockMutex to be able to check it here
    while (d->errorLockOperationStatus == DatabaseFaceCoreBackend::Wait)
    {
        wait();
    }
}

// -----------------------------------------------------------------------------------------

DatabaseFaceCoreBackend::DatabaseFaceCoreBackend(const QString& backendName, DatabaseLocking* const locking)
    : d_ptr(new DatabaseFaceCoreBackendPrivate(this))
{
    d_ptr->init(backendName, locking);
}

DatabaseFaceCoreBackend::DatabaseFaceCoreBackend(const QString& backendName, DatabaseLocking* const locking, DatabaseFaceCoreBackendPrivate& dd)
    : d_ptr(&dd)
{
    d_ptr->init(backendName, locking);
}

DatabaseFaceCoreBackend::~DatabaseFaceCoreBackend()
{
    Q_D(DatabaseFaceCoreBackend);
    close();

    delete d;
}

DatabaseConfigElement DatabaseFaceCoreBackend::configElement() const
{
    Q_D(const DatabaseFaceCoreBackend);
    return DatabaseConfigElement::element(d->parameters.databaseType);
}

DatabaseAction DatabaseFaceCoreBackend::getDBAction(const QString& actionName) const
{
    DatabaseAction action = configElement().sqlStatements.value(actionName);

    if (action.name.isNull())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No DB action defined for" << actionName << "! Implementation missing for this database type.";
    }

    return action;
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execDBAction(const DatabaseAction& action, QList<QVariant>* const values,
                                                                  QVariant* const lastInsertId)
{
    return execDBAction(action, QMap<QString, QVariant>(), values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execDBAction(const QString& action, QList<QVariant>* const values,
                                                                  QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), QMap<QString, QVariant>(), values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execDBAction(const QString& action, const QMap<QString, QVariant>& bindingMap,
                                                                  QList<QVariant>* const values, QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), bindingMap, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execDBAction(const DatabaseAction& action, const QMap<QString, QVariant>& bindingMap,
                                                                  QList<QVariant>* const values, QVariant* const lastInsertId)
{
    Q_D(DatabaseFaceCoreBackend);

    DatabaseFaceCoreBackend::QueryState returnResult = DatabaseFaceCoreBackend::NoErrors;
    QSqlDatabase db                              = d->databaseForThread();

    if (action.name.isNull())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "Attempt to execute null action";
        return DatabaseFaceCoreBackend::SQLError;
    }

#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Executing DBAction ["<<  action.name  <<"]";
#endif

    bool wrapInTransaction = (action.mode == QString::fromLatin1("transaction"));

    if (wrapInTransaction)
    {
        beginTransaction();
    }

    foreach(const DatabaseActionElement& actionElement, action.dbActionElements)
    {
        DatabaseFaceCoreBackend::QueryState result;

        if (actionElement.mode == QString::fromLatin1("query"))
        {
            result = execSql(actionElement.statement, bindingMap, values, lastInsertId);
        }
        else
        {
            result = execDirectSql(actionElement.statement);
        }

        if (result != DatabaseFaceCoreBackend::NoErrors)
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error while executing DBAction ["<<  action.name  <<"] Statement ["<<actionElement.statement<<"]";
            returnResult = result;

/*
            if (wrapInTransaction && !db.rollback())
            {
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error while rollback changes of previous DBAction.";
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
    if (returnResult==DatabaseFaceCoreBackend::NoErrors && wrapInTransaction && !db.commit())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error while committing changes of previous DBAction.";
    }
*/

    return returnResult;
}

QSqlQuery DatabaseFaceCoreBackend::execDBActionQuery(const QString& action, const QMap<QString, QVariant>& bindingMap)
{
    return execDBActionQuery(getDBAction(action), bindingMap);
}

QSqlQuery DatabaseFaceCoreBackend::execDBActionQuery(const DatabaseAction& action, const QMap<QString, QVariant>& bindingMap)
{
    Q_D(DatabaseFaceCoreBackend);

    QSqlDatabase db = d->databaseForThread();

#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Executing DBAction ["<<  action.name  <<"]";
#endif

    QSqlQuery result;

    foreach(const DatabaseActionElement& actionElement, action.dbActionElements)
    {
        if (actionElement.mode == QString::fromLatin1("query"))
        {
            result = execQuery(actionElement.statement, bindingMap);
        }
        else
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error, only DBActions with mode 'query' are allowed at this call!";
        }

        if (result.lastError().isValid() && result.lastError().number())
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error while executing DBAction [" <<  action.name
                                  << "] Statement [" << actionElement.statement 
                                  << "] Errornr. [" << result.lastError() << "]";
            break;
        }
    }

    return result;
}

void DatabaseFaceCoreBackend::setDatabaseErrorHandler(DatabaseErrorHandler* const handler)
{
    Q_D(DatabaseFaceCoreBackend);

    delete d->errorHandler;

    d->errorHandler = handler;
}

bool DatabaseFaceCoreBackend::isCompatible(const DatabaseFaceParameters& parameters)
{
    return QSqlDatabase::drivers().contains(parameters.databaseType);
}

bool DatabaseFaceCoreBackend::open(const DatabaseFaceParameters& parameters)
{
    Q_D(DatabaseFaceCoreBackend);
    d->parameters = parameters;
    // This will make possibly opened thread dbs reload at next access
    d->currentValidity++;

    int retries = 0;

    forever
    {
        QSqlDatabase database = d->databaseForThread();

        if (!database.isOpen())
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Error while opening the database. Trying again.";

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

bool DatabaseFaceCoreBackend::initSchema(DatabaseFaceSchemaUpdater* const updater)
{
    Q_D(DatabaseFaceCoreBackend);

    if (d->status == OpenSchemaChecked)
    {
        return true;
    }

    if (d->status == Unavailable)
    {
        return false;
    }

    if (updater->update())
    {
        d->status = OpenSchemaChecked;
        return true;
    }

    return false;
}

void DatabaseFaceCoreBackend::close()
{
    Q_D(DatabaseFaceCoreBackend);
    d->closeDatabaseForThread();
    d->status = Unavailable;
}

DatabaseFaceCoreBackend::Status DatabaseFaceCoreBackend::status() const
{
    Q_D(const DatabaseFaceCoreBackend);
    return d->status;
}

/*
bool DatabaseFaceCoreBackend::execSql(const QString& sql, QStringList* const values)
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

QList<QVariant> DatabaseFaceCoreBackend::readToList(SqlQuery& query)
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

#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Setting result value list ["<< list <<"]";
#endif
    return list;
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::handleQueryResult(SqlQuery& query, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    if (!query.isActive())
    {
        if (query.lastError().type() == QSqlError::ConnectionError)
        {
            return DatabaseFaceCoreBackend::ConnectionError;
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

    return DatabaseFaceCoreBackend::NoErrors;
}

// -------------------------------------------------------------------------------------

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql, const QVariant& boundValue1,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql,
                                                             const QVariant& boundValue1, const QVariant& boundValue2,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql,
                                                             const QVariant& boundValue1, const QVariant& boundValue2,
                                                             const QVariant& boundValue3, QList<QVariant>* const values,
                                                             QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql,
                                                             const QVariant& boundValue1, const QVariant& boundValue2,
                                                             const QVariant& boundValue3, const QVariant& boundValue4,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql, const QList<QVariant>& boundValues,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValues);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(const QString& sql, const QMap<QString, QVariant>& bindingMap,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, bindingMap);
    return handleQueryResult(query, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(SqlQuery& preparedQuery, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    exec(preparedQuery);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(SqlQuery& preparedQuery, const QVariant& boundValue1,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(SqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(SqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        const QVariant& boundValue3, QList<QVariant>* const values,
        QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(SqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        const QVariant& boundValue3, const QVariant& boundValue4,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execSql(SqlQuery& preparedQuery, const QList<QVariant>& boundValues,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValues);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql, const QVariant& boundValue1)
{
    SqlQuery query = prepareQuery(sql);
#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Trying to sql ["<< sql <<"] query ["<<query.lastQuery()<<"]";
#endif
    execQuery(query, boundValue1);
    return query;
}

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2);
    return query;
}

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3);
    return query;
}

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2,
                                        const QVariant& boundValue3, const QVariant& boundValue4)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3, boundValue4);
    return query;
}

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql, const QList<QVariant>& boundValues)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValues);
    return query;
}

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql)
{
    SqlQuery query = prepareQuery(sql);
#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG)<<"execQuery: Using statement ["<< query.lastQuery() <<"]";
#endif
    exec(query);
    return query;
}

// -------------------------------------------------------------------------------------

void DatabaseFaceCoreBackend::execQuery(SqlQuery& query, const QVariant& boundValue1)
{
    query.bindValue(0, boundValue1);
    exec(query);
}

void DatabaseFaceCoreBackend::execQuery(SqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    exec(query);
}

void DatabaseFaceCoreBackend::execQuery(SqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2,
                                    const QVariant& boundValue3)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    exec(query);
}

void DatabaseFaceCoreBackend::execQuery(SqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2,
                                    const QVariant& boundValue3, const QVariant& boundValue4)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    exec(query);
}

void DatabaseFaceCoreBackend::execQuery(SqlQuery& query, const QList<QVariant>& boundValues)
{
    for (int i=0; i<boundValues.size(); ++i)
    {
        query.bindValue(i, boundValues.at(i));
    }

    exec(query);
}

// -------------------------------------------------------------------------------------

SqlQuery DatabaseFaceCoreBackend::execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap)
{
    QString preparedString = sql;
    QList<QVariant> valuesToBind;

    if (!bindingMap.isEmpty())
    {
#ifdef DATABASCOREBACKEND_DEBUG
        qCDebug(DIGIKAM_FACESENGINE_LOG)<<"Prepare statement ["<< preparedString <<"] with binding map ["<< bindingMap <<"]";
#endif

        QRegExp identifierRegExp(QString::fromLatin1(":[A-Za-z0-9]+"));
        int pos = 0;

        while ( (pos=identifierRegExp.indexIn(preparedString, pos)) != -1)
        {
            QString namedPlaceholder = identifierRegExp.cap(0);

            if (!bindingMap.contains(namedPlaceholder))
            {
                qCWarning(DIGIKAM_FACESENGINE_LOG) << "Missing place holder" << namedPlaceholder
                                        << "in binding map. The following values are defined for this action:"
                                        << bindingMap.keys() <<". This is a setup error!";
                //TODO What should we do here? How can we cancel that action?
            }

            QVariant placeHolderValue = bindingMap.value(namedPlaceholder);
            QString replaceStr;

            if (placeHolderValue.userType() == qMetaTypeId<DBActionType>())
            {
                DBActionType actionType = placeHolderValue.value<DBActionType>();
                bool isValue            = actionType.isValue();
                QVariant value          = actionType.getActionValue();

                if ( value.type() == QVariant::Map )
                {
                    QMap<QString, QVariant> placeHolderMap = value.toMap();
                    QMap<QString, QVariant>::const_iterator iterator;

                    for (iterator = placeHolderMap.constBegin(); iterator != placeHolderMap.constEnd(); ++iterator)
                    {
                        const QString& key    = iterator.key();
                        const QVariant& value = iterator.value();
                        replaceStr.append(key);
                        replaceStr.append(QString::fromLatin1("= ?"));
                        valuesToBind.append(value);

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderMap.constEnd())
                        {
                            replaceStr.append(QString::fromLatin1(", "));
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
                            replaceStr.append(QString::fromLatin1("?"));
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry.value<QString>());
                        }

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(QString::fromLatin1(", "));
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
                            replaceStr.append(QString::fromLatin1("?"));
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry);
                        }

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(QString::fromLatin1(", "));
                        }
                    }
                }
                else
                {
                    if (isValue)
                    {
                        replaceStr = QString::fromLatin1("?");
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
#ifdef DATABASCOREBACKEND_DEBUG
                qCDebug(DIGIKAM_FACESENGINE_LOG)<<"Bind key ["<< namedPlaceholder <<"] to value ["<< bindingMap[namedPlaceholder] <<"]";
#endif

                valuesToBind.append(placeHolderValue);
                replaceStr = QString::fromLatin1("?");
            }

            preparedString = preparedString.replace(pos, identifierRegExp.matchedLength(), replaceStr);
            pos            = 0; // reset pos
        }
    }

#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG)<<"Prepared statement ["<< preparedString <<"] values ["<< valuesToBind <<"]";
#endif

    SqlQuery query = prepareQuery(preparedString);

    for (int i = 0; i < valuesToBind.size(); ++i)
    {
        query.bindValue(i, valuesToBind.at(i));
    }

    exec(query);
    return query;
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execUpsertDBAction(const DatabaseAction& action, const QVariant& id,
                                                                        const QStringList fieldNames, const QList<QVariant>& values)
{
    QMap<QString, QVariant> parameters;
    QMap<QString, QVariant> fieldValueMap;

    for (int i = 0; i < fieldNames.size(); ++i)
    {
        fieldValueMap.insert(fieldNames.at(i), values.at(i));
    }

    DBActionType fieldValueList = DBActionType::value(fieldValueMap);
    DBActionType fieldList      = DBActionType::fieldEntry(fieldNames);
    DBActionType valueList      = DBActionType::value(values);

    parameters.insert(QString::fromLatin1(":id"),             id);
    parameters.insert(QString::fromLatin1(":fieldValueList"), qVariantFromValue(fieldValueList));
    parameters.insert(QString::fromLatin1(":fieldList"),      qVariantFromValue(fieldList));
    parameters.insert(QString::fromLatin1(":valueList"),      qVariantFromValue(valueList));

    return execDBAction(action, parameters);
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execUpsertDBAction(const QString& action, const QVariant& id,
                                                                        const QStringList fieldNames, const QList<QVariant>& values)
{
    return execUpsertDBAction(getDBAction(action), id, fieldNames, values);
}

bool DatabaseFaceCoreBackend::connectionErrorHandling(int /*retries*/)
{
    Q_D(DatabaseFaceCoreBackend);

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

bool DatabaseFaceCoreBackend::queryErrorHandling(SqlQuery& query, int retries)
{
    Q_D(DatabaseFaceCoreBackend);

    if (d->isSQLiteLocqCritical(query))
    {
        if (d->checkRetrySQLiteLocqCritical(retries))
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

bool DatabaseFaceCoreBackend::transactionErrorHandling(const QSqlError& lastError, int retries)
{
    Q_D(DatabaseFaceCoreBackend);

    if (d->isSQLiteLockTransactionError(lastError))
    {
        if (d->checkRetrySQLiteLocqCritical(retries))
        {
            return true;
        }
    }

    d->debugOutputFailedTransaction(lastError);

    // no experience with other forms of failure

    return false;
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::execDirectSql(const QString& sql)
{
    Q_D(DatabaseFaceCoreBackend);

    if (!d->checkOperationStatus())
    {
        return DatabaseFaceCoreBackend::SQLError;
    }

    SqlQuery query = getQuery();
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
                return DatabaseFaceCoreBackend::SQLError;
            }
        }
    }

    return DatabaseFaceCoreBackend::NoErrors;
}

bool DatabaseFaceCoreBackend::exec(SqlQuery& query)
{
    Q_D(DatabaseFaceCoreBackend);

    if (!d->checkOperationStatus())
    {
        return false;
    }

    int retries = 0;

    forever
    {
#ifdef DATABASCOREBACKEND_DEBUG
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Trying to query ["<<query.lastQuery()<<"] values ["<< query.boundValues() <<"]";
#endif

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

bool DatabaseFaceCoreBackend::execBatch(SqlQuery& query)
{
    Q_D(DatabaseFaceCoreBackend);

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

SqlQuery DatabaseFaceCoreBackend::prepareQuery(const QString& sql)
{
    int retries = 0;

    forever
    {
        SqlQuery query = getQuery();

        if (query.prepare(sql))
        {
            return query;
        }
        else
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Prepare failed!";

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

SqlQuery DatabaseFaceCoreBackend::copyQuery(const SqlQuery& old)
{
    SqlQuery query = getQuery();
#ifdef DATABASCOREBACKEND_DEBUG
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Last query was ["<<old.lastQuery()<<"]";
#endif
    query.prepare(old.lastQuery());
    query.setForwardOnly(old.isForwardOnly());

    // only for positional binding
    QList<QVariant> boundValues = old.boundValues().values();

    foreach(const QVariant& value, boundValues)
    {
#ifdef DATABASCOREBACKEND_DEBUG
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Bind value to query ["<<value<<"]";
#endif
        query.addBindValue(value);
    }

    return query;
}

SqlQuery DatabaseFaceCoreBackend::getQuery()
{
    Q_D(DatabaseFaceCoreBackend);
    QSqlDatabase db = d->databaseForThread();

    SqlQuery query(db);
    query.setForwardOnly(true);
    return query;
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::beginTransaction()
{
    Q_D(DatabaseFaceCoreBackend);

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
                        return DatabaseFaceCoreBackend::ConnectionError;
                    }
                    else
                    {
                        return DatabaseFaceCoreBackend::SQLError;
                    }
                }
            }
        }

        d->isInTransaction = true;
    }

    return DatabaseFaceCoreBackend::NoErrors;
}

DatabaseFaceCoreBackend::QueryState DatabaseFaceCoreBackend::commitTransaction()
{
    Q_D(DatabaseFaceCoreBackend);

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
                    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Failed to commit transaction. Starting rollback.";
                    db.rollback();

                    if (lastError.type() == QSqlError::ConnectionError)
                    {
                        return DatabaseFaceCoreBackend::ConnectionError;
                    }
                    else
                    {
                        return DatabaseFaceCoreBackend::SQLError;
                    }
                }
            }
        }

        d->isInTransaction = false;
        d->transactionFinished();
    }

    return DatabaseFaceCoreBackend::NoErrors;
}

bool DatabaseFaceCoreBackend::isInTransaction() const
{
    Q_D(const DatabaseFaceCoreBackend);
    return d->isInTransaction;
}

void DatabaseFaceCoreBackend::rollbackTransaction()
{
    Q_D(DatabaseFaceCoreBackend);
    // we leave that out for transaction counting. It's an exceptional condition.
    d->databaseForThread().rollback();
}

QStringList DatabaseFaceCoreBackend::tables()
{
    Q_D(DatabaseFaceCoreBackend);
    return d->databaseForThread().tables();
}

QSqlError DatabaseFaceCoreBackend::lastSQLError()
{
    Q_D(DatabaseFaceCoreBackend);
    return d->databaseErrorForThread();
}

QSqlError DatabaseFaceCoreBackend::lastError()
{
    Q_D(DatabaseFaceCoreBackend);
    return d->databaseForThread().lastError();
}

int DatabaseFaceCoreBackend::maximumBoundValues() const
{
    Q_D(const DatabaseFaceCoreBackend);

    if (d->parameters.isSQLite())
    {
        return 999;   // SQLITE_MAX_VARIABLE_NUMBER
    }
    else
    {
        return 65535; // MySQL
    }
}

} // namespace FacesEngine
