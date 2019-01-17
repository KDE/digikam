/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_DIGIKAMITEM_DELEGATE_H
#define DIGIKAM_DIGIKAMITEM_DELEGATE_H

// Local includes

#include "itemdelegate.h"

namespace Digikam
{

class ItemCategoryDrawer;
class DigikamItemDelegatePrivate;

class DigikamItemDelegate : public ItemDelegate
{
    Q_OBJECT

public:

    explicit DigikamItemDelegate(ItemCategorizedView* parent);
    ~DigikamItemDelegate();

protected:

    virtual void updateRects();

    DigikamItemDelegate(DigikamItemDelegatePrivate& dd, ItemCategorizedView* parent);

private:

    Q_DECLARE_PRIVATE(DigikamItemDelegate)
};

} // namespace Digikam

#endif // DIGIKAM_DIGIKAMITEM_DELEGATE_H
