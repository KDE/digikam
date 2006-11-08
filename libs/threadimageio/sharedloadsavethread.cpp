/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-20
 * Description : image file IO threaded interface.
 *
 * Copyright 2005-2006 by Marcel Wiesweg, Gilles Caulier
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

// Local includes.

#include "ddebug.h"
#include "sharedloadsavethread.h"
#include "loadingcache.h"
#include "loadsavetask.h"

namespace Digikam
{

void SharedLoadSaveThread::load(LoadingDescription description, AccessMode mode, LoadingPolicy policy)
{
    ManagedLoadSaveThread::load(description, LoadingModeShared, policy, mode);
}

DImg SharedLoadSaveThread::cacheLookup(const QString& filePath, AccessMode /*accessMode*/)
{
    LoadingCache *cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);
    DImg *cachedImg = cache->retrieveImage(filePath);
    // Qt4: uncomment this code.
    // See comments in SharedLoadingTask::execute for explanation.
    /*
    if (cachedImg)
    {
        if (accessMode == AccessModeReadWrite)
            return cachedImg->copy();
        else
            return *cachedImg;
    }
    else
        return DImg();
    */
    if (cachedImg)
        return cachedImg->copy();
    else
        return DImg();
}

}   // namespace Digikam

