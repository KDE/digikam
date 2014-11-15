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

#include "databasecorebackend.moc"
#include "databasecorebackend_p.h"

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

// KDE includes

#include <kdebug.h>

// Local includes

#include "dbactiontype.h"
#include "thumbnailschemaupdater.h"

namespace Digikam
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

DatabaseCoreBackendPrivate::BusyWaiter::BusyWaiter(DatabaseCoreBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->busyWaitMutex, &d->busyWaitCondVar)
{
}

// -----------------------------------------------------------------------------------------

DatabaseCoreBackendPrivate::ErrorLocker::ErrorLocker(DatabaseCoreBackendPrivate* const d)
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
        kDebug() << "WARNING !!! Transaction count is" << transactionCount << "when destroying database!!!";
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

DatabaseCoreBackendPrivate::DatabaseCoreBackendPrivate(DatabaseCoreBackend* const backend)
    : currentValidity(0),
      isInTransaction(false),
      status(DatabaseCoreBackend::Unavailable),
      lock(0),
      operationStatus(DatabaseCoreBackend::ExecuteNormal),
      errorLockOperationStatus(DatabaseCoreBackend::ExecuteNormal),
      errorHandler(0),
      q(backend)
{
}

DatabaseCoreBackendPrivate::~DatabaseCoreBackendPrivate()
{
    // Must be shut down from the main thread.
    // Clean up the QThreadStorage. It deletes any stored data.
    threadDataStorage.setLocalData(0);
}

void DatabaseCoreBackendPrivate::init(const QString& name, DatabaseLocking* const l)
{
    backendName = name;
    lock        = l;

    qRegisterMetaType<DatabaseErrorAnswer*>("DatabaseErrorAnswer*");
    qRegisterMetaType<QSqlError>();
}

// "A connection can only be used from within the thread that created it.
//  Moving connections between threads or creating queries from a different thread is not supported."
// => one QSqlDatabase object per thread.
// The main class' open/close methods only interact with the "firstDatabase" object.
// When another thread requests a DB, a new connection is opened and closed at
// finishing of the thread.
QSqlDatabase DatabaseCoreBackendPrivate::databaseForThread()
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
            kDebug() << "Error while opening the database. Error was" << threadData->database.lastError();
        }
    }

    return threadData->database;
}

QSqlDatabase DatabaseCoreBackendPrivate::createDatabaseConnection()
{
    QSqlDatabase db        = QSqlDatabase::addDatabase(parameters.databaseType, connectionName());
    QString connectOptions = parameters.connectOptions;

    if (parameters.isSQLite())
    {
        QStringList toAdd;
        // enable shared cache, especially useful with SQLite >= 3.5.0
        toAdd << "QSQLITE_ENABLE_SHARED_CACHE";
        // We do our own waiting.
        toAdd << "QSQLITE_BUSY_TIMEOUT=0";

        if (!connectOptions.isEmpty())
        {
            connectOptions += ';';
        }

        connectOptions += toAdd.join(";");
    }

    db.setDatabaseName(parameters.databaseName);
    db.setConnectOptions(connectOptions);
    db.setHostName(parameters.hostName);
    db.setPort(parameters.port);
    db.setUserName(parameters.userName);
    db.setPassword(parameters.password);

    return db;
}

void DatabaseCoreBackendPrivate::closeDatabaseForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->closeDatabase();
    }
}

QSqlError DatabaseCoreBackendPrivate::databaseErrorForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        return threadDataStorage.localData()->lastError;
    }
    return QSqlError();
}

void DatabaseCoreBackendPrivate::setDatabaseErrorForThread(const QSqlError& lastError)
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->lastError = lastError;
    }
}

QString DatabaseCoreBackendPrivate::connectionName()
{
    return backendName + QString::number((quintptr)QThread::currentThread());
}

bool DatabaseCoreBackendPrivate::incrementTransactionCount()
{
    return (!threadDataStorage.localData()->transactionCount++);
}

bool DatabaseCoreBackendPrivate::decrementTransactionCount()
{
    return (!--threadDataStorage.localData()->transactionCount);
}

bool DatabaseCoreBackendPrivate::isInMainThread() const
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

bool DatabaseCoreBackendPrivate::isInUIThread() const
{
    QApplication* const app = qobject_cast<QApplication*>(QCoreApplication::instance());

    if (!app)
    {
        return false;
    }

    return (QThread::currentThread() == app->thread());
}

bool DatabaseCoreBackendPrivate::reconnectOnError() const
{
    return parameters.isMySQL();
}

bool DatabaseCoreBackendPrivate::isSQLiteLockError(const SqlQuery& query) const
{
    return parameters.isSQLite() &&
           (query.lastError().number() == 5 /*SQLITE_BUSY*/ || query.lastError().number() == 6/*SQLITE_LOCKED*/);
}

bool DatabaseCoreBackendPrivate::isSQLiteLockTransactionError(const QSqlError& lastError) const
{
    return parameters.isSQLite() &&
           lastError.type()         == QSqlError::TransactionError &&
           lastError.databaseText() == QLatin1String("database is locked");
    // wouldnt it be great if they gave us the database error number...
}

bool DatabaseCoreBackendPrivate::isConnectionError(const SqlQuery& query) const
{
    // the backend returns connection error e.g. for Constraint Failed errors.
    if (parameters.isSQLite())
    {
        return false;
    }

    return query.lastError().type()   == QSqlError::ConnectionError ||
           query.lastError().number() == 2006;
}

bool DatabaseCoreBackendPrivate::needToConsultUserForError(const SqlQuery&) const
{
    // no such conditions found and defined as yet
    return false;
}

bool DatabaseCoreBackendPrivate::needToHandleWithErrorHandler(const SqlQuery& query) const
{
    return (isConnectionError(query) || needToConsultUserForError(query));
}

bool DatabaseCoreBackendPrivate::checkRetrySQLiteLockError(int retries)
{
    if (!(retries % 25))
    {
        kDebug() << "Database is locked. Waited" << retries*10;
    }

    const int uiMaxRetries = 50;
    const int maxRetries   = 1000;

    if (retries > qMax(uiMaxRetries, maxRetries))
    {
        if (retries > (isInUIThread() ? uiMaxRetries : maxRetries))
        {
            kWarning() << "Detected locked database file. There is an active transaction. Waited but giving up now.";
            return false;
        }
    }

    BusyWaiter waiter(this);
    waiter.wait(10);
    return true;
}

void DatabaseCoreBackendPrivate::debugOutputFailedQuery(const QSqlQuery& query) const
{
    kDebug() << "Failure executing query:\n"
             << query.executedQuery()
             << "\nError messages:" << query.lastError().driverText() << query.lastError().databaseText()
             << query.lastError().number() << query.lastError().type()
             << "\nBound values: " << query.boundValues().values();
}

void DatabaseCoreBackendPrivate::debugOutputFailedTransaction(const QSqlError& error) const
{
    kDebug() << "Failure executing transaction. Error messages:\n"
             << error.driverText() << error.databaseText()
             << error.number() << error.type();
}

void DatabaseCoreBackendPrivate::transactionFinished()
{
    // wakes up any BusyWaiter waiting on the busyWaitCondVar.
    // Possibly called under d->lock->mutex lock, so we do not lock the busyWaitMutex
    busyWaitCondVar.wakeOne();
}

/** Set the wait flag to queryStatus. Typically, call this with Wait. */
void DatabaseCoreBackendPrivate::setQueryOperationFlag(DatabaseCoreBackend::QueryOperationStatus status)
{
    // Enforce lock order (first main mutex, second error lock mutex)
    QMutexLocker l(&errorLockMutex);
    // this change must be done under errorLockMutex lock
    errorLockOperationStatus = status;
    operationStatus          = status;
}

/** Set the wait flag to queryStatus and wake all waiting threads.
 *  Typically, call wakeAll with status ExecuteNormal or AbortQueries. */
void DatabaseCoreBackendPrivate::queryOperationWakeAll(DatabaseCoreBackend::QueryOperationStatus status)
{
    QMutexLocker l(&errorLockMutex);
    operationStatus          = status;
    errorLockOperationStatus = status;
    errorLockCondVar.wakeAll();
}

bool DatabaseCoreBackendPrivate::checkOperationStatus()
{
    while (operationStatus == DatabaseCoreBackend::Wait)
    {
        ErrorLocker locker(this);
        locker.wait();
    }

    if (operationStatus == DatabaseCoreBackend::ExecuteNormal)
    {
        return true;
    }
    else if (operationStatus == DatabaseCoreBackend::AbortQueries)
    {
        return false;
    }

    return false;
}

/// Returns true if the query shall be retried
bool DatabaseCoreBackendPrivate::handleWithErrorHandler(const SqlQuery* const query)
{
    if (errorHandler)
    {
        setQueryOperationFlag(DatabaseCoreBackend::Wait);

        ErrorLocker locker(this);
        bool called         = false;
        QSqlError lastError = query ? query->lastError() : databaseForThread().lastError();
        QString lastQuery   = query ? query->lastQuery() : QString();

        if (!query || isConnectionError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "connectionError", Qt::AutoConnection,
                                               Q_ARG(DatabaseErrorAnswer*, this), Q_ARG(const QSqlError, lastError),
                                               Q_ARG(const QString, lastQuery));
        }
        else if (needToConsultUserForError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "consultUserForError", Qt::AutoConnection,
                                               Q_ARG(DatabaseErrorAnswer*, this), Q_ARG(const QSqlError, lastError),
                                               Q_ARG(const QString, lastQuery));
        }
        else
        {
            // unclear what to do.
            errorLockOperationStatus = DatabaseCoreBackend::ExecuteNormal;
            operationStatus          = DatabaseCoreBackend::ExecuteNormal;
            return true;
        }

        if (called)
        {
            locker.wait();
        }
        else
        {
            kWarning() << "Failed to invoke DatabaseErrorHandler. Aborting all queries.";
            operationStatus = DatabaseCoreBackend::AbortQueries;
        }

        switch (operationStatus)
        {
            case DatabaseCoreBackend::ExecuteNormal:
            case DatabaseCoreBackend::Wait:
                return true;
            case DatabaseCoreBackend::AbortQueries:
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

void DatabaseCoreBackendPrivate::connectionErrorContinueQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(DatabaseCoreBackend::ExecuteNormal);
}

void DatabaseCoreBackendPrivate::connectionErrorAbortQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(DatabaseCoreBackend::AbortQueries);
}

// -----------------------------------------------------------------------------------------

DatabaseCoreBackendPrivate::AbstractUnlocker::AbstractUnlocker(DatabaseCoreBackendPrivate* const d)
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

void DatabaseCoreBackendPrivate::AbstractUnlocker::finishAcquire()
{
    // drop lock acquired in first line. Main mutex is now free.
    // We maintain lock order (first main mutex, second error lock mutex)
    // but we drop main mutex lock for waiting on the cond var.
    d->lock->mutex.unlock();
}

DatabaseCoreBackendPrivate::AbstractUnlocker::~AbstractUnlocker()
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

DatabaseCoreBackendPrivate::AbstractWaitingUnlocker::AbstractWaitingUnlocker(DatabaseCoreBackendPrivate* const d,
        QMutex* const mutex, QWaitCondition* const condVar)
    : AbstractUnlocker(d), mutex(mutex), condVar(condVar)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.
    // lock condvar mutex (lock only if main mutex is locked)
    mutex->lock();

    finishAcquire();
}

DatabaseCoreBackendPrivate::AbstractWaitingUnlocker::~AbstractWaitingUnlocker()
{
    // unlock condvar mutex. Both mutexes are now free.
    mutex->unlock();
    // now base class destructor is executed, reallocating main mutex
}

bool DatabaseCoreBackendPrivate::AbstractWaitingUnlocker::wait(unsigned long time)
{
    return condVar->wait(mutex, time);
}

// -----------------------------------------------------------------------------------------

/** This suspends the current thread if the query status as
 *  set by setFlag() is Wait and until the thread is woken with wakeAll().
 *  The DatabaseAccess mutex will be unlocked while waiting.
 */
void DatabaseCoreBackendPrivate::ErrorLocker::wait()
{
    // we use a copy of the flag under lock of the errorLockMutex to be able to check it here
    while (d->errorLockOperationStatus == DatabaseCoreBackend::Wait)
    {
        wait();
    }
}

// -----------------------------------------------------------------------------------------

DatabaseCoreBackend::DatabaseCoreBackend(const QString& backendName, DatabaseLocking* const locking)
    : d_ptr(new DatabaseCoreBackendPrivate(this))
{
    d_ptr->init(backendName, locking);
}

DatabaseCoreBackend::DatabaseCoreBackend(const QString& backendName, DatabaseLocking* const locking, DatabaseCoreBackendPrivate& dd)
    : d_ptr(&dd)
{
    d_ptr->init(backendName, locking);
}

DatabaseCoreBackend::~DatabaseCoreBackend()
{
    Q_D(DatabaseCoreBackend);
    close();

    delete d;
}

DatabaseConfigElement DatabaseCoreBackend::configElement() const
{
    Q_D(const DatabaseCoreBackend);
    return DatabaseConfigElement::element(d->parameters.databaseType);
}

DatabaseAction DatabaseCoreBackend::getDBAction(const QString& actionName) const
{
    DatabaseAction action = configElement().sqlStatements.value(actionName);

    if (action.name.isNull())
    {
        kWarning() << "No DB action defined for" << actionName << "! Implementation missing for this database type.";
    }

    return action;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDBAction(const DatabaseAction& action, QList<QVariant>* const values,
                                                                  QVariant* const lastInsertId)
{
    return execDBAction(action, QMap<QString, QVariant>(), values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDBAction(const QString& action, QList<QVariant>* const values,
                                                                  QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), QMap<QString, QVariant>(), values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDBAction(const QString& action, const QMap<QString, QVariant>& bindingMap,
                                                                  QList<QVariant>* const values, QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), bindingMap, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDBAction(const DatabaseAction& action, const QMap<QString, QVariant>& bindingMap,
                                                                  QList<QVariant>* const values, QVariant* const lastInsertId)
{
    Q_D(DatabaseCoreBackend);

    DatabaseCoreBackend::QueryState returnResult = DatabaseCoreBackend::NoErrors;
    QSqlDatabase db                              = d->databaseForThread();

    if (action.name.isNull())
    {
        kWarning() << "Attempt to execute null action";
        return DatabaseCoreBackend::SQLError;
    }

#ifdef DATABASCOREBACKEND_DEBUG
    kDebug() << "Executing DBAction ["<<  action.name  <<"]";
#endif

    bool wrapInTransaction = (action.mode == QString("transaction"));

    if (wrapInTransaction)
    {
        beginTransaction();
    }

    foreach(const DatabaseActionElement& actionElement, action.dbActionElements)
    {
        DatabaseCoreBackend::QueryState result;

        if (actionElement.mode == QString("query"))
        {
            result = execSql(actionElement.statement, bindingMap, values, lastInsertId);
        }
        else
        {
            result = execDirectSql(actionElement.statement);
        }

        if (result != DatabaseCoreBackend::NoErrors)
        {
            kDebug() << "Error while executing DBAction ["<<  action.name  <<"] Statement ["<<actionElement.statement<<"]";
            returnResult = result;

/*
            if (wrapInTransaction && !db.rollback())
            {
                kDebug() << "Error while rollback changes of previous DBAction.";
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
    if (returnResult==DatabaseCoreBackend::NoErrors && wrapInTransaction && !db.commit())
    {
        kDebug() << "Error while committing changes of previous DBAction.";
    }
*/

    return returnResult;
}

QSqlQuery DatabaseCoreBackend::execDBActionQuery(const QString& action, const QMap<QString, QVariant>& bindingMap)
{
    return execDBActionQuery(getDBAction(action), bindingMap);
}

QSqlQuery DatabaseCoreBackend::execDBActionQuery(const DatabaseAction& action, const QMap<QString, QVariant>& bindingMap)
{
    Q_D(DatabaseCoreBackend);

    QSqlDatabase db = d->databaseForThread();

#ifdef DATABASCOREBACKEND_DEBUG
    kDebug() << "Executing DBAction ["<<  action.name  <<"]";
#endif

    QSqlQuery result;

    foreach(const DatabaseActionElement& actionElement, action.dbActionElements)
    {
        if (actionElement.mode==QString("query"))
        {
            result = execQuery(actionElement.statement, bindingMap);
        }
        else
        {
            kDebug() << "Error, only DBActions with mode 'query' are allowed at this call!";
        }

        if (result.lastError().isValid() && result.lastError().number())
        {
            kDebug() << "Error while executing DBAction [" <<  action.name
                     << "] Statement [" << actionElement.statement << "] Errornr. [" << result.lastError() << "]";
            break;
        }
    }

    return result;
}

void DatabaseCoreBackend::setDatabaseErrorHandler(DatabaseErrorHandler* const handler)
{
    Q_D(DatabaseCoreBackend);

    delete d->errorHandler;

    d->errorHandler = handler;
}

bool DatabaseCoreBackend::isCompatible(const DatabaseParameters& parameters)
{
    return QSqlDatabase::drivers().contains(parameters.databaseType);
}

bool DatabaseCoreBackend::open(const DatabaseParameters& parameters)
{
    Q_D(DatabaseCoreBackend);
    d->parameters = parameters;
    // This will make possibly opened thread dbs reload at next access
    d->currentValidity++;

    int retries = 0;

    forever
    {
        QSqlDatabase database = d->databaseForThread();

        if (!database.isOpen())
        {
            kDebug() << "Error while opening the database. Trying again.";

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

bool DatabaseCoreBackend::initSchema(ThumbnailSchemaUpdater* const updater)
{
    Q_D(DatabaseCoreBackend);

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

void DatabaseCoreBackend::close()
{
    Q_D(DatabaseCoreBackend);
    d->closeDatabaseForThread();
    d->status = Unavailable;
}

DatabaseCoreBackend::Status DatabaseCoreBackend::status() const
{
    Q_D(const DatabaseCoreBackend);
    return d->status;
}

/*
bool DatabaseCoreBackend::execSql(const QString& sql, QStringList* const values)
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

QList<QVariant> DatabaseCoreBackend::readToList(SqlQuery& query)
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
    kDebug() << "Setting result value list ["<< list <<"]";
#endif
    return list;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::handleQueryResult(SqlQuery& query, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    if (!query.isActive())
    {
        if (query.lastError().type() == QSqlError::ConnectionError)
        {
            return DatabaseCoreBackend::ConnectionError;
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

    return DatabaseCoreBackend::NoErrors;
}

// -------------------------------------------------------------------------------------

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, const QVariant& boundValue1,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql,
                                                             const QVariant& boundValue1, const QVariant& boundValue2,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql,
                                                             const QVariant& boundValue1, const QVariant& boundValue2,
                                                             const QVariant& boundValue3, QList<QVariant>* const values,
                                                             QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql,
                                                             const QVariant& boundValue1, const QVariant& boundValue2,
                                                             const QVariant& boundValue3, const QVariant& boundValue4,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, const QList<QVariant>& boundValues,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValues);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, const QMap<QString, QVariant>& bindingMap,
                                                             QList<QVariant>* const values, QVariant* const lastInsertId)
{
    SqlQuery query = execQuery(sql, bindingMap);
    return handleQueryResult(query, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(SqlQuery& preparedQuery, QList<QVariant>* const values, QVariant* const lastInsertId)
{
    exec(preparedQuery);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(SqlQuery& preparedQuery, const QVariant& boundValue1,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(SqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(SqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        const QVariant& boundValue3, QList<QVariant>* const values,
        QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(SqlQuery& preparedQuery,
        const QVariant& boundValue1, const QVariant& boundValue2,
        const QVariant& boundValue3, const QVariant& boundValue4,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(SqlQuery& preparedQuery, const QList<QVariant>& boundValues,
        QList<QVariant>* const values, QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValues);
    return handleQueryResult(preparedQuery, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QVariant& boundValue1)
{
    SqlQuery query = prepareQuery(sql);
#ifdef DATABASCOREBACKEND_DEBUG
    kDebug() << "Trying to sql ["<< sql <<"] query ["<<query.lastQuery()<<"]";
#endif
    execQuery(query, boundValue1);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                        const QVariant& boundValue1, const QVariant& boundValue2,
                                        const QVariant& boundValue3, const QVariant& boundValue4)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3, boundValue4);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QList<QVariant>& boundValues)
{
    SqlQuery query = prepareQuery(sql);
    execQuery(query, boundValues);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql)
{
    SqlQuery query = prepareQuery(sql);
#ifdef DATABASCOREBACKEND_DEBUG
    kDebug()<<"execQuery: Using statement ["<< query.lastQuery() <<"]";
#endif
    exec(query);
    return query;
}

// -------------------------------------------------------------------------------------

void DatabaseCoreBackend::execQuery(SqlQuery& query, const QVariant& boundValue1)
{
    query.bindValue(0, boundValue1);
    exec(query);
}

void DatabaseCoreBackend::execQuery(SqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    exec(query);
}

void DatabaseCoreBackend::execQuery(SqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2,
                                    const QVariant& boundValue3)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    exec(query);
}

void DatabaseCoreBackend::execQuery(SqlQuery& query,
                                    const QVariant& boundValue1, const QVariant& boundValue2,
                                    const QVariant& boundValue3, const QVariant& boundValue4)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    exec(query);
}

void DatabaseCoreBackend::execQuery(SqlQuery& query, const QList<QVariant>& boundValues)
{
    for (int i=0; i<boundValues.size(); ++i)
    {
        query.bindValue(i, boundValues.at(i));
    }

    exec(query);
}

// -------------------------------------------------------------------------------------

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap)
{
    QString preparedString = sql;
    QList<QVariant> valuesToBind;

    if (!bindingMap.isEmpty())
    {
#ifdef DATABASCOREBACKEND_DEBUG
        kDebug()<<"Prepare statement ["<< preparedString <<"] with binding map ["<< bindingMap <<"]";
#endif

        QRegExp identifierRegExp(":[A-Za-z0-9]+");
        int pos = 0;

        while ( (pos=identifierRegExp.indexIn(preparedString, pos)) != -1)
        {
            QString namedPlaceholder = identifierRegExp.cap(0);

            if (!bindingMap.contains(namedPlaceholder))
            {
                kWarning() << "Missing place holder" << namedPlaceholder
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
                        replaceStr.append("= ?");
                        valuesToBind.append(value);

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderMap.constEnd())
                        {
                            replaceStr.append(", ");
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
                            replaceStr.append("?");
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry.value<QString>());
                        }

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(", ");
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
                            replaceStr.append("?");
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry);
                        }

                        // Add a semicolon to the statement, if we are not on the last entry
                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(", ");
                        }
                    }
                }
                else
                {
                    if (isValue)
                    {
                        replaceStr = '?';
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
                kDebug()<<"Bind key ["<< namedPlaceholder <<"] to value ["<< bindingMap[namedPlaceholder] <<"]";
#endif

                valuesToBind.append(placeHolderValue);
                replaceStr = '?';
            }

            preparedString = preparedString.replace(pos, identifierRegExp.matchedLength(), replaceStr);
            pos=0; // reset pos
        }
    }

#ifdef DATABASCOREBACKEND_DEBUG
    kDebug()<<"Prepared statement ["<< preparedString <<"] values ["<< valuesToBind <<"]";
#endif

    SqlQuery query = prepareQuery(preparedString);

    for (int i=0; i < valuesToBind.size(); ++i)
    {
        query.bindValue(i, valuesToBind.at(i));
    }

    exec(query);
    return query;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execUpsertDBAction(const DatabaseAction& action, const QVariant& id,
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

    parameters.insert(":id",             id);
    parameters.insert(":fieldValueList", qVariantFromValue(fieldValueList));
    parameters.insert(":fieldList",      qVariantFromValue(fieldList));
    parameters.insert(":valueList",      qVariantFromValue(valueList));

    return execDBAction(action, parameters);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execUpsertDBAction(const QString& action, const QVariant& id,
                                                                        const QStringList fieldNames, const QList<QVariant>& values)
{
    return execUpsertDBAction(getDBAction(action), id, fieldNames, values);
}

bool DatabaseCoreBackend::connectionErrorHandling(int /*retries*/)
{
    Q_D(DatabaseCoreBackend);

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

bool DatabaseCoreBackend::queryErrorHandling(SqlQuery& query, int retries)
{
    Q_D(DatabaseCoreBackend);

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

bool DatabaseCoreBackend::transactionErrorHandling(const QSqlError& lastError, int retries)
{
    Q_D(DatabaseCoreBackend);

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

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDirectSql(const QString& sql)
{
    Q_D(DatabaseCoreBackend);

    if (!d->checkOperationStatus())
    {
        return DatabaseCoreBackend::SQLError;
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
                return DatabaseCoreBackend::SQLError;
            }
        }
    }

    return DatabaseCoreBackend::NoErrors;
}

bool DatabaseCoreBackend::exec(SqlQuery& query)
{
    Q_D(DatabaseCoreBackend);

    if (!d->checkOperationStatus())
    {
        return false;
    }

    int retries = 0;

    forever
    {
#ifdef DATABASCOREBACKEND_DEBUG
        kDebug() << "Trying to query ["<<query.lastQuery()<<"] values ["<< query.boundValues() <<"]";
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

bool DatabaseCoreBackend::execBatch(SqlQuery& query)
{
    Q_D(DatabaseCoreBackend);

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

SqlQuery DatabaseCoreBackend::prepareQuery(const QString& sql)
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
            kDebug() << "Prepare failed!";

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

SqlQuery DatabaseCoreBackend::copyQuery(const SqlQuery& old)
{
    SqlQuery query = getQuery();
#ifdef DATABASCOREBACKEND_DEBUG
    kDebug() << "Last query was ["<<old.lastQuery()<<"]";
#endif
    query.prepare(old.lastQuery());
    query.setForwardOnly(old.isForwardOnly());

    // only for positional binding
    QList<QVariant> boundValues = old.boundValues().values();

    foreach(const QVariant& value, boundValues)
    {
#ifdef DATABASCOREBACKEND_DEBUG
        kDebug() << "Bind value to query ["<<value<<"]";
#endif
        query.addBindValue(value);
    }

    return query;
}

SqlQuery DatabaseCoreBackend::getQuery()
{
    Q_D(DatabaseCoreBackend);
    QSqlDatabase db = d->databaseForThread();

    SqlQuery query(db);
    query.setForwardOnly(true);
    return query;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::beginTransaction()
{
    Q_D(DatabaseCoreBackend);

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
                        return DatabaseCoreBackend::ConnectionError;
                    }
                    else
                    {
                        return DatabaseCoreBackend::SQLError;
                    }
                }
            }
        }

        d->isInTransaction = true;
    }

    return DatabaseCoreBackend::NoErrors;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::commitTransaction()
{
    Q_D(DatabaseCoreBackend);

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
                    kDebug() << "Failed to commit transaction. Starting rollback.";
                    db.rollback();

                    if (lastError.type() == QSqlError::ConnectionError)
                    {
                        return DatabaseCoreBackend::ConnectionError;
                    }
                    else
                    {
                        return DatabaseCoreBackend::SQLError;
                    }
                }
            }
        }

        d->isInTransaction = false;
        d->transactionFinished();
    }

    return DatabaseCoreBackend::NoErrors;
}

bool DatabaseCoreBackend::isInTransaction() const
{
    Q_D(const DatabaseCoreBackend);
    return d->isInTransaction;
}

void DatabaseCoreBackend::rollbackTransaction()
{
    Q_D(DatabaseCoreBackend);
    // we leave that out for transaction counting. It's an exceptional condition.
    d->databaseForThread().rollback();
}

QStringList DatabaseCoreBackend::tables()
{
    Q_D(DatabaseCoreBackend);
    return d->databaseForThread().tables();
}

QSqlError DatabaseCoreBackend::lastSQLError()
{
    Q_D(DatabaseCoreBackend);
    return d->databaseErrorForThread();
}

QString DatabaseCoreBackend::lastError()
{
    Q_D(DatabaseCoreBackend);
    return d->databaseForThread().lastError().text();
}

int DatabaseCoreBackend::maximumBoundValues() const
{
    Q_D(const DatabaseCoreBackend);

    if (d->parameters.isSQLite())
    {
        return 999;   // SQLITE_MAX_VARIABLE_NUMBER
    }
    else
    {
        return 65535; // MySQL
    }
}

}  // namespace Digikam
