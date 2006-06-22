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
#include "rawdecodingsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadingDescription
{
public:

    /*
        Use this for files that are not raw files.
        Stores only the filePath.
    */
    LoadingDescription(const QString &filePath)
        : filePath(filePath)
        {
            rawDecodingSettings = RawDecodingSettings();
        };

    /*
        For raw files:
        Stores filePath and RawDecodingSettings
    */
    LoadingDescription(const QString &filePath, RawDecodingSettings settings)
        : filePath(filePath), rawDecodingSettings(settings)
        {};

    QString             filePath;
    RawDecodingSettings rawDecodingSettings;

    QString             cacheKey() const;
    QStringList         lookupCacheKeys() const;
    bool operator==(const LoadingDescription &other) const;
};

}   // namespace Digikam

#endif // LOADING_DESCRIPTION_H
