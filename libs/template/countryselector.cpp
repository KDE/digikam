/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-07
 * Description : country selector combo-box.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QVariant>

// KDE includes

#include <klocale.h>

// Local includes

#include "debug.h"

namespace Digikam
{

class CountrySelectorPriv
{
public:

    CountrySelectorPriv()
    {
        // We cannot use KLocale::allCountriesList() here because KDE only
        // support 2 characters country codes. XMP require 3 characters country
        // following ISO 3166 (http://userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html)

        // Standard ISO 3166 country codes.

        countryCodeMap.insert( "AFG", i18n("Afghanistan") );
        countryCodeMap.insert( "ALB", i18n("Albania") );
        countryCodeMap.insert( "DZA", i18n("Algeria") );
        countryCodeMap.insert( "ASM", i18n("American Samoa") );
        countryCodeMap.insert( "AND", i18n("Andorra") );
        countryCodeMap.insert( "AGO", i18n("Angola") );
        countryCodeMap.insert( "AIA", i18n("Anguilla") );
        countryCodeMap.insert( "AGO", i18n("Angola") );
        countryCodeMap.insert( "ATA", i18n("Antarctica") );
        countryCodeMap.insert( "ATG", i18n("Antigua and Barbuda") );
        countryCodeMap.insert( "ARG", i18n("Argentina") );
        countryCodeMap.insert( "ARM", i18n("Armenia") );
        countryCodeMap.insert( "ABW", i18n("Aruba") );
        countryCodeMap.insert( "AUS", i18n("Australia") );
        countryCodeMap.insert( "AUT", i18n("Austria") );
        countryCodeMap.insert( "AZE", i18n("Azerbaijan") );
        countryCodeMap.insert( "BHS", i18n("Bahamas") );
        countryCodeMap.insert( "BHR", i18n("Bahrain") );
        countryCodeMap.insert( "BGD", i18n("Bangladesh") );
        countryCodeMap.insert( "BRB", i18n("Barbados") );
        countryCodeMap.insert( "BLR", i18n("Belarus") );
        countryCodeMap.insert( "BEL", i18n("Belgium") );
        countryCodeMap.insert( "BLZ", i18n("Belize") );
        countryCodeMap.insert( "BEN", i18n("Benin") );
        countryCodeMap.insert( "BMU", i18n("Bermuda") );
        countryCodeMap.insert( "BTN", i18n("Bhutan") );
        countryCodeMap.insert( "BOL", i18n("Bolivia") );
        countryCodeMap.insert( "BIH", i18n("Bosnia and Herzegovina") );
        countryCodeMap.insert( "BWA", i18n("Botswana") );
        countryCodeMap.insert( "BVT", i18n("Bouvet Island") );
        countryCodeMap.insert( "BRA", i18n("Brazil") );
        countryCodeMap.insert( "IOT", i18n("British Indian Ocean Territory") );
        countryCodeMap.insert( "VGB", i18n("British Virgin Islands") );
        countryCodeMap.insert( "BRN", i18n("Brunei Darussalam") );
        countryCodeMap.insert( "BGR", i18n("Bulgaria") );
        countryCodeMap.insert( "BFA", i18n("Burkina Faso") );
        countryCodeMap.insert( "BDI", i18n("Burundi") );
        countryCodeMap.insert( "KHM", i18n("Cambodia") );
        countryCodeMap.insert( "CMR", i18n("Cameroon") );
        countryCodeMap.insert( "CAN", i18n("Canada") );
        countryCodeMap.insert( "CPV", i18n("Cape Verde") );
        countryCodeMap.insert( "CYM", i18n("Cayman Islands") );
        countryCodeMap.insert( "CAF", i18n("Central African Republic") );
        countryCodeMap.insert( "TCD", i18n("Chad") );
        countryCodeMap.insert( "CHL", i18n("Chile") );
        countryCodeMap.insert( "CHN", i18n("China") );
        countryCodeMap.insert( "CXR", i18n("Christmas Island ") );
        countryCodeMap.insert( "CCK", i18n("Cocos Islands") );
        countryCodeMap.insert( "COL", i18n("Colombia") );
        countryCodeMap.insert( "COM", i18n("Comoros") );
        countryCodeMap.insert( "COD", i18n("Zaire") );
        countryCodeMap.insert( "COG", i18n("Congo") );
        countryCodeMap.insert( "COK", i18n("Cook Islands") );
        countryCodeMap.insert( "CRI", i18n("Costa Rica") );
        countryCodeMap.insert( "CIV", i18n("Ivory Coast") );
        countryCodeMap.insert( "CUB", i18n("Cuba") );
        countryCodeMap.insert( "CYP", i18n("Cyprus") );
        countryCodeMap.insert( "CZE", i18n("Czech Republic") );
        countryCodeMap.insert( "DNK", i18n("Denmark") );
        countryCodeMap.insert( "DJI", i18n("Djibouti") );
        countryCodeMap.insert( "DMA", i18n("Dominica") );
        countryCodeMap.insert( "DOM", i18n("Dominican Republic") );
        countryCodeMap.insert( "ECU", i18n("Ecuador") );
        countryCodeMap.insert( "EGY", i18n("Egypt") );
        countryCodeMap.insert( "SLV", i18n("El Salvador") );
        countryCodeMap.insert( "GNQ", i18n("Equatorial Guinea") );
        countryCodeMap.insert( "ERI", i18n("Eritrea") );
        countryCodeMap.insert( "EST", i18n("Estonia") );
        countryCodeMap.insert( "ETH", i18n("Ethiopia") );
        countryCodeMap.insert( "FRO", i18n("Faeroe Islands") );
        countryCodeMap.insert( "FLK", i18n("Falkland Islands") );
        countryCodeMap.insert( "FJI", i18n("Fiji Islands") );
        countryCodeMap.insert( "FIN", i18n("Finland") );
        countryCodeMap.insert( "FRA", i18n("France") );
        countryCodeMap.insert( "GUF", i18n("French Guiana") );
        countryCodeMap.insert( "PYF", i18n("French Polynesia") );
        countryCodeMap.insert( "ATF", i18n("French Southern Territories") );
        countryCodeMap.insert( "GAB", i18n("Gabon") );
        countryCodeMap.insert( "GMB", i18n("Gambia") );
        countryCodeMap.insert( "GEO", i18n("Georgia") );
        countryCodeMap.insert( "DEU", i18n("Germany") );
        countryCodeMap.insert( "GHA", i18n("Ghana") );
        countryCodeMap.insert( "GIB", i18n("Gibraltar") );
        countryCodeMap.insert( "GRC", i18n("Greece") );
        countryCodeMap.insert( "GRL", i18n("Greenland") );
        countryCodeMap.insert( "GRD", i18n("Grenada") );
        countryCodeMap.insert( "GLP", i18n("Guadaloupe") );
        countryCodeMap.insert( "GUM", i18n("Guam") );
        countryCodeMap.insert( "GTM", i18n("Guatemala") );
        countryCodeMap.insert( "GIN", i18n("Guinea") );
        countryCodeMap.insert( "GNB", i18n("Guinea-Bissau") );
        countryCodeMap.insert( "GUY", i18n("Guyana") );
        countryCodeMap.insert( "HTI", i18n("Haiti") );
        countryCodeMap.insert( "HMD", i18n("Heard and McDonald Islands") );
        countryCodeMap.insert( "VAT", i18n("Vatican") );
        countryCodeMap.insert( "HND", i18n("Honduras") );
        countryCodeMap.insert( "HKG", i18n("Hong Kong") );
        countryCodeMap.insert( "HRV", i18n("Croatia") );
        countryCodeMap.insert( "HUN", i18n("Hungary") );
        countryCodeMap.insert( "ISL", i18n("Iceland") );
        countryCodeMap.insert( "IND", i18n("India") );
        countryCodeMap.insert( "IDN", i18n("Indonesia") );
        countryCodeMap.insert( "IRN", i18n("Iran") );
        countryCodeMap.insert( "IRQ", i18n("Iraq") );
        countryCodeMap.insert( "IRL", i18n("Ireland") );
        countryCodeMap.insert( "ISR", i18n("Israel") );
        countryCodeMap.insert( "ITA", i18n("Italy") );
        countryCodeMap.insert( "JAM", i18n("Jamaica") );
        countryCodeMap.insert( "JPN", i18n("Japan") );
        countryCodeMap.insert( "JOR", i18n("Jordan") );
        countryCodeMap.insert( "KAZ", i18n("Kazakhstan") );
        countryCodeMap.insert( "KEN", i18n("Kenya") );
        countryCodeMap.insert( "KIR", i18n("Kiribati") );
        countryCodeMap.insert( "PRK", i18n("North-Korea") );
        countryCodeMap.insert( "KOR", i18n("South-Korea") );
        countryCodeMap.insert( "KWT", i18n("Kuwait") );
        countryCodeMap.insert( "KGZ", i18n("Kyrgyz Republic") );
        countryCodeMap.insert( "LAO", i18n("Lao") );
        countryCodeMap.insert( "LVA", i18n("Latvia") );
        countryCodeMap.insert( "LBN", i18n("Lebanon") );
        countryCodeMap.insert( "LSO", i18n("Lesotho") );
        countryCodeMap.insert( "LBR", i18n("Liberia") );
        countryCodeMap.insert( "LBY", i18n("Libyan Arab Jamahiriya") );
        countryCodeMap.insert( "LIE", i18n("Liechtenstein") );
        countryCodeMap.insert( "LTU", i18n("Lithuania") );
        countryCodeMap.insert( "LUX", i18n("Luxembourg") );
        countryCodeMap.insert( "MAC", i18n("Macao") );
        countryCodeMap.insert( "MKD", i18n("Macedonia") );
        countryCodeMap.insert( "MDG", i18n("Madagascar") );
        countryCodeMap.insert( "MWI", i18n("Malawi") );
        countryCodeMap.insert( "MYS", i18n("Malaysia") );
        countryCodeMap.insert( "MDV", i18n("Maldives") );
        countryCodeMap.insert( "MLI", i18n("Mali") );
        countryCodeMap.insert( "MLT", i18n("Malta") );
        countryCodeMap.insert( "MHL", i18n("Marshall Islands") );
        countryCodeMap.insert( "MTQ", i18n("Martinique") );
        countryCodeMap.insert( "MRT", i18n("Mauritania") );
        countryCodeMap.insert( "MUS", i18n("Mauritius") );
        countryCodeMap.insert( "MYT", i18n("Mayotte") );
        countryCodeMap.insert( "MEX", i18n("Mexico") );
        countryCodeMap.insert( "FSM", i18n("Micronesia") );
        countryCodeMap.insert( "MDA", i18n("Moldova") );
        countryCodeMap.insert( "MCO", i18n("Monaco") );
        countryCodeMap.insert( "MNG", i18n("Mongolia") );
        countryCodeMap.insert( "MSR", i18n("Montserrat") );
        countryCodeMap.insert( "MAR", i18n("Morocco") );
        countryCodeMap.insert( "MOZ", i18n("Mozambique") );
        countryCodeMap.insert( "MMR", i18n("Myanmar") );
        countryCodeMap.insert( "NAM", i18n("Namibia") );
        countryCodeMap.insert( "NRU", i18n("Nauru") );
        countryCodeMap.insert( "NPL", i18n("Nepal") );
        countryCodeMap.insert( "ANT", i18n("Netherlands Antilles") );
        countryCodeMap.insert( "NLD", i18n("Netherlands") );
        countryCodeMap.insert( "NCL", i18n("New Caledonia") );
        countryCodeMap.insert( "NZL", i18n("New Zealand") );
        countryCodeMap.insert( "NIC", i18n("Nicaragua") );
        countryCodeMap.insert( "NER", i18n("Niger") );
        countryCodeMap.insert( "NGA", i18n("Nigeria") );
        countryCodeMap.insert( "NIU", i18n("Niue") );
        countryCodeMap.insert( "NFK", i18n("Norfolk Island") );
        countryCodeMap.insert( "MNP", i18n("Northern Mariana Islands") );
        countryCodeMap.insert( "NOR", i18n("Norway") );
        countryCodeMap.insert( "OMN", i18n("Oman") );
        countryCodeMap.insert( "PAK", i18n("Pakistan") );
        countryCodeMap.insert( "PLW", i18n("Palau") );
        countryCodeMap.insert( "PSE", i18n("Palestinian Territory") );
        countryCodeMap.insert( "PAN", i18n("Panama") );
        countryCodeMap.insert( "PNG", i18n("Papua New Guinea") );
        countryCodeMap.insert( "PRY", i18n("Paraguay") );
        countryCodeMap.insert( "PER", i18n("Peru") );
        countryCodeMap.insert( "PHL", i18n("Philippines") );
        countryCodeMap.insert( "PCN", i18n("Pitcairn Island") );
        countryCodeMap.insert( "POL", i18n("Poland") );
        countryCodeMap.insert( "PRT", i18n("Portugal") );
        countryCodeMap.insert( "PRI", i18n("Puerto Rico") );
        countryCodeMap.insert( "QAT", i18n("Qatar") );
        countryCodeMap.insert( "REU", i18n("Reunion") );
        countryCodeMap.insert( "ROU", i18n("Romania") );
        countryCodeMap.insert( "RUS", i18n("Russian Federation") );
        countryCodeMap.insert( "RWA", i18n("Rwanda") );
        countryCodeMap.insert( "SHN", i18n("St. Helena") );
        countryCodeMap.insert( "KNA", i18n("St. Kitts and Nevis") );
        countryCodeMap.insert( "LCA", i18n("St. Lucia") );
        countryCodeMap.insert( "SPM", i18n("St. Pierre and Miquelon") );
        countryCodeMap.insert( "VCT", i18n("St. Vincent and the Grenadines") );
        countryCodeMap.insert( "WSM", i18n("Samoa") );
        countryCodeMap.insert( "SMR", i18n("San Marino") );
        countryCodeMap.insert( "STP", i18n("Sao Tome and Principe") );
        countryCodeMap.insert( "SAU", i18n("Saudi Arabia") );
        countryCodeMap.insert( "SEN", i18n("Senegal") );
        countryCodeMap.insert( "SCG", i18n("Serbia and Montenegro") );
        countryCodeMap.insert( "SYC", i18n("Seychelles") );
        countryCodeMap.insert( "SLE", i18n("Sierra Leone") );
        countryCodeMap.insert( "SGP", i18n("Singapore") );
        countryCodeMap.insert( "SVK", i18n("Slovakia") );
        countryCodeMap.insert( "SVN", i18n("Slovenia") );
        countryCodeMap.insert( "SLB", i18n("Solomon Islands") );
        countryCodeMap.insert( "SOM", i18n("Somalia") );
        countryCodeMap.insert( "ZAF", i18n("South Africa") );
        countryCodeMap.insert( "SGS", i18n("South Georgia and the South Sandwich Islands") );
        countryCodeMap.insert( "ESP", i18n("Spain") );
        countryCodeMap.insert( "LKA", i18n("Sri Lanka") );
        countryCodeMap.insert( "SDN", i18n("Sudan") );
        countryCodeMap.insert( "SUR", i18n("Suriname") );
        countryCodeMap.insert( "SJM", i18n("Svalbard & Jan Mayen Islands") );
        countryCodeMap.insert( "SWZ", i18n("Swaziland") );
        countryCodeMap.insert( "SWE", i18n("Sweden") );
        countryCodeMap.insert( "CHE", i18n("Switzerland") );
        countryCodeMap.insert( "SYR", i18n("Syrian Arab Republic") );
        countryCodeMap.insert( "TWN", i18n("Taiwan") );
        countryCodeMap.insert( "TJK", i18n("Tajikistan") );
        countryCodeMap.insert( "TZA", i18n("Tanzania") );
        countryCodeMap.insert( "THA", i18n("Thailand") );
        countryCodeMap.insert( "TLS", i18n("Timor-Leste") );
        countryCodeMap.insert( "TGO", i18n("Togo") );
        countryCodeMap.insert( "TKL", i18n("Tokelau Islands") );
        countryCodeMap.insert( "TON", i18n("Tonga") );
        countryCodeMap.insert( "TTO", i18n("Trinidad and Tobago") );
        countryCodeMap.insert( "TUN", i18n("Tunisia") );
        countryCodeMap.insert( "TUR", i18n("Turkey") );
        countryCodeMap.insert( "TKM", i18n("Turkmenistan") );
        countryCodeMap.insert( "TCA", i18n("Turks and Caicos Islands") );
        countryCodeMap.insert( "TUV", i18n("Tuvalu") );
        countryCodeMap.insert( "VIR", i18n("US Virgin Islands") );
        countryCodeMap.insert( "UGA", i18n("Uganda") );
        countryCodeMap.insert( "UKR", i18n("Ukraine") );
        countryCodeMap.insert( "ARE", i18n("United Arab Emirates") );
        countryCodeMap.insert( "GBR", i18n("United Kingdom") );
        countryCodeMap.insert( "UMI", i18n("United States Minor Outlying Islands") );
        countryCodeMap.insert( "USA", i18n("United States of America") );
        countryCodeMap.insert( "URY", i18n("Uruguay, Eastern Republic of") );
        countryCodeMap.insert( "UZB", i18n("Uzbekistan") );
        countryCodeMap.insert( "VUT", i18n("Vanuatu") );
        countryCodeMap.insert( "VEN", i18n("Venezuela") );
        countryCodeMap.insert( "VNM", i18n("Viet Nam") );
        countryCodeMap.insert( "WLF", i18n("Wallis and Futuna Islands ") );
        countryCodeMap.insert( "ESH", i18n("Western Sahara") );
        countryCodeMap.insert( "YEM", i18n("Yemen") );
        countryCodeMap.insert( "ZMB", i18n("Zambia") );
        countryCodeMap.insert( "ZWE", i18n("Zimbabwe") );

        // Supplemental IPTC/IIM country codes.

        countryCodeMap.insert( "XUN", i18n("United Nations") );
        countryCodeMap.insert( "XEU", i18n("European Union") );
        countryCodeMap.insert( "XSP", i18n("Space") );
        countryCodeMap.insert( "XSE", i18n("At Sea") );
        countryCodeMap.insert( "XIF", i18n("In Flight") );
        countryCodeMap.insert( "XEN", i18n("England") );
        countryCodeMap.insert( "XSC", i18n("Scotland") );
        countryCodeMap.insert( "XNI", i18n("Northern Ireland") );
        countryCodeMap.insert( "XWA", i18n("Wales") );
        countryCodeMap.insert( "PSE", i18n("Palestine") );
        countryCodeMap.insert( "GZA", i18n("Gaza") );
        countryCodeMap.insert( "JRO", i18n("Jericho") );
    }

    typedef QMap<QString, QString> CountryCodeMap;

    CountryCodeMap                 countryCodeMap;
};

CountrySelector::CountrySelector(QWidget* parent)
               : KComboBox(parent), d(new CountrySelectorPriv)
{
    for (CountrySelectorPriv::CountryCodeMap::Iterator it = d->countryCodeMap.begin();
         it != d->countryCodeMap.end(); ++it)
    {
        addItem(QString("%1 - %2").arg(it.key()).arg(it.value()));
    }

    model()->sort(0);

    insertSeparator(count());
    addItem(i18n("Unknown"));
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

    kDebug(digiKamAreaCode) << count() << " :: " << id;
}

bool CountrySelector::country(QString& countryCode, QString& countryName)
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
    CountrySelectorPriv priv;
    return (priv.countryCodeMap[countryCode]);
}

}  // namespace Digikam
