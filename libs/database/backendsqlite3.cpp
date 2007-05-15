/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : SQLite3 backend
 *
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// C Ansi includes

extern "C"
{
#include <sqlite3.h>
}

// Qt includes

#include <qfile.h>

// Local includes

#include "ddebug.h"
#include "schemaupdater.h"
#include "backendsqlite3.h"


namespace Digikam
{

typedef struct sqlite3_stmt sqlite3_stmt;
typedef struct sqlite3 sqleet3;            // hehe.

class BackendSQLite3Priv
{

public:

    BackendSQLite3Priv()
    {
        ready    = false;
        dataBase = 0;
    }

    bool     ready;

    sqleet3 *dataBase;
};


BackendSQLite3::BackendSQLite3()
{
    d = new BackendSQLite3Priv();
}

BackendSQLite3::~BackendSQLite3()
{
    close();
    delete d;
}

bool BackendSQLite3::isCompatible(const DatabaseParameters &parameters)
{
    return parameters.databaseType == "QSQLITE";
}


bool BackendSQLite3::open(const DatabaseParameters &parameters)
{
    if (!isCompatible(parameters))
    {
        DWarning() << "Wrong parameters for SQLite3 db: Type is " << parameters.databaseType << endl;
        return false;
    }

    close();

    sqlite3_open(QFile::encodeName(parameters.databaseName), &d->dataBase);

    if (d->dataBase == 0)
    {
        DWarning() << "Cannot open database: "
                << sqlite3_errmsg(d->dataBase)
                << endl;
        return false;
    }
    return true;
}

bool BackendSQLite3::initSchema()
{
    if (d->ready)
        return true;
    SchemaUpdater updater(this);
    d->ready = updater.update();
    return d->ready;
}

void BackendSQLite3::close()
{
    if (d->dataBase)
    {
        sqlite3_close(d->dataBase);
        d->dataBase = 0;
    }
    d->ready = false;
}

bool BackendSQLite3::isOpen() const
{
    return d->dataBase;
}

bool BackendSQLite3::isReady() const
{
    return d->ready;
}

bool BackendSQLite3::execSql(const QString& sql, QStringList* const values,
                             QString *errMsg, bool debug)
{
    if ( debug )
        DDebug() << "SQL-query: " << sql << endl;

    if ( !d->dataBase )
    {
        DWarning() << k_funcinfo << "SQLite pointer == NULL"
                    << endl;
        if (errMsg)
        {
            *errMsg = QString::fromLatin1("SQLite database not open");
        }
        return false;
    }

    const char*   tail;
    sqlite3_stmt* stmt;
    int           error;

    //compile SQL program to virtual machine
    error = sqlite3_prepare(d->dataBase, sql.utf8(), -1, &stmt, &tail);
    if ( error != SQLITE_OK )
    {
        DWarning() << k_funcinfo
                    << "sqlite_compile error: "
                    << sqlite3_errmsg(d->dataBase)
                    << " on query: "
                    << sql << endl;
        if (errMsg)
        {
            *errMsg = QString::fromLatin1("sqlite_compile error: ") +
                      QString::fromLatin1(sqlite3_errmsg(d->dataBase)) +
                      QString::fromLatin1(" on query: ") +
                      sql;
        }
        return false;
    }

    int cols = sqlite3_column_count(stmt);

    while ( true )
    {
        error = sqlite3_step( stmt );

        if ( error == SQLITE_DONE || error == SQLITE_ERROR )
            break;

        //iterate over columns
        for ( int i = 0; values && i < cols; i++ )
        {
            *values << QString::fromUtf8( (const char*)sqlite3_column_text( stmt, i ) );
        }
    }

    sqlite3_finalize( stmt );

    if ( error != SQLITE_DONE )
    {
        DWarning() << "sqlite_step error: "
                    << sqlite3_errmsg( d->dataBase )
                    << " on query: "
                    << sql << endl;
        if (errMsg)
        {
            *errMsg = QString::fromLatin1("sqlite_step error: ") +
                      QString::fromLatin1(sqlite3_errmsg(d->dataBase)) +
                      QString::fromLatin1(" on query: ") +
                      sql;
        }
        return false;
    }

    return true;
}

QString BackendSQLite3::escapeString(QString str) const
{
    str.replace( "'", "''" );
    return str;
}

Q_LLONG BackendSQLite3::lastInsertedRow()
{
    return sqlite3_last_insert_rowid(d->dataBase);
}

void BackendSQLite3::beginTransaction()
{
    execSql( "BEGIN TRANSACTION;" );
}

void BackendSQLite3::commitTransaction()
{
    execSql( "COMMIT TRANSACTION;" );
}




}

