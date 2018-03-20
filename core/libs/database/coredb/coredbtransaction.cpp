/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Core database convenience object for transactions.
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

#include "coredbtransaction.h"

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"

namespace Digikam
{

CoreDbTransaction::CoreDbTransaction()
    : m_access(0)
{
    CoreDbAccess access;
    access.backend()->beginTransaction();
}

CoreDbTransaction::CoreDbTransaction(CoreDbAccess* const access)
    : m_access(access)
{
    m_access->backend()->beginTransaction();
}

CoreDbTransaction::~CoreDbTransaction()
{
    if (m_access)
    {
        m_access->backend()->commitTransaction();
    }
    else
    {
        CoreDbAccess access;
        access.backend()->commitTransaction();
    }
}

}  // namespace Digikam
