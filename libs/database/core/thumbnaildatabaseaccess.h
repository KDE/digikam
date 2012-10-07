/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnail database access wrapper.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef THUMBNAILDATABASEACCESS_H
#define THUMBNAILDATABASEACCESS_H

// Local includes

#include "digikam_export.h"
#include "databaseparameters.h"
#include "databaseerrorhandler.h"

class QMutexLocker;

namespace Digikam
{

class DatabaseCoreBackend;
class InitializationObserver;
class ThumbnailDB;
class ThumbnailDatabaseAccessStaticPriv;

class DIGIKAM_EXPORT ThumbnailDatabaseAccess
{
public:

    /** This class is written in analogy to DatabaseAccess
     *  (some features stripped off).
     *  For documentation, see databaseaccess.h */

    ThumbnailDatabaseAccess();
    ~ThumbnailDatabaseAccess();

    ThumbnailDB* db() const;

    DatabaseCoreBackend* backend() const;
    QString lastError();

    /**
      * Set the "last error" message. This method is not for public use.
      */
    void setLastError(const QString& error);

    static DatabaseParameters parameters();

    static void initDatabaseErrorHandler(DatabaseErrorHandler* errorhandler);
    static void setParameters(const DatabaseParameters& parameters);
    static bool checkReadyForUse(InitializationObserver* observer);
    static bool isInitialized();
    static void cleanUpDatabase();

private:

    explicit ThumbnailDatabaseAccess(bool);

    static ThumbnailDatabaseAccessStaticPriv* d;
};

}  // namespace Digikam

#endif // THUMBNAILDATABASEACCESS_H
