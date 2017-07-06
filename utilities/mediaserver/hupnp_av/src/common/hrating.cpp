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

#include "hrating.h"

#include <QtCore/QMetaType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::Av::HRating>("Herqq::Upnp::Av::HRating");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace Av
{

namespace
{
HRating::MpaaValues convertMpaa(const QString& arg)
{
    HRating::MpaaValues retVal = HRating::MPAA_Undefined;
    if (arg == QLatin1String("G"))
    {
        retVal = HRating::MPAA_GeneralAudiences;
    }
    else if (arg == QLatin1String("PG"))
    {
        retVal = HRating::MPAA_ParentalGuidanceSuggested;
    }
    else if (arg == QLatin1String("PG-13"))
    {
        retVal = HRating::MPAA_ParentsStronglyCautioned;
    }
    else if (arg == QLatin1String("R"))
    {
        retVal = HRating::MPAA_Restricted;
    }
    else if (arg == QLatin1String("NC-17"))
    {
        retVal = HRating::MPAA_NoOneSeventeenAndUnderAdmitted;
    }
    else if (arg == QLatin1String("NR"))
    {
        retVal = HRating::MPAA_NotRatedYet;
    }
    return retVal;
}
HRating::RiaaValues convertRiaa(const QString& arg)
{
    HRating::RiaaValues retVal = HRating::RIAA_Undefined;
    if (arg == QLatin1String("PA-EC"))
    {
        retVal = HRating::RIAA_ExplicitContent;
    }
    else if (arg.isEmpty())
    {
        retVal = HRating::RIAA_NonExplicitContent;
    }
    return retVal;
}
HRating::EsrbValues convertEsrb(const QString& arg)
{
    HRating::EsrbValues retVal = HRating::ESRB_Undefined;
    if (arg == QLatin1String("EC"))
    {
        retVal = HRating::ESRB_EarlyChildhood;
    }
    else if (arg == QLatin1String("E"))
    {
        retVal = HRating::ESRB_Everyone;
    }
    else if (arg == QLatin1String("E10+"))
    {
        retVal = HRating::ESRB_EveryoneTenAndOlder;
    }
    else if (arg == QLatin1String("T"))
    {
        retVal = HRating::ESRB_Teen;
    }
    else if (arg == QLatin1String("M"))
    {
        retVal = HRating::ESRB_Mature;
    }
    else if (arg == QLatin1String("AO"))
    {
        retVal = HRating::ESRB_AdultsOnly;
    }
    else if (arg == QLatin1String("RP"))
    {
        retVal = HRating::ESRB_RatingPending;
    }
    return retVal;
}
HRating::TvGuidelinesValues convertTvg(const QString& arg)
{
    HRating::TvGuidelinesValues retVal = HRating::TVG_Undefined;
    if (arg == QLatin1String("TV-Y"))
    {
        retVal = HRating::TVG_AllChildren;
    }
    else if (arg == QLatin1String("TV-Y7"))
    {
        retVal = HRating::TVG_OlderChildren;
    }
    else if (arg == QLatin1String("TV-Y7FV"))
    {
        retVal = HRating::TVG_OlderChildren_FantasyViolence;
    }
    else if (arg == QLatin1String("TV-G"))
    {
        retVal = HRating::TVG_GeneralAudience;
    }
    else if (arg == QLatin1String("TV-14"))
    {
        retVal = HRating::TVG_ParentsStronglyCautioned;
    }
    else if (arg == QLatin1String("TV-MA"))
    {
        retVal = HRating::TVG_MatureAudienceOnly;
    }
    return retVal;
}
}

class HRatingPrivate :
    public QSharedData
{
H_DISABLE_ASSIGN(HRatingPrivate)
public:

    HRating::Type m_type;
    QString m_typeAsString;

    union
    {
        HRating::MpaaValues m_mpaaValue;
        HRating::RiaaValues m_riaaValue;
        HRating::EsrbValues m_esrbValue;
        HRating::TvGuidelinesValues m_tvgValue;
    };

    QString m_value;

    HRatingPrivate() :
        m_type(HRating::UndefinedType), m_typeAsString(),
        m_mpaaValue(HRating::MPAA_Undefined), m_value()
    {
    }

    bool setMpaa(const QString& value);
    bool setEsrb(const QString& value);
    bool setRiaa(const QString& value);
    bool setTvg(const QString& value);
};

bool HRatingPrivate::setMpaa(const QString& arg)
{
    HRating::MpaaValues value = convertMpaa(arg);
    if (value != HRating::MPAA_Undefined)
    {
        m_type = HRating::MPAA;
        m_typeAsString = HRating::toString(HRating::MPAA);
        m_value = arg;
        m_mpaaValue = value;
    }
    return value != HRating::MPAA_Undefined;
}

bool HRatingPrivate::setRiaa(const QString& arg)
{
    HRating::RiaaValues value = convertRiaa(arg);
    if (value != HRating::RIAA_Undefined)
    {
        m_type = HRating::RIAA;
        m_typeAsString = HRating::toString(HRating::RIAA);
        m_value = arg;
        m_riaaValue = value;
    }
    return value != HRating::RIAA_Undefined;
}

bool HRatingPrivate::setEsrb(const QString& arg)
{
    HRating::EsrbValues value = convertEsrb(arg);
    if (value != HRating::ESRB_Undefined)
    {
        m_type = HRating::ESRB;
        m_typeAsString = HRating::toString(HRating::ESRB);
        m_value = arg;
        m_esrbValue = value;
    }
    return value != HRating::ESRB_Undefined;
}

bool HRatingPrivate::setTvg(const QString& arg)
{
    HRating::TvGuidelinesValues value = convertTvg(arg);
    if (value != HRating::TVG_Undefined)
    {
        m_type = HRating::TvGuidelines;
        m_typeAsString = HRating::toString(HRating::TvGuidelines);
        m_value = arg;
        m_tvgValue = value;
    }
    return value != HRating::TVG_Undefined;
}

QString HRating::toString(HRating::Type type)
{
    QString retVal;
    switch(type)
    {
    case MPAA:
        retVal = QLatin1String("MPAA.ORG");
        break;
    case RIAA:
        retVal = QLatin1String("RIAA.ORG");
        break;
    case ESRB:
        retVal = QLatin1String("ESRB.ORG");
        break;
    case TvGuidelines:
        retVal = QLatin1String("TVGUIDELINES.ORG");
        break;
    default:
        break;
    }
    return retVal;
}

QString HRating::toString(HRating::MpaaValues type)
{
    QString retVal;
    switch(type)
    {
    case MPAA_GeneralAudiences:
        retVal = QLatin1String("G");
        break;
    case MPAA_ParentalGuidanceSuggested:
        retVal = QLatin1String("PG");
        break;
    case MPAA_ParentsStronglyCautioned:
        retVal = QLatin1String("PG-13");
        break;
    case MPAA_Restricted:
        retVal = QLatin1String("R");
        break;
    case MPAA_NoOneSeventeenAndUnderAdmitted:
        retVal = QLatin1String("NC-17");
        break;
    case MPAA_NotRatedYet:
        retVal = QLatin1String("NR");
        break;
    default:
        break;
    }
    return retVal;
}

QString HRating::toString(HRating::RiaaValues type)
{
    QString retVal;
    switch(type)
    {
    case RIAA_NonExplicitContent:
        break;
    case RIAA_ExplicitContent:
        retVal = QLatin1String("PA-EC");
        break;
    default:
        break;
    }
    return retVal;
}

QString HRating::toString(HRating::EsrbValues type)
{
    QString retVal;
    switch(type)
    {
    case ESRB_EarlyChildhood:
        retVal = QLatin1String("EC");
        break;
    case ESRB_Everyone:
        retVal = QLatin1String("E");
        break;
    case ESRB_EveryoneTenAndOlder:
        retVal = QLatin1String("E10+");
        break;
    case ESRB_Teen:
        retVal = QLatin1String("T");
        break;
    case ESRB_Mature:
        retVal = QLatin1String("M");
        break;
    case ESRB_AdultsOnly:
        retVal = QLatin1String("AO");
        break;
    case ESRB_RatingPending:
        retVal = QLatin1String("RB");
        break;
    default:
        break;
    }
    return retVal;
}

QString HRating::toString(HRating::TvGuidelinesValues type)
{
    QString retVal;
    switch(type)
    {
    case TVG_AllChildren:
        retVal = QLatin1String("TV-Y");
        break;
    case TVG_OlderChildren:
        retVal = QLatin1String("TV-Y7");
        break;
    case TVG_OlderChildren_FantasyViolence:
        retVal = QLatin1String("TV-Y7FV");
        break;
    case TVG_GeneralAudience:
        retVal = QLatin1String("TV-G");
        break;
    case TVG_ParentalGuidanceSuggested:
        retVal = QLatin1String("TV-PG");
        break;
    case TVG_ParentsStronglyCautioned:
        retVal = QLatin1String("TV-14");
        break;
    case TVG_MatureAudienceOnly:
        retVal = QLatin1String("TV-MA");
        break;
    default:
        break;
    }
    return retVal;
}

HRating::HRating() :
    h_ptr(new HRatingPrivate())
{
}

HRating::HRating(const QString& value, const QString& type) :
    h_ptr(new HRatingPrivate())
{
    QString valueTrimmed = value.trimmed();
    QString typeTrimmed = type.trimmed();

    if (typeTrimmed.isEmpty())
    {
        if (!h_ptr->setMpaa(valueTrimmed))
        {
            if (!h_ptr->setEsrb(valueTrimmed))
            {
                if (!h_ptr->setTvg(valueTrimmed))
                {
                    //h_ptr->setRiaa(valueTrimmed);
                    // cannot do this, since an empty string is ambiguous in this context:
                    // 1) it could mean an error, or valid RIAA type / value.
                    //
                    // Otherwise:
                    // The value is invalid, as vendor-defined
                    // rating has to have type defined.
                }
            }
        }
    }
    else if (typeTrimmed == toString(MPAA))
    {
        h_ptr->setMpaa(valueTrimmed);
    }
    else if (typeTrimmed == toString(ESRB))
    {
        h_ptr->setEsrb(valueTrimmed);
    }
    else if (typeTrimmed == toString(TvGuidelines))
    {
        h_ptr->setTvg(valueTrimmed);
    }
    else if (typeTrimmed == toString(RIAA))
    {
        h_ptr->setRiaa(valueTrimmed);
    }
    else
    {
        h_ptr->m_type = HRating::VendorDefined;
        h_ptr->m_typeAsString = typeTrimmed;
        h_ptr->m_value = valueTrimmed;
    }
}

HRating::HRating(MpaaValues arg) :
    h_ptr(new HRatingPrivate())
{
    h_ptr->m_type = MPAA;
    h_ptr->m_typeAsString = toString(MPAA);
    h_ptr->m_value = toString(arg);
    h_ptr->m_mpaaValue = arg;
}

HRating::HRating(RiaaValues arg) :
    h_ptr(new HRatingPrivate())
{
    h_ptr->m_type = RIAA;
    h_ptr->m_typeAsString = toString(RIAA);
    h_ptr->m_value = toString(arg);
    h_ptr->m_riaaValue = arg;
}

HRating::HRating(EsrbValues arg) :
    h_ptr(new HRatingPrivate())
{
    h_ptr->m_type = ESRB;
    h_ptr->m_typeAsString = toString(ESRB);
    h_ptr->m_value = toString(arg);
    h_ptr->m_esrbValue = arg;
}

HRating::HRating(TvGuidelinesValues arg) :
    h_ptr(new HRatingPrivate())
{
    h_ptr->m_type = TvGuidelines;
    h_ptr->m_typeAsString = toString(TvGuidelines);
    h_ptr->m_value = toString(arg);
    h_ptr->m_tvgValue = arg;
}

HRating::HRating(const HRating& other) :
    h_ptr(other.h_ptr)
{
    Q_ASSERT(&other != this);
}

HRating& HRating::operator =(const HRating& other)
{
    Q_ASSERT(&other != this);
    h_ptr = other.h_ptr;
    return *this;
}

HRating::~HRating()
{
}

HRating::Type HRating::type() const
{
    return h_ptr->m_type;
}

QString HRating::typeAsString() const
{
    return h_ptr->m_typeAsString;
}

HRating::MpaaValues HRating::mpaaValue() const
{
    return h_ptr->m_type == MPAA ? h_ptr->m_mpaaValue : MPAA_Undefined;
}

HRating::EsrbValues HRating::esrbValue() const
{
    return h_ptr->m_type == ESRB ? h_ptr->m_esrbValue : ESRB_Undefined;
}

HRating::RiaaValues HRating::riaaValue() const
{
    return h_ptr->m_type == RIAA ? h_ptr->m_riaaValue : RIAA_Undefined;
}

HRating::TvGuidelinesValues HRating::tvGuidelinesValue() const
{
    return h_ptr->m_type == TvGuidelines ? h_ptr->m_tvgValue : TVG_Undefined;
}

QString HRating::value() const
{
    return h_ptr->m_value;
}

bool HRating::isValid() const
{
    return !value().isEmpty() && !typeAsString().isEmpty();
}

bool operator==(const HRating& obj1, const HRating& obj2)
{
    return obj1.value() == obj2.value() &&
           obj1.typeAsString() == obj1.typeAsString();
}

}
}
}

