/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-05
 * Description :
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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

#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <kio/global.h>
#include <kdebug.h>

#include <sqlite3.h>

#include "sqlitedb.h"

#include "config.h" // Needed for NFS_HACK

SqliteDB::SqliteDB()
{
    m_db = 0;
}

SqliteDB::~SqliteDB()
{
    closeDB();
}

void SqliteDB::openDB(const QString& directory)
{
    if (m_db)
    {
        closeDB();
    }

    QString dbPath = directory + "/digikam3.db";

#ifdef NFS_HACK
    dbPath = QDir::homeDirPath() + "/.kde/share/apps/digikam/"  +
             KIO::encodeFileName(QDir::cleanDirPath(dbPath));
#endif

    sqlite3_open(QFile::encodeName(dbPath), &m_db);
    if (m_db == 0)
    {
        kdWarning() << "Cannot open database: "
                    << sqlite3_errmsg(m_db)
                    << endl;
    }
}

void SqliteDB::closeDB()
{
    if (m_db)
    {
        sqlite3_close(m_db);
        m_db = 0;
    }
}

bool SqliteDB::execSql(const QString& sql, QStringList* const values,
                       QString* errMsg, bool debug ) const
{
    if ( debug )
        kdDebug() << "SQL-query: " << sql << endl;

    if ( !m_db )
    {
        kdWarning() << k_funcinfo << "SQLite pointer == NULL"
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
    error = sqlite3_prepare(m_db, sql.utf8(), -1, &stmt, &tail);
    if ( error != SQLITE_OK )
    {
        kdWarning() << k_funcinfo
                    << "sqlite_compile error: "
                    << sqlite3_errmsg(m_db)
                    << " on query: "
                    << sql << endl;
        if (errMsg)
        {
            *errMsg = QString::fromLatin1("sqlite_compile error: ") +
                      QString::fromLatin1(sqlite3_errmsg(m_db)) +
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
        kdWarning() << "sqlite_step error: "
                    << sqlite3_errmsg( m_db )
                    << " on query: "
                    << sql << endl;
        if (errMsg)
        {
            *errMsg = QString::fromLatin1("sqlite_step error: ") +
                      QString::fromLatin1(sqlite3_errmsg(m_db)) +
                      QString::fromLatin1(" on query: ") +
                      sql;
        }
        return false;
    }

    return true;
}

void SqliteDB::setSetting( const QString& keyword, const QString& value )
{
    execSql( QString("REPLACE into Settings VALUES ('%1','%2');")
            .arg( escapeString(keyword) )
            .arg( escapeString(value) ));
}

QString SqliteDB::getSetting( const QString& keyword )
{
    QStringList values;
    execSql( QString("SELECT value FROM Settings "
                     "WHERE keyword='%1';")
            .arg(escapeString(keyword)),
            &values );

    if (values.isEmpty())
        return QString();
    else
        return values[0];
}

extern QString escapeString(const QString& str)
{
    QString st(str);
    st.replace( "'", "''" );
    return st;
}

Q_LLONG SqliteDB::lastInsertedRow() const
{
    return sqlite3_last_insert_rowid(m_db);    
}
