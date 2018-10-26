/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Container for image info objects
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ITEM_INFO_LIST_H
#define DIGIKAM_ITEM_INFO_LIST_H

// Qt includes

#include <QList>

// Local includes

#include "iteminfo.h"
#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{

class ItemInfo;

// NOTE: implementations of batch loading methods:
// See imageinfo.cpp (next to the corresponding single-item implementation)

class DIGIKAM_DATABASE_EXPORT ItemInfoList : public QList<ItemInfo>
{
public:

    ItemInfoList();
    explicit ItemInfoList(const QList<ItemInfo>& list);
    explicit ItemInfoList(const QList<qlonglong>& idList);

    QList<qlonglong> toImageIdList()  const;
    QList<QUrl>      toImageUrlList() const;

    void loadGroupImageIds() const;
    void loadTagIds()        const;

    bool static namefileLessThan(const ItemInfo& d1, const ItemInfo& d2);

    /**
     * @brief singleGroupMainItem
     * @return If the list contains of items of only one group including the
     * main item, this main item is returned, otherwise a null ItemInfo.
     */
    ItemInfo singleGroupMainItem() const;
};

typedef ItemInfoList::iterator ItemInfoListIterator;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemInfoList)

#endif // DIGIKAM_ITEM_INFO_LIST_H
