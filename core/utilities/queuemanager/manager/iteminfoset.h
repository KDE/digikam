/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : queue pool info container.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ITEM_INFO_SET_H
#define ITEM_INFO_SET_H

// Qt includes

#include <QList>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

/** A container of associated ImageInfo and queue id.
 */
class ItemInfoSet
{
public:

    ItemInfoSet()
        : queueId(0)
    {
    };

    ItemInfoSet(int id, const ImageInfo& inf)
    {
        queueId = id;
        info    = inf;
    };

    ~ItemInfoSet()
    {
    };

    int       queueId;
    ImageInfo info;
};

/** A list of all queued items from the pool.
 */
typedef QList<ItemInfoSet> QueuePoolItemsList;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemInfoSet)
Q_DECLARE_METATYPE(Digikam::QueuePoolItemsList)

#endif // ITEM_INFO_SET_H
