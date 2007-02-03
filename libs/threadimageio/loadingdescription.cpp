/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2007-02-03
 * Description : Loading parameters for multithreaded loading
 *
 * Copyright 2006-2007 by Marcel Wiesweg
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

#include "loadingdescription.h"

namespace Digikam
{

QString LoadingDescription::cacheKey() const
{
    // Here we have the knowledge which LoadingDescriptions / RawFileDecodingSettings
    // must be cached separately.
    // When the Raw loading settings are changed in setup, the cache is cleaned,
    // so we do not need to store check for every option here.
    if (rawDecodingSettings.halfSizeColorImage)
        return filePath + "-halfSizeColorImage";
    else if (previewParameters.size)
        return filePath + "-previewImage";
    else
        return filePath;
}

QStringList LoadingDescription::lookupCacheKeys() const
{
    // Build a hierarchy which cache entries may be used for this LoadingDescription.
    // Typically, the first is the best, but an actual loading operation will use a faster
    // way and will effectively add the last entry of the list to the cache
    QStringList keys;
    keys.append(filePath);
    if (rawDecodingSettings.halfSizeColorImage)
        keys.append(filePath + "-halfSizeColorImage");
    if (previewParameters.size)
        keys.append(filePath + "-previewImage");
    return keys;
}

bool LoadingDescription::isReducedVersion() const
{
    // return true if this loads anything but the full version
    return rawDecodingSettings.halfSizeColorImage
        || previewParameters.size;
}

bool LoadingDescription::operator==(const LoadingDescription &other) const
{
    // NOTE: If we start loading RAW files with different loading settings in parallel,
    //       this and the next methods must be better implemented!
    return filePath == other.filePath &&
            rawDecodingSettings.halfSizeColorImage == other.rawDecodingSettings.halfSizeColorImage &&
            previewParameters.size == other.previewParameters.size;
}

bool LoadingDescription::equalsIgnoreReducedVersion(const LoadingDescription &other) const
{
    return filePath == other.filePath;
}

bool LoadingDescription::equalsOrBetterThan(const LoadingDescription &other) const
{
    return filePath == other.filePath &&
            (
             (rawDecodingSettings.halfSizeColorImage == other.rawDecodingSettings.halfSizeColorImage) ||
             other.rawDecodingSettings.halfSizeColorImage
            ) &&
            (
             (previewParameters.size == other.previewParameters.size) ||
              other.previewParameters.size
            );
}

QStringList LoadingDescription::possibleCacheKeys(const QString &filePath)
{
    QStringList keys;
    keys.append(filePath);
    keys.append(filePath + "-halfSizeColorImage");
    keys.append(filePath + "-previewImage");
    return keys;
}


} // namespace Digikam

