/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Abstract database backend
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QCoreApplication>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>

// KDE includes

#include <kglobal.h>

// Local includes.

#include "ddebug.h"
#include "databasebackend.h"
#include "schemaupdater.h"

namespace Digikam
{

class DatabaseBackendPriv
{
public:

    DatabaseBackendPriv(DatabaseBackend *backend)
    {
        q               = backend;
        status          = DatabaseBackend::Unavailable;
    }

    // "A connection can only be used from within the thread that created it.
    //  Moving connections between threads or creating queries from a different thread is not supported."
    // => one QSqlDatabase object per thread.
    // The main class' open/close methods only interact with the "firstDatabase" object.
    // When another thread requests a DB, a new connection is opened and closed at
    // finishing of the thread.

    inline QString connectionName(QThread *thread)
    {
        return QString("digikamDatabase-" + QString::number((quintptr)thread));
    }

    QSqlDatabase databaseForThread()
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

    void closeDatabaseForThread()
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
        QSqlDatabase::removeDatabase(connectionName(thread));
    }

    void closeFirstDatabase()
    {
        QThread *thread = QThread::currentThread();
        firstDatabase.close();
        threadDatabases.remove(thread);
        databasesValid[thread] = 0;
        firstDatabase = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName(thread));
    }

    bool open(QSqlDatabase &db)
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

        threadDatabases[thread] = db;
        databasesValid[thread] = 1;

        return success;
    }

    QSqlDatabase firstDatabase;
    // this is always accessed in mutex context, no need for QThreadStorage
    QHash<QThread*, QSqlDatabase> threadDatabases;
    // this is not only db.isValid(), but also "parameters changed, need to reopen"
    QHash<QThread*, int> databasesValid;

    DatabaseParameters parameters;

    DatabaseBackend::Status status;

    qlonglong lastInsertedRow;

    DatabaseBackend *q;

};

DatabaseBackend::DatabaseBackend()
{
    d = new DatabaseBackendPriv(this);
}

DatabaseBackend::~DatabaseBackend()
{
    delete d;
}

void DatabaseBackend::slotThreadFinished()
{
    d->closeDatabaseForThread();
}

void DatabaseBackend::slotMainThreadFinished()
{
    d->closeFirstDatabase();
}

bool DatabaseBackend::isCompatible(const DatabaseParameters &parameters)
{
    return QSqlDatabase::drivers().contains(parameters.databaseType);
}

bool DatabaseBackend::open(const DatabaseParameters &parameters)
{
    if (QThread::currentThread() != QCoreApplication::instance()->thread())
        DWarning() << "DatabaseBackend::open is not called from the main thread! Fix this!" << endl;

    d->parameters = parameters;

    // Force possibly opened thread dbs to re-open with new parameters.
    // They are not accessible from this thread!
    d->databasesValid.clear();

    if (!d->open(d->firstDatabase))
        return false;

    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
            this, SLOT(slotThreadFinished()));

    d->status = Open;
    return true;
}

bool DatabaseBackend::initSchema(SchemaUpdater *updater)
{
    if (d->status == OpenSchemaChecked)
        return true;
    if (d->status == Unavailable)
        return false;
    if (updater->update())
        d->status = OpenSchemaChecked;
    return true;
}

void DatabaseBackend::close()
{
    d->closeDatabaseForThread();
}

DatabaseBackend::Status DatabaseBackend::status() const
{
    return d->status;
}

bool DatabaseBackend::execSql(const QString& sql, QStringList* values)
{
    QSqlQuery query = execQuery(sql);
    if (!query.isActive())
        return false;
    if (!values)
        return true;
    int count = query.record().count();
    while (query.next())
    {
        for (int i=0; i<count; i++)
            (*values) << query.value(i).toString();
    }
    return true;
}

QList<QVariant> DatabaseBackend::readToList(QSqlQuery &query)
{
    QList<QVariant> list;
    int count = query.record().count();
    while (query.next())
    {
        for (int i=0; i<count; i++)
            list << query.value(i);
    }
    return list;
}

bool DatabaseBackend::execSql(const QString& sql, QList<QVariant>* values, QVariant *lastInsertId)
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

bool DatabaseBackend::execSql(const QString& sql, const QVariant &boundValue1,
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

bool DatabaseBackend::execSql(const QString& sql,
                const QVariant &boundValue1, const QVariant &boundValue2,
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

bool DatabaseBackend::execSql(const QString& sql,
                const QVariant &boundValue1, const QVariant &boundValue2, const QVariant &boundValue3,
                QList<QVariant>* values, QVariant *lastInsertId)
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

bool DatabaseBackend::execSql(const QString& sql,
                const QVariant &boundValue1, const QVariant &boundValue2,
                const QVariant &boundValue3, const QVariant &boundValue4,
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

bool DatabaseBackend::execSql(const QString& sql, const QList<QVariant> &boundValues,
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



QSqlQuery DatabaseBackend::execQuery(const QString& sql, const QVariant &boundValue1)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.exec();
    return query;
}

QSqlQuery DatabaseBackend::execQuery(const QString& sql,
                                     const QVariant &boundValue1, const QVariant &boundValue2)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.exec();
    return query;
}

QSqlQuery DatabaseBackend::execQuery(const QString& sql,
                                     const QVariant &boundValue1, const QVariant &boundValue2, const QVariant &boundValue3)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.exec();
    return query;
}

QSqlQuery DatabaseBackend::execQuery(const QString& sql,
                                     const QVariant &boundValue1, const QVariant &boundValue2,
                                     const QVariant &boundValue3, const QVariant &boundValue4)
{
    QSqlQuery query = prepareQuery(sql);
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    query.exec();
    return query;
}

QSqlQuery DatabaseBackend::execQuery(const QString& sql, const QList<QVariant> &boundValues)
{
    QSqlQuery query = prepareQuery(sql);
    for (int i=0; i<boundValues.size(); i++)
        query.bindValue(i, boundValues[i]);
    query.exec();
    return query;
}

QSqlQuery DatabaseBackend::execQuery(const QString& sql)
{
    QSqlDatabase db = d->databaseForThread();
    QSqlQuery query(db);
    query.exec(sql);
    return query;
}

QSqlQuery DatabaseBackend::prepareQuery(const QString &sql)
{
    QSqlDatabase db = d->databaseForThread();
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare(sql);
    return query;
}

void DatabaseBackend::beginTransaction()
{
    d->databaseForThread().transaction();
}

void DatabaseBackend::commitTransaction()
{
    d->databaseForThread().commit();
}

void DatabaseBackend::rollbackTransaction()
{
    d->databaseForThread().rollback();
}

QStringList DatabaseBackend::tables()
{
    return d->databaseForThread().tables();
}

QString DatabaseBackend::lastError()
{
    return d->databaseForThread().lastError().text();
}

}  // namespace Digikam
