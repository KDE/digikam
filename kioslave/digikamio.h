/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-28
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

#ifndef DIGIKAMIO_H
#define DIGIKAMIO_H

#include <kio/slavebase.h>
#include <qstring.h>

class KURL;
class QCString;

typedef struct sqlite sqleet; 

class kio_digikamioProtocol : public KIO::SlaveBase
{
public:

    kio_digikamioProtocol(const QCString &pool_socket,
                          const QCString &app_socket);
    ~kio_digikamioProtocol();

    void copy(const KURL& src, const KURL& dest,
              int permissions, bool overwrite);
    void rename(const KURL &src, const KURL &dest,
                bool overwrite);
    void del(const KURL& url, bool isfile);
    void stat(const KURL& url);

private:

    void copyInternal(const KURL& src, const KURL& dest,
                      int permissions, bool overwrite,
                      bool& failed);
    
    QString albumURLFromKURL(const KURL& kurl);
    void    removeDirFromDB(const QString& url);
    void    removeFileFromDB(int dirid, const QString& name);
    bool    copyFile(const QString& src, const QString& dest);
    

    bool    execSql(const QString& sql, QStringList* const values = 0, 
                 const bool debug = false);
    QString escapeString(QString str) const;

    sqleet*         m_db;
    bool            m_valid;
    QString         m_libraryPath;
};              

#endif /* DIGIKAMIO_H */
