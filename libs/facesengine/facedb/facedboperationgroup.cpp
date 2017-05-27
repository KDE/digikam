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

#include "facedboperationgroup.h"

// Qt includes

#include <QTime>

// Local includes

#include "facedbaccess.h"
#include "facedbbackend.h"

namespace Digikam
{

class FaceDbOperationGroup::Private
{
public:

    Private()
        : access(0),
          acquired(false),
          maxTime(0)
    {
    }

public:

    FaceDbAccess* access;
    bool          acquired;
    QTime         timeAcquired;
    int           maxTime;

public:

    bool needsTransaction() const
    {
        return FaceDbAccess().parameters().isSQLite();
    }

    void acquire()
    {
        if (access)
        {
            acquired = access->backend()->beginTransaction();
        }
        else
        {
            FaceDbAccess access;
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
                FaceDbAccess access;
                access.backend()->commitTransaction();
            }
        }
    }
};

FaceDbOperationGroup::FaceDbOperationGroup()
    : d(new Private)
{
    if (d->needsTransaction())
    {
        d->acquire();
    }
}

FaceDbOperationGroup::FaceDbOperationGroup(FaceDbAccess* const access)
    : d(new Private)
{
    d->access = access;

    if (d->needsTransaction())
    {
        d->acquire();
    }
}

FaceDbOperationGroup::~FaceDbOperationGroup()
{
    d->release();
    delete d;
}

void FaceDbOperationGroup::lift()
{
    if (d->acquired)
    {
        d->release();

        if (d->access)
        {
            FaceDbAccessUnlock unlock(d->access);
        }

        d->acquire();
    }
}

void FaceDbOperationGroup::setMaximumTime(int msecs)
{
    d->maxTime = msecs;
}

void FaceDbOperationGroup::resetTime()
{
    d->timeAcquired.start();
}

void FaceDbOperationGroup::allowLift()
{
    if (d->maxTime && d->timeAcquired.elapsed() > d->maxTime)
    {
        lift();
    }
}

} // namespace Digikam
