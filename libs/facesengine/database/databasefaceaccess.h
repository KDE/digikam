/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnail database access wrapper.
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

#ifndef _DATABASE_FACE_ACCESS_H_
#define _DATABASE_FACE_ACCESS_H_

// Local includes

#include "databasefacebackend.h"
#include "databaseparameters.h"
#include "databaseerrorhandler.h"
#include "collectionscannerobserver.h"
#include "digikam_export.h"

using namespace Digikam;

namespace FacesEngine
{

class TrainingDB;
class DatabaseFaceAccessData;

class DIGIKAM_DATABASE_EXPORT DatabaseFaceAccess
{
public:

    static DatabaseFaceAccessData* create();
    static void destroy(DatabaseFaceAccessData* const);

    /** This class is written in analogy to DatabaseFaceAccess
     *  (some features stripped off).
     *  For documentation, see databaseaccess.h */

    DatabaseFaceAccess(DatabaseFaceAccessData* const);
    ~DatabaseFaceAccess();

    TrainingDB* db()                  const;
    DatabaseFaceBackend* backend()    const;
    QString              lastError()  const;
    DatabaseParameters   parameters() const;


    static void initDatabaseErrorHandler(DatabaseFaceAccessData* const d, DatabaseErrorHandler* const errorhandler);
    static void setParameters(DatabaseFaceAccessData* const d, const DatabaseParameters& parameters);
    static bool checkReadyForUse(DatabaseFaceAccessData* const d, InitializationObserver* const observer = 0);

    /**
      * Set the "last error" message. This method is not for public use.
      */
    void setLastError(const QString& error);

private:

    DatabaseFaceAccess(bool, DatabaseFaceAccessData* const);

private:

    friend class DatabaseFaceAccessUnlock;
    DatabaseFaceAccessData* const d;
};

// ------------------------------------------------------------------------------------------

class DatabaseFaceAccessUnlock
{
public:

    /** Acquire an object of this class if you want to assure
     *  that the DatabaseFaceAccess is _not_ held during the lifetime of the object.
     *  At creation, the lock is obtained shortly, then all locks are released.
     *  At destruction, all locks are acquired again.
     *  If you need to access any locked structures during lifetime, acquire a new
     *  DatabaseFaceAccess.
     */
    DatabaseFaceAccessUnlock(DatabaseFaceAccessData* const);
    DatabaseFaceAccessUnlock(DatabaseFaceAccess* const access);
    ~DatabaseFaceAccessUnlock();

private:

    DatabaseFaceAccessData* d;
    int                     count;
};

} // namespace FacesEngine

#endif // _DATABASE_FACE_ACCESS_H_
