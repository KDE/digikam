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

// Local includes

#include "imageinfolist.h"

// Qt includes

#include <QMimeData>

namespace Digikam
{

bool GroupingViewImplementation::needGroupResolving(ApplicationSettings::OperationType type, const ImageInfoList& infos) const
{
    ApplicationSettings::ApplyToEntireGroup applyAll =
            ApplicationSettings::instance()->getGroupingOperateOnAll(type);

    if (applyAll == ApplicationSettings::No)
    {
        return false;
    }

    foreach(const ImageInfo& info, infos)
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

ImageInfoList GroupingViewImplementation::resolveGrouping(const ImageInfoList& infos) const
{
    ImageInfoList outInfos;

    foreach(const ImageInfo& info, infos)
    {
        outInfos << info;

        if (hasHiddenGroupedImages(info))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

ImageInfoList GroupingViewImplementation::getHiddenGroupedInfos(const ImageInfoList& infos) const
{
    ImageInfoList outInfos;

    foreach(const ImageInfo& info, infos)
    {
        if (hasHiddenGroupedImages(info))
        {
            outInfos << info.groupedImages();
        }
    }

    return outInfos;
}

} // namespace Digikam
