/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : class GroupStateComputer
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "geogroupstate.h"

// Qt includes

#include <QScopedPointer>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GroupStateComputer
{
public:

    explicit GroupStateComputer();
    virtual ~GroupStateComputer();

    GeoGroupState getState() const;

    void clear();

    void addState(const GeoGroupState state);
    void addSelectedState(const GeoGroupState state);
    void addFilteredPositiveState(const GeoGroupState state);
    void addRegionSelectedState(const GeoGroupState state);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

#endif // GROUP_STATE_COMPUTER_H
