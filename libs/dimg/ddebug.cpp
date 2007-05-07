/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-11
 * Description : thread safe debugging.
 *
 * See B.K.O #133026: because kdDebug() is not thread-safe
 * we need to use a dedicaced debug statements in threaded 
 * implementation to prevent crash.
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <qmutex.h>

// Local includes.

#include "ddebug.h"

#undef DDebug
#undef kdDebug

namespace Digikam
{

//static KStaticDeleter<QMutex> deleter;
static QMutex *_ddebug_mutex_ = 0;

Ddbgstream::Ddbgstream(kdbgstream stream)
          : kdbgstream(stream)
{
    // using a static variable here - we can safely assume that kdDebug
    // is called at least once from the main thread before threads start.
    if (!_ddebug_mutex_)
    {
        // leak the mutex object for simplicity
        _ddebug_mutex_ = new QMutex;
        //deleter.setObject(mutex, new QMutex);
        //KGlobal::unregisterStaticDeleter(&deleter);
    }
    _ddebug_mutex_->lock();
}

Ddbgstream::~Ddbgstream()
{
    _ddebug_mutex_->unlock();
}

Dndbgstream::Dndbgstream(kndbgstream stream)
           : kndbgstream(stream)
{
    // using a static variable here - we can safely assume that kdDebug
    // is called at least once from the main thread before threads start.
    if (!_ddebug_mutex_)
    {
        // leak the mutex object for simplicity
        _ddebug_mutex_ = new QMutex;
        //deleter.setObject(mutex, new QMutex);
        //KGlobal::unregisterStaticDeleter(&deleter);
    }
    _ddebug_mutex_->lock();
}

Dndbgstream::~Dndbgstream()
{
    _ddebug_mutex_->unlock();
}

} // namespace Digikam

Digikam::Ddbgstream DDebug(int area)   { return Digikam::Ddbgstream(kdDebug(area));   }
Digikam::Ddbgstream DError(int area)   { return Digikam::Ddbgstream(kdError(area));   }
Digikam::Ddbgstream DWarning(int area) { return Digikam::Ddbgstream(kdWarning(area)); }

Digikam::Dndbgstream DnDebug(int area) { return Digikam::Dndbgstream(kndDebug(area)); }

