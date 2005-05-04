/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-09
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef DIGIKAMTAGS_H
#define DIGIKAMTAGS_H

#include <kio/slavebase.h>
#include <qstring.h>

class KURL;
class QCString;

typedef struct sqlite sqleet; 

class kio_digikamtagsProtocol : public KIO::SlaveBase
{
public:

    kio_digikamtagsProtocol(const QCString &pool_socket,
                            const QCString &app_socket);
    virtual ~kio_digikamtagsProtocol();

    void special(const QByteArray& data);
    
private:

    void openDB();
    void closeDB();
    
    bool execSql(const QString& sql, QStringList* const values = 0, 
                 const bool debug = false);
    
    sqleet*            m_db;
    QString            m_libraryPath;
};
    

#endif /* DIGIKAMTAGS_H */
