/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-12
 * Description : Metadata info containers
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "metadatainfo.h"

namespace Digikam
{

bool IptcCoreContactInfo::isNull() const
{
    return city.isNull() &&
           country.isNull() &&
           address.isNull() &&
           postalCode.isNull() &&
           provinceState.isNull() &&
           email.isNull() &&
           phone.isNull() &&
           webUrl.isNull();
}

bool IptcCoreLocationInfo::isNull() const
{
    return country.isNull() &&
           countryCode.isNull() &&
           provinceState.isNull() &&
           city.isNull() &&
           location.isNull();
}

} // namespace Digikam
