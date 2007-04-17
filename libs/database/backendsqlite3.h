/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-04-15
 * Description : SQLite3 database
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

#ifndef BACKENDSQLITE3_H
#define BACKENDSQLITE3_H

#include "databasebackend.h"

namespace Digikam
{

class BackendSQLite3Priv;

class BackendSQLite3 : public DatabaseBackend
{

public:

    BackendSQLite3();
    virtual ~BackendSQLite3();

    bool isCompatible(const DatabaseParameters &parameters);
    /**
     * Open the database connection.
     * @returns true on success
     */
    bool open(const DatabaseParameters &parameters);

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     */
    bool initSchema();

    /**
     * Close the database connection
     */
    void close();

    /**
     * Check if the database has been opened. This does not mean
     * that isReady() is true as well
     * (if the file could be opened, but the schema could not be initialized).
     */
    bool isOpen() const;

    /**
     * Check if the database interface is initialized properly.
     * This means not only open() but also initSchema() succeeded.
     * @return true if it's ready to use, else false.
     */
    bool isReady() const;

    /**
     * This will execute a given SQL statement to the database.
     * @param sql The SQL statement
     * @param values This will be filled with the result of the SQL statement
     * @param debug If true, it will output the SQL statement 
     * @return It will return if the execution of the statement was succesfull
     */
    bool execSql(const QString& sql, QStringList* const values = 0,
                 QString *errMsg = 0, bool debug = false);

    /**
     * Escapes text fields. This is needed for all queries to the database
     * which happens with an argument which is a text field. It makes sure
     * a ' is replaced with '', as this is needed for sqlite.
     * @param str String to escape
     * @return The escaped string
     */
    QString escapeString(QString str) const;

    /**
     * @return the last inserted row in one the db table.
     */
    Q_LLONG lastInsertedRow();

    /**
     * Begin a database transaction
     */
    void beginTransaction();
    /**
     * Commit the current database transaction
     */
    void commitTransaction();

private:

    BackendSQLite3Priv *d;
};

}

#endif

