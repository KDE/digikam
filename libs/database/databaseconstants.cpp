/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-26
 * Description : Constants in the database
 *
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databaseconstants.h"

// Qt includes

namespace Digikam
{


QLatin1String TagPropertyName::person()
{
    return QLatin1String("person");
}

QLatin1String TagPropertyName::unknownPerson()
{
    return QLatin1String("unknownPerson");
}

QLatin1String TagPropertyName::kfaceId()
{
    return QLatin1String("kfaceId");
}

QLatin1String ImageTagPropertyName::tagRegion()
{
    return QLatin1String("tagRegion");
}

QLatin1String ImageTagPropertyName::autodetectedFace()
{
    return QLatin1String("autodetectedFace");
}

QLatin1String ImageTagPropertyName::faceToTrain()
{
    return QLatin1String("faceToTrain");
}

}



