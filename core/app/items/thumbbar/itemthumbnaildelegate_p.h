/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : thumbnail bar for items - the delegate
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ITEM_THUMBNAIL_DELEGATE_P_H
#define DIGIKAM_ITEM_THUMBNAIL_DELEGATE_P_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "itemdelegate_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemThumbnailDelegatePrivate : public ItemDelegate::ItemDelegatePrivate
{
public:

    explicit ItemThumbnailDelegatePrivate()
    {
        flow                = QListView::LeftToRight;

        // switch off drawing of frames
        drawMouseOverFrame  = false;
        drawFocusFrame      = false;

        // switch off composing rating over background
        ratingOverThumbnail = true;
    }

    QListView::Flow flow;
    QRect           viewSize;

public:

    void init(ItemThumbnailDelegate* const q);
};

} // namespace Digikam

#endif // DIGIKAM_ITEMS_THUMBNAIL_DELEGATE_P_H
