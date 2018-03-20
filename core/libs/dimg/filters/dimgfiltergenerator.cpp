/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimgfiltergenerator.h"

namespace Digikam
{

bool DImgFilterGenerator::isSupported(const QString& filterIdentifier)
{
    return supportedFilters().contains(filterIdentifier);
}

bool DImgFilterGenerator::isSupported(const QString& filterIdentifier, int version)
{
    if (isSupported(filterIdentifier))
    {
        return supportedVersions(filterIdentifier).contains(version);
    }

    return false;
}

} // namespace Digikam
