/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-05
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <qstring.h>

class QStringList;

class SqliteDB
{
public:

    SqliteDB();
    ~SqliteDB();

    void openDB(const QString& directory);
    void closeDB();

    bool execSql(const QString& sql, QStringList* const values = 0,
                 QString* const errMsg = 0, bool debug = false) const;

    Q_LLONG lastInsertedRow() const;
    
    void    setSetting( const QString& keyword, const QString& value );
    QString getSetting( const QString& keyword );

private:

    mutable struct sqlite3* m_db;
};

extern QString escapeString(const QString& str);

#endif /* SQLITEDB_H */
