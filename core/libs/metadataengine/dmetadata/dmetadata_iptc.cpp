/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - Iptc helpers.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

IptcCoreContactInfo DMetadata::getCreatorContactInfo() const
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreContactInfoCity
           << MetadataInfo::IptcCoreContactInfoCountry
           << MetadataInfo::IptcCoreContactInfoAddress
           << MetadataInfo::IptcCoreContactInfoPostalCode
           << MetadataInfo::IptcCoreContactInfoProvinceState
           << MetadataInfo::IptcCoreContactInfoEmail
           << MetadataInfo::IptcCoreContactInfoPhone
           << MetadataInfo::IptcCoreContactInfoWebUrl;

    QVariantList metadataInfos = getMetadataFields(fields);

    IptcCoreContactInfo info;

    if (metadataInfos.size() == 8)
    {
        info.city          = metadataInfos.at(0).toString();
        info.country       = metadataInfos.at(1).toString();
        info.address       = metadataInfos.at(2).toString();
        info.postalCode    = metadataInfos.at(3).toString();
        info.provinceState = metadataInfos.at(4).toString();
        info.email         = metadataInfos.at(5).toString();
        info.phone         = metadataInfos.at(6).toString();
        info.webUrl        = metadataInfos.at(7).toString();
    }

    return info;
}

bool DMetadata::setCreatorContactInfo(const IptcCoreContactInfo& info) const
{
    if (!supportXmp())
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", info.city))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", info.country))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr", info.address))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode", info.postalCode))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion", info.provinceState))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork", info.email))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork", info.phone))
    {
        return false;
    }

    if (!setXmpTagString("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork", info.webUrl))
    {
        return false;
    }

    return true;
}

IptcCoreLocationInfo DMetadata::getIptcCoreLocation() const
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreCountry
           << MetadataInfo::IptcCoreCountryCode
           << MetadataInfo::IptcCoreCity
           << MetadataInfo::IptcCoreLocation
           << MetadataInfo::IptcCoreProvinceState;

    QVariantList metadataInfos = getMetadataFields(fields);

    IptcCoreLocationInfo location;

    if (fields.size() == 5)
    {
        location.country       = metadataInfos.at(0).toString();
        location.countryCode   = metadataInfos.at(1).toString();
        location.city          = metadataInfos.at(2).toString();
        location.location      = metadataInfos.at(3).toString();
        location.provinceState = metadataInfos.at(4).toString();
    }

    return location;
}

bool DMetadata::setIptcCoreLocation(const IptcCoreLocationInfo& location) const
{
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.photoshop.Country", location.country))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.iptc.CountryCode", location.countryCode))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.City", location.city))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.iptc.Location", location.location))
        {
            return false;
        }

        if (!setXmpTagString("Xmp.photoshop.State", location.provinceState))
        {
            return false;
        }
    }

    if (!setIptcTag(location.country,       64,  "Country",        "Iptc.Application2.CountryName"))
    {
        return false;
    }

    if (!setIptcTag(location.countryCode,    3,  "Country Code",   "Iptc.Application2.CountryCode"))
    {
        return false;
    }

    if (!setIptcTag(location.city,          32,  "City",           "Iptc.Application2.City"))
    {
        return false;
    }

    if (!setIptcTag(location.location,      32,  "SubLocation",    "Iptc.Application2.SubLocation"))
    {
        return false;
    }

    if (!setIptcTag(location.provinceState, 32,  "Province/State", "Iptc.Application2.ProvinceState"))
    {
        return false;
    }

    return true;
}

QStringList DMetadata::getIptcCoreSubjects() const
{
    QStringList list = getXmpSubjects();

    if (!list.isEmpty())
    {
        return list;
    }

    return getIptcSubjects();
}

bool DMetadata::setIptcTag(const QString& text,
                           int maxLength,
                           const char* const debugLabel,
                           const char* const tagKey)  const
{
    QString truncatedText = text;
    truncatedText.truncate(maxLength);
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> " << debugLabel << ": " << truncatedText;

    return setIptcTagString(tagKey, truncatedText);    // returns false if failed
}

QVariant DMetadata::fromIptcEmulateList(const char* const iptcTagName) const
{
    return toStringListVariant(getIptcTagsStringList(iptcTagName));
}

QVariant DMetadata::fromIptcEmulateLangAlt(const char* const iptcTagName) const
{
    QString str = getIptcTagString(iptcTagName);

    if (str.isNull())
    {
        return QVariant(QVariant::Map);
    }

    QMap<QString, QVariant> map;
    map[QLatin1String("x-default")] = str;

    return map;
}

bool DMetadata::removeIptcTags(const QStringList& tagFilters)
{
    MetaDataMap m = getIptcTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeIptcTag(it.key().toLatin1().constData());
    }

    return true;
}

DMetadata::CountryCodeMap DMetadata::countryCodeMap()
{
    CountryCodeMap countryCodeMap;
    // All ISO 639 language code based on 2 characters
    // http://xml.coverpages.org/iso639a.html

    countryCodeMap.insert( QLatin1String("AA"), i18n("Afar"));
    countryCodeMap.insert( QLatin1String("AB"), i18n("Abkhazian"));
    countryCodeMap.insert( QLatin1String("AF"), i18n("Afrikaans"));
    countryCodeMap.insert( QLatin1String("AM"), i18n("Amharic"));
    countryCodeMap.insert( QLatin1String("AR"), i18n("Arabic"));
    countryCodeMap.insert( QLatin1String("AS"), i18n("Assamese"));
    countryCodeMap.insert( QLatin1String("AY"), i18n("Aymara"));
    countryCodeMap.insert( QLatin1String("AZ"), i18n("Azerbaijani"));
    countryCodeMap.insert( QLatin1String("BA"), i18n("Bashkir"));
    countryCodeMap.insert( QLatin1String("BE"), i18n("Byelorussian"));
    countryCodeMap.insert( QLatin1String("BG"), i18n("Bulgarian"));
    countryCodeMap.insert( QLatin1String("BH"), i18n("Bihari"));
    countryCodeMap.insert( QLatin1String("BI"), i18n("Bislama"));
    countryCodeMap.insert( QLatin1String("BN"), i18n("Bengali;Bangla") );
    countryCodeMap.insert( QLatin1String("BO"), i18n("Tibetan"));
    countryCodeMap.insert( QLatin1String("BR"), i18n("Breton"));
    countryCodeMap.insert( QLatin1String("CA"), i18n("Catalan"));
    countryCodeMap.insert( QLatin1String("CO"), i18n("Corsican"));
    countryCodeMap.insert( QLatin1String("CS"), i18n("Czech"));
    countryCodeMap.insert( QLatin1String("CY"), i18n("Welsh"));
    countryCodeMap.insert( QLatin1String("DA"), i18n("Danish"));
    countryCodeMap.insert( QLatin1String("DE"), i18n("German"));
    countryCodeMap.insert( QLatin1String("DZ"), i18n("Bhutani"));
    countryCodeMap.insert( QLatin1String("EL"), i18n("Greek"));
    countryCodeMap.insert( QLatin1String("EN"), i18n("English"));
    countryCodeMap.insert( QLatin1String("EO"), i18n("Esperanto"));
    countryCodeMap.insert( QLatin1String("ES"), i18n("Spanish"));
    countryCodeMap.insert( QLatin1String("ET"), i18n("Estonian"));
    countryCodeMap.insert( QLatin1String("EU"), i18n("Basque"));
    countryCodeMap.insert( QLatin1String("FA"), i18n("Persian(farsi)") );
    countryCodeMap.insert( QLatin1String("FI"), i18n("Finnish"));
    countryCodeMap.insert( QLatin1String("FJ"), i18n("Fiji"));
    countryCodeMap.insert( QLatin1String("FO"), i18n("Faroese") );
    countryCodeMap.insert( QLatin1String("FR"), i18n("French") );
    countryCodeMap.insert( QLatin1String("FY"), i18n("Frisian") );
    countryCodeMap.insert( QLatin1String("GA"), i18n("Irish") );
    countryCodeMap.insert( QLatin1String("GD"), i18n("Scotsgaelic") );
    countryCodeMap.insert( QLatin1String("GL"), i18n("Galician") );
    countryCodeMap.insert( QLatin1String("GN"), i18n("Guarani") );
    countryCodeMap.insert( QLatin1String("GU"), i18n("Gujarati") );
    countryCodeMap.insert( QLatin1String("HA"), i18n("Hausa") );
    countryCodeMap.insert( QLatin1String("HE"), i18n("Hebrew") );
    countryCodeMap.insert( QLatin1String("HI"), i18n("Hindi") );
    countryCodeMap.insert( QLatin1String("HR"), i18n("Croatian") );
    countryCodeMap.insert( QLatin1String("HU"), i18n("Hungarian") );
    countryCodeMap.insert( QLatin1String("HY"), i18n("Armenian") );
    countryCodeMap.insert( QLatin1String("IA"), i18n("Interlingua") );
    countryCodeMap.insert( QLatin1String("IE"), i18n("Interlingue") );
    countryCodeMap.insert( QLatin1String("IK"), i18n("Inupiak") );
    countryCodeMap.insert( QLatin1String("ID"), i18n("Indonesian") );
    countryCodeMap.insert( QLatin1String("IS"), i18n("Icelandic") );
    countryCodeMap.insert( QLatin1String("IT"), i18n("Italian") );
    countryCodeMap.insert( QLatin1String("IU"), i18n("Inuktitut") );
    countryCodeMap.insert( QLatin1String("JA"), i18n("Japanese") );
    countryCodeMap.insert( QLatin1String("JV"), i18n("Javanese") );
    countryCodeMap.insert( QLatin1String("KA"), i18n("Georgian") );
    countryCodeMap.insert( QLatin1String("KK"), i18n("Kazakh") );
    countryCodeMap.insert( QLatin1String("KL"), i18n("Greenlandic") );
    countryCodeMap.insert( QLatin1String("KM"), i18n("Cambodian") );
    countryCodeMap.insert( QLatin1String("KN"), i18n("Kannada") );
    countryCodeMap.insert( QLatin1String("KO"), i18n("Korean") );
    countryCodeMap.insert( QLatin1String("KS"), i18n("Kashmiri") );
    countryCodeMap.insert( QLatin1String("KU"), i18n("Kurdish") );
    countryCodeMap.insert( QLatin1String("KY"), i18n("Kirghiz") );
    countryCodeMap.insert( QLatin1String("LA"), i18n("Latin") );
    countryCodeMap.insert( QLatin1String("LN"), i18n("Lingala") );
    countryCodeMap.insert( QLatin1String("LO"), i18n("Laothian") );
    countryCodeMap.insert( QLatin1String("LT"), i18n("Lithuanian") );
    countryCodeMap.insert( QLatin1String("LV"), i18n("Latvian;Lettish") );
    countryCodeMap.insert( QLatin1String("MG"), i18n("Malagasy") );
    countryCodeMap.insert( QLatin1String("MI"), i18n("Maori") );
    countryCodeMap.insert( QLatin1String("MK"), i18n("Macedonian") );
    countryCodeMap.insert( QLatin1String("ML"), i18n("Malayalam") );
    countryCodeMap.insert( QLatin1String("MN"), i18n("Mongolian") );
    countryCodeMap.insert( QLatin1String("MO"), i18n("Moldavian") );
    countryCodeMap.insert( QLatin1String("MR"), i18n("Marathi") );
    countryCodeMap.insert( QLatin1String("MS"), i18n("Malay") );
    countryCodeMap.insert( QLatin1String("MT"), i18n("Maltese") );
    countryCodeMap.insert( QLatin1String("MY"), i18n("Burmese") );
    countryCodeMap.insert( QLatin1String("NA"), i18n("Nauru") );
    countryCodeMap.insert( QLatin1String("NE"), i18n("Nepali") );
    countryCodeMap.insert( QLatin1String("NL"), i18n("Dutch") );
    countryCodeMap.insert( QLatin1String("NO"), i18n("Norwegian") );
    countryCodeMap.insert( QLatin1String("OC"), i18n("Occitan") );
    countryCodeMap.insert( QLatin1String("OM"), i18n("Afan(oromo)") );
    countryCodeMap.insert( QLatin1String("OR"), i18n("Oriya") );
    countryCodeMap.insert( QLatin1String("PA"), i18n("Punjabi") );
    countryCodeMap.insert( QLatin1String("PL"), i18n("Polish") );
    countryCodeMap.insert( QLatin1String("PS"), i18n("Pashto;Pushto") );
    countryCodeMap.insert( QLatin1String("PT"), i18n("Portuguese") );
    countryCodeMap.insert( QLatin1String("QU"), i18n("Quechua") );
    countryCodeMap.insert( QLatin1String("RM"), i18n("Rhaeto-romance") );
    countryCodeMap.insert( QLatin1String("RN"), i18n("Kurundi") );
    countryCodeMap.insert( QLatin1String("RO"), i18n("Romanian") );
    countryCodeMap.insert( QLatin1String("RU"), i18n("Russian") );
    countryCodeMap.insert( QLatin1String("RW"), i18n("Kinyarwanda") );
    countryCodeMap.insert( QLatin1String("SA"), i18n("Sanskrit") );
    countryCodeMap.insert( QLatin1String("SD"), i18n("Sindhi") );
    countryCodeMap.insert( QLatin1String("SG"), i18n("Sangho") );
    countryCodeMap.insert( QLatin1String("SH"), i18n("Serbo-croatian") );
    countryCodeMap.insert( QLatin1String("SI"), i18n("Singhalese") );
    countryCodeMap.insert( QLatin1String("SK"), i18n("Slovak") );
    countryCodeMap.insert( QLatin1String("SL"), i18n("Slovenian") );
    countryCodeMap.insert( QLatin1String("SM"), i18n("Samoan") );
    countryCodeMap.insert( QLatin1String("SN"), i18n("Shona") );
    countryCodeMap.insert( QLatin1String("SO"), i18n("Somali") );
    countryCodeMap.insert( QLatin1String("SQ"), i18n("Albanian") );
    countryCodeMap.insert( QLatin1String("SR"), i18n("Serbian") );
    countryCodeMap.insert( QLatin1String("SS"), i18n("Siswati") );
    countryCodeMap.insert( QLatin1String("ST"), i18n("Sesotho") );
    countryCodeMap.insert( QLatin1String("SU"), i18n("Sundanese") );
    countryCodeMap.insert( QLatin1String("SV"), i18n("Swedish") );
    countryCodeMap.insert( QLatin1String("SW"), i18n("Swahili") );
    countryCodeMap.insert( QLatin1String("TA"), i18n("Tamil") );
    countryCodeMap.insert( QLatin1String("TE"), i18n("Telugu") );
    countryCodeMap.insert( QLatin1String("TG"), i18n("Tajik") );
    countryCodeMap.insert( QLatin1String("TH"), i18n("Thai") );
    countryCodeMap.insert( QLatin1String("TI"), i18n("Tigrinya") );
    countryCodeMap.insert( QLatin1String("TK"), i18n("Turkmen") );
    countryCodeMap.insert( QLatin1String("TL"), i18n("Tagalog") );
    countryCodeMap.insert( QLatin1String("TN"), i18n("Setswana") );
    countryCodeMap.insert( QLatin1String("TO"), i18n("Tonga") );
    countryCodeMap.insert( QLatin1String("TR"), i18n("Turkish") );
    countryCodeMap.insert( QLatin1String("TS"), i18n("Tsonga") );
    countryCodeMap.insert( QLatin1String("TT"), i18n("Tatar") );
    countryCodeMap.insert( QLatin1String("TW"), i18n("Twi") );
    countryCodeMap.insert( QLatin1String("UG"), i18n("Uigur") );
    countryCodeMap.insert( QLatin1String("UK"), i18n("Ukrainian") );
    countryCodeMap.insert( QLatin1String("UR"), i18n("Urdu") );
    countryCodeMap.insert( QLatin1String("UZ"), i18n("Uzbek") );
    countryCodeMap.insert( QLatin1String("VI"), i18n("Vietnamese") );
    countryCodeMap.insert( QLatin1String("VO"), i18n("Volapuk") );
    countryCodeMap.insert( QLatin1String("WO"), i18n("Wolof") );
    countryCodeMap.insert( QLatin1String("XH"), i18n("Xhosa") );
    countryCodeMap.insert( QLatin1String("YI"), i18n("Yiddish") );
    countryCodeMap.insert( QLatin1String("YO"), i18n("Yoruba") );
    countryCodeMap.insert( QLatin1String("ZA"), i18n("Zhuang") );
    countryCodeMap.insert( QLatin1String("ZH"), i18n("Chinese") );
    countryCodeMap.insert( QLatin1String("ZU"), i18n("Zulu") );

    return countryCodeMap;
}

} // namespace Digikam
