/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Convenience object for database transactions
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASETRANSACTION_H
#define DATABASETRANSACTION_H

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DatabaseAccess;

class DIGIKAM_DATABASE_EXPORT DatabaseTransaction
{
public:

    /**
     * Convenience class: You can create a DatabaseTransaction object for a scope for which
     * you want to declare a database commit.
     * Equivalent to calling beginTransaction and commitTransaction on the album db.
     */

    /**
     * Retrieve a DatabaseAccess object each time when constructing and destructing.
     */
    DatabaseTransaction();
    /**
     * Use an existing DatabaseAccess object, which must live as long as this object exists.
     */
    explicit DatabaseTransaction(DatabaseAccess* access);
    ~DatabaseTransaction();

private:

    DatabaseAccess* m_access;
};

}  // namespace Digikam

#endif // DATABASETRANSACTION_H
