/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2005-04-21
 * Description :
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef DIGIKAMSEARCH_H
#define DIGIKAMSEARCH_H

// KDE includes.

#include <kio/slavebase.h>

// Local includes.

#include "sqlitedb.h"

class QStringList;

class kio_digikamsearch : public KIO::SlaveBase
{

public:

    enum SKey
    {
        ALBUM = 0,
        ALBUMNAME,
        ALBUMCAPTION,
        ALBUMCOLLECTION,
        TAG,
        TAGNAME,
        IMAGENAME,
        IMAGECAPTION,
        IMAGEDATE,
        KEYWORD,
        RATING
    };

    enum SOperator
    {
        EQ = 0,
        NE,
        LT,
        GT,
        LIKE,
        NLIKE,
        LTE,
        GTE
    };

public:

    kio_digikamsearch(const QCString &pool_socket, const QCString &app_socket);
    ~kio_digikamsearch();

    void special(const QByteArray& data);

private:

    QString buildQuery(const KURL& url) const;

    QString subQuery(enum SKey key, enum SOperator op, const QString& val) const;

    QString possibleDate(const QString& str, bool& exact) const;
    
private:

    class RuleType
    {
    public:

        SKey      key;
        SOperator op;
        QString   val;
    };

    SqliteDB m_db;
    QString  m_libraryPath;
    QString  m_longMonths[12];
    QString  m_shortMonths[12];
};

#endif /* DIGIKAMSEARCH_H */
