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

/*
#ifndef DATABASCOREBACKEND_DEBUG
#define DATABASCOREBACKEND_DEBUG
#endif
*/

#include "databasecorebackend.h"
#include "databasecorebackend_p.h"
#include "databasecorebackend.moc"
#include "databaseerrorhandler.moc"

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QtCore/QRegExp>


// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "thumbnailschemaupdater.h"

namespace Digikam
{



DatabaseLocking::DatabaseLocking()
            : mutex(QMutex::Recursive), lockCount(0)// create a recursive mutex
{
}

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

DatabaseCoreBackendPrivate::DatabaseCoreBackendPrivate(DatabaseCoreBackend *backend)
                          : q(backend)
{

    status          = DatabaseCoreBackend::Unavailable;
    isInTransaction = false;
    operationStatus = DatabaseCoreBackend::ExecuteNormal;
    handlingConnectionError = true;
    errorHandler = 0;
}


void DatabaseCoreBackendPrivate::init(const QString &name, DatabaseLocking *l)
{
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                     q, SLOT(slotMainThreadFinished()));

    backendName = name;
    lock        = l;

    qRegisterMetaType<DatabaseErrorAnswer*>("DatabaseErrorAnswer*");
    qRegisterMetaType<SqlQuery>("SqlQuery");
}

// "A connection can only be used from within the thread that created it.
//  Moving connections between threads or creating queries from a different thread is not supported."
// => one QSqlDatabase object per thread.
// The main class' open/close methods only interact with the "firstDatabase" object.
// When another thread requests a DB, a new connection is opened and closed at
// finishing of the thread.
QSqlDatabase DatabaseCoreBackendPrivate::databaseForThread()
{
    QThread *thread = QThread::currentThread();
    QSqlDatabase db = threadDatabases[thread];
    int isValid = databasesValid[thread];
    if (!isValid || !db.isOpen())
    {
        // need to open a db for thread
        bool success = open(db);
        if (!success)
           kDebug(50003) << "Error while opening the database. Details: [" << db.lastError() << "]";

        QObject::connect(thread, SIGNAL(finished()),
                            q, SLOT(slotThreadFinished()));
    }
#ifdef DATABASCOREBACKEND_DEBUG
    else
    {
        kDebug(50003) << "Database ["<< connectionName(thread) <<"] already open for thread ["<< thread <<"].";
    }
#endif

    return db;
}

void DatabaseCoreBackendPrivate::closeDatabaseForThread()
{
    QThread *thread = QThread::currentThread();
    // scope, so that db is destructed when calling removeDatabase
    {
        QSqlDatabase db = threadDatabases[thread];
        if (db.isValid())
            db.close();
    }
    threadDatabases.remove(thread);
    databasesValid[thread] = 0;
    transactionCount.remove(thread);
    QSqlDatabase::removeDatabase(connectionName(thread));
}

QString DatabaseCoreBackendPrivate::connectionName(QThread *thread)
{
    return backendName + QString::number((quintptr)thread);
}

bool DatabaseCoreBackendPrivate::open(QSqlDatabase& db)
{
    if (db.isValid())
        db.close();

    QThread *thread = QThread::currentThread();

    db = QSqlDatabase::addDatabase(parameters.databaseType, connectionName(thread));

    db.setDatabaseName(parameters.databaseName);
    db.setConnectOptions(parameters.connectOptions);
    db.setHostName(parameters.hostName);
    db.setPort(parameters.port);
    db.setUserName(parameters.userName);
    db.setPassword(parameters.password);

    bool success = db.open();

    if (success==false)
    {
        kDebug(50003) << "Error while opening the database. Error was <" << db.lastError() << ">";
    }

    threadDatabases[thread]  = db;
    databasesValid[thread]   = 1;
    transactionCount[thread] = 0;

    return success;
}

bool DatabaseCoreBackendPrivate::incrementTransactionCount()
{
    QThread *thread = QThread::currentThread();
    return !transactionCount[thread]++;
}

bool DatabaseCoreBackendPrivate::decrementTransactionCount()
{
    QThread *thread = QThread::currentThread();
    return !--transactionCount[thread];
}

bool DatabaseCoreBackendPrivate::isInTransactionInOtherThread() const
{
    QThread *thread = QThread::currentThread();
    QHash<QThread*, int>::const_iterator it;
    for (it=transactionCount.constBegin(); it != transactionCount.constEnd(); ++it)
        if (it.key() != thread && it.value())
            return true;
    return false;
}

bool DatabaseCoreBackendPrivate::isInMainThread() const
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

bool DatabaseCoreBackendPrivate::isInUIThread() const
{
    QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (!app)
        return false;
    return QThread::currentThread() == app->thread();
}

bool DatabaseCoreBackendPrivate::isSQLiteLockError(const QSqlQuery &query)
{
    return parameters.isSQLite() && query.lastError().number() == 5;
}

bool DatabaseCoreBackendPrivate::checkRetrySQLiteLockError(int retries)
{
    if (!isInUIThread())
    {
        if (retries == 1)
            kDebug(50003) << "Detected locked database file. Waiting at most 10s trying again.";

        sotoSleep::sleep(1);
        return true;
    }

    kWarning(50003) << "Detected locked database file. There is an active transaction.";
    return false;
}

void DatabaseCoreBackendPrivate::debugOutputFailedQuery(const QSqlQuery &query)
{
    kDebug(50003) << "Failure executing query: ";
    kDebug(50003) << query.executedQuery();
    kDebug(50003) << "Error messages:" << query.lastError().text() << query.lastError().number()
                  << query.lastError().type() << query.lastError().databaseText()
                  << query.lastError().driverText() << query.driver()->lastError();
    kDebug(50003) << "Bound values: " << query.boundValues().values();
}


DatabaseCoreBackendPrivate::ErrorLocker::ErrorLocker(DatabaseCoreBackendPrivate *d)
    : count(0), d(d)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.

    // acquire lock
    d->lock->mutex.lock();
    // store lock count
    int count = d->lock->lockCount;
    // set lock count to 0
    d->lock->lockCount = 0;
    // unlock
    for (int i=0; i<count; ++i)
        d->lock->mutex.unlock();

    // lock condvar mutex (lock only if main mutex is locked)
    d->errorLockMutex.lock();
    // drop lock acquired in first line. Main mutex is now free.
    // We maintain lock order (first main mutex, second error lock mutex)
    // but we drop main mutex lock for waiting on the cond var.
    d->lock->mutex.unlock();
}

    /** This suspends the current thread if the query status as
     *  set by setFlag() is Wait and until the thread is woken with wakeAll().
     *  The DatabaseAccess mutex will be unlocked while waiting.
     */
void DatabaseCoreBackendPrivate::ErrorLocker::wait()
{
    // we use a copy of the flag under lock of the errorLockMutex to be able to check it here
    while (d->errorLockOperationStatus == DatabaseCoreBackend::Wait)
        d->errorLockCondVar.wait(&d->errorLockMutex);  
}

DatabaseCoreBackendPrivate::ErrorLocker::~ErrorLocker()
{
    // unlock condvar mutex. Both mutexes are now free.
    d->errorLockMutex.unlock();

    // lock main mutex as often as it was locked before
    for (int i=0; i<count; ++i)
        d->lock->mutex.lock();
    // update lock count
    d->lock->lockCount = count;
}

    /** Set the wait flag to queryStatus. Typically, call this with Wait. */
void DatabaseCoreBackendPrivate::setQueryOperationFlag(DatabaseCoreBackend::QueryOperationStatus status)
{
    // Enforce lock order (first main mutex, second error lock mutex)
    QMutexLocker l(&errorLockMutex);
    // this change must be done under errorLockMutex lock
    errorLockOperationStatus = status;
    operationStatus = status;
}

    /** Set the wait flag to queryStatus and wake all waiting threads.
     *  Typically, call wakeAll with status ExecuteNormal or AbortQueries. */
void DatabaseCoreBackendPrivate::queryOperationWakeAll(DatabaseCoreBackend::QueryOperationStatus status)
{
    QMutexLocker l(&errorLockMutex);
    operationStatus = status;
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
        return true;
    else if (operationStatus == DatabaseCoreBackend::AbortQueries)
        return false;

    return false;
}

/// Returns true if the query shall be retried
bool DatabaseCoreBackendPrivate::checkDatabaseError(const SqlQuery& query)
{
    if (errorHandler)
    {
        handlingConnectionError = true;
        setQueryOperationFlag(DatabaseCoreBackend::Wait);
        if (isInUIThread())
        {
            errorHandler->databaseError(this, query);
        }
        else
        {
            ErrorLocker locker(this);
            bool called = QMetaObject::invokeMethod(errorHandler, "databaseError",
                                                    Qt::QueuedConnection, Q_ARG(DatabaseErrorAnswer*, this), Q_ARG(const SqlQuery, query));

            if (called)
            {
                locker.wait();
            }
            else
            {
                kError() << "Failed to invoke DatabaseErrorHandler. Aborting all queries.";
                operationStatus = DatabaseCoreBackend::AbortQueries;
            }
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
        //TODO check if it's better to use an own error handler for kio slaves.
        // But for now, close only the database in the hope, that the next
        // access will be successfull.
        closeDatabaseForThread();

    }
    return false;
}

void DatabaseCoreBackendPrivate::connectionErrorContinueQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    handlingConnectionError = false;
    queryOperationWakeAll(DatabaseCoreBackend::ExecuteNormal);
}

void DatabaseCoreBackendPrivate::connectionErrorAbortQueries()
{
    // Attention: called from out of context, maybe without any lock
    QMutexLocker l(&lock->mutex);
    handlingConnectionError = false;
    queryOperationWakeAll(DatabaseCoreBackend::AbortQueries);
}

// ---------------------------

DatabaseCoreBackend::DatabaseCoreBackend(const QString &backendName, DatabaseLocking *locking)
               : d_ptr(new DatabaseCoreBackendPrivate(this))
{
    d_ptr->init(backendName, locking);
}

DatabaseCoreBackend::DatabaseCoreBackend(const QString &backendName, DatabaseLocking *locking, DatabaseCoreBackendPrivate &dd)
               : d_ptr(&dd)
{
    d_ptr->init(backendName, locking);
}

DatabaseCoreBackend::~DatabaseCoreBackend()
{
    Q_D(DatabaseCoreBackend);
    delete d;
}

DatabaseConfigElement DatabaseCoreBackend::configElement() const
{
    Q_D(const DatabaseCoreBackend);
    return DatabaseConfigElement::element(d->parameters.databaseType);
}

DatabaseAction DatabaseCoreBackend::getDBAction(const QString &actionName) const
{
    DatabaseAction action = configElement().sqlStatements.value(actionName);
    if (action.name.isNull())
        kError() << "No DB action defined for" << actionName << "! Implementation missing for this database type.";
    return action;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDBAction(const DatabaseAction &action, QList<QVariant>* values,
                                                                  QVariant *lastInsertId)
{
    return execDBAction(action, QMap<QString, QVariant>(), values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDBAction(const DatabaseAction &action, const QMap<QString,
                                                                  QVariant>& bindingMap, QList<QVariant>* values, QVariant *lastInsertId)
{
    Q_D(DatabaseCoreBackend);

    DatabaseCoreBackend::QueryState returnResult = DatabaseCoreBackend::NoErrors;
    QSqlDatabase db = d->databaseForThread();

    if (action.name.isNull())
        kError() << "Attempt to execute null action";

    #ifdef DATABASCOREBACKEND_DEBUG
    kDebug(50003) << "Executing DBAction ["<<  action.name  <<"]";
    #endif

    bool wrapInTransaction = (action.mode == QString("transaction"));
    if (wrapInTransaction)
    {
        beginTransaction();
    }

    foreach (DatabaseActionElement actionElement, action.dbActionElements)
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
            kDebug(50003) << "Error while executing DBAction ["<<  action.name  <<"] Statement ["<<actionElement.statement<<"]";
            returnResult = result;

            /*
            if (wrapInTransaction && !db.rollback())
            {
                kDebug(50003) << "Error while rollback changes of previous DBAction.";
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
        kDebug(50003) << "Error while committing changes of previous DBAction.";
    }
    */

    return returnResult;
}

QSqlQuery DatabaseCoreBackend::execDBActionQuery(const DatabaseAction &action, const QMap<QString, QVariant>& bindingMap)
{
    Q_D(DatabaseCoreBackend);

    QSqlDatabase db = d->databaseForThread();

    #ifdef DATABASCOREBACKEND_DEBUG
    kDebug(50003) << "Executing DBAction ["<<  action.name  <<"]";
    #endif

    QSqlQuery result;
    foreach (DatabaseActionElement actionElement, action.dbActionElements)
    {
        if (actionElement.mode==QString("query"))
        {
            result = execQuery(actionElement.statement, bindingMap);
        }
        else
        {
            kDebug(50003) << "Error, only DBActions with mode 'query' are allowed at this call!";
        }

        if (result.lastError().isValid() && result.lastError().number())
        {
            kDebug(50003) << "Error while executing DBAction ["<<  action.name
                          <<"] Statement ["<<actionElement.statement<<"] Errornr. [" << result.lastError() << "]";
            break;
        }
    }
    return result;
}

void DatabaseCoreBackend::setDatabaseErrorHandler(DatabaseErrorHandler *handler)
{
    Q_D(DatabaseCoreBackend);
    if (handler!=0 && handler->thread() != QCoreApplication::instance()->thread())
    {
        kError(50003) << "DatabaseErrorHandler must live in the main thread";
        Q_ASSERT(false);
    }
    if (d->errorHandler!=0){
        d->errorHandler->~QObject();
    }
    d->errorHandler = handler;
}

void DatabaseCoreBackend::slotThreadFinished()
{
    Q_D(DatabaseCoreBackend);
    d->closeDatabaseForThread();
}

void DatabaseCoreBackend::slotMainThreadFinished()
{
    Q_D(DatabaseCoreBackend);
    d->closeDatabaseForThread();
}

bool DatabaseCoreBackend::isCompatible(const DatabaseParameters& parameters)
{
    return QSqlDatabase::drivers().contains(parameters.databaseType);
}

bool DatabaseCoreBackend::open(const DatabaseParameters& parameters)
{
    Q_D(DatabaseCoreBackend);
    d->parameters = parameters;

    // Force possibly opened thread dbs to re-open with new parameters.
    // They are not accessible from this thread!
    d->databasesValid.clear();

    int retries = 0;

    QSqlDatabase database = d->databaseForThread();
    SqlQuery query(database);

    forever
    {
        if (!database.isOpen())
        {
            kDebug(50003) << "Error while opening the database. Trying again.";
            if (queryErrorHandling(query, retries++))
            {
                // TODO reopen the database
                d->closeDatabaseForThread();
                database = d->databaseForThread();
                continue;
            }
            else
                return false;
        }else
            break;
    }
    d->status = Open;
    return true;
}

bool DatabaseCoreBackend::initSchema(ThumbnailSchemaUpdater *updater)
{
    Q_D(DatabaseCoreBackend);
    if (d->status == OpenSchemaChecked)
        return true;
    if (d->status == Unavailable)
        return false;
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
bool DatabaseCoreBackend::execSql(const QString& sql, QStringList* values)
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
    int count = record.count();
    
    while (query.next())
    {
        for (int i=0; i<count; ++i)	  
	  list << query.value(i);	
    }
#ifdef DATABASCOREBACKEND_DEBUG
    kDebug(50003) << "Setting result value list ["<< list <<"]";
#endif
    return list;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::handleQueryResult(SqlQuery &query, QList<QVariant>* values, QVariant *lastInsertId)
{
    if (!query.isActive())
    {
        if (query.lastError().type() == QSqlError::ConnectionError)
        {
            return DatabaseCoreBackend::ConnectionError;
        }
    }
        if (lastInsertId)
            (*lastInsertId) = query.lastInsertId();
        if (values)
            (*values) = readToList(query);
        return DatabaseCoreBackend::NoErrors;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, QList<QVariant>* values, QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, const QVariant& boundValue1,
                              QList<QVariant>* values, QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql,
                              const QVariant& boundValue1, const QVariant& boundValue2,
                              QList<QVariant>* values, QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql,
                              const QVariant& boundValue1, const QVariant& boundValue2, 
                              const QVariant& boundValue3, QList<QVariant>* values, 
                              QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql,
                const QVariant& boundValue1, const QVariant& boundValue2,
                const QVariant& boundValue3, const QVariant& boundValue4,
                QList<QVariant>* values, QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3, boundValue4);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, const QList<QVariant>& boundValues,
                              QList<QVariant>* values, QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql, boundValues);
    return handleQueryResult(query, values, lastInsertId);
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execSql(const QString& sql, const QMap<QString, QVariant> &bindingMap,
                                  QList<QVariant> *values, QVariant *lastInsertId)
{
    SqlQuery query = execQuery(sql, bindingMap);
    return handleQueryResult(query, values, lastInsertId);
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QVariant& boundValue1)
{
    SqlQuery query = prepareQuery(sql);
#ifdef DATABASCOREBACKEND_DEBUG
    kDebug(50003) << "Trying to sql ["<< sql <<"] query ["<<query.lastQuery()<<"]";
#endif
    query.bindValue(0, boundValue1);
    exec(query);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1, const QVariant& boundValue2)
{
    SqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    exec(query);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3)
{
    SqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    exec(query);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1, const QVariant& boundValue2,
                                     const QVariant& boundValue3, const QVariant& boundValue4)
{
    SqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    exec(query);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QList<QVariant>& boundValues)
{
    SqlQuery query = prepareQuery(sql);
    for (int i=0; i<boundValues.size(); ++i)
        query.bindValue(i, boundValues[i]);
    exec(query);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql)
{
    SqlQuery query = prepareQuery(sql);
#ifdef DATABASCOREBACKEND_DEBUG
    kDebug(50003)<<"execQuery: Using statement ["<< query.lastQuery() <<"]";
#endif
    exec(query);
    return query;
}

SqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap)
{
    QString preparedString = sql;
    QList<QVariant> namedPlaceholderValues;

    if (!bindingMap.isEmpty())
    {
        #ifdef DATABASCOREBACKEND_DEBUG
        kDebug(50003)<<"Prepare statement ["<< preparedString <<"] with binding map ["<< bindingMap <<"]";
        #endif

        QRegExp identifierRegExp(":[A-Za-z0-9]+");
        int pos=0;

        while ((pos=identifierRegExp.indexIn(preparedString, pos))!=-1)
        {
            QString namedPlaceholder = identifierRegExp.cap(0);
            namedPlaceholderValues.append(bindingMap[namedPlaceholder]);

            #ifdef DATABASCOREBACKEND_DEBUG
            kDebug(50003)<<"Bind key ["<< namedPlaceholder <<"] to value ["<< bindingMap[namedPlaceholder] <<"]";
            #endif

            preparedString = preparedString.replace(pos, identifierRegExp.matchedLength(), "?");
            pos=0; // reset pos
        }
    }
    #ifdef DATABASCOREBACKEND_DEBUG
    kDebug(50003)<<"Prepared statement ["<< preparedString <<"] values ["<< namedPlaceholderValues <<"]";
    #endif

    SqlQuery query = prepareQuery(preparedString);

    for(int i=0; i<namedPlaceholderValues.size(); i++)
    {
        query.bindValue(i, namedPlaceholderValues[i]);
    }

    exec(query);
    return query;
}

bool DatabaseCoreBackend::queryErrorHandling(const SqlQuery& query, int retries)
{
    Q_D(DatabaseCoreBackend);

    d->debugOutputFailedQuery(query);

    if (d->isSQLiteLockError(query))
    {
        if (d->checkRetrySQLiteLockError(retries))
            return true;
    }
    else
    {
        if (d->checkDatabaseError(query))
            return true;
        else
            return false;
    }
    return false;
}

DatabaseCoreBackend::QueryState DatabaseCoreBackend::execDirectSql(const QString& sql)
{
    Q_D(DatabaseCoreBackend);

    if (!d->checkOperationStatus())
        return DatabaseCoreBackend::SQLError;

    SqlQuery query = getQuery();

    int retries = 0;
    forever
    {
        if (query.exec(sql))
            break;
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                // Create a new database connection and retry
                d->closeDatabaseForThread();
                query = getQuery();
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
        return false;

    int retries = 0;
    forever
    {
        #ifdef DATABASCOREBACKEND_DEBUG
        kDebug(50003) << "Trying to query ["<<query.lastQuery()<<"] values ["<< query.boundValues() <<"]";
        #endif

        if (query.exec())
            break;
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                // TODO reopen the database
                d->closeDatabaseForThread();
                query = copyQuery(query);
                continue;
            }
            else
                return false;
        }
    }
    return true;
}

bool DatabaseCoreBackend::execBatch(SqlQuery& query)
{
    Q_D(DatabaseCoreBackend);

    if (!d->checkOperationStatus())
        return false;

    int retries = 0;
    forever
    {
        if (query.execBatch())
            break;
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                d->closeDatabaseForThread();
                query = copyQuery(query);
                continue;
            }
            else
                return false;
        }
    }
    return true;
}


SqlQuery DatabaseCoreBackend::prepareQuery(const QString& sql)
{
    Q_D(DatabaseCoreBackend);
    int retries=0;
    forever
    {
        SqlQuery query = getQuery();
        if (query.prepare(sql))
            return query;
        else
        {
            kDebug(50003) << "Prepare failed!";
            if (queryErrorHandling(query, retries++))
            {
                // TODO reopen the database
                d->closeDatabaseForThread();
                query = copyQuery(query);
                continue;
            }
            else
                return query;
        }
    }
}

SqlQuery DatabaseCoreBackend::copyQuery(const SqlQuery& old)
{
    SqlQuery query = getQuery();
    kDebug(50003) << "Last query was ["<<old.lastQuery()<<"]";
    query.prepare(old.lastQuery());
    query.setForwardOnly(old.isForwardOnly());
    // only for positional binding
    QList<QVariant> boundValues = old.boundValues().values();
    foreach (const QVariant &value, boundValues)
    {
        kDebug(50003) << "Bind value to query ["<<value<<"]";
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
    // Call databaseForThread before touching transaction count - open() will reset the count
    QSqlDatabase db = d->databaseForThread();
    if (d->incrementTransactionCount())
    {
        if (!db.transaction())
        {
            d->decrementTransactionCount();
            if (db.lastError().type() == QSqlError::ConnectionError)
            {
                return DatabaseCoreBackend::ConnectionError;
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
        if (!db.commit())
        {
            d->incrementTransactionCount();
            if (db.lastError().type() == QSqlError::ConnectionError)
            {
                return DatabaseCoreBackend::ConnectionError;
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
    return d->isInTransactionInOtherThread();
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
    return d->databaseForThread().lastError();
}

QString DatabaseCoreBackend::lastError()
{
    Q_D(DatabaseCoreBackend);
    return d->databaseForThread().lastError().text();
}

}  // namespace Digikam
