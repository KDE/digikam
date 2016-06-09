/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Core database access wrapper.
 *
 * Copyright (C) 2016 by Swati Lodha <swatilodha27 at gmail dot com>
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

#ifndef DATABASE_ENGINE_ACCESS_H
#define DATABASE_ENGINE_ACCESS_H

//Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam 
{

class InitializationObserver;
class DbEngineAccessStaticPriv;

class DIGIKAM_DATABASE_EXPORT DbEngineAccess
{
public:
    /** A parent class for factoring common code in 3 analogous DB files.
     * The DbEngineAccess provides access to the database:
     * Create an instance of this class on the stack to retrieve a pointer to the database.
     */
    
    // Create a default DbEngineAccess object.
    explicit DbEngineAccess();
    virtual ~DbEngineAccess();
    
    //checks the availability of drivers
    virtual bool checkReadyForUse();
    
private:
    
    static DbEngineAccessStaticPriv* d;
};

} //namespace Digikam

#endif //DATABASE_ENGINE_ACCESS_H