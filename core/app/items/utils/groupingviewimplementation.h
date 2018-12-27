/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-11-02
 * Description : Implementation of grouping specific functions for views
 *
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
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

#ifndef DIGIKAM_GROUPING_VIEW_IMPLEMENTATION_H
#define DIGIKAM_GROUPING_VIEW_IMPLEMENTATION_H

// Local includes

#include "applicationsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemInfo;
class ItemInfoList;

class DIGIKAM_EXPORT GroupingViewImplementation
{

public:

    virtual ~GroupingViewImplementation()
    {
    }

    // must be implemented by parent view
    virtual bool  hasHiddenGroupedImages(const ItemInfo&) const
    {
        return false;
    }

    bool          needGroupResolving(ApplicationSettings::OperationType type,
                                     const ItemInfoList& infos)    const;

    ItemInfoList resolveGrouping(const ItemInfoList& infos)       const;
    ItemInfoList getHiddenGroupedInfos(const ItemInfoList& infos) const;
};

} // namespace Digikam

#endif // DIGIKAM_GROUPING_VIEW_IMPLEMENTATION_H
