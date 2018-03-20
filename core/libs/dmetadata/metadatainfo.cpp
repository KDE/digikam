/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-12
 * Description : Metadata info containers
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    return city.isNull()          &&
           country.isNull()       &&
           address.isNull()       &&
           postalCode.isNull()    &&
           provinceState.isNull() &&
           email.isNull()         &&
           phone.isNull()         &&
           webUrl.isNull();
}

bool IptcCoreContactInfo::isEmpty() const
{
    return city.isEmpty()          &&
           country.isEmpty()       &&
           address.isEmpty()       &&
           postalCode.isEmpty()    &&
           provinceState.isEmpty() &&
           email.isEmpty()         &&
           phone.isEmpty()         &&
           webUrl.isEmpty();
}

bool IptcCoreContactInfo::operator==(const IptcCoreContactInfo& t) const
{
    bool b1 = city          == t.city;
    bool b2 = country       == t.country;
    bool b3 = address       == t.address;
    bool b4 = postalCode    == t.postalCode;
    bool b5 = provinceState == t.provinceState;
    bool b6 = email         == t.email;
    bool b7 = phone         == t.phone;
    bool b8 = webUrl        == t.webUrl;

    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8;
}

QDebug operator<<(QDebug dbg, const IptcCoreContactInfo& inf)
{
    dbg.nospace() << "IptcCoreContactInfo::city: "
                  << inf.city << ", ";
    dbg.nospace() << "IptcCoreContactInfo::country: "
                  << inf.country << ", ";
    dbg.nospace() << "IptcCoreContactInfo::address: "
                  << inf.address << ", ";
    dbg.nospace() << "IptcCoreContactInfo::postalCode: "
                  << inf.postalCode << ", ";
    dbg.nospace() << "IptcCoreContactInfo::provinceState: "
                  << inf.provinceState << ", ";
    dbg.nospace() << "IptcCoreContactInfo::email: "
                  << inf.email << ", ";
    dbg.nospace() << "IptcCoreContactInfo::phone: "
                  << inf.phone << ", ";
    dbg.nospace() << "IptcCoreContactInfo::webUrl: "
                  << inf.webUrl;
    return dbg.space();
}

// -------------------------------------------------------------------------

bool IptcCoreLocationInfo::isEmpty() const
{
    return country.isEmpty()       &&
           countryCode.isEmpty()   &&
           provinceState.isEmpty() &&
           city.isEmpty()          &&
           location.isEmpty();
}

bool IptcCoreLocationInfo::isNull() const
{
    return country.isNull()       &&
           countryCode.isNull()   &&
           provinceState.isNull() &&
           city.isNull()          &&
           location.isNull();
}

bool IptcCoreLocationInfo::operator==(const IptcCoreLocationInfo& t) const
{
    bool b1 = country       == t.country;
    bool b2 = countryCode   == t.countryCode;
    bool b3 = provinceState == t.provinceState;
    bool b4 = city          == t.city;
    bool b5 = location      == t.location;

    return b1 && b2 && b3 && b4 && b5;
}

QDebug operator<<(QDebug dbg, const IptcCoreLocationInfo& inf)
{
    dbg.nospace() << "IptcCoreLocationInfo::country: "
                  << inf.country << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::countryCode: "
                  << inf.countryCode << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::provinceState: "
                  << inf.provinceState << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::city: "
                  << inf.city << ", ";
    dbg.nospace() << "IptcCoreLocationInfo::location: "
                  << inf.location;
    return dbg.space();
}

} // namespace Digikam
