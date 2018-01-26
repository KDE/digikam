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
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

    /**
     * This static method returns the current db parameters.
     * @return the current db parameters.
     */
    static DbEngineParameters parameters();

public:

    /**
     * This static method initialises the error handler for the similarity db.
     * @param errorhandler The error handler.
     */
    static void initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler);

    /**
     * This static method sets the database parameters that are needed to initialise the db connection.
     * @param parameters The db parameters.
     */
    static void setParameters(const DbEngineParameters& parameters);

    /**
     * This static method checks if the similarity db is ready for use.
     * @param observer the observer.
     * @return true, if the database is ready for use.
     */
    static bool checkReadyForUse(InitializationObserver* const observer);

    /**
     * This static method returns if the similarity db is initialised.
     * @return true, if the similarityDb is initialised.
     */
    static bool isInitialized();

    /**
     * This static method removes the connection to the similarity database.
     */
    static void cleanUpDatabase();

private:

    /**
     * Constructs the db access without checking the connectivity to the database itself.
     */
    explicit SimilarityDbAccess(bool);

    static SimilarityDbAccessStaticPriv* d;
};

} // namespace Digikam

#endif // SIMILARITY_DATABASE_ACCESS_H
