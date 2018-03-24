/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Container for image info objects
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imageinfolist.h"

// Local includes

#include "imageinfo.h"

namespace Digikam
{

ImageInfoList::ImageInfoList(const QList<qlonglong>& idList)
{
    foreach(const qlonglong& id, idList)
    {
        append(ImageInfo(id));
    }
}

QList<qlonglong> ImageInfoList::toImageIdList() const
{
    QList<qlonglong> idList;

    foreach(const ImageInfo& info, *this)
    {
        idList << info.id();
    }

    return idList;
}

QList<QUrl> ImageInfoList::toImageUrlList() const
{
    QList<QUrl> urlList;

    foreach(const ImageInfo& info, *this)
    {
        urlList << info.fileUrl();
    }

    return urlList;
}

bool ImageInfoList::namefileLessThan(const ImageInfo& d1, const ImageInfo& d2)
{
    return d1.name().toLower() < d2.name().toLower(); // sort by name
}

ImageInfo ImageInfoList::singleGroupMainItem() const
{
    if (length() == 1)
    {
        return first();
    }

    ImageInfo mainItem;
    ImageInfoList grouped;

    if (first().isGrouped())
    {
        mainItem = first().groupImage();

        if (!this->contains(mainItem))
        {
            return ImageInfo();
        }
    }
    else if (first().hasGroupedImages())
    {
        mainItem = first();
    }
    else
    {
        return ImageInfo();
    }

    grouped << mainItem << mainItem.groupedImages();

    foreach(const ImageInfo& info, *this)
    {
        if (!grouped.contains(info))
        {
            return ImageInfo();
        }
    }

    return mainItem;
}

// Implementations of batch loading methods: See imageinfo.cpp (next to the corresponding single-item implementation)

} // namespace Digikam
