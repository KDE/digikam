/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-28
 * Description : Similarity database access wrapper.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
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

#ifndef SIMILARITY_DATABASE_ACCESS_H
#define SIMILARITY_DATABASE_ACCESS_H

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class InitializationObserver;
class SimilarityDb;
class SimilarityDbAccessStaticPriv;
class SimilarityDbBackend;

class DIGIKAM_EXPORT SimilarityDbAccess
{
public:

    /** This class is written in analogy to CoreDbAccess
     *  (some features stripped off).
     *  For documentation, see coredbaccess.h
     */
    explicit SimilarityDbAccess();
    ~SimilarityDbAccess();

    SimilarityDb*        db()        const;
    SimilarityDbBackend* backend()   const;
    QString              lastError() const;

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

    explicit SimilarityDbAccess(bool);

    static SimilarityDbAccessStaticPriv* d;
};

} // namespace Digikam

#endif // SIMILARITY_DATABASE_ACCESS_H
