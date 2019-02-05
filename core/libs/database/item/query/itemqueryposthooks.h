/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#ifndef DIGIKAM_ITEM_QUERY_POST_HOOKS_H
#define DIGIKAM_ITEM_QUERY_POST_HOOKS_H

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemQueryPostHook
{
public:

    // This is the single hook, ItemQueryPostHookS is the container
    virtual ~ItemQueryPostHook()
    {
    };

    virtual bool checkPosition(double /*latitudeNumber*/, double /*longitudeNumber*/)
    {
        return true;
    };
};

// --------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemQueryPostHooks
{
public:

    explicit ItemQueryPostHooks();
    ~ItemQueryPostHooks();

    /** Call this method after passing the object to buildQuery
     *  and executing the statement. Returns true if the search is matched.
     */
    bool checkPosition(double latitudeNumber, double longitudeNumber);

    /** Called by ItemQueryBuilder. Ownership of the object is passed.
     */
    void addHook(ItemQueryPostHook* const hook);

protected:

    QList<ItemQueryPostHook*> m_postHooks;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_QUERY_POST_HOOKS_H
