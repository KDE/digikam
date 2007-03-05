/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-01-16
 * Description : image file IO threaded interface.
 *
 * Copyright 2006 by Marcel Wiesweg
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

#ifndef LOADING_DESCRIPTION_H
#define LOADING_DESCRIPTION_H

// Digikam includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadingDescription
{
public:

    class PreviewParameters
    {
    public:
        PreviewParameters()
        {
            size = 0;
            exifRotate = false;
        }
        int size;
        bool exifRotate;
    };

    /*
        An invalid LoadingDescription
    */
    LoadingDescription()
    {
    }

    /*
        Use this for files that are not raw files.
        Stores only the filePath.
    */
    LoadingDescription(const QString &filePath)
        : filePath(filePath)
        {
            rawDecodingSettings = KDcrawIface::RawDecodingSettings();
        };

    /*
        For raw files:
        Stores filePath and RawDecodingSettings
    */
    LoadingDescription(const QString &filePath, KDcrawIface::RawDecodingSettings settings)
        : filePath(filePath), rawDecodingSettings(settings)
        {};

    /*
        For preview jobs:
        Stores preview max size and exif rotation
    */
    LoadingDescription(const QString &filePath, int size, bool exifRotate)
        : filePath(filePath)
        {
            previewParameters.size       = size;
            previewParameters.exifRotate = exifRotate;
        };

    QString                          filePath;
    KDcrawIface::RawDecodingSettings rawDecodingSettings;
    PreviewParameters                previewParameters;

    /*
        Return the cache key this description shall be stored as
    */
    QString             cacheKey() const;
    /*
        Return all possible cache keys, starting with the best choice,
        for which a result may be found in the cache for this description.
        Included in the list are better quality versions, if this description is reduced.
    */
    QStringList         lookupCacheKeys() const;
    /*
        Returns whether this description describes a loading operation which
        loads the image in a reduced version (quality, size etc.)
    */
    bool                isReducedVersion() const;

    /*
        Returns whether the other loading task equals this one
    */
    bool operator==(const LoadingDescription &other) const;
    bool operator!=(const LoadingDescription &other) const
        { return !operator==(other); }
    /*
        Returns whether the other loading task equals this one
        ignoring parameters used to specify a reduced version.
    */
    bool equalsIgnoreReducedVersion(const LoadingDescription &other) const;
    /*
        Returns whether this loading task equals the other one
        or is superior to it, if the other one is a reduced version
    */
    bool equalsOrBetterThan(const LoadingDescription &other) const;

    /*
        Returns all possible cacheKeys for the given file path
        (all cache keys under which the given file could be stored in the cache).
    */
    static QStringList possibleCacheKeys(const QString &filePath);
};

}   // namespace Digikam

#endif // LOADING_DESCRIPTION_H
