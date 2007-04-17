/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-04-15
 * Description : Database backend
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

class DIGIKAM_EXPORT DatabaseBackend
{

    // NOTE: when porting to Qt SQL, most of the methods can be implemented here
public:

    static DatabaseBackend* createBackend(const DatabaseParameters &parameters);
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
    virtual bool initSchema() = 0;

    /**
     * Close the database connection
     */
    virtual void close() = 0;

    /**
     * Check if the database has been opened. This does not mean
     * that isReady() is true as well
     * (if the file could be opened, but the schema could not be initialized).
     */
    virtual bool isOpen() const = 0;

    /**
     * Check if the database interface is initialized properly.
     * This means not only open() but also initSchema() succeeded.
     * @return true if it's ready to use, else false.
     */
    virtual bool isReady() const = 0;

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

