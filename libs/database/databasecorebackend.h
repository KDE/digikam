/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-07
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

#ifndef DATABASECOREBACKEND_H
#define DATABASECOREBACKEND_H

// Qt includes

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSqlQuery>

// Local includes

#include "digikam_export.h"
#include "databaseparameters.h"
#include "sqlquery.h"

namespace Digikam
{

class ThumbnailSchemaUpdater;
class DatabaseCoreBackendPrivate;

class DIGIKAM_EXPORT DatabaseErrorAnswer
{
public:
    virtual ~DatabaseErrorAnswer() {};
    virtual void connectionErrorContinueQueries() = 0;
    virtual void connectionErrorAbortQueries() = 0;
};

class DIGIKAM_EXPORT DatabaseErrorHandler : public QObject
{
    Q_OBJECT

public Q_SLOTS:

    /** In the situation of a connection error,
     *  all threads will be waiting with their queries
     *  and this method is called.
     *  This method can display an error dialog and try to repair
     *  the connection.
     *  It must then call either connectionErrorContinueQueries()
     *  or connectionErrorAbortQueries().
     */
    virtual void connectionError(DatabaseErrorAnswer *answer) = 0;

};

class DIGIKAM_EXPORT DatabaseLocking
{
public:
    DatabaseLocking();
    QMutex mutex;
    int    lockCount;
};

class DIGIKAM_EXPORT DatabaseCoreBackend : public QObject
{

Q_OBJECT

public:

    /** Creates a database backend. The backend name is an arbitrary string that
     *  shall be unique for this backend object.
     *  It will be used to create unique connection names per backend and thread.
     */
    DatabaseCoreBackend(const QString &backendName, DatabaseLocking *locking);
    DatabaseCoreBackend(const QString &backendName, DatabaseLocking *locking, DatabaseCoreBackendPrivate &dd);
    ~DatabaseCoreBackend();

    /**
     * Checks if the parameters can be used for this database backend.
     */
    bool isCompatible(const DatabaseParameters& parameters);

    /**
     * Open the database connection.
     * @returns true on success
     */
    bool open(const DatabaseParameters& parameters);

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(ThumbnailSchemaUpdater *updater);

    /**
     * Close the database connection.
     * Shall only be called from the thread that called open().
     */
    void close();

    enum QueryState
    {
        /**
         * No errors occurred while executing the query.
         */
        NoErrors,

        /**
         * An SQLError has occurred while executing the query.
         */
        SQLError,

        /**
         * An connection error has occured while executing the query.
         */
        ConnectionError
    };

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
     * Add a DatabaseErrorHandler. This object must be created in the main thread.
     * If a database error occurs, this object can handle problem solving and user interaction.
     */
    void setDatabaseErrorHandler(DatabaseErrorHandler *handler);

    enum QueryOperationStatus
    {
        ExecuteNormal,
        Wait,
        AbortQueries
    };

    /**
     * TODO: API docs
     */
    databaseAction getDBAction(const QString &actionName);
    bool execDBAction(const databaseAction &action, QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    bool execDBAction(const databaseAction &action, const QMap<QString, QVariant>& bindingMap,
                      QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    QSqlQuery execDBActionQuery(const databaseAction &action, const QMap<QString, QVariant>& bindingMap);

    /**
     * Executes the SQL statement, and write the returned data into the values list.
     * If you are not interested in the returned data, set values to 0.
     * Methods are provided for up to four bound values (positional binding), or for a list of bound values.
     * If you want the last inserted id (and your query is suitable), sett lastInsertId to the address of a QVariant.
     */
    QueryState execSql(const QString& sql, QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    QueryState execSql(const QString& sql, const QVariant& boundValue1,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    QueryState execSql(const QString& sql,
                 const QVariant& boundValue1, const QVariant& boundValue2,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    QueryState execSql(const QString& sql,
                 const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    QueryState execSql(const QString& sql,
                 const QVariant& boundValue1, const QVariant& boundValue2,
                 const QVariant& boundValue3, const QVariant& boundValue4,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    QueryState execSql(const QString& sql, const QList<QVariant>& boundValues, QList<QVariant>* values = 0, QVariant *lastInsertId = 0);

    QueryState handleQueryResult(SqlQuery &query, QList<QVariant>* values, QVariant *lastInsertId);

    /**
     * Method which accepts a map for named binding
     */
    QueryState execSql(const QString& sql, const QMap<QString, QVariant>& bindingMap,
                 QList<QVariant>* values = 0, QVariant *lastInsertId = 0);
    /**
     * Calls exec on the query, and handles debug output if something went wrong.
     * The query is not prepared, which can be fail in certain situations
     * (e.g. trigger statements on QMYSQL).
     */
    QueryState execDirectSql(const QString& query);



    /**
     * Executes the statement and returns the query object.
     * Methods are provided for up to four bound values (positional binding), or for a list of bound values.
     */
    SqlQuery execQuery(const QString& sql);
    SqlQuery execQuery(const QString& sql, const QVariant& boundValue1);
    SqlQuery execQuery(const QString& sql,
                        const QVariant& boundValue1, const QVariant& boundValue2);
    SqlQuery execQuery(const QString& sql,
                        const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3);
    SqlQuery execQuery(const QString& sql,
                        const QVariant& boundValue1, const QVariant& boundValue2,
                        const QVariant& boundValue3, const QVariant& boundValue4);
    SqlQuery execQuery(const QString& sql, const QList<QVariant>& boundValues);

    /**
     * Method which accept a hashmap with key, values which are used for named binding
     */
    SqlQuery execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap);


    /**
     * Calls exec/execBatch on the query, and handles debug output if something went wrong
     */
    bool exec(SqlQuery& query);
    bool execBatch(SqlQuery& query);

    /**
     * Creates a query object prepared with the statement, waiting for bound values
     */
    SqlQuery prepareQuery(const QString& sql);
    /**
     * Creates an empty query object waiting for the statement
     */
    SqlQuery getQuery();
    /**
     * Creates a faithful copy of the passed query, with the current db connection.
     */
    SqlQuery copyQuery(const SqlQuery& old);

    /**
     * Called with a failed query. Handles certain known errors and debug output.
     * If it returns true, reexecute the query; if it returns false, return it as failed.
     * Pass the number of retries already done for this query to help with some decisions.
     */
    bool queryErrorHandling(const SqlQuery& query, int retries);

    QList<QVariant> readToList(SqlQuery& query);

    /**
     * Begin a database transaction
     */
    DatabaseCoreBackend::QueryState beginTransaction();
    /**
     * Commit the current database transaction
     */
    DatabaseCoreBackend::QueryState commitTransaction();
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

private Q_SLOTS:

    void slotThreadFinished();
    void slotMainThreadFinished();

protected:

    DatabaseCoreBackendPrivate * const d_ptr;

private:

    Q_DECLARE_PRIVATE(DatabaseCoreBackend)
};

} // namespace Digikam

#endif // DATABASECOREBACKEND_H
