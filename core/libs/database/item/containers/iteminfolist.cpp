/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Container for image info objects
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iteminfolist.h"

// Local includes

#include "iteminfo.h"

namespace Digikam
{

ItemInfoList::ItemInfoList()
{
}

ItemInfoList::ItemInfoList(const QList<ItemInfo>& list)
    : QList<ItemInfo>(list)
{
}

ItemInfoList::ItemInfoList(const QList<qlonglong>& idList)
{
    foreach (const qlonglong& id, idList)
    {
        append(ItemInfo(id));
    }
}

QList<qlonglong> ItemInfoList::toImageIdList() const
{
    QList<qlonglong> idList;

    foreach (const ItemInfo& info, *this)
    {
        idList << info.id();
    }

    return idList;
}

QList<QUrl> ItemInfoList::toImageUrlList() const
{
    QList<QUrl> urlList;

    foreach (const ItemInfo& info, *this)
    {
        urlList << info.fileUrl();
    }

    return urlList;
}

bool ItemInfoList::namefileLessThan(const ItemInfo& d1, const ItemInfo& d2)
{
    return d1.name().toLower() < d2.name().toLower(); // sort by name
}

ItemInfo ItemInfoList::singleGroupMainItem() const
{
    if (length() == 1)
    {
        return first();
    }

    ItemInfo mainItem;
    ItemInfoList grouped;

    if (first().isGrouped())
    {
        mainItem = first().groupImage();

        if (!this->contains(mainItem))
        {
            return ItemInfo();
        }
    }
    else if (first().hasGroupedImages())
    {
        mainItem = first();
    }
    else
    {
        return ItemInfo();
    }

    grouped << mainItem << mainItem.groupedImages();

    foreach (const ItemInfo& info, *this)
    {
        if (!grouped.contains(info))
        {
            return ItemInfo();
        }
    }

    return mainItem;
}

} // namespace Digikam
