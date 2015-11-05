/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Thumbnail database backend
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAIL_DATABASE_BACKEND_H
#define THUMBNAIL_DATABASE_BACKEND_H

// Local includes

#include "digikam_export.h"
#include "databasecorebackend.h"

namespace Digikam
{

class ThumbnailSchemaUpdater;
class ThumbnailDatabaseBackendPrivate;

class DIGIKAM_DATABASE_EXPORT ThumbnailDatabaseBackend : public DatabaseCoreBackend
{
    Q_OBJECT

public:

    explicit ThumbnailDatabaseBackend(DatabaseLocking* const locking, const QString& backendName = QLatin1String("thumbnailDatabase-"));
    ~ThumbnailDatabaseBackend();

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(ThumbnailSchemaUpdater* const updater);    

private:
    
    Q_DECLARE_PRIVATE(ThumbnailDatabaseBackend)
};

} // namespace Digikam

#endif // THUMBNAIL_DATABASE_BACKEND_H
