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

#include "htransportaction.h"

#include "hmisc_utils_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HTransportAction
 ******************************************************************************/
HTransportAction::HTransportAction() :
    m_type(Undefined), m_typeAsString()
{
}

HTransportAction::HTransportAction(Type action) :
    m_type(action), m_typeAsString(toString(action))
{
}

HTransportAction::HTransportAction(const QString& arg) :
    m_type(), m_typeAsString()
{
    QString tmp = arg.trimmed();
    m_type = fromString(tmp);
    m_typeAsString = tmp;
}

QString HTransportAction::toString(Type type)
{
    QString retVal = QLatin1String("");
    switch(type)
    {
    case Undefined:
    case VendorDefined:
        break;
    case Play:
        retVal = QLatin1String("PLAY");
        break;
    case Stop:
        retVal = QLatin1String("STOP");
        break;
    case Pause:
        retVal = QLatin1String("PAUSE");
        break;
    case Seek:
        retVal = QLatin1String("SEEK");
        break;
    case Next:
        retVal = QLatin1String("NEXT");
        break;
    case Previous:
        retVal = QLatin1String("PREVIOUS");
        break;
    case Record:
        retVal = QLatin1String("RECORD");
        break;
    }
    return retVal;
}

HTransportAction::Type HTransportAction::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare(QLatin1String("PLAY"), Qt::CaseInsensitive) == 0)
    {
        retVal = Play;
    }
    else if (type.compare(QLatin1String("STOP"), Qt::CaseInsensitive) == 0)
    {
        retVal = Stop;
    }
    else if (type.compare(QLatin1String("PAUSE"), Qt::CaseInsensitive) == 0)
    {
        retVal = Pause;
    }
    else if (type.compare(QLatin1String("SEEK"), Qt::CaseInsensitive) == 0)
    {
        retVal = Seek;
    }
    else if (type.compare(QLatin1String("NEXT"), Qt::CaseInsensitive) == 0)
    {
        retVal = Next;
    }
    else if (type.compare(QLatin1String("PREVIOUS"), Qt::CaseInsensitive) == 0)
    {
        retVal = Previous;
    }
    else if (type.compare(QLatin1String("RECORD"), Qt::CaseInsensitive) == 0)
    {
        retVal = Record;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

QSet<HTransportAction> HTransportAction::allActions()
{
    QSet<HTransportAction> retVal;
    retVal.insert(Play);
    retVal.insert(Stop);
    retVal.insert(Pause);
    retVal.insert(Seek);
    retVal.insert(Next);
    retVal.insert(Previous);
    retVal.insert(Record);
    return retVal;
}

bool operator==(const HTransportAction& obj1, const HTransportAction& obj2)
{
    return obj1.toString() == obj2.toString();
}

quint32 qHash(const HTransportAction& key)
{
    QByteArray data = key.toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
}
