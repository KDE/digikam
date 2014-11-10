/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-12
 * Description : Convenience object for grouping database operations
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

#include "databaseoperationgroup.h"

// Qt includes

#include <QTime>

// KDE includes

#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasebackend.h"

namespace Digikam
{

class DatabaseOperationGroup::Private
{
public:

    Private()
    {
        access   = 0;
        acquired = false;
        maxTime  = 0;
    }

public:

    DatabaseAccess* access;
    bool            acquired;
    QTime           timeAcquired;
    int             maxTime;

public:

    bool needsTransaction() const
    {
        return DatabaseAccess::parameters().isSQLite();
    }

    void acquire()
    {
        if (access)
        {
            acquired = access->backend()->beginTransaction();
        }
        else
        {
            DatabaseAccess access;
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
                DatabaseAccess access;
                access.backend()->commitTransaction();
            }
        }
    }
};

DatabaseOperationGroup::DatabaseOperationGroup()
    : d(new Private)
{
    if (d->needsTransaction())
    {
        d->acquire();
    }
}

DatabaseOperationGroup::DatabaseOperationGroup(DatabaseAccess* const access)
    : d(new Private)
{
    d->access = access;

    if (d->needsTransaction())
    {
        d->acquire();
    }
}

DatabaseOperationGroup::~DatabaseOperationGroup()
{
    d->release();
    delete d;
}

void DatabaseOperationGroup::lift()
{
    if (d->acquired)
    {
        d->release();

        if (d->access)
        {
            DatabaseAccessUnlock unlock(d->access);
        }

        d->acquire();
    }
}

void DatabaseOperationGroup::setMaximumTime(int msecs)
{
    d->maxTime = msecs;
}

void DatabaseOperationGroup::resetTime()
{
    d->timeAcquired.start();
}

void DatabaseOperationGroup::allowLift()
{
    if (d->maxTime && d->timeAcquired.elapsed() > d->maxTime)
    {
        lift();
    }
}

}  // namespace Digikam
