/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Face database access wrapper.
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

#ifndef _FACE_DATABASE_ACCESS_H_
#define _FACE_DATABASE_ACCESS_H_

// Local includes

#include "facedbbackend.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"
#include "collectionscannerobserver.h"
#include "digikam_export.h"

using namespace Digikam;

namespace FacesEngine
{

class TrainingDB;
class FaceDbAccessData;

/** This class is written in analogy to CoreDbAccess
 *  (some features stripped off).
 *  For documentation, see databaseaccess.h
 */
class DIGIKAM_DATABASE_EXPORT FaceDbAccess
{
public:

    FaceDbAccess(FaceDbAccessData* const);
    ~FaceDbAccess();

    TrainingDB*          db()         const;
    FaceDbBackend*       backend()    const;
    QString              lastError()  const;
    DbEngineParameters   parameters() const;

    /**
      * Set the "last error" message. This method is not for public use.
      */
    void setLastError(const QString& error);

public:

    static FaceDbAccessData* create();
    static void destroy(FaceDbAccessData* const);

    static void initDbEngineErrorHandler(FaceDbAccessData* const d, DbEngineErrorHandler* const errorhandler);
    static void setParameters(FaceDbAccessData* const d, const DbEngineParameters& parameters);
    static bool checkReadyForUse(FaceDbAccessData* const d, InitializationObserver* const observer = 0);

private:

    FaceDbAccess(bool, FaceDbAccessData* const);

private:

    friend class FaceDbAccessUnlock;
    FaceDbAccessData* const d;
};

// ------------------------------------------------------------------------------------------

class FaceDbAccessUnlock
{
public:

    /** Acquire an object of this class if you want to assure
     *  that the FaceDbAccess is _not_ held during the lifetime of the object.
     *  At creation, the lock is obtained shortly, then all locks are released.
     *  At destruction, all locks are acquired again.
     *  If you need to access any locked structures during lifetime, acquire a new
     *  FaceDbAccess.
     */
    FaceDbAccessUnlock(FaceDbAccessData* const);
    FaceDbAccessUnlock(FaceDbAccess* const access);
    ~FaceDbAccessUnlock();

private:

    FaceDbAccessData* d;
    int               count;
};

} // namespace FacesEngine

#endif // _FACE_DATABASE_ACCESS_H_
