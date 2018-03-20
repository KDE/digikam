/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnail database access wrapper.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAILS_DATABASE_ACCESS_H
#define THUMBNAILS_DATABASE_ACCESS_H

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class InitializationObserver;
class ThumbsDb;
class ThumbsDbAccessStaticPriv;
class ThumbsDbBackend;

class DIGIKAM_EXPORT ThumbsDbAccess
{
public:

    /** This class is written in analogy to CoreDbAccess
     *  (some features stripped off).
     *  For documentation, see coredbaccess.h
     */
    ThumbsDbAccess();
    ~ThumbsDbAccess();

    ThumbsDb*        db()        const;
    ThumbsDbBackend* backend()   const;
    QString          lastError() const;

    /**
     * Set the "last error" message. This method is not for public use.
     */
    void setLastError(const QString& error);

    static DbEngineParameters parameters();

public:

    static void initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler);
    static void setParameters(const DbEngineParameters& parameters);
    static bool checkReadyForUse(InitializationObserver* const observer);
    static bool isInitialized();
    static void cleanUpDatabase();

private:

    explicit ThumbsDbAccess(bool);

    static ThumbsDbAccessStaticPriv* d;
};

}  // namespace Digikam

#endif // THUMBNAILS_DATABASE_ACCESS_H
