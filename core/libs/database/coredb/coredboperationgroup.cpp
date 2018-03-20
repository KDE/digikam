/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-12
 * Description : Core database convenience object for grouping operations
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "coredboperationgroup.h"

// Qt includes

#include <QTime>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"

namespace Digikam
{

class CoreDbOperationGroup::Private
{
public:

    Private()
    {
        access   = 0;
        acquired = false;
        maxTime  = 0;
    }

public:

    CoreDbAccess* access;
    bool          acquired;
    QTime         timeAcquired;
    int           maxTime;

public:

    bool needsTransaction() const
    {
        return CoreDbAccess::parameters().isSQLite();
    }

    void acquire()
    {
        if (access)
        {
            acquired = access->backend()->beginTransaction();
        }
        else
        {
            CoreDbAccess access;
            acquired = access.backend()->beginTransaction();
        }

        timeAcquired.start();
    }

    void release()
    {
        if (acquired)
        {
            if (access)
            {
                access->backend()->commitTransaction();
            }
            else
            {
                CoreDbAccess access;
                access.backend()->commitTransaction();
            }
        }
    }
};

CoreDbOperationGroup::CoreDbOperationGroup()
    : d(new Private)
{
    if (d->needsTransaction())
    {
        d->acquire();
    }
}

CoreDbOperationGroup::CoreDbOperationGroup(CoreDbAccess* const access)
    : d(new Private)
{
    d->access = access;

    if (d->needsTransaction())
    {
        d->acquire();
    }
}

CoreDbOperationGroup::~CoreDbOperationGroup()
{
    d->release();
    delete d;
}

void CoreDbOperationGroup::lift()
{
    if (d->acquired)
    {
        d->release();

        if (d->access)
        {
            CoreDbAccessUnlock unlock(d->access);
        }

        d->acquire();
    }
}

void CoreDbOperationGroup::setMaximumTime(int msecs)
{
    d->maxTime = msecs;
}

void CoreDbOperationGroup::resetTime()
{
    d->timeAcquired.start();
}

void CoreDbOperationGroup::allowLift()
{
    if (d->maxTime && d->timeAcquired.elapsed() > d->maxTime)
    {
        lift();
    }
}

}  // namespace Digikam
