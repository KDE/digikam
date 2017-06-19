/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : class GroupStateComputer
 *
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GROUP_STATE_COMPUTER_H
#define GROUP_STATE_COMPUTER_H

#include "groupstate.h"

// Qt includes

#include <QScopedPointer>

// Local includes

#include "digikam_export.h"

namespace GeoIface
{

class DIGIKAM_EXPORT GroupStateComputer
{
public:

    GroupStateComputer();
    virtual ~GroupStateComputer();

    GroupState getState() const;

    void clear();

    void addState(const GroupState state);
    void addSelectedState(const GroupState state);
    void addFilteredPositiveState(const GroupState state);
    void addRegionSelectedState(const GroupState state);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace GeoIface

#endif // GROUP_STATE_COMPUTER_H
