/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-06
 * Description : shared image loading and caching
 *
 * Copyright (C) 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadingCacheInterface
{
public:

    static void initialize();

    /** clean up cache at shutdown */
    static void cleanUp();

    /**
     * Remove an image from the cache
     * because it may have changed on disk
     */
    static void fileChanged(const QString& filePath);

    /**
     * Connect the given object/slot to the signal
     *  void fileChanged(const QString& filePath);
     * which is emitted when the cache gains knowledge about a possible
     * change of this file on disk.
     */
    static void connectToSignalFileChanged(QObject* object, const char* slot);

    /**
     * remove all images from the cache
     * (e.g. when loading settings changed)
     * Does not affect thumbnails.
     */
    static void cleanCache();

    /**
     * Remove all thumbnails from the thumbnail cache.
     * Does not affect main image cache.
     */
    static void cleanThumbnailCache();

    /** add a copy of the image to cache */
    static void putImage(const QString& filePath, const DImg& img);

    /**
     * Set cache size in Megabytes.
     * Set to 0 to disable caching.
     */
    static void setCacheOptions(int cacheSize);
};

}   // namespace Digikam

#endif // LOADING_CACHE_INTERFACE_H
