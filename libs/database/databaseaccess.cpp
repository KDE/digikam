/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-03-18
 * Description : database interface.
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

#include <qmutex.h>

#include "albumdb.h"
#include "databaseaccess.h"

namespace Digikam
{

class DatabaseAccessStaticPriv
{
public:
    DatabaseAccessStaticPriv() : db(0) {};
    ~DatabaseAccessStaticPriv() {};

    AlbumDB *db;
    DatabaseParameters parameters;
    QMutex mutex;
    QString albumRoot;
};

DatabaseAccessStaticPriv *DatabaseAccess::d = 0;

DatabaseAccess::DatabaseAccess()
{
    d->mutex.lock();
    if (!d->db->isOpen())
    {
        d->db->open(d->parameters);
    }
}

DatabaseAccess::~DatabaseAccess()
{
    d->mutex.unlock();
}

AlbumDB *DatabaseAccess::db()
{
    return d->db;
}

DatabaseParameters DatabaseAccess::parameters()
{
    return d->parameters;
}

void DatabaseAccess::setParameters(const DatabaseParameters &parameters)
{
    if (!d)
    {
        d = new DatabaseAccessStaticPriv();
        d->db = new AlbumDB();
    }
    else
    {
        QMutexLocker lock(&d->mutex);
        if (d->parameters != parameters)
        {
            d->db->close();
        }
    }
    d->parameters = parameters;
}

void DatabaseAccess::setAlbumRoot(const QString &root)
{
    d->albumRoot = root;
}

QString DatabaseAccess::albumRoot()
{
    return d->albumRoot;
}

void DatabaseAccess::cleanUpDatabase()
{
    if (d)
    {
        QMutexLocker lock(&d->mutex);
        d->db->close();
        delete d->db;
    }
    delete d;
    d = 0;
}



}
