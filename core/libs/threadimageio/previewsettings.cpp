/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-11-14
 * Description : Settings container for preview settings
 *
 * Copyright (C) 2014 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "previewsettings.h"

namespace Digikam
{

PreviewSettings::PreviewSettings(Quality quality, RawLoading rawLoading)
    : quality(quality),
      rawLoading(rawLoading),
      zoomOrgSize(true),
      convertToEightBit(true)
{
}

bool PreviewSettings::operator==(const PreviewSettings& other) const
{
    return (quality           == other.quality &&
            rawLoading        == other.rawLoading &&
            zoomOrgSize       == other.zoomOrgSize &&
            convertToEightBit == other.convertToEightBit);
}

}   // namespace Digikam

