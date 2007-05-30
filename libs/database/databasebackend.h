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

#ifndef DATABASEBACKEND_H
#define DATABASEBACKEND_H

// Qt includes

#include <qstring.h>
#include <qstringlist.h>

// Local includes

#include "digikam_export.h"
#include "databaseparameters.h"

namespace Digikam
{

class SchemaUpdater;

class DIGIKAM_EXPORT DatabaseBackend
{

    // NOTE: when porting to Qt SQL, most of the methods can be implemented here
public:

    /**
     * Creates a DatabaseBackend based on the given parameters.
     * Returns null on failure.
     */
    static DatabaseBackend* createBackend(const DatabaseParameters &parameters);

    DatabaseBackend();
    virtual ~DatabaseBackend() {};

    /**
     * Checks if the parameters can be used for this database backend.
     */
    virtual bool isCompatible(const DatabaseParameters &parameters) = 0;

    /**
     * Open the database connection. Subclasses require a specific set of parameters.
     * @returns true on success
     */
    virtual bool open(const DatabaseParameters &parameters) = 0;

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     */
    virtual bool initSchema(SchemaUpdater *updater) = 0;

    /**
     * Close the database connection
     */
    virtual void close() = 0;

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
    virtual Status status() const = 0;

    bool isOpen() const { return status() > Unavailable; }
    bool isReady() const { return status() == OpenSchemaChecked; }

    /**
     * This will execute a given SQL statement to the database.
     * @param sql The SQL statement
     * @param values This will be filled with the result of the SQL statement
     * @param debug If true, it will output the SQL statement 
     * @return It will return if the execution of the statement was succesfull
     */
    virtual bool execSql(const QString& sql, QStringList* const values = 0,
                 QString *errMsg = 0, bool debug = false) = 0;

    /**
     * Escapes text fields. This is needed for all queries to the database
     * which happens with an argument which is a text field. It makes sure
     * a ' is replaced with '', as this is needed for sqlite.
     * @param str String to escape
     * @return The escaped string
     */
    virtual QString escapeString(QString str) const = 0;

    /**
     * @return the last inserted row in one the db table.
     */
    virtual Q_LLONG lastInsertedRow() = 0;

    /**
     * Begin a database transaction
     */
    virtual void beginTransaction() = 0;
    /**
     * Commit the current database transaction
     */
    virtual void commitTransaction() = 0;

    /**
     * Return a list with the names of the tables in the database
     */
    virtual QStringList tables() = 0;

    /**
     * Returns a description of the last error that occurred on this database.
     * Use DatabaseAccess::lastError for errors presented to the user.
     * This error will be included in that message.
     * It may be empty.
     */
    virtual QString lastError() = 0;

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

};

}

#endif

