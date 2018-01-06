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

    Private()
    {
        // We cannot use KLocale::allCountriesList() here because KDE only
        // support 2 characters country codes. XMP require 3 characters country
        // following ISO 3166 (http://userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html)

        // Standard ISO 3166 country codes.

        countryCodeMap.insert( QString::fromLatin1("AFG"), i18n("Afghanistan") );
        countryCodeMap.insert( QString::fromLatin1("ALB"), i18n("Albania") );
        countryCodeMap.insert( QString::fromLatin1("DZA"), i18n("Algeria") );
        countryCodeMap.insert( QString::fromLatin1("ASM"), i18n("American Samoa") );
        countryCodeMap.insert( QString::fromLatin1("AND"), i18n("Andorra") );
        countryCodeMap.insert( QString::fromLatin1("AGO"), i18n("Angola") );
        countryCodeMap.insert( QString::fromLatin1("AIA"), i18n("Anguilla") );
        countryCodeMap.insert( QString::fromLatin1("AGO"), i18n("Angola") );
        countryCodeMap.insert( QString::fromLatin1("ATA"), i18n("Antarctica") );
        countryCodeMap.insert( QString::fromLatin1("ATG"), i18n("Antigua and Barbuda") );
        countryCodeMap.insert( QString::fromLatin1("ARG"), i18n("Argentina") );
        countryCodeMap.insert( QString::fromLatin1("ARM"), i18n("Armenia") );
        countryCodeMap.insert( QString::fromLatin1("ABW"), i18n("Aruba") );
        countryCodeMap.insert( QString::fromLatin1("AUS"), i18n("Australia") );
        countryCodeMap.insert( QString::fromLatin1("AUT"), i18n("Austria") );
        countryCodeMap.insert( QString::fromLatin1("AZE"), i18n("Azerbaijan") );
        countryCodeMap.insert( QString::fromLatin1("BHS"), i18n("Bahamas") );
        countryCodeMap.insert( QString::fromLatin1("BHR"), i18n("Bahrain") );
        countryCodeMap.insert( QString::fromLatin1("BGD"), i18n("Bangladesh") );
        countryCodeMap.insert( QString::fromLatin1("BRB"), i18n("Barbados") );
        countryCodeMap.insert( QString::fromLatin1("BLR"), i18n("Belarus") );
        countryCodeMap.insert( QString::fromLatin1("BEL"), i18n("Belgium") );
        countryCodeMap.insert( QString::fromLatin1("BLZ"), i18n("Belize") );
        countryCodeMap.insert( QString::fromLatin1("BEN"), i18n("Benin") );
        countryCodeMap.insert( QString::fromLatin1("BMU"), i18n("Bermuda") );
        countryCodeMap.insert( QString::fromLatin1("BTN"), i18n("Bhutan") );
        countryCodeMap.insert( QString::fromLatin1("BOL"), i18n("Bolivia") );
        countryCodeMap.insert( QString::fromLatin1("BIH"), i18n("Bosnia and Herzegovina") );
        countryCodeMap.insert( QString::fromLatin1("BWA"), i18n("Botswana") );
        countryCodeMap.insert( QString::fromLatin1("BVT"), i18n("Bouvet Island") );
        countryCodeMap.insert( QString::fromLatin1("BRA"), i18n("Brazil") );
        countryCodeMap.insert( QString::fromLatin1("IOT"), i18n("British Indian Ocean Territory") );
        countryCodeMap.insert( QString::fromLatin1("VGB"), i18n("British Virgin Islands") );
        countryCodeMap.insert( QString::fromLatin1("BRN"), i18n("Brunei Darussalam") );
        countryCodeMap.insert( QString::fromLatin1("BGR"), i18n("Bulgaria") );
        countryCodeMap.insert( QString::fromLatin1("BFA"), i18n("Burkina Faso") );
        countryCodeMap.insert( QString::fromLatin1("BDI"), i18n("Burundi") );
        countryCodeMap.insert( QString::fromLatin1("KHM"), i18n("Cambodia") );
        countryCodeMap.insert( QString::fromLatin1("CMR"), i18n("Cameroon") );
        countryCodeMap.insert( QString::fromLatin1("CAN"), i18n("Canada") );
        countryCodeMap.insert( QString::fromLatin1("CPV"), i18n("Cape Verde") );
        countryCodeMap.insert( QString::fromLatin1("CYM"), i18n("Cayman Islands") );
        countryCodeMap.insert( QString::fromLatin1("CAF"), i18n("Central African Republic") );
        countryCodeMap.insert( QString::fromLatin1("TCD"), i18n("Chad") );
        countryCodeMap.insert( QString::fromLatin1("CHL"), i18n("Chile") );
        countryCodeMap.insert( QString::fromLatin1("CHN"), i18n("China") );
        countryCodeMap.insert( QString::fromLatin1("CXR"), i18n("Christmas Island ") );
        countryCodeMap.insert( QString::fromLatin1("CCK"), i18n("Cocos Islands") );
        countryCodeMap.insert( QString::fromLatin1("COL"), i18n("Colombia") );
        countryCodeMap.insert( QString::fromLatin1("COM"), i18n("Comoros") );
        countryCodeMap.insert( QString::fromLatin1("COD"), i18n("Zaire") );
        countryCodeMap.insert( QString::fromLatin1("COG"), i18n("Congo") );
        countryCodeMap.insert( QString::fromLatin1("COK"), i18n("Cook Islands") );
        countryCodeMap.insert( QString::fromLatin1("CRI"), i18n("Costa Rica") );
        countryCodeMap.insert( QString::fromLatin1("CIV"), i18n("Ivory Coast") );
        countryCodeMap.insert( QString::fromLatin1("CUB"), i18n("Cuba") );
        countryCodeMap.insert( QString::fromLatin1("CYP"), i18n("Cyprus") );
        countryCodeMap.insert( QString::fromLatin1("CZE"), i18n("Czechia") );
        countryCodeMap.insert( QString::fromLatin1("DNK"), i18n("Denmark") );
        countryCodeMap.insert( QString::fromLatin1("DJI"), i18n("Djibouti") );
        countryCodeMap.insert( QString::fromLatin1("DMA"), i18n("Dominica") );
        countryCodeMap.insert( QString::fromLatin1("DOM"), i18n("Dominican Republic") );
        countryCodeMap.insert( QString::fromLatin1("ECU"), i18n("Ecuador") );
        countryCodeMap.insert( QString::fromLatin1("EGY"), i18n("Egypt") );
        countryCodeMap.insert( QString::fromLatin1("SLV"), i18n("El Salvador") );
        countryCodeMap.insert( QString::fromLatin1("GNQ"), i18n("Equatorial Guinea") );
        countryCodeMap.insert( QString::fromLatin1("ERI"), i18n("Eritrea") );
        countryCodeMap.insert( QString::fromLatin1("EST"), i18n("Estonia") );
        countryCodeMap.insert( QString::fromLatin1("ETH"), i18n("Ethiopia") );
        countryCodeMap.insert( QString::fromLatin1("FRO"), i18n("Faeroe Islands") );
        countryCodeMap.insert( QString::fromLatin1("FLK"), i18n("Falkland Islands") );
        countryCodeMap.insert( QString::fromLatin1("FJI"), i18n("Fiji Islands") );
        countryCodeMap.insert( QString::fromLatin1("FIN"), i18n("Finland") );
        countryCodeMap.insert( QString::fromLatin1("FRA"), i18n("France") );
        countryCodeMap.insert( QString::fromLatin1("GUF"), i18n("French Guiana") );
        countryCodeMap.insert( QString::fromLatin1("PYF"), i18n("French Polynesia") );
        countryCodeMap.insert( QString::fromLatin1("ATF"), i18n("French Southern Territories") );
        countryCodeMap.insert( QString::fromLatin1("GAB"), i18n("Gabon") );
        countryCodeMap.insert( QString::fromLatin1("GMB"), i18n("Gambia") );
        countryCodeMap.insert( QString::fromLatin1("GEO"), i18n("Georgia") );
        countryCodeMap.insert( QString::fromLatin1("DEU"), i18n("Germany") );
        countryCodeMap.insert( QString::fromLatin1("GHA"), i18n("Ghana") );
        countryCodeMap.insert( QString::fromLatin1("GIB"), i18n("Gibraltar") );
        countryCodeMap.insert( QString::fromLatin1("GRC"), i18n("Greece") );
        countryCodeMap.insert( QString::fromLatin1("GRL"), i18n("Greenland") );
        countryCodeMap.insert( QString::fromLatin1("GRD"), i18n("Grenada") );
        countryCodeMap.insert( QString::fromLatin1("GLP"), i18n("Guadaloupe") );
        countryCodeMap.insert( QString::fromLatin1("GUM"), i18n("Guam") );
        countryCodeMap.insert( QString::fromLatin1("GTM"), i18n("Guatemala") );
        countryCodeMap.insert( QString::fromLatin1("GIN"), i18n("Guinea") );
        countryCodeMap.insert( QString::fromLatin1("GNB"), i18n("Guinea-Bissau") );
        countryCodeMap.insert( QString::fromLatin1("GUY"), i18n("Guyana") );
        countryCodeMap.insert( QString::fromLatin1("HTI"), i18n("Haiti") );
        countryCodeMap.insert( QString::fromLatin1("HMD"), i18n("Heard and McDonald Islands") );
        countryCodeMap.insert( QString::fromLatin1("VAT"), i18n("Vatican") );
        countryCodeMap.insert( QString::fromLatin1("HND"), i18n("Honduras") );
        countryCodeMap.insert( QString::fromLatin1("HKG"), i18n("Hong Kong") );
        countryCodeMap.insert( QString::fromLatin1("HRV"), i18n("Croatia") );
        countryCodeMap.insert( QString::fromLatin1("HUN"), i18n("Hungary") );
        countryCodeMap.insert( QString::fromLatin1("ISL"), i18n("Iceland") );
        countryCodeMap.insert( QString::fromLatin1("IND"), i18n("India") );
        countryCodeMap.insert( QString::fromLatin1("IDN"), i18n("Indonesia") );
        countryCodeMap.insert( QString::fromLatin1("IRN"), i18n("Iran") );
        countryCodeMap.insert( QString::fromLatin1("IRQ"), i18n("Iraq") );
        countryCodeMap.insert( QString::fromLatin1("IRL"), i18n("Ireland") );
        countryCodeMap.insert( QString::fromLatin1("ISR"), i18n("Israel") );
        countryCodeMap.insert( QString::fromLatin1("ITA"), i18n("Italy") );
        countryCodeMap.insert( QString::fromLatin1("JAM"), i18n("Jamaica") );
        countryCodeMap.insert( QString::fromLatin1("JPN"), i18n("Japan") );
        countryCodeMap.insert( QString::fromLatin1("JOR"), i18n("Jordan") );
        countryCodeMap.insert( QString::fromLatin1("KAZ"), i18n("Kazakhstan") );
        countryCodeMap.insert( QString::fromLatin1("KEN"), i18n("Kenya") );
        countryCodeMap.insert( QString::fromLatin1("KIR"), i18n("Kiribati") );
        countryCodeMap.insert( QString::fromLatin1("PRK"), i18n("North-Korea") );
        countryCodeMap.insert( QString::fromLatin1("KOR"), i18n("South-Korea") );
        countryCodeMap.insert( QString::fromLatin1("KWT"), i18n("Kuwait") );
        countryCodeMap.insert( QString::fromLatin1("KGZ"), i18n("Kyrgyz Republic") );
        countryCodeMap.insert( QString::fromLatin1("LAO"), i18n("Lao") );
        countryCodeMap.insert( QString::fromLatin1("LVA"), i18n("Latvia") );
        countryCodeMap.insert( QString::fromLatin1("LBN"), i18n("Lebanon") );
        countryCodeMap.insert( QString::fromLatin1("LSO"), i18n("Lesotho") );
        countryCodeMap.insert( QString::fromLatin1("LBR"), i18n("Liberia") );
        countryCodeMap.insert( QString::fromLatin1("LBY"), i18n("Libyan Arab Jamahiriya") );
        countryCodeMap.insert( QString::fromLatin1("LIE"), i18n("Liechtenstein") );
        countryCodeMap.insert( QString::fromLatin1("LTU"), i18n("Lithuania") );
        countryCodeMap.insert( QString::fromLatin1("LUX"), i18n("Luxembourg") );
        countryCodeMap.insert( QString::fromLatin1("MAC"), i18n("Macao") );
        countryCodeMap.insert( QString::fromLatin1("MKD"), i18n("Macedonia") );
        countryCodeMap.insert( QString::fromLatin1("MDG"), i18n("Madagascar") );
        countryCodeMap.insert( QString::fromLatin1("MWI"), i18n("Malawi") );
        countryCodeMap.insert( QString::fromLatin1("MYS"), i18n("Malaysia") );
        countryCodeMap.insert( QString::fromLatin1("MDV"), i18n("Maldives") );
        countryCodeMap.insert( QString::fromLatin1("MLI"), i18n("Mali") );
        countryCodeMap.insert( QString::fromLatin1("MLT"), i18n("Malta") );
        countryCodeMap.insert( QString::fromLatin1("MHL"), i18n("Marshall Islands") );
        countryCodeMap.insert( QString::fromLatin1("MTQ"), i18n("Martinique") );
        countryCodeMap.insert( QString::fromLatin1("MRT"), i18n("Mauritania") );
        countryCodeMap.insert( QString::fromLatin1("MUS"), i18n("Mauritius") );
        countryCodeMap.insert( QString::fromLatin1("MYT"), i18n("Mayotte") );
        countryCodeMap.insert( QString::fromLatin1("MEX"), i18n("Mexico") );
        countryCodeMap.insert( QString::fromLatin1("FSM"), i18n("Micronesia") );
        countryCodeMap.insert( QString::fromLatin1("MDA"), i18n("Moldova") );
        countryCodeMap.insert( QString::fromLatin1("MCO"), i18n("Monaco") );
        countryCodeMap.insert( QString::fromLatin1("MNG"), i18n("Mongolia") );
        countryCodeMap.insert( QString::fromLatin1("MSR"), i18n("Montserrat") );
        countryCodeMap.insert( QString::fromLatin1("MAR"), i18n("Morocco") );
        countryCodeMap.insert( QString::fromLatin1("MOZ"), i18n("Mozambique") );
        countryCodeMap.insert( QString::fromLatin1("MMR"), i18n("Myanmar") );
        countryCodeMap.insert( QString::fromLatin1("NAM"), i18n("Namibia") );
        countryCodeMap.insert( QString::fromLatin1("NRU"), i18n("Nauru") );
        countryCodeMap.insert( QString::fromLatin1("NPL"), i18n("Nepal") );
        countryCodeMap.insert( QString::fromLatin1("ANT"), i18n("Netherlands Antilles") );
        countryCodeMap.insert( QString::fromLatin1("NLD"), i18n("Netherlands") );
        countryCodeMap.insert( QString::fromLatin1("NCL"), i18n("New Caledonia") );
        countryCodeMap.insert( QString::fromLatin1("NZL"), i18n("New Zealand") );
        countryCodeMap.insert( QString::fromLatin1("NIC"), i18n("Nicaragua") );
        countryCodeMap.insert( QString::fromLatin1("NER"), i18n("Niger") );
        countryCodeMap.insert( QString::fromLatin1("NGA"), i18n("Nigeria") );
        countryCodeMap.insert( QString::fromLatin1("NIU"), i18n("Niue") );
        countryCodeMap.insert( QString::fromLatin1("NFK"), i18n("Norfolk Island") );
        countryCodeMap.insert( QString::fromLatin1("MNP"), i18n("Northern Mariana Islands") );
        countryCodeMap.insert( QString::fromLatin1("NOR"), i18n("Norway") );
        countryCodeMap.insert( QString::fromLatin1("OMN"), i18n("Oman") );
        countryCodeMap.insert( QString::fromLatin1("PAK"), i18n("Pakistan") );
        countryCodeMap.insert( QString::fromLatin1("PLW"), i18n("Palau") );
        countryCodeMap.insert( QString::fromLatin1("PSE"), i18n("Palestinian Territory") );
        countryCodeMap.insert( QString::fromLatin1("PAN"), i18n("Panama") );
        countryCodeMap.insert( QString::fromLatin1("PNG"), i18n("Papua New Guinea") );
        countryCodeMap.insert( QString::fromLatin1("PRY"), i18n("Paraguay") );
        countryCodeMap.insert( QString::fromLatin1("PER"), i18n("Peru") );
        countryCodeMap.insert( QString::fromLatin1("PHL"), i18n("Philippines") );
        countryCodeMap.insert( QString::fromLatin1("PCN"), i18n("Pitcairn Island") );
        countryCodeMap.insert( QString::fromLatin1("POL"), i18n("Poland") );
        countryCodeMap.insert( QString::fromLatin1("PRT"), i18n("Portugal") );
        countryCodeMap.insert( QString::fromLatin1("PRI"), i18n("Puerto Rico") );
        countryCodeMap.insert( QString::fromLatin1("QAT"), i18n("Qatar") );
        countryCodeMap.insert( QString::fromLatin1("REU"), i18n("Reunion") );
        countryCodeMap.insert( QString::fromLatin1("ROU"), i18n("Romania") );
        countryCodeMap.insert( QString::fromLatin1("RUS"), i18n("Russian Federation") );
        countryCodeMap.insert( QString::fromLatin1("RWA"), i18n("Rwanda") );
        countryCodeMap.insert( QString::fromLatin1("SHN"), i18n("St. Helena") );
        countryCodeMap.insert( QString::fromLatin1("KNA"), i18n("St. Kitts and Nevis") );
        countryCodeMap.insert( QString::fromLatin1("LCA"), i18n("St. Lucia") );
        countryCodeMap.insert( QString::fromLatin1("SPM"), i18n("St. Pierre and Miquelon") );
        countryCodeMap.insert( QString::fromLatin1("VCT"), i18n("St. Vincent and the Grenadines") );
        countryCodeMap.insert( QString::fromLatin1("WSM"), i18n("Samoa") );
        countryCodeMap.insert( QString::fromLatin1("SMR"), i18n("San Marino") );
        countryCodeMap.insert( QString::fromLatin1("STP"), i18n("Sao Tome and Principe") );
        countryCodeMap.insert( QString::fromLatin1("SAU"), i18n("Saudi Arabia") );
        countryCodeMap.insert( QString::fromLatin1("SEN"), i18n("Senegal") );
        countryCodeMap.insert( QString::fromLatin1("SCG"), i18n("Serbia") );
        countryCodeMap.insert( QString::fromLatin1("SYC"), i18n("Seychelles") );
        countryCodeMap.insert( QString::fromLatin1("SLE"), i18n("Sierra Leone") );
        countryCodeMap.insert( QString::fromLatin1("SGP"), i18n("Singapore") );
        countryCodeMap.insert( QString::fromLatin1("SVK"), i18n("Slovakia") );
        countryCodeMap.insert( QString::fromLatin1("SVN"), i18n("Slovenia") );
        countryCodeMap.insert( QString::fromLatin1("SLB"), i18n("Solomon Islands") );
        countryCodeMap.insert( QString::fromLatin1("SOM"), i18n("Somalia") );
        countryCodeMap.insert( QString::fromLatin1("ZAF"), i18n("South Africa") );
        countryCodeMap.insert( QString::fromLatin1("SGS"), i18n("South Georgia and the South Sandwich Islands") );
        countryCodeMap.insert( QString::fromLatin1("ESP"), i18n("Spain") );
        countryCodeMap.insert( QString::fromLatin1("LKA"), i18n("Sri Lanka") );
        countryCodeMap.insert( QString::fromLatin1("SDN"), i18n("Sudan") );
        countryCodeMap.insert( QString::fromLatin1("SUR"), i18n("Suriname") );
        countryCodeMap.insert( QString::fromLatin1("SJM"), i18n("Svalbard & Jan Mayen Islands") );
        countryCodeMap.insert( QString::fromLatin1("SWZ"), i18n("Swaziland") );
        countryCodeMap.insert( QString::fromLatin1("SWE"), i18n("Sweden") );
        countryCodeMap.insert( QString::fromLatin1("CHE"), i18n("Switzerland") );
        countryCodeMap.insert( QString::fromLatin1("SYR"), i18n("Syrian Arab Republic") );
        countryCodeMap.insert( QString::fromLatin1("TWN"), i18n("Taiwan") );
        countryCodeMap.insert( QString::fromLatin1("TJK"), i18n("Tajikistan") );
        countryCodeMap.insert( QString::fromLatin1("TZA"), i18n("Tanzania") );
        countryCodeMap.insert( QString::fromLatin1("THA"), i18n("Thailand") );
        countryCodeMap.insert( QString::fromLatin1("TLS"), i18n("Timor-Leste") );
        countryCodeMap.insert( QString::fromLatin1("TGO"), i18n("Togo") );
        countryCodeMap.insert( QString::fromLatin1("TKL"), i18n("Tokelau Islands") );
        countryCodeMap.insert( QString::fromLatin1("TON"), i18n("Tonga") );
        countryCodeMap.insert( QString::fromLatin1("TTO"), i18n("Trinidad and Tobago") );
        countryCodeMap.insert( QString::fromLatin1("TUN"), i18n("Tunisia") );
        countryCodeMap.insert( QString::fromLatin1("TUR"), i18n("Turkey") );
        countryCodeMap.insert( QString::fromLatin1("TKM"), i18n("Turkmenistan") );
        countryCodeMap.insert( QString::fromLatin1("TCA"), i18n("Turks and Caicos Islands") );
        countryCodeMap.insert( QString::fromLatin1("TUV"), i18n("Tuvalu") );
        countryCodeMap.insert( QString::fromLatin1("VIR"), i18n("US Virgin Islands") );
        countryCodeMap.insert( QString::fromLatin1("UGA"), i18n("Uganda") );
        countryCodeMap.insert( QString::fromLatin1("UKR"), i18n("Ukraine") );
        countryCodeMap.insert( QString::fromLatin1("ARE"), i18n("United Arab Emirates") );
        countryCodeMap.insert( QString::fromLatin1("GBR"), i18n("United Kingdom") );
        countryCodeMap.insert( QString::fromLatin1("UMI"), i18n("United States Minor Outlying Islands") );
        countryCodeMap.insert( QString::fromLatin1("USA"), i18n("United States of America") );
        countryCodeMap.insert( QString::fromLatin1("URY"), i18n("Uruguay, Eastern Republic of") );
        countryCodeMap.insert( QString::fromLatin1("UZB"), i18n("Uzbekistan") );
        countryCodeMap.insert( QString::fromLatin1("VUT"), i18n("Vanuatu") );
        countryCodeMap.insert( QString::fromLatin1("VEN"), i18n("Venezuela") );
        countryCodeMap.insert( QString::fromLatin1("VNM"), i18n("Viet Nam") );
        countryCodeMap.insert( QString::fromLatin1("WLF"), i18n("Wallis and Futuna Islands ") );
        countryCodeMap.insert( QString::fromLatin1("ESH"), i18n("Western Sahara") );
        countryCodeMap.insert( QString::fromLatin1("YEM"), i18n("Yemen") );
        countryCodeMap.insert( QString::fromLatin1("ZMB"), i18n("Zambia") );
        countryCodeMap.insert( QString::fromLatin1("ZWE"), i18n("Zimbabwe") );

        // Supplemental IPTC/IIM country codes.

        countryCodeMap.insert( QString::fromLatin1("XUN"), i18n("United Nations") );
        countryCodeMap.insert( QString::fromLatin1("XEU"), i18n("European Union") );
        countryCodeMap.insert( QString::fromLatin1("XSP"), i18n("Space") );
        countryCodeMap.insert( QString::fromLatin1("XSE"), i18n("At Sea") );
        countryCodeMap.insert( QString::fromLatin1("XIF"), i18n("In Flight") );
        countryCodeMap.insert( QString::fromLatin1("XEN"), i18n("England") );
        countryCodeMap.insert( QString::fromLatin1("XSC"), i18n("Scotland") );
        countryCodeMap.insert( QString::fromLatin1("XNI"), i18n("Northern Ireland") );
        countryCodeMap.insert( QString::fromLatin1("XWA"), i18n("Wales") );
        countryCodeMap.insert( QString::fromLatin1("PSE"), i18n("Palestine") );
        countryCodeMap.insert( QString::fromLatin1("GZA"), i18n("Gaza") );
        countryCodeMap.insert( QString::fromLatin1("JRO"), i18n("Jericho") );
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

}  // namespace Digikam
