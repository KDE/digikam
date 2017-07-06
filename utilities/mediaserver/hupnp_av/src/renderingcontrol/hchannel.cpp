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

#include "hchannel.h"
#include <../../HUpnpCore/private/hmisc_utils_p.h>

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HChannel
 ******************************************************************************/
HChannel::HChannel() :
    m_type(), m_typeAsString()
{
}

HChannel::HChannel(Type type) :
    m_type(type), m_typeAsString(toString(type))
{
}

HChannel::HChannel(const QString& arg) :
    m_type(), m_typeAsString()
{
    QString trimmed = arg.trimmed();
    m_type = fromString(trimmed);
    m_typeAsString = trimmed;
}

QString HChannel::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case Master:
        retVal = QLatin1String("Master");
        break;
    case LeftFront:
        retVal = QLatin1String("LF");
        break;
    case RightFront:
        retVal = QLatin1String("RF");
        break;
    case CenterFront:
        retVal = QLatin1String("CF");
        break;
    case LFE:
        retVal = QLatin1String("LFE");
        break;
    case LeftSurround:
        retVal = QLatin1String("LS");
        break;
    case RightSurround:
        retVal = QLatin1String("RS");
        break;
    case LeftOfCenter:
        retVal = QLatin1String("LFC");
        break;
    case RightOfCenter:
        retVal = QLatin1String("RFC");
        break;
    case Surround:
        retVal = QLatin1String("SD");
        break;
    case SideLeft:
        retVal = QLatin1String("SL");
        break;
    case SideRight:
        retVal = QLatin1String("SR");
        break;
    case Top:
        retVal = QLatin1String("T");
        break;
    case Bottom:
        retVal = QLatin1String("B");
        break;
    default:
        break;
    }
    return retVal;
}

HChannel::Type HChannel::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare(QLatin1String("MASTER"), Qt::CaseInsensitive) == 0)
    {
        retVal = Master;
    }
    else if (type.compare(QLatin1String("LF"), Qt::CaseInsensitive) == 0)
    {
        retVal = LeftFront;
    }
    else if (type.compare(QLatin1String("RF"), Qt::CaseInsensitive) == 0)
    {
        retVal = RightFront;
    }
    else if (type.compare(QLatin1String("CF"), Qt::CaseInsensitive) == 0)
    {
        retVal = CenterFront;
    }
    else if (type.compare(QLatin1String("LFE"), Qt::CaseInsensitive) == 0)
    {
        retVal = LFE;
    }
    else if (type.compare(QLatin1String("LS"), Qt::CaseInsensitive) == 0)
    {
        retVal = LeftSurround;
    }
    else if (type.compare(QLatin1String("RS"), Qt::CaseInsensitive) == 0)
    {
        retVal = RightSurround;
    }
    else if (type.compare(QLatin1String("LFC"), Qt::CaseInsensitive) == 0)
    {
        retVal = LeftOfCenter;
    }
    else if (type.compare(QLatin1String("RFC"), Qt::CaseInsensitive) == 0)
    {
        retVal = RightOfCenter;
    }
    else if (type.compare(QLatin1String("SD"), Qt::CaseInsensitive) == 0)
    {
        retVal = Surround;
    }
    else if (type.compare(QLatin1String("SL"), Qt::CaseInsensitive) == 0)
    {
        retVal = SideLeft;
    }
    else if (type.compare(QLatin1String("SR"), Qt::CaseInsensitive) == 0)
    {
        retVal = SideRight;
    }
    else if (type.compare(QLatin1String("T"), Qt::CaseInsensitive) == 0)
    {
        retVal = Top;
    }
    else if (type.compare(QLatin1String("B"), Qt::CaseInsensitive) == 0)
    {
        retVal = Bottom;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

QSet<HChannel> HChannel::allChannels()
{
    QSet<HChannel> retVal;
    retVal.insert(Master);
    retVal.insert(LeftFront);
    retVal.insert(RightFront);
    retVal.insert(CenterFront);
    retVal.insert(LFE);
    retVal.insert(LeftSurround);
    retVal.insert(RightSurround);
    retVal.insert(LeftOfCenter);
    retVal.insert(RightOfCenter);
    retVal.insert(Surround);
    retVal.insert(SideLeft);
    retVal.insert(SideRight);
    retVal.insert(Top);
    retVal.insert(Bottom);
    return retVal;
}

bool operator==(const HChannel& obj1, const HChannel& obj2)
{
    return obj1.toString() == obj2.toString();
}

quint32 qHash(const HChannel& key)
{
    QByteArray data = key.toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
}
