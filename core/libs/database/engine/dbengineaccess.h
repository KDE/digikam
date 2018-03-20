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

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * The DbEngineAccess class provides access to the database:
 * Create an instance of this class on the stack to retrieve a pointer to the database.
 */
class DIGIKAM_EXPORT DbEngineAccess
{
public:

    /** Checks the availability of drivers. Must be used in children class.
     *  Return true if low level drivers are ready to use, else false with
     *  an error string of the problem.
     */
    static bool checkReadyForUse(QString& error);
};

} // namespace Digikam

#endif // DATABASE_ENGINE_ACCESS_H
