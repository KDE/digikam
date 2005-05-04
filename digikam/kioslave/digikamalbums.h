/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
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

#ifndef DIGIKAMALBUMS_H
#define DIGIKAMALBUMS_H

#include <kio/slavebase.h>

typedef struct sqlite sqleet;
class QStringList;

class kio_digikamalbums : public KIO::SlaveBase
{
public:

    kio_digikamalbums(const QCString &pool_socket,
                      const QCString &app_socket);
    ~kio_digikamalbums();

    void special(const QByteArray& data);

private:

    void openDB();
    void closeDB();
    bool execSql(const QString& sql, QStringList* const values = 0, 
                 const bool debug = false);
    QString escapeString(const QString& str) const;

    sqleet*   m_db;
    QString   m_libraryPath;
};


#endif /* DIGIKAMALBUMS_H */
