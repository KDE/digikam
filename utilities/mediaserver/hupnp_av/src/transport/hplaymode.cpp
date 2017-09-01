/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hplaymode.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HPlayMode
 ******************************************************************************/
HPlayMode::HPlayMode() :
    m_type(Undefined), m_typeAsString()
{
}

HPlayMode::HPlayMode(HPlayMode::Type type) :
    m_type(type), m_typeAsString(HPlayMode::toString(type))
{
}

HPlayMode::HPlayMode(const QString& arg) :
    m_type(), m_typeAsString()
{
    QString trimmed = arg.trimmed();
    m_type = fromString(trimmed);
    m_typeAsString = trimmed;
}

QString HPlayMode::toString(HPlayMode::Type type)
{
    QString retVal;
    switch(type)
    {
    case Undefined:
    case VendorDefined:
        break;
    case Normal:
        retVal = QLatin1String("NORMAL");
        break;
    case Shuffle:
        retVal = QLatin1String("SHUFFLE");
        break;
    case RepeatOne:
        retVal = QLatin1String("REPEAT_ONE");
        break;
    case RepeatAll:
        retVal = QLatin1String("REPEAT_ALL");
        break;
    case Random:
        retVal = QLatin1String("RANDOM");
        break;
    case Direct_1:
        retVal = QLatin1String("DIRECT_1");
        break;
    case Intro:
        retVal = QLatin1String("INTRO");
        break;
    }
    return retVal;
}

HPlayMode::Type HPlayMode::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare(QLatin1String("NORMAL"), Qt::CaseInsensitive) == 0)
    {
        retVal = Normal;
    }
    else if (type.compare(QLatin1String("SHUFFLE"), Qt::CaseInsensitive) == 0)
    {
        retVal = Shuffle;
    }
    else if (type.compare(QLatin1String("REPEAT_ONE"), Qt::CaseInsensitive) == 0)
    {
        retVal = RepeatOne;
    }
    else if (type.compare(QLatin1String("REPEAT_ALL"), Qt::CaseInsensitive) == 0)
    {
        retVal = RepeatAll;
    }
    else if (type.compare(QLatin1String("RANDOM"), Qt::CaseInsensitive) == 0)
    {
        retVal = Random;
    }
    else if (type.compare(QLatin1String("DIRECT_1"), Qt::CaseInsensitive) == 0)
    {
        retVal = Direct_1;
    }
    else if (type.compare(QLatin1String("INTRO"), Qt::CaseInsensitive) == 0)
    {
        retVal = Intro;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HPlayMode& obj1, const HPlayMode& obj2)
{
    return obj1.toString() == obj2.toString();
}

}
}
}
