/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-07
 * Description : country selector combo-box.
 *
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

#include "countryselector.h"

// Qt includes

#include <QMap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN CountrySelector::Private
{
public:

    explicit Private()
    {
        // We cannot use KLocale::allCountriesList() here because KDE only
        // support 2 characters country codes. XMP require 3 characters country
        // following ISO 3166 (http://userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html)

        // Standard ISO 3166 country codes.

        countryCodeMap.insert( QLatin1String("AFG"), i18n("Afghanistan") );
        countryCodeMap.insert( QLatin1String("ALB"), i18n("Albania") );
        countryCodeMap.insert( QLatin1String("DZA"), i18n("Algeria") );
        countryCodeMap.insert( QLatin1String("ASM"), i18n("American Samoa") );
        countryCodeMap.insert( QLatin1String("AND"), i18n("Andorra") );
        countryCodeMap.insert( QLatin1String("AGO"), i18n("Angola") );
        countryCodeMap.insert( QLatin1String("AIA"), i18n("Anguilla") );
        countryCodeMap.insert( QLatin1String("AGO"), i18n("Angola") );
        countryCodeMap.insert( QLatin1String("ATA"), i18n("Antarctica") );
        countryCodeMap.insert( QLatin1String("ATG"), i18n("Antigua and Barbuda") );
        countryCodeMap.insert( QLatin1String("ARG"), i18n("Argentina") );
        countryCodeMap.insert( QLatin1String("ARM"), i18n("Armenia") );
        countryCodeMap.insert( QLatin1String("ABW"), i18n("Aruba") );
        countryCodeMap.insert( QLatin1String("AUS"), i18n("Australia") );
        countryCodeMap.insert( QLatin1String("AUT"), i18n("Austria") );
        countryCodeMap.insert( QLatin1String("AZE"), i18n("Azerbaijan") );
        countryCodeMap.insert( QLatin1String("BHS"), i18n("Bahamas") );
        countryCodeMap.insert( QLatin1String("BHR"), i18n("Bahrain") );
        countryCodeMap.insert( QLatin1String("BGD"), i18n("Bangladesh") );
        countryCodeMap.insert( QLatin1String("BRB"), i18n("Barbados") );
        countryCodeMap.insert( QLatin1String("BLR"), i18n("Belarus") );
        countryCodeMap.insert( QLatin1String("BEL"), i18n("Belgium") );
        countryCodeMap.insert( QLatin1String("BLZ"), i18n("Belize") );
        countryCodeMap.insert( QLatin1String("BEN"), i18n("Benin") );
        countryCodeMap.insert( QLatin1String("BMU"), i18n("Bermuda") );
        countryCodeMap.insert( QLatin1String("BTN"), i18n("Bhutan") );
        countryCodeMap.insert( QLatin1String("BOL"), i18n("Bolivia") );
        countryCodeMap.insert( QLatin1String("BIH"), i18n("Bosnia and Herzegovina") );
        countryCodeMap.insert( QLatin1String("BWA"), i18n("Botswana") );
        countryCodeMap.insert( QLatin1String("BVT"), i18n("Bouvet Island") );
        countryCodeMap.insert( QLatin1String("BRA"), i18n("Brazil") );
        countryCodeMap.insert( QLatin1String("IOT"), i18n("British Indian Ocean Territory") );
        countryCodeMap.insert( QLatin1String("VGB"), i18n("British Virgin Islands") );
        countryCodeMap.insert( QLatin1String("BRN"), i18n("Brunei Darussalam") );
        countryCodeMap.insert( QLatin1String("BGR"), i18n("Bulgaria") );
        countryCodeMap.insert( QLatin1String("BFA"), i18n("Burkina Faso") );
        countryCodeMap.insert( QLatin1String("BDI"), i18n("Burundi") );
        countryCodeMap.insert( QLatin1String("KHM"), i18n("Cambodia") );
        countryCodeMap.insert( QLatin1String("CMR"), i18n("Cameroon") );
        countryCodeMap.insert( QLatin1String("CAN"), i18n("Canada") );
        countryCodeMap.insert( QLatin1String("CPV"), i18n("Cape Verde") );
        countryCodeMap.insert( QLatin1String("CYM"), i18n("Cayman Islands") );
        countryCodeMap.insert( QLatin1String("CAF"), i18n("Central African Republic") );
        countryCodeMap.insert( QLatin1String("TCD"), i18n("Chad") );
        countryCodeMap.insert( QLatin1String("CHL"), i18n("Chile") );
        countryCodeMap.insert( QLatin1String("CHN"), i18n("China") );
        countryCodeMap.insert( QLatin1String("CXR"), i18n("Christmas Island ") );
        countryCodeMap.insert( QLatin1String("CCK"), i18n("Cocos Islands") );
        countryCodeMap.insert( QLatin1String("COL"), i18n("Colombia") );
        countryCodeMap.insert( QLatin1String("COM"), i18n("Comoros") );
        countryCodeMap.insert( QLatin1String("COD"), i18n("Zaire") );
        countryCodeMap.insert( QLatin1String("COG"), i18n("Congo") );
        countryCodeMap.insert( QLatin1String("COK"), i18n("Cook Islands") );
        countryCodeMap.insert( QLatin1String("CRI"), i18n("Costa Rica") );
        countryCodeMap.insert( QLatin1String("CIV"), i18n("Ivory Coast") );
        countryCodeMap.insert( QLatin1String("CUB"), i18n("Cuba") );
        countryCodeMap.insert( QLatin1String("CYP"), i18n("Cyprus") );
        countryCodeMap.insert( QLatin1String("CZE"), i18n("Czechia") );
        countryCodeMap.insert( QLatin1String("DNK"), i18n("Denmark") );
        countryCodeMap.insert( QLatin1String("DJI"), i18n("Djibouti") );
        countryCodeMap.insert( QLatin1String("DMA"), i18n("Dominica") );
        countryCodeMap.insert( QLatin1String("DOM"), i18n("Dominican Republic") );
        countryCodeMap.insert( QLatin1String("ECU"), i18n("Ecuador") );
        countryCodeMap.insert( QLatin1String("EGY"), i18n("Egypt") );
        countryCodeMap.insert( QLatin1String("SLV"), i18n("El Salvador") );
        countryCodeMap.insert( QLatin1String("GNQ"), i18n("Equatorial Guinea") );
        countryCodeMap.insert( QLatin1String("ERI"), i18n("Eritrea") );
        countryCodeMap.insert( QLatin1String("EST"), i18n("Estonia") );
        countryCodeMap.insert( QLatin1String("ETH"), i18n("Ethiopia") );
        countryCodeMap.insert( QLatin1String("FRO"), i18n("Faeroe Islands") );
        countryCodeMap.insert( QLatin1String("FLK"), i18n("Falkland Islands") );
        countryCodeMap.insert( QLatin1String("FJI"), i18n("Fiji Islands") );
        countryCodeMap.insert( QLatin1String("FIN"), i18n("Finland") );
        countryCodeMap.insert( QLatin1String("FRA"), i18n("France") );
        countryCodeMap.insert( QLatin1String("GUF"), i18n("French Guiana") );
        countryCodeMap.insert( QLatin1String("PYF"), i18n("French Polynesia") );
        countryCodeMap.insert( QLatin1String("ATF"), i18n("French Southern Territories") );
        countryCodeMap.insert( QLatin1String("GAB"), i18n("Gabon") );
        countryCodeMap.insert( QLatin1String("GMB"), i18n("Gambia") );
        countryCodeMap.insert( QLatin1String("GEO"), i18n("Georgia") );
        countryCodeMap.insert( QLatin1String("DEU"), i18n("Germany") );
        countryCodeMap.insert( QLatin1String("GHA"), i18n("Ghana") );
        countryCodeMap.insert( QLatin1String("GIB"), i18n("Gibraltar") );
        countryCodeMap.insert( QLatin1String("GRC"), i18n("Greece") );
        countryCodeMap.insert( QLatin1String("GRL"), i18n("Greenland") );
        countryCodeMap.insert( QLatin1String("GRD"), i18n("Grenada") );
        countryCodeMap.insert( QLatin1String("GLP"), i18n("Guadaloupe") );
        countryCodeMap.insert( QLatin1String("GUM"), i18n("Guam") );
        countryCodeMap.insert( QLatin1String("GTM"), i18n("Guatemala") );
        countryCodeMap.insert( QLatin1String("GIN"), i18n("Guinea") );
        countryCodeMap.insert( QLatin1String("GNB"), i18n("Guinea-Bissau") );
        countryCodeMap.insert( QLatin1String("GUY"), i18n("Guyana") );
        countryCodeMap.insert( QLatin1String("HTI"), i18n("Haiti") );
        countryCodeMap.insert( QLatin1String("HMD"), i18n("Heard and McDonald Islands") );
        countryCodeMap.insert( QLatin1String("VAT"), i18n("Vatican") );
        countryCodeMap.insert( QLatin1String("HND"), i18n("Honduras") );
        countryCodeMap.insert( QLatin1String("HKG"), i18n("Hong Kong") );
        countryCodeMap.insert( QLatin1String("HRV"), i18n("Croatia") );
        countryCodeMap.insert( QLatin1String("HUN"), i18n("Hungary") );
        countryCodeMap.insert( QLatin1String("ISL"), i18n("Iceland") );
        countryCodeMap.insert( QLatin1String("IND"), i18n("India") );
        countryCodeMap.insert( QLatin1String("IDN"), i18n("Indonesia") );
        countryCodeMap.insert( QLatin1String("IRN"), i18n("Iran") );
        countryCodeMap.insert( QLatin1String("IRQ"), i18n("Iraq") );
        countryCodeMap.insert( QLatin1String("IRL"), i18n("Ireland") );
        countryCodeMap.insert( QLatin1String("ISR"), i18n("Israel") );
        countryCodeMap.insert( QLatin1String("ITA"), i18n("Italy") );
        countryCodeMap.insert( QLatin1String("JAM"), i18n("Jamaica") );
        countryCodeMap.insert( QLatin1String("JPN"), i18n("Japan") );
        countryCodeMap.insert( QLatin1String("JOR"), i18n("Jordan") );
        countryCodeMap.insert( QLatin1String("KAZ"), i18n("Kazakhstan") );
        countryCodeMap.insert( QLatin1String("KEN"), i18n("Kenya") );
        countryCodeMap.insert( QLatin1String("KIR"), i18n("Kiribati") );
        countryCodeMap.insert( QLatin1String("PRK"), i18n("North-Korea") );
        countryCodeMap.insert( QLatin1String("KOR"), i18n("South-Korea") );
        countryCodeMap.insert( QLatin1String("KWT"), i18n("Kuwait") );
        countryCodeMap.insert( QLatin1String("KGZ"), i18n("Kyrgyz Republic") );
        countryCodeMap.insert( QLatin1String("LAO"), i18n("Lao") );
        countryCodeMap.insert( QLatin1String("LVA"), i18n("Latvia") );
        countryCodeMap.insert( QLatin1String("LBN"), i18n("Lebanon") );
        countryCodeMap.insert( QLatin1String("LSO"), i18n("Lesotho") );
        countryCodeMap.insert( QLatin1String("LBR"), i18n("Liberia") );
        countryCodeMap.insert( QLatin1String("LBY"), i18n("Libyan Arab Jamahiriya") );
        countryCodeMap.insert( QLatin1String("LIE"), i18n("Liechtenstein") );
        countryCodeMap.insert( QLatin1String("LTU"), i18n("Lithuania") );
        countryCodeMap.insert( QLatin1String("LUX"), i18n("Luxembourg") );
        countryCodeMap.insert( QLatin1String("MAC"), i18n("Macao") );
        countryCodeMap.insert( QLatin1String("MKD"), i18n("Macedonia") );
        countryCodeMap.insert( QLatin1String("MDG"), i18n("Madagascar") );
        countryCodeMap.insert( QLatin1String("MWI"), i18n("Malawi") );
        countryCodeMap.insert( QLatin1String("MYS"), i18n("Malaysia") );
        countryCodeMap.insert( QLatin1String("MDV"), i18n("Maldives") );
        countryCodeMap.insert( QLatin1String("MLI"), i18n("Mali") );
        countryCodeMap.insert( QLatin1String("MLT"), i18n("Malta") );
        countryCodeMap.insert( QLatin1String("MHL"), i18n("Marshall Islands") );
        countryCodeMap.insert( QLatin1String("MTQ"), i18n("Martinique") );
        countryCodeMap.insert( QLatin1String("MRT"), i18n("Mauritania") );
        countryCodeMap.insert( QLatin1String("MUS"), i18n("Mauritius") );
        countryCodeMap.insert( QLatin1String("MYT"), i18n("Mayotte") );
        countryCodeMap.insert( QLatin1String("MEX"), i18n("Mexico") );
        countryCodeMap.insert( QLatin1String("FSM"), i18n("Micronesia") );
        countryCodeMap.insert( QLatin1String("MDA"), i18n("Moldova") );
        countryCodeMap.insert( QLatin1String("MCO"), i18n("Monaco") );
        countryCodeMap.insert( QLatin1String("MNG"), i18n("Mongolia") );
        countryCodeMap.insert( QLatin1String("MSR"), i18n("Montserrat") );
        countryCodeMap.insert( QLatin1String("MAR"), i18n("Morocco") );
        countryCodeMap.insert( QLatin1String("MOZ"), i18n("Mozambique") );
        countryCodeMap.insert( QLatin1String("MMR"), i18n("Myanmar") );
        countryCodeMap.insert( QLatin1String("NAM"), i18n("Namibia") );
        countryCodeMap.insert( QLatin1String("NRU"), i18n("Nauru") );
        countryCodeMap.insert( QLatin1String("NPL"), i18n("Nepal") );
        countryCodeMap.insert( QLatin1String("ANT"), i18n("Netherlands Antilles") );
        countryCodeMap.insert( QLatin1String("NLD"), i18n("Netherlands") );
        countryCodeMap.insert( QLatin1String("NCL"), i18n("New Caledonia") );
        countryCodeMap.insert( QLatin1String("NZL"), i18n("New Zealand") );
        countryCodeMap.insert( QLatin1String("NIC"), i18n("Nicaragua") );
        countryCodeMap.insert( QLatin1String("NER"), i18n("Niger") );
        countryCodeMap.insert( QLatin1String("NGA"), i18n("Nigeria") );
        countryCodeMap.insert( QLatin1String("NIU"), i18n("Niue") );
        countryCodeMap.insert( QLatin1String("NFK"), i18n("Norfolk Island") );
        countryCodeMap.insert( QLatin1String("MNP"), i18n("Northern Mariana Islands") );
        countryCodeMap.insert( QLatin1String("NOR"), i18n("Norway") );
        countryCodeMap.insert( QLatin1String("OMN"), i18n("Oman") );
        countryCodeMap.insert( QLatin1String("PAK"), i18n("Pakistan") );
        countryCodeMap.insert( QLatin1String("PLW"), i18n("Palau") );
        countryCodeMap.insert( QLatin1String("PSE"), i18n("Palestinian Territory") );
        countryCodeMap.insert( QLatin1String("PAN"), i18n("Panama") );
        countryCodeMap.insert( QLatin1String("PNG"), i18n("Papua New Guinea") );
        countryCodeMap.insert( QLatin1String("PRY"), i18n("Paraguay") );
        countryCodeMap.insert( QLatin1String("PER"), i18n("Peru") );
        countryCodeMap.insert( QLatin1String("PHL"), i18n("Philippines") );
        countryCodeMap.insert( QLatin1String("PCN"), i18n("Pitcairn Island") );
        countryCodeMap.insert( QLatin1String("POL"), i18n("Poland") );
        countryCodeMap.insert( QLatin1String("PRT"), i18n("Portugal") );
        countryCodeMap.insert( QLatin1String("PRI"), i18n("Puerto Rico") );
        countryCodeMap.insert( QLatin1String("QAT"), i18n("Qatar") );
        countryCodeMap.insert( QLatin1String("REU"), i18n("Reunion") );
        countryCodeMap.insert( QLatin1String("ROU"), i18n("Romania") );
        countryCodeMap.insert( QLatin1String("RUS"), i18n("Russian Federation") );
        countryCodeMap.insert( QLatin1String("RWA"), i18n("Rwanda") );
        countryCodeMap.insert( QLatin1String("SHN"), i18n("St. Helena") );
        countryCodeMap.insert( QLatin1String("KNA"), i18n("St. Kitts and Nevis") );
        countryCodeMap.insert( QLatin1String("LCA"), i18n("St. Lucia") );
        countryCodeMap.insert( QLatin1String("SPM"), i18n("St. Pierre and Miquelon") );
        countryCodeMap.insert( QLatin1String("VCT"), i18n("St. Vincent and the Grenadines") );
        countryCodeMap.insert( QLatin1String("WSM"), i18n("Samoa") );
        countryCodeMap.insert( QLatin1String("SMR"), i18n("San Marino") );
        countryCodeMap.insert( QLatin1String("STP"), i18n("Sao Tome and Principe") );
        countryCodeMap.insert( QLatin1String("SAU"), i18n("Saudi Arabia") );
        countryCodeMap.insert( QLatin1String("SEN"), i18n("Senegal") );
        countryCodeMap.insert( QLatin1String("SCG"), i18n("Serbia") );
        countryCodeMap.insert( QLatin1String("SYC"), i18n("Seychelles") );
        countryCodeMap.insert( QLatin1String("SLE"), i18n("Sierra Leone") );
        countryCodeMap.insert( QLatin1String("SGP"), i18n("Singapore") );
        countryCodeMap.insert( QLatin1String("SVK"), i18n("Slovakia") );
        countryCodeMap.insert( QLatin1String("SVN"), i18n("Slovenia") );
        countryCodeMap.insert( QLatin1String("SLB"), i18n("Solomon Islands") );
        countryCodeMap.insert( QLatin1String("SOM"), i18n("Somalia") );
        countryCodeMap.insert( QLatin1String("ZAF"), i18n("South Africa") );
        countryCodeMap.insert( QLatin1String("SGS"), i18n("South Georgia and the South Sandwich Islands") );
        countryCodeMap.insert( QLatin1String("ESP"), i18n("Spain") );
        countryCodeMap.insert( QLatin1String("LKA"), i18n("Sri Lanka") );
        countryCodeMap.insert( QLatin1String("SDN"), i18n("Sudan") );
        countryCodeMap.insert( QLatin1String("SUR"), i18n("Suriname") );
        countryCodeMap.insert( QLatin1String("SJM"), i18n("Svalbard & Jan Mayen Islands") );
        countryCodeMap.insert( QLatin1String("SWZ"), i18n("Swaziland") );
        countryCodeMap.insert( QLatin1String("SWE"), i18n("Sweden") );
        countryCodeMap.insert( QLatin1String("CHE"), i18n("Switzerland") );
        countryCodeMap.insert( QLatin1String("SYR"), i18n("Syrian Arab Republic") );
        countryCodeMap.insert( QLatin1String("TWN"), i18n("Taiwan") );
        countryCodeMap.insert( QLatin1String("TJK"), i18n("Tajikistan") );
        countryCodeMap.insert( QLatin1String("TZA"), i18n("Tanzania") );
        countryCodeMap.insert( QLatin1String("THA"), i18n("Thailand") );
        countryCodeMap.insert( QLatin1String("TLS"), i18n("Timor-Leste") );
        countryCodeMap.insert( QLatin1String("TGO"), i18n("Togo") );
        countryCodeMap.insert( QLatin1String("TKL"), i18n("Tokelau Islands") );
        countryCodeMap.insert( QLatin1String("TON"), i18n("Tonga") );
        countryCodeMap.insert( QLatin1String("TTO"), i18n("Trinidad and Tobago") );
        countryCodeMap.insert( QLatin1String("TUN"), i18n("Tunisia") );
        countryCodeMap.insert( QLatin1String("TUR"), i18n("Turkey") );
        countryCodeMap.insert( QLatin1String("TKM"), i18n("Turkmenistan") );
        countryCodeMap.insert( QLatin1String("TCA"), i18n("Turks and Caicos Islands") );
        countryCodeMap.insert( QLatin1String("TUV"), i18n("Tuvalu") );
        countryCodeMap.insert( QLatin1String("VIR"), i18n("US Virgin Islands") );
        countryCodeMap.insert( QLatin1String("UGA"), i18n("Uganda") );
        countryCodeMap.insert( QLatin1String("UKR"), i18n("Ukraine") );
        countryCodeMap.insert( QLatin1String("ARE"), i18n("United Arab Emirates") );
        countryCodeMap.insert( QLatin1String("GBR"), i18n("United Kingdom") );
        countryCodeMap.insert( QLatin1String("UMI"), i18n("United States Minor Outlying Islands") );
        countryCodeMap.insert( QLatin1String("USA"), i18n("United States of America") );
        countryCodeMap.insert( QLatin1String("URY"), i18n("Uruguay, Eastern Republic of") );
        countryCodeMap.insert( QLatin1String("UZB"), i18n("Uzbekistan") );
        countryCodeMap.insert( QLatin1String("VUT"), i18n("Vanuatu") );
        countryCodeMap.insert( QLatin1String("VEN"), i18n("Venezuela") );
        countryCodeMap.insert( QLatin1String("VNM"), i18n("Viet Nam") );
        countryCodeMap.insert( QLatin1String("WLF"), i18n("Wallis and Futuna Islands ") );
        countryCodeMap.insert( QLatin1String("ESH"), i18n("Western Sahara") );
        countryCodeMap.insert( QLatin1String("YEM"), i18n("Yemen") );
        countryCodeMap.insert( QLatin1String("ZMB"), i18n("Zambia") );
        countryCodeMap.insert( QLatin1String("ZWE"), i18n("Zimbabwe") );

        // Supplemental IPTC/IIM country codes.

        countryCodeMap.insert( QLatin1String("XUN"), i18n("United Nations") );
        countryCodeMap.insert( QLatin1String("XEU"), i18n("European Union") );
        countryCodeMap.insert( QLatin1String("XSP"), i18n("Space") );
        countryCodeMap.insert( QLatin1String("XSE"), i18n("At Sea") );
        countryCodeMap.insert( QLatin1String("XIF"), i18n("In Flight") );
        countryCodeMap.insert( QLatin1String("XEN"), i18n("England") );
        countryCodeMap.insert( QLatin1String("XSC"), i18n("Scotland") );
        countryCodeMap.insert( QLatin1String("XNI"), i18n("Northern Ireland") );
        countryCodeMap.insert( QLatin1String("XWA"), i18n("Wales") );
        countryCodeMap.insert( QLatin1String("PSE"), i18n("Palestine") );
        countryCodeMap.insert( QLatin1String("GZA"), i18n("Gaza") );
        countryCodeMap.insert( QLatin1String("JRO"), i18n("Jericho") );
    }

    typedef QMap<QString, QString> CountryCodeMap;

    CountryCodeMap                 countryCodeMap;
};

CountrySelector::CountrySelector(QWidget* const parent)
    : QComboBox(parent),
      d(new Private)
{
    for (Private::CountryCodeMap::Iterator it = d->countryCodeMap.begin();
         it != d->countryCodeMap.end(); ++it)
    {
        addItem(QString::fromLatin1("%1 - %2").arg(it.key()).arg(it.value()));
    }

    model()->sort(0);

    insertSeparator(count());
    addItem(i18nc("Unknown country", "Unknown"));
}

CountrySelector::~CountrySelector()
{
    delete d;
}

void CountrySelector::setCountry(const QString& countryCode)
{
    // NOTE: if countryCode is empty or do not matches code map, unknow is selected from the list.

    int id = count()-1;

    for (int i = 0 ; i < d->countryCodeMap.count() ; i++)
    {
        if (itemText(i).left(3) == countryCode)
        {
            id = i;
            break;
        }
    }

    setCurrentIndex(id);

    qCDebug(DIGIKAM_WIDGETS_LOG) << count() << " :: " << id;
}

bool CountrySelector::country(QString& countryCode, QString& countryName) const
{
    // Unknow is selected ?
    if (currentIndex() == count()-1)
        return false;

    countryName = currentText().mid(6);
    countryCode = currentText().left(3);
    return true;
}

QString CountrySelector::countryForCode(const QString& countryCode)
{
    Private priv;
    return (priv.countryCodeMap[countryCode]);
}

} // namespace Digikam
