/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  class GroupStateComputer
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GROUPSTATECOMPUTER_H
#define GROUPSTATECOMPUTER_H

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

#endif // GROUPSTATECOMPUTER_H
