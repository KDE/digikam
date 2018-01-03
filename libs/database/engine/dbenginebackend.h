/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-07
 * Description : Database engine abstract database backend
 *
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASE_ENGINE_BACKEND_H
#define DATABASE_ENGINE_BACKEND_H

// Qt includes

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSqlQuery>
#include <QSqlError>

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbenginesqlquery.h"

namespace Digikam
{

class DbEngineConfigSettings;
class BdEngineBackendPrivate;
class DbEngineErrorHandler;

class DIGIKAM_EXPORT DbEngineLocking
{
public:

    DbEngineLocking();

public:

    QMutex mutex;
    int    lockCount;
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT BdEngineBackend : public QObject
{
    Q_OBJECT

public:

    enum QueryStateEnum
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
         * An connection error has occurred while executing the query.
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

    enum QueryOperationStatus
    {
        ExecuteNormal,
        Wait,
        AbortQueries
    };

    enum DbType{
        SQLite,
        MySQL
    };

public:

    /** Creates a database backend. The backend name is an arbitrary string that
     *  shall be unique for this backend object.
     *  It will be used to create unique connection names per backend and thread.
     */
    BdEngineBackend(const QString& backendName, DbEngineLocking* const locking);
    BdEngineBackend(const QString& backendName, DbEngineLocking* const locking, BdEngineBackendPrivate& dd);
    ~BdEngineBackend();

    /**
     * Checks if the parameters can be used for this database backend.
     */
    bool isCompatible(const DbEngineParameters& parameters);

    /**
     * Open the database connection.
     * @returns true on success
     */
    bool open(const DbEngineParameters& parameters);

    /**
     * Close the database connection.
     * Shall only be called from the thread that called open().
     */
    void close();

public:

    class QueryState
    {
    public:

        QueryState()
            : value(BdEngineBackend::NoErrors)
        {
        }

        QueryState(const QueryStateEnum value)
            : value(value)
        {
        }

        operator QueryStateEnum() const
        {
            return value;
        }

        operator bool() const
        {
            return value == BdEngineBackend::NoErrors;
        }

    private:

        QueryStateEnum value;
    };

public:

    /**
     * Returns the current status of the database backend
     */
    Status status() const;

    bool isOpen() const
    {
        return status() > Unavailable;
    }

    bool isReady() const
    {
        return status() == OpenSchemaChecked;
    }

    /**
     * Add a DbEngineErrorHandler. This object must be created in the main thread.
     * If a database error occurs, this object can handle problem solving and user interaction.
     */
    void setDbEngineErrorHandler(DbEngineErrorHandler* const handler);

    /**
      * Return config read from XML,
      * corresponding to this backend's database type.
      */
    DbEngineConfigSettings configElement() const;

    /**
     * Return the database type.
     */
    DbType databaseType() const;

    /**
     * Returns a database action with name, specified in actionName,
     * for the current database.
     */
    DbEngineAction getDBAction(const QString& actionName) const;

    /**
     * Performs the database action on the current database.
     * Queries by the specified parameters mustn't have named parameters.
     * The result values (if any) are stored within the values list.
     */
    QueryState execDBAction(const DbEngineAction& action, QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    QueryState execDBAction(const QString& action, QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    /**
     * Performs the database action on the current database.
     * Queries by the specified parameters can have named parameters which are
     * substituded with values from the bindingMap parameter.
     * The result values (if any) are stored within the values list.
     */
    QueryState execDBAction(const DbEngineAction& action, const QMap<QString, QVariant>& bindingMap,
                            QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    QueryState execDBAction(const QString& action, const QMap<QString, QVariant>& bindingMap,
                            QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    /**
     * Performs a special DBAction that is usually needed to "INSERT or UPDATE" entries in a table.
     * The corresponding DBAction must contain exactly the named parameters :id, :fieldValueList,
     * :fieldList and :valueList.
     * You pass the value to be bound to the ":id" field, then two lists of the same size:
     * The first containing the field names, the second one
     * containing the values as QVariants ready for binding.
     */
    QueryState execUpsertDBAction(const DbEngineAction& action, const QVariant& id,
                                  const QStringList fieldNames, const QList<QVariant>& values);
    QueryState execUpsertDBAction(const QString& action, const QVariant& id,
                                  const QStringList fieldNames, const QList<QVariant>& values);

    /**
     * Performs the database action on the current database.
     * Queries by the specified parameters can have named parameters which are
     * substituded with values from the bindingMap parameter.
     * The result values (if any) are stored within the values list.
     * This method returns the last query, which is used to handle special cases.
     */
    QSqlQuery execDBActionQuery(const DbEngineAction& action, const QMap<QString, QVariant>& bindingMap);

    QSqlQuery execDBActionQuery(const QString& action, const QMap<QString, QVariant>& bindingMap);

    /**
     * Executes the SQL statement, and write the returned data into the values list.
     * If you are not interested in the returned data, set values to 0.
     * Methods are provided for up to four bound values (positional binding), or for a list of bound values.
     * If you want the last inserted id (and your query is suitable), set lastInsertId to the address of a QVariant.
     * Additionally, methods are provided for prepared statements.
     */
    QueryState execSql(const QString& sql, QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(const QString& sql, const QVariant& boundValue1,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(const QString& sql,
                       const QVariant& boundValue1, const QVariant& boundValue2,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(const QString& sql,
                       const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(const QString& sql,
                       const QVariant& boundValue1, const QVariant& boundValue2,
                       const QVariant& boundValue3, const QVariant& boundValue4,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(const QString& sql, const QList<QVariant>& boundValues,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    QueryState execSql(DbEngineSqlQuery& preparedQuery, QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(DbEngineSqlQuery& preparedQuery, const QVariant& boundValue1,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(DbEngineSqlQuery& preparedQuery,
                       const QVariant& boundValue1, const QVariant& boundValue2,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(DbEngineSqlQuery& preparedQuery,
                       const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(DbEngineSqlQuery& preparedQuery,
                       const QVariant& boundValue1, const QVariant& boundValue2,
                       const QVariant& boundValue3, const QVariant& boundValue4,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    QueryState execSql(DbEngineSqlQuery& preparedQuery, const QList<QVariant>& boundValues,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    /**
     * Checks if there was a connection error. If so BdEngineBackend::ConnectionError is returned.
     * If not, the values are extracted from the query and inserted in the values list,
     * the last insertion id is taken from the query
     * and BdEngineBackend::NoErrors is returned.
     */
    QueryState handleQueryResult(DbEngineSqlQuery& query, QList<QVariant>* const values, QVariant* const lastInsertId);

    /**
     * Method which accepts a map for named binding.
     * For special cases it's also possible to add a DbEngineActionType which wraps another
     * data object (also lists or maps) which can be used as field entry or as value
     * (where it's prepared with positional binding). See more on DbEngineActionType class.
     * If the wrapped data object is an instance of list, then the elements are
     * separated by comma.
     * If the wrapped data object is an instance of map, then the elements are
     * inserted in the following way: key1=value1, key2=value2,...,keyN=valueN.
     */
    QueryState execSql(const QString& sql, const QMap<QString, QVariant>& bindingMap,
                       QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);
    /**
     * Calls exec on the query, and handles debug output if something went wrong.
     * The query is not prepared, which can be fail in certain situations
     * (e.g. trigger statements on QMYSQL).
     */
    QueryState execDirectSql(const QString& query);

    /**
     * Calls exec on the query, and handles debug output if something went wrong.
     * The query is not prepared, which can be fail in certain situations
     * (e.g. trigger statements on QMYSQL).
     */
    QueryState execDirectSqlWithResult(const QString& query, QList<QVariant>* const values = 0, QVariant* const lastInsertId = 0);

    /**
     * Executes the statement and returns the query object.
     * Methods are provided for up to four bound values (positional binding), or for a list of bound values.
     */
    DbEngineSqlQuery execQuery(const QString& sql);
    DbEngineSqlQuery execQuery(const QString& sql, const QVariant& boundValue1);
    DbEngineSqlQuery execQuery(const QString& sql,
                       const QVariant& boundValue1, const QVariant& boundValue2);
    DbEngineSqlQuery execQuery(const QString& sql,
                       const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3);
    DbEngineSqlQuery execQuery(const QString& sql,
                       const QVariant& boundValue1, const QVariant& boundValue2,
                       const QVariant& boundValue3, const QVariant& boundValue4);
    DbEngineSqlQuery execQuery(const QString& sql, const QList<QVariant>& boundValues);

    /**
     * Binds the values and executes the prepared query.
     */
    void execQuery(DbEngineSqlQuery& preparedQuery, const QVariant& boundValue1);
    void execQuery(DbEngineSqlQuery& preparedQuery,
                   const QVariant& boundValue1, const QVariant& boundValue2);
    void execQuery(DbEngineSqlQuery& preparedQuery,
                   const QVariant& boundValue1, const QVariant& boundValue2, const QVariant& boundValue3);
    void execQuery(DbEngineSqlQuery& preparedQuery,
                   const QVariant& boundValue1, const QVariant& boundValue2,
                   const QVariant& boundValue3, const QVariant& boundValue4);
    void execQuery(DbEngineSqlQuery& preparedQuery, const QList<QVariant>& boundValues);

    /**
     * Method which accept a hashmap with key, values which are used for named binding
     */
    DbEngineSqlQuery execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap);

    /**
     * Calls exec/execBatch on the query, and handles debug output if something went wrong
     */
    bool exec(DbEngineSqlQuery& query);
    bool execBatch(DbEngineSqlQuery& query);

    /**
     * Creates a query object prepared with the statement, waiting for bound values
     */
    DbEngineSqlQuery prepareQuery(const QString& sql);
    /**
     * Creates an empty query object waiting for the statement
     */
    DbEngineSqlQuery getQuery();
    /**
     * Creates a faithful copy of the passed query, with the current db connection.
     */
    DbEngineSqlQuery copyQuery(const DbEngineSqlQuery& old);

    /**
     * Called with a failed query. Handles certain known errors and debug output.
     * If it returns true, reexecute the query; if it returns false, return it as failed.
     * Pass the number of retries already done for this query to help with some decisions.
     */
    bool queryErrorHandling(DbEngineSqlQuery& query, int retries);
    bool transactionErrorHandling(const QSqlError& lastError, int retries);

    /**
     * Called when an attempted connection to the database failed.
     * If it returns true, retry; if it returns false, bail out.
     * Pass the number of connection retries to help with some decisions.
     */
    bool connectionErrorHandling(int retries);

    /**
     * Reads data of returned result set into a list which is returned.
     * The read process is column wise, which means
     * all data elements of a row is read, then the resultset is switched
     * to the next row.
     */
    QList<QVariant> readToList(DbEngineSqlQuery& query);

    /**
     * Begin a database transaction
     */
    BdEngineBackend::QueryState beginTransaction();
    /**
     * Commit the current database transaction
     */
    BdEngineBackend::QueryState commitTransaction();
    /**
     * Rollback the current database transaction
     */
    void rollbackTransaction();
    /**
     * Returns if the database is in a different thread in a transaction.
     * Note that a transaction does not require holding CoreDbAccess.
     * Note that this does not give information about other processes
     * locking the database.
     */
    bool isInTransaction() const;

    /**
     * Returns a list with the names of tables in the database.
     */
    QStringList tables();

    /**
     * Returns a description of the last error that occurred on this database.
     * Use CoreDbAccess::lastError for errors presented to the user.
     * This error will be included in that message.
     * It may be empty.
     */
    QString lastError();

    /**
     * Returns the last error that occurred on this database.
     * Use CoreDbAccess::lastError for errors presented to the user.
     * It may be empty.
     */
    QSqlError lastSQLError();

    /**
     * Returns the maximum number of bound parameters allowed per query.
     * This value depends on the database engine.
     */
    int maximumBoundValues() const;

    /**
     * Enables or disables FOREIGN_KEY_CHECKS for the database.
     * This function depends on the database engine.
     */
    void setForeignKeyChecks(bool check);

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

protected:

    BdEngineBackendPrivate* const d_ptr;

private:

    Q_DECLARE_PRIVATE(BdEngineBackend)
};

} // namespace Digikam

Q_DECLARE_METATYPE(QSqlError)

#endif // DATABASE_ENGINE_BACKEND_H
