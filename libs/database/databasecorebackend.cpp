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
#include "databasecorebackend.moc"
#include "databasecorebackend_p.h"

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "thumbnailschemaupdater.h"

namespace Digikam
{

DatabaseCoreBackendPrivate::DatabaseCoreBackendPrivate(DatabaseCoreBackend *backend)
            : q(backend)
{

    status          = DatabaseCoreBackend::Unavailable;
    isInTransaction = false;
}

void DatabaseCoreBackendPrivate::init()
{
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                     q, SLOT(slotMainThreadFinished()));
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


// ---------------------------


DatabaseCoreBackend::DatabaseCoreBackend()
               : d_ptr(new DatabaseCoreBackendPrivate(this))
{
    d_ptr->init();
}

DatabaseCoreBackend::DatabaseCoreBackend(DatabaseCoreBackendPrivate &dd)
               : d_ptr(&dd)
{
    d_ptr->init();
}

DatabaseCoreBackend::~DatabaseCoreBackend()
{
    Q_D(DatabaseCoreBackend);
    delete d;
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
    int count = query.record().count();
    while (query.next())
    {
        for (int i=0; i<count; ++i)
            list << query.value(i);
    }
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

bool DatabaseCoreBackend::exec(QSqlQuery& query)
{
    Q_D(DatabaseCoreBackend);
    if (!query.exec())
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
                kWarning(50003) << "Detected locked database file. There is an active transaction." << endl;
        }
        kDebug(50003) << "Failure executing query: " << endl;
        kDebug(50003) << query.executedQuery() << endl;
        kDebug(50003) << query.lastError().text() << query.lastError().number() << endl;
        kDebug(50003) << "Bound values: " << query.boundValues().values() << endl;
        return false;
    }
    return true;
}

bool DatabaseCoreBackend::execBatch(QSqlQuery& query)
{
    if (!query.execBatch())
    {
        // Use DatabaseAccess::lastError?
        kDebug(50003) << "Failure executing batch query: " << endl;
        kDebug(50003) << query.executedQuery() << endl;
        kDebug(50003) << query.lastError().text() << query.lastError().number() << endl;
        kDebug(50003) << "Bound values: " << query.boundValues().values() << endl;
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
