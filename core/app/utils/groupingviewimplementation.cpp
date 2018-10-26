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

#include "groupingviewimplementation.h"

// Qt includes

#include <QMimeData>

// Local includes

#include "iteminfolist.h"

namespace Digikam
{

bool GroupingViewImplementation::needGroupResolving(ApplicationSettings::OperationType type, const ItemInfoList& infos) const
{
    ApplicationSettings::ApplyToEntireGroup applyAll =
            ApplicationSettings::instance()->getGroupingOperateOnAll(type);

    if (applyAll == ApplicationSettings::No)
    {
        return false;
    }

    foreach(const ItemInfo& info, infos)
    {
        if (hasHiddenGroupedImages(info))
        {
            if (applyAll == ApplicationSettings::Yes)
            {
                return true;
            }

            return ApplicationSettings::instance()->askGroupingOperateOnAll(type);
        }
    }

    return false;
}

ItemInfoList GroupingViewImplementation::resolveGrouping(const ItemInfoList& infos) const
{
    ItemInfoList outInfos;

    foreach(const ItemInfo& info, infos)
    {
        outInfos << info;

        if (hasHiddenGroupedImages(info))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

ItemInfoList GroupingViewImplementation::getHiddenGroupedInfos(const ItemInfoList& infos) const
{
    ItemInfoList outInfos;

    foreach(const ItemInfo& info, infos)
    {
        if (hasHiddenGroupedImages(info))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

} // namespace Digikam
