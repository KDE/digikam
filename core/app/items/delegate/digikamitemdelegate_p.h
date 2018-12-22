/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_DIGIKAMITEM_DELEGATE_P_H
#define DIGIKAM_DIGIKAMITEM_DELEGATE_P_H

// Qt includes

#include <QCache>

// Local includes

#include "itemdelegate_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DigikamItemDelegatePrivate : public ItemDelegate::ItemDelegatePrivate
{
public:

    explicit DigikamItemDelegatePrivate()
    {
    }

    virtual ~DigikamItemDelegatePrivate();

    void init(DigikamItemDelegate* const q, ItemCategorizedView* const parent);
};

// -----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemFaceDelegatePrivate : public DigikamItemDelegatePrivate
{
public:

    explicit ItemFaceDelegatePrivate()
    {
    }
};

} // namespace Digikam

#endif // DIGIKAM_DIGIKAMITEM_DELEGATE_P_H
