/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : database thumbnail interface.
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnaildb.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "databasebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class ThumbnailDBPriv
{

public:

    ThumbnailDBPriv()
    {
        db = 0;
    }

    DatabaseBackend *db;
};

ThumbnailDB::ThumbnailDB(DatabaseBackend *backend)
           : d(new ThumbnailDBPriv)
{
    d->db = backend;
}

ThumbnailDB::~ThumbnailDB()
{
    delete d;
}

void ThumbnailDB::setSetting(const QString& keyword, const QString& value )
{
    d->db->execSql( QString("REPLACE into Settings VALUES (?,?);"),
                    keyword, value );
}

QString ThumbnailDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT value FROM Settings "
                            "WHERE keyword=?;"),
                    keyword, &values );

    if (values.isEmpty())
        return QString();
    else
        return values.first().toString();
}

}  // namespace Digikam
