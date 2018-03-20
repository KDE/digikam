/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Core database access wrapper.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COREDATABASEACCESS_H
#define COREDATABASEACCESS_H

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class CoreDbBackend;
class CoreDB;
class CoreDbWatch;
class InitializationObserver;
class CoreDbAccessStaticPriv;

/** The CoreDbAccess provides access to the database:
 *  Create an instance of this class on the stack to retrieve a pointer to the database.
 *  While you hold an instance of CoreDbAccess, the database access is locked for other threads,
 *  but _not_ for other processes. This is due to the fact that while databases allow
 *  concurrent access (of course), their client libs may not be thread-safe.
 *
 *  When initializing your application, you need to call two methods:
 *  - in a not-yet-multithreaded context, you need to call setParameters
 *  - to make sure that the database is available and the schema
 *    is properly initialized, call checkReadyForUse()
 */
class DIGIKAM_DATABASE_EXPORT CoreDbAccess
{
public:

    enum ApplicationStatus
    {
        MainApplication,
        DatabaseSlave
    };

public:

    /**
     * Create a CoreDbAccess object for the default database.
     * Note that when initializing your app, setParameters need to be called
     * (in a not-yet-multithreaded context) for this to work.
     * If the database is not yet opened, it will be opened.
     * The schema will not be checked, use checkReadyForUse()
     * for a full opening process including schema update and error messages.
     */
    CoreDbAccess();
    ~CoreDbAccess();

    /**
     * Retrieve a pointer to the album database
     */
    CoreDB* db() const;

    /**
     * Retrieve a pointer to the database backend
     */
    CoreDbBackend* backend() const;

    /**
     * Returns the error message for the last error that occurred,
     * or a null QString of no error occurred.
     */
    QString lastError();

    /**
     * Set the "last error" message. This method is not for public use.
     */
    void setLastError(const QString& error);

public:

    /**
     * Return the default parameters
     */
    static DbEngineParameters parameters();

    /**
     * Set the default parameters.
     * Call this function at least once in the starting phase of your application,
     * when no other threads will yet access the database, to initialize DatabaseAcccess.
     * After this initial call, it is thread-safe to call this function again.
     * In a subsequent call, if the parameters are identical, nothing is done.
     * If the parameters change, the current database will be closed.
     * When parameters have been set or changed, the new one will be opened on-demand,
     * i.e. when the first CoreDbAccess object is constructed.
     */
    static void setParameters(const DbEngineParameters& parameters);
    static void setParameters(const DbEngineParameters& parameters, ApplicationStatus status);

    /**
     * Method to one-time initialize a database when new parameters have been set:
     * Make sure that the database is open, that the schema has properly been initialized.
     * If the parameters were not changed, this method has no effect.
     * @returns if the database is ready for use
     */
    static bool checkReadyForUse(InitializationObserver* const observer = 0);

    /**
     * Clean up the database access.
     * When this function has been called, the access can be restored by calling setParameters.
     * Construction a database access object otherwise after calling this method will crash.
     */
    static void cleanUpDatabase();

    /**
     * Return the CoreDbWatch.
     */
    static CoreDbWatch* databaseWatch();

    /**
     * Setup the errors handler instance.
     */
    static void initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler);

private:

    explicit CoreDbAccess(bool);

    friend class CoreDbAccessUnlock;
    static CoreDbAccessStaticPriv* d;
};

// -----------------------------------------------------------------------------

class CoreDbAccessUnlock
{
public:

    /** Acquire an object of this class if you want to assure
     *  that the CoreDbAccess is _not_ held during the lifetime of the object.
     *  At creation, the lock is obtained shortly, then all locks are released.
     *  At destruction, all locks are acquired again.
     *  If you need to access any locked structures during lifetime, acquire a new
     *  CoreDbAccess.
     */
    CoreDbAccessUnlock();
    explicit CoreDbAccessUnlock(CoreDbAccess* const access);
    ~CoreDbAccessUnlock();

private:

    int count;
};

}  // namespace Digikam

#endif // COREDATABASEACCESS_H
