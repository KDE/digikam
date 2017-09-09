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

#include "hstoragemedium.h"

#include "hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HStorageMedium
 ******************************************************************************/
HStorageMedium::HStorageMedium() :
    m_typeAsString(toString(Unknown)), m_type(Unknown)
{
}

HStorageMedium::HStorageMedium(Type arg) :
    m_typeAsString(toString(arg)), m_type(arg)
{
}

HStorageMedium::HStorageMedium(const QString& arg) :
    m_typeAsString(), m_type()
{
    QString trimmed = arg.trimmed();
    m_typeAsString = trimmed;
    m_type = fromString(trimmed);
}

QString HStorageMedium::toString(Type type)
{
    QString retVal;
    switch(type)
    {
    case Unknown:
        retVal = QLatin1String("UNKNOWN");
        break;
    case DigitalVideo:
        retVal = QLatin1String("DV");
        break;
    case MiniDigitalVideo:
        retVal = QLatin1String("MINI-DV");
        break;
    case VHS:
        retVal = QLatin1String("VHS");
        break;
    case W_VHS:
        retVal = QLatin1String("W-VHS");
        break;
    case S_VHS:
        retVal = QLatin1String("S-VHS");
        break;
    case D_VHS:
        retVal = QLatin1String("D-VHS");
        break;
    case VHSC:
        retVal = QLatin1String("VHSC");
        break;
    case Video8:
        retVal = QLatin1String("VIDEO8");
        break;
    case HI8:
        retVal = QLatin1String("HI8");
        break;
    case CD_ROM:
        retVal = QLatin1String("CD-ROM");
        break;
    case CD_DA:
        retVal = QLatin1String("CD-DA");
        break;
    case CD_R:
        retVal = QLatin1String("CD-R");
        break;
    case CD_RW:
        retVal = QLatin1String("CD-RW");
        break;
    case Video_CD:
        retVal = QLatin1String("VIDEO-CD");
        break;
    case SACD:
        retVal = QLatin1String("SACD");
        break;
    case MiniDiscAudio:
        retVal = QLatin1String("MD-AUDIO");
        break;
    case MiniDiscPicture:
        retVal = QLatin1String("MD-PICTURE");
        break;
    case DVD_ROM:
        retVal = QLatin1String("DVD-ROM");
        break;
    case DVD_Video:
        retVal = QLatin1String("DVD-VIDEO");
        break;
    case DVD_PlusRecordable:
        retVal = QLatin1String("DVD+R");
        break;
    case DVD_MinusRecordable:
        retVal = QLatin1String("DVD-R");
        break;
    case DVD_PlusRewritable:
        retVal = QLatin1String("DVD+RW");
        break;
    case DVD_MinusRewritable:
        retVal = QLatin1String("DVD-RW");
        break;
    case DVD_RAM:
        retVal = QLatin1String("DVD-RAM");
        break;
    case DVD_Audio:
        retVal = QLatin1String("DVD-AUDIO");
        break;
    case DAT:
        retVal = QLatin1String("DAT");
        break;
    case LD:
        retVal = QLatin1String("LD");
        break;
    case HDD:
        retVal = QLatin1String("HDD");
        break;
    case MicroMV:
        retVal = QLatin1String("MICRO-MV");
        break;
    case Network:
        retVal = QLatin1String("NETWORK");
        break;
    case None:
        retVal = QLatin1String("NONE");
        break;
    case NotImplemented:
        retVal = QLatin1String("NOT_IMPLEMENTED");
        break;
    case SecureDigital:
        retVal = QLatin1String("SD");
        break;
    case PC_Card:
        retVal = QLatin1String("PC-CARD");
        break;
    case MultimediaCard:
        retVal = QLatin1String("MMC");
        break;
    case CompactFlash:
        retVal = QLatin1String("CF");
        break;
    case BluRay:
        retVal = QLatin1String("BD");
        break;
    case MemoryStick:
        retVal = QLatin1String("MS");
        break;
    case HD_DVD:
        retVal = QLatin1String("HD_DVD");
        break;
    default:
        break;
    }
    return retVal;
}

HStorageMedium::Type HStorageMedium::fromString(const QString& type)
{
    Type retVal = Unknown;
    if (type.compare(QLatin1String("UNKNOWN"), Qt::CaseInsensitive) == 0)
    {
        retVal = Unknown;
    }
    else if (type.compare(QLatin1String("DV"), Qt::CaseInsensitive) == 0)
    {
        retVal = DigitalVideo;
    }
    else if (type.compare(QLatin1String("VHS"), Qt::CaseInsensitive) == 0)
    {
        retVal = VHS;
    }
    else if (type.compare(QLatin1String("W-VHS"), Qt::CaseInsensitive) == 0)
    {
        retVal = W_VHS;
    }
    else if (type.compare(QLatin1String("S-VHS"), Qt::CaseInsensitive) == 0)
    {
        retVal = S_VHS;
    }
    else if (type.compare(QLatin1String("D_VHS"), Qt::CaseInsensitive) == 0)
    {
        retVal = D_VHS;
    }
    else if (type.compare(QLatin1String("VHSC"), Qt::CaseInsensitive) == 0)
    {
        retVal = VHSC;
    }
    else if (type.compare(QLatin1String("VIDEO8"), Qt::CaseInsensitive) == 0)
    {
        retVal = Video8;
    }
    else if (type.compare(QLatin1String("HI8"), Qt::CaseInsensitive) == 0)
    {
        retVal = HI8;
    }
    else if (type.compare(QLatin1String("CD-ROM"), Qt::CaseInsensitive) == 0)
    {
        retVal = CD_ROM;
    }
    else if (type.compare(QLatin1String("CD-DA"), Qt::CaseInsensitive) == 0)
    {
        retVal = CD_DA;
    }
    else if (type.compare(QLatin1String("CD-R"), Qt::CaseInsensitive) == 0)
    {
        retVal = CD_R;
    }
    else if (type.compare(QLatin1String("CD-RW"), Qt::CaseInsensitive) == 0)
    {
        retVal = CD_RW;
    }
    else if (type.compare(QLatin1String("VIDEO-CD"), Qt::CaseInsensitive) == 0)
    {
        retVal = Video_CD;
    }
    else if (type.compare(QLatin1String("SACD"), Qt::CaseInsensitive) == 0)
    {
        retVal = SACD;
    }
    else if (type.compare(QLatin1String("MD-AUDIO"), Qt::CaseInsensitive) == 0)
    {
        retVal = MiniDiscAudio;
    }
    else if (type.compare(QLatin1String("MD-PICTURE"), Qt::CaseInsensitive) == 0)
    {
        retVal = MiniDiscPicture;
    }
    else if (type.compare(QLatin1String("DVD-ROM"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_ROM;
    }
    else if (type.compare(QLatin1String("DVD-VIDEO"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_Video;
    }
    else if (type.compare(QLatin1String("DVD+R"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_PlusRecordable;
    }
    else if (type.compare(QLatin1String("DVD-R"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_MinusRecordable;
    }
    else if (type.compare(QLatin1String("DVD+RW"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_PlusRewritable;
    }
    else if (type.compare(QLatin1String("DVD-RW"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_MinusRewritable;
    }
    else if (type.compare(QLatin1String("DVD-RAM"), Qt::CaseInsensitive) == 0)
    {
        retVal = DVD_RAM;
    }
    else if (type.compare(QLatin1String("DAT"), Qt::CaseInsensitive) == 0)
    {
        retVal = DAT;
    }
    else if (type.compare(QLatin1String("LD"), Qt::CaseInsensitive) == 0)
    {
        retVal = LD;
    }
    else if (type.compare(QLatin1String("HDD"), Qt::CaseInsensitive) == 0)
    {
        retVal = HDD;
    }
    else if (type.compare(QLatin1String("MICRO-MV"), Qt::CaseInsensitive) == 0)
    {
        retVal = MicroMV;
    }
    else if (type.compare(QLatin1String("NETWORK"), Qt::CaseInsensitive) == 0)
    {
        retVal = Network;
    }
    else if (type.compare(QLatin1String("NONE"), Qt::CaseInsensitive) == 0)
    {
        retVal = None;
    }
    else if (type.compare(QLatin1String("NOT_IMPLEMENTED"), Qt::CaseInsensitive) == 0)
    {
        retVal = NotImplemented;
    }
    else if (type.compare(QLatin1String("SD"), Qt::CaseInsensitive) == 0)
    {
        retVal = SecureDigital;
    }
    else if (type.compare(QLatin1String("PC-CARD"), Qt::CaseInsensitive) == 0)
    {
        retVal = PC_Card;
    }
    else if (type.compare(QLatin1String("MMC"), Qt::CaseInsensitive) == 0)
    {
        retVal = MultimediaCard;
    }
    else if (type.compare(QLatin1String("CF"), Qt::CaseInsensitive) == 0)
    {
        retVal = CompactFlash;
    }
    else if (type.compare(QLatin1String("BD"), Qt::CaseInsensitive) == 0)
    {
        retVal = BluRay;
    }
    else if (type.compare(QLatin1String("MS"), Qt::CaseInsensitive) == 0)
    {
        retVal = MemoryStick;
    }
    else if (type.compare(QLatin1String("HD_DVD"), Qt::CaseInsensitive) == 0)
    {
        retVal = HD_DVD;
    }
    else if (!type.isEmpty())
    {
        retVal = VendorDefined;
    }
    return retVal;
}

bool operator==(const HStorageMedium& obj1, const HStorageMedium& obj2)
{
    return obj1.toString() == obj2.toString();
}

quint32 qHash(const HStorageMedium& key)
{
    QByteArray data = key.toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
}
