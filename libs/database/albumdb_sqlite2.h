/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description : SQlite version 2 database interface.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef ALBUMDB_SQLITE2_H
#define ALBUMDB_SQLITE2_H

// C ANSI includes

extern "C"
{
#include <sqlite.h>
}

// Qt includes

#include <QString>
#include <QStringList>
#include <QDateTime>

namespace Digikam
{

typedef struct sqlite sqleet2; // hehe.

/**
 * This class is responsible for the communication
 * with the sqlite database.
 */
class AlbumDB_Sqlite2
{
public:

    /**
     * Constructor
     */
    AlbumDB_Sqlite2();

    /**
     * Destructor
     */
    ~AlbumDB_Sqlite2();

    /**
     * Makes a connection to the database and makes sure all tables
     * are available.
     * @param path The database to open
     */
    void setDBPath(const QString& path);

    /**
     * This will execute a given SQL statement to the database.
     * @param sql The SQL statement
     * @param values This will be filled with the result of the SQL statement
     * @param debug If true, it will output the SQL statement
     * @return It will return if the execution of the statement was succesfull
     */
    bool execSql(const QString& sql, QStringList* const values = 0,
                 const bool debug = false);

    bool isValid() const
    {
        return m_valid;
    }

private:

    bool      m_valid;
    sqleet2*  m_db;
};

}  // namespace Digikam

#endif /* ALBUMDB_SQLITE2_H */
