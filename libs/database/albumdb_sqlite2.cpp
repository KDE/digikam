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

#include "albumdb_sqlite2.h"

// C ANSI includes

extern "C"
{
#include <sys/time.h>
}

// C++ includes

#include <ctime>
#include <cstdio>
#include <cstdlib>

// Qt includes

#include <QFile>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

typedef struct sqlite_vm sqlite_vm;

AlbumDB_Sqlite2::AlbumDB_Sqlite2()
{
    m_db    = 0;
    m_valid = false;
}

AlbumDB_Sqlite2::~AlbumDB_Sqlite2()
{
    if (m_db)
    {
        sqlite_close(m_db);
    }
}

void AlbumDB_Sqlite2::setDBPath(const QString& path)
{
    if (m_db)
    {
        sqlite_close(m_db);
        m_db    = 0;
        m_valid = false;
    }

    char* errMsg = 0;
    m_db = sqlite_open(QFile::encodeName(path), 0, &errMsg);

    if (m_db == 0)
    {
        kWarning() << "Cannot open database: " << errMsg;
        free(errMsg);
        return;
    }

    QStringList values;
    execSql("SELECT * FROM sqlite_master", &values);
    m_valid = values.contains("Albums");
}

bool AlbumDB_Sqlite2::execSql(const QString& sql, QStringList* const values,
                              const bool debug)
{
    if ( debug )
    {
        kDebug() << "SQL-query: " << sql;
    }

    if ( !m_db )
    {
        kWarning() << "SQLite pointer == NULL";
        return false;
    }

    const char* tail;

    sqlite_vm*  vm;

    char*       errorStr;

    int         error;

    // Compile SQL program to virtual machine
    error = sqlite_compile( m_db, QFile::encodeName(sql), &tail, &vm, &errorStr );

    if ( error != SQLITE_OK )
    {
        kWarning() << "sqlite_compile error: "
                   << errorStr
                   << " on query: " << sql;
        sqlite_freemem( errorStr );
        return false;
    }

    int          number;
    const char** value;
    const char** colName;

    // Execute virtual machine by iterating over rows
    while ( true )
    {
        error = sqlite_step( vm, &number, &value, &colName );

        if ( error == SQLITE_DONE || error == SQLITE_ERROR )
        {
            break;
        }

        // Iterate over columns
        for ( int i = 0; values && i < number; ++i )
        {
            *values << QString::fromLocal8Bit( value [i] );
        }
    }

    // Deallocate vm resources
    sqlite_finalize( vm, &errorStr );

    if ( error != SQLITE_DONE )
    {
        kWarning() << "sqlite_step error: "
                   << errorStr
                   << " on query: " << sql;
        return false;
    }

    return true;
}

}  // namespace Digikam
