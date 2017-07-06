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

#include "htransportstate.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HTransportState
 ******************************************************************************/
HTransportState::HTransportState() :
    m_type(Undefined), m_typeAsString()
{
}

HTransportState::HTransportState(Type type) :
    m_type(type), m_typeAsString(toString(type))
{
}

HTransportState::HTransportState(const QString& arg) :
    m_type(), m_typeAsString()
{
    QString trimmed = arg.trimmed();
    m_type = fromString(trimmed);
    m_typeAsString = trimmed;
}

QString HTransportState::toString(HTransportState::Type type)
{
    QString retVal;
    switch(type)
    {
    case Undefined:
        break;
    case NoMediaPresent:
        retVal = QLatin1String("NO_MEDIA_PRESENT");
        break;
    case Stopped:
        retVal = QLatin1String("STOPPED");
        break;
    case Playing:
        retVal = QLatin1String("PLAYING");
        break;
    case Transitioning:
        retVal = QLatin1String("TRANSITIONING");
        break;
    case PausedPlayback:
        retVal = QLatin1String("PAUSED_PLAYBACK");
        break;
    case PausedRecording:
        retVal = QLatin1String("PAUSED_RECORDING");
        break;
    case Recording:
        retVal = QLatin1String("RECORDING");
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

HTransportState::Type HTransportState::fromString(const QString& type)
{
    Type retVal = Undefined;
    if (type.compare(QLatin1String("NO_MEDIA_PRESENT"), Qt::CaseInsensitive) == 0)
    {
        retVal = NoMediaPresent;
    }
    else if (type.compare(QLatin1String("STOPPED"), Qt::CaseInsensitive) == 0)
    {
        retVal = Stopped;
    }
    else if (type.compare(QLatin1String("PLAYING"), Qt::CaseInsensitive) == 0)
    {
        retVal = Playing;
    }
    else if (type.compare(QLatin1String("TRANSITIONING"), Qt::CaseInsensitive) == 0)
    {
        retVal = Transitioning;
    }
    else if (type.compare(QLatin1String("PAUSED_PLAYBACK"), Qt::CaseInsensitive) == 0)
    {
        retVal = PausedPlayback;
    }
    else if (type.compare(QLatin1String("PAUSED_RECORDING"), Qt::CaseInsensitive) == 0)
    {
        retVal = PausedRecording;
    }
    else if (type.compare(QLatin1String("RECORDING"), Qt::CaseInsensitive) == 0)
    {
        retVal = Recording;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HTransportState& obj1, const HTransportState& obj2)
{
    return obj1.toString() == obj2.toString();
}

}
}
}
