/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : database SQL queries helper class
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemqueryposthooks.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

ItemQueryPostHooks::ItemQueryPostHooks()
{
}

ItemQueryPostHooks::~ItemQueryPostHooks()
{
    foreach (ItemQueryPostHook* const hook, m_postHooks)
    {
        delete hook;
    }
}

void ItemQueryPostHooks::addHook(ItemQueryPostHook* const hook)
{
    m_postHooks << hook;
}

bool ItemQueryPostHooks::checkPosition(double latitudeNumber, double longitudeNumber)
{
    foreach (ItemQueryPostHook* const hook, m_postHooks)
    {
        if (!hook->checkPosition(latitudeNumber, longitudeNumber))
        {
            return false;
        }
    }

    return true;
}

} // namespace Digikam
