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

#include "databasecorebackend.h"
#include "databasecorebackend_p.h"
#include "databasecorebackend.moc"

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QSqlRecord>
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

DatabaseCoreBackendPrivate::DatabaseCoreBackendPrivate(DatabaseCoreBackend *backend)
            : q(backend)
{

    status          = DatabaseCoreBackend::Unavailable;
    isInTransaction = false;
    operationStatus = DatabaseCoreBackend::ExecuteNormal;
    handlingConnectionError = true;
}


void DatabaseCoreBackendPrivate::init(const QString &name, DatabaseLocking *l)
{
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                     q, SLOT(slotMainThreadFinished()));

    backendName = name;
    lock        = l;

    qRegisterMetaType<DatabaseErrorAnswer*>("DatabaseErrorAnswer*");
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
    if (!isValid)
    {
        // need to open a db for thread
        open(db);

        QObject::connect(thread, SIGNAL(finished()),
                            q, SLOT(slotThreadFinished()));
    }
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

    if (success==false){
        kWarning(50003) << "Error while opening the database. Error was <" << db.lastError() << ">";
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


    /** This suspends the current thread if the query status as
     *  set by setFlag() is Wait and until the thread is woken with wakeAll().
     *  The DatabaseAccess mutex will be unlocked while waiting.
     */
void DatabaseCoreBackendPrivate::queryOperationWait()
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.

    // acquire lock
    lock->mutex.lock();
    // store lock count
    int count = lock->lockCount;
    // set lock count to 0
    lock->lockCount = 0;
    // unlock
    for (int i=0; i<count; ++i)
        lock->mutex.unlock();

    // lock condvar mutex (lock only if main mutex is locked)
    errorLockMutex.lock();
    // drop lock acquired in first line. Main mutex is now free.
    // We maintain lock order (first main mutex, second error lock mutex)
    // but we drop main mutex lock for waiting on the cond var.
    lock->mutex.unlock();

    // we need a copy of the flag under lock of the errorLockMutex to be able to check it here
    while (errorLockOperationStatus == DatabaseCoreBackend::Wait)
        errorLockCondVar.wait(&errorLockMutex);  

    // unlock condvar mutex. Both mutexes are now free.
    errorLockMutex.unlock();

    // lock main mutex as often as it was locked before
    for (int i=0; i<count; ++i)
        lock->mutex.lock();
    // update lock count
    lock->lockCount = count;
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
        queryOperationWait();

    if (operationStatus == DatabaseCoreBackend::ExecuteNormal)
        return true;
    else if (operationStatus == DatabaseCoreBackend::AbortQueries)
        return false;

    return false;
}

bool DatabaseCoreBackendPrivate::handleConnectionError()
{
    if (errorHandler)
    {
        handlingConnectionError = true;
        setQueryOperationFlag(DatabaseCoreBackend::Wait);
        if (isInUIThread())
        {
            errorHandler->connectionError(this);
        }
        else
        {
            QMetaObject::invokeMethod(errorHandler, SLOT(connectionError(DatabaseErrorAnswer *)),
                                      Qt::QueuedConnection, Q_ARG(DatabaseErrorAnswer*, this));
            queryOperationWait();
        }
        return true;
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

databaseAction DatabaseCoreBackend::getDBAction(const QString &actionName)
{
    Q_D(DatabaseCoreBackend);
    return d->parameters.m_DatabaseConfigs[d->parameters.databaseType].m_SQLStatements[actionName];
}

bool DatabaseCoreBackend::execDBAction(const databaseAction &action, QList<QVariant>* values, QVariant *lastInsertId)
{
    return execDBAction(action, QMap<QString, QVariant>(), values, lastInsertId);
}

bool DatabaseCoreBackend::execDBAction(const databaseAction &action, const QMap<QString, QVariant>& bindingMap,
                                       QList<QVariant>* values, QVariant *lastInsertId)
{
    Q_D(DatabaseCoreBackend);

    bool returnResult = true;
    QSqlDatabase db = d->databaseForThread();

    kDebug(50003) << "Executing DBAction ["<<  action.m_Name  <<"]";
    bool wrapInTransaction = (action.m_Mode == QString("transaction"));
    if (wrapInTransaction)
    {
        db.transaction();
    }

    foreach (databaseActionElement actionElement, action.m_DBActionElements)
    {
        bool result;
        if (actionElement.m_Mode==QString("query"))
        {
            result = execSql(actionElement.m_Statement, bindingMap, values, lastInsertId);
        }
        else
        {
            result = exec(actionElement.m_Statement);
        }
        if (result)
        {
            if (wrapInTransaction)
                db.commit();
        }
        else
        {
            kDebug(50003) << "Error while executing DBAction ["<<  action.m_Name  <<"] Statement ["<<actionElement.m_Statement<<"]";
            returnResult = result;
            if (wrapInTransaction)
                db.rollback();
            break;
        }
    }
    return returnResult;
}

void DatabaseCoreBackend::setDatabaseErrorHandler(DatabaseErrorHandler *handler)
{
    Q_D(DatabaseCoreBackend);
    if (handler->thread() != QCoreApplication::instance()->thread())
    {
        kError(50003) << "DatabaseErrorHandler must live in the main thread";
        Q_ASSERT(false);
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

    if (!d->databaseForThread().isOpen())
        return false;

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

QList<QVariant> DatabaseCoreBackend::readToList(QSqlQuery& query)
{
    QList<QVariant> list;
    
    QSqlRecord record = query.record();
    int count = record.count();
    
    while (query.next())
    {
        for (int i=0; i<count; ++i)	  
	  list << query.value(i);	
    }
    
    kDebug(50003) << "Setting result value list ["<< list <<"]";
    return list;
}

bool DatabaseCoreBackend::execSql(const QString& sql, QList<QVariant>* values, QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql);
    if (!query.isActive())
        return false;
    if (lastInsertId)
        (*lastInsertId) = query.lastInsertId();
    if (values)
        (*values) = readToList(query);
    return true;
}

bool DatabaseCoreBackend::execSql(const QString& sql, const QVariant& boundValue1,
                              QList<QVariant>* values, QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1);
    if (!query.isActive())
        return false;
    if (lastInsertId)
        (*lastInsertId) = query.lastInsertId();
    if (values)
        (*values) = readToList(query);
    return true;
}

bool DatabaseCoreBackend::execSql(const QString& sql,
                              const QVariant& boundValue1, const QVariant& boundValue2,
                              QList<QVariant>* values, QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1, boundValue2);
    if (!query.isActive())
        return false;
    if (lastInsertId)
        (*lastInsertId) = query.lastInsertId();
    if (values)
        (*values) = readToList(query);
    return true;
}

bool DatabaseCoreBackend::execSql(const QString& sql,
                              const QVariant& boundValue1, const QVariant& boundValue2, 
                              const QVariant& boundValue3, QList<QVariant>* values, 
                              QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3);
    if (!query.isActive())
        return false;
    if (lastInsertId)
        (*lastInsertId) = query.lastInsertId();
    if (values)
        (*values) = readToList(query);
    return true;
}

bool DatabaseCoreBackend::execSql(const QString& sql,
                const QVariant& boundValue1, const QVariant& boundValue2,
                const QVariant& boundValue3, const QVariant& boundValue4,
                QList<QVariant>* values, QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3, boundValue4);
    if (!query.isActive())
        return false;
    if (lastInsertId)
        (*lastInsertId) = query.lastInsertId();
    if (values)
        (*values) = readToList(query);
    return true;
}

bool DatabaseCoreBackend::execSql(const QString& sql, const QList<QVariant>& boundValues,
                              QList<QVariant>* values, QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValues);
    if (!query.isActive())
        return false;
    if (lastInsertId)
        (*lastInsertId) = query.lastInsertId();
    if (values)
        (*values) = readToList(query);
    return true;
}

bool DatabaseCoreBackend::execSql(const QString& sql, const QMap<QString, QVariant> &bindingMap,
                                  QList<QVariant> *values, QVariant *lastInsertId)
{
    QSqlQuery query = execQuery(sql, bindingMap);
    if (!query.isActive())
        return false;
    if (lastInsertId)
    {
        (*lastInsertId) = query.lastInsertId();
        kDebug(50003) << "Last Insert ID was ["<<*lastInsertId<<"]";
    }
    if (values)
    {
        (*values) = readToList(query);
    }
    return true;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QVariant& boundValue1)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    exec(query);
    return query;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1, const QVariant& boundValue2)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    exec(query);
    return query;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    exec(query);
    return query;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1, const QVariant& boundValue2,
                                     const QVariant& boundValue3, const QVariant& boundValue4)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    exec(query);
    return query;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QList<QVariant>& boundValues)
{
    QSqlQuery query = prepareQuery(sql);
    for (int i=0; i<boundValues.size(); ++i)
        query.bindValue(i, boundValues[i]);
    exec(query);
    return query;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql)
{
    QSqlQuery query = prepareQuery(sql);
    exec(query);
    return query;
}

QSqlQuery DatabaseCoreBackend::execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap)
{
    QString preparedString = sql;
    QList<QVariant> namedPlaceholderValues;

    if (!bindingMap.isEmpty())
    {
        kDebug(50003)<<"Prepare statement ["<< preparedString <<"]";
        QRegExp identifierRegExp(":[A-Za-z0-9]+");
        int pos=0;

        while ((pos=identifierRegExp.indexIn(preparedString, pos))!=-1)
        {
            QString namedPlaceholder = identifierRegExp.cap(0);
            namedPlaceholderValues.append(bindingMap[namedPlaceholder]);
            kDebug(50003)<<"Bind key ["<< namedPlaceholder <<"] to value ["<< bindingMap[namedPlaceholder] <<"]";
            preparedString = preparedString.replace(pos, identifierRegExp.matchedLength(), "?");
            pos=0; // reset pos
        }
    }
    kDebug(50003)<<"Prepared statement ["<< preparedString <<"] values ["<< namedPlaceholderValues <<"]";

    QSqlQuery query = prepareQuery(preparedString);

    for(int i=0; i<namedPlaceholderValues.size(); i++)
    {
        query.bindValue(i, namedPlaceholderValues[i]);
    }

    exec(query);

    return query;
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

bool DatabaseCoreBackend::exec(const QString& sql)
{
    Q_D(DatabaseCoreBackend);
    QSqlDatabase db = d->databaseForThread();
    QSqlQuery query(db);
    query.setForwardOnly(true);

    if (!query.exec(sql))
    {
        if (d->parameters.isSQLite() && query.lastError().number() == 5)
        {
            QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
            if (!app)
            {
                int limit = 10;
                kDebug(50003) << "Detected locked database file. Waiting 10s trying again.";

                do
                {
                    sotoSleep::sleep(1);
                    --limit;
                    if (query.exec())
                        return true;
                }
                while (limit > 0 && query.lastError().number() == 5);
            }
            else
                kWarning(50003) << "Detected locked database file. There is an active transaction.";
        }
        kDebug(50003) << "Failure executing query: ";
        kDebug(50003) << query.executedQuery();
        kDebug(50003) << query.lastError().text() << query.lastError().number() << query.lastError().databaseText() << query.lastError().driverText();
        kDebug(50003) << "Bound values: " << query.boundValues().values();
        return false;
    }
    return true;
}

bool DatabaseCoreBackend::exec(QSqlQuery& query)
{
    Q_D(DatabaseCoreBackend);

    if (!d->checkOperationStatus())
        return false;

    if (!query.exec())
    {
        if (false /*detect a connection error here*/)
        {
            if (!d->handleConnectionError())
                return false;
            switch (d->operationStatus)
            {
                case ExecuteNormal:
                    if (query.exec())
                        return true;
                    break;
                case AbortQueries:
                    return false;
                case Wait:
                    return exec(query);
            }
        }
        else if (d->parameters.isSQLite() && query.lastError().number() == 5)
        {
            QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
            if (!app)
            {
                int limit = 10;
                kDebug(50003) << "Detected locked database file. Waiting 10s trying again.";

                do
                {
                    sotoSleep::sleep(1);
                    --limit;
                    if (query.exec())
                        return true;
                }
                while (limit > 0 && query.lastError().number() == 5);
            }
            else
                kWarning(50003) << "Detected locked database file. There is an active transaction.";
        }
        kDebug(50003) << "Failure executing query: ";
        kDebug(50003) << query.executedQuery();
        kDebug(50003) << query.lastError().text() << query.lastError().number() << query.lastError().databaseText() << query.lastError().driverText();
        kDebug(50003) << "Bound values: " << query.boundValues().values();
        return false;
    }
    return true;
}

bool DatabaseCoreBackend::execBatch(QSqlQuery& query)
{
    if (!query.execBatch())
    {
        // Use DatabaseAccess::lastError?
        kDebug(50003) << "Failure executing batch query: ";
        kDebug(50003) << query.executedQuery();
        kDebug(50003) << query.lastError().text() << query.lastError().number();
        kDebug(50003) << "Bound values: " << query.boundValues().values();
        return false;
    }
    return true;
}


QSqlQuery DatabaseCoreBackend::prepareQuery(const QString& sql)
{
    Q_D(DatabaseCoreBackend);
    QSqlDatabase db = d->databaseForThread();
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare(sql);
    return query;
}

bool DatabaseCoreBackend::beginTransaction()
{
    Q_D(DatabaseCoreBackend);
    // Call databaseForThread before touching transaction count - open() will reset the count
    QSqlDatabase db = d->databaseForThread();
    if (d->incrementTransactionCount())
    {
        if (!db.transaction())
        {
            d->decrementTransactionCount();
            return false;
        }
        d->isInTransaction = true;
    }
    return true;
}

bool DatabaseCoreBackend::commitTransaction()
{
    Q_D(DatabaseCoreBackend);
    if (d->decrementTransactionCount())
    {
        if (!d->databaseForThread().commit())
        {
            d->incrementTransactionCount();
            return false;
        }
        d->isInTransaction = false;
        d->transactionFinished();
    }
    return true;
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

QString DatabaseCoreBackend::lastError()
{
    Q_D(DatabaseCoreBackend);
    return d->databaseForThread().lastError().text();
}

}  // namespace Digikam
