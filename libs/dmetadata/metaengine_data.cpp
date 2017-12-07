/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Shared data container.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "metaengine_data.h"
#include "metaengine.h"
#include "metaengine_p.h"

namespace Digikam
{

MetaEngineData::MetaEngineData()
    : d(0)
{
}

MetaEngineData::MetaEngineData(const MetaEngineData& other)
{
    d = other.d;
}

MetaEngineData::~MetaEngineData()
{
}

MetaEngineData& MetaEngineData::operator=(const MetaEngineData& other)
{
    d = other.d;
    return *this;
}

}  // namespace Digikam
