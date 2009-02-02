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

#ifndef DATABASEBACKEND_H
#define DATABASEBACKEND_H

// Qt includes.

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSqlQuery>

// Local includes.

#include "digikam_export.h"
#include "databaseparameters.h"
#include "databasechangesets.h"

namespace Digikam
{

class SchemaUpdater;
class DatabaseWatch;
class DatabaseBackendPriv;

class DIGIKAM_DATABASE_EXPORT DatabaseBackend : public QObject
{

Q_OBJECT

public:

    DatabaseBackend();
    ~DatabaseBackend();

    void setDatabaseWatch(DatabaseWatch *watch);

    /**
     * Checks if the parameters can be used for this database backend.
     */
    bool isCompatible(const DatabaseParameters &parameters);

    /**
     * Open the database connection.
     * @returns true on success
     */
    bool open(const DatabaseParameters &parameters);

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(SchemaUpdater *updater);

    /**
     * Close the database connection.
     * Shall only be called from the thread that called open().
     */
    void close();

    enum Status
    {
        /**
         * The database is not available, because it has not been
         * opened yet or because of an error condition.
         */
        Unavailable,
        /**
         * The database is open. It has not been verified that
         * the schema is up to date.
         * This status is sufficient for use in a context where it
         * can be assumed that the necessary schema check has been carried out
         * by a master process.
         */
        Open,
        /**
         * The database is open, and it has been verified that the schema is up to
         * date, or the schema has been updated.
         */
        OpenSchemaChecked
    };

    /**
     * Returns the current status of the database backend
     */
    Status status() const;

    bool isOpen() const { return status() > Unavailable; }
    bool isReady() const { return status() == OpenSchemaChecked; }

    /**
     * Executes the SQL statement, and write the returned data into the values list.
     * If you are not interested in the returned data, set values to 0.
     * Methods are provided for up to four bound values (positional binding), or for a list of bound values.
     * If you want the last inserted id (and your query is suitable), sett lastInsertId to the address of a QVariant.
     */
    bool execSql(const QString& sql, QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    bool execSql(const QString& sql, const QVariant &boundValue1,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    bool execSql(const QString& sql,
                 const QVariant &boundValue1, const QVariant &boundValue2,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    bool execSql(const QString& sql,
                 const QVariant &boundValue1, const QVariant &boundValue2, const QVariant &boundValue3,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    bool execSql(const QString& sql,
                 const QVariant &boundValue1, const QVariant &boundValue2,
                 const QVariant &boundValue3, const QVariant &boundValue4,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    bool execSql(const QString& sql, const QList<QVariant> &boundValues, QList<QVariant>* values = 0, QVariant *lastInsertId = 0);

    /**
     * Executes the statement and returns the query object.
     * Methods are provided for up to four bound values (positional binding), or for a list of bound values.
     */
    QSqlQuery execQuery(const QString& sql);
    QSqlQuery execQuery(const QString& sql, const QVariant &boundValue1);
    QSqlQuery execQuery(const QString& sql,
                        const QVariant &boundValue1, const QVariant &boundValue2);
    QSqlQuery execQuery(const QString& sql,
                        const QVariant &boundValue1, const QVariant &boundValue2, const QVariant &boundValue3);
    QSqlQuery execQuery(const QString& sql,
                        const QVariant &boundValue1, const QVariant &boundValue2,
                        const QVariant &boundValue3, const QVariant &boundValue4);
    QSqlQuery execQuery(const QString& sql, const QList<QVariant> &boundValues);

    /**
     * Calls exec/execBatch on the query, and handles debug output if something went wrong
     */
    bool exec(QSqlQuery &query);
    bool execBatch(QSqlQuery &query);

    /**
     * Creates a query object prepared with the statement, waiting for bound values
     */
    QSqlQuery prepareQuery(const QString &sql);

    QList<QVariant> readToList(QSqlQuery &query);

    /**
     * Begin a database transaction
     */
    bool beginTransaction();
    /**
     * Commit the current database transaction
     */
    bool commitTransaction();
    /**
     * Rollback the current database transaction
     */
    void rollbackTransaction();
    /**
     * Returns if the database is in a different thread in a transaction.
     * Note that a transaction does not require holding DatabaseAccess.
     * Note that this does not give information about other processes
     * locking the database.
     */
    bool isInTransaction() const;

    /**
     * Return a list with the names of the tables in the database
     */
    QStringList tables();

    /**
     * Returns a description of the last error that occurred on this database.
     * Use DatabaseAccess::lastError for errors presented to the user.
     * This error will be included in that message.
     * It may be empty.
     */
    QString lastError();

    /**
     * Notify all listeners of the changeset
     */
    void recordChangeset(const ImageChangeset changeset);
    void recordChangeset(const ImageTagChangeset changeset);
    void recordChangeset(const CollectionImageChangeset changeset);
    void recordChangeset(const AlbumChangeset changeset);
    void recordChangeset(const TagChangeset changeset);
    void recordChangeset(const AlbumRootChangeset changeset);
    void recordChangeset(const SearchChangeset changeset);

/*
    Qt SQL driver supported features
    SQLITE3:
        BLOB
        Transactions
        Unicode
        LastInsertId
        PreparedQueries
        PositionalPlaceholders
        SimpleLocking
    MySQL:
        Transactions (3.?)
        QuerySize
        BLOB
        LastInsertId
        Unicode
        PreparedQueries (4.1)
        PositionalPlaceholders (4.1)
    Postgresql:
        Transactions
        QuerySize
        LastInsertId
*/

private slots:

    void slotThreadFinished();
    void slotMainThreadFinished();

private:

    DatabaseBackendPriv * const d;
};

} // namespace Digikam

#endif // DATABASEBACKEND_H
