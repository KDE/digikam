/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-02-06
 * Description : shared image loading and caching
 *
 * Copyright 2005 by Marcel Wiesweg
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

#ifndef LOADING_CACHE_INTERFACE_H
#define LOADING_CACHE_INTERFACE_H

#include <qstring.h>

#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadingCacheInterface
{
public:
    // clean up cache at shutdown
    static void cleanUp();
    // remove an image from the cache
    // (e.g. when image has changed on disk)
    static void cleanFromCache(const QString &filePath);
    // remove all images from the cache
    // (e.g. when loading settings changed)
    static void cleanCache();
    // add a copy of the image to cache
    static void putImage(const QString &filePath, const DImg &img);
    // Set cache size in Megabytes.
    // Set to 0 to disable caching.
    static void setCacheOptions(int cacheSize);
};

}   // namespace Digikam

#endif

